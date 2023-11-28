/**
 ****************************************************************************************
 *
 * @file tipc.h
 *
 * @brief Header file - Time Profile - Client Role.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef _TIPC_H_
#define _TIPC_H_

/**
 ****************************************************************************************
 * @addtogroup TIPC Time Profile Client
 * @ingroup TIP
 * @brief Time Profile Client
 *
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_TIP_CLIENT)
#include "tipc_task.h"
#include "ke_task.h"
#include "prf_types.h"
#include "prf_utils.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Time Client task instances
#define TIPC_IDX_MAX    (BLE_CONNECTION_MAX)

/// Possible states of the TIPC task
enum
{
    /// Free state
    TIPC_FREE,
    /// IDLE state
    TIPC_IDLE,
    /// Discovering Blood Pressure SVC and CHars
    TIPC_DISCOVERING,
    /// Busy state
    TIPC_BUSY,

    /// Number of defined states.
    TIPC_STATE_MAX
};

/// Internal codes for reading a CTS or NDCS or RTUS characteristic with one single request
enum
{
    /// Read CTS Current Time
    TIPC_RD_CTS_CURR_TIME           = TIPC_CHAR_CTS_CURR_TIME,
    /// Read CTS Local Time Info
    TIPC_RD_CTS_LOCAL_TIME_INFO     = TIPC_CHAR_CTS_LOCAL_TIME_INFO,
    /// Read CTS Reference Time Info
    TIPC_RD_CTS_REF_TIME_INFO       = TIPC_CHAR_CTS_REF_TIME_INFO,

    /// Read CTS Current Time Client Cfg. Desc
    TIPC_RD_CTS_CURR_TIME_CLI_CFG   = (TIPC_DESC_CTS_MASK | TIPC_DESC_CTS_CURR_TIME_CLI_CFG),

    /// Read NDCS Time With DST
    TIPC_RD_NDCS_TIME_WITH_DST      = (TIPC_CHAR_NDCS_MASK | TIPC_CHAR_NDCS_TIME_WITH_DST),

    /// Read RTUS Time Update State
    TIPC_RD_RTUS_TIME_UPD_STATE     = (TIPC_CHAR_RTUS_MASK | TIPC_CHAR_RTUS_TIME_UPD_STATE),
};

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

struct tipc_cnx_env
{
    /// Last requested UUID(to keep track of the two services and char)
    uint16_t last_uuid_req;
    /// Last service for which something was discovered
    uint16_t last_svc_req;
    /// Last characteristic code used in a read or a write request
    uint16_t last_char_code;
    /// Counter used to check service uniqueness
    uint8_t nb_svc;

    /// Current Time Service Characteristics
    struct tipc_cts_content cts;
    /// Next DST Change Characteristics
    struct tipc_ndcs_content ndcs;
    /// Reference Time Update Characteristics
    struct tipc_rtus_content rtus;
};


/// Time Profile Client environment variable
struct tipc_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Environment variable pointer for each connections
    struct tipc_cnx_env *p_env[TIPC_IDX_MAX];
    /// State of different task instances
    ke_state_t state[TIPC_IDX_MAX];
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve TIP client profile interface
 * @return TIP client profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* tipc_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send Time ATT DB discovery results to TIPC host.
 *
 * @param p_tipc_env    TIP Client Role task environment
 * @param conidx        Connection index
 * @param status        Response status code
 ****************************************************************************************
 */
void tipc_enable_rsp_send(struct tipc_env_tag *p_tipc_env, uint8_t conidx, uint8_t status);

/**
 ****************************************************************************************
 * @brief Unpack Current Time value
 *
 * @param[out] p_curr_time_val Pointer to unpacked Current Time value
 * @param[in] p_packed_ct Pointer to the parameters of the message
 ****************************************************************************************
 */
void tipc_unpack_curr_time_value(struct tip_curr_time *p_curr_time_val, uint8_t *p_packed_ct);

/**
 ****************************************************************************************
 * @brief Unpack Time with DST value
 *
 * @param[out] p_time_dst_val Pointer to unpacked Time with DST value
 * @param[in] p_packed_tdst Pointer to the parameters of the message
 ****************************************************************************************
 */
void tipc_unpack_time_dst_value(struct tip_time_with_dst *p_time_dst_val, uint8_t *p_packed_tdst);

/**
 ****************************************************************************************
 * @brief Send an TLPC_CMP_EVT message to a requester.
 *
 * @param[in] p_tipc_env    TIP Client environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Code of the completed operation
 * @param[in] status        Status of the request
 ****************************************************************************************
 */
void tipc_send_cmp_evt(struct tipc_env_tag *p_tipc_env, uint8_t conidx, uint8_t operation, uint8_t status);

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
void tipc_task_init(struct ke_task_desc *p_task_desc);

#endif /* (BLE_TIP_CLIENT) */

/// @} TIPC

#endif /* _TIPC_H_ */
