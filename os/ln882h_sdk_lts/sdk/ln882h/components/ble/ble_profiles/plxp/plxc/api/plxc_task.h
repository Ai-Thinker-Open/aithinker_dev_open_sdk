/**
 ****************************************************************************************
 *
 * @file plxc_task.h
 *
 * @brief Header file - Pulse Oximeter Profile Collector/Client Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _PLXC_TASK_H_
#define _PLXC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup PLXCTASK  Pulse Oximeter Collector Task
 * @ingroup PLXC
 * @brief  Pulse Oximeter ProfileProfile Collector Task
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


/// Message IDs
enum plxc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    PLXC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_PLXC),
    /// Response to Enable the Profile Collector task - at connection
    PLXC_ENABLE_RSP,
    /// Read Characteristic
    PLXC_READ_CMD,
    /// Configure Characteristic's CCC descriptor
    PLXC_CFG_CCC_CMD,
    /// Write Command to the Control Point
    PLXC_WRITE_RACP_CMD,
    /// Receive the Spot-check Measurement or Measurement Record Indication SPOT_MEAS
    /// *   Receive Continuous Measurement Notification CONT_MEAS
    /// *   Receive Control Point Response Indication RACP_RESP
    PLXC_VALUE_IND,
    /// Read CCC value of specific characteristic
    PLXC_RD_CHAR_CCC_IND,

    /// Complete event for the Application commands
    PLXC_CMP_EVT,
};


/// Pulse Oximeter Service Characteristics
enum plxc_char_ids
{
    /// PLXP SPOT-Measurement Characteristic
    PLXC_CHAR_SPOT_MEASUREMENT = 0,
    /// PLXP Continuous Measurement Characteristic
    PLXC_CHAR_CONT_MEASUREMENT,
    /// PLXP Features Characteristic
    PLXC_CHAR_FEATURES,
    /// PLXP Record Access Control Point Characteristic
    PLXC_CHAR_RACP,

    PLXC_CHAR_MAX,
};

/// Pulse Oximeter Service Characteristic Descriptors
enum plxc_desc_ids
{
    /// PLXP SPOT-Measurement Characteristic Client Configuration
    PLXC_DESC_SPOT_MEASUREMENT_CCC = 0,
    /// PLXP Continuous Measurement Characteristic Client Configuration
    PLXC_DESC_CONT_MEASUREMENT_CCC,
    /// PLXP Record Access Control Point Characteristic Client Configuration
    PLXC_DESC_RACP_CCC,

    PLXC_DESC_MAX,
};

/// Define command operation
enum plxc_op_codes
{
    PLXC_NO_OP                       = 0,
    PLXC_SPOT_CHECK_MEAS_CMD_OP_CODE = 1,
    PLXC_CONTINUOUS_MEAS_CMD_OP_CODE = 2,
    PLXC_RACP_CMD_OP_CODE            = 3,

    PLXC_OP_CODE_READ_FEATURES       = 4,
    PLXC_OP_CODE_READ_CCC            = 5,

    PLXC_OP_CODE_WRITE_CCC           = 6,
    PLXC_OP_CODE_WRITE_RACP          = 7,
    PLXC_OP_CODE_GATTC_REGISTER      = 8,
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Pulse Oximeter Service
 */
struct plxc_plxp_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    struct prf_char_inf chars[PLXC_CHAR_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    struct prf_char_desc_inf descs[PLXC_DESC_MAX];
};

/// Parameters of the @see PLXC_ENABLE_REQ message
struct plxc_enable_req
{
    /// Connection type
    uint8_t con_type;
    /// Existing handle values PLXC
    struct plxc_plxp_content plxc;
};

/// Parameters of the @see PLXC_ENABLE_RSP message
struct plxc_enable_rsp
{
    /// status
    uint8_t status;
    /// Existing handle values PLXC
    struct plxc_plxp_content plxc;
    /// Supported Features @see common supported_features
    uint16_t sup_feat;
    /// If enabled in  Supported Features @see common measurement_status_supported
    uint16_t meas_stat_sup;
    /// If enabled in  Supported Features @see common device_status_supported
    uint32_t dev_stat_sup;
};

///*** PLXC CHARACTERISTIC READ REQUESTS
/// Parameters of the @see PLXC_READ_CMD message
/// operation
struct plxc_read_cmd
{
    /// Operation - select characteristic @see enum plxc_op_codes
    uint8_t operation;
};

