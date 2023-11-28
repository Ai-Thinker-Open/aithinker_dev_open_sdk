/**
 ****************************************************************************************
 *
 * @file fhost_rx.c
 *
 * @brief Implementation of the fully hosted RX task.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST_RX
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rtos.h"
#include "fhost.h"
#include "fhost_rx.h"
#include "fhost_rx_def.h"
#include "mac_frame.h"
#include "co_endian.h"
#include "cfgrwnx.h"
#include "fhost_tx.h"
#if NX_FHOST_RX_STATS
#include "fhost_config.h"
#endif
#include "fhost_cntrl.h"
#if RW_MESH_EN
#include "mesh.h"
#endif

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
struct fhost_rx_env_tag fhost_rx_env;

/// Pool of RX buffers
static struct fhost_rx_buf_tag fhost_rx_buf_mem[FHOST_RX_BUF_CNT] __SHAREDRAM;
#if NX_UF_EN
/// Pool of UF buffers
static struct fhost_rx_uf_buf_tag fhost_rx_uf_buf_mem[FHOST_RX_BUF_CNT] __SHAREDRAM;
#endif // NX_UF_EN

#if NX_FHOST_RX_STATS
struct fhost_rx_stats_sta *rx_stats[NX_REMOTE_STA_MAX];
#endif // NX_FHOST_RX_STATS
/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Send FHOST_TX_RESEND_REQ message to Tx task
 *
 * @param[in] buf Pointer to Rx buffer structure
 ****************************************************************************************
 */
static void fhost_rx_do_resend_req(struct fhost_rx_buf_tag* buf)
{
    struct fhost_tx_msg_tag msg;

    // Prepare the message
    msg.msg_id = FHOST_TX_RESEND_REQ;
    msg.u.resend.rx_buf = buf;

    rtos_queue_write(fhost_tx_env.queue_msg, &msg, -1, false);
}

#if NX_UF_EN
/**
 ****************************************************************************************
 * @brief Push an unsupported frame buffer to the WiFi task.
 *
 * @param[in] net_buf   Pointer to the RX buffer to push
 ****************************************************************************************
 */
static void fhost_rx_uf_buf_push(void *net_buf)
{
    struct fhost_rx_uf_buf_tag *buf = net_buf;
    struct fhost_rx_uf_buf_desc_tag buf_desc;
    int res;

    // Push back the RX buffer to the MAC
    buf->info.pattern = 0;
    buf_desc.host_id = (uint32_t)buf;

    res = rtos_queue_write(fhost_rx_env.queue_uf_buf, &buf_desc, 0, false);
    ASSERT_ERR(res == 0);
}
#endif // NX_UF_EN

/**
 ****************************************************************************************
 * @brief Resend a RX buffer to Tx path.
 *
 * @param[in] rx_buf_action  Pointer to the RX action status
 * @param[in] buf            Pointer to the RX buffer to forward
 * @param[in] length         Length of Rx buffer to resend
 * @param[in] offset         Offset on the payload to send
 * @param[in] mac_hdr_len    Length of the MAC header
 ****************************************************************************************
 */
static void fhost_rx_resend(uint8_t rx_buf_action,
                            struct fhost_rx_buf_tag *buf,
                            int length,
                            int offset,
                            int mac_hdr_len)
{
    net_buf_tx_t *net_buf_tx_1, *net_buf_tx_2;

    // Allocation of the 2 buffer Tx: Ethernet header + payload @ref Rx payload
    net_buf_tx_1 = net_buf_tx_alloc(mac_hdr_len);
    net_buf_tx_2 = net_buf_tx_alloc_ref(length - mac_hdr_len);

    if ((net_buf_tx_1 == NULL) || (net_buf_tx_2 == NULL))
    {
        if (!(rx_buf_action & RX_BUF_FORWARD))
        {
            fhost_rx_buf_push(buf);
        }
    }
    else
    {
        if (rx_buf_action & RX_BUF_FORWARD)
            buf->ref++;

        // Update the ref of the payload
        net_buf_tx_2->payload = buf->payload;
        net_buf_tx_2->payload += (mac_hdr_len + offset);

        // Copy the ethernet header from Rx
        memcpy(net_buf_tx_1->payload, (uint8_t*)(buf->payload) + offset, mac_hdr_len);

        //concatenate the 2 buffers Tx
        net_buf_tx_cat(net_buf_tx_1, net_buf_tx_2);

        buf->info_tx.net_buf = net_buf_tx_1;

        fhost_rx_do_resend_req(buf);
    }
}

