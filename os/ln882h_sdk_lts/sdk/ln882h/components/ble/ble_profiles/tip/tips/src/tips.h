/**
 ****************************************************************************************
 *
 * @file tips.h
 *
 * @brief Header file - Time Profile Server.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

#ifndef TIPS_H_
#define TIPS_H_

/**
 ****************************************************************************************
 * @addtogroup TIPS Time Profile Server
 * @ingroup TIP
 * @brief Time Profile Server
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#if (BLE_TIP_SERVER)
#include <stdint.h>
#include <stdbool.h>

#include "tip_common.h"
#include "prf_types.h"
#include "gap.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Time Server task instances
#define TIPS_IDX_MAX                       (BLE_CONNECTION_MAX)

/// CTS Configuration Flag Masks
#define TIPS_CTS_CURRENT_TIME_MASK         (0x0F)
#define TIPS_CTS_LOC_TIME_INFO_MASK        (0x30)
#define TIPS_CTS_REF_TIME_INFO_MASK        (0xC0)

/// Packed Values Length
#define CTS_CURRENT_TIME_VAL_LEN           (10)
#define NDCS_TIME_DST_VAL_LEN              (8)

/*
 * MACROS
 ****************************************************************************************
 */


/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the TIPS task
enum
{
    /// idle state
    TIPS_IDLE,
    /// connected state
    TIPS_BUSY,

    /// Number of defined states.
    TIPS_STATE_MAX
};

/// Database Configuration - Bit Field Flags
enum tips_db_config_bf
{
    /// Current Time Service Local Time Information char. support bit
    TIPS_CTS_LOC_TIME_INFO_SUP_POS = 0,
    TIPS_CTS_LOC_TIME_INFO_SUP_BIT = CO_BIT(TIPS_CTS_LOC_TIME_INFO_SUP_POS),

    /// Current Time Service Reference Time Information char. support bit
    TIPS_CTS_REF_TIME_INFO_SUP_POS = 1,
    TIPS_CTS_REF_TIME_INFO_SUP_BIT = CO_BIT(TIPS_CTS_REF_TIME_INFO_SUP_POS),

    /// Next DST Change Service support bit
    TIPS_NDCS_SUP_POS = 2,
    TIPS_NDCS_SUP_BIT = CO_BIT(TIPS_NDCS_SUP_POS),

    /// Reference Time Update Service support bit
    TIPS_RTUS_SUP_POS = 3,
    TIPS_RTUS_SUP_BIT = CO_BIT(TIPS_RTUS_SUP_POS),

    /// Current Time Service Current Time Configuration support bit
    TIPS_CTS_CURRENT_TIME_CFG_POS = 4,
    TIPS_CTS_CURRENT_TIME_CFG_BIT = CO_BIT(TIPS_CTS_CURRENT_TIME_CFG_POS),
};

/// CTS Attributes code
enum
{
    CTS_IDX_SVC,

    CTS_IDX_CURRENT_TIME_CHAR,
    CTS_IDX_CURRENT_TIME_VAL,
    CTS_IDX_CURRENT_TIME_CFG,

    CTS_IDX_LOCAL_TIME_INFO_CHAR,
    CTS_IDX_LOCAL_TIME_INFO_VAL,

    CTS_IDX_REF_TIME_INFO_CHAR,
    CTS_IDX_REF_TIME_INFO_VAL,

    CTS_IDX_NB,
};

/// NDCS Attributes code
enum
{
    NDCS_IDX_SVC,
    NDCS_IDX_TIME_DST_CHAR,
    NDCS_IDX_TIME_DST_VAL,

    NDCS_IDX_NB,
};

/// RTUS Attributes code
enum
{
    RTUS_IDX_SVC,
    RTUS_IDX_TIME_UPD_CTNL_PT_CHAR,
    RTUS_IDX_TIME_UPD_CTNL_PT_VAL,
    RTUS_IDX_TIME_UPD_STATE_CHAR,
    RTUS_IDX_TIME_UPD_STATE_VAL,

    RTUS_IDX_NB,
};

/// CTS Char. code
enum
{
    CTS_CURRENT_TIME_CHAR,
    CTS_LOCAL_TIME_INFO_CHAR,
    CTS_REF_TIME_INFO_CHAR,

    CTS_CHAR_MAX,
};

/// NDCS Char. code
enum
{
    NDCS_TIME_DST_CHAR,

    NDCS_CHAR_MAX,
};

/// RTUS Char. code
enum
{
    RTUS_TIME_UPD_CTNL_PT_CHAR,
    RTUS_TIME_UPD_STATE_CHAR,

    RTUS_CHAR_MAX,
};

/// Read char. Op code
enum
{
    TIPS_CTS_CURRENT_TIME,
    TIPS_CTS_LOCAL_TIME_INFO,
    TIPS_CTS_REF_TIME_INFO,
    TIPS_NDCS_TIME_DST,
    TIPS_RTUS_TIME_UPD_STATE_VAL,
    TIPS_CTS_CURRENT_TIME_CLI
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Time Profile Server Instance Environment variable
struct tips_cnx_env
{
    /// NTF State
    uint8_t ntf_state;
    /// Saved read handle
    uint16_t handle;
};


/// Time Profile Server. Environment variable
struct tips_env_tag
{
    /// profile environment
    prf_env_t prf_env;

    /// CTS Start Handle
    uint16_t cts_shdl;
    /// NDCS Start Handle
    uint16_t ndcs_shdl;
    /// RTUS Start Handle
    uint16_t rtus_shdl;

    /// CTS Attribute Table
    uint8_t cts_att_tbl[CTS_CHAR_MAX];
    /// NDCS Attribute Table
    uint8_t ndcs_att_tbl[NDCS_CHAR_MAX];
    /// NDCS Attribute Table
    uint8_t rtus_att_tbl[RTUS_CHAR_MAX];

    /// Environment variable pointer for each connections
    struct tips_cnx_env *p_env[BLE_CONNECTION_MAX];

    /// Database configuration
    uint8_t features;
    /// Time update state for the Ctl Pt
    uint8_t time_upd_state;

    /// State of different task instances
    ke_state_t state[TIPS_IDX_MAX];
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
 * @brief Retrieve TIP service profile interface
 *
 * @return TIP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *tips_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Pack Current Time value
 *
 * @param p_curr_time_val Current Time value
 ****************************************************************************************
 */
uint8_t tips_pack_curr_time_value(uint8_t *p_pckd_time,
        const struct tip_curr_time* p_curr_time_val);

/**
 ****************************************************************************************
 * @brief Pack Time With DST value
 *
 * @param p_time_dst_val Time With DST value
 ****************************************************************************************
 */
uint8_t tips_pack_time_dst_value(uint8_t *p_pckd_time_dst,
                              const struct tip_time_with_dst *p_time_dst_val);

/**
 ****************************************************************************************
 * @brief Send an TLPS_CMP_EVT message to a requester.
 *
 * @param[in] p_tips_env    TIP Server environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Code of the completed operation
 * @param[in] status        Status of the request
 ****************************************************************************************
 */
void tips_send_cmp_evt(struct tips_env_tag *p_tips_env, uint8_t conidx, uint8_t operation, uint8_t status);

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
void tips_task_init(struct ke_task_desc *p_task_desc);


#endif //BLE_TIP_SERVER

/// @} TIPS

#endif // TIPS_H_
