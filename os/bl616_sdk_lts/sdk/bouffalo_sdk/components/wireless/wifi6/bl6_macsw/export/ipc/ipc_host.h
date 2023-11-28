/**
 ****************************************************************************************
 *
 * @file ipc_host.h
 *
 * @brief IPC module.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */
#ifndef _IPC_HOST_H_
#define _IPC_HOST_H_

/** @addtogroup IPC
 *  @{
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "ipc_shared.h"

/*
 * ENUMERATION
 ****************************************************************************************
 */

enum ipc_host_desc_status
{
    /// Descriptor is IDLE
    IPC_HOST_DESC_IDLE      = 0,
    /// Data can be forwarded
    IPC_HOST_DESC_FORWARD,
    /// Data has to be kept in UMAC memory
    IPC_HOST_DESC_KEEP,
    /// Delete stored packet
    IPC_HOST_DESC_DELETE,
    /// Update Frame Length status
    IPC_HOST_DESC_LEN_UPDATE,
    /// Update Frame Length + Length field inside Ethernet Header status
    IPC_HOST_DESC_ETH_LEN_UPDATE,
};

/*
 * TYPE DECLARATION
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief This structure is used to initialize the MAC SW
 *
 * The WLAN device driver provides functions call-back with this structure
 ****************************************************************************************
 */
struct ipc_txcfm
{
    uint32_t    hostid;     ///< Pre-allocated packet ID from the OS (skbuff on Linux)
    uint32_t    status;     ///< Status of the transmission
};

struct ipc_host_cb_tag
{
    /// WLAN driver call-back function: send_data_cfm
    int (*send_data_cfm)(void *pthis, uint32_t host_id);

    /// WLAN driver call-back function: recv_data_ind
    uint8_t (*recv_data_ind)(void *pthis, uint32_t host_id);

    /// WLAN driver call-back function: recv_msg_ind
    uint8_t (*recv_msg_ind)(void *pthis, uint32_t host_id);

    /// WLAN driver call-back function: recv_msgack_ind
    uint8_t (*recv_msgack_ind)(void *pthis, uint32_t host_id);

    /// WLAN driver call-back function: recv_dbg_ind
    uint8_t (*recv_dbg_ind)(void *pthis, uint32_t host_id);

    /// WLAN driver call-back function: prim_tbtt_ind
    void (*prim_tbtt_ind)(void *pthis);

    /// WLAN driver call-back function: sec_tbtt_ind
    void (*sec_tbtt_ind)(void *pthis);

};

/*
 * Struct used to store information about host buffers (DMA Address and local pointer)
 */
struct ipc_hostbuf
{
    uint32_t hostid;      ///< ptr to hostbuf client (ipc_host client) structure
    uint32_t dma_addr;    ///< ptr to real hostbuf dma address
};

#if (NX_UMAC_PRESENT)
/*
 * Struct used to store information about host descriptors (DMA Address and local pointer)
 */
struct ipc_hostdesc
{
    uint32_t dma_addr;    ///< ptr to real hostbuf dma address
};
#endif //(NX_UMAC_PRESENT)

/// Definition of the IPC Host environment structure.
struct ipc_host_env_tag
{
    /// Structure containing the callback pointers
    struct ipc_host_cb_tag cb;

    /// Pointer to the shared environment
    struct ipc_shared_env_tag *shared;

    #if (NX_UMAC_PRESENT)
    // Array used to store the descriptor addresses
    struct ipc_hostdesc ipc_host_rxdesc_array[IPC_RXDESC_CNT];
    // Index of the host RX descriptor array (ipc_shared environment)
    uint8_t ipc_host_rxdesc_idx;
    #endif //(NX_UMAC_PRESENT)

    /// Fields for Data Rx handling
    // Global array used to store the hostid and hostbuf addresses
    struct ipc_hostbuf ipc_host_rxbuf_array[IPC_RXBUF_CNT];
    // Index used for ipc_host_rxbuf_array to point to current buffer
    uint8_t ipc_host_rxbuf_idx;
    // Store the number of Rx Data buffers
    uint32_t rx_bufnb;
    // Store the size of the Rx Data buffers
    uint32_t rx_bufsz;