/**
 ****************************************************************************************
 * @brief Forward/Resend a RX buffer to the networking stack/Tx path.
 *
 * @param[in] buf                Pointer to the RX buffer to forward
 * @param[in] rx_buf_action      RX send action status
 * @param[in] net_if             Network interface to which the buffer is intended
 * @param[in] length             Length of Rx buffer to send
 * @param[in] offset             Offset to point on RX buffer
 * @param[in] skip_after_eth_hdr Offset to that should be skipped for the forward
 ****************************************************************************************
 */
static void fhost_rx_buf_send(struct fhost_rx_buf_tag *buf,
                              uint8_t rx_buf_action,
                              net_if_t *net_if,
                              uint16_t length,
                              uint8_t offset,
                              uint8_t skip_after_eth_hdr)
{
    if (rx_buf_action & RX_BUF_RESEND)
    {
        // Resend to another sta connected to the same AP
        fhost_rx_resend(rx_buf_action, buf, length, offset, sizeof(struct mac_eth_hdr) + skip_after_eth_hdr);
    }

    if (rx_buf_action & RX_BUF_FORWARD)
    {
        if (skip_after_eth_hdr != 0)
            memcpy((char*)buf->payload + skip_after_eth_hdr, buf->payload, sizeof(struct mac_eth_hdr));
        // Forward to the networking stack
        net_if_input(&buf->net_buf, net_if, (uint8_t*)(buf->payload) + offset + skip_after_eth_hdr,
                     length - skip_after_eth_hdr, fhost_rx_buf_push);
    }
}

/**
 ****************************************************************************************
 * @brief Forward a RX buffer containing a A-MSDU to the networking stack.
 *
 * @param[in] buf                Pointer to the RX buffer to forward
 * @param[in] net_if             Network interface to which the buffer is intended
 * @param[in] buf_rx_action      RX send action status
 * @param[in] skip_after_eth_hdr Offset to that should be skipped for the forward
 ****************************************************************************************
 */
static void fhost_rx_amsdu_forward(struct fhost_rx_buf_tag *buf, net_if_t *net_if,
                                   uint8_t buf_rx_action, uint8_t skip_after_eth_hdr)
{
    struct rx_info *info =&buf->info;
    uint8_t *payload;
    uint16_t len;
    int subframe_idx = 0;
    struct llc_snap *llc_snap;
    uint8_t offset;

    do
    {
        offset = 0;

        // Get payload pointer
        payload = (uint8_t *)buf->payload;

        // Get the subframe length
        len = co_ntohs(co_read16(&payload[LLC_ETHERTYPE_LEN_OFT])) + LLC_ETHER_HDR_LEN;

        // Map LLC/SNAP structure on buffer
        llc_snap = (struct llc_snap *)&payload[LLC_ETHER_HDR_LEN];
        if ((!memcmp(llc_snap, &llc_rfc1042_hdr, sizeof(llc_rfc1042_hdr))
             //&& (llc_snap->proto_id != LLC_ETHERTYPE_AARP) - Appletalk depracated ?
             && (llc_snap->proto_id != LLC_ETHERTYPE_IPX))
            || (!memcmp(llc_snap, &llc_bridge_tunnel_hdr, sizeof(llc_bridge_tunnel_hdr))))
        {
            // Packet becomes
            /********************************************
             *  DA  |  SA  |  SNAP->ETHERTYPE  |  DATA  |
             ********************************************/
            // We remove the LLC/SNAP, so adjust the length and the offset
            len -= LLC_802_2_HDR_LEN;
            offset = LLC_802_2_HDR_LEN;

            // Move the source/dest addresses at the right place
            MAC_ADDR_CPY(&payload[offset + MAC_ADDR_LEN], &payload[MAC_ADDR_LEN]);
            MAC_ADDR_CPY(&payload[offset], &payload[0]);

            // No need to copy the Ethertype which is already in place
        }

        // Forward/Resend
        fhost_rx_buf_send(buf, buf_rx_action, net_if, len, offset, skip_after_eth_hdr);

        // Check if we may still have some subframes
        if (subframe_idx == (NX_MAX_MSDU_PER_RX_AMSDU - 1))
            break;

        // Get next subframe
        buf = (struct fhost_rx_buf_tag *)info->amsdu_hostids[subframe_idx++];

        // Copy rx_info into subframe (fix MIOT616-233)
        if (buf) {
            memcpy(&buf->info, info, sizeof(*info));
        }

    } while (buf != NULL);
}

