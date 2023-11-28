/**
 ****************************************************************************************
 *
 * @file cgms.c
 *
 * @brief Continuous Glucose Monitoring Profile Service implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup CGMS
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

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "cgms.h"
#include "cgms_task.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

// default read perm
#define RD_P   (PERM(RD, ENABLE)        | PERM(RP, UNAUTH))
// default write perm
#define WR_P   (PERM(WRITE_REQ, ENABLE) | PERM(WP, UNAUTH))
// default notify perm
#define NTF_P  (PERM(NTF, ENABLE)       | PERM(NP, UNAUTH))
// ind perm
#define IND_P  (PERM(IND, ENABLE)       | PERM(IP, NO_AUTH))
// enable Read Indication (1 = Value not present in Database)
#define RI_P   PERM(RI, ENABLE)

/// Full CGMS Database Description - Used to add attributes into the database
static const struct attm_desc cgms_att_db[CGMS_IDX_NB] =
{
    // ATT Index                    | ATT UUID                   | Permission       | EXT PERM | MAX ATT SIZE
    // service attribute
    [CGMS_IDX_SVC]                  = {ATT_DECL_PRIMARY_SERVICE,    PERM(RD, ENABLE),   0,        0},
    // CGM Measurement Characteristic
    [CGMS_IDX_MEAS_CHAR]            = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),   0,        0},
    [CGMS_IDX_MEAS_VAL]             = {ATT_CHAR_CGM_MEASUREMENT,    NTF_P,              0,        0},
    [CGMS_IDX_MEAS_CCC]             = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,     RI_P,        0},
    // CGM Features Characteristic
    [CGMS_IDX_FEAT_CHAR]            = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),   0,        0},
    [CGMS_IDX_FEAT_VAL]             = {ATT_CHAR_CGM_FEATURES,       RD_P,               0, CGMS_FEAT_SIZE_MAX},
    // CGM Status Characteristic
    [CGMS_IDX_STATUS_CHAR]          = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),   0,        0},
    [CGMS_IDX_STATUS_VAL]           = {ATT_CHAR_CGM_STATUS,         RD_P,            RI_P,        0},
    // CGM Session Start Time Characteristic
    [CGMS_IDX_SESS_ST_TIME_CHAR]    = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),   0,        0},
    [CGMS_IDX_SESS_ST_TIME_VAL]     = {ATT_CHAR_CGM_SESSION_START,  RD_P | WR_P,     RI_P, CGMP_SESS_ST_TIME_SIZE_MAX},
    // CGM Session Run Time Characteristic
    [CGMS_IDX_SESS_RUN_TIME_CHAR]   = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [CGMS_IDX_SESS_RUN_TIME_VAL]    = {ATT_CHAR_CGM_SESSION_RUN,    RD_P,            RI_P,        0},
    // CGM Record Access Control Point Characteristic
    [CGMS_IDX_RACP_CHAR]            = {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [CGMS_IDX_RACP_VAL]             = {ATT_CHAR_REC_ACCESS_CTRL_PT, WR_P | IND_P,        0, CGMS_RACP_SIZE_MAX},
    [CGMS_IDX_RACP_CCC]             = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,     RI_P,        0},
    // CGM Specific Ops Control Point Characteristic
    [CGMS_IDX_SPEC_OPS_CTRL_PT_CHAR]= {ATT_DECL_CHARACTERISTIC,     PERM(RD, ENABLE),    0,        0},
    [CGMS_IDX_SPEC_OPS_CTRL_PT_VAL] = {ATT_CHAR_CGM_SPECIFIC_OPS_CTRL_PT, WR_P | IND_P,  0, CGMS_OPS_CTRL_PT_SIZE_MAX},
    [CGMS_IDX_SPEC_OPS_CTRL_PT_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,    RD_P | WR_P,     RI_P,        0},
};

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the CGMS module.
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
 * @param[in]     p_param      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t cgms_init(struct prf_task_env *p_env, uint16_t *p_start_hdl,
        uint16_t app_task, uint8_t sec_lvl, struct cgms_db_cfg *p_param)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint8_t cfg_flag[] = {0xFF,0xFF,0x03};
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_CONTINUOUS_GLUCOSE_MONITORING, (uint8_t *)&cfg_flag[0],
            CGMS_IDX_NB, NULL, p_env->task, &cgms_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS )) | PERM(SVC_MI, ENABLE));

    // Check if an error has occured
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate CGMS required environment variable
        struct cgms_env_tag *p_cgms_env =
                (struct cgms_env_tag* ) ke_malloc(sizeof(struct cgms_env_tag), KE_MEM_ATT_DB);

        // Initialize CGMS environment

        p_env->env           = (prf_env_t*) p_cgms_env;
        p_cgms_env->shdl     = *p_start_hdl;

        // Keep CGM Features @see enum cgm_feat_bf
        p_cgms_env->cgm_feature = p_param->cgm_feature;

        // Multi Instantiated APP task
        p_cgms_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Multi Instantiated app task 
        p_cgms_env->prf_env.prf_task    = p_env->task
                |  PERM(PRF_MI, ENABLE);

        // initialize environment variable
        p_env->id                     = TASK_ID_CGMS;
        cgms_task_init(&(p_env->desc));

        // Store the Features into AT database
        uint8_t value[CGMS_FEAT_SIZE_MAX];
        uint8_t *p_val = &value[0];
        uint16_t handle = cgms_att2handle(CGMS_IDX_FEAT_VAL);
        // e2e crc to protect the fields
        // predefined value when crc is not supported
        // this field exist in all the cases
        uint16_t crc = 0xFFFF;;

        co_write24p(p_val, p_param->cgm_feature);
        p_val += 3;
        *p_val++ = ((p_param->sample_location) << 4) | p_param->type_sample;

        if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
        {
            crc = prf_e2e_crc(&value[0], CGMS_FEAT_SIZE_MAX - 2);
        }

        co_write16p(p_val, crc);

        attm_att_set_value(handle, CGMS_FEAT_SIZE_MAX, 0, &value[0]);

        // If we are here, database has been fulfilled with success, go to idle state
        for(uint8_t idx = 0; idx < BLE_CONNECTION_MAX ; idx++)
        {
            /* Put CGMS in disabled state */
            ke_state_set(KE_BUILD_ID(p_env->task, idx), CGMS_FREE);
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
static void cgms_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    /* Put CGMS in disabled state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), CGMS_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the CGMS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env     Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void cgms_destroy(struct prf_task_env *p_env)
{
    struct cgms_env_tag *p_cgms_env = (struct cgms_env_tag *) p_env->env;

    // cleanup environment variable for each task instances

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_cgms_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void cgms_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct cgms_env_tag *p_cgms_env = (struct cgms_env_tag*) p_env->env;
    p_cgms_env->prfl_state[conidx] = CGMS_PRFL_STATE_CLEAR;

    /* Put CGMS in IDLE state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), CGMS_IDLE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// CGMS Task interface required by profile manager
const struct prf_task_cbs cgms_itf =
{
    (prf_init_fnct) cgms_init,
    cgms_destroy,
    cgms_create,
    cgms_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *cgms_prf_itf_get(void)
{
   return &cgms_itf;
}

uint16_t cgms_att2handle(uint8_t att_idx)
{
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
    uint16_t handle = p_cgms_env->shdl + att_idx;

    return handle;
}

uint8_t cgms_handle2att(uint16_t handle)
{
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
    uint8_t att_idx = handle - p_cgms_env->shdl;

    return att_idx;
}

#endif //(BLE_CGM_SERVER)

/// @} CGMS
