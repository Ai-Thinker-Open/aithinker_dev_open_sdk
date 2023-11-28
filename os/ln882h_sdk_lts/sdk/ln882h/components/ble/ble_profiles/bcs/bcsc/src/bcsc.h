/**
 ****************************************************************************************
 *
 * @file bcsc.h
 *
 * @brief Header file - Body composition service - Collector Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSC_H_
#define _BCSC_H_

/**
 ****************************************************************************************
 * @addtogroup BCSC Body Composition Service Collector
 * @ingroup BCSC
 * @brief Body Composition Service Collector
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Body Composition instances
#define BCSC_IDX_MAX        (BLE_CONNECTION_MAX)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "bcs_common.h"

#if (BLE_BCS_CLIENT)

#include "ke_task.h"
#include "prf_types.h"
#include "prf_utils.h"

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the BCSC task
enum bcsc_states
{
    /// Idle state
	BCSC_FREE,
    /// Connected state
	BCSC_IDLE,
    /// Discovering Services
    BCSC_DISCOVERING_SVC,
    /// Discovering Characteristics
    BCSC_DISCOVERING_CHARS,
    /// Discovering Characteristic Descriptors
    BCSC_DISCOVERING_CHAR_DESC,
    /// Busy state
    BCSC_BUSY,
    /// Number of defined states.
    BCSC_STATE_MAX
};

/// Operation Codes for pending operations in the client.
enum bcsc_op_codes
{
    /// Read feature operation code
    BCSC_READ_FEAT_OP_CODE = 0,
    /// Read CCC operation code
    BCSC_READ_CCC_OP_CODE = 1,
    /// Write CCC operation codes
    BCSC_WRITE_CCC_OP_CODE = 2,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Stored Environment content/context information for a connection
struct bcsc_cnx_env
{
    /// Current Operation
    uint8_t op_pending;

    /// Counter used to check service uniqueness
    uint8_t nb_svc;

    /// Store the @ref bcsc_meas_ind param when Multipacket presents
    struct ke_msg* meas_evt_msg;

    /// Current Operation
    void *operation;

    /// Weight Scale Service Characteristics
    struct bcsc_bcs_content bcs;
};

/// BCSC environment variable
struct bcsc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct bcsc_cnx_env* env[BCSC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[BCSC_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Pool of Environmental Profile Collector task environments.
extern struct bcsc_env_tag **bcsc_envs;

/*
 * GLOBAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve Body Compistion serice client interface
 *
 * @return Body Composition client interface
 ****************************************************************************************
 */
const struct prf_task_cbs* bcsc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Body Composition ATT DB discovery results to BCSC host.
 *
 * @param[in] p_bcsc_env   The profile environment for the connection
 * @param[in] conidx       Connection Identifier
 * @param[in] status       Indicates the outcome of the operation
 ****************************************************************************************
 */
void bcsc_enable_rsp_send(struct bcsc_env_tag *p_bcsc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send a BCSC_CMP_EVT message to the task which enabled the profile
 *
 * @param[in] p_bcsc_env   The profile environment
 * @param[in] conidx       Connection Identifier
 * @param[in] operation    Indicates the operation for which the cmp_evt is being sent.
 * @param[in] status       Indicates the outcome of the operation
 ****************************************************************************************
 */
void bcsc_send_cmp_evt(struct bcsc_env_tag *p_bcsc_env, uint8_t conidx, uint8_t operation, uint8_t status);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param p_task_desc Task descriptor to fill
 ****************************************************************************************
 */
void bcsc_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_BCS_CLIENT)

/// @} BCSC

#endif //(_BCSC_H_)
