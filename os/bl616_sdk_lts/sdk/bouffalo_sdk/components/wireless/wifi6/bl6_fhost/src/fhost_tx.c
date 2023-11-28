/**
 ****************************************************************************************
 *
 * @file fhost_tx.c
 *
 * @brief Implementation of the fully hosted TX task.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST_TX
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rtos.h"
#include "fhost_tx.h"
#include "fhost.h"
#include "mac_frame.h"
#include "co_endian.h"
#include "fhost_rx.h"
#if RW_MESH_EN
#include "mesh_task.h"
#endif

/// Type of packet to send
enum fhost_tx_buf_type
{
    /// Packet with an ethernet header (from the network stack)
    IEEE802_3,
    /// Packet with a wifi header
    IEEE802_11,
};

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
struct fhost_tx_env_tag fhost_tx_env;

/// Fake function used to detected too small TX headroom
void net_all_tx_headroom_too_small(void);

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Return pointer to fhost_tx_desc_tag from a txdesc pointer
 *
 * @param[in] txdesc TX desc
 * @return pointer to the fhost_tx_desc_tag structure that contains this txdesc
 ****************************************************************************************
 */
#if 0
__INLINE struct fhost_tx_desc_tag *fhost_tx_desc(struct txdesc *txdesc)
{
    void *desc = txdesc - offsetof(struct fhost_tx_desc_tag, txdesc);
    return desc;
}
#endif

#if NX_BEACONING
/**
 ****************************************************************************************
 * @brief Start PS clean up timer for a STA
 *
 * This timer is used to drop packet that remain queued for too long
 * (cf @ref fhost_tx_ps_cleanup_cb)
 * Does nothing if timer is already started.
 *
 * @param[in] sta_id  Station Index
 ****************************************************************************************
 */
void fhost_tx_ps_cleanup_timer_start(int sta_id)
{
    struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
    uint32_t timer_value;

    if (tfc->ps_cleanup_tmr.time)
        // timer already set
        return;

    timer_value = inline_hal_machw_time() + tfc->ps_max_queue_time;

    // The time value is tested to check whether the timer is started or not
    // so ensure that timer is never configured with time=0
    if (!timer_value)
        timer_value ++;

    mm_timer_set(&tfc->ps_cleanup_tmr, timer_value);
}

/**
 ****************************************************************************************
 * @brief Stop PS clean up timer for a STA
 *
 * Does nothing if timer is not started.
 *
 * @param[in] sta_id  Station Index
 ****************************************************************************************
 */
void fhost_tx_ps_cleanup_timer_stop(int sta_id)
{
    struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];

    if (!tfc->ps_cleanup_tmr.time)
        // timer not active
        return;

    mm_timer_clear(&tfc->ps_cleanup_tmr);
    tfc->ps_cleanup_tmr.time = 0;
}

/**
 ****************************************************************************************
 * @brief Send traffic information update to Wifi Task
 *
 * No need to wait confirmation from Wifi Task for this
 * It also start/stop the PS cleanup timer.
 *
 * @param[in] sta_id    Station Index
 * @param[in] tx_avail  Whether traffic is available or not
 * @param[in] ps_type   Type of PS traffic updated
 ****************************************************************************************
 */
__INLINE void fhost_tx_ps_traffic_ind(uint8_t sta_id, bool tx_avail,
                                      enum fhost_tx_ps_type ps_type)
{
    void *req = me_traffic_ind_req_alloc();

    me_traffic_ind_req_set_sta_idx(req, sta_id);
    me_traffic_ind_req_set_tx_avail(req, tx_avail);
    me_traffic_ind_req_set_uapsd(req, ps_type == PS_UAPSD);
    macif_kmsg_push(ME_TRAFFIC_IND_REQ, TASK_ME, req, me_traffic_ind_req_len());

    if (tx_avail)
        fhost_tx_ps_cleanup_timer_start(sta_id);
    else
        fhost_tx_ps_cleanup_timer_stop(sta_id);
}

#endif // NX_BEACONING

/**
 ****************************************************************************************
 * @brief Queue one buffer in the ready list of a TXQ
 *
 * @param[in] txq    TXQ structure
 * @param[in] desc   Descriptor of the buffer to queue
 * @param[in] retry  True if this is a retry pkt
 ****************************************************************************************
 */
__INLINE void fhost_tx_queue(struct fhost_tx_queue_tag *txq,
                             struct fhost_tx_desc_tag *desc,
                             bool retry)
{
    #if NX_BEACONING
    uint8_t sta_id = desc->txdesc.api.host.staid;

    if ((sta_id != INVALID_STA_IDX) && (!retry))
    {
        struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
        if (tfc->ps_ready[txq->ps_type] >= 0)
        {
            tfc->ps_ready[txq->ps_type]++;
            if (tfc->ps_ready[txq->ps_type] == 1)
                fhost_tx_ps_traffic_ind(sta_id, true, txq->ps_type);
        }

        desc->ctrl.timeout = inline_hal_machw_time() + tfc->ps_max_queue_time;
    }
    #endif

    txq->nb_ready++;
    if (retry)
    {
        co_list_insert_after_fast(&txq->ready,
                                  (txq->last_retry) ? &txq->last_retry->hdr : NULL,
                                  &desc->hdr);
        txq->last_retry = desc;
    }
    else
        co_list_push_back(&txq->ready, &desc->hdr);
}

/**
 ****************************************************************************************
 * @brief Dequeue the oldest buffer from the ready list of a TXQ
 *
 * @param[in] txq  TXQ structure
 * @return The oldest pkt queued for this TXQ (may be NULL)
 ****************************************************************************************
 */
__INLINE struct fhost_tx_desc_tag *fhost_tx_dequeue(struct fhost_tx_queue_tag *txq)
{
    struct fhost_tx_desc_tag *desc;

    desc = (struct fhost_tx_desc_tag *)co_list_pop_front(&txq->ready);
    txq->nb_ready--;

    if (desc == txq->last_retry)
        txq->last_retry = NULL;

    return desc;
}

/**
 ****************************************************************************************
 * @brief Dequeue the oldest buffer from the ready list of a TXQ and update its
 * descriptor before pushing it WIFI Task.
 *
 * @param[in] txq  TXQ structure
 * @return The oldest pkt queued for this TXQ
 ****************************************************************************************
 */
__INLINE struct fhost_tx_desc_tag *fhost_tx_dequeue_for_push(struct fhost_tx_queue_tag *txq)
{
    struct fhost_tx_desc_tag *desc = fhost_tx_dequeue(txq);
    ASSERT_ERR(desc);

    #if NX_BEACONING
    uint8_t sta_id = desc->txdesc.api.host.staid;
    if (sta_id != INVALID_STA_IDX)
    {
        struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
        if (tfc->sp_cnt[txq->ps_type] >= 0)
        {
            tfc->sp_cnt[txq->ps_type]--;
            if ((tfc->sp_cnt[txq->ps_type] == 0) &&
                (txq->ps_type == PS_UAPSD))
                desc->txdesc.api.host.flags |= TXU_CNTRL_EOSP;

            if (tfc->sp_cnt[txq->ps_type] ||
                tfc->ps_ready[txq->ps_type])
                desc->txdesc.api.host.flags |= TXU_CNTRL_MORE_DATA;
            else
                fhost_tx_ps_traffic_ind(sta_id, false, txq->ps_type);
        }
    }
    #endif

    // LMAC firmware expects a TID of 0xFF for non Qos frame
    if (desc->txdesc.api.host.tid == TID_MGT)
        desc->txdesc.api.host.tid = 0xFF;

    return desc;
}

