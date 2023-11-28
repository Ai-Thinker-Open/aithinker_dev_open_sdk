/**
 ****************************************************************************************
 *
 * @file ipc_emb.h
 *
 * @brief IPC module context on emb side.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

#ifndef _IPC_EMB_H_
#define _IPC_EMB_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "export/common/co_int.h"
#include "export/common/co_bool.h"

#include "compiler.h"

#include "export/common/co_status.h"
#include "ipc_shared.h"
#include "modules/ke/src/ke_queue.h"
#include "modules/ke/src/ke_event.h"
#include "plf_build_reg/reg_ipc_emb.h"

// forward declarations
struct ke_msg;

/**
 ****************************************************************************************
 * @addtogroup IPC
 * @{
 ****************************************************************************************
 */
/// Structure describing a queue used for delayed TX descriptors
struct ipc_tx_queue
{
    /// For chaining prior to processing in delayed event
    struct co_list_hdr list_hdr;
    /// TX descriptor lists for queues that are disabled
    struct co_list queue;
    /// Index of the queue
    uint8_t index;
    /// User of the queue
    uint8_t user;
};

/// Structure describing the IPC environment
struct ipc_emb_env_tag
{
    /// Queue of RX descriptors pending for indication to host
    struct co_list rx_queue;

    /// Queue of TX confirmations pending for indication to host
    struct co_list cfm_queue;

    #if (NX_UMAC_PRESENT)
    /// Index of the host RX descriptor array
    uint8_t ipc_rxdesc_idx;
    #endif //(NX_UMAC_PRESENT)
    /// Index of the host RX buffer array
    uint8_t ipc_rxbuf_idx;
    /// Index of the host radar buffer array
    uint8_t ipc_radar_buf_idx;
    /// index of the host MSG E2A buffer array
    uint8_t ipc_msge2a_buf_idx;
    /// index of the host Debug buffer array
    uint8_t ipc_dbg_buf_idx;
    /// count of E2A MSG ACKs of A2E MSGs
    uint8_t ipc_msgacke2a_cnt;
    /// Index of the host unsupported rx vector buffer array
    uint8_t ipc_unsup_rx_vec_buf_idx;

    /// Bitfield indicating which TX queues are involved in the IPC TX event
    uint32_t tx_q_status;
    /// Index of the next TX descriptor pending for download
    uint32_t ipc_txdesc_pending_idx;
    /// Index of the next TX descriptor ready to be pushed to TX path
    uint32_t ipc_txdesc_ready_idx;
    /// Index for next entry in txcfm_hostbuf_addr array
    uint32_t ipc_txcfm_idx;

    #if (NX_BEACONING && !NX_UMAC_PRESENT) || RW_MUMIMO_TX_EN
    /// Bitfield indicating which user positions can be scheduled
    uint8_t user_active[IPC_TXQUEUE_CNT];
    /// Queues of TX descriptor delayed because of disabled users/hw queue
    struct ipc_tx_queue txq[IPC_TXQUEUE_CNT][RW_USER_MAX];
    /// Lists of delayed queues to be processed in the delayed event
    struct co_list delayed_queue_list;
    #endif
};

/// Embedded IPC environment variable
extern struct ipc_emb_env_tag ipc_emb_env;

/// Mask of the TX descriptor length per queue. These values allow wrapping the TX descriptor
/// indexes used to address the TX descriptor arrays in shared RAM.
extern const int nx_txdesc_cnt_msk[];

/**
 ****************************************************************************************
 * @brief Get the host address of the TX payload descriptor pattern
 *
 * @return The address of the TX descriptor pattern
 *
 ****************************************************************************************
 */
__INLINE uint32_t ipc_emb_tx_pattern_addr_get(void)
{
    return(ipc_shared_env.pattern_addr);
}

/**
 ****************************************************************************************
 * @brief Get the buffered data from the IPC shared memory for a given sta/tid
 *
 * @param[in] sta  STA index
 * @param[in] tid  TID
 *
 * @return number of bytes buffered
 *
 ****************************************************************************************
 */
__INLINE uint32_t ipc_emb_buffered_get(uint8_t sta, uint8_t tid)
{
    return(ipc_shared_env.buffered[sta][tid]);
}

#if NX_BEACONING && !NX_UMAC_PRESENT
/**
 ****************************************************************************************
 * @brief Check if some delayed TX descriptors are available on the requested queue
 *
 * @param[in] queue_idx Index of the TX queue
 *
 * @return true if some descriptors are available, false otherwise
 ****************************************************************************************
 */
__INLINE bool ipc_emb_tx_q_has_data(int queue_idx, int user_pos)
{
    struct ipc_tx_queue *txq = &ipc_emb_env.txq[queue_idx][user_pos];

    return !co_list_is_empty(&txq->queue);
}

