/**
 ****************************************************************************************
 *
 * @file ipc_shared.h
 *
 * @brief Shared data between both IPC modules for fully hosted firmware.
 * In this case IPC is only use for test or debug
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

#ifndef _IPC_SHARED_H_
#define _IPC_SHARED_H_

/**
 ****************************************************************************************
 * @defgroup IPC IPC
 * @ingroup PLATFORM_DRIVERS
 * @brief Inter Processor Communication module.
 *
 * In fully hosted firmware IPC is only for debug and test purpose.
 *
 * @{
 ****************************************************************************************
 */

#include "export/common/co_int.h"
#include "plf/refip/src/driver/dma/dma.h"


/// To remove
#define IPC_RXBUF_CNT       128

/// Number of Host buffers available for Emb->App MSGs sending (through DMA)
#define IPC_MSGE2A_BUF_CNT       64

/// Number of Host buffers available for Debug Messages sending (through DMA)
#define IPC_DBGBUF_CNT       32

/// Size, in bytes, of IPC buffers
#define IPC_MSG_BUF_SIZE 256

/// Length used in APP2EMB MSGs structures
#define IPC_A2E_MSG_BUF_SIZE    127 // size in 4-byte words

/// Length used in EMB2APP MSGs structures
#define IPC_E2A_MSG_PARAM_SIZE   256 // size in 4-byte words

/// Debug messages buffers size (in bytes)
#define IPC_DBG_PARAM_SIZE       256

/// Configuration Size in bytes
#define IPC_CFG_SIZE 512

/**
 * Define used for MSG buffers validity.
 * This value will be written only when a MSG buffer is used for sending from Emb to App.
 */
#define IPC_MSGE2A_VALID_PATTERN 0xADDEDE2A

/**
 * Define used for Debug messages buffers validity.
 * This value will be written only when a DBG buffer is used for sending from Emb to App.
 */
#define IPC_DBG_VALID_PATTERN 0x000CACA0

/// Message structure for Debug messages from Emb to App
struct ipc_dbg_msg
{
    uint32_t string[IPC_DBG_PARAM_SIZE/4]; ///< Debug string
    uint32_t pattern;                    ///< Used to stamp a valid buffer
};

/// Message structure for IPC message
struct ipc_fhost_msg
{
    uint32_t id;   ///< Id of the message (not used)
    uint32_t len;  ///< Length in bytes
    uint32_t data[IPC_MSG_BUF_SIZE/4]; ///< Message data
    uint32_t pattern; ///< unique pattern
};

/// Structure describing the IPC data shared with the host CPU
struct ipc_shared_env_tag
{
    /// Room for MSG to be sent from App to Emb
    volatile struct ipc_fhost_msg msg_a2e_buf;

    /// Room to build the Emb->App MSGs Xferred
    volatile struct ipc_fhost_msg msg_e2a_buf;
    /// DMA descriptor for Emb->App MSGs Xfers
    volatile struct dma_desc msg_dma_desc;
    /// Host buffer addresses for Emb->App MSGs DMA Xfers
    volatile uint32_t  msg_e2a_hostbuf_addr [IPC_MSGE2A_BUF_CNT];

    /// Room to build the Emb->App DBG messages
    volatile struct ipc_dbg_msg dbg_buf;
    /// DMA descriptor for Emb->App DBG messages Xfers
    volatile struct dma_desc dbg_dma_desc;
    /// Host buffer addresses for DBG messages Xfers
    volatile uint32_t dbg_hostbuf_addr [IPC_DBGBUF_CNT];

    /// Host buffer address for the debug dump information
    volatile uint32_t la_dbginfo_addr;

    /// Configuration buffer
    volatile uint32_t config[IPC_CFG_SIZE/4];

    /// Pattern for host to check if trace is initialized
    volatile uint16_t trace_pattern;
    /// Index (in 16bits word) of the first trace within the trace buffer
    volatile uint32_t trace_start;
    /// Index (in 16bits word) of the last trace within the trace buffer
    volatile uint32_t trace_end;
    /// Size (in 16bits word) of the trace buffer
    volatile uint32_t trace_size;
    /// Offset (in bytes) from here to the start of the trace buffer
    volatile uint32_t trace_offset;
    /// Number of filterable trace components
    volatile uint32_t trace_nb_compo;
    /// Offset (in bytes) from here to the start of the activation table
    volatile uint32_t trace_offset_compo;
};

/// IPC Shared environment
extern struct ipc_shared_env_tag ipc_shared_env;

/// @name APP2EMB IPC interrupt definitions
/// @{

/// IPC APP2EMB message interrupt bit
#define IPC_IRQ_A2E_MSG             CO_BIT(1)
/// IPC APP2EMB debug interrupt bit
#define IPC_IRQ_A2E_DBG             CO_BIT(0)
/// IPC global interrupt mask
#define IPC_IRQ_A2E_ALL             (IPC_IRQ_A2E_MSG|IPC_IRQ_A2E_DBG)

/// @}

/// @name EMB2APP IPC interrupt definitions
/// @{

/// IPC APP2EMB message acknowledgment interrupt
#define IPC_IRQ_E2A_MSG_ACK     CO_BIT(2)
/// IPC EMB2APP message interrupt
#define IPC_IRQ_E2A_MSG         CO_BIT(1)
/// IPC EMB2APP debug interrupt
#define IPC_IRQ_E2A_DBG         CO_BIT(0)
/// IPC EMB2APP global interrupt mask
#define IPC_IRQ_E2A_ALL         ( IPC_IRQ_E2A_MSG_ACK       \
                                  | IPC_IRQ_E2A_MSG         \
                                  | IPC_IRQ_E2A_DBG )

/// @}

/// @}

#endif // _IPC_SHARED_H_

