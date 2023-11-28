/**
 ****************************************************************************************
 *
 * @file cscps.h
 *
 * @brief Header file - Cycling Speed and Cadence Profile Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _CSCPS_H_
#define _CSCPS_H_

/**
 ****************************************************************************************
 * @addtogroup CSCPS Cycling Speed and Cadence Profile Sensor
 * @ingroup CSCP
 * @brief Cycling Speed and Cadence Profile Sensor
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CSC_SENSOR)
#include "cscp_common.h"
#include "co_utils.h"
#include "prf_types.h"
#include "prf.h"
#include "cscps_task.h"
#include "attm.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Cycling Speed and Cadence Profile Sensor role task instances
#define CSCPS_IDX_MAX        (1)

/********************************************
 ******* CSCPS Configuration Flag Masks ******
 ********************************************/

/// Mandatory Attributes (CSC Measurement + CSC Feature)
#define CSCPS_MANDATORY_MASK               (0x003F)
/// Sensor Location Attributes
#define CSCPS_SENSOR_LOC_MASK              (0x00C0)
/// SC Control Point Attributes
#define CSCPS_SC_CTNL_PT_MASK              (0x0700)

/*
 * MACROS
 ****************************************************************************************
 */

#define CSCPS_IS_FEATURE_SUPPORTED(features, flag) ((features & flag) == flag)

#define CSCPS_HANDLE(idx) \
    (p_cscps_env->shdl + (idx) - \
        ((!(CSCPS_IS_FEATURE_SUPPORTED(p_cscps_env->prfl_cfg, CSCPS_SENSOR_LOC_MASK)) && \
                ((idx) > CSCS_IDX_SENSOR_LOC_CHAR))? (1) : (0)))

// Get database attribute index
 #define CSCPS_IDX(hdl) \
    ((hdl - p_cscps_env->shdl) + \
        ((!(CSCPS_IS_FEATURE_SUPPORTED(p_cscps_env->prfl_cfg, CSCPS_SENSOR_LOC_MASK)) && \
                ((hdl - p_cscps_env->shdl) > CSCS_IDX_SENSOR_LOC_CHAR)) ? (1) : (0)))

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the CSCPS task
enum cscps_state
{
    /// Idle state
    CSCPS_IDLE,
    /// Busy state
    CSCPS_BUSY,

    /// Number of defined states.
    CSCPS_STATE_MAX
};

/// Cycling Speed and Cadence Service - Attribute List
enum cscps_cscs_att_list
{
    /// Cycling Speed and Cadence Service
    CSCS_IDX_SVC,
    /// CSC Measurement
    CSCS_IDX_CSC_MEAS_CHAR,
    CSCS_IDX_CSC_MEAS_VAL,
    CSCS_IDX_CSC_MEAS_NTF_CFG,
    /// CSC Feature
    CSCS_IDX_CSC_FEAT_CHAR,
    CSCS_IDX_CSC_FEAT_VAL,
    /// Sensor Location
    CSCS_IDX_SENSOR_LOC_CHAR,
    CSCS_IDX_SENSOR_LOC_VAL,
    /// SC Control Point
    CSCS_IDX_SC_CTNL_PT_CHAR,
    CSCS_IDX_SC_CTNL_PT_VAL,
    CSCS_IDX_SC_CTNL_PT_NTF_CFG,

    /// Number of attributes
    CSCS_IDX_NB,
};

/// Profile Configuration Additional Flags bit field
enum cscps_prf_cfg_flag_bf
{
    /// CSC Measurement - Client Char. Cfg
    CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_POS = 3,
    CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_BIT = CO_BIT(CSCP_PRF_CFG_FLAG_CSC_MEAS_NTF_POS),

    /// SC Control Point - Client Char. Cfg
    CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS = 4,
    CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_BIT = CO_BIT(CSCP_PRF_CFG_FLAG_SC_CTNL_PT_IND_POS),

    /// Bonded data used
    CSCP_PRF_CFG_PERFORMED_OK_POS = 7,
    CSCP_PRF_CFG_PERFORMED_OK_BIT = CO_BIT(CSCP_PRF_CFG_PERFORMED_OK_POS),
};

/// Sensor Location Supported Flag
enum cscps_sensor_loc_supp
{
    /// Sensor Location Char. is not supported
    CSCPS_SENSOR_LOC_NOT_SUPP,
    /// Sensor Location Char. is supported
    CSCPS_SENSOR_LOC_SUPP,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// ongoing notification information
struct cscps_ntf
{
     /// Cursor on connection
     uint8_t cursor;
     /// Packed notification/indication data size
     uint8_t length;
     /// Packed notification/indication data
     uint8_t value[CSCP_CSC_MEAS_MAX_LEN];
};

/// Cycling Speed and Cadence Profile Sensor environment variable
struct cscps_env_tag
{
    /// profile environment
    prf_env_t prf_env;
    /// Notification data
    struct cscps_ntf *p_ntf;
    /// Wheel revolution
    uint32_t tot_wheel_rev;

    /// Cycling Speed and Cadence Service Start Handle
    uint16_t shdl;
    /// Features configuration
    uint16_t features;
    /// profile configuration
    uint16_t prfl_cfg;

    /// Operation
    uint8_t operation;
    /// Sensor location
    uint8_t sensor_loc;

    /// Environment variable pointer for each connections
    uint8_t prfl_ntf_ind_cfg[BLE_CONNECTION_MAX];

    /// State of different task instances
    ke_state_t state[CSCPS_IDX_MAX];
};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Retrieve CSCP service profile interface
 *
 * @return CSCP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* cscps_prf_itf_get(void);

/**
 ****************************************************************************************
 * @brief Send a CSCPS_CMP_EVT message to the application.
 *
 * @param[in] conidx         Connection index
 * @param[in] src_id         Source ID
 * @param[in] dest_id        Destination ID
 * @param[in] operation      Code of the completed operation
 * @param[in] status         Status of the request
 ****************************************************************************************
 */
void cscps_send_cmp_evt(uint8_t conidx, uint8_t src_id, uint8_t dest_id,
                        uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * @brief  This function fully manage event to trigg to peer(s) device(s) according
 *         to on-going operation requested by application
 * @param[in] conidx         Connection index
 ****************************************************************************************
 */
void cscps_exe_operation(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief  This function sends control point indication error
 *
 * @param conidx        Connection index
 * @param req_op_code   Request operation code
 * @param status        Response status code
 ****************************************************************************************
 */
void cscps_send_rsp_ind(uint8_t conidx, uint8_t req_op_code, uint8_t status);

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
void cscps_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_CSC_SENSOR)

/// @} CSCPS

#endif //(_CSCPS_H_)