/**
 ****************************************************************************************
 * @brief Forward a MGMT frame to the registered callback.
 *
 * @param[in] buf        Pointer to the RX buffer to forward
 * @param[in] fhost_vif  Interface that received the frame (can be NULL)
 ****************************************************************************************
 */
static void fhost_rx_mgmt_buf_forward(struct fhost_rx_buf_tag *buf,
                                      struct fhost_vif_tag *fhost_vif)
{
    struct fhost_frame_info info;

    if (fhost_rx_env.mgmt_cb == NULL)
        return;

    info.payload = (uint8_t *)buf->payload;
    info.length = buf->info.vect.frmlen;
    info.freq = PHY_INFO_CHAN(buf->info.phy_info);
    info.rssi = export_hal_desc_get_rssi((void *)&buf->info.vect.rx_vec_1, NULL);

    if (fhost_vif)
    {
        info.fvif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
        fhost_rx_env.mgmt_cb(&info, fhost_rx_env.mgmt_cb_arg);
    }
    else
    {
        for (int i = 0; i < NX_VIRT_DEV_MAX; i++)
        {
            if (fhost_env.vif[i].mac_vif)
            {
                info.fvif_idx = i;
                fhost_rx_env.mgmt_cb(&info, fhost_rx_env.mgmt_cb_arg);
            }
        }
    }
}

/**
 ****************************************************************************************
 * @brief Select which action to send RX buffer should be activated: Resend/Forward.
 *
 * @param[in] fhost_vif          virtual interfaces information
 * @param[in] mac_addr           MAC address of the destination buffer
 * @param[in] rx_info_flags      SW flags about the RX packet
 * @param[in] vif_idx            Interface the station belongs to
 * @param[in] skip_after_eth_hdr Offset to that should be skipped for the forward
 *
 * @return the Bitfield indicating the RX action to perform
 ****************************************************************************************
 */
