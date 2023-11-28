/**
 ****************************************************************************************
 *
 * @file cgmp_common.h
 *
 * @brief Header File - Continuous Glucose Monitoring Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 *
 ****************************************************************************************
 */


#ifndef _CGMP_COMMON_H_
#define _CGMP_COMMON_H_

#include "rwip_config.h"

#if (BLE_CGM_CLIENT || BLE_CGM_SERVER)
#include "co_math.h"
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// All valid bits of Flags field
#define CGMS_MEAS_FLAGS_VALID (0xE3)
/// Special value for measurements
/// NaN (not a number)
#define CGMP_NAN    (0x07FF)
/// NRes (not at this resolution)
#define CGMP_NRES   (0x0800)
/// + INFINITY
#define CGMP_INFINITY_PLUS  (0x07FE)
/// - INFINITY
#define CGMP_INFINITY_MINUS (0x0802)
/// Size of the E2E CRC
#define CGMP_CRC_LEN (2)
/// Minimum Size of the Control Point Request
#define CGMP_CP_MIN_LEN (2)
/// Maximum Session Start Time size
#define CGMP_SESS_ST_TIME_SIZE_MAX (7+1+1+2)
/// Size of the calibration block in the Sp Ops requests
#define CGMP_OPS_CALIBRATION_SIZE (11)

/// ERROR CODES
#define CGM_ERROR_MISSING_CRC     (0x80)
#define CGM_ERROR_INVALID_CRC     (0x81)
 
/// Time zone defines
#define CGM_TIME_ZONE_MIN (-48)
#define CGM_TIME_ZONE_MAX (56)
#define CGM_TIME_ZONE_UNKNOWN (-128)

/// Compacted size pf the struct cgm_calibration_operand
#define CGMP_CALIBR_SIZE                                 (11)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// CGM Feature Support bit field
enum cgm_feat_bf
{
    /// Calibration supported
    CGM_FEAT_CALIB_SUP_POS                          = 0,
    CGM_FEAT_CALIB_SUP_BIT                          = CO_BIT(CGM_FEAT_CALIB_SUP_POS),

    /// Patient High/Low Alerts supported
    CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP_POS             = 1,
    CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP_BIT             = CO_BIT(CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP_POS),

    /// Hypo Alert supported
    CGM_FEAT_HYPO_ALERT_SUP_POS                     = 2,
    CGM_FEAT_HYPO_ALERT_SUP_BIT                     = CO_BIT(CGM_FEAT_HYPO_ALERT_SUP_POS),

    /// Hyper Alert supported
    CGM_FEAT_HYPER_ALERT_SUP_POS                    = 3,
    CGM_FEAT_HYPER_ALERT_SUP_BIT                    = CO_BIT(CGM_FEAT_HYPER_ALERT_SUP_POS),

    /// Rate of Increase/Decrease Alert supported
    CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP_POS        = 4,
    CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP_BIT        = CO_BIT(CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP_POS),

    /// Device Specific Alert supported
    CGM_FEAT_DEV_SPEC_ALERT_SUP_POS                 = 5,
    CGM_FEAT_DEV_SPEC_ALERT_SUP_BIT                 = CO_BIT(CGM_FEAT_DEV_SPEC_ALERT_SUP_POS),

    /// Sensor Malfunction Detection supported
    CGM_FEAT_SENSOR_MALFUNC_DETEC_SUP_POS           = 6,
    CGM_FEAT_SENSOR_MALFUNC_DETEC_SUP_BIT           = CO_BIT(CGM_FEAT_SENSOR_MALFUNC_DETEC_SUP_POS),

    /// Sensor Temperature High-Low Detection supported
    CGM_FEAT_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_POS    = 7,
    CGM_FEAT_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_BIT    = CO_BIT(CGM_FEAT_SENSOR_TEMP_HIGH_LOW_DETECT_SUP_POS),

    /// Sensor Result High-Low Detection supported
    CGM_FEAT_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_POS  = 8,
    CGM_FEAT_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_BIT  = CO_BIT(CGM_FEAT_SENSOR_RESULT_HIGH_LOW_DETECT_SUP_POS),

    /// Low Battery Detection supported
    CGM_FEAT_LOW_BATT_DETECT_SUP_POS                = 9,
    CGM_FEAT_LOW_BATT_DETECT_SUP_BIT                = CO_BIT(CGM_FEAT_LOW_BATT_DETECT_SUP_POS),

