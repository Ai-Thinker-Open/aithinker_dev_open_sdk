/**
 ****************************************************************************************
 *
 * @file plxs.c
 *
 * @brief Pulse Oximeter Profile Service implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup PLXS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_PLX_SERVER)
#include "plxp_common.h"

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "plxs.h"
#include "plxs_task.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default read perm
#define RD_P        (PERM(RD, ENABLE)        | PERM(RP, UNAUTH))
/// Default write perm
#define WR_P        (PERM(WRITE_REQ, ENABLE) | PERM(WP, UNAUTH))
/// Default notify perm
#define NTF_P       (PERM(NTF, ENABLE)       | PERM(NP, UNAUTH))
/// Ind perm
#define IND_P       (PERM(IND, ENABLE)       | PERM(IP, NO_AUTH))
/// Enable Read Indication (1 = Value not present in Database)
#define  RI_P       PERM(RI, ENABLE)

/// Full PLXS Database Description - Used to add attributes into the database
static const struct attm_desc plxs_att_db[PLXS_IDX_NB] =
{
    // ATT Index                     | ATT UUID                    | Permission        | EXT PERM | MAX ATT SIZE
    // service attribute
    [PLXS_IDX_SVC]                   = {ATT_DECL_PRIMARY_SERVICE,    PERM(RD, ENABLE),    0,        0},
    // PLXP SPOT-Measurement Characteristic
    [PLXS_IDX_SPOT_MEASUREMENT_CHAR] = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [PLXS_IDX_SPOT_MEASUREMENT_VAL]  = {ATT_CHAR_PLX_SPOT_CHECK_MEASUREMENT_LOC, IND_P,   0,        0},
    [PLXS_IDX_SPOT_MEASUREMENT_CCC]  = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,      RI_P,        0},
    // PLXP Continuous Measurement Characteristic
    [PLXS_IDX_CONT_MEASUREMENT_CHAR] = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [PLXS_IDX_CONT_MEASUREMENT_VAL]  = {ATT_CHAR_PLX_CONTINUOUS_MEASUREMENT_LOC, NTF_P,   0,        0},
    [PLXS_IDX_CONT_MEASUREMENT_CCC]  = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,      RI_P,        0},
    // PLXP Features Characteristic
    [PLXS_IDX_FEATURES_CHAR]         = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [PLXS_IDX_FEATURES_VAL]          = {ATT_CHAR_PLX_FEATURES_LOC,   RD_P,                0, PLXS_IDX_FEAT_SIZE_MAX},
    // PLXP Record Access Control Point Characteristic
    [PLXS_IDX_RACP_CHAR]             = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [PLXS_IDX_RACP_VAL]              = {ATT_CHAR_REC_ACCESS_CTRL_PT, WR_P | IND_P,        0, PLXS_IDX_RACP_SIZE_MAX},
    [PLXS_IDX_RACP_CCC]              = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,      RI_P,        0},
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
uint16_t plxs_att2handle(uint8_t attribute)
{
    // Get the address of the environment
    struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);
    uint16_t handle = p_plxs_env->shdl + attribute;

    if (PLXS_OPTYPE_SPOT_CHECK_ONLY == p_plxs_env->optype)
    {
        // adjust by not existent CONT_MEAS attributes
        if (attribute >= PLXS_IDX_FEATURES_CHAR)
        {
            handle -= PLXS_IDX_FEATURES_CHAR - PLXS_IDX_CONT_MEASUREMENT_CHAR;
        }
    }
    else if (PLXS_OPTYPE_CONTINUOUS_ONLY == p_plxs_env->optype)
    {
        // adjust by not existent SPOT_MEASUREMENT attributes
        if (attribute >= PLXS_IDX_CONT_MEASUREMENT_CHAR)
        {
            attribute += PLXS_IDX_CONT_MEASUREMENT_CHAR - PLXS_IDX_SPOT_MEASUREMENT_CHAR;
        }
    }

    return handle;
}

uint8_t plxs_handle2att(uint16_t handle)
{
    // Get the address of the environment
    struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);
    uint8_t att_idx = handle - p_plxs_env->shdl;

    if (PLXS_OPTYPE_SPOT_CHECK_ONLY == p_plxs_env->optype)
    {
        // if >4 then +3 (4-->7)
        if (att_idx > 4)
        {
            att_idx += PLXS_IDX_FEATURES_CHAR -PLXS_IDX_CONT_MEASUREMENT_CHAR;
        }
    }
    else if (PLXS_OPTYPE_CONTINUOUS_ONLY == p_plxs_env->optype)
    {
        // if >0 then +3 (1-->4)
        if (att_idx > 0)
        {
            att_idx += PLXS_IDX_CONT_MEASUREMENT_CHAR -PLXS_IDX_SPOT_MEASUREMENT_CHAR;
        }
    }

    return att_idx;
}
/**
 ****************************************************************************************
 * @brief Initialization of the PLXS module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env        Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task     Application task number.
 * @param[in]     sec_lvl      Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params     Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t plxs_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct plxs_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint8_t cfg_flag[] = {0xFF,0x0F};
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    if (PLXS_OPTYPE_SPOT_CHECK_ONLY == p_params->optype)
    {
        // mask off Continuous Measurement Characteristic
        // mask off Records Access Control Point Characteristic
        cfg_flag[0] = 0x8F;
        cfg_flag[1] = 0x01;
    }
    else if (PLXS_OPTYPE_CONTINUOUS_ONLY == p_params->optype)
    {
        // mask off SPOT-Measurement Characteristic
        cfg_flag[0] = 0xF1;
    }
    else
    {
        p_params->optype = 0;
    }
    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_PULSE_OXIMETER, (uint8_t *)&cfg_flag[0],
            PLXS_IDX_NB, NULL, p_env->task, &plxs_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS )) | PERM(SVC_MI, ENABLE));

    // Check if an error has occured
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate PLXS required environment variable
        struct plxs_env_tag *p_plxs_env =
                (struct plxs_env_tag *) ke_malloc(sizeof(struct plxs_env_tag), KE_MEM_ATT_DB);

        // Initialize PLXS environment
        p_env->env = (prf_env_t *) p_plxs_env;
        p_plxs_env->shdl = *p_start_hdl;
        // Keep the Type of Operation @see enum plxs_optype_id
        p_plxs_env->optype = p_params->optype;
        // Keep Supported Features @see common sup_feat
        p_plxs_env->sup_feat = p_params->sup_feat;
        // Keep meas_stat_sup @see common meas_stat_sup
        p_plxs_env->meas_stat_sup = p_params->meas_stat_sup;
        // Keep dev_stat_sup @see common dev_stat_sup
        p_plxs_env->dev_stat_sup = p_params->dev_stat_sup;

        // Multi Instantiated APP task
        p_plxs_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Multi Instantiated task ???depend on Multiple user support Feature
        p_plxs_env->prf_env.prf_task = p_env->task
                |  PERM(PRF_MI, ENABLE);

        // initialize environment variable
        p_env->id = TASK_ID_PLXS;
        plxs_task_init(&(p_env->desc));

        // Initialize CCC per connection
        memset(p_plxs_env->prfl_ind_cfg_spot, 0, sizeof(p_plxs_env->prfl_ind_cfg_spot));
        memset(p_plxs_env->prfl_ind_cfg_cont, 0, sizeof(p_plxs_env->prfl_ind_cfg_cont));
        memset(p_plxs_env->prfl_ind_cfg_racp, 0, sizeof(p_plxs_env->prfl_ind_cfg_racp));
        memset(p_plxs_env->racp_in_progress, 0, sizeof(p_plxs_env->racp_in_progress));

        // Store the Features into at database
        {
            uint8_t value[PLXS_IDX_FEAT_SIZE_MAX];
            uint16_t length;
            uint8_t *p_data = &value[0];
            uint16_t handle = plxs_att2handle(PLXS_IDX_FEATURES_VAL);

            co_write16p(p_data, p_plxs_env->sup_feat);
            p_data += 2;
            length = 2;
            
            if (GETB(p_plxs_env->sup_feat, PLXP_FEAT_MEAS_STATUS_SUP))
            {
                co_write16p(p_data, p_plxs_env->meas_stat_sup);
                length += 2; // sizeof(uint16_t)
                p_data += 2;
            }
            if (GETB(p_plxs_env->sup_feat, PLXP_FEAT_DEV_SENSOR_STATUS_SUP))
            {
                co_write24p(p_data, p_plxs_env->dev_stat_sup);
                length += 3; // sizeof(uint24_t)
            }
            attm_att_set_value(handle, length, 0, &value[0]);
        }

        // If we are here, database has been fulfilled with success, go to idle state
        for (uint8_t idx = 0; idx < BLE_CONNECTION_MAX ; idx++)
        {
            // Put PLXS in disabled state
            ke_state_set(KE_BUILD_ID(p_env->task, idx), PLXS_FREE);
        }
    }

    return (status);
}
/**
 ****************************************************************************************
 * @brief Handles Disconnection
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void plxs_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct plxs_env_tag *p_plxs_env = (struct plxs_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    p_plxs_env->prfl_ind_cfg_spot[conidx] = 0;
    p_plxs_env->prfl_ind_cfg_cont[conidx] = 0;
    p_plxs_env->prfl_ind_cfg_racp[conidx] = 0;
    p_plxs_env->racp_in_progress[conidx] = 0;

    // Put PLXS in disabled state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PLXS_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the PLXS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void plxs_destroy(struct prf_task_env *p_env)
{
    struct plxs_env_tag *p_plxs_env = (struct plxs_env_tag *) p_env->env;

    // cleanup environment variable for each task instances

    // free profile environment variables
    p_env->env = NULL;

    ke_free(p_plxs_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void plxs_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct plxs_env_tag *p_plxs_env = (struct plxs_env_tag *) p_env->env;
    p_plxs_env->prfl_ind_cfg_spot[conidx] = 0;
    p_plxs_env->prfl_ind_cfg_cont[conidx] = 0;
    p_plxs_env->prfl_ind_cfg_racp[conidx] = 0;
    p_plxs_env->racp_in_progress[conidx] = 0;

    // Put PLXS in IDLE state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PLXS_IDLE);}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// PLXS Task interface required by profile manager
const struct prf_task_cbs plxs_itf =
{
    (prf_init_fnct) plxs_init,
    plxs_destroy,
    plxs_create,
    plxs_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *plxs_prf_itf_get(void)
{
   return &plxs_itf;
}


#endif //(BLE_PLX_SERVER)

/// @} PLXS
