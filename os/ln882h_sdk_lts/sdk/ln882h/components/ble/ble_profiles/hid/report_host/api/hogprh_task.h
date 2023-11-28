/**
 ****************************************************************************************
 *
 * @file hogprh_task.h
 *
 * @brief Header file - HOGP REPORT HOST TASK.
 *
 * Copyright (C) LIGHTNINGSEMI 2020-2026
 *
 *
 ****************************************************************************************
 */

#ifndef HOGPRH_TASK_H_
#define HOGPRH_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup HOGP REPORT HOST Task
 * @ingroup HOGPRH
 * @brief HID Over GATT Profile REPORT HOST Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 #include "rwprf_config.h"
 #if (BLE_HID_REPORT_HOST)
#include <stdint.h>
#include "rwip_task.h" // Task definitions
#include "prf_types.h"
#include "co_math.h"

/*
 * DEFINES
 ****************************************************************************************
 */




/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Messages for 
/*@TRACE*/
enum hogprh_msg_id
{
    //start HID Over GATT Profile Device
    HOGPRH_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_HOGPRH),
    /// Confirm profile
    HOGPRH_ENABLE_RSP,

    HOGPRH_RD_CHAR_REQ,
    HOGPRH_RD_CHAR_IND,

    HOGPRH_RD_REQ,
    
    HOGPRH_WR_REQ,

    HOGPRH_NTY_IND,

    HOGPRH_CMP_EVT,
    
    HOGPRH_MSG_MAX,
 
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */
struct hogprh_cfg
{
    uint8_t report_nb;
    uint8_t report_id[8];
    uint8_t reprot_char_cfg[8];
};

struct hogprh_db_cfg
{
    uint8_t hid_nb;
    struct hogprh_cfg cfg;  
};

/// Characteristics
enum hogprh_info
{
    HOGPRH_INCLUDE_CHAR,
    HOGPRH_HID_INFO_CHAR,
    HOGPRH_REPORT_MAP_CHAR,
    HOGPRH_REPORT_INPUT_KB_CHAR,
    HOGPRH_REPORT_INPUT_CONS_CTRL_CHAR,
    HOGPRH_REPORT_INPUT_AUDIO_DATA_CHAR,
    HOGPRH_REPORT_INPUT_AUDIO_CFG_CHAR,
    HOGPRH_REPORT_OUTPUT_AUDIO_STATE_CHAR,
    HOGPRH_REPORT_DIAG_CHAR,
    HOGPRH_HID_CTRL_POINT_CHAR,

    HOGPRH_CHAR_MAX,
};

/// Operation codes
enum disc_op_codes
{
    /// Database Creation Procedure
    HOGPRH_RESERVED_OP_CODE = 0,

    /// Read char. Operation Code
    HOGPRH_RD_CHAR_CMD_OP_CODE = 1,
    HOGRPH_WR_CMD_OP_CODE = 2,
};

enum hogprh_dfu_info
{
    HOGPRH_DFU_ID_CHAR,
    HOGPRH_DFU_AUTH_CHAR,
    HOGPRH_DFU_DATA_CHAR,

    HOGPRH_DFU_CHAR_MAX
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the HID report host
 */
struct hogprh_device_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic info:
    struct prf_char_inf chars[HOGPRH_CHAR_MAX];
};

/// Parameters of the @ref HOGPRH_ENABLE_REQ message
struct hogprh_enable_req
{
    ///Connection type
    uint8_t con_type;

    /// Existing handle values device
    struct hogprh_device_content device;
};

/// Parameters of the @ref HOGPRH_ENABLE_RSP message
struct hogprh_enable_rsp
{
    ///status
    uint8_t status;

    /// DIS handle values and characteristic properties
    struct hogprh_device_content device;
};

///Parameters of the @ref HOGPRH_RD_CHAR_CMD message
struct hogprh_rd_char_cmd
{
    ///Characteristic value code
    uint8_t char_code;
};

///Parameters of the @ref HOGPRH_RD_CHAR_IND message
struct hogprh_rd_char_ind
{
    /// Attribute data information
    struct prf_att_info info;
};

struct hogprh_rd_cmd
{
    ///handle
    uint16_t handle;
};


///Parameters of the @ref HOGPRH_CMP_EVT message
struct hogprh_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Status
    uint8_t status;
};

struct hogprh_wr_cmd
{
    uint8_t char_idx;
    uint8_t value;
};

struct hogprh_notify_ind
{
    uint8_t type;
    uint16_t length;
    uint16_t value[0];
};

#endif

/// @} HOGPRHTASK
#endif // HOGPRH_TASK_H_