    /// Sensor Type Error Detection supported
    CGM_FEAT_SENSOR_TYPE_ERR_DETECT_SUP_POS         = 10,
    CGM_FEAT_SENSOR_TYPE_ERR_DETECT_SUP_BIT         = CO_BIT(CGM_FEAT_SENSOR_TYPE_ERR_DETECT_SUP_POS),

    /// General Device Fault supported
    CGM_FEAT_GENERAL_DEV_FAULT_SUP_POS              = 11,
    CGM_FEAT_GENERAL_DEV_FAULT_SUP_BIT              = CO_BIT(CGM_FEAT_GENERAL_DEV_FAULT_SUP_POS),

    /// E2E-CRC supported
    CGM_FEAT_E2E_CRC_SUP_POS                        = 12,
    CGM_FEAT_E2E_CRC_SUP_BIT                        = CO_BIT(CGM_FEAT_E2E_CRC_SUP_POS),

    /// Multiple Bond supported
    CGM_FEAT_MULTI_BOND_SUP_POS                     = 13,
    CGM_FEAT_MULTI_BOND_SUP_BIT                     = CO_BIT(CGM_FEAT_MULTI_BOND_SUP_POS),

    /// Multiple Session supported
    CGM_FEAT_MULTI_SESS_SUP_POS                     = 14,
    CGM_FEAT_MULTI_SESS_SUP_BIT                     = CO_BIT(CGM_FEAT_MULTI_SESS_SUP_POS),

    /// CGM Trend Information supported
    CGM_FEAT_CGM_TREND_INFO_SUP_POS                 = 15,
    CGM_FEAT_CGM_TREND_INFO_SUP_BIT                 = CO_BIT(CGM_FEAT_CGM_TREND_INFO_SUP_POS),

    /// CGM Quality supported
    CGM_FEAT_CGM_QUALITY_SUP_POS                    = 16,
    CGM_FEAT_CGM_QUALITY_SUP_BIT                    = CO_BIT(CGM_FEAT_CGM_QUALITY_SUP_POS),
};

/// Calibration sample type ID
enum cgmp_type_sample_id
{
    /// Capillary whole blood
    CGM_TYPE_SMP_CAPILLARY_WHOLE_BLOOD     = 1,
    /// Capillary Plasma
    CGM_TYPE_SMP_CAPILLARY_PLASMA          = 2,
    /// Capillary whole blood 2
    CGM_TYPE_SMP_CAPILLARY_WHOLE_BLOOD_2   = 3,
    /// Venous plasma
    CGM_TYPE_SMP_VENOUS_PLASMA             = 4,
    /// Arterial whole blood
    CGM_TYPE_SMP_ARTERIAL_WHOLE_BLOOD      = 5,
    /// Arterial plasma
    CGM_TYPE_SMP_ARTERIAL_PLASMA           = 6,
    /// Undetermined whole blood
    CGM_TYPE_SMP_UNDETERMINED_WHOLE_BLOOD  = 7,
    /// Undetermined plasma
    CGM_TYPE_SMP_UNDETERMINED_PLASMA       = 8,
    /// Interstitial fluid
    CGM_TYPE_SMP_INTERSTITIAL_FLUID        = 9,
    /// Control solution
    CGM_TYPE_SMP_CONTROL_SOLUTION          = 10,
};

/// Calibration location of sample
enum cgmp_sample_location_id
{
    /// Finger
    CGM_SMP_LOC_FINGER                    = 1,
    /// Alternate site test
    CGM_SMP_LOC_ALTERNATE_SITE_TEST       = 2,
    /// Earlobe
    CGM_SMP_LOC_EARLOBE                   = 3,
    /// Control solution
    CGM_SMP_LOC_CONTROL_SOLUTION          = 4,
    /// Subcutaneous tissue
    CGM_SMP_LOC_SUBCUTANEOUS_TISSUE       = 5,
    /// Location not available
    CGM_SMP_LOC_LOCATION_NOT_AVAILABLE    = 15,
};

/// Calibration status
enum cgmp_calib_status_bf
{
    /// Calibration Data rejected (Calibration failed)
    /// 0 for false
    /// 1 for true
    CGMP_CAL_STATUS_DATA_REJECTED_POS                   = 0,
    CGMP_CAL_STATUS_DATA_REJECTED_BIT                   = CO_BIT(CGMP_CAL_STATUS_DATA_REJECTED_POS),

