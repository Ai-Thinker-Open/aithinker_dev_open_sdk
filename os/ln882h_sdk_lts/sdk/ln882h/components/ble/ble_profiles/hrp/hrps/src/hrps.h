/**
 ****************************************************************************************
 *
 * @file hrps.h
 *
 * @brief Header file - Heart Rate Profile Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _HRPS_H_
#define _HRPS_H_

/**
 ****************************************************************************************
 * @addtogroup HRPS Heart Rate Profile Sensor
 * @ingroup HRP
 * @brief Heart Rate Profile Sensor
 *
 * Heart Rate sensor (HRS) profile provides functionalities to upper layer module
 * application. The device using this profile takes the role of Heart Rate sensor.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Notify peer device during Heart Rate measurement (from APP)
 *  - Indicate measurements performed to peer device (from APP)
 *
 * Profile didn't manages multiple users configuration and storage of offline measurements.
 * This must be handled by application.
 *
 * Heart Rate Profile Sensor. (HRPS): A HRPS (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can perform Heart Rate
 * measurement and notify about on-going measurement and indicate final result to a peer
 * BLE device.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_HR_SENSOR)
#include "hrp_common.h"
#include "prf_types.h"
#include "prf.h"
#include "hrps_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */
///Maximum number of Heart Rate task instances
#define HRPS_IDX_MAX     0x01

#define HRPS_HT_MEAS_MAX_LEN            (5 + (2 * HRS_MAX_RR_INTERVAL))

#define HRPS_MANDATORY_MASK             (0x0F)
#define HRPS_BODY_SENSOR_LOC_MASK       (0x30)
#define HRPS_HR_CTNL_PT_MASK            (0xC0)

#define HRP_PRF_CFG_PERFORMED_OK        (0x80)

/*
 * MACROS
 ****************************************************************************************
 */

#define HRPS_IS_SUPPORTED(features, mask) ((features & mask) == mask)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// Possible states of the HRPS task
enum
{
    /// Idle state
    HRPS_IDLE,
    /// Connected state
    HRPS_BUSY,

    /// Number of defined states.
    HRPS_STATE_MAX,
};

/// Attributes State Machine
enum
{
    HRS_IDX_SVC,

    HRS_IDX_HR_MEAS_CHAR,
    HRS_IDX_HR_MEAS_VAL,
    HRS_IDX_HR_MEAS_NTF_CFG,

    HRS_IDX_BODY_SENSOR_LOC_CHAR,
    HRS_IDX_BODY_SENSOR_LOC_VAL,

    HRS_IDX_HR_CTNL_PT_CHAR,
    HRS_IDX_HR_CTNL_PT_VAL,

    HRS_IDX_NB,
};

enum
{
    HRPS_HR_MEAS_CHAR,
    HRPS_BODY_SENSOR_LOC_CHAR,
    HRPS_HR_CTNL_PT_CHAR,

    HRPS_CHAR_MAX,
};

/// Feature bit field
enum hrps_feat_bf
{
    /// Body Sensor Location Feature Supported
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS = 0,
    HRPS_BODY_SENSOR_LOC_CHAR_SUP_BIT = CO_BIT(HRPS_BODY_SENSOR_LOC_CHAR_SUP_POS),

    /// Energy Expanded Feature Supported
    HRPS_ENGY_EXP_FEAT_SUP_POS = 1,
    HRPS_ENGY_EXP_FEAT_SUP_BIT = CO_BIT(HRPS_ENGY_EXP_FEAT_SUP_POS),

    /// Heart Rate Measurement Notification Supported
    HRPS_HR_MEAS_NTF_CFG_POS = 2,
    HRPS_HR_MEAS_NTF_CFG_BIT = CO_BIT(HRPS_HR_MEAS_NTF_CFG_POS),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// ongoing operation information
struct hrps_op
{
     /// Cursor on connection
     uint8_t cursor;
     /// Packed notification/indication data size
     uint8_t length;
     /// Packed notification/indication data
     uint8_t data[HRPS_HT_MEAS_MAX_LEN];
};

/// Heart Rate Profile Sensor environment variable
struct hrps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Operation pointer used to generate notifications
    struct hrps_op *p_operation;
    ///Service Start Handle
    uint16_t shdl;
    ///Database configuration
    uint8_t features;
    /// Sensor location
    uint8_t sensor_location;
    /// Measurement notification config
    uint16_t hr_meas_ntf[BLE_CONNECTION_MAX];

    /// State of different task instances
    ke_state_t state[HRPS_IDX_MAX];
};


/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve HRP service profile interface
 *
 * @return HRP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* hrps_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send a HRPS_CMP_EVT message to the application.
 *
 * @param[in] p_hrps_env    HRP Sensor environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Code of the completed operation
 * @param[in] status        Status of the request
 *****************************************************************************************
 */
void hrps_send_cmp_evt(struct hrps_env_tag *p_hrps_env, uint8_t conidx, uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * @brief Pack Heart Rate measurement value
 *
 * @param[out] p_packed_hr  Pointer to Heart Rate measurement structure destinationHeart Rate measurement value
 * @param[in] p_meas_val    Heart Rate measurement. Pointer of the packed data of Heart
 *                          Rate Measurement
 *                          information
 ****************************************************************************************
 */
uint8_t hrps_pack_meas_value(uint8_t *p_packed_hr, const struct hrs_hr_meas *p_meas_val);

/**
 ****************************************************************************************
 * @brief  This function fully manage notification of Heart Rate Measurement
 *         to peer(s) device(s) according to on-going operation requested by application
 *
 * @param[in] conidx    Connection index
 ****************************************************************************************
 */
void hrps_exe_operation(uint8_t conidx);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void hrps_task_init(struct ke_task_desc *p_task_desc);

#endif /* #if (BLE_HR_SENSOR) */

/// @} HRPS

#endif /* _HRPS_H_ */