/**
 ****************************************************************************************
 * @brief Retrieve the destination station index from the destination MAC address and VIF
 * information in case of MESH.
 *
 * Parse all existing mesh_path of the vif and check if the destination adress is present
 * as Next Hop STA. If it is not the case, create a new path with a MESH_PATH_CREATE_REQ
 * request.
 *
 * @param[in] vif Pointer to the VIF structure
 * @param[in] dst_addr Pointer to the destination MAC address in the Ethernet header
 *
 * @return The station index
 ****************************************************************************************
 */
#if RW_MESH_EN
static uint8_t fhost_tx_get_mesh_staid(struct fhost_vif_tag *vif, struct mac_addr *dst_addr)
{
    /* Path to be used */
    struct fhost_mesh_path *mesh_path;
    mesh_path = (struct fhost_mesh_path *) co_list_pick(&vif->mpath_list);
    bool found = 0;

    while (mesh_path != NULL)
    {
        if (!memcmp(&mesh_path->tgt_mac_addr, dst_addr, MAC_ADDR_LEN))
        {
            found = 1;
            break;
        }
        mesh_path = (struct fhost_mesh_path *) co_list_next((struct co_list_hdr *)mesh_path);
    }

    if (found)
    {
        return mesh_path->nhop_sta;
    }
    else
    {
        struct mesh_path_create_req req;
        req.vif_idx = mac_vif_get_index(vif->mac_vif);
        req.has_orig_addr = false;
        memcpy(&req.tgt_mac_addr, dst_addr, MAC_ADDR_LEN);

        macif_kmsg_push(MESH_PATH_CREATE_REQ, TASK_MESH, &req, sizeof(req));

        return INVALID_STA_IDX;
    }
}
#endif

/**
 ****************************************************************************************
 * @brief Retrieve the destination station index from the destination MAC address and VIF
 * information.
 *
 * @param[in] vif Pointer to the VIF structure
 * @param[in] dst_addr Pointer to the destination MAC address in the Ethernet header
 * @param[in] mgmt_frame Whether Frame is management or not
 *
 * @return The station index
 ****************************************************************************************
 */
static uint8_t fhost_tx_get_staid(struct fhost_vif_tag *vif, struct mac_addr *dst_addr, bool mgmt_frame)
{
    uint8_t sta_id = INVALID_STA_IDX;

    switch (mac_vif_get_type(vif->mac_vif))
    {
        case VIF_STA:
            if (mac_vif_get_active(vif->mac_vif))
                sta_id = vif->ap_id;
            break;
        case VIF_AP:
            if (MAC_ADDR_GROUP(dst_addr))
                sta_id = VIF_TO_BCMC_IDX(mac_vif_get_index(vif->mac_vif));
            else
                sta_id = vif_mgmt_get_staid(vif->mac_vif, dst_addr);
            break;
        #if RW_MESH_EN
        case VIF_MESH_POINT:
            if (MAC_ADDR_GROUP(dst_addr))
                sta_id = VIF_TO_BCMC_IDX(mac_vif_get_index(vif->mac_vif));
            else if (mgmt_frame)
                sta_id = vif_mgmt_get_staid(vif->mac_vif, dst_addr);
            else
                sta_id = fhost_tx_get_mesh_staid(vif, dst_addr);
            break;
        #endif //RW_MESH_EN
        default:
            break;
    }

    return sta_id;
}

/**
 ****************************************************************************************
 * @brief Retrieve the TX queue index to use for a transmission
 *
 * If the sta_id is valid, then the function returns the TXQ associated to the given
 * sta_id/tid pair. In this case vif_idx is not used.
 * If sta_id is not valid, then the function returns the TXQ associated to the VIF and
 * in this case tid is not used
 * If neither sta_id nor vif_idx is valid then NULL is returned.
 *
 * @param[in] vif_idx  Index of the MAC VIF
 * @param[in] sta_id   Index of the destination STA
 * @param[in] tid      TID to use for the transmission
 *
 * @return The associated TXQ or NULL if cannot find the txq
 ****************************************************************************************
 */
static struct fhost_tx_queue_tag *fhost_tx_get_txq(uint8_t vif_idx, uint8_t sta_id,
                                                   uint8_t tid)
{
    uint8_t txq_id;

    if (sta_id < STA_MAX)
    {
        if (tid > TID_MAX)
            tid = 0;

        txq_id = (sta_id * TID_MAX + tid);
    }
    else if (vif_idx < NX_VIRT_DEV_MAX)
    {
        txq_id = (STA_MAX * TID_MAX) + vif_idx;
    }
    else
    {
        return NULL;
    }

    ASSERT_ERR(txq_id < FHOST_TXQ_CNT);
    return &fhost_tx_env.tx_q[txq_id];
}


/// Mapping table of access category and traffic ID
static const int fhost_tx_ac2tid[3] = {
    [AC_BK] = TID_2,
    [AC_BE] = TID_0,
    [AC_VI] = TID_5,
};

/**
 ****************************************************************************************
 * @brief return the downgraded tid which corresponds VIF
 *
 * @param[in] vif Pointer to the VIF structure
 * @param[in] tid Current TID
 *
 * @return The downgraded TID
 ****************************************************************************************
 */
static uint8_t fhost_tx_downgrade_ac(struct fhost_vif_tag *vif, uint8_t tid)
{
    int8_t ac = mac_tid2ac[tid];

    while ((vif->acm & CO_BIT(ac)) && (ac > AC_BK))
    {
        ac--;
        tid = fhost_tx_ac2tid[ac];
    }
    return tid;
}

/**
 ****************************************************************************************
 * @brief Add a TX queue to the scheduling process.
 * If the queue is already in the scheduling process, do nothing. If no queue is currently
 * in the scheduling process, the TXQ pushed will be the first scheduled. Otherwise the
 * TXQ is scheduled right after the queue currently scheduled.
 *
 * @param[in]     txq    Pointer to the TX queue to add to the scheduling process
 * @param[in,out] sched  Pointer to the scheduling element
 ****************************************************************************************
 */
static int fhost_tx_add_txq_to_sched(struct fhost_tx_queue_tag *txq,
                                      struct fhost_tx_queue_sched_tag *sched)
{
    // Nothing to do if the queue is already in the scheduling list
    if (txq->status & TXQ_IN_HWQ_LIST) {
        //printf("[FH]%s  Nothing to do if the queue is already in the scheduling lis (Line:%d)!\r\n", __func__, __LINE__);
        return -4;
    }

    // Check if some TXQs are already part of this scheduling element
    if (sched->current)
    {
        struct fhost_tx_queue_tag *current = sched->current;

        // Insert the TXQ right after the currently scheduled queue
        txq->next = current->next;
        txq->prev = current;
        current->next->prev = txq;
        current->next = txq;
    }
    else
    {
        // No TXQ being scheduled now, add this one
        sched->current = txq;
        txq->next = txq;
        txq->prev = txq;
    }

    // Update the status
    txq->status |= TXQ_IN_HWQ_LIST;
    return 0;
}

/**
 ****************************************************************************************
 * @brief Remove a TX queue from the scheduling process.
 * The TX queue is extracted from the sheduling list. If the TX queue is the one currently
 * scheduled, then the pointer to the current queue is updated.
 *
 * @param[in]     txq    Pointer to the TX queue to remove from the scheduling process
 * @param[in,out] sched  Pointer to the scheduling element
 ****************************************************************************************
 */