    /// Calibration Data out of range
    /// 0 for false
    /// 1 for true
    CGMP_CAL_STATUS_DATA_OUT_RANGE_POS                  = 1,
    CGMP_CAL_STATUS_DATA_OUT_RANGE_BIT                  = CO_BIT(CGMP_CAL_STATUS_DATA_OUT_RANGE_POS),

    /// Calibration Process Pending
    /// 0 for false
    /// 1 for true
    CGMP_CAL_STATUS_PROCESS_PENDING_POS                 = 2,
    CGMP_CAL_STATUS_PROCESS_PENDING_BIT                 = CO_BIT(CGMP_CAL_STATUS_PROCESS_PENDING_POS),
};


/// CGM Measurement Flags bit field
enum cgm_meas_flags_bf
{
    /// CGM Trend Information Field (bit 0)
    /// 0 for not present
    /// 1 for present
    CGM_MEAS_FLAGS_CGM_TREND_INFO_POS                 = 0,
    CGM_MEAS_FLAGS_CGM_TREND_INFO_BIT                 = CO_BIT(CGM_MEAS_FLAGS_CGM_TREND_INFO_POS),

    /// CGM Quality Field (bit 1)
    /// 0 for not present
    /// 1 for present
    CGM_MEAS_FLAGS_CGM_QUALITY_POS                    = 1,
    CGM_MEAS_FLAGS_CGM_QUALITY_BIT                    = CO_BIT(CGM_MEAS_FLAGS_CGM_QUALITY_POS),

    /// Sensor Status Annunciation Field - Warning (bit 5)
    /// 0 for not present
    /// 1 for present
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN_POS       = 5,
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN_BIT       = CO_BIT(CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN_POS),

    /// Sensor Status Annunciation Field - Cal/Temp (bit 6)
    /// 0 for not present
    /// 1 for present
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP_POS   = 6,
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP_BIT   = CO_BIT(CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP_POS),

    /// Sensor Status Annunciation Field - Status (bit 7)
    /// 0 for not present
    /// 1 for present
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS_POS     = 7,
    CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS_BIT     = CO_BIT(CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS_POS),
};

/// CGM Measurement Sensor Status Annunciation bit field[0:7] - Status-Octet present
enum cgm_meas_sensor_status_annuc_status_bf
{
    /// Session Stopped Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_SESS_STOPPED_POS = 0,
    CGM_MEAS_ANNUNC_STATUS_SESS_STOPPED_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_SESS_STOPPED_POS),

    /// Device Battery Low Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_DEV_BATT_LOW_POS = 1,
    CGM_MEAS_ANNUNC_STATUS_DEV_BATT_LOW_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_DEV_BATT_LOW_POS),

    /// Sensor type incorrect for device Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_SENSOR_TYPE_INCOR_POS = 2,
    CGM_MEAS_ANNUNC_STATUS_SENSOR_TYPE_INCOR_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_SENSOR_TYPE_INCOR_POS),

    /// Sensor malfunction Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_SENSOR_MALFUNC_POS = 3,
    CGM_MEAS_ANNUNC_STATUS_SENSOR_MALFUNC_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_SENSOR_MALFUNC_POS),

    /// Device Specific Alert Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_DEV_SPEC_ALERT_POS = 4,
    CGM_MEAS_ANNUNC_STATUS_DEV_SPEC_ALERT_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_DEV_SPEC_ALERT_POS),

    /// General device fault has occurred in the sensor Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_STATUS_GEN_DEV_FAULT_POS = 5,
    CGM_MEAS_ANNUNC_STATUS_GEN_DEV_FAULT_BIT = CO_BIT(CGM_MEAS_ANNUNC_STATUS_GEN_DEV_FAULT_POS),
};

/// CGM Measurement Sensor Status Annunciation Field[8:15] - Cal/Temp-Octet present
enum cgm_meas_sensor_status_annuc_cal_temp_bf
{
    /// Time synchronization between sensor and collector required Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_TIME_SYNC_REQ_POS = 0,
    CGM_MEAS_ANNUNC_CAL_TEMP_TIME_SYNC_REQ_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_TIME_SYNC_REQ_POS),

    /// Calibration not allowed Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_NOT_ALLOWED_POS = 1,
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_NOT_ALLOWED_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_NOT_ALLOWED_POS),

    /// Calibration recommended Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_RECMD_POS = 2,
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_RECMD_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_RECMD_POS),

    /// Calibration required Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_REQ_POS = 3,
    CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_REQ_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_REQ_POS),

    /// Sensor Temperature too high for valid test/result at time of measurement Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_HIGH_POS = 4,
    CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_HIGH_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_HIGH_POS),

    /// Sensor Temperature too low for valid test/result at time of measurement Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_LOW_POS = 5,
    CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_LOW_BIT = CO_BIT(CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_LOW_POS),
};

