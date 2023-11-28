/**
 ****************************************************************************************
 *
 * @file cgms.h
 *
 * @brief Header file - Continuous Glucose Monitoring Profile Service.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _CGMS_H_
#define _CGMS_H_

/**
 ****************************************************************************************
 * @addtogroup CGMS Continuous Glucose Monitoring Profile Service
 * @ingroup CGMS
 * @brief Continuous Glucose Monitoring Profile Service
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CGM_SERVER)
#include "cgmp_common.h"
#include "cgms_task.h"

#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Continuous Glucose Monitoring Profile Service role task instances
#define CGMS_IDX_MAX                            (BLE_CONNECTION_MAX)
/// Internal state of CCC enabled set to 0
#define CGMS_PRFL_STATE_CLEAR                   (0)
/// Maximum Features size (uint24 + 2x 4bit + crc)
#define CGMS_FEAT_SIZE_MAX                      (3 + 1 + 2)
///Maximum Status size
#define CGMS_IDX_STATUS_SIZE_MAX                (2 + 3 + 2)
/// Maximum Session Run Time size
#define CGMS_IDX_SESS_RUN_TIME_SIZE_MAX         (2 + 2)
/// Maximum Control Point size (OpCode + Operator+ Filter + FilterParameter)
#define CGMS_RACP_SIZE_MAX                      (2 + 5 + 2)
/// Maximum Specific OPs Control Point size (OpCode + Operator+ Operand)
#define CGMS_OPS_CTRL_PT_SIZE_MAX               (2 + sizeof(struct cgm_calib_operand) + 2)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */
 /// Possible states of the CGMS task
enum cgms_state_id
{
    /// Not connected state
    CGMS_FREE,
    /// Idle state
    CGMS_IDLE,
    /// Read request
    CGMS_OP_READ,
    /// Notify
    CGMS_OP_NOTIFY,
    /// Indicate Spot_Measurement
    CGMS_OP_INDICATE_MEAS,
    /// Indicate RACP
    CGMS_OP_INDICATE_RACP,
    CGMS_OP_INDICATE_RACP_NO_CMP,
    /// Application request
    CGMS_OP_WRITE_OPS_CTRL_PT,
    CGMS_OP_INDICATE_OPS_CTRL_PT,

    /// Number of defined states.
    CGMS_STATE_MAX
};

/// CGM Service - Attribute List
enum cgms_att_list
{
    /// CGM Service
    CGMS_IDX_SVC,
    /// CGM Measurement Characteristic
    CGMS_IDX_MEAS_CHAR,
    CGMS_IDX_MEAS_VAL,
    /// CCC Descriptor
    CGMS_IDX_MEAS_CCC,
    /// CGM Features Characteristic
    CGMS_IDX_FEAT_CHAR,
    CGMS_IDX_FEAT_VAL,
    /// CGM Status Characteristic
    CGMS_IDX_STATUS_CHAR,
    CGMS_IDX_STATUS_VAL,
    /// CGM Session Start Time Characteristic
    CGMS_IDX_SESS_ST_TIME_CHAR,
    CGMS_IDX_SESS_ST_TIME_VAL,
    /// CGM Session Run Time Characteristic
    CGMS_IDX_SESS_RUN_TIME_CHAR,
    CGMS_IDX_SESS_RUN_TIME_VAL,
    /// CGM Record Access Control Point Characteristic
    CGMS_IDX_RACP_CHAR,
    CGMS_IDX_RACP_VAL,
    /// CCC Descriptor
    CGMS_IDX_RACP_CCC,
    /// CGM Specific Ops Control Point Characteristic
    CGMS_IDX_SPEC_OPS_CTRL_PT_CHAR,
    CGMS_IDX_SPEC_OPS_CTRL_PT_VAL,
    /// CCC Descriptor
    CGMS_IDX_SPEC_OPS_CTRL_PT_CCC,

    /// Number of attributes
    CGMS_IDX_NB,
};

/// Internal state of CCC enabled per Characteristic and RACP process @see struct cgms_env_tag
enum cgms_prfl_state_bf
{
    /// CCC for Measurement
    CGMS_NTF_MEAS_ENABLED_POS = 0,
    CGMS_NTF_MEAS_ENABLED_BIT = CO_BIT(CGMS_NTF_MEAS_ENABLED_POS),

    /// CCC for RACP
    CGMS_IND_RACP_ENABLED_POS = 1,
    CGMS_IND_RACP_ENABLED_BIT = CO_BIT(CGMS_IND_RACP_ENABLED_POS),

    /// CCC for Specific OPS Control Point
    CGMS_IND_SOPS_ENABLED_POS = 2,
    CGMS_IND_SOPS_ENABLED_BIT = CO_BIT(CGMS_IND_SOPS_ENABLED_POS),

    /// RACP keep state of the procedure
    CGMS_RACP_IN_PROG_POS = 3,
    CGMS_RACP_IN_PROG_BIT = CO_BIT(CGMS_RACP_IN_PROG_POS),
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Continuous Glucose Monitoring Profile Service environment variable
struct cgms_env_tag
{
    /// profile environment
    prf_env_t prf_env;

    /// CGM Service Start Handle
    uint16_t shdl;
    /// CGM Features @see enum cgm_feat_bf
    uint32_t cgm_feature;
    /// Internal state of CCC enabled @see enum cgms_prfl_state_bf:
    uint8_t prfl_state[CGMS_IDX_MAX];
    /// State of different task instances
    ke_state_t state[CGMS_IDX_MAX];

};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Convert raw handle to the attribute index @see enum cgms_att_list
 *
 * @param[in]        handle      Raw handle
 *
 * @return Attribute index
 ****************************************************************************************
 */
uint8_t cgms_handle2att(uint16_t handle);

/**
 ****************************************************************************************
 * @brief Convert attribute to the raw handle index @see enum cgms_att_list
 *
 * @param[in]        att_idx      Attribute index
 *
 * @return Handle of the attribute
 ****************************************************************************************
 */
uint16_t cgms_att2handle(uint8_t att_idx);

/**
 ****************************************************************************************
 * @brief Retrieve CGMP service profile interface
 *
 * @return CGMP service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs *cgms_prf_itf_get(void);

/*
 * TASK DESCRIPTOR DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void cgms_task_init(struct ke_task_desc *task_desc);

#endif //(BLE_CGM_SERVER)

/// @} CGMS

#endif //(_CGMS_H_)