static void fhost_tx_del_txq_from_sched(struct fhost_tx_queue_tag *txq,
                                        struct fhost_tx_queue_sched_tag *sched)
{
    // Sanity check - The queue shall be in the scheduling list
    ASSERT_ERR((txq->status & TXQ_IN_HWQ_LIST) || (!txq->status));

    // Check if the TXQ is alone in the scheduling element
    if (txq->next == txq)
    {
        sched->current = NULL;
    }
    else
    {
        // Extract the element from the scheduling list and update the current one
        txq->next->prev = txq->prev;
        txq->prev->next = txq->next;
        if (sched->current == txq)
        {
            sched->current = txq->next;
        }
    }

    // Reset the status of the queue
    txq->status &= ~TXQ_IN_HWQ_LIST;
}

/**
 ****************************************************************************************
 * @brief Schedule the TX queues ready on a specific access category.
 *
 * This function goes through the queues attached to the scheduling element of this
 * access category and pushes as many packets as possible to the MAC.
 *
 * @param[in] ac Access category to be scheduled
 ****************************************************************************************
 */
static void fhost_tx_schedule(int ac)
{
    struct fhost_tx_queue_sched_tag *sched = &fhost_tx_env.sched[ac];
    bool pushed = false;

    // Loop until there is a TXQ ready to be scheduled, and there is space in the AC queue
    while (fhost_tx_env.hwq_credits[ac] && sched->current)
    {
        struct fhost_tx_queue_tag *txq = sched->current;

        // Go through the TX queue and push for transmission as many packets as possible
        while (fhost_tx_env.hwq_credits[ac])
        {
            struct txdesc_host *txdesc;
            struct fhost_tx_desc_tag *desc;

            // Check if we can still get data from the TX queue
            if (co_list_is_empty(&txq->ready) || (txq->credits <= 0))
            {
                // Queue empty or no credits anymore, remove TXQ from scheduling
                // This will also update the currently scheduled queue
                fhost_tx_del_txq_from_sched(txq, sched);
                break;
            }

            // Get the packet descriptor from the TX queue and update it if needed
            desc = fhost_tx_dequeue_for_push(txq);
            txdesc = &desc->txdesc;
            txdesc->ctrl.hwq = ac;

            // Push the packet to the WiFi task
            rtos_queue_write(fhost_tx_env.queue_tx, &txdesc, -1, false);

            // Update the number of credits of the queue and its status if required
            pushed = true;
            fhost_tx_env.hwq_credits[ac]--;
            txq->credits--;
            if (txq->credits <= 0)
                txq->status |= TXQ_STOP_FULL;

            #if NX_BEACONING
            if (txq->limit && (--txq->limit == 0))
            {
                fhost_tx_del_txq_from_sched(txq, sched);
                break;
            }
            #endif
        }
    }

    // Check if we need to signal the pushed packets to the WiFi task
    if (pushed)
    {
        macif_tx_data_ind(ac);
    } else {
        printf("[FH]pushed:%d\r\n", pushed);
    }
}

/**
 ****************************************************************************************
 * @brief Release a Rx buffer pushed by upper layer
 *
 * This is called whether the Tx buffer has been transmitted or discarded.
 *
 * @param[in] buf_rx     Rx Buffer to release
 ****************************************************************************************
 */
static void fhost_tx_release_buf_rx(struct fhost_rx_buf_tag *buf_rx)
{
    if (buf_rx != NULL)
        fhost_rx_buf_push(buf_rx);
}

/**
 ****************************************************************************************
 * @brief Release a buffer pushed by upper layer
 *
 * This is called whether the buffer has been transmitted or discarded.
 *
 * @param[in] net_buf     Net Buffer to release
 * @param[in] tx_status   Buffer TX status as reported by umac (0 if discarded)
 * @param[in] cfm_cb      Confirmation callback for the buffer
 * @param[in] cfm_cb_arg  Confirmation callback private argument
 * @param[in] buf_rx      Rx Buffer to release
 ****************************************************************************************
 */
static void fhost_tx_release_buf(net_buf_tx_t *net_buf, uint32_t tx_status,
                                 cb_fhost_tx cfm_cb, void *cfm_cb_arg,
                                 struct fhost_rx_buf_tag *buf_rx )
{
    bool is_hold_mutex;
    if (cfm_cb)
    {
        is_hold_mutex = (rtos_get_task_handle() == rtos_mutex_get_holder(fhost_tx_env.tx_lock));
        if (is_hold_mutex) {
            FHOST_TX_UNLOCK();
        }
        cfm_cb((uint32_t)net_buf, tx_status & TX_STATUS_ACKNOWLEDGED, cfm_cb_arg);
        if (is_hold_mutex) {
            FHOST_TX_LOCK();
        }
    }
    net_buf_tx_free(net_buf);

    fhost_tx_release_buf_rx(buf_rx);
}

#if NX_BEACONING
/**
 ****************************************************************************************
 * @brief PS clean up timer callback
 *
 * Check each TXQ for the STA and drop packet that reached their timeout limit.
 * Timer is restarted if PS packet are still queued.
 *
 * @param[in] env  Callback registered parameter, in this case it is the station index
 ****************************************************************************************
 */
static void fhost_tx_ps_cleanup_cb(void *env)
{
    struct fhost_tx_queue_tag *txq;
    struct fhost_tx_sta_traffic *tfc;
    int sta_id = (int)env;
    int ps, tid;
    int txq_cnt = TID_MAX;
    int ps_dropped[PS_TYPE_CNT];

    if (sta_id > STA_MAX)
        return;

    txq = fhost_tx_get_txq(sta_idx_get_inst_nbr(sta_id), sta_id, 0);
    if (!txq)
        return;

    if (sta_id > NX_REMOTE_STA_MAX)
        txq_cnt = 1;

    memset(ps_dropped, 0, sizeof(ps_dropped));
    tfc = &fhost_tx_env.traffic[sta_id];
    tfc->ps_cleanup_tmr.time = 0;

    for (tid = 0; txq_cnt > 0; txq_cnt--, txq++, tid++)
    {
        struct fhost_tx_desc_tag *desc;

        if (!(txq->status & TXQ_ENABLED))
            return;

        if (!txq->nb_ready)
            continue;

        FHOST_TX_LOCK();
        desc = (struct fhost_tx_desc_tag *)co_list_pick(&txq->ready);
        while (desc)
        {
            if (!inline_hal_machw_time_past(desc->ctrl.timeout))
                break;

            desc = fhost_tx_dequeue(txq);
            if (tfc->sp_cnt[txq->ps_type] > 0)
            {
                ps_dropped[txq->ps_type]++;
                tfc->sp_cnt[txq->ps_type]--;
                if (txq->limit)
                    txq->limit--;
            }
            else if (tfc->ps_ready[txq->ps_type] > 0)
            {
                ps_dropped[txq->ps_type]++;
                tfc->ps_ready[txq->ps_type]--;
            }

            fhost_tx_release_buf(desc->ctrl.buf, 0,
                                 desc->ctrl.cfm_cb, desc->ctrl.cfm_cb_arg,
                                 desc->ctrl.buf_rx);

            desc = (struct fhost_tx_desc_tag *)co_list_pick(&txq->ready);
        }

        // very unlikely
        if (!desc && (txq->status & TXQ_IN_HWQ_LIST))
            fhost_tx_del_txq_from_sched(txq, &fhost_tx_env.sched[mac_tid2ac[tid]]);

        FHOST_TX_UNLOCK();
    }


    for (ps = PS_LEGACY; ps < PS_TYPE_CNT; ps++)
    {
        if (ps_dropped[ps] && (tfc->ps_ready[ps] + tfc->sp_cnt[ps]) == 0)
            fhost_tx_ps_traffic_ind(sta_id, false, ps);
        else if ((tfc->ps_ready[ps] > 0) || (tfc->sp_cnt[ps] > 0))
            fhost_tx_ps_cleanup_timer_start(sta_id);
    }
}