/// CGM Measurement Sensor Status Annunciation bit field[16:23] - Warning-Octet present
enum cgm_meas_sensor_status_annuc_warn_bf
{

    /// Sensor result lower than the Patient Low level Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_PAT_POS = 0,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_PAT_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_PAT_POS),

    /// Sensor result higher than the Patient High level Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_PAT_POS = 1,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_PAT_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_PAT_POS),

    /// Sensor result lower than the Hypo level Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_HYPO_POS = 2,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_HYPO_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_HYPO_POS),

    /// Sensor result higher than the Hyper level Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_HYPER_POS = 3,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_HYPER_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_HYPER_POS),

    /// Sensor Rate of Decrease exceeded Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_DECR_POS = 4,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_DECR_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_DECR_POS),

    /// Sensor Rate of Increase exceeded Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_INCR_POS = 5,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_INCR_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_INCR_POS),

    /// Sensor result lower than the device can process Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_DEV_POS = 6,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_DEV_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_DEV_POS),

    /// Sensor result higher than the device can process Field
    /// 0 for false
    /// 1 for true
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_DEV_POS = 7,
    CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_DEV_BIT = CO_BIT(CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_DEV_POS),
};


/// Daylight Saving Time Offset
enum cgmp_dst_offset_id
{
    /// Daylight Saving Time Offset +0h
    CGM_DST_OFFSET_STANDARD_TIME              = 0,
    /// Daylight Saving Time Offset +0.5h
    CGM_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME = 2,
    /// Daylight Saving Time Offset +1h
    CGM_DST_OFFSET_DAYLIGHT_TIME              = 4,
    /// Daylight Saving Time Offset +2h
    CGM_DST_OFFSET_DOUBLE_DAYLIGHT_TIME       = 8,
    /// Unknown value
    CGM_DST_OFFSET_UNKNOWN_DAYLIGHT_TIME      = 255,
};

/// Control Point OpCodes
enum cgmp_cp_opcodes_id
{
    /// Report stored records
    CGMP_OPCODE_REP_STRD_RECS               = 1,
    /// Delete stored records
    CGMP_OPCODE_DEL_STRD_RECS               = 2,
    /// Abort operation
    CGMP_OPCODE_ABORT_OP                    = 3,
    /// Report number of stored records
    CGMP_OPCODE_REP_NUM_OF_STRD_RECS        = 4,
    /// Number of stored records response
    CGMP_OPCODE_NUM_OF_STRD_RECS_RSP        = 5,
    /// Response code
    CGMP_OPCODE_RSP_CODE                    = 6,
};

/// Control Point Operators
enum cgmp_cp_operator_id
{
    /// NULL operation
    CGMP_OP_NULL                  = 0,
    /// All records
    CGMP_OP_ALL_RECS              = 1,
    /// Less than or equal to
    CGMP_OP_LT_OR_EQ              = 2,
    /// Greater than or equal to
    CGMP_OP_GT_OR_EQ              = 3,
    /// Within range of (inclusive)
    CGMP_OP_WITHIN_RANGE_OF       = 4,
    /// First record(i.e. oldest record)
    CGMP_OP_FIRST_REC             = 5,
    /// Last record (i.e. most recent record)
    CGMP_OP_LAST_REC              = 6,
};


/// record access control filter type
enum cgmp_racp_filter
{
    /// Filter using Time Offset
    CGMP_FILTER_TIME_OFFSET = (1),
};


/// Record Access Control Point Response Code Values
enum cgmp_racp_rsp_id
{
    /// Success
    CGMP_RSP_SUCCESS                             = 1,
    /// Not supported
    CGMP_RSP_OP_CODE_NOT_SUP                     = 2,
    /// Invalid Operator
    CGMP_RSP_INV_OPERATOR                        = 3,
    /// Operator not supported
    CGMP_RSP_OPERATOR_NOT_SUP                    = 4,
    /// Invalid Operand
    CGMP_RSP_INV_OPERAND                         = 5,
    /// No records found
    CGMP_RSP_NO_RECORDS_FOUND                    = 6,
    /// Abort unsuccessful
    CGMP_RSP_ABORT_UNSUCCESSFUL                  = 7,
    /// Procedure not completed
    CGMP_RSP_PROC_NOT_CMP                        = 8,
    /// Operand not supported
    CGMP_RSP_OPERAND_NOT_SUP                     = 9,
};

