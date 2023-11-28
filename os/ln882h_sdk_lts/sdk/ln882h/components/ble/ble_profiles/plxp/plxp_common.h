/**
 ****************************************************************************************
 *
 * @file plxp_common.h
 *
 * @brief Header File - Pulse Oximeter Profile common types.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 *
 ****************************************************************************************
 */


#ifndef _PLXP_COMMON_H_
#define _PLXP_COMMON_H_

#include "rwip_config.h"

#if (BLE_PLX_CLIENT || BLE_PLX_SERVER)
#include "co_math.h"
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Measurement Status characteristic all valid field mask
#define PLXP_MEAS_STATUS_VALID_MASK (0xFFE0)

/// Device and Sensor Status all valid field mask
#define PLXP_DEVICE_STATUS_VALID_MASK   (0xFFFF)

/// All Valid bits of spot check flags field
#define PLXP_SPOT_MEAS_FLAGS_VALID_MASK   (0x1F)

/// All Valid bits of continuous measurement flags field
#define PLXP_CONT_MEAS_FLAGS_VALID_MASK   (0x1F)

 /*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Control Point OpCodes
enum plxp_cp_opcodes_id
{
    PLXP_OPCODE_REPORT_STORED_RECORDS           = 1,
    PLXP_OPCODE_DELETE_STORED_RECORDS           = 2,
    PLXP_OPCODE_ABORT_OPERATION                 = 3,
    PLXP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS = 4,
    PLXP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP   = 5,
    PLXP_OPCODE_RESPONSE_CODE                   = 6,
};

/// Control Point Operators
enum plxp_cp_operator_id
{
    PLXP_OPERATOR_NULL                  = 0,
    PLXP_OPERATOR_ALL_RECORDS           = 1,
};

/// Control Point Response Code Values
enum plxp_cp_resp_code_id
{
    PLXP_RESP_SUCCESS                             = 1,
    PLXP_RESP_OP_CODE_NOT_SUPPORTED               = 2,
    PLXP_RESP_INVALID_OPERATOR                    = 3,
    PLXP_RESP_OPERATOR_NOT_SUPPORTED              = 4,
    PLXP_RESP_INVALID_OPERAND                     = 5,
    PLXP_RESP_NO_RECORDS_FOUND                    = 6,
    PLXP_RESP_ABORT_UNSUCCESSFUL                  = 7,
    PLXP_RESP_PROCEDURE_NOT_COMPLETED             = 8,
    PLXP_RESP_OPERAND_NOT_SUPPORTED               = 9,
};

/// Supported Features bit field
enum plxp_sup_feat_bf
{
    /// Measurement Status support is present
    PLXP_FEAT_MEAS_STATUS_SUP_POS               = 0,
    PLXP_FEAT_MEAS_STATUS_SUP_BIT               = CO_BIT(PLXP_FEAT_MEAS_STATUS_SUP_POS),

    /// Device and Sensor Status support is present
    PLXP_FEAT_DEV_SENSOR_STATUS_SUP_POS         = 1,
    PLXP_FEAT_DEV_SENSOR_STATUS_SUP_BIT         = CO_BIT(PLXP_FEAT_DEV_SENSOR_STATUS_SUP_POS),

    /// Measurement Storage for Spot-check measurements is supported
    PLXP_FEAT_MEAS_STORAGE_SUP_POS              = 2,
    PLXP_FEAT_MEAS_STORAGE_SUP_BIT              = CO_BIT(PLXP_FEAT_MEAS_STORAGE_SUP_POS),

    /// Timestamp for Spot-check measurements is supported
    PLXP_FEAT_TIMESTAMP_SUP_POS                 = 3,
    PLXP_FEAT_TIMESTAMP_SUP_BIT                 = CO_BIT(PLXP_FEAT_TIMESTAMP_SUP_POS),

    /// SpO2PR-Fast metric is supported
    PLXP_FEAT_SPO2PR_FAST_SUP_POS               = 4,
    PLXP_FEAT_SPO2PR_FAST_SUP_BIT               = CO_BIT(PLXP_FEAT_SPO2PR_FAST_SUP_POS),

    /// SpO2PR-Slow metric is supported
    PLXP_FEAT_SPO2PR_SLOW_SUP_POS               = 5,
    PLXP_FEAT_SPO2PR_SLOW_SUP_BIT               = CO_BIT(PLXP_FEAT_SPO2PR_SLOW_SUP_POS),

    /// Pulse Amplitude Index field is supported
    PLXP_FEAT_PULSE_AMPL_SUP_POS                = 6,
    PLXP_FEAT_PULSE_AMPL_SUP_BIT                = CO_BIT(PLXP_FEAT_PULSE_AMPL_SUP_POS),

    /// Multiple Bonds Supported
    PLXP_FEAT_MULT_BONDS_SUP_POS                = 7,
    PLXP_FEAT_MULT_BONDS_SUP_BIT                = CO_BIT(PLXP_FEAT_MULT_BONDS_SUP_POS),

    // Bit 8-15 RFU
};

/// Spot check measurement characteristic flags bit field
enum plxp_spot_check_flags_bf
{
    /// Timestamp field (bit 0)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_POS              = 0,
    PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_BIT              = CO_BIT(PLXP_SPOT_MEAS_FLAGS_TIMESTAMP_POS),

    /// Measurement Status field (bit 1)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_POS            = 1,
    PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_BIT            = CO_BIT(PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS_POS),

    /// Device and Sensor Status field (bit 2)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS      = 2,
    PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_BIT      = CO_BIT(PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS),

    /// Pulse Amplitude Index field (bit 3)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_POS        = 3,
    PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_BIT        = CO_BIT(PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE_POS),

    /// Device Clock is Not Set (bit 4)
    /// 0 for not present
    /// 1 for present
    PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_POS          = 4,
    PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_BIG          = CO_BIT(PLXP_SPOT_MEAS_FLAGS_CLOCK_NOT_SET_POS),

    // Bit 5-7 RFU
};

/// Continuous measurement characteristic flags bit field
enum plxp_cont_meas_flags_bf
{
    /// SpO2PR-Fast field is present
    PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_POS            = 0,
    PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_BIT            = CO_BIT(PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST_POS),

    //// SpO2PR-Slow field is present
    PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_POS            = 1,
    PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_BIT            = CO_BIT(PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW_POS),

    /// Measurement Status field is present
    PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_POS            = 2,
    PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_BIT            = CO_BIT(PLXP_CONT_MEAS_FLAGS_MEAS_STATUS_POS),

    /// Device and Sensor Status field is present
    PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS      = 3,
    PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_BIT      = CO_BIT(PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS_POS),

    /// Pulse Amplitude Index field is present
    PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_POS             = 4,
    PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_BIT             = CO_BIT(PLXP_CONT_MEAS_FLAGS_PULSE_AMPL_POS),

    // Bit 5-7 RFU
};

/// Measurement Status field (not used)
enum plxp_meas_status_sup_bf
{
    // Bit 0-4 RFU
    /// Measurement Ongoing bit supported
    PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_POS           = 5,
    PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_BIT           = CO_BIT(PLXP_MEAS_STATUS_MEAS_ONGOING_SUP_POS),

    /// Early Estimated Data bit supported
    PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_POS       = 6,
    PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_BIT       = CO_BIT(PLXP_MEAS_STATUS_EARLY_ESTIM_DATA_SUP_POS),

    /// Validated Data bit supported
    PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_POS         = 7,
    PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_BIT         = CO_BIT(PLXP_MEAS_STATUS_VALIDATED_DATA_SUP_POS),

    /// Fully Qualified Data bit supported
    PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_POS      = 8,
    PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_BIT      = CO_BIT(PLXP_MEAS_STATUS_FULLY_QUALIF_DATA_SUP_POS),

    /// Data from Measurement Storage bit supported
    PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_POS    = 9,
    PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_BIT    = CO_BIT(PLXP_MEAS_STATUS_DATA_FROM_MEAS_STRG_SUP_POS),

    /// Data for Demonstration bit supported
    PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_POS          = 10,
    PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_BIT          = CO_BIT(PLXP_MEAS_STATUS_DATA_FOR_DEMO_SUP_POS),

    /// Data for Testing bit supported
    PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_POS          = 11,
    PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_BIT          = CO_BIT(PLXP_MEAS_STATUS_DATA_FOR_TEST_SUP_POS),

    /// Calibration Ongoing bit supported
    PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_POS          = 12,
    PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_BIT          = CO_BIT(PLXP_MEAS_STATUS_CALIB_ONGOING_SUP_POS),

    /// Measurement Unavailable bit supported
    PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_POS          = 13,
    PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_BIT          = CO_BIT(PLXP_MEAS_STATUS_MEAS_UNAVAILB_SUP_POS),

    /// Questionable Measurement Detected bit supported
    PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_POS         = 14,
    PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_BIT         = CO_BIT(PLXP_MEAS_STATUS_QUEST_MEAS_DET_SUP_POS),

    /// Invalid Measurement Detected bit supported
    PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_POS           = 15,
    PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_BIT           = CO_BIT(PLXP_MEAS_STATUS_INV_MEAS_DET_SUP_POS),
};


/// Device and Sensor Status field (not used)
enum plxp_dev_sensor_status_bf
{
    /// Extended Display Update Ongoing
    PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_POS      = 0,
    PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_BIT      = CO_BIT(PLXP_DEV_STATUS_EXT_DISP_UPDATE_ONGOING_POS),

    /// Signal Processing Irregularity Detected bit supported
    PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_POS        = 1,
    PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_BIT        = CO_BIT(PLXP_DEV_STATUS_EQUIP_MALFUNC_DET_SUP_POS),

    /// Signal Processing Irregularity Detected bit supported
    PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_POS  = 2,
    PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_BIT  = CO_BIT(PLXP_DEV_STATUS_SIGNAL_PROC_IRREGUL_DET_SUP_POS),

    /// Inadequate Signal Detected bit supported
    PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_POS        = 3,
    PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_BIT        = CO_BIT(PLXP_DEV_STATUS_INADEQ_SIGNAL_DET_SUP_POS),

    /// Poor Signal Detected bit supported
    PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_POS          = 4,
    PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_BIT          = CO_BIT(PLXP_DEV_STATUS_POOR_SIGNAL_DET_SUP_POS),

    /// Low Perfusion Detected bit supported
    PLXP_DEV_STATUS_LOW_PERF_DET_SUP_POS             = 5,
    PLXP_DEV_STATUS_LOW_PERF_DET_SUP_BIT             = CO_BIT(PLXP_DEV_STATUS_LOW_PERF_DET_SUP_POS),

    /// Erratic Signal Detected bit supported
    PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_POS       = 6,
    PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_BIT       = CO_BIT(PLXP_DEV_STATUS_ERRATIC_SIGNAL_DET_SUP_POS),

    /// Nonpulsatile Signal Detected bit supported
    PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_POS       = 7,
    PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_BIT       = CO_BIT(PLXP_DEV_STATUS_NONPULS_SIGNAL_DET_SUP_POS),

    /// Questionable Pulse Detected bit supported
    PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_POS          = 8,
    PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_BIT          = CO_BIT(PLXP_DEV_STATUS_QUEST_PULSE_DET_SUP_POS),

    /// Signal Analysis Ongoing bit supported
    PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_SUP          = 9,
    PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_BIT          = CO_BIT(PLXP_DEV_STATUS_SIGNAL_ANAL_ONGOING_SUP),

    /// Sensor Interference Detected bit supported
    PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_POS        = 10,
    PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_BIT        = CO_BIT(PLXP_DEV_STATUS_SENSOR_INTERF_DET_SUP_POS),

    /// Sensor Unconnected to User bit supported
    PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_POS = 11,
    PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_BIT = CO_BIT(PLXP_DEV_STATUS_SENSOR_UNCONNCTD_TO_USER_SUP_POS),

    /// Unknown Sensor Connected bit supported
    PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_POS   = 12,
    PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_BIT   = CO_BIT(PLXP_DEV_STATUS_UNKNOWN_SENSOR_CONNCTD_SUP_POS),

    /// Sensor Displaced bit supported
    PLXP_DEV_STATUS_SENSOR_DISPL_SUP_POS             = 13,
    PLXP_DEV_STATUS_SENSOR_DISPL_SUP_BIT             = CO_BIT(PLXP_DEV_STATUS_SENSOR_DISPL_SUP_POS),

    /// Sensor Malfunctioning bit supported
    PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_POS           = 14,
    PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_BIT           = CO_BIT(PLXP_DEV_STATUS_SENSOR_MALFUNC_SUP_POS),

    /// Sensor Disconnected bit supported
    PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_POS        = 15,
    PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_BIT        = CO_BIT(PLXP_DEV_STATUS_SENSOR_DISCONNCTD_SUP_POS),

    // Bit 16-23 RFU
};

#endif /* (BLE_PLXP_CLIENT || BLE_PLXP_SERVER) */

#endif /* _PLXP_COMMON_H_ */
