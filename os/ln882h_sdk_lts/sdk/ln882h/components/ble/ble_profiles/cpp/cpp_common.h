/**
 ****************************************************************************************
 *
 * @file cpp_common.h
 *
 * @brief Header File - Cycling Power Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


#ifndef _CPP_COMMON_H_
#define _CPP_COMMON_H_

/**
 ****************************************************************************************
 * @addtogroup CPP Cycling Power Profile
 * @ingroup PROFILE
 * @brief Cycling Power Profile
 *
 * The Cycling Power Profile is used to enable a collector device in order to obtain
 * data from a Cycling Power Sensor (CP Sensor) that exposes the Cycling Power Service
 * in sports and fitness applications.
 *
 * This file contains all definitions that are common for the server and the client parts
 * of the profile.
 *****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */


#include "prf_types.h"
#include <stdint.h>
#include "co_math.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// Procedure Already in Progress Error Code
#define CPP_ERROR_PROC_IN_PROGRESS        (0xFE)

/// CP Measurement Notification Value Max Length
#define CPP_CP_MEAS_NTF_MAX_LEN           (35)
/// CP Measurement Value Min Length
#define CPP_CP_MEAS_NTF_MIN_LEN           (4)

/// ADV Header size
#define CPP_CP_ADV_HEADER_LEN             (3)
/// ADV Length size
#define CPP_CP_ADV_LENGTH_LEN             (1)

/// CP Measurement Advertisement Value Max Length
#define CPP_CP_MEAS_ADV_MAX_LEN           (CPP_CP_MEAS_NTF_MAX_LEN + CPP_CP_ADV_HEADER_LEN)
/// CP Measurement Value Min Length
#define CPP_CP_MEAS_ADV_MIN_LEN           (CPP_CP_MEAS_NTF_MIN_LEN + CPP_CP_ADV_HEADER_LEN)

/// CP Vector Value Max Length
#define CPP_CP_VECTOR_MAX_LEN             (19)
/// CP Vector Value Min Length
#define CPP_CP_VECTOR_MIN_LEN             (1)

/// CP Control Point Value Max Length
#define CPP_CP_CNTL_PT_REQ_MAX_LEN        (9)
/// CP Control Point Value Min Length
#define CPP_CP_CNTL_PT_REQ_MIN_LEN        (1)

/// CP Control Point Value Max Length
#define CPP_CP_CNTL_PT_RSP_MAX_LEN        (20)
/// CP Control Point Value Min Length
#define CPP_CP_CNTL_PT_RSP_MIN_LEN        (3)

/// CPP Measurement all supported
#define CPP_MEAS_ALL_SUPP                 (0x1FFF)

/// CPP Feature all supported
#define CPP_FEAT_ALL_SUPP                 (0x0007FFFF)

/// CPP Vector all supported
#define CPP_VECTOR_ALL_SUPP               (0x3F)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// CPP Service Characteristics
enum cpp_cpps_char
{
    /// CPP Measurement
    CPP_CPS_MEAS_CHAR,
    /// CPP Feature
    CPP_CPS_FEAT_CHAR,
    /// Sensor Location
    CPP_CPS_SENSOR_LOC_CHAR,
    ///Cycling Power Vector
    CPP_CPS_VECTOR_CHAR,
    /// CP Control Point
    CPP_CPS_CTNL_PT_CHAR,

    CPP_CPS_CHAR_MAX,
};

/// CPP Measurement Flags bit field
enum cpp_meas_flags_bf
{
    /// Pedal Power Balance Present
    CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_POS        = 0,
    CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_BIT        = CO_BIT(CPP_MEAS_PEDAL_POWER_BALANCE_PRESENT_POS),

    /// Pedal Power Balance Reference
    CPP_MEAS_PEDAL_POWER_BALANCE_REFERENCE_POS      = 1,
    CPP_MEAS_PEDAL_POWER_BALANCE_REFERENCE_BIT      = CO_BIT(CPP_MEAS_PEDAL_POWER_BALANCE_REFERENCE_POS),