/// Specific Ops Control Point Response Codes
enum cgmp_spec_ops_cp_rsp_id
{
    /// Success - Normal response for successful operation
    CGMP_OPS_RSP_SUCCESS                            = 1,
    /// Op Code not supported - Normal response if unsupported Op Code is received
    CGMP_OPS_RSP_OP_CODE_NOT_SUP                    = 2,
    /// Invalid Operand - Normal response if Operand received does not meet the requirements of the service
    CGMP_OPS_RSP_INV_OPERAND                        = 3,
    /// Procedure not completed - Normal response if unable to complete a procedure for any reason
    CGMP_OPS_RSP_PROC_NOT_CMP                       = 4,
    /// Parameters out of range - Normal response if Operand received does not meet the range requirements
    CGMP_OPS_RSP_PARAM_OUT_OF_RANGE                 = 5,

};

/// CGM OPS CONTROL POINT Op code
enum cgmp_spec_ops_opcodes_id
{
    /// Set CGM communication interval
    /// The response to this control point is Response Code (Op Code 0x0F)
    CGMP_OPS_CODE_SET_CGM_COM_INTVL                     = 1,
    /// Get CGM communication interval
    /// The normal response to this control point is Op Code 0x03.
    /// For error conditions, the response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_CGM_COM_INTVL                     = 2,
    /// Communication interval response
    /// This is the normal response to Op Code 0x02
    CGMP_OPS_CODE_CGM_COM_INTVL_RSP                     = 3,
    /// Set Glucose Calibration value
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_GLUC_CALIB_VALUE                  = 4,
    /// Get Glucose Calibration value
    /// The normal response to this control point is Op Code 0x06. for error conditions,
    /// the response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE                  = 5,
    /// Glucose Calibration value
    /// Glucose Calibration Value response
    CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP                  = 6,
    /// Set Patient High Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_PAT_HIGH_ALERT_LEVEL              = 7,
    /// Get Patient High Alert Level
    /// The normal response to this control point is Op Code 0x09.
    /// For error conditions, the response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_PAT_HIGH_ALERT_LEVEL              = 8,
    /// Patient High Alert Level response
    /// This is the normal response to Op Code 0x08
    CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP              = 9,
    /// Set Patient Low Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_PAT_LOW_ALERT_LEVEL               = 10,
    /// Get Patient Low Alert Level
    /// The normal response to this control point is Op Code 0x0C.
    /// the response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_PAT_LOW_ALERT_LEVEL               = 11,
    /// Patient Low Alert Level response
    /// This is the normal response to Op Code 0x0B
    CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP               = 12,
    /// Set Hypo Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_HYPO_ALERT_LEVEL                  = 13,
    /// Get Hypo Alert Level
    /// The normal response to this control point is Op Code 0x0F.
    /// the response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_HYPO_ALERT_LEVEL                  = 14,
    /// Hypo Alert Level response
    /// This is the normal response to Op Code 0x0E
    CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP                  = 15,
    /// Set Hyper Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_HYPER_ALERT_LEVEL                 = 16,
    /// Get Hyper Alert Level
    /// The normal response to this control point is Op Code 0x12.
    /// The response is defined in the Op Code - Response Codes field
    CGMP_OPS_CODE_GET_HYPER_ALERT_LEVEL                 = 17,
    /// Hyper Alert Level response
    /// This is the normal response to Op Code 0x11
    CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP                 = 18,
    /// Set Rate of Decrease Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_RATE_OF_DECR_ALERT_LEVEL          = 19,
    /// Get Rate of Decrease Alert Level
    /// The normal response to this control point is Op Code 0x15.
    /// For error conditions, the response is Response Code
    CGMP_OPS_CODE_GET_RATE_OF_DECR_ALERT_LEVEL          = 20,
    /// Rate of Decrease Alert Level Response
    /// This is the normal response to Op Code 0x14
    CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP          = 21,
    /// Set Rate of Increase Alert Level
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_SET_RATE_OF_INCR_ALERT_LEVEL          = 22,
    /// Get Rate of Increase Alert Level
    /// The normal response to this control point is Op Code 0x18.
    /// For error conditions, the response is Response Code
    CGMP_OPS_CODE_GET_RATE_OF_INCR_ALERT_LEVEL          = 23,
    /// Rate of Increase Alert Level Response
    /// This is the normal response to Op Code 0x17
    CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP          = 24,
    /// Reset Device Specific Alert
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_RESET_DEV_SPEC_ALERT                  = 25,
    /// Start Session
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_START_SESS                            = 26,
    /// Stop Session
    /// The response to this control point is defined in Op Code - Response Codes field
    CGMP_OPS_CODE_STOP_SESS                             = 27,
    /// Response
    /// see Op Code - Response Codes field
    CGMP_OPS_CODE_RSP_CODE                              = 28,

};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// CGM Calibration Operand value
struct cgm_calib_operand
{
    /// Glucose Concentration at Calibration
    prf_sfloat cal_gluc_concent;
    /// Calibration Time in minutes
    uint16_t cal_time;
    /// Calibration Type of Sample @see common enum cgmp_type_sample_id
    uint8_t cal_type_spl;
    /// Calibration Location of Sample @see common enum cgmp_sample_location_id
    uint8_t cal_spl_loc;
    /// Next Calibration Time in minutes
    uint16_t next_cal_time;
    /// Calibration Record Number
    uint16_t cal_record_num;
    /// Calibration Status - bitfield @see common enum cgmp_calib_status_bf
    uint8_t cal_status;
};

