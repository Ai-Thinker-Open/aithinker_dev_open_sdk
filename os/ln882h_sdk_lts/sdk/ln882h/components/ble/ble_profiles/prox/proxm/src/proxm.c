/**
 ****************************************************************************************
 *
 * @file proxm.c
 *
 * @brief Proximity monitor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup PROXM
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_PROX_MONITOR)

#include "proxm.h"
#include "proxm_task.h"
#include "prf_utils.h"
#include "gap.h"

#include "ke_mem.h"

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */



/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialization of the PROXM module.
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
static uint8_t proxm_init (struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct proxm_env_tag *p_proxm_env =
            (struct proxm_env_tag *) ke_malloc(sizeof(struct proxm_env_tag), KE_MEM_ATT_DB);

    // allocate PROXM required environment variable
    p_env->env = (prf_env_t *) p_proxm_env;

    p_proxm_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_proxm_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_PROXM;
    proxm_task_init(&(p_env->desc));

    for (idx = 0; idx < PROXM_IDX_MAX; idx++)
    {
        p_proxm_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), PROXM_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the PROXM module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void proxm_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct proxm_env_tag *p_proxm_env = (struct proxm_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < PROXM_IDX_MAX; idx++)
    {
        if (p_proxm_env->p_env[idx] != NULL)
        {
            ke_free(p_proxm_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_proxm_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void proxm_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put PROX Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PROXM_IDLE);
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
static void proxm_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct proxm_env_tag *p_proxm_env = (struct proxm_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_proxm_env->p_env[conidx] != NULL)
    {
        ke_free(p_proxm_env->p_env[conidx]);
        p_proxm_env->p_env[conidx] = NULL;
    }

    // Put PROX Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PROXM_FREE);
}
/*
* GLOBAL VARIABLE DEFINITIONS
****************************************************************************************
*/

/// PROXM Task interface required by profile manager
const struct prf_task_cbs proxm_itf =
{
    (prf_init_fnct) proxm_init,
    proxm_destroy,
    proxm_create,
    proxm_cleanup,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *proxm_prf_itf_get(void)
{
    return &proxm_itf;
}

void proxm_enable_rsp_send(struct proxm_env_tag *p_proxm_env, uint8_t conidx, uint8_t status)
{
    // format response to app
    struct proxm_enable_rsp *p_cfm = KE_MSG_ALLOC(PROXM_ENABLE_RSP,
                                                prf_dst_task_get(&(p_proxm_env->prf_env), conidx),
                                                prf_src_task_get(&(p_proxm_env->prf_env), conidx),
                                                proxm_enable_rsp);

    p_cfm->lls    = p_proxm_env->p_env[conidx]->prox[PROXM_LK_LOSS_SVC];
    p_cfm->ias    = p_proxm_env->p_env[conidx]->prox[PROXM_IAS_SVC];
    p_cfm->txps   = p_proxm_env->p_env[conidx]->prox[PROXM_TX_POWER_SVC];
    p_cfm->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        // Go to IDLE state
        ke_state_set(prf_src_task_get(&(p_proxm_env->prf_env), conidx), PROXM_IDLE);
    }
    else
    {
        // clean-up environment variable allocated for task instance
        ke_free(p_proxm_env->p_env[conidx]);
        p_proxm_env->p_env[conidx] = NULL;
    }

    ke_msg_send(p_cfm);
}

uint8_t proxm_validate_request(struct proxm_env_tag *p_proxm_env, uint8_t conidx, uint8_t char_code)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    // check if feature val characteristic exists
    if (p_proxm_env->p_env[conidx]->prox[char_code].characts[0].val_hdl == ATT_INVALID_HANDLE)
    {
        status = PRF_ERR_INEXISTENT_HDL;
    }

    return (status);
}

void proxm_send_cmp_evt(struct proxm_env_tag *p_proxm_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct proxm_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_proxm_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_proxm_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == PROXM_BUSY)
    {
        ke_state_set(src_id, PROXM_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(PROXM_CMP_EVT, dest_id, src_id, proxm_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}


#endif //BLE_PROX_MONITOR

/// @} PROXM