    /// Accumulated Torque Present
    CPP_MEAS_ACCUM_TORQUE_PRESENT_POS               = 2,
    CPP_MEAS_ACCUM_TORQUE_PRESENT_BIT               = CO_BIT(CPP_MEAS_ACCUM_TORQUE_PRESENT_POS),

    /// Accumulated Torque Source
    CPP_MEAS_ACCUM_TORQUE_SOURCE_POS                = 3,
    CPP_MEAS_ACCUM_TORQUE_SOURCE_BIT                = CO_BIT(CPP_MEAS_ACCUM_TORQUE_SOURCE_POS),

    /// Wheel Revolution Data Present
    CPP_MEAS_WHEEL_REV_DATA_PRESENT_POS             = 4,
    CPP_MEAS_WHEEL_REV_DATA_PRESENT_BIT             = CO_BIT(CPP_MEAS_WHEEL_REV_DATA_PRESENT_POS),

    /// Crank Revolution Data Present
    CPP_MEAS_CRANK_REV_DATA_PRESENT_POS             = 5,
    CPP_MEAS_CRANK_REV_DATA_PRESENT_BIT             = CO_BIT(CPP_MEAS_CRANK_REV_DATA_PRESENT_POS),

    /// Extreme Force Magnitudes Present
    CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_POS   = 6,
    CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_BIT   = CO_BIT(CPP_MEAS_EXTREME_FORCE_MAGNITUDES_PRESENT_POS),

    /// Extreme Torque Magnitudes Present
    CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_POS  = 7,
    CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_BIT  = CO_BIT(CPP_MEAS_EXTREME_TORQUE_MAGNITUDES_PRESENT_POS),

    /// Extreme Angles Present
    CPP_MEAS_EXTREME_ANGLES_PRESENT_POS             = 8,
    CPP_MEAS_EXTREME_ANGLES_PRESENT_BIT             = CO_BIT(CPP_MEAS_EXTREME_ANGLES_PRESENT_POS),

    /// Top Dead Spot Angle Present
    CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_POS        = 9,
    CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_BIT        = CO_BIT(CPP_MEAS_TOP_DEAD_SPOT_ANGLE_PRESENT_POS),

    /// Bottom Dead Spot Angle Present
    CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_POS     = 10,
    CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_BIT     = CO_BIT(CPP_MEAS_BOTTOM_DEAD_SPOT_ANGLE_PRESENT_POS),

    /// Accumulated Energy Present
    CPP_MEAS_ACCUM_ENERGY_PRESENT_POS               = 11,
    CPP_MEAS_ACCUM_ENERGY_PRESENT_BIT               = CO_BIT(CPP_MEAS_ACCUM_ENERGY_PRESENT_POS),

    /// Offset Compensation Indicator
    CPP_MEAS_OFFSET_COMPENSATION_INDICATOR_POS      = 12,
    CPP_MEAS_OFFSET_COMPENSATION_INDICATOR_BIT      = CO_BIT(CPP_MEAS_OFFSET_COMPENSATION_INDICATOR_POS),

    // bit 13 - 15 RFU
};

/// CPP Feature Flags bit field
enum cpp_feat_flags_bf
{
    /// Pedal Power Balance Supported
    CPP_FEAT_PEDAL_POWER_BALANCE_SUP_POS                = 0,
    CPP_FEAT_PEDAL_POWER_BALANCE_SUP_BIT                = CO_BIT(CPP_FEAT_PEDAL_POWER_BALANCE_SUP_POS),

    /// Accumulated Torque Supported
    CPP_FEAT_ACCUM_TORQUE_SUP_POS                       = 1,
    CPP_FEAT_ACCUM_TORQUE_SUP_BIT                       = CO_BIT(CPP_FEAT_ACCUM_TORQUE_SUP_POS),

    /// Wheel Revolution Data Supported
    CPP_FEAT_WHEEL_REV_DATA_SUP_POS                     = 2,
    CPP_FEAT_WHEEL_REV_DATA_SUP_BIT                     = CO_BIT(CPP_FEAT_WHEEL_REV_DATA_SUP_POS),

    /// Crank Revolution Data Supported
    CPP_FEAT_CRANK_REV_DATA_SUP_POS                     = 3,
    CPP_FEAT_CRANK_REV_DATA_SUP_BIT                     = CO_BIT(CPP_FEAT_CRANK_REV_DATA_SUP_POS),