/// CGM Ops Operand value
union cgmp_ops_operand
{
    /// Communication interval in minutes
    /// @see CGMP_OPS_CODE_SET_CGM_COM_INTVL - 1
    /// @see CGMP_OPS_CODE_CGM_COM_INTVL_RSP - 3
    uint16_t intvl;
    /// Operand value as defined in the Calibration Value Fields.
    /// @see CGMP_OPS_CODE_SET_GLUC_CALIB_VALUE - 4
    struct cgm_calib_operand calib;
    /// Calibration Data Record Number
    /// @see CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE - 5
    /// @see CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP - 6
    uint16_t cal_data_record_num;
    /// Patient High bG value in mg/dL
    /// @see CGMP_OPS_CODE_SET_PAT_HIGH_ALERT_LEVEL - 7
    /// @see CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP - 9
    prf_sfloat pat_high_bg;
    /// Patient Low bG value in mg/dL
    /// @see CGMP_OPS_CODE_SET_PAT_LOW_ALERT_LEVEL - 10
    /// @see CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP - 12
    prf_sfloat pat_low_bg;
    /// Hypo Alert Level value in mg/dL
    /// @see CGMP_OPS_CODE_SET_HYPO_ALERT_LEVEL - 13
    /// @see CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP - 15
    prf_sfloat hypo_alert_level;
    /// Hyper Alert Level value in mg/dL
    /// @see CGMP_OPS_CODE_SET_HYPER_ALERT_LEVEL - 16
    /// @see CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP - 18
    prf_sfloat hyper_alert_level;
    /// Rate of Decrease Alert Level value in mg/dL/min
    /// @see CGMP_OPS_CODE_SET_RATE_OF_DECR_ALERT_LEVEL - 19
    /// @see CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP - 21
    prf_sfloat rate_decr_alert_level;
    /// Rate of Increase Alert Level value in mg/dL/min
    /// @see CGMP_OPS_CODE_SET_RATE_OF_INCR_ALERT_LEVEL - 22
    /// @see CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP - 24
    prf_sfloat rate_incr_alert_level;
    /// Request Op Code, Response Code Value
    /// @see CGMP_OPS_CODE_RSP_CODE - 28
    struct req_rsp_code
    {
        /// request op code value
        uint8_t req_op_code;
        /// response code value
        uint8_t rsp_code;
    } req_rsp_value;
};

/// Record access control point response
struct cgmp_racp_rsp
{
    /// operation code
    uint8_t op_code;

    /// operator (ignored, always set to null)
    uint8_t operator;

    /// response operand
    union
    {
        /// Number of record (if op_code = CGMP_REQ_NUM_OF_STRD_RECS_RSP)
        uint16_t num_of_record;

        /// Command Status (if op_code = CGMP_REQ_RSP_CODE)
        struct
        {
            /// Request Op Code
            uint8_t op_code_req;
            /// Response Code status
            uint8_t status;
        } rsp;
    } operand;
};


#endif /* (BLE_CGMP_CLIENT || BLE_CGMP_SERVER) */

#endif /* _CGMP_COMMON_H_ */
