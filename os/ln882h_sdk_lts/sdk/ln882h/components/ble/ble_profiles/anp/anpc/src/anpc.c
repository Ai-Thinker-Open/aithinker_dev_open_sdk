/**
 ****************************************************************************************
 *
 * @file anpc.c
 *
 * @brief Alert Notification Profile Client implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup ANPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_AN_CLIENT)

#include "anp_common.h"
#include "anpc.h"
#include "anpc_task.h"

#include "ke_mem.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the ANPC module.
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
static uint8_t anpc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, void* p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct anpc_env_tag* p_anpc_env =
            (struct anpc_env_tag* ) ke_malloc(sizeof(struct anpc_env_tag), KE_MEM_ATT_DB);

    // allocate ANPC required environment variable
    p_env->env = (prf_env_t*) p_anpc_env;

    p_anpc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_anpc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_ANPC;
    anpc_task_init(&(p_env->desc));

    for (idx = 0; idx < ANPC_IDX_MAX ; idx++)
    {
        p_anpc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), ANPC_FREE);
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
static void anpc_cleanup(struct prf_task_env* p_env, uint8_t conidx, uint8_t reason)
{
    struct anpc_env_tag* p_anpc_env = (struct anpc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_anpc_env->p_env[conidx] != NULL)
    {
        ke_free(p_anpc_env->p_env[conidx]);
        p_anpc_env->p_env[conidx] = NULL;
    }

    // Put ANP Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), ANPC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the ANPC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void anpc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct anpc_env_tag* p_anpc_env = (struct anpc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < ANPC_IDX_MAX; idx++)
    {
        anpc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_anpc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void anpc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put ANP Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), ANPC_IDLE);
}

/// ANPC Task interface required by profile manager
const struct prf_task_cbs anpc_itf =
{
    anpc_init,
    anpc_destroy,
    anpc_create,
    anpc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* anpc_prf_itf_get(void)
{
    return &anpc_itf;
}

void anpc_enable_rsp_send(struct anpc_env_tag *p_anpc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on ANPS
    struct anpc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            ANPC_ENABLE_RSP,
            prf_dst_task_get(&(p_anpc_env->prf_env), conidx),
            prf_src_task_get(&(p_anpc_env->prf_env), conidx),
            anpc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->ans = p_anpc_env->p_env[conidx]->ans;
        // Register ANPC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&(p_anpc_env->prf_env), conidx, &(p_anpc_env->p_env[conidx]->ans.svc));
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_anpc_env->prf_env), conidx), ANPC_IDLE);
    }

    ke_msg_send(p_rsp);
}

bool anpc_found_next_alert_cat(struct anpc_env_tag *p_anpc_env, uint8_t conidx, struct anp_cat_id_bit_mask cat_id)
{
    // Next Category Found ?
    bool found = false;

    if (p_anpc_env->p_env[conidx]->last_uuid_req < CAT_ID_HIGH_PRTY_ALERT)
    {
        // Look in the first part of the categories
        while ((p_anpc_env->p_env[conidx]->last_uuid_req < CAT_ID_HIGH_PRTY_ALERT) && (!found))
        {
            if (((cat_id.cat_id_mask_0 >> p_anpc_env->p_env[conidx]->last_uuid_req) & 1) == 1)
            {
                found = true;
            }

            p_anpc_env->p_env[conidx]->last_uuid_req++;
        }
    }

    if (p_anpc_env->p_env[conidx]->last_uuid_req >= CAT_ID_HIGH_PRTY_ALERT)
    {
        // Look in the first part of the categories
        while ((p_anpc_env->p_env[conidx]->last_uuid_req < CAT_ID_NB) && (!found))
        {
            if (((cat_id.cat_id_mask_1 >>
                    (p_anpc_env->p_env[conidx]->last_uuid_req - CAT_ID_HIGH_PRTY_ALERT)) & 1) == 1)
            {
                found = true;
            }

            p_anpc_env->p_env[conidx]->last_uuid_req++;
        }
    }

    return found;
}

void anpc_write_alert_ntf_ctnl_pt(struct anpc_env_tag *p_anpc_env, uint8_t conidx, uint8_t cmd_id, uint8_t cat_id)
{
    struct anp_ctnl_pt ctnl_pt = {cmd_id, cat_id};

    // Send the write request
    prf_gatt_write(&(p_anpc_env->prf_env), conidx,
                   p_anpc_env->p_env[conidx]->ans.chars[ANPC_CHAR_ALERT_NTF_CTNL_PT].val_hdl,
                   (uint8_t *)&ctnl_pt, sizeof(struct anp_ctnl_pt), GATTC_WRITE);
}

void anpc_send_cmp_evt(struct anpc_env_tag *p_anpc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct anpc_cmp_evt *p_evt;

    if (p_anpc_env->p_env[conidx] != NULL)
    {
        // Free the stored operation if needed
        if (p_anpc_env->p_env[conidx]->p_operation != NULL)
        {
            ke_msg_free(ke_param2msg(p_anpc_env->p_env[conidx]->p_operation));
            p_anpc_env->p_env[conidx]->p_operation = NULL;
        }
    }

    // Go back to the CONNECTED state if the state is busy
    if (ke_state_get(prf_src_task_get(&(p_anpc_env->prf_env), conidx)) == ANPC_BUSY)
    {
        ke_state_set(prf_src_task_get(&p_anpc_env->prf_env, conidx), ANPC_IDLE);
    }

    // Send the message
    p_evt = KE_MSG_ALLOC(ANPC_CMP_EVT,
            prf_dst_task_get(&(p_anpc_env->prf_env), conidx),
            prf_src_task_get(&(p_anpc_env->prf_env), conidx),
            anpc_cmp_evt);

    p_evt->operation  = operation;
    p_evt->status     = status;

    ke_msg_send(p_evt);
}

#endif //(BLE_AN_CLIENT)

/// @} ANPC
