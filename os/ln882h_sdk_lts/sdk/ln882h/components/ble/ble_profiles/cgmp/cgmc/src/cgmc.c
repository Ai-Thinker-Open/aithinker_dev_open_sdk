/**
 ****************************************************************************************
 *
 * @file cgmc.c
 *
 * @brief Continuous Glucose Monitoring Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup CGMC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_CGM_CLIENT)
#include "prf.h"
#include "cgmp_common.h"

#include "cgmc_task.h"
#include "cgmc.h"
#include "gap.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the CGMC module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env        Collector or Service allocated environment data.
 * @param[in|out] start_hdl    Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task     Application task number.
 * @param[in]     sec_lvl      Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params     Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t cgmc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct cgmc_env_tag *p_cgmc_env =
            (struct cgmc_env_tag *) ke_malloc(sizeof(struct cgmc_env_tag), KE_MEM_ATT_DB);

    // allocate CGMC required environment variable
    p_env->env = (prf_env_t *) p_cgmc_env;

    p_cgmc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_cgmc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_CGMC;
    cgmc_task_init(&(p_env->desc));

    for(idx = 0; idx < CGMC_IDX_MAX ; idx++)
    {
        p_cgmc_env->env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), CGMC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx       Connection index
 * @param[in]        reason       Detach reason
 ****************************************************************************************
 */
static void cgmc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct cgmc_env_tag *p_cgmc_env = (struct cgmc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if(p_cgmc_env->env[conidx] != NULL)
    {
        ke_free(p_cgmc_env->env[conidx]);
        p_cgmc_env->env[conidx] = NULL;
    }

    /* Put WSCP Client in Free state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), CGMC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the CGMC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void cgmc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct cgmc_env_tag* p_cgmc_env = (struct cgmc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for(idx = 0; idx < CGMC_IDX_MAX ; idx++)
    {
        cgmc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_cgmc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void cgmc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    /* Put WSC Client in Idle state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), CGMC_IDLE);
}

/// CGMC Task interface required by profile manager
const struct prf_task_cbs cgmc_itf =
{
        cgmc_init,
        cgmc_destroy,
        cgmc_create,
        cgmc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* cgmc_prf_itf_get(void)
{
   return &cgmc_itf;
}

void cgmc_enable_rsp_send(struct cgmc_env_tag *p_cgmc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on WSCP
    struct cgmc_enable_rsp * rsp = KE_MSG_ALLOC(
            CGMC_ENABLE_RSP,
            prf_dst_task_get(&(p_cgmc_env->prf_env), conidx),
            prf_src_task_get(&(p_cgmc_env->prf_env), conidx),
            cgmc_enable_rsp);

    rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        // Provide all Discovered Characteristics and Descritors
        rsp->cgmc = p_cgmc_env->env[conidx]->cgmc;

        // Register CGMC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_cgmc_env->prf_env), conidx, &(p_cgmc_env->env[conidx]->cgmc.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_cgmc_env->prf_env), conidx), CGMC_IDLE);
    }
    else
    {
        // clean-up environment variable allocated for task instance
        if(p_cgmc_env->env[conidx] != NULL)
        {
            ke_free(p_cgmc_env->env[conidx]);
            p_cgmc_env->env[conidx] = NULL;
        }
    }

    ke_msg_send(rsp);
}

void cgmc_send_cmp_evt(struct cgmc_env_tag *p_cgmc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct cgmc_cmp_evt *p_evt = NULL;

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_cgmc_env->prf_env), conidx)) == CGMC_BUSY)
    {
        ke_state_set(prf_src_task_get(&(p_cgmc_env->prf_env), conidx), CGMC_IDLE);
    }

    // Send the message
    p_evt = KE_MSG_ALLOC(CGMC_CMP_EVT,prf_dst_task_get(&(p_cgmc_env->prf_env), conidx),
                      prf_src_task_get(&(p_cgmc_env->prf_env), conidx),
                      cgmc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

#endif //(BLE_CGM_CLIENT)

/// @} CGMC
