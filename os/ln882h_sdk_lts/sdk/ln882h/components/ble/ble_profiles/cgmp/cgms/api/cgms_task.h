/**
 ****************************************************************************************
 *
 * @file cgms_task.h
 *
 * @brief Header file - Continuous Glucose Monitoring Profile Service Task.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _CGMS_TASK_H_
#define _CGMS_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup CGMSTASK Task
 * @ingroup CGMS
 * @brief Continuous Glucose Monitoring Profile Task.
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h"
#include "prf_types.h"

#include "cgmp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */


/// minimum configuration:
/// 1oct(Size)+ 1oct(flags) + 2oct(Glucose Concentration) +2oct(Time Offset)
#define CGMS_MEAS_CMD_MIN_LENGTH    (1 +1 +2 +2)

/// size of read Session Start Time request without e2e-crc
#define CGMS_RD_SESS_START_TIME_CFM_SIZE (9)
/// with e2e-crc
#define CGMS_SESS_START_TIME_CFM_SIZE_MAX (9+2)
/// size of read status request without e2e-crc
#define CGMS_RD_STATUS_CFM_SIZE (5)
/// size of read Session Run Time request without e2e-crc
#define CGMS_RD_SESS_RUN_TIME_CFM_SIZE (2)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for Continuous Glucose Monitoring Profile Service
enum cgms_msg_id
{
    /// Enable the CGMP Sensor task at connection
    CGMS_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_CGMS),
    /// Enable the CGMP Sensor task at connection
    CGMS_ENABLE_RSP,
    /// Send the Measurement or Measurement Record
    CGMS_MEAS_VALUE_CMD,

    /// Write to the Session Start Time Indication
    CGMS_WR_SESS_START_TIME_IND,

    /// Read Request for the CGM Status, CGM Session Start Time, CGM Session Run Time
    CGMS_RD_CHAR_REQ_IND,
    /// Read confirmation from the app with data
    CGMS_RD_CHAR_CFM,

    /// Inform Application on the Characteristics CCC descriptor changes
    CGMS_WR_CHAR_CCC_IND,
    /// Write to the Control Point forward to Application
    CGMS_WR_RACP_IND,
    /// Send Control Point Response
    CGMS_RACP_RESP_CMD,
    /// Request for the OPS Control Point
    CGMS_WR_OPS_CTRL_PT_REQ_IND,
    /// Confirmation from OPS Control Point
    CGMS_WR_OPS_CTRL_PT_CFM,
    /// Complete event for the Application commands
    CGMS_CMP_EVT,
};

/// Define Complete Event command (CGMS_CMP_EVT) operation op codes
enum cgms_op_codes
{
    /// CGMS Measurement operation
    CGMS_MEAS_CMD_OP_CODE            = 1,
    /// Special OPS Control Point operation
    CGMS_OPS_CTRL_PT_OP_CODE         = 2,
    /// RACP operation
    CGMS_RACP_CMD_OP_CODE            = 3,
    /// Read Characteristic operation
    CGMS_RD_CHAR_OP_CODE             = 4,
};

