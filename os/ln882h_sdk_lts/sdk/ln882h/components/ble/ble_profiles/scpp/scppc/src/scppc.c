/**
 ****************************************************************************************
 *
 * @file scppc.c
 *
 * @brief Scan Parameters Profile Client implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup SCPPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_SP_CLIENT)

#include "scppc.h"
#include "scppc_task.h"
#include "gap.h"

#include "ke_mem.h"

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Initialization of the SCPPC module.
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
static uint8_t scppc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct scppc_env_tag *p_scppc_env =
            (struct scppc_env_tag *) ke_malloc(sizeof(struct scppc_env_tag), KE_MEM_ATT_DB);

    // allocate SCPPC required environment variable
    p_env->env = (prf_env_t *) p_scppc_env;

    p_scppc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_scppc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id                     = TASK_ID_SCPPC;
    scppc_task_init(&(p_env->desc));

    for (idx = 0; idx < SCPPC_IDX_MAX ; idx++)
    {
        p_scppc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), SCPPC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Destruction of the SCPPC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void scppc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct scppc_env_tag *p_scppc_env = (struct scppc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < SCPPC_IDX_MAX; idx++)
    {
        if (p_scppc_env->p_env[idx] != NULL)
        {
            ke_free(p_scppc_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_scppc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void scppc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put SCPP Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), SCPPC_IDLE);
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
static void scppc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct scppc_env_tag *p_scppc_env = (struct scppc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_scppc_env->p_env[conidx] != NULL)
    {
        ke_free(p_scppc_env->p_env[conidx]);
        p_scppc_env->p_env[conidx] = NULL;
    }

    // Put SCPP Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), SCPPC_FREE);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// SCPPC Task interface required by profile manager
const struct prf_task_cbs scppc_itf =
{
    scppc_init,
    scppc_destroy,
    scppc_create,
    scppc_cleanup,
};


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *scppc_prf_itf_get(void)
{
    return &scppc_itf;
}

void scppc_enable_rsp_send(struct scppc_env_tag *p_scppc_env, uint8_t conidx, uint8_t status)
{
    //send APP the details of the discovered attributes on SCPPC
    struct scppc_enable_rsp *p_rsp = KE_MSG_ALLOC(SCPPC_ENABLE_RSP,
                                                 prf_dst_task_get(&(p_scppc_env->prf_env) ,conidx),
                                                 prf_src_task_get(&(p_scppc_env->prf_env) ,conidx),
                                                 scppc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->scps = p_scppc_env->p_env[conidx]->scps;

        // If Scan Refresh Char. has been discovered
        if (p_scppc_env->p_env[conidx]->scps.chars[SCPPC_CHAR_SCAN_REFRESH].char_hdl != ATT_INVALID_HANDLE)
        {
            // Register SCPPC task in gatt for notifications
            prf_register_atthdl2gatt(&(p_scppc_env->prf_env), conidx, &p_scppc_env->p_env[conidx]->scps.svc);
        }
    }
    else
    {
        memset(&p_rsp->scps, 0 , sizeof(p_rsp->scps));
    }

    ke_msg_send(p_rsp);
}

void scppc_send_cmp_evt(struct scppc_env_tag *p_scppc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct scppc_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_scppc_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_scppc_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == SCPPC_BUSY)
    {
        ke_state_set(src_id, SCPPC_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(SCPPC_CMP_EVT, dest_id, src_id, scppc_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif /* (BLE_SP_CLIENT) */

/// @} SCPPC
