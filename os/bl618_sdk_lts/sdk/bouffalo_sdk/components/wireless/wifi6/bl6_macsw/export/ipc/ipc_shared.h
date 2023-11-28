/**
 ****************************************************************************************
 *
 * @file ipc_shared.h
 *
 * @brief Shared data between both IPC modules.
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
 * The IPC module implements the protocol to communicate between the Host CPU
 * and the Embedded CPU.
 *
 * A typical use case of the IPC Tx path API:
 * @msc
 * hscale = "2";
 *
 * a [label=Driver],
 * b [label="IPC host"],
 * c [label="IPC emb"],
 * d [label=Firmware];
 *
 * ---   [label="Tx descriptor queue example"];
 * a=>a  [label="Driver receives a Tx packet from OS"];
 * a=>b  [label="ipc_host_txdesc_get()"];
 * a<<b  [label="struct txdesc_host *"];
 * a=>a  [label="Driver fill the descriptor"];
 * a=>b  [label="ipc_host_txdesc_push()"];
 * ...   [label="(several Tx desc can be pushed)"];
 * b:>c  [label="Tx desc queue filled IRQ"];
 * c=>>d [label="IPC emb Tx desc callback"];
 * ...   [label="(several Tx desc can be popped)"];
 * d=>d  [label="Packets are sent or discarded"];
 * ---   [label="Tx confirm queue example"];
 * c<=d  [label="ipc_emb_txcfm_push()"];
 * c>>d  [label="Request accepted"];
 * ...   [label="(several Tx cfm can be pushed)"];
 * b<:c  [label="Tx cfm queue filled IRQ"];
 * a<<=b [label="Driver's Tx Confirm callback"];
 * a=>b  [label="ipc_host_txcfm_pop()"];
 * a<<b  [label="struct ipc_txcfm"];
 * a<=a  [label="Packets are freed by the driver"];
 * @endmsc
 *
 * A typical use case of the IPC Rx path API:
 * @msc
 * hscale = "2";
 *
 * a [label=Firmware],
 * b [label="IPC emb"],
 * c [label="IPC host"],
 * d [label=Driver];
 *
 * ---   [label="Rx buffer and desc queues usage example"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * ...   [label="(several Rx buffer are pushed)"];
 * a=>a  [label=" Frame is received\n from the medium"];
 * a<<b  [label="struct ipc_rxbuf"];
 * a=>a  [label=" Firmware fill the buffer\n with received frame"];
 * a<<b  [label="Push accepted"];
 * ...   [label="(several Rx desc can be pushed)"];
 * b:>c  [label="Rx desc queue filled IRQ"];
 * c=>>d [label="Driver Rx packet callback"];
 * c<=d  [label="ipc_host_rxdesc_pop()"];
 * d=>d  [label="Rx packet is handed \nover to the OS "];
 * ...   [label="(several Rx desc can be poped)"];
 * ---   [label="Rx buffer request exemple"];
 * b:>c  [label="Low Rx buffer count IRQ"];
 * a<<b  [label="struct ipc_rxbuf"];
 * c=>>d [label="Driver Rx buffer callback"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * d=>c  [label="ipc_host_rxbuf_push()"];
 * ...   [label="(several Rx buffer are pushed)"];
 * @endmsc
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#ifndef __KERNEL__
#include "export/common/co_int.h"
#include "ip/lmac/src/tx/tx_swdesc.h"
#else
#include "export/dbg/dbg_assert.h"
#include "lmac_mac.h"
#endif

/*
 * DEFINES AND MACROS
 ****************************************************************************************
 */
/// Number of IPC TX queues
#define IPC_TXQUEUE_CNT     NX_TXQ_CNT

/// Number of Host buffers available for Tx confirmation (through DMA)
#define IPC_TXCFM_CNT NX_TXDESC_CNT

/// Number of TX DMA and host descriptors available in shared RAM
#define IPC_TXDESC_CNT (NX_TXDESC_CNT + 1)

/// Number of Host buffers available for Data Rx handling (through DMA)
#define IPC_RXBUF_CNT       128

#if (NX_UMAC_PRESENT)
/// Number of shared descriptors available for Data RX handling
#define IPC_RXDESC_CNT      128
#endif //(NX_UMAC_PRESENT)

/// Number of Host buffers available for Radar events handling (through DMA)
#define IPC_RADARBUF_CNT       16

///Number of Host buffers available for unsupported rx vector handling (through DMA)
#define IPC_UNSUPRXVECBUF_CNT   8

/// RX Data buffers size (in bytes)
#define IPC_RXBUF_SIZE 4096

/// Number of Host buffers available for Emb->App MSGs sending (through DMA)
#if NX_UMAC_PRESENT
#define IPC_MSGE2A_BUF_CNT       64
#else
#define IPC_MSGE2A_BUF_CNT       4
#endif

