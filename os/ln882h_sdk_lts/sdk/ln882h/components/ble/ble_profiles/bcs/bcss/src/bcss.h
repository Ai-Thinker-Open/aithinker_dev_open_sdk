/**
 ****************************************************************************************
 *
 * @file wscs.h
 *
 * @brief Header file - Body Composition Service Sensor.
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

#ifndef _BCSS_H_
#define _BCSS_H_

/**
 ****************************************************************************************
 * @addtogroup BCSS Body Composition Service Sensor
 * @ingroup CSCP
 * @brief Body Composition Service Sensor
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BCS_SERVER)
#include "bcs_common.h"
#include "bcss_task.h"

#include "prf_types.h"
#include "prf.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Maximum number of Body Composition  Sensor role task instances
#define BCSS_IDX_MAX                        (BLE_CONNECTION_MAX)
/// Feature value size 32 bits
#define BCSS_FEAT_VAL_SIZE                  (4)
/// Measurement Indication size (plus 3 bytes for meas_u, mass_resol and hght_resol)
#define BCSS_MEAS_IND_SIZE                  (sizeof(struct bcss_meas_indicate_cmd) + 3)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Possible states of the Body Composition Service task
enum bcss_states
{
    /// not connected state
	BCSS_FREE,
    /// idle state
	BCSS_IDLE,

    /// indicate 
	BCSS_OP_INDICATE,

    /// Number of defined states.
	BCSS_STATE_MAX
};

/// Body Composition Service - Attribute List
enum bcss_bcs_att_list
{
    /// Body Composition Service
	BCSS_IDX_SVC,
    /// Body Composition Feature Characteristic
	BCSS_IDX_FEAT_CHAR,
	BCSS_IDX_FEAT_VAL,
    /// Body Composition Measurement Characteristic
	BCSS_IDX_MEAS_CHAR,
	BCSS_IDX_MEAS_IND,
    /// CCC Descriptor
	BCSS_IDX_MEAS_CCC,

    /// Number of attributes
    BCSS_IDX_NB,
};

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// Body Composition Service Sensor environment variable
struct bcss_env_tag
{
    /// profile environment
    prf_env_t prf_env;

    /// Body Composition Service Start Handle
    uint16_t shdl;
    /// Feature configuration
    uint32_t feature;

    /// CCC for each connections
    uint8_t prfl_ind_cfg[BLE_CONNECTION_MAX];

    /// State of different task instances
    ke_state_t state[BCSS_IDX_MAX];

    /// Store the @ref bcss_meas_indicate_cmd param when Multipacket presents
    struct ke_msg* meas_cmd_msg[BLE_CONNECTION_MAX];

};

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Inform the APP that a procedure has been completed
 *
 * @param[in] p_bcss_env    BCS Sensor environment variable
 * @param[in] conidx        Connection index
 * @param[in] operation     Operation code
 * @param[in] status        Operation status
 ****************************************************************************************
 */
void bcss_send_cmp_evt(struct bcss_env_tag *p_bcss_env, uint8_t conidx, uint8_t operation, uint8_t status);

/**
 ****************************************************************************************
 * @brief Retrieve BSCS service profile interface
 *
 * @return Body composition service profile interface
 ****************************************************************************************
 */
const struct prf_task_cbs* bcss_prf_itf_get(void);


/**
 ****************************************************************************************
 * @brief Get Mass Resolution from feature value
 *
 * @param[in] feature Body Composition Feature value
 *
 * @return Mass Resolution
 ****************************************************************************************
 */
uint8_t bcss_get_mass_resol(uint32_t feature);

/**
 ****************************************************************************************
 * @brief Get Height Resolution from feature value
 *
 * @param[in] feature Body Composition Feature value
 *
 * @return Height Resolution
 ****************************************************************************************
 */
uint8_t bcss_get_hght_resol(uint32_t feature);

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
void bcss_task_init(struct ke_task_desc *p_task_desc);

#endif //(BLE_BSC_SERVER)

/// @} BCSS

#endif //(_BSCS_H_)
