/**
 ****************************************************************************************
 *
 * @file bcss_task.h
 *
 * @brief Header file - Body Composition Service Sensor/Server Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSS_TASK_H_
#define _BCSS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup BCSSTASK Task
 * @ingroup BCSS
 * @brief Body Composition Service Task.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bcs_common.h"
#include <stdint.h>
#include "rwip_task.h" // Task definitions

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Body Composition Service Sensor
/*@TRACE*/
enum bcss_msg_id
{
    /// Enable the BCS Sensor task for a connection
    BCSS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_BCSS),
    /// Enable the BCS Sensor task for a connection
    BCSS_ENABLE_RSP,
    /// Indicate a new IND configuration to the application
    /// Send CCC descriptor write update to the APP
    BCSS_MEAS_CCC_IND,

    /// Make BCS Sensor indicate it's peer device with the measurement
    BCSS_MEAS_INDICATE_CMD,

    /// Send a complete event status to the application
    BCSS_CMP_EVT,
};

/// Operation codes
enum bcss_op_codes
{
    /// Indicate Measurement Operation Code
    BCSS_MEAS_INDICATE_CMD_OP_CODE = 1,
};
/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct bcss_db_cfg
{
    /// Body Composition service Feature Value
    uint32_t feature;
    /// Secondary service value
    uint8_t secondary_service;
};

/// Parameters of the @ref BCSS_ENABLE_REQ message
struct bcss_enable_req
{
    /// CCC for the current connection
    uint16_t ind_cfg;
};

/// Parameters of the @ref BCSS_ENABLE_RSP message
struct bcss_enable_rsp
{
    /// Status
    uint8_t status;
};

/// Parameters of the @ref BCSS_MEAS_INDICATE_CMD message
struct bcss_meas_indicate_cmd
{
    /// Presence of optional fields and measurement units
    uint16_t flags;
    /// Body fat percentage value
    uint16_t body_fat_percent;
    /// Time stamp value
    struct prf_date_time time_stamp;
    /// User ID value
    uint8_t user_id;
    /// Basal Metabolism value
    uint16_t basal_metab;
    /// Muscle Percentage value
    uint16_t muscle_percent;
    /// Muscle Mass value
    uint16_t muscle_mass;
    /// Fat Free Mass value
    uint16_t fat_free_mass;
    /// Soft Lean Mass value
    uint16_t soft_lean_mass;
    /// Body Water Mass value
    uint16_t body_water_mass;
    /// Impedance Value
    uint16_t impedance;
    /// Weight value
    uint16_t weight;
    /// Height value
    uint16_t height;
};

/// Parameters of the @ref BCSS_MEAS_CCC_IND message
struct bcss_meas_ccc_ind
{
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};

/// Parameters of the @ref BCSS_CMP_EVT message
struct bcss_cmp_evt
{
	/// operation
    uint8_t operation;
    /// Operation Status
    uint8_t  status;
};

/// @} BCSSTASK

#endif //(_BCSS_TASK_H_)
