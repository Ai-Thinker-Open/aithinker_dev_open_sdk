/**
 ****************************************************************************************
 *
 * @file udsc_task.h
 *
 * @brief Header file - User Data Service Collector/Client Role Task.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _UDSC_TASK_H_
#define _UDSC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup UDSCTASK  User Data Service Collector Task
 * @ingroup UDSC
 * @brief  User Data Service Profile Collector Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "uds_common.h"

#include "rwip_task.h"      // Task definitions
#include "prf_types.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Message IDs
/*@TRACE*/
enum udsc_msg_ids
{
    /// Enable the Profile Collector task - at connection
    UDSC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_UDSC),
    /// Response to Enable the Profile Collector task - at connection
    UDSC_ENABLE_RSP,

    ///*** UDS CHARACTERISTIC READ REQUESTS
    UDSC_RD_CHAR_CMD,

    /// Read the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_RD_CHAR_CCC_CMD,

    ///***  CHARACTERISTIC WRITE COMMAND
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_CHAR_CMD,
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_CHAR_UTF8_CMD,

    ///*** UDS CHARACTERISTIC READ RESPONSE
    UDSC_RD_CHAR_IND,
    UDSC_RD_CHAR_UTF8_IND,

    /// Read Response the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_RD_CHAR_CCC_IND,

    ///***  CHARACTERISTIC/DESCRIPTOR WRITE REQUESTS
    /// Write the CCC of a measurement sensor characteristic
    UDSC_WR_USER_CTRL_PT_CMD,

    /// Write the CCC of a DB_CHANGE & USER_CTRL_PT characteristic
    UDSC_WR_CHAR_CCC_CMD,

    /// Characteristic DB_CHG_INC Indication from peer
    UDSC_DB_CHG_INC_IND,
    /// Characteristic User Control Point Indication from peer
    UDSC_USER_CTRL_PT_IND,
    /// Complete Event Information
    UDSC_CMP_EVT,
};

/// User Data Service Service Characteristics
enum udsc_char_idx
{
    /// Strings: utf8s
    UDSC_CHAR_UDS_FIRST_NAME                                        = 0,
    UDSC_CHAR_UDS_LAST_NAME                                         = 1,
    UDSC_CHAR_UDS_EMAIL_ADDRESS                                     = 2,
    UDSC_CHAR_UDS_LANGUAGE                                          = 3,
    /// Date
    UDSC_CHAR_UDS_DATE_OF_BIRTH                                     = 4,
    UDSC_CHAR_UDS_DATE_OF_THRESHOLD_ASSESSMENT                      = 5,
    /// uint16
    UDSC_CHAR_UDS_WEIGHT                                            = 6,
    UDSC_CHAR_UDS_HEIGHT                                            = 7,
    UDSC_CHAR_UDS_WAIST_CIRCUMFERENCE                               = 8,
    UDSC_CHAR_UDS_HIP_CIRCUMFERENCE                                 = 9,
    /// uint8
    UDSC_CHAR_UDS_AGE                                               = 10,
    UDSC_CHAR_UDS_GENDER                                            = 11,
    UDSC_CHAR_UDS_VO2_MAX                                           = 12,
    UDSC_CHAR_UDS_MAX_HEART_RATE                                    = 13,
    UDSC_CHAR_UDS_RESTING_HEART_RATE                                = 14,
    UDSC_CHAR_UDS_MAXIMUM_RECOMMENDED_HEART_RATE                    = 15,
    UDSC_CHAR_UDS_AEROBIC_THRESHOLD                                 = 16,
    UDSC_CHAR_UDS_ANAEROBIC_THRESHOLD                               = 17,
    UDSC_CHAR_UDS_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS   = 18,
    UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_LOWER_LIMIT                   = 19,
    UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_UPPER_LIMIT                   = 20,
    UDSC_CHAR_UDS_AEROBIC_HEART_RATE_LOWER_LIMIT                    = 21,
    UDSC_CHAR_UDS_AEROBIC_HEART_RATE_UPPER_LIMIT                    = 22,
    UDSC_CHAR_UDS_ANAEROBIC_HEART_RATE_LOWER_LIMIT                  = 23,
    UDSC_CHAR_UDS_ANAEROBIC_HEART_RATE_UPPER_LIMIT                  = 24,
    UDSC_CHAR_UDS_TWO_ZONE_HEART_RATE_LIMITS                        = 25,
    /// set
    UDSC_CHAR_UDS_FIVE_ZONE_HEART_RATE_LIMITS                       = 26,
    UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS                      = 27,

    UDSC_CHAR_UDS_USER_DEFINED_1                                    = 28,
    UDSC_CHAR_UDS_USER_DEFINED_2                                    = 29,
    UDSC_CHAR_UDS_USER_DEFINED_3                                    = 30,
    UDSC_CHAR_UDS_USER_DEFINED_4                                    = 31,

    /// Database Change Increment
    UDSC_CHAR_UDS_DB_CHG_INC                                        = 32,
    /// User Index
    UDSC_CHAR_UDS_USER_INDEX                                        = 33,
    /// User Control Point
    UDSC_CHAR_UDS_USER_CTRL_PT                                      = 34,

    UDSC_CHAR_UDS_MAX,
};