    // Index used that points to the first free TX desc
    uint32_t txdesc_free_idx[IPC_TXQUEUE_CNT];
    // Index used that points to the first used TX desc
    uint32_t txdesc_used_idx[IPC_TXQUEUE_CNT];
    // Array storing the currently pushed host ids for the BK queue
    uint32_t tx_host_id0[NX_TXDESC_CNT0];
    // Array storing the currently pushed host ids for the BE queue
    uint32_t tx_host_id1[NX_TXDESC_CNT1];
    // Array storing the currently pushed host ids for the VI queue
    uint32_t tx_host_id2[NX_TXDESC_CNT2];
    // Array storing the currently pushed host ids for the VO queue
    uint32_t tx_host_id3[NX_TXDESC_CNT3];
    #if (NX_BEACONING)
    // Array storing the currently pushed host ids for the BCN queue
    uint32_t tx_host_id4[NX_TXDESC_CNT4];
    #endif
    // Pointer to the different host ids arrays, per IPC queue
    uint32_t *tx_host_id[IPC_TXQUEUE_CNT];
    // Pointer to the different TX descriptor arrays, per IPC queue
    volatile struct txdesc_host *txdesc[IPC_TXQUEUE_CNT];

    /// Fields for Emb->App MSGs handling
    // Global array used to store the hostid and hostbuf addresses for msg/ind
    struct ipc_hostbuf ipc_host_msgbuf_array[IPC_MSGE2A_BUF_CNT];
    // Index of the MSG E2A buffers array to point to current buffer
    uint8_t ipc_host_msge2a_idx;
    // Store the number of E2A MSG buffers
    uint32_t ipc_e2amsg_bufnb;
    // Store the size of the E2A MSG buffers
    uint32_t ipc_e2amsg_bufsz;

    /// E2A ACKs of A2E MSGs
    uint8_t msga2e_cnt;
    uint32_t msga2e_hostid;

    /// Fields for Debug MSGs handling
    // Global array used to store the hostid and hostbuf addresses for Debug messages
    struct ipc_hostbuf ipc_host_dbgbuf_array[IPC_DBGBUF_CNT];
    // Index of the Debug messages buffers array to point to current buffer
    uint8_t ipc_host_dbg_idx;
    // Store the number of Debug messages buffers
    uint32_t ipc_dbg_bufnb;
    // Store the size of the Debug messages buffers
    uint32_t ipc_dbg_bufsz;

    /// Pointer to the attached object (used in callbacks and register accesses)
    void *pthis;
};

/**
 ****************************************************************************************
 * @brief Initialize the IPC running on the Application CPU.
 *
 * This function:
 *   - initializes the IPC software environments
 *   - enables the interrupts in the IPC block
 *
 * @param[in]   env   Pointer to the IPC host environment
 *
 * @warning Since this function resets the IPC Shared memory, it must be called before
 * the LMAC FW is launched because LMAC sets some init values in IPC Shared memory at boot.
 *
 ****************************************************************************************
 */
void ipc_host_init(struct ipc_host_env_tag *env,
                  struct ipc_host_cb_tag *cb,
                  struct ipc_shared_env_tag *shared_env_ptr,
                  void *pthis);


/**
 ****************************************************************************************
 * @brief Retrieve a new free Tx descriptor (host side).
 *
 * This function returns a pointer to the next Tx descriptor available from the queue
 * queue_idx to the host driver. The driver will have to fill it with the
 * appropriate endianness and to send it to the
 * emb side with ipc_host_txdesc_push().
 *
 * This function should only be called once until ipc_host_txdesc_push() is called.
 *
 * This function will return NULL if the queue is full.
 *
 * @param[in]   env   Pointer to the IPC host environment
 * @param[in]   queue_idx   Queue index. The index can be inferred from the user priority
 *                          of the incoming packet.
 * @return                  Pointer to the next Tx descriptor free. This can point to
 *                          the host memory or to shared memory, depending on IPC
 *                          implementation.
 *
 ****************************************************************************************
 */
volatile struct txdesc_host *ipc_host_txdesc_get(struct ipc_host_env_tag *env,
                                                 const int queue_idx);


/**
 ****************************************************************************************
 * @brief Push a filled Tx descriptor (host side).
 *
 * This function sets the next Tx descriptor available by the host side:
 * - as used for the host side
 * - as available for the emb side.
 * The Tx descriptor must be correctly filled before calling this function.
 *
 * This function may trigger an IRQ to the emb CPU depending on the interrupt
 * mitigation policy and on the push count.
 *
 * @param[in]   env   Pointer to the IPC host environment
 * @param[in]   queue_idx   Queue index. Same value than ipc_host_txdesc_get()
 * @param[in]   host_id     Parameter indicated by the IPC at TX confirmation, that allows
 *                          the driver finding the buffer
 *
 ****************************************************************************************
 */
void ipc_host_txdesc_push(struct ipc_host_env_tag *env, const int queue_idx, const uint32_t host_id);

void ipc_host_patt_addr_push(struct ipc_host_env_tag *env, uint32_t addr);

/**
 ****************************************************************************************
 * @brief Push a pre-allocated buffer descriptor for Rx packet (host side)
 *
 * This function should be called by the host IRQ handler to supply the embedded
 * side with new empty buffer.
 *
 * @param[in]   env         Pointer to the IPC host environment
 * @param[in]   hostid      Packet ID used by the host (skbuff pointer on Linux)
 * @param[in]   hostbuf     Pointer to the start of the buffer payload in the
 *                          host memory (this may be inferred from the skbuff?)
 *                          The length of this buffer should be predefined
 *                          between host and emb statically (constant needed?).
 *
 ****************************************************************************************
 */