#endif // NX_BEACONING
/**
 ****************************************************************************************
 * @brief Push a network buffer to a TX queue.
 *
 * This function is directly call from the network stack thread.
 * It is called with the tx mutex hold
 *
 * @param[in] net_if      Pointer to the net interface for which the packet is pushed
 * @param[in] net_buf     Pointer to the net buffer to transmit.
 * @param[in] type        Type of buffer
 * @param[in] cfm_cb      Confirmation callback for the buffer (Only if type == IEEE802_11)
 * @param[in] cfm_cb_arg  Private argument for confirmation callback
 * @param[in] buf_rx      Pointer to Rx buffer structure
 * @param[in] no_cck      Whether CCK rates must not be used to transmit this frame
 ****************************************************************************************
 */
static int fhost_tx_req(net_if_t *net_if, net_buf_tx_t *net_buf, enum fhost_tx_buf_type type,
                         cb_fhost_tx cfm_cb, void *cfm_cb_arg, struct fhost_rx_buf_tag* buf_rx,
                         int no_cck)
{
    struct fhost_tx_desc_tag *desc;
    struct fhost_tx_ctrl_tag *ctrl;
    struct hostdesc *host;
    struct fhost_tx_queue_tag *txq;
    struct fhost_vif_tag *vif;
    uint32_t seg_addr[TX_PBD_CNT];
    uint16_t seg_len[TX_PBD_CNT];
    int seg_cnt = TX_PBD_CNT;
    uint16_t buf_len;

    desc = net_buf_tx_info(net_buf, &buf_len, &seg_cnt, seg_addr, seg_len);

    if (!desc || !seg_cnt || seg_cnt > TX_PBD_CNT)
    {
        fhost_tx_release_buf(net_buf, 0, cfm_cb, cfm_cb_arg, buf_rx);
        printf("[FH]%s relese this buf(Line:%d)!\r\n", __func__, __LINE__);
        return -1;
    }

    vif = net_if_vif_info(net_if);

    ctrl = &desc->ctrl;
    host = &desc->txdesc.api.host;
    ctrl->buf = net_buf;
    ctrl->buf_rx = buf_rx;
    ctrl->cfm_cb = cfm_cb;
    ctrl->cfm_cb_arg = cfm_cb_arg;
    host->hostid = desc;
    if (no_cck)
        host->flags = TXU_CNTRL_MGMT_NO_CCK;
    else
        host->flags = 0;
    host->vif_idx = mac_vif_get_index(vif->mac_vif);

    if (type == IEEE802_3)
    {
        struct mac_eth_hdr *eth_hdr = (struct mac_eth_hdr *)seg_addr[0];

        // Parse Ethernet header
        host->eth_dest_addr = eth_hdr->da;
        host->eth_src_addr = eth_hdr->sa;
        host->ethertype = eth_hdr->len;

        if (co_ntohs(host->ethertype) == LLC_ETHERTYPE_IP)
        {
            uint8_t dscp;

            // Read DSCP field in the IP header to determine TID (2nd byte after the header)
            if (buf_rx != NULL)
                dscp = *((uint8_t *) seg_addr[1] + 1 );
            else
                dscp = *((uint8_t *)(eth_hdr + 1) + 1);

            host->tid = (dscp & 0xFC) >> 5;
        }
        else
        {
            host->tid = 0;
        }

        // and skip it once info have been extracted
        seg_addr[0] += sizeof(*eth_hdr);
        seg_len[0] -= sizeof(*eth_hdr);
        buf_len -= sizeof(*eth_hdr);
    }
    else if (type == IEEE802_11)
    {
        struct mac_hdr *mac_hdr = (struct mac_hdr *)seg_addr[0];
        mac_hdr->seq = 0;
        host->eth_dest_addr = mac_hdr->addr1;
        host->tid = TID_MGT;
        host->flags |= TXU_CNTRL_MGMT;
        #if NX_MFP
        if (mfp_is_robust_frame(mac_hdr->fctl, *((uint8_t *)(mac_hdr + 1))))
            host->flags |= TXU_CNTRL_MGMT_ROBUST;
        #endif
    }

    host->packet_addr[0] = seg_addr[0];
    host->packet_len[0] = buf_len;

    // Get destination STA id
    host->staid = fhost_tx_get_staid(vif, &host->eth_dest_addr, host->flags & TXU_CNTRL_MGMT);

    // Update TID if necessary
    if (vif->acm)
        host->tid = fhost_tx_downgrade_ac(vif, host->tid);

#ifndef CFG_BCN_AC_MAX
    // XXX hacks here, remove it later
    if (type == IEEE802_3 && (host->tid == TID_4 || host->tid == TID_5))
    {
        host->tid = TID_6;
    }
#endif

    // Check if the queue is active
    txq = fhost_tx_get_txq(host->vif_idx, host->staid, host->tid);
    if (!txq || !(txq->status & TXQ_ENABLED))
    {
        fhost_tx_release_buf(net_buf, 0, cfm_cb, cfm_cb_arg, buf_rx);
        printf("[FH]%s relese this buf(Line:%d)!\r\n", __func__, __LINE__);
        return -2;
    }

    #if RW_MESH_EN
    if ((vif->mac_vif->type == VIF_MESH_POINT) &&
        (host->staid < NX_REMOTE_STA_MAX))
    {
        host->flags |= TXU_CNTRL_USE_4ADDR;
    }

    if ((vif->mac_vif->type == VIF_MESH_POINT) &&
        (vif->is_resending))
    {
        host->flags |= TXU_CNTRL_MESH_FWD;
    }
    #endif

    tx_desc_init_for_fhost(desc, seg_cnt, seg_addr, seg_len);
    // Queue descriptor
    fhost_tx_queue(txq, desc, false);

    // Check if we can add the TX queue to the schedule list
    if (!(txq->status & TXQ_STOP))
    {
        int ac = mac_tid2ac[host->tid];
        fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
        fhost_tx_schedule(ac);
        return 0;
    } else {
        //printf("[FH]%s add tx queue fail (Line:%d)!\r\n", __func__, __LINE__);
        return -3;
    }

}

/**
 ****************************************************************************************
 * @brief Process a TX confirmation from the MAC.
 *
 * This function is directly called from the MAC thread after processing a CFM event
 * from the MACHW.
 * It is called with the tx mutex hold
 * Depending on the status of the confirmation, the packet might be freed and the
 * TXQ corresponding to the transmission scheduled again for transmission.
 * If the packet needs to be retried, it is pushed back to the MAC immediately.
 *
 * @param[in] txdesc Pointer to the TX descriptor that is confirmed.
 ****************************************************************************************
 */