/**
 ****************************************************************************************
 * @brief Check how many delayed tx descriptors are available on the requested queue
 * for a VIF
 *
 * @param[in] queue_idx Index of the TX queue
 * @param[in] vif_idx   Index of the Vif
 *
 * @return Number of delayed txdesc ready on the TX queue for the specified vif
 ****************************************************************************************
 */
uint8_t ipc_emb_tx_q_len(int queue_idx, int vif_idx);

/**
 ****************************************************************************************
 * @brief Push all the delayed descriptors of a specific queue_idx/user_pos pair to the
 * TX path.
 *
 * The queue remains in the disabled state.
 *
 * @param[in] queue_idx Index of the TX queue
 * @param[in] user_pos  User position
 ****************************************************************************************
 */
void ipc_emb_tx_q_push_all(int queue_idx, uint8_t user_pos);
#endif //NX_BEACONING && !NX_UMAC_PRESENT

#if (NX_BEACONING && !NX_UMAC_PRESENT) || RW_MUMIMO_TX_EN
/**
 ****************************************************************************************
 * @brief Re-enable the user positions IPC queues
 *
 * @param[in] queue_idx     Index of the TX queue
 * @param[in] active_users  Bit field of user position queues to re-enable
 *
 ****************************************************************************************
 */
void ipc_emb_enable_users(int queue_idx, uint8_t active_users);

/**
 ****************************************************************************************
 * @brief Disable IPC queue for user positions
 *
 * @param[in] queue_idx     Index of the TX queue
 * @param[in] active_users  Bit field of user position queues to pause
 *
 ****************************************************************************************
 */
__INLINE void ipc_emb_disable_users(int queue_idx, uint8_t paused_users)
{
    ipc_emb_env.user_active[queue_idx] &= ~(paused_users);
}

/**
 ****************************************************************************************
 * @brief Test whether user positions IPC queue are enabled or not
 *
 * @param[in] queue_idx     Index of the TX queue
 * @param[in] active_users  Bit field of user position queues to test
 * @return True if all IPC queues of specified user positions are enabled and false
 * otherwise
 ****************************************************************************************
 */
__INLINE bool ipc_emb_users_enabled(int queue_idx, uint8_t users)
{
    uint32_t user_enabled = ipc_emb_env.user_active[queue_idx];
    return (user_enabled & users) == users;
}
#endif // (NX_BEACONING && !NX_UMAC_PRESENT) || RW_MUMIMO_TX_EN

#if RW_MUMIMO_TX_EN
/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of the delayed TX descriptors queues.
 *
 * Triggered when users are re-enabled
 *
 * @param[in] dummy Unused parameter
 ****************************************************************************************
 */
void ipc_emb_tx_q_evt(int dummy);
#endif // RW_MUMIMO_TX_EN

/**
 ****************************************************************************************
 * @brief Indicate to the upper side that at least one radar event is pending.
 *
 ****************************************************************************************
 */
void ipc_emb_radar_event_ind(void);

#if NX_UF_EN
/**
 ****************************************************************************************
 * @brief Indicate to the upper side that one unsupported rx vector is pending.
 *
 ****************************************************************************************
 */
void ipc_emb_unsup_rx_vec_event_ind(void);
#endif

/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address to upload a TX confirmation
 *
 * @return Hostbuf address.
 ****************************************************************************************
 */
uint32_t ipc_emb_hosttxcfmbuf_get(void);

/**
 ****************************************************************************************
 * @brief Indicate to the upper side that at least on confirmation is pending.
 *
 * @param[in]   queue_bits   Bitfields indicating for which queues confirmations are pending
 *
 ****************************************************************************************
 */
void ipc_emb_txcfm_ind(uint32_t queue_bits);


/**
 ****************************************************************************************
 * @brief Checks if a host buffer is currently available.
 *
 * If no buffer is available, then the RX buffer pushed interrupt is enabled in order
 * to be warned about a new buffer push when RX flow control is enabled.
 *
 * @return true if a buffer is available, false otherwise.
 *
 ****************************************************************************************
 */
bool ipc_emb_hostrxbuf_check(void);

/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address for future DMA transfer
 *
 * This function gives back the hostbuf address according to the current value of
 * the rx buffer index. It just returns the pointer values set by the upper
 * layer at init time and after each reception.
 * The buffer address in memory is then cleared and the FIFO index is incremented. The
 * buffer is therefore not available anymore after the call.
 * Prior to the call to this function, a call to @ref ipc_emb_hostrxbuf_check has to be
 * performed to ensure that there is a buffer available.
 *
 * @if full_doc
 * @param[out] host_id    HostId linked to the buffer
 * @endif
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
#if (NX_UMAC_PRESENT)
uint32_t ipc_emb_hostrxbuf_get(uint32_t *host_id);
#else
uint32_t ipc_emb_hostrxbuf_get(void);
#endif //(NX_UMAC_PRESENT)

