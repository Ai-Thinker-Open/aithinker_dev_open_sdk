/**
 ****************************************************************************************
 *
 * @file disc_task.h
 *
 * @brief Header file - DISCTASK.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef DISC_TASK_H_
#define DISC_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup DISCTASK Device Information Service Client Task
 * @ingroup DISC
 * @brief Device Information Service Client Task
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_task.h" // Task definitions
#include "prf_types.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*@TRACE*/
enum disc_msg_id
{
    /// Start the find me locator profile - at connection
    DISC_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_DISC),
    /// Confirm that cfg connection has finished with discovery results, or that normal cnx started
    DISC_ENABLE_RSP,

    /// Generic message to read a DIS characteristic value
    DISC_RD_CHAR_CMD,
    /// Generic message for read responses for APP
    DISC_RD_CHAR_IND,
    /// Complete Event Information
    DISC_CMP_EVT,
};

/// Characteristics
enum
{
    /// Manufacturer Name String
    DISC_MANUFACTURER_NAME_CHAR,
    /// Model Number String
    DISC_MODEL_NB_STR_CHAR,
    /// Serial Number String
    DISC_SERIAL_NB_STR_CHAR,
    /// Hardware Revision String
    DISC_HARD_REV_STR_CHAR,
    /// Firmware Revision String
    DISC_FIRM_REV_STR_CHAR,
    /// Software Revision String
    DISC_SW_REV_STR_CHAR,
    /// System ID
    DISC_SYSTEM_ID_CHAR,
    /// IEEE 11073-20601 Regulatory Certification Data List
    DISC_IEEE_CHAR,
    /// PnP ID
    DISC_PNP_ID_CHAR,

    DISC_CHAR_MAX,
};

/// Operation codes
enum disc_op_codes
{
    /// Database Creation Procedure
    DISC_RESERVED_OP_CODE = 0,

    /// Read char. Operation Code
    DISC_RD_CHAR_CMD_OP_CODE = 1,
};

/*
 * API MESSAGES STRUCTURES
 ****************************************************************************************
 */

/**
 * Structure containing the characteristics handles, value handles and descriptors for
 * the Device Information Service
 */
struct disc_dis_content
{
    /// service info
    struct prf_svc svc;

    /// Characteristic info:
    struct prf_char_inf chars[DISC_CHAR_MAX];
};

/// Parameters of the @ref DISC_ENABLE_REQ message
struct disc_enable_req
{
    ///Connection type
    uint8_t con_type;

    /// Existing handle values dis
    struct disc_dis_content dis;
};

/// Parameters of the @ref DISC_ENABLE_RSP message
struct disc_enable_rsp
{
    ///status
    uint8_t status;

    /// DIS handle values and characteristic properties
    struct disc_dis_content dis;
};

///Parameters of the @ref DISC_RD_CHAR_CMD message
struct disc_rd_char_cmd
{
    ///Characteristic value code
    uint8_t char_code;
};

///Parameters of the @ref DISC_RD_CHAR_IND message
struct disc_rd_char_ind
{
    /// Attribute data information
    struct prf_att_info info;
};

///Parameters of the @ref DISC_CMP_EVT message
struct disc_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} DISCTASK

#endif // DISC_TASK_H_