static void fhost_tx_cfm(struct txdesc *txdesc)
{
    struct fhost_tx_queue_tag *txq;
    struct fhost_tx_desc_tag *desc = txdesc->host.hostid;
    struct fhost_tx_ctrl_tag *ctrl = &desc->ctrl;
    struct hostdesc *host = &desc->txdesc.api.host;
    int ac;
    uint32_t status = txdesc->host.cfm.status;
    int8_t credits = txdesc->host.cfm.credits;

    if (txdesc->host.tid == 0xFF)
        txdesc->host.tid = TID_MGT;

    ac = desc->txdesc.ctrl.hwq;

    txq = fhost_tx_get_txq(txdesc->host.vif_idx, txdesc->host.staid, txdesc->host.tid);
    ASSERT_ERR(txq);

    // New credits available
    fhost_tx_env.hwq_credits[ac]++;

    // Release the MAC descriptor
    tx_txdesc_release(txdesc);

    // Check if the queue is active
    if (!(txq->status & TXQ_ENABLED))
    {
        fhost_tx_release_buf(ctrl->buf, status,
                             ctrl->cfm_cb, ctrl->cfm_cb_arg,
                             ctrl->buf_rx);
    }
    // Check the status of the packet
    else if (status & TX_STATUS_SW_RETRY_REQUIRED)
    {
        txq->credits += credits;
        if (txq->credits > 0)
        {
            txq->status &= ~TXQ_STOP_FULL;
        }
        if (status & TX_STATUS_RETRY_REQUIRED) {
            uint16_t *flags = &host->flags;
            *flags |= TXU_CNTRL_REUSE_SN;
            host->sn_for_retry = (status & TX_STATUS_SN_MSK) >> TX_STATUS_SN_OFT;
        }
        fhost_tx_queue(txq, desc, true);
    }
    else if (status & TX_STATUS_DONE)
    {
        // Update the number of credits of the queue
        txq->credits += credits;
        if (txq->credits > 0)
        {
            txq->status &= ~TXQ_STOP_FULL;
        }

        fhost_tx_release_buf(ctrl->buf, status,
                             ctrl->cfm_cb, ctrl->cfm_cb_arg,
                             ctrl->buf_rx);

        // Check if we can add the TX queue to the schedule list
        if (!(txq->status & TXQ_STOP) && !co_list_is_empty(&txq->ready))
        {
            fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
            fhost_tx_schedule(ac);
        }
        #if NX_BEACONING
        else if (txq->limit && ((txq->status & TXQ_STOP) == TXQ_STOP_STA_PS))
        {
            fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
            fhost_tx_schedule(ac);
        }
        #endif // NX_BEACONING
    }
}

/**
 ****************************************************************************************
 * @brief Update the credits of a TX queue, upon the creation or deletion of a BlockAck
 * agreement of the association STA/TID pair.
 *
 * This function is called in the FHOST TX thread after getting the
 * @ref FHOST_TX_CREDITS_UPDATE message that was sent from the MAC thread.
 *
 * @param[in] sta_id Index of the station
 * @param[in] tid Priority
 * @param[in] credits Number of credits to add/remove on the TX queue
 ****************************************************************************************
 */
static void fhost_tx_do_credits_update(uint8_t sta_id, uint8_t tid, int8_t credits)
{
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, tid);
    uint8_t ac = mac_tid2ac[tid];

    // Check if queue is active
    if (!(txq->status & TXQ_ENABLED))
        return;

    // Update the credits
    txq->credits += credits;
    if (txq->credits > 0)
    {
        txq->status &= ~TXQ_STOP_FULL;
    }
    else
    {
        txq->status |= TXQ_STOP_FULL;
    }

    // Check if we have to add the TX queue to the schedule list
    if (!(txq->status & TXQ_STOP) && !co_list_is_empty(&txq->ready))
    {
        fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
    }
    #if NX_BEACONING
    else if (txq->limit && ((txq->status & TXQ_STOP) == TXQ_STOP_STA_PS))
    {
        fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
    }
    #endif // NX_BEACONING
    else if (txq->status & TXQ_IN_HWQ_LIST)
    {
        fhost_tx_del_txq_from_sched(txq, &fhost_tx_env.sched[ac]);
    }

    if (fhost_tx_env.sched[ac].current)
        fhost_tx_schedule(ac);
}


/**
 ****************************************************************************************
 * @brief Enable a TXQ queue.
 *
 * Set a TXQ as enabled and reset its internal status
 *
 * @param[in] txq      TXQ to enabled
 * @param[in] ps_type  PS type of the queue (only used for STA's TXQ)
 ****************************************************************************************
 */
static void fhost_tx_txq_enable(struct fhost_tx_queue_tag *txq, int8_t ps_type)
{
    txq->status = TXQ_ENABLED;
    txq->credits = NX_DEFAULT_TX_CREDIT_CNT;
    co_list_init(&txq->ready);
    txq->nb_ready = 0;
    txq->last_retry = NULL;
    #if NX_BEACONING
    txq->ps_type = ps_type;
    txq->limit = 0;
    #endif
}

/**
 ****************************************************************************************
 * @brief Enable the TX queues for the given STA.
 *
 * This function is called in the FHOST TX thread after getting the @ref FHOST_TX_STA_ADD
 * message that was sent from the control thread.
 *
 * @param[in] sta_id Station index to add
 ****************************************************************************************
 */
static void fhost_tx_do_sta_add(uint8_t sta_id)
{
    for (int tid = 0; tid < TID_MAX; tid++)
    {
        struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, tid);
        int8_t ps_type = PS_LEGACY;

        #if NX_BEACONING
        if (sta_idx_get_uapsd_queues(sta_id) & mac_ac2uapsd[mac_tid2ac[tid]])
            ps_type = PS_UAPSD;
        #endif

        fhost_tx_txq_enable(txq, ps_type);
    }

    #if NX_BEACONING
    fhost_tx_env.traffic[sta_id].ps_ready[PS_LEGACY] = -1;
    fhost_tx_env.traffic[sta_id].ps_ready[PS_UAPSD] = -1;
    fhost_tx_env.traffic[sta_id].sp_cnt[PS_LEGACY] = 0;
    fhost_tx_env.traffic[sta_id].sp_cnt[PS_UAPSD] = 0;
    fhost_tx_env.traffic[sta_id].ps_cleanup_tmr.cb = fhost_tx_ps_cleanup_cb;
    fhost_tx_env.traffic[sta_id].ps_cleanup_tmr.env = (void *)((int)sta_id);
    fhost_tx_env.traffic[sta_id].ps_cleanup_tmr.time = 0;
    #endif
}

/**
 ****************************************************************************************
 * @brief Disable the TX queues for the given STA.
 *
 * This function is called in the FHOST TX thread after getting the @ref FHOST_TX_STA_DEL
 * message that was sent from the control thread. It goes through all the TX queues
 * attached to this STA and free the packets pending in those queues.
 *
 * @param[in] sta_id Station index to delete
 ****************************************************************************************
 */
static void fhost_tx_do_sta_del(uint8_t sta_id)
{
    #if NX_BEACONING
    fhost_tx_ps_cleanup_timer_stop(sta_id);
    #endif

    for (int tid = 0; tid < TID_MAX; tid++)
    {
        struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, tid);
        struct fhost_tx_desc_tag *desc;

        // Go through the TX queue and free all the packets
        while ((desc = fhost_tx_dequeue(txq)))
        {
            fhost_tx_release_buf(desc->ctrl.buf, 0,
                                 desc->ctrl.cfm_cb,desc->ctrl.cfm_cb_arg,
                                 desc->ctrl.buf_rx);
        }

        // TX queue is now disabled
        txq->status = 0;
    }
}