int ipc_host_rxbuf_push(struct ipc_host_env_tag *env, uint32_t hostid, uint32_t hostbuf);

#if (NX_UMAC_PRESENT)
void ipc_host_rx_bufnb_inc(struct ipc_host_env_tag *env);
void ipc_host_rx_bufnb_dec(struct ipc_host_env_tag *env);
int ipc_host_rxdesc_push(struct ipc_host_env_tag *env, uint32_t dma_addr);
#endif //(NX_UMAC_PRESENT)

/**
 ****************************************************************************************
 * @brief Push a pre-allocated buffer descriptor for MSGs (host side)
 *
 * This function is only called at Init time since the MSGs will be handled directly and
 * buffer can be re-used as soon as the message is handled, no need to re-allocate new
 * buffers in the meantime.
 *
 * @param[in]   env         Pointer to the IPC host environment
 * @param[in]   hostid      Packet ID used by the host (skbuff pointer on Linux)
 * @param[in]   hostbuf     Pointer to the start of the buffer payload in the
 *                          host memory.
 *                          The length of this buffer should be predefined
 *                          between host and emb statically.
 *
 ****************************************************************************************
 */
int ipc_host_msgbuf_push(struct ipc_host_env_tag *env, uint32_t hostid,
                                                       uint32_t hostbuf);

/**
 ****************************************************************************************
 * @brief Push a pre-allocated buffer descriptor for Debug messages (host side)
 *
 * This function is only called at Init time since the Debug messages will be handled directly and
 * buffer can be re-used as soon as the message is handled, no need to re-allocate new
 * buffers in the meantime.
 *
 * @param[in]   env         Pointer to the IPC host environment
 * @param[in]   hostid      Packet ID used by the host (skbuff pointer on Linux)
 * @param[in]   hostbuf     Pointer to the start of the buffer payload in the
 *                          host memory.
 *                          The length of this buffer should be predefined
 *                          between host and emb statically.
 *
 ****************************************************************************************
 */
int ipc_host_dbgbuf_push(struct ipc_host_env_tag *env, uint32_t hostid,
                                                       uint32_t hostbuf);

/**
 ****************************************************************************************
 * @brief Push the pre-allocated logic analyzer and debug information buffer
 *
 * @param[in]   env         Pointer to the IPC host environment
 * @param[in]   infobuf     Pointer to the start of the buffer payload in the
 *                          host memory.
 *                          The length of this buffer should be predefined
 *                          between host and emb statically.
 *
 ****************************************************************************************
 */
void ipc_host_dbginfobuf_push(struct ipc_host_env_tag *env, uint32_t infobuf);


/**
 ****************************************************************************************
 * @brief Handle all IPC interrupts on the host side.
 *
 * The following interrupts should be handled:
 * Tx confirmation, Rx buffer requests, Rx packet ready and kernel messages
 *
 * @param[in]   env   Pointer to the IPC host environment
 *
 ****************************************************************************************
 */
void ipc_host_irq(struct ipc_host_env_tag *env, uint32_t status);

/**
 ****************************************************************************************
 * @brief Handler for kernel message called from general IRQ handler
 *
 * @param[in]   env      pointer to the IPC Host environment
 ****************************************************************************************
 */
void ipc_host_msg_handler(struct ipc_host_env_tag *env);

/**
 ****************************************************************************************
 * @brief Send a message to the embedded side
 *
 * @param[in]   env      Pointer to the IPC host environment
 * @param[in]   msg_buf  Pointer to the message buffer
 * @param[in]   msg_len  Length of the message to be transmitted
 *
 * @return      Non-null value on failure
 *
 ****************************************************************************************
 */
int ipc_host_msg_push(struct ipc_host_env_tag *env, void *msg_buf, uint16_t len);

/**
 ****************************************************************************************
 * @brief Enable IPC interrupts
 *
 * @param[in]   env  Global ipc_host environment pointer
 * @param[in]   value  Bitfield of the interrupts to enable
 *
 * @warning After calling this function, IPC interrupts can be triggered at any time.
 * Potentially, an interrupt could happen even before returning from the function if
 * there is a request pending from the embedded side.
 *
 ****************************************************************************************
 */
void ipc_host_enable_irq(struct ipc_host_env_tag *env, uint32_t value);

uint32_t ipc_host_get_status(struct ipc_host_env_tag *env);
uint32_t ipc_host_get_rawstatus(struct ipc_host_env_tag *env);

/// @} IPC


#endif // _IPC_HOST_H_