static uint8_t fhost_rx_select_action(struct fhost_vif_tag *fhost_vif,
                                      struct fhost_rx_buf_tag *buf,
                                      uint32_t rx_info_flags,
                                      uint8_t vif_idx,
                                      uint8_t *skip_after_eth_hdr)
{
    uint8_t rx_buf_action = RX_BUF_FORWARD;

    if (!fhost_vif->isolation_mode && (mac_vif_get_type(fhost_vif->mac_vif) == VIF_AP))
    {
        struct mac_eth_hdr *hdr = (struct mac_eth_hdr *)buf->payload;

        if (MAC_ADDR_GROUP(hdr))
        {
            rx_buf_action |= RX_BUF_RESEND;
        }
        else
        {
            uint8_t dest_sta_idx = (rx_info_flags & RX_FLAGS_DST_INDEX_MSK) >> RX_FLAGS_DST_INDEX_OFT;
            if (dest_sta_idx != INVALID_STA_IDX)
            {
                if (sta_idx_get_valid(dest_sta_idx) && (sta_idx_get_inst_nbr(dest_sta_idx) == vif_idx))
                {
                    // Reset forward and set to resend
                    rx_buf_action = RX_BUF_RESEND;
                }
            }
        }
    }
    #if RW_MESH_EN
    else if (mac_vif_get_type(fhost_vif->mac_vif) == VIF_MESH_POINT)
    {
        struct mac_eth_hdr *hdr = (struct mac_eth_hdr *)buf->payload;
        struct rx_info *info = &buf->info;
        uint8_t dest_sta_idx = (info->flags & RX_FLAGS_DST_INDEX_MSK) >> RX_FLAGS_DST_INDEX_OFT;

        rx_buf_action = RX_BUF_FORWARD;

        if (dest_sta_idx != INVALID_STA_IDX)
        {
            // TO DO: LLC-SNAP is already removed in AMSDU.
            ASSERT_ERR(!(info->flags & RX_FLAGS_IS_AMSDU_BIT));

            // MC/BC frames are uploaded with mesh control (6 bytes) and LLC/snap (8 bytes)
            // (so they can be mesh forwarded) that need to be removed.
            uint8_t *mesh_ctrl = (uint8_t *)(hdr + 1);
            *skip_after_eth_hdr = 8 + 6;
            if ((*mesh_ctrl & MESH_CTRL_FLAGS_AE_MODE_MASK) == MESH_CTRL_MESH_AE_ADDR4)
                *skip_after_eth_hdr += sizeof(struct mac_addr);
            else if ((*mesh_ctrl & MESH_CTRL_FLAGS_AE_MODE_MASK) == MESH_CTRL_MESH_AE_ADDR5_6)
                *skip_after_eth_hdr += 2 * sizeof(struct mac_addr);

            if (MAC_ADDR_GROUP(hdr))
            {
                rx_buf_action |= RX_BUF_RESEND;
            }
            else
            {
                rx_buf_action = RX_BUF_RESEND;
            }
        }
    }
    #endif

    return(rx_buf_action);
}

#if NX_FHOST_RX_STATS && 0
/**
 ****************************************************************************************
 * @brief Update RX statistics with the received payload
 *
 * @param[in] info      Information about the received payload
 ****************************************************************************************
 */
