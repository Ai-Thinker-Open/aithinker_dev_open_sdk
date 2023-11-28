/**
 ****************************************************************************************
 *
 * @file scpps_task.h
 *
 * @brief Header file - Scan Parameters Profile Server Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _SCPPS_TASK_H_
#define _SCPPS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup SCPPSTASK Task
 * @ingroup SCPPS
 * @brief Scan Parameters Profile Task.
 *
 * The SCPPS_TASK is responsible for handling the messages coming in and out of the
 * @ref SCPPS block of the BLE Host.
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


/// Messages for Scan Parameters Profile Server
/*@TRACE*/
enum scpps_msg_id
{
    /// Update bond data of the Scan Parameters Profile Server for a specific connection - Request
    SCPPS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_SCPPS),
    /// Update bond data of the Scan Parameters Profile Server for a specific connection - Response
    SCPPS_ENABLE_RSP,

    /// Indicate to APP that the Scan Interval Window value has been written by Client
    SCPPS_SCAN_INTV_WD_IND,
    /// Indicate to APP that the Scan Refresh Notification Configuration has been changed
    SCPPS_SCAN_REFRESH_NTF_CFG_IND,

    /// Notify client about new Scan Refresh Value
    SCPPS_SCAN_REFRESH_SEND_CMD,
    /// Complete Event Information
    SCPPS_CMP_EVT,
};

/// Features Flag Masks
enum scpps_features
{
    /// Scan Refresh Characteristic is not supported
    SCPPS_SCAN_REFRESH_NOT_SUP = 0,
    /// Scan Refresh Characteristic is supported
    SCPPS_SCAN_REFRESH_SUP,
};

/// Operation codes
enum scpps_op_codes
{
    /// Database Creation Procedure
    SCPPS_RESERVED_OP_CODE = 0,

    /// Notify Scan Refresh Operation Code
    SCPPS_SCAN_REFRESH_SEND_CMD_OP_CODE = 1,
};

/*
 * APIs Structures
 ****************************************************************************************
 */

/// Parameters for the database creation
struct scpps_db_cfg
{
    /// Optional features to add - @see enum scpps_features
    uint8_t features;
};

/// Parameters of the @ref SCPPS_ENABLE_REQ message
struct scpps_enable_req
{
    /// Connection index
    uint8_t conidx;
    /// Scan Refresh Notification Configurations
    uint8_t ntf_cfg;
};

/// Parameters of the @ref SCPPS_ENABLE_RSP message
struct scpps_enable_rsp
{
    /// Connection index
    uint8_t conidx;
    /// Status
    uint8_t status;
};

/// Parameters of the @ref SCPPS_SCAN_INTV_WD_IND message
struct scpps_scan_intv_wd_ind
{
    /// Connection index
    uint8_t conidx;
    /// Scan Interval Window
    struct scpp_scan_intv_wd scan_intv_wd;
};

/// Parameters of the @ref SCPPS_SCAN_REFRESH_NTF_CFG_IND message
struct scpps_scan_refresh_ntf_cfg_ind
{
    /// Connection index
    uint8_t conidx;
    /// Scan Refresh Notification Configuration
    uint8_t ntf_cfg;
};

/// Parameters of the @ref SCPPS_SCAN_REFRESH_SEND_CMD message
struct scpps_scan_refresh_send_cmd
{
    /// Connection index
    uint8_t conidx;
};

/// Parameters of the @ref SCPPS_CMP_EVT message
struct scpps_cmp_evt
{
    /// Connection index
    uint8_t conidx;
    /// Connection index
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} SCPPSTASK

#endif /* _SCPPS_TASK_H_ */
