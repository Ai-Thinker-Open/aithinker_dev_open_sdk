/**
 ****************************************************************************************
 *
 * @file l2cc_task.h
 *
 * @brief Header file - L2CCTASK.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef L2CC_TASK_H_
#define L2CC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup L2CCTASK Task
 * @ingroup L2CC
 * @brief Handles ALL messages to/from L2CC block.
 *
 * The L2CC task is responsible for L2CAP attribute and security block handling.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_task.h" // Task definitions
#include "ln_compiler.h"

#include <stdbool.h>
#include "ke_msg.h"

/*
 * MACRO DEFINITIONS
 ****************************************************************************************
 */

/// Number of L2CC Deprecated messages
#define L2CC_NB_DEPRECATED_MSG   (4)

/*
 * STATES
 ****************************************************************************************
 */

/*
 * MESSAGES
 ****************************************************************************************
 */

/// Message API of the L2CC task
/*@TRACE*/
enum l2cc_msg_id
{
    /// L2CAP Operation completed event
    L2CC_CMP_EVT               = TASK_FIRST_MSG(TASK_ID_L2CC) + L2CC_NB_DEPRECATED_MSG,//!< L2CC_CMP_EVT

    /* ************* LE Credit Based API    ************* */
    /// LE credit based connection request
    L2CC_LECB_CONNECT_CMD,                                                             //!< L2CC_LECB_CONNECT_CMD
    /// LE credit based connection request indication
    L2CC_LECB_CONNECT_REQ_IND,                                                         //!< L2CC_LECB_CONNECT_REQ_IND
    /// LE credit based connection request confirmation
    L2CC_LECB_CONNECT_CFM,                                                             //!< L2CC_LECB_CONNECT_CFM
    /// LE credit based connection indication
    L2CC_LECB_CONNECT_IND,                                                             //!< L2CC_LECB_CONNECT_IND
    /// LE credit based disconnect request
    L2CC_LECB_DISCONNECT_CMD,                                                          //!< L2CC_LECB_DISCONNECT_CMD
    /// LE credit based disconnect indication
    L2CC_LECB_DISCONNECT_IND,                                                          //!< L2CC_LECB_DISCONNECT_IND
    /// LE credit based credit addition
    L2CC_LECB_ADD_CMD,                                                                 //!< L2CC_LECB_ADD_CMD
    /// LE credit based credit addition indication
    L2CC_LECB_ADD_IND,                                                                 //!< L2CC_LECB_ADD_IND

    /// Send data over an LE Credit Based Connection
    L2CC_LECB_SDU_SEND_CMD,                                                            //!< L2CC_LECB_SDU_SEND_CMD
    /// Inform that a data packet has been received from a LE Credit Based connection.
    L2CC_LECB_SDU_RECV_IND,                                                            //!< L2CC_LECB_SDU_RECV_IND
    /// Indication to the task that sends the unknown message
    L2CC_UNKNOWN_MSG_IND,                                                              //!< L2CC_UNKNOWN_MSG_IND

    /* ******************* Debug API    ******************* */
    /// Send Debug PDU data
    L2CC_DBG_PDU_SEND_CMD,                                                             //!< L2CC_DBG_PDU_SEND_CMD
    /// Debug PDU data received
    L2CC_DBG_PDU_RECV_IND,                                                             //!< L2CC_DBG_PDU_RECV_IND


    /* **************** PDU Internal API ****************** */
    /// Send Legacy PDU data
    L2CC_PDU_SEND_CMD,                                                                 //!< L2CC_PDU_SEND_CMD
    /// Legacy PDU data received
    L2CC_PDU_RECV_IND,                                                                 //!< L2CC_PDU_RECV_IND

    /// Timeout indication for a transaction on signaling channel
    L2CC_SIGNALING_TRANS_TO_IND,                                                       //!< L2CC_SIGNALING_TRANS_TO_IND
};


/// request operation type - application interface
enum l2cc_operation
{
    /*                 Operation Flags                  */
    /* No Operation (if nothing has been requested)     */
    /* ************************************************ */
    /// No operation
    L2CC_NO_OP                                    = 0x00,

    /* LE Credit Based                                  */
    /* ************************************************ */
    /// LE credit based connection request
    L2CC_LECB_CONNECT,
    /// LE credit based disconnection request
    L2CC_LECB_DISCONNECT,
    /// LE credit addition request
    L2CC_LECB_CREDIT_ADD,
    /// Send SDU
    L2CC_LECB_SDU_SEND,

    /* Debug PDU Transmission (internal)                */
    /* ************************************************ */
    /// Send a Debug PDU
    L2CC_DBG_PDU_SEND,

    /* PDU Transmission (internal)                      */
    /* ************************************************ */
    /// Send internal LE Legacy PDU
    L2CC_PDU_SEND,
};


/// Default L2Cap SDU definition
/*@TRACE*/
struct l2cc_sdu
{
    /// Channel Identifier
    uint16_t cid;
    /// Number of credit used
    uint16_t credit;
    /// SDU Data length
    uint16_t length;
    /// data
    uint8_t data[__ARRAY_EMPTY];
};