static void fhost_rx_save_stats(struct rx_info *info)
{
    uint8_t staid = (info->flags & RX_FLAGS_STA_INDEX_MSK) << RX_FLAGS_STA_INDEX_OFT;
    int rate_idx, mcs = 0, sgi = 0, nss = 0;
    struct rx_vector_1 *rx_vect = &info->vect.rx_vec_1;

    // STA idx invalid, do not save statistics
    if (!rx_stats[staid])
        return;

    switch (rx_vect->format_mod) {
        case FORMATMOD_NON_HT:
        case FORMATMOD_NON_HT_DUP_OFDM:
        {
            int idx = rxv2macrate[rx_vect->leg_rate];
            if (idx < 4) {
                rate_idx = idx * 2 + rx_vect->pre_type;
            } else {
                rate_idx = N_CCK + idx - 4;
            }
            break;
        }
        case FORMATMOD_HT_MF:
        case FORMATMOD_HT_GF:
            #if NX_MAC_VER >= 20
            mcs = rx_vect->ht.mcs % 8;
            nss = rx_vect->ht.mcs / 8;
            sgi = rx_vect->ht.short_gi;
            #else
            mcs = rx_vect->mcs % 8;
            nss = rx_vect->mcs / 8;
            sgi = rx_vect->short_gi;
            #endif
            rate_idx = fhost_conf_rx.first_ht + nss * (8 * fhost_conf_rx.ht_rates_per_mcs) +
                       mcs * fhost_conf_rx.ht_rates_per_mcs + rx_vect->ch_bw * 2 + sgi;
            break;
        case FORMATMOD_VHT:
            #if NX_MAC_VER < 20
            default:
            mcs = rx_vect->mcs;
            nss = rx_vect->stbc ? rx_vect->n_sts/2 : rx_vect->n_sts;
            sgi = rx_vect->short_gi;
            #else
            mcs = rx_vect->vht.mcs;
            nss = rx_vect->vht.nss;
            sgi = rx_vect->vht.short_gi;
            #endif
            rate_idx = fhost_conf_rx.first_vht + nss * (fhost_conf_rx.max_vht_mcs * fhost_conf_rx.vht_rates_per_mcs) +
                       mcs * fhost_conf_rx.vht_rates_per_mcs + rx_vect->ch_bw * 2 + sgi;
            break;
        #if NX_MAC_VER >= 20
        case FORMATMOD_HE_SU:
            mcs = rx_vect->he.mcs;
            nss = rx_vect->he.nss;
            sgi = rx_vect->he.gi_type;
            rate_idx = fhost_conf_rx.first_he_su + nss * (fhost_conf_rx.max_he_mcs * fhost_conf_rx.he_su_rates_per_mcs) +
                       mcs * fhost_conf_rx.he_su_rates_per_mcs + rx_vect->ch_bw * 3 + sgi;
            break;
        case FORMATMOD_HE_MU:
            mcs = rx_vect->he.mcs;
            nss = rx_vect->he.nss;
            sgi = rx_vect->he.gi_type;
            rate_idx = fhost_conf_rx.first_he_mu + nss * fhost_conf_rx.max_he_mcs * fhost_conf_rx.he_mu_rates_per_mcs +
                       mcs * fhost_conf_rx.he_mu_rates_per_mcs + rx_vect->he.ru_size * 3 + sgi;
            break;
        default:
            mcs = rx_vect->he.mcs;
            nss = rx_vect->he.nss;
            sgi = rx_vect->he.gi_type;
            rate_idx = fhost_conf_rx.first_he_er + rx_vect->ch_bw * 9 +
                       mcs * fhost_conf_rx.he_er_rates_per_mcs + sgi;
        #endif
    }

    rx_stats[staid]->table[rate_idx]++;
    rx_stats[staid]->cpt++;
    rx_stats[staid]->last_rx = info->vect.rx_vec_1;
}
#endif // NX_FHOST_RX_STATS

/**
 ****************************************************************************************
 * @brief Forward a RX buffer to the networking stack.
 *
 * @param[in] buf Pointer to the RX buffer to forward
 ****************************************************************************************
 */
static void fhost_rx_buf_forward(struct fhost_rx_buf_tag *buf)
{
    struct rx_info *info = &buf->info;
    struct fhost_vif_tag *fhost_vif = NULL;
    uint8_t vif_idx = (info->flags & RX_FLAGS_VIF_INDEX_MSK) >> RX_FLAGS_VIF_INDEX_OFT;
    uint8_t skip_after_eth_hdr = 0;

    if (vif_idx < NX_VIRT_DEV_MAX)
        fhost_vif = fhost_env.mac2fhost_vif[vif_idx];

    if(info->flags & RX_FLAGS_NEW_MESH_PEER_BIT)
    {
        fhost_cntrl_new_peer_candidate(vif_idx, buf);
        fhost_rx_buf_push(buf);
    }
    else if (info->flags & RX_FLAGS_NON_MSDU_MSK)
    {
        // Mgmt frame may be received with invalid vif idx
        fhost_rx_mgmt_buf_forward(buf, fhost_vif);
        fhost_rx_buf_push(buf);
    }
    else if (fhost_vif)
    {
        uint8_t rx_buf_action;
        rx_buf_action = fhost_rx_select_action(fhost_vif, buf, info->flags,
                                               mac_vif_get_index(fhost_vif->mac_vif), &skip_after_eth_hdr);

        #if NX_FHOST_RX_STATS
        fhost_rx_save_stats(info);
        #endif

        if (info->flags & RX_FLAGS_IS_AMSDU_BIT)
        {
            fhost_rx_amsdu_forward(buf, fhost_vif->net_if, rx_buf_action, skip_after_eth_hdr);
        }
        else
        {
            fhost_rx_buf_send(buf, rx_buf_action, fhost_vif->net_if, info->vect.frmlen, 0, skip_after_eth_hdr);
        }
    }
    else
    {
        ASSERT_ERR(0);
        fhost_rx_buf_push(buf);
    }
}

