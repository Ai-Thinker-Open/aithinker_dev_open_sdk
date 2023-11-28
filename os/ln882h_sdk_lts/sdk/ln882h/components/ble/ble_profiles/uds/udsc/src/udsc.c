/**
 ****************************************************************************************
 *
 * @file udsc.c
 *
 * @brief User Data Service Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup UDSC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_UDS_CLIENT)
#include "prf.h"
#include "uds_common.h"

#include "udsc_task.h"
#include "udsc.h"
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
 * @brief Initialization of the UDSC module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env           Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl     Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task      Application task number.
 * @param[in]     sec_lvl       Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params      Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t udsc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl,
        uint16_t app_task, uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct udsc_env_tag *p_udsc_env =
            (struct udsc_env_tag *) ke_malloc(sizeof(struct udsc_env_tag), KE_MEM_ATT_DB);

    // allocate UDSC required environment variable
    p_env->env = (prf_env_t *) p_udsc_env;

    p_udsc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_udsc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_UDSC;
    udsc_task_init(&(p_env->desc));

    for (idx = 0; idx < UDSC_IDX_MAX ; idx++)
    {
        p_udsc_env->p_env[idx] = NULL;

        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), UDSC_FREE);
    }

    return GAP_ERR_NO_ERROR;
}

/**
 ****************************************************************************************
 * @brief Clean-up connection dedicated environment parameters
 * This function performs cleanup of ongoing operations
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 * @param[in]        reason     Detach reason
 ****************************************************************************************
 */
static void udsc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct udsc_env_tag* p_udsc_env = (struct udsc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_udsc_env->p_env[conidx] != NULL)
    {
        if (p_udsc_env->p_env[conidx]->p_op != NULL)
        {
            ke_free(ke_param2msg(p_udsc_env->p_env[conidx]->p_op));
        }

        ke_free(p_udsc_env->p_env[conidx]);
        p_udsc_env->p_env[conidx] = NULL;
    }

    // Put UDS Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), UDSC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the UDSC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void udsc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct udsc_env_tag *p_udsc_env = (struct udsc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < UDSC_IDX_MAX ; idx++)
    {
        udsc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_udsc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void udsc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put UDS Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), UDSC_IDLE);
}

/// UDSC Task interface required by profile manager
const struct prf_task_cbs udsc_itf =
{
    udsc_init,
    udsc_destroy,
    udsc_create,
    udsc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
const struct prf_task_cbs *udsc_prf_itf_get(void)
{
   return &udsc_itf;
}

void udsc_enable_rsp_send(struct udsc_env_tag *p_udsc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on UDSP
    struct udsc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            UDSC_ENABLE_RSP,
            prf_dst_task_get(&(p_udsc_env->prf_env), conidx),
            prf_src_task_get(&(p_udsc_env->prf_env), conidx),
            udsc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {

        p_rsp->uds = p_udsc_env->p_env[conidx]->uds;

        // Register UDSC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_udsc_env->prf_env), conidx, &(p_udsc_env->p_env[conidx]->uds.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_udsc_env->prf_env), conidx), UDSC_BUSY);
    }
    else
    {
        memset(&p_rsp->uds, 0, sizeof(p_rsp->uds));
        // Enabled failed - so go back to the FREE state
        ke_state_set(prf_src_task_get(&(p_udsc_env->prf_env), conidx), UDSC_FREE);
    }

    ke_msg_send(p_rsp);
}

void udsc_send_cmp_evt(struct udsc_env_tag *p_udsc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct udsc_cmp_evt *p_evt;

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_udsc_env->prf_env), conidx)) == UDSC_BUSY)
    {
        ke_state_set(prf_src_task_get(&(p_udsc_env->prf_env), conidx), UDSC_IDLE);
    }

    // Send the message
    p_evt = KE_MSG_ALLOC(UDSC_CMP_EVT,
                         prf_dst_task_get(&(p_udsc_env->prf_env), conidx),
                         prf_src_task_get(&(p_udsc_env->prf_env), conidx),
                         udsc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

#endif //(BLE_UDS_CLIENT)

/// @} UDS
