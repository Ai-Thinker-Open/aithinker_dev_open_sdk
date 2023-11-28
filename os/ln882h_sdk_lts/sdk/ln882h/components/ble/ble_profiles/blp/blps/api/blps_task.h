/**
 ****************************************************************************************
 *
 * @file blps_task.h
 *
 * @brief Header file - Blood Pressure Profile Sensor Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _BLPS_TASK_H_
#define _BLPS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup BLPSTASK Task
 * @ingroup BLPS
 * @brief Blood Pressure Profile Task.
 *
 * The BLPSTASK is responsible for handling the messages coming in and out of the
 * @ref BLPS collector block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "blp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

enum
{
    /// measurement sent by profile
    BLPS_BP_MEAS_SEND,
    /// peer device confirm reception
    BLPS_CENTRAL_IND_CFM,
};

/// Messages for Blood Pressure Profile Sensor
/*@TRACE*/
enum blps_msg_id
{
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_BLPS),
    /// Start the Blood Pressure Profile Sensor - at connection
    BLPS_ENABLE_RSP,

    /// Send blood pressure measurement value from APP
    BLPS_MEAS_SEND_CMD,

    /// Inform APP of new configuration value
    BLPS_CFG_INDNTF_IND,

    /// Complete Event Information
    BLPS_CMP_EVT,
};

/// Operation codes
enum blps_op_codes
{
    /// Database Creation Procedure
    BLPS_RESERVED_OP_CODE = 0,

    /// Indicate Measurement Operation Code
    BLPS_MEAS_SEND_CMD_OP_CODE = 1,
};

/// Parameters of the @ref BLPS_ENABLE_REQ message
struct blps_enable_req
{
    ///Connection index
    uint8_t conidx;

    /// Blood Pressure indication configuration
    uint16_t bp_meas_ind_en;
    /// Intermediate Cuff Pressure Notification configuration
    uint16_t interm_cp_ntf_en;
};

/// Parameters of the @ref BLPS_ENABLE_RSP message
struct blps_enable_rsp
{
    ///Connection index
    uint8_t conidx;
    ///Status
    uint8_t status;
};

///Parameters of the @ref BLPS_CFG_INDNTF_IND message
struct blps_cfg_indntf_ind
{
    ///Connection index
    uint8_t conidx;
    ///Own code for differentiating between Blood Pressure Measurement, and Intermediate
    /// Cuff Pressure Measurement characteristics
    uint8_t char_code;
    ///Stop/notify/indicate value to configure into the peer characteristic
    uint16_t cfg_val;
};

/////Parameters of the @ref BLPS_MEAS_SEND_CMD message
struct blps_meas_send_cmd
{
    ///Connection index
    uint8_t conidx;
    /// Flag indicating if it is a intermediary cuff pressure measurement (0) or
    /// stable blood pressure measurement (1).
    uint8_t flag_interm_cp;
    ///Blood Pressure measurement
    struct bps_bp_meas meas_val;
};

///Parameters of the @ref BLPS_CMP_EVT message
struct blps_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Status
    uint8_t status;
};

///Parameters of the @ref BLPS_CREATE_DB_REQ message
struct blps_db_cfg
{
    /// Supported features
    uint16_t features;
    /// Profile Configuration
    uint8_t prfl_cfg;
};

/// @} BLPSTASK

#endif /* _BLPS_TASK_H_ */
