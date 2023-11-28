/**
 ****************************************************************************************
 *
 * @file fhost_tx.h
 *
 * @brief Definitions of the fully hosted TX task.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#ifndef _FHOST_TX_H_
#define _FHOST_TX_H_

/**
 ****************************************************************************************
 * @defgroup FHOST_TX FHOST_TX
 * @ingroup FHOST
 * @brief Fully Hosted TX task implementation.
 * This module creates a task that interfaces between the networking stack and the WiFi
 * task. It is responsible for the assignment of the packets to the TX queues
 * and for the scheduling of these queues.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rtos.h"
#include "net_al.h"
#include "co_list.h"
#include "fhost.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */
/// Number of elements in the TX message queue
#define FHOST_TX_QUEUE_MSG_ELT_CNT     64

/// IDs of messages received by the FHOST TX task
enum fhost_tx_msg_id
{
    /// Indicate that a STA is connected
    FHOST_TX_STA_ADD = 0,
    /// Indicate that a STA is disconnected
    FHOST_TX_STA_DEL,
    /// Indicate that the credits for a TX queue are updated
    FHOST_TX_CREDITS_UPDATE,
    /// Enable TXQ associated to a VIF
    FHOST_TX_VIF_ENABLE,
    /// Disable TXQ associated to a VIF
    FHOST_TX_VIF_DISABLE,
    /// Indicate that a STA enabled PowerSave mode
    FHOST_TX_STA_ENTER_PS,
    /// Indicate that a STA disabled PowerSave mode
    FHOST_TX_STA_EXIT_PS,
    /// Request traffic for STA in PS mode
    FHOST_TX_PS_TRAFFIC_REQ,
    /// Request resend packet
    FHOST_TX_RESEND_REQ,
    /// Indicate that the VIF enters in PS mode and all TXQs linked to the VIF are stopped
    FHOST_TX_VIF_ENTER_PS,
    /// Indicate that the VIF exits PS mode and all TXQs linked to the VIF are started
    /// and triggered if necessary
    FHOST_TX_VIF_EXIT_PS,
};

/// TXQ status flag
enum fhost_txq_flags {
    /// The queue is scheduled for transmission
    TXQ_IN_HWQ_LIST  = CO_BIT(0),
    /// No more credits for the queue
    TXQ_STOP_FULL    = CO_BIT(1),
    /// CSA is in progress
    TXQ_STOP_CSA     = CO_BIT(2),
    /// Destination sta is currently in power save mode
    TXQ_STOP_STA_PS  = CO_BIT(3),
    /// VIF owning this queue is currently in power save mode
    TXQ_STOP_VIF_PS  = CO_BIT(4),
    /// Channel of this queue is not the current active channel
    TXQ_STOP_CHAN    = CO_BIT(5),
    /// All possible reasons to have a txq stopped
    TXQ_STOP         = (TXQ_STOP_FULL | TXQ_STOP_CSA | TXQ_STOP_STA_PS | TXQ_STOP_VIF_PS |
                        TXQ_STOP_CHAN),
    /// TX queue is enabled, i.e. meaning that it can be used for transmission
    TXQ_ENABLED      = CO_BIT(7),
};

