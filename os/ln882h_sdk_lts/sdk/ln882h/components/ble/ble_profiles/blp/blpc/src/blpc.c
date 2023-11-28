/**
 ****************************************************************************************
 *
 * @file blpc.c
 *
 * @brief Blood Pressure Profile Collector implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup BLPC
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_BP_COLLECTOR)
#include "blpc.h"
#include "blpc_task.h"
#include "gap.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the BLPC module.
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
static uint8_t blpc_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl,  void *p_params)
{
    uint8_t idx;
    //-------------------- allocate memory required for the profile  ---------------------

    struct blpc_env_tag *p_blpc_env =
            (struct blpc_env_tag *) ke_malloc(sizeof(struct blpc_env_tag), KE_MEM_ATT_DB);

    // allocate BLPC required environment variable
    p_env->env = (prf_env_t *) p_blpc_env;

    p_blpc_env->prf_env.app_task = app_task
            | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
    p_blpc_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, ENABLE);

    // initialize environment variable
    p_env->id = TASK_ID_BLPC;
    blpc_task_init(&(p_env->desc));

    for (idx = 0; idx < BLPC_IDX_MAX; idx++)
    {
        p_blpc_env->p_env[idx] = NULL;
        // service is ready, go into an Idle state
        ke_state_set(KE_BUILD_ID(p_env->task, idx), BLPC_FREE);
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
static void blpc_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct blpc_env_tag *p_blpc_env = (struct blpc_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_blpc_env->p_env[conidx] != NULL)
    {
        ke_free(p_blpc_env->p_env[conidx]);
        p_blpc_env->p_env[conidx] = NULL;
    }

    // Put BLP Client in Free state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BLPC_FREE);
}

/**
 ****************************************************************************************
 * @brief Destruction of the BLPC module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void blpc_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct blpc_env_tag *p_blpc_env = (struct blpc_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < BLPC_IDX_MAX; idx++)
    {
        blpc_cleanup(p_env, idx, 0);
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_blpc_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void blpc_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Put BLP Client in Idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), BLPC_IDLE);
}

/// BLPC Task interface required by profile manager
const struct prf_task_cbs blpc_itf =
{
    blpc_init,
    blpc_destroy,
    blpc_create,
    blpc_cleanup,
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *blpc_prf_itf_get(void)
{
    return &blpc_itf;
}

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void blpc_enable_rsp_send(struct blpc_env_tag *p_blpc_env, uint8_t conidx, uint8_t status)
{
    // Send to APP the details of the discovered attributes on BLPS
    struct blpc_enable_rsp *p_rsp = KE_MSG_ALLOC(
            BLPC_ENABLE_RSP,
            prf_dst_task_get(&(p_blpc_env->prf_env), conidx),
            prf_src_task_get(&(p_blpc_env->prf_env), conidx),
            blpc_enable_rsp);

    p_rsp->status = status;

    if (status == GAP_ERR_NO_ERROR)
    {
        p_rsp->bps = p_blpc_env->p_env[conidx]->bps;

        prf_register_atthdl2gatt(&(p_blpc_env->prf_env), conidx, &(p_blpc_env->p_env[conidx]->bps.svc));

        // Go to connected state
        ke_state_set(prf_src_task_get(&(p_blpc_env->prf_env), conidx), BLPC_IDLE);
    }

    ke_msg_send(p_rsp);
}

void blpc_unpack_meas_value(struct bps_bp_meas *p_meas_val, uint8_t *p_packed_bp)
{
    uint8_t cursor;

    // blood pressure measurement flags
    p_meas_val->flags = p_packed_bp[0];

    // Blood Pressure Measurement Compound Value - Systolic
    p_meas_val->systolic = co_read16p(&(p_packed_bp[1]));

    // Blood Pressure Measurement Compound Value - Diastolic (mmHg)
    p_meas_val->diastolic = co_read16p(&(p_packed_bp[3]));

    //  Blood Pressure Measurement Compound Value - Mean Arterial Pressure (mmHg)
    p_meas_val->mean_arterial_pressure = co_read16p(&(p_packed_bp[5]));

    cursor = 7;

    // time flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_FLAG_TIME_STAMP))
    {
        cursor += prf_unpack_date_time(p_packed_bp + cursor, &(p_meas_val->time_stamp));
    }

    // pulse rate flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_PULSE_RATE))
    {
        p_meas_val->pulse_rate = co_read16p(&(p_packed_bp[cursor + 0]));
        cursor += 2;
    }

    // User ID flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_USER_ID))
    {
        p_meas_val->user_id = p_packed_bp[cursor + 0];
        cursor += 1;
    }

    // measurement status flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_MEAS_STATUS))
    {
        p_meas_val->meas_status = co_read16p(&(p_packed_bp[cursor + 0]));
        cursor += 2;
    }
}

void blpc_send_cmp_evt(struct blpc_env_tag *p_blpc_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct blpc_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_blpc_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_blpc_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == BLPC_BUSY)
    {
        ke_state_set(src_id, BLPC_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(BLPC_CMP_EVT, dest_id, src_id, blpc_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif /* (BLE_BP_COLLECTOR) */

/// @} BLPC