/// User Data Service Service Characteristic Descriptors
enum udsc_desc_idx
{
    /// Database Change Increment Client config
    UDSC_DESC_UDS_DB_CHG_INC_CCC,
    /// User Control Point Client config
    UDSC_DESC_UDS_USER_CTRL_PT_CCC,
    /// Extended
    UDSC_DESC_UDS_FIRST_NAME_EXT,
    UDSC_DESC_UDS_LAST_NAME_EXT,
    UDSC_DESC_UDS_EMAIL_ADDRESS_EXT,
    UDSC_DESC_UDS_LANGUAGE_EXT,

    UDSC_DESC_UDS_MAX,
};

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the User Data Service Service
 */
struct udsc_uds_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic info:
    ///  - Feature
    ///  - Measurement
    struct prf_char_inf chars[UDSC_CHAR_UDS_MAX];

    /// Descriptor handles:
    ///  - Client cfg
    struct prf_char_desc_inf descs[UDSC_DESC_UDS_MAX];
};

/*
 * API MESSAGE STRUCTURES
 ****************************************************************************************
 */

/// Parameters of the @ref UDSC_ENABLE_REQ message
struct udsc_enable_req
{
    /// Connection type
    uint8_t con_type;
    /// Existing handle values UDSC ES
    struct udsc_uds_content uds;
};

/// Parameters of the @ref UDSC_ENABLE_RSP message
struct udsc_enable_rsp
{
    /// status
    uint8_t status;
    /// Existing handle values UDSC ES
    struct udsc_uds_content uds;
};

///*** UDS CHARACTERISTIC/DESCRIPTOR READ REQUESTS
/// Parameters of the @ref UDSC_RD_CHAR_CMD message
/// Read the User Data Service Feature
struct udsc_rd_char_cmd
{
    /// Characteristic from @ref enum uds_char_id
    uint8_t char_idx;
};

/// Parameters of the @ref UDSC_RD_CHAR_CCC_CMD message
/// Read the CCC of a measurement sensor characteristic
struct udsc_rd_char_ccc_cmd
{
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t char_idx;
};

///***  CHARACTERISTIC/DESCRIPTOR WRITE REQUESTS
/// Parameters of the @ref UDSC_WR_CHAR_CCC_CMD message
/// Write the CCC to a selected characteristic
struct udsc_wr_char_ccc_cmd
{
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t char_idx;
    /// CCC value
    uint16_t ccc;
};

///*** UDS CHARACTERISTIC/DESCRIPTOR READ RESPONSE
/// Parameters of the @ref UDSC_RD_CHAR_IND message
/// Read the User Data Service Characteristic
struct udsc_rd_char_ind
{
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t char_idx;
    /// Characteristic indication value
    union char_val value;
};

/// Parameters of the @ref UDSC_WR_CHAR_UTF8_CMD message
/// Write to User Data Service UTF8 Characteristic
struct udsc_wr_char_utf8_cmd
{
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t char_idx;
    /// UTF Name
    struct utf_8_name utf_name;
};

/// Parameters of the @ref UDSC_WR_CHAR_CMD message
/// Write to User Data Service Characteristic
struct udsc_wr_char_cmd
{
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t char_idx;
    /// Characteristic value
    union char_val value;
};

/// Parameters of the @ref UDSC_RD_CHAR_UTF8_IND message
/// Read the User Data Service UTF8 Characteristic
struct udsc_rd_char_utf8_ind
{
    /// Characteristic from @ref enum udsc_char_idx
    uint8_t char_idx;
    /// UTF Name
    struct utf_8_name utf_name;
};

/// Parameters of the @ref UDSC_RD_CHAR_CCC_IND message
/// Read the CCC of a measurement sensor characteristic
struct udsc_rd_char_ccc_ind
{
    /// Characteristic from @ref enum udsc_char_idx
    /// UDSC_CHAR_UDS_DB_CHG_INC, UDSC_CHAR_UDS_USER_CTRL_PT
    uint8_t char_idx;
    /// Indication Configuration
    uint16_t ccc;
};

/// Parameters of the @ref UDSC_DB_CHG_INC_IND message
/// Characteristic Measurement Indication from peer
struct udsc_db_chg_inc_ind
{
    /// Database change increment value
    uint32_t value;
};

/// Parameters of the @ref UDSC_USER_CTRL_PT_IND message
/// User Data Service User Control Point Characteristic Indication
struct udsc_user_ctrl_pt_ind
{
    /// response op-code =0x20
    uint8_t resp_code;
    /// requested op-code
    uint8_t req_op_code;
    /// Response value
    uint8_t resp_value;
    /// User ID used for register new user command
    uint8_t user_id;
    /// Parameter length
    uint8_t length;
    /// Parameter value - for the future extension  to send additional commands
    uint8_t parameter[__ARRAY_EMPTY];
};

/// Parameters of the @ref UDSC_WR_USER_CTRL_PT_CMD message
/// Write to User Data Service User Control Point Characteristic
struct udsc_wr_user_ctrl_pt_cmd
{
    /// User Control Point Op-codes @see enum uds_ctrl_pt_op_code
    uint8_t op_code;
    /// User Index
    uint8_t user_id;
    /// used for sending consent command
    uint16_t consent;
    /// Parameter length
    uint8_t length;
    /// Parameter value - for the future extension  to send additional commands
    uint8_t parameter[__ARRAY_EMPTY];
};

/// Parameters of the @ref UDSC_CMP_EVT message
struct udsc_cmp_evt
{
    /// Operation code
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} UDSCTASK

#endif //(_UDSC_TASK_H_)
