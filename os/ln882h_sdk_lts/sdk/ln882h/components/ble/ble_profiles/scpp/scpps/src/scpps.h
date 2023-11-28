/**
 ****************************************************************************************
 *
 * @file scpps.h
 *
 * @brief Header file - Scan Parameters Profile Server.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _SCPPS_H_
#define _SCPPS_H_

/**
 ****************************************************************************************
 * @addtogroup SCPPS Scan Parameters Profile Server
 * @ingroup SCPP
 * @brief Scan Parameters Profile Server
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_SP_SERVER)

#include "scpp_common.h"
#include "scpps_task.h"
#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

///Maximum number of Scan Parameters Device task instances
#define SCPPS_IDX_MAX     0x01

#define SCPPS_CFG_FLAG_MANDATORY_MASK       (0x07)
#define SCPPS_CFG_FLAG_SCAN_REFRESH_MASK    (0x38)

/// Scanning set parameters range min (0x4)
#define SCPPS_SCAN_INTERVAL_MIN                    4
/// Scanning set parameters range max (0x4000)
#define SCPPS_SCAN_INTERVAL_MAX                    16384

/// Scanning set parameters range min (0x4)
#define SCPPS_SCAN_WINDOW_MIN                    4
/// Scanning set parameters range max (0x4000)
#define SCPPS_SCAN_WINDOW_MAX                    16384


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the SCPPS task
enum scpps_state
{
    /// Idle state
    SCPPS_IDLE,
    /// Busy state
    SCPPS_BUSY,

    /// Number of defined states.
    SCPPS_STATE_MAX
};

/// Scan Parameters Service Attributes Indexes
enum
{
    SCPS_IDX_SVC,

    SCPS_IDX_SCAN_INTV_WD_CHAR,
    SCPS_IDX_SCAN_INTV_WD_VAL,

    SCPS_IDX_SCAN_REFRESH_CHAR,
    SCPS_IDX_SCAN_REFRESH_VAL,
    SCPS_IDX_SCAN_REFRESH_NTF_CFG,

    SCPS_IDX_NB,
};

enum
{
    SCPP_SERVER_REQUIRES_REFRESH    = 0x00,
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Scan Parameters Profile Server environment variable
struct scpps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// SCPS Start Handle
    uint16_t start_hdl;
    /// Database configuration
    uint8_t features;
    /// Notification configuration of peer devices.
    uint8_t ntf_cfg[BLE_CONNECTION_MAX];
    /// BASS task state
    ke_state_t state[SCPPS_IDX_MAX];

};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve SCPPS service profile interface
 *
 * @return SCPPS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *scpps_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send an SCPPS_CMP_EVT message to a requester.
 *
 * @param[in] p_scpps_env   Scan Parameters Server environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Code of the completed operation
 * @param[in] status        Status of the request
 ****************************************************************************************
 */
void scpps_send_cmp_evt(struct scpps_env_tag *p_scpps_env, uint8_t conidx, uint8_t operation, uint8_t status);

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
void scpps_task_init(struct ke_task_desc *p_task_desc);


#endif /* #if (BLE_SP_SERVER) */

/// @} SCPPS

#endif /* _SCPPS_H_ */