/// Number of Host buffers available for Debug Messages sending (through DMA)
#define IPC_DBGBUF_CNT       32

/// Length used in APP2EMB MSGs structures
#ifdef CFG_RWTL
#define IPC_A2E_MSG_BUF_SIZE    127 // size in 4-byte words
#else
#define IPC_A2E_MSG_BUF_SIZE    127 // size in 4-byte words
#endif

/// Length required in EMB2APP MSGs structures, for non-TL4 case
#if NX_UMAC_PRESENT
#define IPC_E2A_MSG_SIZE_BASE   256 // size in 4-byte words
#else
#define IPC_E2A_MSG_SIZE_BASE   15 // size in 4-byte words
#endif

/// Length used in EMB2APP MSGs structures, including the potential overhead for TL4
#ifdef CFG_RWTL
#define IPC_E2A_MSG_PARAM_SIZE   (IPC_E2A_MSG_SIZE_BASE + (IPC_E2A_MSG_SIZE_BASE / 2))
#else
#define IPC_E2A_MSG_PARAM_SIZE   IPC_E2A_MSG_SIZE_BASE
#endif

/// Debug messages buffers size (in bytes)
#define IPC_DBG_PARAM_SIZE       256

/// Define used for Rx hostbuf validity.
/// This value should appear only when hostbuf was used for a Reception.
#define RX_DMA_OVER_PATTERN 0xAAAAAA00

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

/// Length of the receive vectors, in bytes
#define DMA_HDR_PHYVECT_LEN    36

/// Message struct/ID API version
#define MSG_API_VER     33

/** IPC header structure.  This structure is stored at the beginning of every IPC message.
 * @warning This structure's size must NOT exceed 4 bytes in length.
 */
struct ipc_header
{
    /// IPC message type.
    uint16_t type;
    /// IPC message size in number of bytes.
    uint16_t size;
};

/// Structure of a IPC message
struct ipc_msg_elt
{
    /// Message header (alignment forced on word size, see allocation in shared env).
    struct ipc_header header __ALIGN4;
};

/// Message structure for MSGs from Emb to App
struct ipc_e2a_msg
{
    uint16_t id;                ///< Message id.
    uint16_t dummy_dest_id;     ///< Not used
    uint16_t dummy_src_id;      ///< Not used
    uint16_t param_len;         ///< Parameter embedded struct length.
    uint32_t param[IPC_E2A_MSG_PARAM_SIZE];  ///< Parameter embedded struct. Must be word-aligned.
    uint32_t pattern;           ///< Used to stamp a valid MSG buffer
};

/// Message structure for Debug messages from Emb to App
struct ipc_dbg_msg
{
    uint32_t string[IPC_DBG_PARAM_SIZE/4]; ///< Debug string
    uint32_t pattern;                    ///< Used to stamp a valid buffer
};

/// Message structure for MSGs from App to Emb.
/// Actually a sub-structure will be used when filling the messages.
struct ipc_a2e_msg
{
    uint32_t dummy_word;                ///< used to cope with kernel message structure
    uint32_t msg[IPC_A2E_MSG_BUF_SIZE]; ///< body of the msg
};

#if (NX_UMAC_PRESENT)
/// Information provided by host to indentify RX buffer
struct ipc_shared_rx_buf
{
    /// ptr to hostbuf client (ipc_host client) structure
    uint32_t hostid;
    /// ptr to real hostbuf dma address
    uint32_t dma_addr;
};

/// Information provided by host to indentify RX desc
struct ipc_shared_rx_desc
{
    /// DMA Address
    uint32_t dma_addr;
};
#endif //(NX_UMAC_PRESENT)

/*
 * TYPE and STRUCT DEFINITIONS
 ****************************************************************************************
 */

/// Structure containing FW characteristics for compatibility checking
struct compatibility_tag {
    /// Size of IPC shared memory
    uint16_t ipc_shared_size;
    /// Message struct/ID API version
    uint16_t msg_api;
    /// Version of IPC shared
    uint8_t ipc_shared_version;
    /// Number of host buffers available for Emb->App MSGs sending
    uint8_t msge2a_buf_cnt;
    /// Number of host buffers available for Debug Messages sending
    uint8_t dbgbuf_cnt;
    /// Number of host buffers available for Radar events handling
    uint8_t radarbuf_cnt;
    /// Number of host buffers available for unsupported Rx vectors handling
    uint8_t unsuprxvecbuf_cnt;
    /// Number of shared descriptors available for Data RX handling
    uint8_t rxdesc_cnt;
    /// Number of host buffers available for Data Rx handling
    uint8_t rxbuf_cnt;
    /// Number of descriptors in BK TX queue (power of 2, min 4, max 64)
    uint8_t bk_txq;
    /// Number of descriptors in BE TX queue (power of 2, min 4, max 64)
    uint8_t be_txq;
    /// Number of descriptors in VI TX queue (power of 2, min 4, max 64)
    uint8_t vi_txq;
    /// Number of descriptors in VO TX queue (power of 2, min 4, max 64)
    uint8_t vo_txq;
    /// Number of descriptors in BCN TX queue (power of 2, min 4, max 64)
    uint8_t bcn_txq;
};

