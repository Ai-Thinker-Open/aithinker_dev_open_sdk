/**
 ****************************************************************************************
 *
 * @file basc.c
 *
 * @brief Battery Service Client implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup BASC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "rwprf_config.h"
#if (BLE_BATT_CLIENT)

#include "gap.h"
#include "basc.h"
#include "basc_task.h"
#include "co_math.h"

#include "ke_mem.h"

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the BASC module.
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
static uint8_t basc_init (struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, void *p_param)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct basc_env_tag *p_basc_env =
            (struct basc_env_tag *) ke_malloc(sizeof(struct basc_env_tag), KE_MEM_ATT_DB);

    // allocate BASC required environment variable
    p_env->env = (prf_env_t *) p_basc_env;

    p_basc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_basc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_BASC;
    basc_task_init(&(p_env->desc));

    for (idx = 0; idx < BASC_IDX_MAX; idx++)
    {
        p_basc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), BASC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the BASC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void basc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct basc_env_tag *p_basc_env = (struct basc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < BASC_IDX_MAX; idx++)
    {
        if (p_basc_env->p_env[idx] != NULL)
        {
            if (p_basc_env->p_env[idx]->p_operation != NULL)
            {
                ke_free(p_basc_env->p_env[idx]->p_operation);
            }

            ke_free(p_basc_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_basc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void basc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put BAS Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BASC_IDLE);
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
static void basc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct basc_env_tag *p_basc_env = (struct basc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_basc_env->p_env[conidx] != NULL)
    {
        if (p_basc_env->p_env[conidx]->p_operation != NULL)
        {
            ke_free(p_basc_env->p_env[conidx]->p_operation);
        }
        ke_free(p_basc_env->p_env[conidx]);

        p_basc_env->p_env[conidx] = NULL;
    }

    /* Put BAS Client in Free state */
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BASC_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// BASC Task interface required by profile manager
const struct prf_task_cbs basc_itf =
{
    basc_init,
    basc_destroy,
    basc_create,
    basc_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *basc_prf_itf_get(void)
{
    return &basc_itf;
}


void basc_enable_rsp_send(struct basc_env_tag *p_basc_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on BASC
    struct basc_enable_rsp *p_rsp = KE_MSG_ALLOC(BASC_ENABLE_RSP,
                                                prf_dst_task_get(&(p_basc_env->prf_env) ,conidx),
                                                prf_src_task_get(&(p_basc_env->prf_env) ,conidx),
                                                basc_enable_rsp);

    p_rsp->status = status;
    p_rsp->bas_nb = 0;

    if (status == GAP_ERR_NO_ERROR)
    {
        // Counter
        uint8_t svc_inst;

        p_rsp->bas_nb = p_basc_env->p_env[conidx]->bas_nb;

        for (svc_inst = 0; svc_inst < co_min(p_basc_env->p_env[conidx]->bas_nb, BASC_NB_BAS_INSTANCES_MAX); svc_inst++)
        {
            p_rsp->bas[svc_inst] = p_basc_env->p_env[conidx]->bas[svc_inst];

            // Register BASC task in gatt for indication/notifications
            prf_register_atthdl2gatt(&(p_basc_env->prf_env), conidx, &p_basc_env->p_env[conidx]->bas[svc_inst].svc);
        }
    }

    ke_msg_send(p_rsp);
}

#endif /* (BLE_BATT_CLIENT) */

/// @} BASC
