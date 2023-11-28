/**
 ****************************************************************************************
 *
 * @file scppc_task.h
 *
 * @brief Header file - Scan Parameters Profile Client Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _SCPPC_TASK_H_
#define _SCPPC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SCPPCTASK Scan Parameters Profile Client Task
 * @ingroup SCPPC
 * @brief Scan Parameters Profile Client Task
 *
 * The SCPPCTASK is responsible for handling the messages coming in and out of the
 * @ref SCPPC monitor block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "scpp_common.h"
#include "prf_types.h"
#include "rwip_task.h" // Task definitions
/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum scppc_msg_id
{
    /// Start the Scan Parameters profile Client Role - at connection
    SCPPC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_SCPPC),
    ///Confirm that cfg connection has finished with discovery results, or that normal cnx started
    SCPPC_ENABLE_RSP,

    /// Write Scan Interval Window Characteristic Value Request - Write Without Response
    SCPPC_SCAN_INTV_WD_WR_CMD,
    /// Read Scan Refresh Notification Configuration Value request
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD,
    /// Inform APP that Scan Refresh Notification Configuration has been read
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_IND,
    /// Write Scan Refresh Notification Configuration Value request
    SCPPC_SCAN_REFRESH_NTF_CFG_CMD,
    /// Inform APP that peer service request a refresh of Scan Parameters values
    SCPPC_SCAN_REFRESH_IND,
    /// Complete Event Information
    SCPPC_CMP_EVT,
};

/// Characteristics
enum scppc_char
{
    /// Scan Interval Window
    SCPPC_CHAR_SCAN_INTV_WD,
    /// Scan Refresh
    SCPPC_CHAR_SCAN_REFRESH,

    SCPPC_CHAR_MAX,
};


/// Characteristic descriptors
enum scppc_desc
{
    /// Scan Refresh Client Config
    SCPPC_DESC_SCAN_REFRESH_CFG,

    SCPPC_DESC_MAX,
};

/// Operation codes
enum scppc_op_codes
{
    /// Database Creation Procedure
    SCPPC_RESERVED_OP_CODE = 0,

    /// Configure CCC Operation Code
    SCPPC_SCAN_INTV_WD_WR_CMD_OP_CODE = 1,

    /// Read Scan Refresh CCC Operation Code
    SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD_OP_CODE = 2,

    /// Configure Scan Refresh CCC Operation Code
    SCPPC_SCAN_REFRESH_NTF_CFG_CMD_OP_CODE = 3,
};
/*
 * APIs Structure
 ****************************************************************************************
 */

///Structure containing the characteristics handles, value handles and descriptors
struct scps_content
{
    /// service info
    struct prf_svc svc;

    /// characteristic info:
    ///  - Scan Interval Window
    ///     - Scan Refresh
    struct prf_char_inf chars[SCPPC_CHAR_MAX];

    /// Descriptor handles:
    ///  - Scan Refresh Client Cfg
    struct prf_char_desc_inf descs[SCPPC_DESC_MAX];
};

/// Parameters of the @ref SCPPC_ENABLE_REQ message
struct scppc_enable_req
{
    ///Connection type
    uint8_t con_type;
    ///Existing handle values scps
    struct scps_content scps;
};

/// Parameters of the @ref SCPPC_ENABLE_RSP message
struct scppc_enable_rsp
{
    ///status
    uint8_t status;
    ///Existing handle values scps
    struct scps_content scps;
};

///Parameters of the @ref SCPPC_SCAN_INTV_WD_WR_CMD message
struct scppc_scan_intv_wd_wr_cmd
{
    ///Scan Interval Window
    struct scpp_scan_intv_wd scan_intv_wd;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_RD_IND message
struct scppc_scan_refresh_ntf_cfg_rd_ind
{
    ///Notification Configuration Value
    uint16_t ntf_cfg;
};

///Parameters of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_CMD message
struct scppc_scan_refresh_ntf_cfg_cmd
{
    ///Notification Configuration
    uint16_t ntf_cfg;
};

///Parameters of the @ref SCPPC_CMP_EVT message
struct scppc_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} SCPPCTASK

#endif /* _SCPPC_TASK_H_ */