/// Define characteristic type
enum cgms_char_type
{
    /// CGM Measurement CCC
    CGMS_CHAR_ID_MEAS_CCC               = 1,
    /// CGM Specific Ops Control Point CCC
    CGMS_CHAR_ID_OPS_CTRL_PT_CCC        = 2,
    /// Record Access Control Point CCC
    CGMS_CHAR_ID_RACP_CCC               = 3,
    /// CGM Status value
    CGMS_CHAR_ID_STATUS_VAL             = 4,
    /// CGM Session Start Time value
    CGMS_CHAR_ID_SESSION_START_TIME_VAL = 5,
    /// CGM Session Run Time value
    CGMS_CHAR_ID_SESSION_RUN_TIME_VAL   = 6,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the initialization function
struct cgms_db_cfg
{
    /// If enabled in CGM Features @see enum cgm_feat_bf
    uint32_t cgm_feature;
    /// Define the Type of Sample @see enum cgmp_type_sample_id
    uint8_t type_sample;
    /// Define the Location of Sample @see enum cgmp_sample_location_id
    uint8_t sample_location;
};

/// Parameters of the @see CGMS_ENABLE_REQ message
struct cgms_enable_req
{
    /**
     * Profile characteristic configuration:
     *   CGM Measurement Characteristic notification config Bit 0
     *   Record Access Control Point indication config      Bit 1
     *   CGM Specific Ops Control Point indication config   Bit 2
     */
    uint16_t prfl_ntf_ind_cfg;
};

/// Parameters of the @see CGMS_ENABLE_RSP message
struct cgms_enable_rsp
{
    /// Status
    uint8_t status;
};

/// CGM Measurement Value
/// Parameters of the @see CGMS_MEAS_VALUE_CMD,
struct cgms_meas_value_cmd
{
    /// CGMS_MEAS_CMD_OP_CODE for this command @see enum cgms_op_codes
    uint8_t operation;
    /// CGM Measurement Characteristic Flags field @see enum cgm_meas_flags_bf
    uint8_t flags;
    /// The CGM Glucose Concentration Field contains the CGM glucose concentration in mg/dL as a SFLOAT data type
    prf_sfloat gluc_concent;
    /// The following values are defined for the Time Offset Field, specifying 
    /// the relative time difference of the single CGM values to the session 
    /// start time. Value: (0x0000 - 0xFFFF)
    /// Description: (Time offset in minutes as offset to the Session Start Time)
    uint16_t time_offset;
    /// The Sensor Status Annunciation Field shall be attached to every CGM Measurement Record
    /// to which the status applies. Bits[16:23] of @see enum cgm_meas_sensor_status_annuc_bf
    uint8_t warn;
    /// The Sensor Status Annunciation Field, bits[8:13] of @see enum cgm_meas_sensor_status_annuc_bf
    uint8_t cal_temp;
    /// The Sensor Status Annunciation Field, bits[0:5] of @see enum cgm_meas_sensor_status_annuc_bf
    uint8_t status;
    /// The CGM Trend Information Field contains the CGM Trend information in (mg/dL)/min as an SFLOAT data type
    /// This field is optional if the device supports CGM Trend information
    /// (Bit 15 in CGM Feature is set to 1) otherwise excluded
    prf_sfloat trend_info;
    /// This field contains the CGM Quality information in % as an SFLOAT data type
    /// The SFLOAT-Type is a 16-bit word comprising a signed 4-bit integer exponent
    /// followed by a signed 12-bit Mantissa, each in twos-complement form
    /// This field is optional if the device supports CGM Quality
    /// (Bit 16 in CGM Feature is set to 1) otherwise excluded
    prf_sfloat quality;
};


/// CGM Status
/// Parameters of the @see CGMS_RD_CHAR_REQ_IND
struct cgms_rd_char_req_ind
{
    /// characteristic id @see cgms_char_type
    uint8_t char_type;
};

/// CGM Status
struct cgms_rd_status
{
    /// The following values are defined for the Time Offset Field, specifying 
    /// the relative time difference of the single CGM values to the session 
    /// start time. Value: (0x0000 - 0xFFFF), 
    /// Description: (Time offset in minutes as offset to the Session Start Time). 
    uint16_t time_offset;
    /// The Sensor Status Annunciation Field shall be attached to
    /// every CGM Measurement Record to which the status applies.
    uint8_t warning;
    /// The Sensor Status Annunciation Field.
    ///  - bitfield @see common Sensor Status Annunciation Field CAL_TEMP
    uint8_t cal_temp;
    /// The Sensor Status Annunciation Field.
    /// - bitfield @see common Sensor Status Annunciation Field STATUS
    uint8_t annunc_status;
};

/// CGM Session Start Time
struct cgms_rd_sess_start_time
{
    /// time of Session Start Time
    struct prf_date_time sess_start_time;
    /// (-48..+56) Offset from UTC in number of 15 minutes increments.
    /// A value of -128 means that the time zone offset is not known.
    /// The offset defined in this characteristic is constant, regardless whether daylight savings is in effect.
    int8_t        time_zone;
    /// DSTOOffset  @see common enum cgmp_dst_offset_id
    uint8_t       dst_offset;
};

/// Parameters of the @see CGMS_WR_SESS_START_TIME_IND message
struct cgms_wr_sess_start_time_ind
{
    /// time of Session Start Time
    struct prf_date_time sess_start_time;
    /// (-48..+56) Offset from UTC in number of 15 minutes increments.
    /// A value of -128 means that the time zone offset is not known.
    /// The offset defined in this characteristic is constant, regardless whether daylight savings is in effect.
    int8_t        time_zone;
    /// DSTOOffset  @see common enum cgmp_dst_offset_id
    uint8_t       dst_offset;
};

/// CGM Session Run Time
struct cgms_rd_sess_run_time
{
    /// Run time in minutes
    uint16_t run_time;
};

/// Parameters of the @see CGMS_RD_CHAR_CFM message
struct cgms_rd_char_cfm
{
    /// characteristic id @see cgms_char_type
    uint8_t char_type;
    /// Operation Status
    uint8_t status;
    /// data for the characteristic
    union 
    {
        /// CGM Status
        struct cgms_rd_status          status;
        /// CGM Session Start Time
        struct cgms_rd_sess_start_time sess_start_time;
        /// CGN Session Run Time
        struct cgms_rd_sess_run_time   sess_run_time;
    } value;
}; 

/// Send Control Point Response
/// Parameters of the @see     CGMS_RACP_RESP_CMD,
struct cgms_racp_resp_cmd
{
    /// Operation = CGMS_RACP_CMD_OP_CODE @see enum cgms_op_codes
    uint8_t operation;
    /// Control Point OpCode @see enum cgmp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Request Control Point OpCode @see enum cgmp_cp_opcodes_id
    uint8_t req_cp_opcode;
    /// Response Code @see enum cgmp_racp_rsp_id
    uint8_t rsp_code;
    /// Number of Records
    uint16_t records_num;
};

/// Write to the Control Point forward to Application
/// Parameters of the @see CGMS_WR_RACP_IND,
struct cgms_wr_racp_ind
{
    /// Control Point OpCode @see enum cgmp_cp_opcodes_id
    uint8_t cp_opcode;
    /// Operator @see enum cgmp_cp_operator_id
    uint8_t cp_operator;
    /// Filter Type
    uint8_t filter_type;
    /// Filter parameter
    uint16_t min_time_offset;
    /// Filter parameter
    uint16_t max_time_offset;
};

/// Inform Application on the Characteristic's CCC descriptor changes
/// Parameters of the @see CGMS_WR_CHAR_CCC_IND,
struct cgms_wr_char_ccc_ind
{
    /// Characteristic type @see enum cgms_char_type
    uint8_t char_type;
    /// Char. Client Characteristic Configuration
    uint16_t ind_cfg;
};


/// Write to the Control Point forward to Application
/// Parameters of the @see CGMS_WR_OPS_CTRL_PT_REQ_IND,
struct cgms_wr_ops_ctrl_pt_req_ind
{
    /// Control Point OpCode @see enum cgmp_spec_ops_opcodes_id
    uint8_t opcode;
    /// Op-code Response  @see enum cgmp_spec_ops_cp_rsp_id
    uint8_t rsp;
    /// operand specific to opcode
    union cgmp_ops_operand operand;
};

/// OPS Control Point Response
/// Parameters of the @see CGMS_WR_OPS_CTRL_PT_CFM,
struct cgms_wr_ops_ctrl_pt_cfm
{
    /// Control Point OpCode @see enum cgmp_spec_ops_opcodes_id
    uint8_t opcode;
    /// Op-code Response  @see enum cgmp_spec_ops_cp_rsp_id
    uint8_t rsp;
    /// operand specific to opcode
    union cgmp_ops_operand operand;
};

/// Send App command complete  event
/// Parameters of the @see CGMS_CMP_EVT message
struct cgms_cmp_evt
{
    /// Operation @see enum cgms_op_codes
    uint8_t operation;
    /// Operation Status
    uint8_t status;
};

/// @} CGMSTASK

#endif //(_CGMS_TASK_H_)
