/**
 ****************************************************************************************
 *
 * @file disc.c
 *
 * @brief Device Information Service Client Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DISC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_DIS_CLIENT)
#include "disc.h"
#include "../api/disc_task.h"
#include "gap.h"

#include "ke_mem.h"


/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the DISC module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env         Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl   Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task      Application task number.
 * @param[in]     sec_lvl       Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t disc_init (struct prf_task_env *p_env, uint16_t *p_start_hdl,
        uint16_t app_task, uint8_t sec_lvl, void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct disc_env_tag *p_disc_env =
            (struct disc_env_tag *) ke_malloc(sizeof(struct disc_env_tag), KE_MEM_ATT_DB);

    // allocate DISC required environment variable
    p_env->env = (prf_env_t *) p_disc_env;

    p_disc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_disc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_DISC;
    disc_task_init(&(p_env->desc));

    for (idx = 0; idx < DISC_IDX_MAX; idx++)
    {
        p_disc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), DISC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the DISC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void disc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct disc_env_tag *p_disc_env = (struct disc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < DISC_IDX_MAX; idx++)
    {
        if (p_disc_env->p_env[idx] != NULL)
        {
            ke_free(p_disc_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_disc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void disc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put DIS Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), DISC_IDLE);
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
static void disc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct disc_env_tag *p_disc_env = (struct disc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_disc_env->p_env[conidx] != NULL)
    {
        ke_free(p_disc_env->p_env[conidx]);
        p_disc_env->p_env[conidx] = NULL;
    }

    // Put DIS Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), DISC_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// DISC Task interface required by profile manager
const struct prf_task_cbs disc_itf =
{
    disc_init,
    disc_destroy,
    disc_create,
    disc_cleanup,
};


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* disc_prf_itf_get(void)
{
    return &disc_itf;
}


void disc_enable_rsp_send(struct disc_env_tag *p_disc_env, uint8_t conidx, uint8_t status)
{
    // Send APP the details of the discovered attributes on DISC
    struct disc_enable_rsp *p_rsp = KE_MSG_ALLOC(DISC_ENABLE_RSP,
                                                prf_dst_task_get(&(p_disc_env->prf_env), conidx),
                                                prf_src_task_get(&(p_disc_env->prf_env), conidx),
                                                disc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->dis = p_disc_env->p_env[conidx]->dis;

        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_disc_env->prf_env), conidx), DISC_IDLE);
    }
    else
    {
        // clean-up environment variable allocated for task instance
        ke_free(p_disc_env->p_env[conidx]);
        p_disc_env->p_env[conidx] = NULL;
    }

    ke_msg_send(p_rsp);
}

void disc_send_cmp_evt(struct disc_env_tag *p_disc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct disc_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_disc_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_disc_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == DISC_BUSY)
    {
        ke_state_set(src_id, DISC_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(DISC_CMP_EVT, dest_id, src_id, disc_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif //BLE_DIS_CLIENT

/// @} DISC
