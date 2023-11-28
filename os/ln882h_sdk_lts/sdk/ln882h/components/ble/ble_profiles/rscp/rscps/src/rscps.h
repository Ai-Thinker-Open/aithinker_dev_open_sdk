/**
 ****************************************************************************************
 *
 * @file rscps
 *
 * @brief Header file - Running Speed and Cadence Profile Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _RSCPS_H_
#define _RSCPS_H_

/**
 ****************************************************************************************
 * @addtogroup RSCPS Running Speed and Cadence Profile Sensor
 * @ingroup RSCP
 * @brief Running Speed and Cadence Profile Sensor
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_RSC_SENSOR)
#include "rscp_common.h"

#include "prf_types.h"
#include "prf.h"
#include "rscps_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Running Speed and Cadence Profile Sensor role task instances
#define RSCPS_IDX_MAX        (1)

/********************************************
 ******* RSCS Configuration Flag Masks ******
 ********************************************/

/// Mandatory Attributes (RSC Measurement + RSC Feature)
#define RSCPS_MANDATORY_MASK               (0x003F)
/// Sensor Location Attributes
#define RSCPS_SENSOR_LOC_MASK              (0x00C0)
/// SC Control Point Attributes
#define RSCPS_SC_CTNL_PT_MASK              (0x0700)

/*
 * MACROS
 ****************************************************************************************
 */

#define RSCPS_IS_FEATURE_SUPPORTED(features, flag) ((features & flag) == flag)

#define RSCPS_HANDLE(idx) \
    (p_rscps_env->shdl + (idx) - \
        ((!(RSCPS_IS_FEATURE_SUPPORTED(p_rscps_env->prf_cfg, RSCPS_SENSOR_LOC_MASK)) && \
                ((idx) > RSCS_IDX_SENSOR_LOC_CHAR))? (1) : (0)))

// Get database attribute index
 #define RSCPS_IDX(hdl) \
    ((hdl - p_rscps_env->shdl) + \
        ((!(RSCPS_IS_FEATURE_SUPPORTED(p_rscps_env->prf_cfg, RSCPS_SENSOR_LOC_MASK)) && \
                ((hdl - p_rscps_env->shdl) > RSCS_IDX_SENSOR_LOC_CHAR)) ? (1) : (0)))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the RSCPS task
enum rscps_state
{
    /// Idle state
    RSCPS_IDLE,
    /// Busy state
    RSCPS_BUSY,

    /// Number of defined states.
    RSCPS_STATE_MAX
};

/// Running Speed and Cadence Service - Attribute List
enum rscps_rscs_att_list
{
    /// Running Speed and Cadence Service
    RSCS_IDX_SVC,
    /// RSC Measurement
    RSCS_IDX_RSC_MEAS_CHAR,
    RSCS_IDX_RSC_MEAS_VAL,
    RSCS_IDX_RSC_MEAS_NTF_CFG,
    /// RSC Feature
    RSCS_IDX_RSC_FEAT_CHAR,
    RSCS_IDX_RSC_FEAT_VAL,
    /// Sensor Location
    RSCS_IDX_SENSOR_LOC_CHAR,
    RSCS_IDX_SENSOR_LOC_VAL,
    /// SC Control Point
    RSCS_IDX_SC_CTNL_PT_CHAR,
    RSCS_IDX_SC_CTNL_PT_VAL,
    RSCS_IDX_SC_CTNL_PT_NTF_CFG,

    /// Number of attributes
    RSCS_IDX_NB,
};

/// Profile Configuration Additional Flags bit field
enum rscps_prf_cfg_flag_bf
{
    /// RSC Measurement - Client Char. Cfg
    RSCP_PRF_CFG_FLAG_RSC_MEAS_NTF_POS = 5,
    RSCP_PRF_CFG_FLAG_RSC_MEAS_NTF_BIT = CO_BIT(RSCP_PRF_CFG_FLAG_RSC_MEAS_NTF_POS),

    /// SC Control Point - Client Char. Cfg
    RSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS = 6,
    RSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_BIT = CO_BIT(RSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS),

    /// Bonded data configured
    RSCP_PRF_CFG_PERFORMED_OK_POS = 7,
    RSCP_PRF_CFG_PERFORMED_OK_BIT = CO_BIT(RSCP_PRF_CFG_PERFORMED_OK_POS),
};

/// Sensor Location Supported Flag
enum rscps_sensor_loc_supp
{
    /// Sensor Location Char. is not supported
    RSCPS_SENSOR_LOC_NOT_SUPP,
    /// Sensor Location Char. is supported
    RSCPS_SENSOR_LOC_SUPP,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// ongoing notification information
struct rscps_ntf
{
     /// Cursor on connection
     uint8_t cursor;
     /// Packed notification/indication data size
     uint8_t length;
     /// Packed notification/indication data
     uint8_t value[RSCP_RSC_MEAS_MAX_LEN];
};

/// Running Power Profile Sensor environment variable per connection
struct rscps_cnx_env
{
    /// Profile Notify/Indication Flags
    uint8_t prfl_ntf_ind_cfg;
};

/// Running Speed and Cadence Profile Sensor environment variable
struct rscps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Notification data
    struct rscps_ntf *p_ntf;

    /// Cycling Speed and Cadence Service Start Handle
    uint16_t shdl;

    /// Features configuration
    uint16_t features;
    /// profile configuration
    uint16_t prf_cfg;

    /// Operation
    uint8_t operation;
    /// Sensor location
    uint8_t sensor_loc;

    /// State of different task instances
    ke_state_t state[RSCPS_IDX_MAX];
    /// Profile Notify/Indication Flags
    uint8_t prfl_ntf_ind_cfg[BLE_CONNECTION_MAX];
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
 * @brief Retrieve RSCP service profile interface
 *
 * @return RSCP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* rscps_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send a RSCPS_CMP_EVT message to the application.
 *
 * @param[in] conidx         Connection index
 * @param[in] src_id         Source ID
 * @param[in] dest_id        Destination ID
 * @param[in] operation      Code of the completed operation
 * @param[in] status         Status of the request
 ****************************************************************************************
 */
void rscps_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id, uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * @brief  This function fully manage event to trigg to peer(s) device(s) according
 *         to on-going operation requested by application
 * @param[in] conidx         Connection index
 ****************************************************************************************
 */
void rscps_exe_operation(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief  This function sends control point indication error
 *
 * @param conidx        Connection index
 * @param req_op_code   Request operation code
 * @param status        Response status code
 ****************************************************************************************
 */
void rscps_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status);

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
void rscps_task_init(struct ke_task_desc *p_task_desc);


#endif //(BLE_RSC_SENSOR)

/// @} RSCPS

#endif //(_RSCPS_H_)