    /// Extreme Magnitudes Supported
    CPP_FEAT_EXTREME_MAGNITUDES_SUP_POS                 = 4,
    CPP_FEAT_EXTREME_MAGNITUDES_SUP_BIT                 = CO_BIT(CPP_FEAT_EXTREME_MAGNITUDES_SUP_POS),

    /// Extreme Angles Supported
    CPP_FEAT_EXTREME_ANGLES_SUP_POS                     = 5,
    CPP_FEAT_EXTREME_ANGLES_SUP_BIT                     = CO_BIT(CPP_FEAT_EXTREME_ANGLES_SUP_POS),

    /// Top and Bottom Dead Spot Angles Supported
    CPP_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_POS            = 6,
    CPP_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_BIT            = CO_BIT(CPP_FEAT_TOPBOT_DEAD_SPOT_ANGLES_SUP_POS),

    /// Accumulated Energy Supported
    CPP_FEAT_ACCUM_ENERGY_SUP_POS                       = 7,
    CPP_FEAT_ACCUM_ENERGY_SUP_BIT                       = CO_BIT(CPP_FEAT_ACCUM_ENERGY_SUP_POS),

    /// Offset Compensation Indicator Supported
    CPP_FEAT_OFFSET_COMP_IND_SUP_POS                    = 8,
    CPP_FEAT_OFFSET_COMP_IND_SUP_BIT                    = CO_BIT(CPP_FEAT_OFFSET_COMP_IND_SUP_POS),

    /// Offset Compensation Supported
    CPP_FEAT_OFFSET_COMP_SUP_POS                        = 9,
    CPP_FEAT_OFFSET_COMP_SUP_BIT                        = CO_BIT(CPP_FEAT_OFFSET_COMP_SUP_POS),

    /// CP Measurement CH Content Masking Supported
    CPP_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_POS         = 10,
    CPP_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_BIT         = CO_BIT(CPP_FEAT_CP_MEAS_CH_CONTENT_MASKING_SUP_POS),

    /// Multiple Sensor Locations Supported
    CPP_FEAT_MULT_SENSOR_LOC_SUP_POS                    = 11,
    CPP_FEAT_MULT_SENSOR_LOC_SUP_BIT                    = CO_BIT(CPP_FEAT_MULT_SENSOR_LOC_SUP_POS),

    /// Crank Length Adjustment Supported
    CPP_FEAT_CRANK_LENGTH_ADJ_SUP_POS                   = 12,
    CPP_FEAT_CRANK_LENGTH_ADJ_SUP_BIT                   = CO_BIT(CPP_FEAT_CRANK_LENGTH_ADJ_SUP_POS),

    /// Chain Length Adjustment Supported
    CPP_FEAT_CHAIN_LENGTH_ADJ_SUP_POS                   = 13,
    CPP_FEAT_CHAIN_LENGTH_ADJ_SUP_BIT                   = CO_BIT(CPP_FEAT_CHAIN_LENGTH_ADJ_SUP_POS),

    /// Chain Weight Adjustment Supported
    CPP_FEAT_CHAIN_WEIGHT_ADJ_SUP_POS                   = 14,
    CPP_FEAT_CHAIN_WEIGHT_ADJ_SUP_BIT                   = CO_BIT(CPP_FEAT_CHAIN_WEIGHT_ADJ_SUP_POS),

    /// Span Length Adjustment Supported
    CPP_FEAT_SPAN_LENGTH_ADJ_SUP_POS                    = 15,
    CPP_FEAT_SPAN_LENGTH_ADJ_SUP_BIT                    = CO_BIT(CPP_FEAT_SPAN_LENGTH_ADJ_SUP_POS),

    /// Sensor Measurement Context
    CPP_FEAT_SENSOR_MEAS_CONTEXT_POS                    = 16,
    CPP_FEAT_SENSOR_MEAS_CONTEXT_BIT                    = CO_BIT(CPP_FEAT_SENSOR_MEAS_CONTEXT_POS),