#if (NX_UMAC_PRESENT)
/**
 ****************************************************************************************
 * @brief Check if a RX descriptor is available
 *
 * @return true if a descriptor is available, false otherwise.
 *
 ****************************************************************************************
 */
bool ipc_emb_hostrxdesc_check(void);

/**
 ****************************************************************************************
 * @brief Retrieve a host RX descriptor address for future DMA transfer
 *
 * @return RX descriptor address.
 *
 ****************************************************************************************
 */
uint32_t ipc_emb_hostrxdesc_get(void);
#endif //(NX_UMAC_PRESENT)

/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address for debug dump transfer
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
uint32_t ipc_emb_hostdbgdumpbuf_get(void);

/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address for future radar event DMA transfer
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
uint32_t ipc_emb_hostradarbuf_get(void);

#if NX_UF_EN
/**
 ****************************************************************************************
 * @brief Retrieve a hostbuf address for future rx vector DMA transfer
 *
 * @return Hostbuf address.
 *
 ****************************************************************************************
 */
uint32_t ipc_emb_hostunsuprxvectbuf_get(void);
#endif

/**
 ****************************************************************************************
 * @brief Sends an Data Reception Indication to the upper layer
 *
 * This function warns the upper layer about Data Reception through the setting of
 * IPC_IRQ_E2A_RXDESC trigger.
 *
 ****************************************************************************************
 */
void ipc_emb_rxdata_ind(void);


/**
 ****************************************************************************************
 * @brief Forward a kernel message to the host side (emb side)
 *
 * Implementation of @ref macif_kmsg_fwd when control layer is running on a remote host
 *
 * @param[in] ke_msg  Pointer on the message
 *
 ****************************************************************************************
 */
void ipc_emb_kmsg_fwd(const struct ke_msg *ke_msg);

/**
 ****************************************************************************************
 * @brief Initialize the IPC on the embedded side.
 *
 * @warning Since this function sets some Init values in IPC Shared memory it must not be
 * called before ipc_host_init() which resets all IPC Shared memory at boot.
 *
 ****************************************************************************************
 */
void ipc_emb_init(void);


/**
 ****************************************************************************************
 * @brief Handle Tx packet interrupt on the emb side
 *
 ****************************************************************************************
 */
void ipc_emb_tx_irq(void);

/**
 ****************************************************************************************
 * @brief Handle the DMA interrupt triggered upon download of TX descriptors
 *
 ****************************************************************************************
 */
void ipc_emb_dma_tx_irq(void);

/**
 ****************************************************************************************
 * @brief Handle kernel message interrupt on the emb side
 *
 ****************************************************************************************
 */
void ipc_emb_msg_irq(void);


/**
 ****************************************************************************************
 * @brief Handle confirmation events interrupt on the emb side
 *
 ****************************************************************************************
 */
void ipc_emb_cfmback_irq(void);

/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of the TX descriptor pushed by the host
 *
 * @param[in] queue_idx Index of the TX queues for which the event is called
 *
 ****************************************************************************************
 */
void ipc_emb_tx_evt(int queue_idx);

/**
 ****************************************************************************************
 * @brief Kernel event handler for the handling of messages pushed by the host
 *
 * Implementation of @ref macif_msg_evt when control layer is running on a remote host
 *
 * @param[in] dummy Parameter not used but required to follow the kernel event callback
 * format
 *
 ****************************************************************************************
 */
void ipc_emb_msg_evt(int dummy);

/**
 ****************************************************************************************
 * @brief Handles the DMA interrupt generated when the MSG from Embedded side to
 * Application has been transferred.
 *
 ****************************************************************************************
 */
void ipc_emb_msg_dma_int_handler(void);

/**
 ****************************************************************************************
 * @brief Send an EMB string to the APP - IPC API function
 *
 * @param[in]   poll    Poll to ensure that print has gone when we exit the function
 * @param[in]   len     Length of the message
 * @param[in]   string  Pointer to the message
 *                      Start must be word aligned (assumed to be the sprintf buffer)
 *                      The string may be truncated if it is too long.
 *
 ****************************************************************************************
 */
void ipc_emb_print_fwd(bool poll, const uint32_t len, char *string);

/**
 ****************************************************************************************
 * @brief Handles the DMA interrupt generated when the Debug message from Embedded side to
 * Application has been transferred.
 *
 ****************************************************************************************
 */
void ipc_emb_dbg_dma_int_handler(void);

/// @} IPC_MISC

#endif // _IPC_EMB_H_