/**
 ****************************************************************************************
 * @brief Enable the TX queues for the given VIF.
 *
 * This function is called in the FHOST TX thread after receiving the
 * @ref FHOST_TX_VIF_ENABLE message sent from the control thread.
 *
 * @param[in] vif_idx  MAC VIF index
 ****************************************************************************************
 */
static void fhost_tx_do_vif_enable(uint8_t vif_idx)
{
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(vif_idx, INVALID_STA_IDX, 0);
    fhost_tx_txq_enable(txq, PS_LEGACY);
}

/**
 ****************************************************************************************
 * @brief Disable the TX queues for the given VIF.
 *
 * This function is called in the FHOST TX thread after receiving the
 * @ref FHOST_TX_VIF_DISABLE message sent from the control thread.
 *
 * @param[in] vif_idx  MAC VIF index
 ****************************************************************************************
 */
static void fhost_tx_do_vif_disable(uint8_t vif_idx)
{
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(vif_idx, INVALID_STA_IDX, 0);
    struct fhost_tx_desc_tag *desc;

    // Go through the TX queue and free all the packets
    while ((desc = fhost_tx_dequeue(txq)))
    {
        fhost_tx_release_buf(desc->ctrl.buf, 0,
                             desc->ctrl.cfm_cb, desc->ctrl.cfm_cb_arg,
                             desc->ctrl.buf_rx);
    }

    txq->status = 0;
}

/**
 ****************************************************************************************
 * @brief Stop TX queues for a STA that enabled PS mode.
 *
 * @param[in] sta_id  Index of the station
 ****************************************************************************************
 */
static void fhost_tx_do_sta_enter_ps(uint8_t sta_id)
{
    #if NX_BEACONING
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, 0);
    struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
    int tid, ac, ps;

    for (ps = PS_LEGACY; ps < PS_TYPE_CNT; ps++)
    {
        tfc->ps_ready[ps] = 0;
        tfc->sp_cnt[ps] = 0;
    }

    for (tid = 0; tid < TID_MAX; tid++, txq++)
    {
        ac = mac_tid2ac[tid];
        txq->status |= TXQ_STOP_STA_PS;
        if (txq->status & TXQ_IN_HWQ_LIST)
            fhost_tx_del_txq_from_sched(txq, &fhost_tx_env.sched[ac]);
        tfc->ps_ready[txq->ps_type] += txq->nb_ready;
    }

    for (ps = PS_LEGACY; ps < PS_TYPE_CNT; ps++)
    {
        if (tfc->ps_ready[ps])
            fhost_tx_ps_traffic_ind(sta_id, true, ps);
    }
    #endif
}

/**
 ****************************************************************************************
 * @brief (re)start TXQ for a given reason
 *
 * @param[in] sta_id  Index of the station
 * @param[in] status  TXQ status flag
 ****************************************************************************************
 */
static void fhost_tx_sta_txq_start(uint8_t sta_id, int reason)
{
    int tid, ac, ac_to_sched = 0;
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, 0);

    for (tid = 0; tid < TID_MAX; tid++, txq++)
    {
        ac = mac_tid2ac[tid];
        #if NX_BEACONING
        if (reason & TXQ_STOP_STA_PS)
            txq->limit = 0;
        #endif
        txq->status &= ~reason;

        if (!(txq->status & TXQ_STOP) && txq->nb_ready)
        {
            fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
            ac_to_sched |= CO_BIT(ac);
        }
    }

    for (ac = AC_VO; ac >= AC_BK; ac--)
    {
        if (ac_to_sched & CO_BIT(ac))
            fhost_tx_schedule(ac);
    }
}

/**
 ****************************************************************************************
 * @brief Stop TXQ for a given reason
 *
 * @param[in] sta_id  Index of the station
 ****************************************************************************************
 */
static void fhost_tx_sta_txq_stop(uint8_t sta_id, int reason)
{
    int tid, ac;
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, 0);

    if (!txq || !(txq->status & TXQ_ENABLED))
        return;

    for (tid = 0; tid < TID_MAX; tid++, txq++)
    {
        ac = mac_tid2ac[tid];
        txq->status |= reason;

        if (txq->status & TXQ_IN_HWQ_LIST)
            fhost_tx_del_txq_from_sched(txq, &fhost_tx_env.sched[ac]);
    }
}

/**
 ****************************************************************************************
 * @brief Restart TX queues for a STA that disabled PS mode.
 *
 * @param[in] sta_id  Index of the station
 ****************************************************************************************
 */
static void fhost_tx_do_sta_exit_ps(uint8_t sta_id)
{
    #if NX_BEACONING
    struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
    int ps = 0;

    for (ps = PS_LEGACY; ps < PS_TYPE_CNT; ps++)
    {
        if (tfc->ps_ready[ps])
            fhost_tx_ps_traffic_ind(sta_id, false, ps);
        tfc->ps_ready[ps] = -1;
        tfc->sp_cnt[ps] = 0;
    }

    fhost_tx_sta_txq_start(sta_id, TXQ_STOP_STA_PS);

    #endif
}

/**
 ****************************************************************************************
 * @brief Call the function given as argument on all STA associated to the current VIF.
 *
 * @param[in] vif_idx  Index of the VIF
 * @param[in] control  Function pointer
 ****************************************************************************************
 */
static void fhost_tx_txq_vif_foreach_sta(uint8_t vif_idx, void (*control)(uint8_t, int))
{
    struct fhost_vif_tag *fhost_vif = NULL;
    fhost_vif = &fhost_env.vif[vif_idx];

    void *sta = co_list_pick(mac_vif_get_sta_list(fhost_vif->mac_vif));

    while (sta != NULL)
    {
        control(sta_get_staid(sta), TXQ_STOP_VIF_PS);
        sta = sta_get_list_next(sta);
    }
}

/**
 ****************************************************************************************
 * @brief Stop all TX queues of the VIF.
 *
 * This function is called in the FHOST TX thread after receiving the
 * @ref FHOST_TX_VIF_ENTER_PS message sent from the control thread.
 *
 * @param[in] vif_idx  Index of the VIF
 ****************************************************************************************
 */
static void fhost_tx_do_vif_enter_ps(uint8_t vif_idx)
{
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(vif_idx, INVALID_STA_IDX, 0);

    if (txq && (txq->status & TXQ_ENABLED))
    {
        txq->status |= TXQ_STOP_VIF_PS;
        if (txq->status & TXQ_IN_HWQ_LIST)
            fhost_tx_del_txq_from_sched(txq, &fhost_tx_env.sched[AC_VO]);
    }

    fhost_tx_txq_vif_foreach_sta(vif_idx, fhost_tx_sta_txq_stop);
}

/**
 ****************************************************************************************
 * @brief Start all TX queues of the VIF.
 *
 * This function is called in the FHOST TX thread after receiving the
 * @ref FHOST_TX_VIF_EXIT_PS message sent from the control thread.
 *
 * @param[in] vif_idx  Index of the VIF
 ****************************************************************************************
 */