    /// Instantaneous Measurement Direction Supported
    CPP_FEAT_INSTANT_MEAS_DIRECTION_SUP_POS             = 17,
    CPP_FEAT_INSTANT_MEAS_DIRECTION_SUP_BIT             = CO_BIT(CPP_FEAT_INSTANT_MEAS_DIRECTION_SUP_POS),

    /// Factory Calibration Date Supported
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_POS           = 18,
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_BIT           = CO_BIT(CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_POS),

    /// Enhanced Offset Compensation Supported
    CPP_FEAT_ENHANCED_OFFSET_COMPENS_SUP_POS            = 19,
    CPP_FEAT_ENHANCED_OFFSET_COMPENS_SUP_BIT            = CO_BIT(CPP_FEAT_ENHANCED_OFFSET_COMPENS_SUP_POS),

    /// Distribute System Supported LSB
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_POS       = 20,
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_BIT       = CO_BIT(CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_LSB_POS),

    /// Distribute System Supported MSB
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_POS       = 21,
    CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_BIT       = CO_BIT(CPP_FEAT_FACTORY_CALIBRATION_DATE_SUP_MSB_POS),

    // bit 22 - 31 RFU
};

/// CPP Sensor Locations Keys
enum cpp_sensor_loc
{
    /// Other (0)
    CPP_LOC_OTHER          = 0,
    /// Top of shoe (1)
    CPP_LOC_TOP_SHOE,
    /// In shoe (2)
    CPP_LOC_IN_SHOE,
    /// Hip (3)
    CPP_LOC_HIP,
    /// Front Wheel (4)
    CPP_LOC_FRONT_WHEEL,
    /// Left Crank (5)
    CPP_LOC_LEFT_CRANK,
    /// Right Crank (6)
    CPP_LOC_RIGHT_CRANK,
    /// Left Pedal (7)
    CPP_LOC_LEFT_PEDAL,
    /// Right Pedal (8)
    CPP_LOC_RIGHT_PEDAL,
    /// Front Hub (9)
    CPP_LOC_FRONT_HUB,
    /// Rear Dropout (10)
    CPP_LOC_REAR_DROPOUT,
    /// Chainstay (11)
    CPP_LOC_CHAINSTAY,
    /// Rear Wheel (12)
    CPP_LOC_REAR_WHEEL,
    /// Rear Hub (13)
    CPP_LOC_REAR_HUB,
    /// Chest (14)
    CPP_LOC_CHEST,

    CPP_LOC_MAX,
};

/// CPP Vector Flags bit field
enum cpp_vector_flags_bf
{
    /// Crank Revolution Data Present
    CPP_VECTOR_CRANK_REV_DATA_PRESENT_POS               = 0,
    CPP_VECTOR_CRANK_REV_DATA_PRESENT_BIT               = CO_BIT(CPP_VECTOR_CRANK_REV_DATA_PRESENT_POS),

    /// First Crank Measurement Angle Present
    CPP_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_POS       = 1,
    CPP_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_BIT       = CO_BIT(CPP_VECTOR_FIRST_CRANK_MEAS_ANGLE_PRESENT_POS),

    /// Instantaneous Force Magnitude Array Present
    CPP_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_POS   = 2,
    CPP_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_BIT   = CO_BIT(CPP_VECTOR_INST_FORCE_MAGNITUDE_ARRAY_PRESENT_POS),

    /// Instantaneous Torque Magnitude Array Present
    CPP_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_POS  = 3,
    CPP_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_BIT  = CO_BIT(CPP_VECTOR_INST_TORQUE_MAGNITUDE_ARRAY_PRESENT_POS),

    /// Instantaneous Measurement Direction LSB
    CPP_VECTOR_INST_MEAS_DIRECTION_LSB_POS              = 4,
    CPP_VECTOR_INST_MEAS_DIRECTION_LSB_BIT              = CO_BIT(CPP_VECTOR_INST_MEAS_DIRECTION_LSB_POS),

    /// Instantaneous Measurement Direction MSB
    CPP_VECTOR_INST_MEAS_DIRECTION_MSB_POS              = 5,
    CPP_VECTOR_INST_MEAS_DIRECTION_MSB_BIT              = CO_BIT(CPP_VECTOR_INST_MEAS_DIRECTION_MSB_POS),