/**
 ****************************************************************************************
 * @brief Update the length of a reassembled RX buffer.
 *
 * @param[in] buf Pointer to the reassembled RX buffer
 * @param[in] status Pointer to the status descriptor
 ****************************************************************************************
 */
static void fhost_rx_buf_len_update(struct fhost_rx_buf_tag *buf, struct rxu_stat_val *status)
{
    struct rx_info *info =&buf->info;
    info->vect.frmlen = status->frame_len;

    if (status->status & RX_STAT_ETH_LEN_UPDATE)
    {
        // Update Length Field inside the Ethernet Header
        struct mac_eth_hdr *hdr = (struct mac_eth_hdr *)buf->payload;

        hdr->len = co_htons(status->frame_len - sizeof(struct mac_eth_hdr));
    }
}

/**
 ****************************************************************************************
 * @brief Call registered monitor callback for the received buffer.
 *
 * Extract useful information from RX buffer and call the monitor callback with this as
 * parameter. Returns immediately if no monitor callback is registered.
 *
 * @param[in] buf  Pointer to the RX buffer
 * @param[in] uf   Whether frame has been decoded or not by the modem.
 *                 (false: decoded frame, true: unsupported frame)
 ****************************************************************************************
 */
static void fhost_rx_monitor(struct fhost_rx_buf_tag *buf, bool uf)
{
    struct fhost_frame_info info;
    uint8_t vif_idx;

    if (fhost_rx_env.monitor_cb == NULL)
        return;

    if (uf)
    {
        info.payload = NULL;
        info.length = export_hal_desc_get_ht_length((void *)&buf->info.vect.rx_vec_1);
    }
    else
    {
        info.payload = (uint8_t *)buf->payload;
        info.length = buf->info.vect.frmlen;
    }

    info.freq = PHY_INFO_CHAN(buf->info.phy_info);
    info.rssi = export_hal_desc_get_rssi((void *)&buf->info.vect.rx_vec_1, NULL);

    vif_idx = (buf->info.flags & RX_FLAGS_VIF_INDEX_MSK) >> RX_FLAGS_VIF_INDEX_OFT;
    if (vif_idx < NX_VIRT_DEV_MAX)
    {
        struct fhost_vif_tag *fhost_vif = fhost_env.mac2fhost_vif[vif_idx];
        info.fvif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    }
    else
        info.fvif_idx = -1;

    fhost_rx_env.monitor_cb(&info, fhost_rx_env.monitor_cb_arg);
}