static void fhost_tx_do_vif_exit_ps(uint8_t vif_idx)
{
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(vif_idx, INVALID_STA_IDX, 0);

    txq->status &= ~TXQ_STOP_VIF_PS;
    if (!(txq->status & TXQ_STOP) && txq->nb_ready)
    {
        fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[AC_VO]);
        fhost_tx_schedule(AC_VO);
    }

    fhost_tx_txq_vif_foreach_sta(vif_idx, fhost_tx_sta_txq_start);
}

/**
 ****************************************************************************************
 * @brief Configure STA's TXQ in order to push requested packet
 *
 *  This function is called in the FHOST TX thread after receiving the
 *  @ref FHOST_TX_PS_TRAFFIC_REQ message sent from the control thread.
 *
 * @param[in] sta_id   Index of the station
 * @param[in] req_cnt  Number of packet requested (0 means all)
 * @param[in] ps_type  Type of PS traffic requested
 ****************************************************************************************
 */
static void fhost_tx_do_ps_traffic_req(uint8_t sta_id, int req_cnt,
                                       enum fhost_tx_ps_type ps_type)
{
    #if NX_BEACONING
    struct fhost_tx_queue_tag *txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, TID_MGT);
    struct fhost_tx_sta_traffic *tfc = &fhost_tx_env.traffic[sta_id];
    int tid, sp_size = 0, ac, ac_to_sched = 0;;

    if (tfc->ps_ready[ps_type] == -1)
    {
        // ignore traffic request if STA is not in PS mode
#ifdef CFG_ENABLE_AP_PS
        ASSERT_WARN(0);
#endif
        return;
    }
    ASSERT_ERR(tfc->sp_cnt[ps_type] == 0);

    for (tid = TID_MGT; tid >= 0; tid--, txq--)
    {
        if (!txq->nb_ready || (ps_type != txq->ps_type))
            continue;

        if (!req_cnt || (txq->nb_ready < req_cnt))
            txq->limit = txq->nb_ready;
        else
            txq->limit = req_cnt;

        if (sta_id < NX_REMOTE_STA_MAX)
            ac = mac_tid2ac[tid];
        else
            // ac = AC_MAX; // use BCN queue for broadcast/multicast sta
            ac = AC_VO;   //XXX Issue 2

        if ((txq->status & TXQ_STOP) == TXQ_STOP_STA_PS)
        {
            fhost_tx_add_txq_to_sched(txq, &fhost_tx_env.sched[ac]);
            ac_to_sched |= CO_BIT(ac);
        }

        sp_size += txq->limit;
        if (req_cnt)
        {
            req_cnt -= txq->limit;
            if (!req_cnt)
                break;
        }
    }

    tfc->sp_cnt[ps_type] = sp_size;
    tfc->ps_ready[ps_type] -= sp_size;

    for (ac = AC_MAX; ac >= AC_BK; ac--)
    {
        if (ac_to_sched & CO_BIT(ac))
        {
            fhost_tx_schedule(ac);
        }
    }
    #endif
}

/**
 ****************************************************************************************
 * @brief Push Tx buffer to a TX queueSend Tx buffer and push RX buffer to the WiFi task
 *
 * @param[in] buf Pointer to the Rx buffer
 ****************************************************************************************
 */
static void fhost_tx_do_resend_req(struct fhost_rx_buf_tag* buf)
{
    struct fhost_vif_tag *fhost_vif = NULL;
    uint8_t vif_idx = (buf->info.flags & RX_FLAGS_VIF_INDEX_MSK) >> RX_FLAGS_VIF_INDEX_OFT;

    fhost_vif = fhost_env.mac2fhost_vif[vif_idx];
    #if RW_MESH_EN
    fhost_vif->is_resending = true;
    #endif
    fhost_tx_req(fhost_vif->net_if, buf->info_tx.net_buf, IEEE802_3, NULL, NULL, buf, 0);
    #if RW_MESH_EN
    fhost_vif->is_resending = false;
    #endif
}

