/**
 ****************************************************************************************
 *
 * @file plxc.h
 *
 * @brief Header file - Pulse Oximeter Profile - Collector Role.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _PLXC_H_
#define _PLXC_H_

/**
 ****************************************************************************************
 * @addtogroup PLXC Pulse Oximeter Profile Collector
 * @ingroup WSCP
 * @brief Pulse Oximeter Profile Collector
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Pulse Oximeter Task instances
#define PLXC_IDX_MAX        (BLE_CONNECTION_MAX)

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "plxp_common.h"

#if (BLE_PLX_CLIENT)

#include "ke_task.h"
#include "prf_types.h"
#include "prf_utils.h"

//#define CCC_READY

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/// Possible states of the PLXC task
enum plxc_states
{
    /// Idle state
    PLXC_FREE,
    /// Connected state
    PLXC_IDLE,
    /// Discovering Services
    PLXC_DISCOVERING_SVC,
    /// Busy state
    PLXC_BUSY,
    /// Number of defined states.
    PLXC_STATE_MAX
};

/// Operation Codes for pending operations in the client.
enum plxc_op_pending
{
    /// Reserved operation code
    PLXC_OP_PENDING_NO_OP = 0x00,

    // Read op-codes
    /// internal state when reading features after discovery
    PLXC_OP_PENDING_READ_FEATURES_AT_INIT,
    /// read FEATURES as the result of the command request
    PLXC_OP_PENDING_READ_FEATURES,
    /// Read CCC
    PLXC_OP_PENDING_READ_CCC,

    // Write op-codes
    /// Write Record Access Control Point
    PLXC_OP_PENDING_WRITE_RACP,
    /// Write Client Characteristic Configuration
    PLXC_OP_PENDING_WRITE_CCC,

};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Stored Environment content/context information for a connection
struct plxc_cnx_env
{
    /// Current Operation
    uint8_t op_pending;

    /// Counter used to check service uniqueness
    uint8_t nb_svc;

    /// Pulse Oximeter Service Characteristics
    struct plxc_plxp_content plxc;
};

/// Pulse Oximeter environment variable
struct plxc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct plxc_cnx_env *p_env[PLXC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[PLXC_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Pool of Pulse Oximeter Collector task environments.
extern struct plxc_env_tag **plxc_envs;

/*
 * GLOBAL FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve PLXP client profile interface
 *
 * @return PLXP client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* plxc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Sends a PLXC_ENABLE_RSP message.
 *
 * @param[in] p_plxc_env        The profile environment for the connection
 * @param[in] conidx            Connection Identifier
 * @param[in] status            Indicates the outcome of the operation
 * @param[in] sup_feat          Supported Feature @see supported_features
 * @param[in] meas_stat_sup     Measurement Status Supported @see measurement_status_supported
 * @param[in] dev_stat_sup      Device Status Supported @see device_status_supported
 ****************************************************************************************
 */
void plxc_enable_rsp_send(struct plxc_env_tag *p_plxc_env, uint8_t conidx, uint8_t status,
        uint16_t sup_feat, uint16_t meas_stat_sup, uint32_t dev_stat_sup);

/**
 ****************************************************************************************
 * @brief Sends a Complete_Event to the App
 *
 * @param[in] p_plxc_env     The profile environment
 * @param[in] conidx         Connection Identifier
 * @param[in] operation      Indicates the operation for which the cmp_evt is being sent.
 * @param[in] status         Indicates the outcome of the operation
 ****************************************************************************************
 */
void plxc_send_cmp_evt(struct plxc_env_tag *p_plxc_env, uint8_t conidx, uint8_t operation, uint8_t status);

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
void plxc_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_PLXP_CLIENT)

/// @} PLXC

#endif //(_PLXC_H_)