/// Type of PS traffic
enum fhost_tx_ps_type
{
    /// Legacy power save
    PS_LEGACY,
    /// UAPSD
    PS_UAPSD,
    /// Number of PS traffic
    PS_TYPE_CNT,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/// Structure used for the inter-task communication
struct fhost_tx_msg_tag
{
    /// Message ID (from @ref fhost_tx_msg_id)
    enum fhost_tx_msg_id msg_id;
    /// Message structure
    union
    {
        /// Command for a station
        struct
        {
            /// Id of the station
            uint8_t sta_id;
        } sta;
        /// Update credits
        struct
        {
            /// Id of the station for which credits are updated
            uint8_t sta_id;
            /// TID for which credits are updated
            uint8_t tid;
            /// Credits
            int8_t credits;
        } credits_upd;
        /// Enable/Disable vif
        struct
        {
            /// Id of the MAC Vif
            uint8_t vif_idx;
        } vif;
        /// traffic request
        struct
        {
            /// Id of the station
            uint8_t sta_id;
            /// Number of packet to push (0 means all)
            uint8_t req_cnt;
            /// Type of traffic requested (@ref fhost_tx_ps_type)
            uint8_t ps_type;
        } ps_traffic;
        /// resend request
        struct
        {
            /// Pointer to the RX buffer
            struct fhost_rx_buf_tag* rx_buf;
        } resend;
    } u;
};

/// Structure containing FHOST control information for the present buffer
struct fhost_tx_ctrl_tag
{
    /// Pointer to the network stack buffer structure
    void *buf;
    /// TX confirmation callback (Only used for mgmt frame)
    void *cfm_cb;
    /// TX confirmation callback argument
    void *cfm_cb_arg;
    /// RX environment structure that is resent
    struct fhost_rx_buf_tag *buf_rx;
    /// Buffer timeout
    uint32_t timeout;
};

/*
 * -------------------------------- WARNNING start of duplicate definition --------------------------------
 * These should be kept identical between fhost & macsw.
 */
/// Structure indicating the status and other information about the transmission
struct tx_cfm_tag
{
    #if NX_UMAC_PRESENT
    /// Number of flow control credits allocated
    int8_t credits;
    /// Size (in mdpu) of the A-MPDU in which this frame has been transmitted.
    /// Valid only for singleton (=1) or last frame of the A-MPDU
    uint8_t ampdu_size;
    #if NX_AMSDU_TX
    /// Size allowed for AMSDU
    uint16_t amsdu_size;
    #endif
    #endif // NX_UMAC_PRESENT

    /// TX status
    uint32_t status;

    #if !NX_FULLY_HOSTED
    /// Host identifier to retrieve TX buffer (set to txdesc.host.hostid)
    /// Keep it last
    uint32_t hostid;
    #endif
};

#if NX_FULLY_HOSTED
/// Pointer to the network stack buffer structure
typedef void * hostid_t;
#else
/// Unique host identifier sent back in TX confirmation when txdesc has been processed
typedef uint32_t hostid_t;
#endif

/// Descriptor filled by the Host
struct hostdesc
{
    /// Unique host identifier sent back in TX confirmation when txdesc has been processed
    hostid_t hostid;
    /// Pointers to packet payloads
    uint32_t packet_addr[NX_TX_PAYLOAD_MAX];
    /// Sizes of the MPDU/MSDU payloads
    uint16_t packet_len[NX_TX_PAYLOAD_MAX];
    #if NX_AMSDU_TX
    /// Number of payloads forming the MPDU
    uint8_t packet_cnt;
    #endif //(NX_AMSDU_TX)

    #if NX_UMAC_PRESENT
    /// Destination Address
    struct mac_addr eth_dest_addr;
    /// Source Address
    struct mac_addr eth_src_addr;
    /// Ethernet Type
    uint16_t ethertype;
    /// TX flags
    uint16_t flags;
    /// SN to use for the transmission (only valid if flag TXU_CNTRL_REUSE_SN is set)
    uint16_t sn_for_retry;
    #else
    /// Padding between the buffer control structure and the MPDU in host memory
    uint8_t padding;
    #endif
    /// Packet TID
    uint8_t tid;
    /// VIF index
    uint8_t vif_idx;
    /// Station Id (0xFF if station is unknown)
    uint8_t staid;
    #if RW_MUMIMO_TX_EN
    /// MU-MIMO information (GroupId and User Position in the group) - The GroupId
    /// is located on bits 0-5 and the User Position on bits 6-7. The GroupId value is set
    /// to 63 if MU-MIMO shall not be used
    uint8_t mumimo_info;
    #endif
    #if NX_FULLY_HOSTED
    /// TX confirmation info
    struct tx_cfm_tag cfm;
    #endif
};

/// Description of the TX API
struct txdesc_api
{
    /// Information provided by Host
    struct hostdesc host;
    #if (!NX_UMAC_PRESENT)
    /// Information provided by UMAC
    struct umacdesc umac;
    #endif
};

/// Additional temporary control information passed from the host to the emb regarding
/// the TX descriptor
struct txdesc_ctrl
{
    /// HW queue in which the TX descriptor shall be pushed
    uint32_t hwq;
};

/// Descriptor used for Host/Emb TX frame information exchange
struct txdesc_host
{
    /// API of the embedded part
    struct txdesc_api api;