    // bit 6 - 7 RFU
};

/// CPP Control Point Code Keys
enum cpp_ctnl_pt_code
{
    /// Reserved value
    CPP_CTNL_PT_RESERVED                    = 0,

    /// Set Cumulative Value
    CPP_CTNL_PT_SET_CUMUL_VAL               = 1,
    /// Update Sensor Location
    CPP_CTNL_PT_UPD_SENSOR_LOC              = 2,
    /// Request Supported Sensor Locations
    CPP_CTNL_PT_REQ_SUPP_SENSOR_LOC         = 3,
    /// Set Crank Length
    CPP_CTNL_PT_SET_CRANK_LENGTH            = 4,
    /// Request Crank Length
    CPP_CTNL_PT_REQ_CRANK_LENGTH            = 5,
    /// Set Chain Length
    CPP_CTNL_PT_SET_CHAIN_LENGTH            = 6,
    /// Request Chain Length
    CPP_CTNL_PT_REQ_CHAIN_LENGTH            = 7,
    /// Set Chain Weight
    CPP_CTNL_PT_SET_CHAIN_WEIGHT            = 8,
    /// Request Chain Weight
    CPP_CTNL_PT_REQ_CHAIN_WEIGHT            = 9,
    /// Set Span Length
    CPP_CTNL_PT_SET_SPAN_LENGTH             = 10,
    /// Request Span Length
    CPP_CTNL_PT_REQ_SPAN_LENGTH             = 11,
    /// Start Offset Compensation
    CPP_CTNL_PT_START_OFFSET_COMP           = 12,
    /// Mask CP Measurement Characteristic Content
    CPP_CTNL_MASK_CP_MEAS_CH_CONTENT        = 13,
    /// Request Sampling Rate
    CPP_CTNL_REQ_SAMPLING_RATE              = 14,
    /// Request Factory Calibration Date
    CPP_CTNL_REQ_FACTORY_CALIBRATION_DATE   = 15,
    /// Start Enhanced Offset Compensation
    CPP_CTNL_START_ENHANCED_OFFSET_COMP     = 16,

    // Value 17 - 31 RFU

    /// Response Code
    CPP_CTNL_PT_RSP_CODE                    = 32,

    // Value 33 - 255 RFU
};

/// CPP Control Point Response Value
enum cpp_ctnl_pt_resp_val
{
    /// Reserved value
    CPP_CTNL_PT_RESP_RESERVED      = 0,

    /// Success
    CPP_CTNL_PT_RESP_SUCCESS,
    /// Operation Code Not Supported
    CPP_CTNL_PT_RESP_NOT_SUPP,
    /// Invalid Parameter
    CPP_CTNL_PT_RESP_INV_PARAM,
    /// Operation Failed
    CPP_CTNL_PT_RESP_FAILED,
};

/// CPP Control Point Error Response Parameter
enum cpp_ctnl_pt_err_resp_param
{
    /// Reserved value
    CPP_CTNL_PT_ERR_RSP_PARAM_RESERVED = 0x00,

    /// Incorrect calibration position
    CPP_CTNL_PT_ERR_RSP_PARAM_INCORRECT_CALIB_POS = 0x01,

    // Value 0x02 - 0xFE RFU

    /// Manufacturer specific error follows
    CPP_CTNL_PT_ERR_RSP_PARAM_MANUF_SPEC_ERR_FOLLOWS = 0xFF,

};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// CP Measurement
struct cpp_cp_meas
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Power
    int16_t inst_power;
    /// Pedal Power Balance
    uint8_t pedal_power_balance;
    /// Accumulated torque
    uint16_t accum_torque;
    /// Cumulative Wheel Revolutions
    int16_t cumul_wheel_rev;
    /// Last Wheel Event Time
    uint16_t last_wheel_evt_time;
    /// Cumulative Crank Revolution
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// Maximum Force Magnitude
    int16_t max_force_magnitude;
    /// Minimum Force Magnitude
    int16_t min_force_magnitude;
    /// Maximum Torque Magnitude
    int16_t max_torque_magnitude;
    /// Minimum Torque Magnitude
    int16_t min_torque_magnitude;
    /// Maximum Angle (12 bits)
    uint16_t max_angle;
    /// Minimum Angle (12bits)
    uint16_t min_angle;
    /// Top Dead Spot Angle
    uint16_t top_dead_spot_angle;
    /// Bottom Dead Spot Angle
    uint16_t bot_dead_spot_angle;
    ///Accumulated energy
    uint16_t accum_energy;
};

