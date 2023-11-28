/**
 ****************************************************************************************
 *
 * @file htpc.h
 *
 * @brief Header file - Health Thermometer Profile - Collector Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef HTPC_H_
#define HTPC_H_

/**
 ****************************************************************************************
 * @addtogroup HTP Health Thermometer Profile
 * @ingroup PROFILE
 * @brief Health Thermometer Profile
 *
 * The HTP module is the responsible block for implementing the Health Thermometer Profile
 * functionalities in the BLE Host.
 *
 * The Health Thermometer Profile defines the functionality required in a device that allows
 * the user (Collector device) to configure and recover temperature measurements from a Thermometer device.
 *****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HTPC Health Thermometer Profile Collector
 * @ingroup HTP
 * @brief Health Thermometer Profile Collector
 *
 * The HTPC is responsible for providing Health Thermometer Profile Collector functionalities to
 * upper layer module or application. The device using this profile takes the role
 * of Health Thermometer Profile Collector.
 *
 * Health Thermometer Profile Collector (HTPC): A HTPC (e.g. PC, phone, etc)
 * is the term used by this profile to describe a device that can set temperature
 * indication/notification intervals in a HTP Thermometer, interpreting them in a way
 * suitable to the user application.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_HT_COLLECTOR || BLE_HT_THERMOM)
#include "ke_task.h"
#include "prf_utils.h"
#include "htp/htpc/api/htpc_task.h"
#include "htp/htp_common.h"

/*
 * DEFINES
 ****************************************************************************************
 */

#define HTPC_PACKED_TEMP_MIN_LEN        (5)
#define HTPC_PACKED_TEMP_MAX_LEN        (13)

/// Maximum number of task instances
#define HTPC_IDX_MAX                    (BLE_CONNECTION_MAX)

/// Possible states of the HTPC task
enum htpc_state
{
    ///Free state
    HTPC_FREE,
    ///Idle state
    HTPC_IDLE,
    ///Busy state
    HTPC_BUSY,

    ///Number of defined states.
    HTPC_STATE_MAX
};

/*
 * MACROS
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/// Health Temperature Collector environment variable per connection
struct htpc_cnx_env
{
    /// Last service for which something was discovered
    uint8_t last_svc_req;
    /// counter used to check service uniqueness
    uint8_t nb_svc;
    /// used to store if measurement context
    uint8_t meas_ctx_en;
    /// context of request
    struct ke_msg *p_operation;
    /// HTS handle values
    struct htpc_hts_content hts;
};

/// Health Thermometer Profile Client environment variable
struct htpc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct htpc_cnx_env *p_env[HTPC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[HTPC_IDX_MAX];
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
 * @brief Retrieve HTS service profile interface
 *
 * @return HTS service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *htpc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Thermometer ATT DB discovery results to HTPC host.
 *
 * @param p_htpc_env    Health Thermometer Profile Client Role task environment
 * @param conidx        Connection index
 * @param status        Response status code
 ****************************************************************************************
 */
void htpc_enable_rsp_send(struct htpc_env_tag *p_htpc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Unpack the received temperature measurement value
 *
 * @param p_htpc_env        Health Thermometer Profile Client Role task environment
 * @param p_packed_temp     Packed temperature value
 * @param length            Length of received value
 * @param flag_stable_meas  Stable or intermediary type of temperature
 * @param conidx            Connection index
 ****************************************************************************************
 */
void htpc_unpack_temp(struct htpc_env_tag *p_htpc_env, uint8_t *p_packed_temp,
                      uint8_t length, uint8_t flag_stable_meas, uint8_t conidx);

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
void htpc_task_init(struct ke_task_desc *p_task_desc);


#endif //BLE_HT_COLLECTOR
/// @} HTPC
#endif // HTPC_H_