/// Operation completed event
/*@TRACE*/
struct l2cc_cmp_evt
{
    /// L2CC request type (@see enum l2cc_operation)
    uint8_t operation;
    /// Status of request.
    uint8_t status;
    /// Channel ID
    uint16_t cid;
    /// Number of peer credit used - only relevant for LECB
    uint16_t credit;
};

/// LE credit based connection request
/*@TRACE*/
struct l2cc_lecb_connect_cmd
{
    /// L2CC request type:
    /// - L2CC_LECB_CONNECT: LE credit connection
    uint8_t operation;
    /// parameter used for internal usage
    uint8_t  pkt_id;
    /// LE Protocol/Service Multiplexer
    uint16_t le_psm;
    /// Local Channel identifier (0: automatically allocate a free channel)
    uint16_t local_cid;
    /// Credit allocated for the LE Credit Based Connection
    /// Shall be at least: floor(((SDU + 2) + (MPS - 1)) / MPS) + 1
    /// To be sure that 1 SDU can be fully received without requesting credits to application
    uint16_t local_credit;
    /// Maximum SDU size - Shall not exceed device MTU
    uint16_t local_mtu;
    /// Maximum Packet size - Shall not exceed device MPS
    uint16_t local_mps;
};

/// LE credit based connection request indication
/*@TRACE*/
struct l2cc_lecb_connect_req_ind
{
    /// LE Protocol/Service Multiplexer
    uint16_t le_psm;
    /// Peer Channel identifier
    uint16_t peer_cid;
    /// Maximum SDU size
    uint16_t peer_mtu;
    /// Maximum Packet size
    uint16_t peer_mps;
};

/// LE credit based connection request confirmation
/*@TRACE*/
struct l2cc_lecb_connect_cfm
{
    /// Peer Channel identifier
    uint16_t peer_cid;
    /// True to accept the incoming connection, False else
    bool accept;
    /// Local Channel identifier (0: automatically allocate a free channel)
    uint16_t local_cid;
    /// Credit allocated for the LE Credit Based Connection
    /// Shall be at least: floor(((SDU + 2) + (MPS - 1)) / MPS) + 1
    /// To be sure that 1 SDU can be fully received without requesting credits to application
    uint16_t local_credit;
    /// Maximum SDU size - Shall not exceed device MTU
    uint16_t local_mtu;
    /// Maximum Packet size - Shall not exceed device MPS
    uint16_t local_mps;
};

/// LE credit based connection indication
/*@TRACE*/
struct l2cc_lecb_connect_ind
{
    /// Status
    uint8_t  status;
    /// LE Protocol/Service Multiplexer
    uint16_t le_psm;
    /// Local Channel identifier
    uint16_t local_cid;
    ///  Destination Credit for the LE Credit Based Connection
    uint16_t peer_credit;
    /// Maximum SDU size
    uint16_t peer_mtu;
    /// Maximum Packet size
    uint16_t peer_mps;
};

/// LE credit based disconnect request
/*@TRACE*/
struct l2cc_lecb_disconnect_cmd
{
    /// L2CC request type:
    /// - L2CC_LECB_DISCONNECT: LE credit disconnection
    uint8_t  operation;
    /// parameter used for internal usage
    uint8_t  pkt_id;
    /// Local Channel identifier
    uint16_t local_cid;
};

/// LE credit based disconnect indication
/*@TRACE*/
struct l2cc_lecb_disconnect_ind
{
    /// Local Channel identifier
    uint16_t local_cid;
    /// Reason
    uint8_t reason;
};

/// LE credit based credit addition
/*@TRACE*/
struct l2cc_lecb_add_cmd
{
    /// L2CC request type:
    /// - L2CC_LECB_CREDIT_ADD: LE credit addition
    uint8_t  operation;
    /// parameter used for internal usage
    uint8_t  pkt_id;
    /// Local Channel identifier
    uint16_t local_cid;
    /// Credit added locally for channel identifier
    uint16_t credit;
};

///LE credit based credit addition indication
/*@TRACE*/
struct l2cc_lecb_add_ind
{
    /// Local Channel identifier
    uint16_t local_cid;
    /// Destination added credit (relative value)
    uint16_t peer_added_credit;
};

/// Send data over an LE Credit Based Connection
/*@TRACE*/
struct l2cc_lecb_sdu_send_cmd
{
    /// L2CC request type (@see enum l2cc_operation):
    /// - L2CC_LECB_SDU_SEND: Send a SDU
    uint8_t operation;
    /// offset value information - for internal use only
    uint16_t offset;
    /// SDU information
    struct l2cc_sdu sdu;
};

/// Inform that a data packet has been received from a LE Credit Based connection.
/*@TRACE*/
struct l2cc_lecb_sdu_recv_ind
{
    /// Status information
    uint8_t status;
    /// offset value information
    uint16_t offset;
    /// SDU information
    struct l2cc_sdu sdu;
};

/// Indicate that an unknown message has been received
/*@TRACE*/
struct l2cc_unknown_msg_ind
{
    /// Unknown message id
    ke_msg_id_t unknown_msg_id;
};

/// @} L2CCTASK

#endif // L2CC_TASK_H_
