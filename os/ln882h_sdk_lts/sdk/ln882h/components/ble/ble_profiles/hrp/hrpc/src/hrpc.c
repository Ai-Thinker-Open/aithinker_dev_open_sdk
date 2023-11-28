/**
 ****************************************************************************************
 *
 * @file hrpc.c
 *
 * @brief Heart Rate Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HRPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_HR_COLLECTOR)

#include "hrp_common.h"
#include "hrpc.h"
#include "hrpc_task.h"

#include "ke_mem.h"
#include "co_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the HRPC module.
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
static uint8_t hrpc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl,
        uint16_t app_task, uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct hrpc_env_tag *p_hrpc_env =
            (struct hrpc_env_tag *) ke_malloc(sizeof(struct hrpc_env_tag), KE_MEM_ATT_DB);

    // allocate HRPC required environment variable
    p_env->env = (prf_env_t *) p_hrpc_env;

    p_hrpc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_hrpc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_HRPC;
    hrpc_task_init(&(p_env->desc));

    for (idx = 0; idx < HRPC_IDX_MAX; idx++)
    {
        p_hrpc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), HRPC_FREE);
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
static void hrpc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct hrpc_env_tag *p_hrpc_env = (struct hrpc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_hrpc_env->p_env[conidx] != NULL)
    {
        ke_free(p_hrpc_env->p_env[conidx]);
        p_hrpc_env->p_env[conidx] = NULL;
    }

    // Put HRP Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), HRPC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the HRPC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void hrpc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct hrpc_env_tag *p_hrpc_env = (struct hrpc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < HRPC_IDX_MAX; idx++)
    {
        hrpc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_hrpc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void hrpc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put HRP Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), HRPC_IDLE);
}

/// HRPC Task interface required by profile manager
const struct prf_task_cbs hrpc_itf =
{
    hrpc_init,
    hrpc_destroy,
    hrpc_create,
    hrpc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *hrpc_prf_itf_get(void)
{
    return &hrpc_itf;
}

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void hrpc_enable_rsp_send(struct hrpc_env_tag *p_hrpc_env, uint8_t conidx, uint8_t status)
{
    //send APP the details of the discovered attributes on HTPT
    struct hrpc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            HRPC_ENABLE_RSP,
            prf_dst_task_get(&(p_hrpc_env->prf_env), conidx),
            prf_src_task_get(&(p_hrpc_env->prf_env), conidx),
            hrpc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->hrs = p_hrpc_env->p_env[conidx]->hrs;
        //register HRPC task in gatt for indication/notifications
        prf_register_atthdl2gatt(&p_hrpc_env->prf_env, conidx, &p_hrpc_env->p_env[conidx]->hrs.svc);
        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_hrpc_env->prf_env), conidx), HRPC_IDLE);
    }

    ke_msg_send(p_rsp);
}

void hrpc_unpack_meas_value(struct hrs_hr_meas *p_meas_val, uint8_t *p_packed_hr, uint8_t size)
{
    int8_t cursor = 0;
    int8_t i = 0;

    // Heart Rate measurement flags
    p_meas_val->flags = p_packed_hr[0];
    cursor += 1;

    if (GETB(p_meas_val->flags, HRS_FLAG_HR_VALUE_FORMAT))
    {
        // Heart Rate Measurement Value 16 bits
        p_meas_val->heart_rate = co_read16p(&p_packed_hr[cursor]);
        cursor += 2;
    }
    else
    {
        // Heart Rate Measurement Value 8 bits
        p_meas_val->heart_rate = (uint16_t) p_packed_hr[cursor];
        cursor += 1;
    }

    if (GETB(p_meas_val->flags, HRS_FLAG_ENERGY_EXPENDED_PRESENT))
    {
        // Energy Expended present
        p_meas_val->energy_expended = co_read16p(&p_packed_hr[cursor]);
        cursor += 2;
    }

    // retrieve number of rr intervals
    p_meas_val->nb_rr_interval = ((size - cursor) > 0 ? ((size - cursor)/ 2) : (0));

    for (i = 0; (i < (p_meas_val->nb_rr_interval)) && (i < (HRS_MAX_RR_INTERVAL)); i++)
    {
        // RR-Intervals
        p_meas_val->rr_intervals[i] = co_read16p(&p_packed_hr[cursor]);
        cursor += 2;
    }
}

void hrpc_send_cmp_evt(struct hrpc_env_tag *p_hrpc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct hrpc_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_hrpc_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_hrpc_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == HRPC_BUSY)
    {
        ke_state_set(src_id, HRPC_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(HRPC_CMP_EVT, dest_id, src_id, hrpc_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif /* (BLE_HR_COLLECTOR) */

/// @} HRPC
