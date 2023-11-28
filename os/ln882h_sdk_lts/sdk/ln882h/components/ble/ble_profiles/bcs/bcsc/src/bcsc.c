/**
 ****************************************************************************************
 *
 * @file bcsc.c
 *
 * @brief Body Composition Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup BCSC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_BCS_CLIENT)
#include "prf.h"
#include "bcs_common.h"

#include "bcsc_task.h"
#include "bcsc.h"
#include "gap.h"

#include "ke_mem.h"

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
 * @brief Initialization of the BCSC module.
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
static uint8_t bcsc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct bcsc_env_tag *p_bcsc_env =
            (struct bcsc_env_tag *) ke_malloc(sizeof(struct bcsc_env_tag), KE_MEM_ATT_DB);

    // allocate BCSC required environment variable
    p_env->env = (prf_env_t *) p_bcsc_env;

    p_bcsc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_bcsc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_BCSC;
    bcsc_task_init(&(p_env->desc));

    for(idx = 0; idx < BCSC_IDX_MAX; idx++)
    {
        p_bcsc_env->env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), BCSC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void bcsc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct bcsc_env_tag *p_bcsc_env = (struct bcsc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if(p_bcsc_env->env[conidx] != NULL)
    {
        if (p_bcsc_env->env[conidx]->meas_evt_msg != 0)
        {
            ke_msg_free(p_bcsc_env->env[conidx]->meas_evt_msg);
        }

        ke_free(p_bcsc_env->env[conidx]);
        p_bcsc_env->env[conidx] = NULL;
    }

    /* Put BSC Client in Free state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BCSC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the BCSC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void bcsc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct bcsc_env_tag *p_bcsc_env = (struct bcsc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for(idx = 0; idx < BCSC_IDX_MAX ; idx++)
    {
        bcsc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_bcsc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void bcsc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    /* Put BCS Client in Idle state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BCSC_IDLE);
}

/// BCSC Task interface required by profile manager
const struct prf_task_cbs bcsc_itf =
{
        bcsc_init,
        bcsc_destroy,
        bcsc_create,
        bcsc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* bcsc_prf_itf_get(void)
{
   return &bcsc_itf;
}

void bcsc_enable_rsp_send(struct bcsc_env_tag *p_bcsc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on BCSC
    struct  bcsc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            BCSC_ENABLE_RSP,
            prf_dst_task_get(&(p_bcsc_env->prf_env), conidx),
            prf_src_task_get(&(p_bcsc_env->prf_env), conidx),
            bcsc_enable_rsp);

    p_rsp->status = status;
    p_bcsc_env->env[conidx]->meas_evt_msg = 0;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->bcs = p_bcsc_env->env[conidx]->bcs;

        // Register BCSC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_bcsc_env->prf_env), conidx, &(p_bcsc_env->env[conidx]->bcs.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_bcsc_env->prf_env), conidx), BCSC_BUSY);
    }
    else
    {
        memset(&p_rsp->bcs, 0 , sizeof(p_rsp->bcs));
        // Enabled failed - so go back to the FREE state
        ke_state_set(prf_src_task_get(&(p_bcsc_env->prf_env), conidx), BCSC_FREE);
    }

    ke_msg_send(p_rsp);
}

void bcsc_send_cmp_evt(struct bcsc_env_tag *p_bcsc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct bcsc_cmp_evt *p_evt = NULL;

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_bcsc_env->prf_env), conidx)) == BCSC_BUSY)
    {
        ke_state_set(prf_src_task_get(&(p_bcsc_env->prf_env), conidx), BCSC_IDLE);
    }

    p_bcsc_env->env[conidx]->op_pending = 0;
    // Send the message
    p_evt = KE_MSG_ALLOC(BCSC_CMP_EVT,prf_dst_task_get(&(p_bcsc_env->prf_env), conidx),
                         prf_src_task_get(&(p_bcsc_env->prf_env), conidx),
                         bcsc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

#endif //(BLE_BCS_CLIENT)

/// @} BCSC
