/**
 ****************************************************************************************
 *
 * @file plxc.c
 *
 * @brief Pulse Oximeter Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup PLXC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_PLX_CLIENT)
#include "prf.h"
#include "plxp_common.h"

#include "plxc_task.h"
#include "plxc.h"
#include "gap.h"

#include "ke_mem.h"

/*
 * GLOBAL VARIABLES DECLARATION
 ****************************************************************************************
 */


/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the PLXC module.
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
static uint8_t plxc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl,
        uint16_t app_task, uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct plxc_env_tag *p_plxc_env =
            (struct plxc_env_tag *) ke_malloc(sizeof(struct plxc_env_tag), KE_MEM_ATT_DB);

    // allocate PLXC required environment variable
    p_env->env = (prf_env_t *) p_plxc_env;

    p_plxc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_plxc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_PLXC;

    plxc_task_init(&(p_env->desc));

    for (idx = 0; idx < PLXC_IDX_MAX ; idx++)
    {
        p_plxc_env->p_env[idx] = NULL;

        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), PLXC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void plxc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct plxc_env_tag *p_plxc_env = (struct plxc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_plxc_env->p_env[conidx] != NULL)
    {
        ke_free(p_plxc_env->p_env[conidx]);
        p_plxc_env->p_env[conidx] = NULL;
    }

    // Put PLXP Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PLXC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the PLXC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void plxc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct plxc_env_tag* p_plxc_env = (struct plxc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < PLXC_IDX_MAX ; idx++)
    {
        plxc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_plxc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void plxc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put WSC Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), PLXC_IDLE);
}

/// PLXC Task interface required by profile manager
const struct prf_task_cbs plxc_itf =
{
    plxc_init,
    plxc_destroy,
    plxc_create,
    plxc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *plxc_prf_itf_get(void)
{
   return &plxc_itf;
}

void plxc_enable_rsp_send(struct plxc_env_tag *p_plxc_env, uint8_t conidx, uint8_t status,
    uint16_t sup_feat, uint16_t meas_stat_sup, uint32_t dev_stat_sup)
{
    // Send to APP the details of the discovered attributes on WSCP
    struct plxc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            PLXC_ENABLE_RSP,
            prf_dst_task_get(&(p_plxc_env->prf_env), conidx),
            prf_src_task_get(&(p_plxc_env->prf_env), conidx),
            plxc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        // Provide all Discovered Characteristics and Descriptors
        p_rsp->plxc = p_plxc_env->p_env[conidx]->plxc;
        // Supported Features
        p_rsp->sup_feat = sup_feat;
        // If enabled in  Supported Features
        p_rsp->meas_stat_sup = meas_stat_sup;
        // If enabled in  Supported Features
        p_rsp->dev_stat_sup = dev_stat_sup;

        // Register PLXC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_plxc_env->prf_env), conidx, &(p_plxc_env->p_env[conidx]->plxc.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_plxc_env->prf_env), conidx), PLXC_IDLE);
    }
    else
    {
        memset(&p_rsp->plxc, 0 , sizeof(p_rsp->plxc));
        // Provide all Discovered Characteristics and Descriptors
        p_rsp->plxc = p_plxc_env->p_env[conidx]->plxc;
        // Enabled failed - so go back to the FREE state
        ke_state_set(prf_src_task_get(&(p_plxc_env->prf_env), conidx), PLXC_FREE);
    }

    ke_msg_send(p_rsp);
}

void plxc_send_cmp_evt(struct plxc_env_tag *p_plxc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct plxc_cmp_evt *p_evt;

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_plxc_env->prf_env), conidx)) == PLXC_BUSY)
    {
        ke_state_set(prf_src_task_get(&(p_plxc_env->prf_env), conidx), PLXC_IDLE);
    }

    // Send the message
    p_evt = KE_MSG_ALLOC(PLXC_CMP_EVT,
                         prf_dst_task_get(&(p_plxc_env->prf_env), conidx),
                         prf_src_task_get(&(p_plxc_env->prf_env), conidx),
                         plxc_cmp_evt);

    p_evt->operation = operation;
    p_evt->status = status;

    ke_msg_send(p_evt);
}

#endif //(BLE_PLX_CLIENT)

/// @} PLXC