/**
 ****************************************************************************************
 * @brief TX task implementation.
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_tx_task)
{
    fhost_task_ready(TX_TASK);

    for (;;)
    {
        struct fhost_tx_msg_tag msg;

        // Wait for a message on the queue
        rtos_queue_read(fhost_tx_env.queue_msg, &msg, -1, false);

        FHOST_TX_LOCK();
        switch (msg.msg_id)
        {
            case FHOST_TX_CREDITS_UPDATE:
                fhost_tx_do_credits_update(msg.u.credits_upd.sta_id,
                                           msg.u.credits_upd.tid,
                                           msg.u.credits_upd.credits);
                break;

            case FHOST_TX_STA_ADD:
                fhost_tx_do_sta_add(msg.u.sta.sta_id);
                break;

            case FHOST_TX_STA_DEL:
                fhost_tx_do_sta_del(msg.u.sta.sta_id);
                break;

            case FHOST_TX_VIF_ENABLE:
                fhost_tx_do_vif_enable(msg.u.vif.vif_idx);
                break;

            case FHOST_TX_VIF_DISABLE:
                fhost_tx_do_vif_disable(msg.u.vif.vif_idx);
                break;

            case FHOST_TX_STA_ENTER_PS:
                fhost_tx_do_sta_enter_ps(msg.u.sta.sta_id);
                break;

            case FHOST_TX_STA_EXIT_PS:
                fhost_tx_do_sta_exit_ps(msg.u.sta.sta_id);
                break;

            case FHOST_TX_PS_TRAFFIC_REQ:
                fhost_tx_do_ps_traffic_req(msg.u.ps_traffic.sta_id, msg.u.ps_traffic.req_cnt,
                                           msg.u.ps_traffic.ps_type);
                break;

            case FHOST_TX_RESEND_REQ:
                fhost_tx_do_resend_req(msg.u.resend.rx_buf);
                break;

            case FHOST_TX_VIF_ENTER_PS:
                fhost_tx_do_vif_enter_ps(msg.u.vif.vif_idx);
                break;

            case FHOST_TX_VIF_EXIT_PS:
                fhost_tx_do_vif_exit_ps(msg.u.vif.vif_idx);
                break;

            default:
                // Unknown message ID
                ASSERT_ERR(0);
                break;
        }
        FHOST_TX_UNLOCK();
    }
}

int fhost_tx_init(void)
{
    // Ensure the required headroom is enough to store fhost_tx_desc_tag
    if ((sizeof(struct fhost_tx_desc_tag) + 3) > NET_AL_TX_HEADROOM)
    {
        net_all_tx_headroom_too_small();
    }

    // Reset the environment
    memset(&fhost_tx_env, 0, sizeof(fhost_tx_env));

    // Create the message queue
    if (rtos_queue_create(sizeof(struct fhost_tx_msg_tag), FHOST_TX_QUEUE_MSG_ELT_CNT,
                          &fhost_tx_env.queue_msg))
    {
        return 1;
    }

    // Create the TX queue
    if (rtos_queue_create(sizeof(void *), NX_TXDESC_CNT, &fhost_tx_env.queue_tx))
    {
        return 2;
    }

    // Create the TX mutex
    if (rtos_mutex_create(&fhost_tx_env.tx_lock))
    {
        return 3;
    }

    // Create the TX task
    if (rtos_task_create(fhost_tx_task, "TX", TX_TASK, FHOST_TX_STACK_SIZE, NULL,
                         FHOST_TX_PRIORITY, NULL))
    {
        return 4;
    }

    // Initialize per HW-queue credits
    for (int i = 0; i < NX_TXQ_CNT; i++)
    {
        fhost_tx_env.hwq_credits[i] = get_nx_txdesc_cnt(i);
    }

    return 0;
}

int fhost_tx_start(net_if_t *net_if, net_buf_tx_t *net_buf,
                   cb_fhost_tx cfm_cb, void *cfm_cb_arg)
{
    FHOST_TX_LOCK();
    fhost_tx_req(net_if, net_buf, IEEE802_3, cfm_cb, cfm_cb_arg, NULL, 0);
    FHOST_TX_UNLOCK();

    return 0;
}

// FIXME Move to somewhere else
typedef void (*bl_custom_tx_callback_t)(void *cb_arg, bool tx_ok);

struct bl_custom_tx_cfm {
    bl_custom_tx_callback_t cb;
    void *cb_arg;
};

static void cfm_cb(uint32_t frame_id, bool acknowledged, void *arg)
{
    bl_custom_tx_callback_t cb = arg;
    if (cb) {
        cb(NULL, acknowledged);
    }
}

int bl_wifi_eth_tx(struct pbuf *p, bool is_sta, struct bl_custom_tx_cfm *custom_cfm)
{
    net_if_t *net_if;

    if (is_sta) {
        // FIXME
        net_if = fhost_env.vif[0].net_if;
    } else {
        net_if = fhost_env.vif[1].net_if;
    }

    pbuf_ref(p);

    return fhost_tx_start(net_if, p, cfm_cb, custom_cfm->cb);
}

void fhost_tx_cfm_push(uint8_t access_category, struct txdesc *txdesc)
{
    FHOST_TX_LOCK();
    fhost_tx_cfm(txdesc);
    FHOST_TX_UNLOCK();
}

void fhost_tx_credits_update(uint8_t sta_id, uint8_t tid, int8_t credits)
{
    struct fhost_tx_msg_tag msg;

    // Prepare the message
    msg.msg_id = FHOST_TX_CREDITS_UPDATE;
    msg.u.credits_upd.sta_id = sta_id;
    msg.u.credits_upd.tid = tid;
    msg.u.credits_upd.credits = credits;

    // Push the message to the queue
    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

void fhost_tx_sta_add(uint8_t sta_id, uint32_t buf_timeout)
{
    struct fhost_tx_msg_tag msg;

    if (sta_id >= STA_MAX)
        return;

    // Prepare the message
    msg.msg_id = FHOST_TX_STA_ADD;
    msg.u.sta.sta_id = sta_id;

    #if NX_BEACONING
    fhost_tx_env.traffic[sta_id].ps_max_queue_time = buf_timeout;
    #endif

    // Push the message to the queue
    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

void fhost_tx_sta_del(uint8_t sta_id)
{
    struct fhost_tx_msg_tag msg;

    if (sta_id >= STA_MAX)
        return;

    // Prepare the message
    msg.msg_id = FHOST_TX_STA_DEL;
    msg.u.sta.sta_id = sta_id;

    // Push the message to the queue
    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

void fhost_tx_vif_txq_enable(struct fhost_vif_tag *fhost_vif)
{
    struct fhost_tx_msg_tag msg;

    if (!fhost_vif || ! fhost_vif->mac_vif)
        return;

    // vif TXQ is used to send data to 'unknown' STA
    // AP: For all frames generated by wpa_supplicant (probe resp, auth ...)
    // STA: Needed for external authentication only
    // MONITOR: Needed by some external OS framework
    if ((mac_vif_get_type(fhost_vif->mac_vif) != VIF_MONITOR) &&
        (mac_vif_get_type(fhost_vif->mac_vif) != VIF_AP) &&
        (mac_vif_get_type(fhost_vif->mac_vif) != VIF_STA))
        return;

    msg.msg_id = FHOST_TX_VIF_ENABLE;
    msg.u.vif.vif_idx = mac_vif_get_index(fhost_vif->mac_vif);

    // Push the message to the queue
    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

void fhost_tx_vif_txq_disable(struct fhost_vif_tag *fhost_vif)
{
    struct fhost_tx_msg_tag msg;

    if (!fhost_vif || ! fhost_vif->mac_vif)
        return;

    if ((mac_vif_get_type(fhost_vif->mac_vif) != VIF_MONITOR) &&
        (mac_vif_get_type(fhost_vif->mac_vif) != VIF_AP) &&
        (mac_vif_get_type(fhost_vif->mac_vif) != VIF_STA))
        return;

    msg.msg_id = FHOST_TX_VIF_DISABLE;
    msg.u.vif.vif_idx = mac_vif_get_index(fhost_vif->mac_vif);

    // Push the message to the queue
    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

#if NX_BEACONING
int fhost_tx_sta_ps_enable(uint8_t sta_id, bool enable)
{
    struct fhost_tx_msg_tag msg;
    struct fhost_tx_queue_tag *txq;

    if (sta_id >= STA_MAX)
        return -1;

    txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, 0);
    if (!txq || !(txq->status & TXQ_ENABLED))
        return -1;

    if (enable)
        msg.msg_id = FHOST_TX_STA_ENTER_PS;
    else
        msg.msg_id = FHOST_TX_STA_EXIT_PS;
    msg.u.sta.sta_id = sta_id;

    return rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

int fhost_tx_sta_ps_traffic_req(uint8_t sta_id, int pkt_cnt, bool uapsd)
{
    struct fhost_tx_msg_tag msg;
    struct fhost_tx_queue_tag *txq;

    if (sta_id >= STA_MAX)
        return -1;

    txq = fhost_tx_get_txq(INVALID_VIF_IDX, sta_id, 0);
    if (!txq || !(txq->status & TXQ_ENABLED))
        return -1;

    msg.msg_id = FHOST_TX_PS_TRAFFIC_REQ;
    msg.u.ps_traffic.sta_id = sta_id;
    msg.u.ps_traffic.req_cnt = pkt_cnt;
    msg.u.ps_traffic.ps_type = uapsd ? PS_UAPSD : PS_LEGACY;

    return rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}
#endif // NX_BEACONING

int fhost_tx_vif_ps_enable(uint8_t vif_idx, uint8_t enable)
{
    struct fhost_tx_msg_tag msg;

    if (enable)
        msg.msg_id = FHOST_TX_VIF_ENTER_PS;
    else
        msg.msg_id = FHOST_TX_VIF_EXIT_PS;
    msg.u.vif.vif_idx = vif_idx;

    return rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

uint32_t fhost_send_80211_frame(int fvif_idx, const uint8_t *frame, uint32_t length,
                                int no_cck, cb_fhost_tx cfm_cb, void *cfm_cb_arg)
{
    struct fhost_vif_tag *fhost_vif;
    net_buf_tx_t *net_buf;
    int ret = 0;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return 0;

    fhost_vif = &fhost_env.vif[fvif_idx];
    if (!fhost_vif->mac_vif)
        return 1;

    net_buf = net_buf_tx_alloc(length);
    if (net_buf == NULL)
        return 2;

    memcpy(net_buf->payload, frame, length);

    FHOST_TX_LOCK();
    ret = fhost_tx_req(fhost_vif->net_if, net_buf, IEEE802_11, cfm_cb, cfm_cb_arg, NULL, no_cck);
    FHOST_TX_UNLOCK();
    if (ret) {
        printf("[FH]%s fhost_tx_req fail (ret:%d)!\r\n", __func__, ret);
        ret = 3;
    } else {
        ret = (uint32_t)net_buf;
    }

    return ret;
}

/// @}


