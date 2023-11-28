/**
 ****************************************************************************************
 *
 * @file proxm_task.h
 *
 * @brief Header file - PROXMTASK.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef PROXM_TASK_H_
#define PROXM_TASK_H_

/**
 ****************************************************************************************
 * @addtogroup PROXMTASK Proximity Monitor Task
 * @ingroup PROXM
 * @brief Proximity Monitor Task
 *
 * The Proximity Monitor Task is responsible for handling the API messages received from
 * either the Application or internal tasks.
 *
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

/// Max number of characteristics for all services
#define PROXM_CHAR_NB_MAX       1

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Proximity Monitor API messages
/*@TRACE*/
enum proxm_msg_id
{
    /// Proximity Monitor role enable request from application.
    PROXM_ENABLE_REQ = TASK_FIRST_MSG(TASK_ID_PROXM),
    /// Proximity Monitor role enable confirmation to application.
    PROXM_ENABLE_RSP,

    /// Read LLS Alert or TX Power Level - request
    PROXM_RD_CMD,
    /// Read LLS Alert or TX Power Level - response
    PROXM_RD_IND,

    /// Set Alert level
    PROXM_WR_ALERT_LVL_CMD,

    /// Complete Event Information
    PROXM_CMP_EVT,
};

enum
{
    PROXM_LK_LOSS_SVC,
    PROXM_IAS_SVC,
    PROXM_TX_POWER_SVC,

    PROXM_SVC_NB
};

/// Operation codes
enum proxm_op_codes
{
    /// Database Creation Procedure
    PROXM_RESERVED_OP_CODE = 0,
    /// Read Operation Code
    PROXM_RD_OP_CODE = 1,
    /// Set Alert Level Operation Code
    PROXM_WR_ALERT_LVL_OP_CODE = 2,
};

//

/*
 * API Messages Structures
 ****************************************************************************************
 */

/// Service information
struct svc_content
{
    /// Service info
    struct prf_svc svc;

    /// Characteristic info:
    /// - Alert Level for IAS and LLS
    /// - TX Power Level for TXPS
    struct prf_char_inf characts[PROXM_CHAR_NB_MAX];
};

/// Proximity monitor enable command structure
struct proxm_enable_req
{
    /// Connection type
    uint8_t con_type;

    /// Reporter LLS details kept in APP
    struct svc_content lls;
    /// Reporter IAS details kept in APP
    struct svc_content ias;
    /// Reporter TPS details kept in APP
    struct svc_content txps;
};

/// Proximity monitor enable confirm structure
struct proxm_enable_rsp
{
    /// Status
    uint8_t status;

    /// Reporter LLS details to keep in APP
    struct svc_content lls;
    /// Reporter IAS details to keep in APP
    struct svc_content ias;
    /// Reporter TPS details to keep in APP
    struct svc_content txps;
};

/// Parameters of the @ref PROXM_WR_ALERT_LVL_CMD message
struct proxm_wr_alert_lvl_cmd
{
    /// 0=LLS or 1=IAS, code for the service in which the alert level should be written
    uint8_t svc_code;
    /// Alert level
    uint8_t lvl;
};

/// Parameters of the @ref PROXM_RD_CMD message
struct proxm_rd_cmd
{
    /// 0=LLS or 1=TXPS, code for the service in which the alert level should be read @see enum proxm_rd_char_code
    uint8_t svc_code;
};

/// Parameters of the @ref PROXM_RD_IND message
struct proxm_rd_ind
{
    /// 0=LLS or 1=TXPS, code for the service in which the alert level should be read
    uint8_t svc_code;
    /// Value
    uint8_t value;
};

/// Parameters of the @ref PROXM_CMP_EVT message
struct proxm_cmp_evt
{
    /// Operation
    uint8_t operation;
    /// Status
    uint8_t status;
};

/// @} PROXMTASK

#endif // PROXM_TASK_H_