/// CP Measurement
struct cpp_cp_meas_ind
{
    /// Flags
    uint16_t flags;
    /// Instantaneous Power
    int16_t inst_power;
    /// Pedal Power Balance
    uint8_t pedal_power_balance;
    /// Accumulated torque
    uint16_t accum_torque;
    /// Cumulative Wheel Revolutions
    uint32_t cumul_wheel_rev;
    /// Last Wheel Event Time
    uint16_t last_wheel_evt_time;
    /// Cumulative Crank Revolution
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// Maximum Force Magnitude
    int16_t max_force_magnitude;
    /// Minimum Force Magnitude
    int16_t min_force_magnitude;
    /// Maximum Torque Magnitude
    int16_t max_torque_magnitude;
    /// Minimum Torque Magnitude
    int16_t min_torque_magnitude;
    /// Maximum Angle (12 bits)
    uint16_t max_angle;
    /// Minimum Angle (12bits)
    uint16_t min_angle;
    /// Top Dead Spot Angle
    uint16_t top_dead_spot_angle;
    /// Bottom Dead Spot Angle
    uint16_t bot_dead_spot_angle;
    /// Accumulated energy
    uint16_t accum_energy;

};

/// Enhanced Offset Compensation
struct cpp_enhanced_offset_comp
{
    /// Offset
    int16_t comp_offset;
    /// Manufacturer Company ID
    uint16_t manu_comp_id;
    /// Response Parameter (error condition)
    uint8_t rsp_param;
    /// Manufacturer specific data length
    uint8_t length;
    /// Manufacturer specific data
    uint8_t data[__ARRAY_EMPTY];
};

/// CP Vector
struct cpp_cp_vector
{
    /// Flags
    uint8_t flags;
    /// Force-Torque Magnitude Array Length
    uint8_t nb;
    /// Cumulative Crank Revolutions
    uint16_t cumul_crank_rev;
    /// Last Crank Event Time
    uint16_t last_crank_evt_time;
    /// First Crank Measurement Angle
    uint16_t first_crank_meas_angle;
    /// Mutually excluded Force and Torque Magnitude Arrays
    int16_t  force_torque_magnitude[__ARRAY_EMPTY];
};

/// CP Control Point Request
struct cpp_ctnl_pt_req
{
    /// Operation Code
    uint8_t op_code;
    /// Value
    union cpp_ctnl_pt_req_val
    {
        /// Cumulative Value
        uint32_t cumul_val;
        /// Sensor Location
        uint8_t sensor_loc;
        /// Crank Length
        uint16_t crank_length;
        /// Chain Length
        uint16_t chain_length;
        /// Chain Weight
        uint16_t chain_weight;
        /// Span Length
        uint16_t span_length;
        /// Mask Content
        uint16_t mask_content;
    } value;
};

/// CP Control Point Response
struct cpp_ctnl_pt_rsp
{
    /// Operation code - Response Code (0x20)
    uint8_t op_code;
    /// Requested Operation Code
    uint8_t req_op_code;
    /// Response Value
    uint8_t resp_value;

    ///Value
    union cpp_ctnl_pt_rsp_val
    {
        /// List of supported locations
        uint32_t supp_loc;
        /// Crank Length
        uint16_t crank_length;
        /// Chain Length
        uint16_t chain_length;
        /// Chain Weight
        uint16_t chain_weight;
        /// Span Length
        uint16_t span_length;
        /// Start Offset Compensation
        int16_t offset_comp;
        /// Sampling Rate Procedure
        uint8_t sampling_rate;
        /// Enhanced Offset Compensation
        struct cpp_enhanced_offset_comp enhanced_offset_comp;
        /// Calibration date
        struct prf_date_time factory_calibration;
    } value;
};

/// @} cpp_common

#endif //(_CPP_COMMON_H_)