/**
 ****************************************************************************************
 * @brief RX task implementation.
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_rx_task)
{
    // Push the RX buffers
    for (int i=0; i<FHOST_RX_BUF_CNT; i++)
    {
        struct fhost_rx_buf_tag *buf = &fhost_rx_buf_mem[i];
        #if NX_UF_EN
        struct fhost_rx_uf_buf_tag *uf_buf = &fhost_rx_uf_buf_mem[i];
        // Indicate the UF buffer to the MAC
        fhost_rx_uf_buf_push(uf_buf);
        #endif // NX_UF_EN

        // Initialize all buffer Rx to 1 in order to be pushed
        buf->ref = 1;

        // Indicate the RX buffer to the MAC
        fhost_rx_buf_push(buf);
    }
    fhost_task_ready(RX_TASK);

    for (;;)
    {
        struct fhost_rx_msg_tag msg;
        struct fhost_rx_buf_tag *buf;

        // Wait for a message on the RX queue
        rtos_queue_read(fhost_rx_env.queue_desc, &msg, -1, false);

        // Get the pointer to the RX buffer associated to the event
        buf = (struct fhost_rx_buf_tag *)msg.desc.host_id;

        // Handle the received message
        if (msg.desc.status & RX_STAT_LEN_UPDATE)
        {
            // Update the length
            fhost_rx_buf_len_update(buf, &msg.desc);
        }
        if (msg.desc.status & RX_STAT_FORWARD)
        {
            // Forward the buffer to the networking stack
            fhost_rx_buf_forward(buf);
        }
        else if (msg.desc.status & RX_STAT_ALLOC)
        {
            // printf("Not handler in fhost_rx");
        }
        #if NX_UF_EN
        if (msg.desc.status & RX_STAT_UF)
        {
            struct fhost_rx_uf_buf_tag *uf_buf;
            uf_buf = (struct fhost_rx_uf_buf_tag *)msg.desc.host_id;

            fhost_rx_monitor((struct fhost_rx_buf_tag *)uf_buf, true);
            fhost_rx_uf_buf_push(uf_buf);
        }
        #endif // NX_UF_EN
        if (msg.desc.status & RX_STAT_MONITOR)
        {
            fhost_rx_monitor(buf, false);
            fhost_rx_buf_push(buf);
        }
        else if (msg.desc.status & RX_STAT_DELETE)
        {
            // Push back the buffer to the MAC
            fhost_rx_buf_push(buf);
        }

        if (msg.desc.status & RX_STAT_SPURIOUS)
        {
            // Push back the buffer to the MAC
            fhost_rx_buf_push(buf);
        }
    }
}

void fhost_rx_buf_push(void *net_buf)
{
    struct fhost_rx_buf_tag *buf = net_buf;
    struct fhost_rx_buf_desc_tag buf_desc;
    int res __MAYBE_UNUSED;

    ASSERT_ERR(buf->ref != 0);
    buf->ref--;

    if (buf->ref == 0)
    {
        // Push back the RX buffer to the MAC
        buf->info.pattern = 0;
        buf_desc.host_id = (uint32_t)buf;
        buf_desc.addr = (uint32_t)&buf->info;
        // Initialise the number of reference
        buf->ref = 1;

        res = rtos_queue_write(fhost_rx_env.queue_buf, &buf_desc, 0, false);
        ASSERT_ERR(res == 0);

        // Warn the WiFi task about the new buffer availability, if required
        macif_rx_buf_ind();
    }
}

int fhost_rx_init(void)
{
    // Initialize monitor callback
    fhost_rx_env.monitor_cb     = NULL;
    fhost_rx_env.monitor_cb_arg = NULL;

    // Create the RX queue
    if (rtos_queue_create(sizeof(struct fhost_rx_msg_tag), FHOST_RX_QUEUE_DESC_ELT_CNT,
                          &fhost_rx_env.queue_desc))
    {
        return 1;
    }

    // Create the RX buffer queue
    if (rtos_queue_create(sizeof(struct fhost_rx_buf_desc_tag), FHOST_RX_BUF_CNT, &fhost_rx_env.queue_buf))
    {
        return 2;
    }

    #if NX_UF_EN
    // Create the UF buffer queue
    if (rtos_queue_create(sizeof(struct fhost_rx_uf_buf_desc_tag), FHOST_RX_BUF_CNT, &fhost_rx_env.queue_uf_buf))
    {
        return 3;
    }
    #endif // NX_UF_EN

    // Create the RX task
    if (rtos_task_create(fhost_rx_task, "RX", RX_TASK, FHOST_RX_STACK_SIZE, NULL,
                         FHOST_RX_PRIORITY, NULL))
    {
        return 4;
    }

    return 0;
}

void fhost_rx_set_mgmt_cb(void *arg, cb_fhost_rx cb)
{
    fhost_rx_env.mgmt_cb = cb;
    fhost_rx_env.mgmt_cb_arg = arg;
}

void fhost_rx_set_monitor_cb(void *arg, cb_fhost_rx cb)
{
    fhost_rx_env.monitor_cb = cb;
    fhost_rx_env.monitor_cb_arg = arg;
}

/// @}
