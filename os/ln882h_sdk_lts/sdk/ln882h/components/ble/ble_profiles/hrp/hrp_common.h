/**
 ****************************************************************************************
 *
 * @file hrp_common.h
 *
 * @brief Header File - Heart Rate Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


#ifndef _HRP_COMMON_H_
#define _HRP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup HRP Heart Rate Profile
 * @ingroup PROFILE
 * @brief Heart Rate Profile
 *
 * The HRP module is the responsible block for implementing the Heart Rate Profile
 * functionalities in the BLE Host.
 *
 * The Heart Rate Profile defines the functionality required in a device that allows
 * the user (Collector device) to configure and recover Heart Rate measurements from
 * a Heart Rate device.
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

/// maximum number of RR-Interval supported
#define HRS_MAX_RR_INTERVAL  (4)

/// Heart Rate Control Point Not Supported error code
#define HRS_ERR_HR_CNTL_POINT_NOT_SUPPORTED   (0x80)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// HRS codes for the 2 possible client configuration characteristic descriptors determination
enum
{
    /// Heart Rate Measurement
    HRS_HR_MEAS_CODE = 0x01,
    /// Energy Expended - Heart Rate Control Point
    HRS_HR_CNTL_POINT_CODE,
};


/// Heart Rate Measurement Flags field bit values
enum hrp_meas_flags_bf
{
    /// Heart Rate Value Format bit
    /// 0 for UINT8 and 1 for UINT16. Units: beats per minute (bpm)
    HRS_FLAG_HR_VALUE_FORMAT_POS                = 0,
    HRS_FLAG_HR_VALUE_FORMAT_BIT                = CO_BIT(HRS_FLAG_HR_VALUE_FORMAT_POS),

    /// Sensor Contact Status bits
    /// Sensor Contact feature supported in the current connection
    /// Contact is detected
    HRS_FLAG_SENSOR_CCT_DETECTED_POS            = 1,
    HRS_FLAG_SENSOR_CCT_DETECTED_BIT            = CO_BIT(HRS_FLAG_SENSOR_CCT_DETECTED_POS),

    HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_POS       = 2,
    HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_BIT       = CO_BIT(HRS_FLAG_SENSOR_CCT_FET_SUPPORTED_POS),

    /// Energy Expended Status bit
    /// Energy Expended field is present. Units: kilo Joules
    HRS_FLAG_ENERGY_EXPENDED_PRESENT_POS        = 3,
    HRS_FLAG_ENERGY_EXPENDED_PRESENT_BIT        = CO_BIT(HRS_FLAG_ENERGY_EXPENDED_PRESENT_POS),

    /// RR-Interval bit
    /// One or more RR-Interval values are present. Units: 1/1024 seconds
    HRS_FLAG_RR_INTERVAL_PRESENT_POS            = 4,
    HRS_FLAG_RR_INTERVAL_PRESENT_BIT            = CO_BIT(HRS_FLAG_RR_INTERVAL_PRESENT_POS),
};

/// Body Sensor Location
enum
{
    HRS_LOC_OTHER       = 0,
    HRS_LOC_CHEST       = 1,
    HRS_LOC_WRIST       = 2,
    HRS_LOC_FINGER      = 3,
    HRS_LOC_HAND        = 4,
    HRS_LOC_EAR_LOBE    = 5,
    HRS_LOC_FOOT        = 6,
    HRS_LOC_MAX,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Heart Rate measurement structure
struct hrs_hr_meas
{
    /// Flag
    uint8_t flags;
    /// Heart Rate Measurement Value
    uint16_t heart_rate;
    /// Energy Expended
    uint16_t energy_expended;
    /// RR-Interval numbers (max 4)
    uint8_t nb_rr_interval;
    /// RR-Intervals
    uint16_t rr_intervals[HRS_MAX_RR_INTERVAL];
};


/// @} hrp_common

#endif /* _HRP_COMMON_H_ */