    /// Additional control information about the descriptor
    struct txdesc_ctrl ctrl;

    /// Flag indicating if the TX descriptor is ready (different from 0) or not (equal to 0)
    /// Shall be the last element of the structure, i.e. downloaded at the end
    uint32_t ready;
};

/// LMAC Tx Descriptor
struct txdesc
{
    /// List element. A txdesc can be in three list:
    /// - the available list (except txdesc of internal frames and in Fully Hosted)
    /// - the transmitting list
    /// - the confirmation list
    struct co_list_hdr list_hdr;
    /// Information provided by Host
    struct hostdesc host;

    // WARNING magic number
    // Guarded by MFCC, it should be OK.
    char padding[36 + 16];
    /// Information provided by UMAC
    /* struct umacdesc umac; */ // 36 bytes
    /// Information provided by LMAC
    /* struct lmacdesc lmac; */ // 16 bytes
};
/*
 * -------------------------------- WARNNING end of duplicate definition --------------------------------
 */

/// Structure mapped into the TX buffer for internal handling
struct fhost_tx_desc_tag
{
    /// Chained list element
    struct co_list_hdr hdr;
    /// FHOST TX control information
    struct fhost_tx_ctrl_tag ctrl;
    /// TX SW descriptor passed to MAC
    struct txdesc_host txdesc;

