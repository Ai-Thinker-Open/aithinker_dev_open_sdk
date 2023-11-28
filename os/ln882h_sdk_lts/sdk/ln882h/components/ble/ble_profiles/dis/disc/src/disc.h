/**
 ****************************************************************************************
 *
 * @file disc.h
 *
 * @brief Header file - Device Information Service - Client Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef DISC_H_
#define DISC_H_

/**
 ****************************************************************************************
 * @addtogroup DIS Device Information Service
 * @ingroup PROFILE
 * @brief Device Information Service
 *****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DISC Device Information Service Client
 * @ingroup DIS
 * @brief Device Information Service Client
 * @{
 ****************************************************************************************
 */
/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_DIS_CLIENT)
#include <stdint.h>
#include <stdbool.h>
#include "ke_task.h"
#include "prf_utils.h"
#include "../api/disc_task.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Device Information Service Client task instances
#define DISC_IDX_MAX    (BLE_CONNECTION_MAX)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the DISC task
enum
{
    /// Not Connected State
    DISC_FREE,
    /// Idle state
    DISC_IDLE,
    /// Discovering State
    DISC_DISCOVERING,
    /// Busy State
    DISC_BUSY,

    ///Number of defined states.
    DISC_STATE_MAX
};


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/// Environment variable for each Connections
struct disc_cnx_env
{
    ///Current Time Service Characteristics
    struct disc_dis_content dis;
    /// Last char. code requested to read.
    uint8_t last_char_code;
    /// counter used to check service uniqueness
    uint8_t nb_svc;
};

/// Device Information Service Client environment variable
struct disc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct disc_cnx_env *p_env[DISC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[DISC_IDX_MAX];
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Retrieve DIS client profile interface
 *
 * @return DIS client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* disc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Thermometer ATT DB discovery results to DIS host.
 *
 * @param p_disc_env    DIS Client Role task environment
 * @param conidx        Connection index
 * @param status        Response status code
 *****************************************************************************************
 */
void disc_enable_rsp_send(struct disc_env_tag *p_disc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Send an DISC_CMP_EVT message to a requester.
 *
 * @param[in] p_disc_env    DIS Client environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Code of the completed operation
 * @param[in] status        Status of the request
 ****************************************************************************************
 */
void disc_send_cmp_evt(struct disc_env_tag *p_disc_env, uint8_t conidx, uint8_t operation, uint8_t status);

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
void disc_task_init(struct ke_task_desc *p_task_desc);

#endif //BLE_DIS_CLIENT

/// @} DISC

#endif // DISC_H_