// Indexes are defined in the MIB shared structure
/// Structure describing the IPC data shared with the host CPU
struct ipc_shared_env_tag
{
    ///FW characteristics
    volatile struct compatibility_tag comp_info;

    /// Room for MSG to be sent from App to Emb
    volatile struct ipc_a2e_msg msg_a2e_buf;

    /// Room to build the Emb->App MSGs Xferred
    volatile struct    ipc_e2a_msg msg_e2a_buf;
    /// DMA descriptor for Emb->App MSGs Xfers
    volatile struct    dma_desc msg_dma_desc;
    /// Host buffer addresses for Emb->App MSGs DMA Xfers
    volatile uint32_t  msg_e2a_hostbuf_addr [IPC_MSGE2A_BUF_CNT];

    /// Room to build the Emb->App DBG messages
    volatile struct    ipc_dbg_msg dbg_buf;
    /// DMA descriptor for Emb->App DBG messages Xfers
    volatile struct    dma_desc dbg_dma_desc;
    /// Host buffer addresses for DBG messages Xfers
    volatile uint32_t  dbg_hostbuf_addr [IPC_DBGBUF_CNT];

    /// Host buffer address for the debug dump information
    volatile uint32_t  la_dbginfo_addr;

    /// Host buffer address for the TX payload descriptor pattern
    volatile uint32_t  pattern_addr;

    /// Host buffer addresses for the radar events
    volatile uint32_t  radarbuf_hostbuf [IPC_RADARBUF_CNT]; // buffers @ for Radar Events

    /// Host buffer addresses for the unsupported rx vectors
    volatile uint32_t unsuprxvecbuf_hostbuf [IPC_UNSUPRXVECBUF_CNT];

    /// Array of TX DMA descriptors
    volatile struct dma_desc txdmadesc[IPC_TXDESC_CNT];
    /// Host buffer addresses for TX confirmation
    volatile uint32_t txcfm_hostbuf_addr[IPC_TXCFM_CNT];

    #if (NX_UMAC_PRESENT)
    /// RX Descriptors Array
    volatile struct ipc_shared_rx_desc host_rxdesc[IPC_RXDESC_CNT];
    /// RX Buffers Array
    volatile struct ipc_shared_rx_buf  host_rxbuf[IPC_RXBUF_CNT];
    #else
    /// Host buffer addresses for Data Rx
    volatile uint32_t host_rxbuf[IPC_RXBUF_CNT];
    #endif //(NX_UMAC_PRESENT)