    // WARNING magic number
    // Guarded by MFCC, it should be OK.
    uint8_t pad[68 + 200];
    /// TX HW descriptor attached to the MPDU
    // 68 bytes
    //struct tx_hw_desc hw_desc;
    /// Buffer header structure - Must be the last element of the structure as it shall
    /// be placed just before the payload space
    // 200 bytes
    //struct txl_buffer_tag buffer;
};

/// TX queue structure
struct fhost_tx_queue_tag
{
    /// Next TX queue element (used to chain the TX queue to the scheduler)
    struct fhost_tx_queue_tag *next;
    /// Next TX queue element (used to chain the TX queue to the scheduler)
    struct fhost_tx_queue_tag *prev;
    /// List of TX buffers ready for transmission (@ref fhost_tx_desc_tag.hdr)
    struct co_list ready;
    /// Status of the queue - see @ref fhost_txq_flags
    uint8_t status;
    /// Number of credits currently available in this queue
    int8_t credits;
    /// Number of packet queued in 'ready' list
    uint16_t nb_ready;
    /// Pointer on the last desc that is a sw retry (NULL when there is no retry)
    struct fhost_tx_desc_tag *last_retry;
    #if NX_BEACONING
    /// TXQ PS type (@ref fhost_tx_ps_type)
    int8_t ps_type;
    /// Number of packet to push to complete PS service period
    uint16_t limit;
    #endif //NX_BEACONING
};

/// TX queue scheduler structure
struct fhost_tx_queue_sched_tag
{
    /// Current TX queue element being scheduled
    struct fhost_tx_queue_tag *current;
};

/// Traffic information for one station
struct fhost_tx_sta_traffic
{
    /// Number of packet ready for PS service period among all STA's TXQ
    /// (-1 if station is not in PS mode)
    int ps_ready[PS_TYPE_CNT];
    /// Number of packet to push to complete the PS service period
    int sp_cnt[PS_TYPE_CNT];
    /// Timer to clean packet queued for too long (i.e. > ps_max_queue_time)
    struct mm_timer_tag ps_cleanup_tmr;
    /// Maximum time, in us, a packet can be buffered (for a PS station)
    uint32_t ps_max_queue_time;
};

/**
 * Number of TXQ allocated
 *
 * Currently we have:
 * - One TXQ per TID and per STA:
 *   TXQ id is @verbatim  <sta_id> * TID_MAX + <tid> @endverbatim
 * - One TXQ for unknown STA for each VIF (Only enabled for AP and Monitor interface)
 *   TXQ id is @verbatim (NX_REMOTE_STA_MAX * TID_MAX) + <vif_idx> @endverbatim
 */
#define FHOST_TXQ_CNT ((STA_MAX * TID_MAX) + NX_VIRT_DEV_MAX)

/// FHOST TX environment structure
struct fhost_tx_env_tag
{
    /// Queue used for the transmission requests sent to the WiFi task
    rtos_queue queue_tx;
    /// Queue used to allow other tasks interacting with the FHOST TX task (confirmations
    /// from the WiFi task, transmission requests from the networking stack, etc.)
    rtos_queue queue_msg;
    /// Internal per-STA/TID and per-VIF TX queues
    struct fhost_tx_queue_tag tx_q[FHOST_TXQ_CNT];
    /// TX queue scheduler structures (one per HW queue)
    struct fhost_tx_queue_sched_tag sched[NX_TXQ_CNT];
    /// Mutex used for TX locking
    rtos_mutex tx_lock;
    #if NX_BEACONING
    /// Traffic information for each station(only used by AP interface)
    struct fhost_tx_sta_traffic traffic[STA_MAX];
    #endif
    /// HW queue credits
    uint8_t hwq_credits[NX_TXQ_CNT];
};

/// Macro used to lock the TX mutex
#define FHOST_TX_LOCK()    rtos_mutex_lock(fhost_tx_env.tx_lock)
/// Macro used to unlock the TX mutex
#define FHOST_TX_UNLOCK()  rtos_mutex_unlock(fhost_tx_env.tx_lock)

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/// FHOST RX environment
extern struct fhost_tx_env_tag fhost_tx_env;

/*
 * FUNCTIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the TX task.
 * This function initializes the different data structures used for the TX and creates the
 * RTOS task dedicated to the TX processing.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_tx_init(void);

/**
 ****************************************************************************************
 * @brief Push a buffer for transmission.
 *
 * The buffer is directly pushed with the TX mutex hold.
 *
 * @param[in] net_if      Pointer to the net interface for which the packet is pushed
 * @param[in] net_buf     Pointer to the net buffer to transmit.
 * @param[in] cfm_cb      Callback when transmission has been completed (may be NULL)
 * @param[in] cfm_cb_arg  Private argument for the callback.
 *
 * @return 0 on success and != 0 if error occurred.
 ****************************************************************************************
 */
int fhost_tx_start(net_if_t *net_if, net_buf_tx_t *net_buf,
                   cb_fhost_tx cfm_cb, void *cfm_cb_arg);

/**
 ****************************************************************************************
 * @brief Process a TX confirmation
 *
 * The TX confirmation is directly processed with the TX mutex hold.
 *
 * @param[in] access_category  Access category for the confirmation
 * @param[in] txdesc Pointer to the TX descriptor for which a CFM is pushed
 *
 ****************************************************************************************
 */
void fhost_tx_cfm_push(uint8_t access_category, struct txdesc *txdesc);

/**
 ****************************************************************************************
 * @brief Update the credits of a TX queue, upon the creation or deletion of a BlockAck
 * agreement of the association STA/TID pair.
 *
 * @param[in] sta_id Index of the station
 * @param[in] tid Priority
 * @param[in] credits Number of credits to add/remove on the TX queue
 ****************************************************************************************
 */
void fhost_tx_credits_update(uint8_t sta_id, uint8_t tid, int8_t credits);

/**
 ****************************************************************************************
 * @brief Enable the TX queues for the given STA.
 *
 * Send message @ref FHOST_TX_STA_ADD to FHOST TX thread to enable the TX queues
 * associated to this STA.
 *
 * @param[in] sta_id       Station index to add
 * @param[in] buf_timeout  Maximum time, in us, that a buffer can remain queued in a TXQ
 *                         (only used when adding STA to an AP interface)
 ****************************************************************************************
 */
void fhost_tx_sta_add(uint8_t sta_id, uint32_t buf_timeout);

/**
 ****************************************************************************************
 * @brief Disable the TX queues for the given STA.
 *
 * Send message @ref FHOST_TX_STA_DEL to FHOST TX thread to disable the TX queues
 * associated to this STA. Any pending packets in the queues will be freed.
 *
 * @param[in] sta_id Station index to delete
 ****************************************************************************************
 */
void fhost_tx_sta_del(uint8_t sta_id);

/**
 ****************************************************************************************
 * @brief Enable the TX queues for the given VIF.
 *
 * Send message @ref FHOST_TX_VIF_ENABLE to FHOST TX thread to enable the TX queues
 * associated to a VIF. The queues enabled depends on the interface type. For example a
 * STA interface doesn't have any VIF TX queues;
 * To be called as soon as the interface is created.
 *
 * @param[in] fhost_vif  Pointer to the fhost VIF structure
 ****************************************************************************************
 */
void fhost_tx_vif_txq_enable(struct fhost_vif_tag *fhost_vif);

/**
 ****************************************************************************************
 * @brief Disable the TX queues for the given VIF.
 *
 * Send message @ref FHOST_TX_VIF_DISABLE to FHOST TX thread to disable the TX queues
 * associated to a VIF. The queues disabled depends on the interface type.
 * To be called before deleting the interface.
 *
 * @param[in] fhost_vif  Pointer to the fhost VIF structure
 ****************************************************************************************
 */
void fhost_tx_vif_txq_disable(struct fhost_vif_tag *fhost_vif);

/**
 ****************************************************************************************
 * @brief Inform TX task that a station enter or exist PS mode
 *
 * When station enters PS mode, the TX task will stop pushing packet to this station.
 * Packets are buffered until they are requested (@ref fhost_tx_sta_ps_traffic_req) by
 * the UMAC layer to complete a service period (legacy or UAPSD) or station leaves PS.
 *
 * When a station exit PS mode, the TX task will restart pushing packet to this station
 * stating by all the packet currently buffered.
 *
 * @param[in] sta_id   Station Index
 * @param[in] enable   Whether PS is enable or not
 *
 * @return 0 if information has been successfully sent to TX task and 0 otherwise.
 ****************************************************************************************
 */
int fhost_tx_sta_ps_enable(uint8_t sta_id, bool enable);

/**
 ****************************************************************************************
 * @brief Requested buffered traffic for a STATION in PS mode
 *
 * Upon traffic request the TX task will configure STA's TXQ so that the requested packet
 * are allowed to be pushed.
 *
 * @param[in] sta_id   Station Index
 * @param[in] pkt_cnt  Number of packet requested (0 means all)
 * @param[in] uapsd    Whether UAPSD traffic or legacy traffic is requested
 *
 * @return 0 if request has been successfully sent to TX task and 0 otherwise.
 ****************************************************************************************
 */
int fhost_tx_sta_ps_traffic_req(uint8_t sta_id, int pkt_cnt, bool uapsd);

/**
 ****************************************************************************************
 * @brief Inform TX task that the VIF enters/exits PS mode
 *
 * When the VIF enters in PS mode, all TXQ linked to the VIF are stopped
 *
 * When the VIF exit PS mode, the TX task will restart the TXQ and will push packet to the
 * station starting by all the packet currently buffered.
 *
 * @param[in] vif_idx  VIF Index
 * @param[in] enable   Whether PS is enable or not
 *
 * @return 0 if information has been successfully sent to TX task and 0 otherwise.
 ****************************************************************************************
 */
int fhost_tx_vif_ps_enable(uint8_t vif_idx, uint8_t enable);

/// @}

#endif // _FHOST_TX_H_