struct plxc_read_cmd_value
{
    /// Supported Features @see common supported_features
    uint16_t sup_feat;
    /// If enabled in  Supported Features @see common measurement_status_supported
    uint16_t meas_stat_sup;
    /// If enabled in  Supported Features @see common device_status_supported
    uint32_t dev_stat_sup;
};

///*** PLXC CHARACTERISTIC/CCC DESCRIPTOR  WRITE REQUESTS
/// Parameters of the @see PLXC_CFG_CCC_CMD message
/// operation
struct  plxc_cfg_ccc_cmd
{
    /// operation = char_opcode - select characteristic @see enum plxc_op_codes
    uint8_t operation;
    /// The Client Characteristic Configuration Value
    uint16_t ccc;
};

/// Write Operation Command to the Control Point forward to Application
/// Parameters of the @see PLXC_WRITE_RACP_CMD,
struct plxc_write_racp_cmd
{
    /// Operation = PLXC_RASP_CMD_OP_CODE @see enum plxc_op_codes
    uint8_t operation;
    /// Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Operator  @see enum plxp_cp_operator_id
    uint8_t cp_operator;
};

/// Receive the Spot-check Measurement or Measurement Record Indication SPOT_MEAS
/// *   Receive Continuous Measurement Notification CONT_MEAS
/// *   Receive Control Point Response Indication RACP_RESP
///    PLXC_VALUE_IND,
/// Indication Record Access Control Point Response
struct plxc_racp_resp
{
    /// Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Request Control Point OpCode @see enum plxp_cp_opcodes_id
    uint8_t req_cp_opcode;
    /// Control Point Operator @see enum plxp_cp_operator_id
    uint8_t cp_operator;
    /// Response Code @see enum plxp_cp_resp_code_id
    uint8_t rsp_code;
    /// Number of Records
    uint16_t rec_num;
};

/// PLX Spot-Check Measurement Value
struct plxc_spot_meas_value
{
    /// bitfield of supplied data @see common plxp_spot_meas_flag
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
struct plxc_spo2pr_value
{
    /// Percentage with a resolution of 1
    uint16_t sp_o2;
    /// Period (beats per minute) with a resolution of 1
    uint16_t pr;
};

/// PLX Continuous Measurement Value
struct plxc_cont_meas_value
{
    /// bitfield of supplied data @see common plxp_cont_meas_flag
    uint8_t cont_flags;
    /// SpO2 - PR measurements - Normal
    struct plxc_spo2pr_value normal;
    /// SpO2 - PR measurements - Fast
    struct plxc_spo2pr_value fast;
    /// SpO2 - PR measurements - Slow
    struct plxc_spo2pr_value slow;
    /// Measurement Status (bitfield) @see common measurement_status_supported
    uint16_t meas_stat;
    /// Device and Sensor Status (bitfield) @see common device_status_supported
    uint32_t dev_sensor_stat;
    /// Pulse Amplitude Index - Unit is percentage with a resolution of 1
    uint16_t pulse_ampl;
};

/// Indication the Record Access Control Point response
/// Indication the Spot-check Measurement or Measurement Record
/// Indication Continuous Measurement
/// Parameters of the @see PLXC_VALUE_IND,
struct plxc_value_ind
{
    /// Operation @see enum plxc_op_codes
    uint8_t operation;
    union plxc_meas_value_tag
    {
        /// Spot-Check Measurement
        struct plxc_spot_meas_value spot_meas;
        /// Continuous Measurement
        struct plxc_cont_meas_value cont_meas;
        /// Record Access Control Point response
        struct plxc_racp_resp racp_resp;
        /// Read Features request command value
        struct  plxc_read_cmd_value features;
    } value;
};

/// Inform Application about the Characteristic's CCC descriptor
/// Parameters of the @see PLXC_RD_CHAR_CCC_IND,
struct plxc_rd_char_ccc_ind
{
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};

/// Complete event for the Application commands
///    PLXC_CMP_EVT,
/// Parameters of the @see PLXC_CMP_EVT message
/// Complete Event Information
struct  plxc_cmp_evt
{
    /// Operation  @see enum plxc_op_codes
    uint8_t operation;
    /// Status
    uint8_t status;
};


/// @} PLXCTASK

#endif //(_PLXC_TASK_H_)