    /// Buffered data per STA/TID in number of bytes
    volatile uint32_t buffered[NX_REMOTE_STA_MAX][TID_MAX];
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

/*
 * TYPE and STRUCT DEFINITIONS
 ****************************************************************************************
 */

/// @name APP2EMB IPC interrupt definitions
/// @{

#if RW_MUMIMO_TX_EN
#ifdef CFG_IPC_VER_V10
#error "MU-MIMO cannot be compiled for old IPC"
#endif
#endif

/// IPC TX descriptor interrupt bit
#define IPC_IRQ_A2E_TXDESC          CO_BIT(7)
/// IPC RX buffer allocation interrupt bit
#define IPC_IRQ_A2E_RXBUF_BACK      CO_BIT(5)
/// IPC RX buffer allocation interrupt bit
#define IPC_IRQ_A2E_RXDESC_BACK     CO_BIT(4)
/// IPC APP2EMB message interrupt bit
#define IPC_IRQ_A2E_MSG             CO_BIT(1)
/// IPC APP2EMB debug interrupt bit
#define IPC_IRQ_A2E_DBG             CO_BIT(0)
/// IPC global interrupt mask
#define IPC_IRQ_A2E_ALL             (IPC_IRQ_A2E_TXDESC|IPC_IRQ_A2E_MSG|IPC_IRQ_A2E_DBG)

/// @}

/// @name EMB2APP IPC interrupt definitions
/// @{

/// IPC TX confirmation interrupt bit position
#define IPC_IRQ_E2A_TXCFM_POS   7

#if RW_MUMIMO_TX_EN
#ifdef CFG_IPC_VER_V10
#error "MU-MIMO cannot be compiled for old IPC"
#endif
/// Interrupts bits used
#if RW_USER_MAX > 3
#define IPC_IRQ_E2A_USER_MSK       0xF
#elif RW_USER_MAX > 2
#define IPC_IRQ_E2A_USER_MSK       0x7
#else
#define IPC_IRQ_E2A_USER_MSK       0x3
#endif

/// Offset of the interrupts for AC0
#define IPC_IRQ_E2A_AC0_OFT        7
/// Mask of the interrupts for AC0
#define IPC_IRQ_E2A_AC0_MSK       (IPC_IRQ_E2A_USER_MSK << IPC_IRQ_E2A_AC0_OFT)
/// Offset of the interrupts for AC1
#define IPC_IRQ_E2A_AC1_OFT       (IPC_IRQ_E2A_AC0_OFT + RW_USER_MAX)
/// Mask of the interrupts for AC1
#define IPC_IRQ_E2A_AC1_MSK       (IPC_IRQ_E2A_USER_MSK << IPC_IRQ_E2A_AC1_OFT)
/// Offset of the interrupts for AC2
#define IPC_IRQ_E2A_AC2_OFT       (IPC_IRQ_E2A_AC1_OFT + RW_USER_MAX)
/// Mask of the interrupts for AC2
#define IPC_IRQ_E2A_AC2_MSK       (IPC_IRQ_E2A_USER_MSK << IPC_IRQ_E2A_AC2_OFT)
/// Offset of the interrupts for AC3
#define IPC_IRQ_E2A_AC3_OFT       (IPC_IRQ_E2A_AC2_OFT + RW_USER_MAX)
/// Mask of the interrupts for AC3
#define IPC_IRQ_E2A_AC3_MSK       (IPC_IRQ_E2A_USER_MSK << IPC_IRQ_E2A_AC3_OFT)
/// Offset of the interrupts for BCN
#define IPC_IRQ_E2A_BCN_OFT       (IPC_IRQ_E2A_AC3_OFT + RW_USER_MAX)
/// Mask of the interrupts for BCN
#define IPC_IRQ_E2A_BCN_MSK       CO_BIT(IPC_IRQ_E2A_BCN_OFT)

#define IPC_IRQ_E2A_AC_TXCFM     (IPC_IRQ_E2A_AC0_MSK | IPC_IRQ_E2A_AC1_MSK | \
                                   IPC_IRQ_E2A_AC2_MSK | IPC_IRQ_E2A_AC3_MSK)

/// Interrupts bits used for the TX descriptors of the BCN queue
#if NX_UMAC_PRESENT && NX_BEACONING
#define IPC_IRQ_E2A_BCN_TXCFM      (0x01 << IPC_IRQ_E2A_BCN_OFT)
#else
#define IPC_IRQ_E2A_BCN_TXCFM      0
#endif

/// IPC TX descriptor interrupt mask
#define IPC_IRQ_E2A_TXCFM          (IPC_IRQ_E2A_AC_TXCFM | IPC_IRQ_E2A_BCN_TXCFM)

#else

#if NX_UMAC_PRESENT && NX_BEACONING
#define IPC_IRQ_E2A_TXCFM          ((1 << 5) - 1 ) << IPC_IRQ_E2A_TXCFM_POS
#else
#define IPC_IRQ_E2A_TXCFM          ((1 << 4) - 1 ) << IPC_IRQ_E2A_TXCFM_POS
#endif

#endif /* RW_MUMIMO_TX_EN */


/// IPC unsupported rx vector interrupt
#define IPC_IRQ_E2A_UNSUP_RX_VEC CO_BIT(5)
/// IPC radar interrupt
#define IPC_IRQ_E2A_RADAR       CO_BIT(4)
/// IPC RX descriptor interrupt
#define IPC_IRQ_E2A_RXDESC      CO_BIT(3)
/// IPC APP2EMB message acknowledgment interrupt
#define IPC_IRQ_E2A_MSG_ACK     CO_BIT(2)
/// IPC EMB2APP message interrupt
#define IPC_IRQ_E2A_MSG         CO_BIT(1)
/// IPC EMB2APP debug interrupt
#define IPC_IRQ_E2A_DBG         CO_BIT(0)
/// IPC EMB2APP global interrupt mask
#define IPC_IRQ_E2A_ALL         ( IPC_IRQ_E2A_TXCFM         \
                                | IPC_IRQ_E2A_RXDESC        \
                                | IPC_IRQ_E2A_MSG_ACK       \
                                | IPC_IRQ_E2A_MSG           \
                                | IPC_IRQ_E2A_DBG           \
                                | IPC_IRQ_E2A_TBTT_PRIM     \
                                | IPC_IRQ_E2A_TBTT_SEC      \
                                | IPC_IRQ_E2A_RADAR         \
                                | IPC_IRQ_E2A_UNSUP_RX_VEC)

/// @}

/// @}

#endif // _IPC_SHARED_H_

