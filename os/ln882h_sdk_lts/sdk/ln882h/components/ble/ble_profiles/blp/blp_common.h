/**
 ****************************************************************************************
 *
 * @file blp_common.h
 *
 * @brief Header File - Blood Pressure Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _BLP_COMMON_H_
#define _BLP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup BLP Blood Pressure Profile
 * @ingroup PROFILE
 * @brief Blood Pressure Profile
 *
 * The BLP module is the responsible block for implementing the Blood Pressure Profile
 * functionalities in the BLE Host.
 *
 * The Blood Pressure Profile defines the functionality required in a device that allows
 * the user (Collector device) to configure and recover blood pressure measurements from
 * a blood pressure device.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "prf_types.h"
#include <stdint.h>
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// BLPC codes for the 2 possible client configuration characteristic descriptors determination in BPS
enum blp_ccc_code
{
    ///Blood Pressure Measurement
    BPS_BP_MEAS_CODE = 0x01,
    ///Intermediate Cuff Pressure Measurement
    BPS_INTERM_CP_CODE,
};

/// Blood Pressure Measurement Flags bit field values
enum blp_meas_bf
{
    /// Blood Pressure Units Flag
    /// 0 : Blood pressure for Systolic, Diastolic and MAP in units of mmHg
    /// 1 : Blood pressure for Systolic, Diastolic and MAP in units of kPa
    BPS_MEAS_FLAG_BP_UNITS_POS = 0,
    BPS_MEAS_FLAG_BP_UNITS_BIT = CO_BIT(BPS_MEAS_FLAG_BP_UNITS_POS),

    /// Time Stamp Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_FLAG_TIME_STAMP_POS = 1,
    BPS_MEAS_FLAG_TIME_STAMP_BIT = CO_BIT(BPS_MEAS_FLAG_TIME_STAMP_POS),

    /// Pulse Rate Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_PULSE_RATE_POS = 2,
    BPS_MEAS_PULSE_RATE_BIT = CO_BIT(BPS_MEAS_PULSE_RATE_POS),

    /// User ID Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_USER_ID_POS = 3,
    BPS_MEAS_USER_ID_BIT = CO_BIT(BPS_MEAS_USER_ID_POS),

    /// Measurement Status Flag
    /// 0 : not present
    /// 1 : present
    BPS_MEAS_MEAS_STATUS_POS = 4,
    BPS_MEAS_MEAS_STATUS_BIT = CO_BIT(BPS_MEAS_MEAS_STATUS_POS),

    // Bit 5 - 7 RFU
};

/// Blood Pressure Measurement Status Flags field bit values
enum blp_meas_status_bf
{
    /// Body Movement Detection Flag
    /// 0 : No body movement
    /// 1 : Body movement during measurement
    BPS_STATUS_MVMT_DETECT_POS = 0,
    BPS_STATUS_MVMT_DETECT_BIT = CO_BIT(BPS_STATUS_MVMT_DETECT_POS),

    /// Cuff Fit Detection Flag
    /// 0 : Cuff fits properly
    /// 1 : Cuff too loose
    BPS_STATUS_CUFF_FIT_DETECT_POS = 1,
    BPS_STATUS_CUFF_FIT_DETECT_BIT = CO_BIT(BPS_STATUS_CUFF_FIT_DETECT_POS),

    /// Irregular Pulse Detection Flag
    /// 0 : No irregular pulse detected
    /// 1 : Irregular pulse detected
    BPS_STATUS_IRREGULAR_PULSE_DETECT_POS = 2,
    BPS_STATUS_IRREGULAR_PULSE_DETECT_BIT = CO_BIT(BPS_STATUS_IRREGULAR_PULSE_DETECT_POS),

    /// Pulse Rate Range Detection Flags
    /// value 0 : Pulse rate is within the range
    /// value 1 : Pulse rate exceeds upper limit
    /// value 2 : Pulse rate is less than lower limit
    BPS_STATUS_PR_RANGE_DETECT_LSB_POS = 3,
    BPS_STATUS_PR_RANGE_DETECT_LSB_BIT = CO_BIT(BPS_STATUS_PR_RANGE_DETECT_LSB_POS),

    BPS_STATUS_PR_RANGE_DETECT_MSB_POS = 4,
    BPS_STATUS_PR_RANGE_DETECT_MSB_BIT = CO_BIT(BPS_STATUS_PR_RANGE_DETECT_MSB_POS),

    /// Measurement Position Detection Flag
    /// 0 : Proper measurement position
    /// 1 : Improper measurement position
    BPS_STATUS_MEAS_POS_DETECT_POS = 5,
    BPS_STATUS_MEAS_POS_DETECT_BIT = CO_BIT(BPS_STATUS_MEAS_POS_DETECT_POS),

    // Bit 6 - 15 RFU
};

/// Blood Pressure Feature Flags field bit values
enum blp_feat_flags_bf
{
    ///Body Movement Detection Support bit
    BPS_F_BODY_MVMT_DETECT_SUP_POS = 0,
    BPS_F_BODY_MVMT_DETECT_SUP_BIT = CO_BIT(BPS_F_BODY_MVMT_DETECT_SUP_POS),

    /// Cuff Fit Detection Support bit
    BPS_F_CUFF_FIT_DETECT_SUP_POS = 1,
    BPS_F_CUFF_FIT_DETECT_SUP_BIT = CO_BIT(BPS_F_CUFF_FIT_DETECT_SUP_POS),

    /// Irregular Pulse Detection Support bit
    BPS_F_IRREGULAR_PULSE_DETECT_SUP_POS = 2,
    BPS_F_IRREGULAR_PULSE_DETECT_SUP_BIT = CO_BIT(BPS_F_IRREGULAR_PULSE_DETECT_SUP_POS),

    /// Pulse Rate Range Detection Support bit
    BPS_F_PULSE_RATE_RANGE_DETECT_SUP_POS = 3,
    BPS_F_PULSE_RATE_RANGE_DETECT_SUP_BIT = CO_BIT(BPS_F_PULSE_RATE_RANGE_DETECT_SUP_POS),

    /// Measurement Position Detection Support bit
    BPS_F_MEAS_POS_DETECT_SUP_POS = 4,
    BPS_F_MEAS_POS_DETECT_SUP_BIT = CO_BIT(BPS_F_MEAS_POS_DETECT_SUP_POS),

    /// Multiple Bond Support bit
    BPS_F_MULTIPLE_BONDS_SUP_POS = 5,
    BPS_F_MULTIPLE_BONDS_SUP_BIT = CO_BIT(BPS_F_MULTIPLE_BONDS_SUP_POS),

    // Bit 6 - 15 RFU
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Blood Pressure measurement structure
struct bps_bp_meas
{
    /// Flag
    uint8_t flags;
    /// User ID
    uint8_t user_id;
    /// Systolic (mmHg/kPa)
    prf_sfloat systolic;
    /// Diastolic (mmHg/kPa)
    prf_sfloat diastolic;
    /// Mean Arterial Pressure (mmHg/kPa)
    prf_sfloat mean_arterial_pressure;
    /// Pulse Rate
    prf_sfloat pulse_rate;
    /// Measurement Status
    uint16_t meas_status;
    /// Time stamp
    struct prf_date_time time_stamp;
};

/// @} blp_common

#endif /* _BLP_COMMON_H_ */
