/**
 ****************************************************************************************
 *
 * @file bcss.c
 *
 * @brief Body Composition Service  implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup BCSS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "prf.h"
#if (BLE_BCS_SERVER)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "prf_utils.h"
#include "ke_mem.h"
#include "co_utils.h"

#include "bcss.h"
#include "bcss_task.h"
#include "bcs_common.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// default read perm
#define RD_P   (PERM(RD, ENABLE)        | PERM(RP, NO_AUTH))
/// default write perm
#define WR_P   (PERM(WRITE_REQ, ENABLE) | PERM(WP, NO_AUTH))
/// ind perm
#define IND_P  (PERM(IND, ENABLE)       | PERM(IP, NO_AUTH))
/// enable Read Indication (1 = Value not present in Database)
#define EXT_P  (PERM(RI, ENABLE))


/// Full BCSS Database Description - Used to add attributes into the database
static const struct attm_desc bcss_att_db[BCSS_IDX_NB] =
{
    /// ATT Index                 | ATT UUID                     | Permission | EXT PERM | MAX ATT SIZE
    // Weight SCale Service Declaration
    [BCSS_IDX_SVC]             = {ATT_DECL_PRIMARY_SERVICE,           RD_P,       0,         0},

    // Weight SCale Feature
    [BCSS_IDX_FEAT_CHAR]       = {ATT_DECL_CHARACTERISTIC,            RD_P,       0,         0},
    // Descriptor Value Changed Characteristic Value
    [BCSS_IDX_FEAT_VAL]        = {ATT_CHAR_BODY_COMPOSITION_FEATURE,  RD_P,     EXT_P,    BCSS_FEAT_VAL_SIZE},

    // Weight SCale Measurement
    [BCSS_IDX_MEAS_CHAR]       = {ATT_DECL_CHARACTERISTIC,            RD_P,       0,         0},
    // Weight SCale Measurement Characteristic Value
    [BCSS_IDX_MEAS_IND]        = {ATT_CHAR_BODY_COMPOSITION_MEASUREMENT, IND_P,   0,      BCSS_MEAS_IND_SIZE},
    // Weight SCale Measurement Characteristic - Client Characteristic Configuration Descriptor
    [BCSS_IDX_MEAS_CCC]        = {ATT_DESC_CLIENT_CHAR_CFG,       RD_P | WR_P,  EXT_P,    sizeof(uint16_t)},
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the BCSS module.
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
static uint8_t bcss_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct bcss_db_cfg *p_param)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint8_t cfg_flag = (1 << BCSS_IDX_NB) - 1;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;
    uint8_t svc_perm;

    // Add service in the database
    if (p_param->secondary_service)
    {
        svc_perm = PERM(SVC_SECONDARY,ENABLE);
    }
    else
    {
        svc_perm = PERM(SVC_SECONDARY,DISABLE);
    }

    status = attm_svc_create_db(p_start_hdl, ATT_SVC_BODY_COMPOSITION, (uint8_t *)&cfg_flag,
        BCSS_IDX_NB, NULL, p_env->task, &bcss_att_db[0],
        (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, ENABLE) | svc_perm);

    // Check if an error has occured
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate BCSS required environment variable
        struct bcss_env_tag *p_bcss_env =
                (struct bcss_env_tag *) ke_malloc(sizeof(struct bcss_env_tag), KE_MEM_ATT_DB);

        // Initialize BCSS environment

        p_env->env = (prf_env_t *) p_bcss_env;
        p_bcss_env->shdl = *p_start_hdl;
        p_bcss_env->feature = p_param->feature;

        // Mono Instantiated APP task
        p_bcss_env->prf_env.app_task = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Multi Instantiated task depending on Multiple user support Feature
        p_bcss_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

        // initialize environment variable
        p_env->id = TASK_ID_BCSS;
        bcss_task_init(&(p_env->desc));

        // Initialize CCC per connection
        memset(p_bcss_env->prfl_ind_cfg, 0, BLE_CONNECTION_MAX);
        // If we are here, database has been fulfilled with success, go to idle state
        for(uint8_t idx = 0; idx < BLE_CONNECTION_MAX ; idx++)
        {
            // Put WSCS in disabled state
            ke_state_set(KE_BUILD_ID(p_env->task, idx), BCSS_FREE);
        }
    }

    return status;
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
static void bcss_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct bcss_env_tag *p_bcss_env = (struct bcss_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    p_bcss_env->prfl_ind_cfg[conidx] = 0;

    /* Put BCSS in disabled state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BCSS_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the BCSS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void bcss_destroy(struct prf_task_env *p_env)
{
    struct bcss_env_tag *p_bcss_env = (struct bcss_env_tag *) p_env->env;

    // cleanup environment variable for each task instances

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_bcss_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void bcss_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct bcss_env_tag *p_bcss_env = (struct bcss_env_tag *) p_env->env;

    p_bcss_env->prfl_ind_cfg[conidx] = 0;
    // Change state to IDLE
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BCSS_IDLE);
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// BCSS Task interface required by profile manager
const struct prf_task_cbs bcss_itf =
{
    (prf_init_fnct) bcss_init,
    bcss_destroy,
    bcss_create,
    bcss_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *bcss_prf_itf_get(void)
{
   return &bcss_itf;
}


void bcss_send_cmp_evt(struct bcss_env_tag *p_bcss_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct bcss_cmp_evt *p_evt = KE_MSG_ALLOC(BCSS_CMP_EVT,
                                             prf_dst_task_get(&(p_bcss_env->prf_env), conidx),
                                             prf_src_task_get(&(p_bcss_env->prf_env), conidx),
                                             bcss_cmp_evt);

    p_evt->status     = status;
    p_evt->operation = operation;
    ke_msg_send(p_evt);
}

uint8_t bcss_get_mass_resol(uint32_t feature)
{
    uint8_t mass_resol;

    // Imperial - Weight and Mass in units of pound (lb) and Height in units of inch (in)
    switch(GETF(feature, BCS_FEAT_MASS_RESOL))
    {
        case BCS_MASS_RESOL_05kg_1lb:
        {
            mass_resol = BCS_MASS_RESOL_05kg_1lb;
        } break;

        case BCS_MASS_RESOL_02kg_05lb:
        {
            mass_resol = BCS_MASS_RESOL_02kg_05lb;
        } break;

        case BCS_MASS_RESOL_01kg_02lb:
        {
            mass_resol = BCS_MASS_RESOL_01kg_02lb;
        } break;

        case BCS_MASS_RESOL_005kg_01lb:
        {
            mass_resol = BCS_MASS_RESOL_005kg_01lb;
        } break;

        case BCS_MASS_RESOL_002kg_005lb:
        {
            mass_resol = BCS_MASS_RESOL_002kg_005lb;
        } break;

        case BCS_MASS_RESOL_001kg_002lb:
        {
            mass_resol = BCS_MASS_RESOL_001kg_002lb;
        } break;

        case BCS_MASS_RESOL_0005kg_001lb:
        {
            mass_resol = BCS_MASS_RESOL_0005kg_001lb;
        } break;

        case BCS_MASS_RESOL_NOT_SPECIFIED:
        default:
        {
            mass_resol = BCS_MASS_RESOL_NOT_SPECIFIED;
        } break;
    }

    return mass_resol;
}

uint8_t bcss_get_hght_resol(uint32_t feature)
{
    uint8_t hght_resol;

    // Imperial - Weight and Mass in units of pound (lb) and Height in units of inch (in)
    switch(GETF(feature, BCS_FEAT_HGHT_RESOL))
    {
        case BCS_HGHT_RESOL_001mtr_1inch:
        {
            hght_resol = BCS_HGHT_RESOL_001mtr_1inch;
        } break;

        case BCS_HGHT_RESOL_0005mtr_05inch:
        {
            hght_resol = BCS_HGHT_RESOL_0005mtr_05inch;
        } break;

        case BCS_HGHT_RESOL_0001mtr_01inch:
        {
            hght_resol = BCS_HGHT_RESOL_0001mtr_01inch;
        } break;

        case BCS_HGHT_RESOL_NOT_SPECIFIED:
        default:
        {
            hght_resol = BCS_HGHT_RESOL_NOT_SPECIFIED;
        } break;
    }

    return hght_resol;
}
#endif //(BLE_BCS_SERVER)

/// @} BCSS
