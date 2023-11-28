/**
 ****************************************************************************************
 *
 * @file plxs_task.h
 *
 * @brief Header file - Pulse Oximeter Profile Service Task.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _PLXS_TASK_H_
#define _PLXS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup PLXSTASK Task
 * @ingroup PLXS
 * @brief Pulse Oximeter Profile Task.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "prf_types.h"

#include "plxp_common.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Pulse Oximeter Profile Service
enum plxs_msg_id
{
    /// Enable the PLXP Sensor task for a connection
    PLXS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_PLXS),
    /// Enable the PLXP Sensor task for a connection
    PLXS_ENABLE_RSP,
    /// Send the Spot-check Measurement or Measurement Record
    /// Send Continuous Measurement
    PLXS_MEAS_VALUE_CMD,
    /// Send Control Point Response
    PLXS_RACP_RESP_CMD,
    /// Inform Application on the Characteristics CCC descriptor changes
    PLXS_WR_CHAR_CCC_IND,
    /// Write to the Control Point forward to Application
    PLXS_WR_RACP_IND,
    /// Complete event for the Application commands
    PLXS_CMP_EVT,
};

/// type_of_operation - Additional to Specification to select the type of operation
enum plxs_optype_id
{
    PLXS_OPTYPE_SPOT_CHECK_AND_CONTINUOUS   = 0, 
    PLXS_OPTYPE_SPOT_CHECK_ONLY             = 1, 
    PLXS_OPTYPE_CONTINUOUS_ONLY             = 2
};

/// Define command operation
enum plxs_op_codes
{
    PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE = 1,
    PLXS_CONTINUOUS_MEAS_CMD_OP_CODE = 2,
    PLXS_RACP_CMD_OP_CODE            = 3,
};

/// Define characteristic type
enum plxs_char_types
{
    PLXS_SPOT_CHECK_MEAS_CODE    = 1,
    PLXS_CONTINUOUS_MEAS_CODE    = 2,
    PLXS_RACP_CODE               = 3,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct plxs_db_cfg
{
    /// Define the Type of Operation @see enum plxs_optype_id
    uint8_t optype;
    /// Supported Features @see common enum plxp_sup_feat_bf
    uint16_t sup_feat;
    /// If enabled in  Supported Features @see enum plxp_meas_status_sup_bf
    uint16_t meas_stat_sup;
    /// If enabled in  Supported Features @see enum plxp_dev_sensor_status_bf
    uint32_t dev_stat_sup;
};

/// Parameters of the @see PLXS_ENABLE_REQ message
struct plxs_enable_req
{
    /// CCC for the current connection
    uint16_t ind_cfg_spot_meas;
    /// CCC for the current connection
    uint16_t ind_cfg_cont_meas;
    /// CCC for the current connection
    uint16_t ind_cfg_racp_meas;
};

/// Parameters of the @see PLXS_ENABLE_RSP message
struct plxs_enable_rsp
{
    /// Status
    uint8_t status;
};

/// PLX Spot-Check Measurement Value
struct plxs_spot_meas_value
{
    /// bitfield of supplied data @see common plxp_spot_check_flags_bf
    uint8_t spot_flags;
    /// Percentage with a resolution of 1
    uint16_t sp_o2;
    /// Period (beats per minute) with a resolution of 1
    uint16_t pr;
    /// Timestamp (if present)
    struct prf_date_time timestamp;
    /// Measurement Status (bitfield) @see common measurement_status_supported
    uint16_t meas_stat;
    /// Device and Sensor Status (bitfield) @see common device_status_supported
    uint32_t dev_sensor_stat;
    /// Pulse Amplitude Index - Unit is percentage with a resolution of 1
    uint16_t pulse_ampl;
};

/// SpO2 - PR measurements
struct plxs_spo2pr_value
{
    /// Percentage with a resolution of 1
    uint16_t sp_o2;
    /// Period (beats per minute) with a resolution of 1
    uint16_t pr;
};

/// PLX Continuous Measurement Value
struct plxs_cont_meas_value
{
    /// bitfield of supplied data @see common plxp_cont_meas_flag
    uint8_t cont_flags;
    /// SpO2 - PR measurements - Normal
    struct plxs_spo2pr_value normal;
    /// SpO2 - PR measurements - Fast
    struct plxs_spo2pr_value fast;
    /// SpO2 - PR measurements - Slow
    struct plxs_spo2pr_value slow;
    /// Measurement Status (bitfield) @see common measurement_status_supported
    uint16_t meas_stat;
    /// Device and Sensor Status (bitfield) @see common device_status_supported
    uint32_t dev_sensor_stat;
    /// Pulse Amplitude Index - Unit is percentage with a resolution of 1
    uint16_t pulse_ampl;
};

/// Send the Spot-check Measurement or Measurement Record
/// Send Continuous Measurement
/// Parameters of the @see PLXS_MEAS_VALUE_CMD,
struct plxs_meas_value_cmd
{
    /// Operation @see enum plxs_op_codes
    uint8_t operation;
    union plxs_meas_value_tag
    {
        /// Spot-Check Measurement
        struct plxs_spot_meas_value spot_meas;
        /// Continuous Measurement
        struct plxs_cont_meas_value cont_meas;
    } value;
};

/// Send Control Point Response
/// Parameters of the @see PLXS_RACP_RESP_CMD,
struct plxs_racp_resp_cmd
{
    /// Operation = PLXS_RASP_CMD_OP_CODE @see enum plxs_op_codes
    uint8_t operation;
    /// Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Request Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t req_cp_opcode;
    /// Response Code @see enum plxp_cp_resp_code_id
    uint8_t rsp_code;
    /// Number of Records
    uint16_t rec_num;
};

/// Write to the Control Point forward to Application
/// Parameters of the @see PLXS_WR_RACP_IND,
struct plxs_wr_racp_ind
{
    /// Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Operator  @see enum plxp_cp_operator_id
    uint8_t cp_operator;
};

/// Inform Application on the Characteristic's CCC descriptor changes
/// Parameters of the @see PLXS_WR_CHAR_CCC_IND,
struct plxs_wr_char_ccc_ind
{
    /// CHaracteristic type @see enum plxs_char_types
    uint8_t char_type;
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};

/// Parameters of the @see PLXS_CMP_EVT message
struct plxs_cmp_evt
{
    /// Operation @see enum plxs_op_codes
    uint8_t operation;
    /// Operation Status
    uint8_t status;
};

/// @} PLXSTASK

#endif //(_PLXS_TASK_H_)
