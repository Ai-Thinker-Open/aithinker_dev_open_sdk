/**
 ****************************************************************************************
 *
 * @file blps.c
 *
 * @brief Blood Pressure Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup BLPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BP_SENSOR)
#include "gap.h"
#include "gattc_task.h"
#include "blps.h"
#include "blps_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * BLPS PROFILE ATTRIBUTES
 ****************************************************************************************
 */

/// Full BLPS Database Description - Used to add attributes into the database
const struct attm_desc blps_att_db[BPS_IDX_NB] =
{
    // Blood Pressure Service Declaration
    [BPS_IDX_SVC] = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Blood Pressure Measurement Characteristic Declaration
    [BPS_IDX_BP_MEAS_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Blood Pressure Measurement Characteristic Value
    [BPS_IDX_BP_MEAS_VAL] = {ATT_CHAR_BLOOD_PRESSURE_MEAS, PERM(IND, ENABLE), PERM(RI, ENABLE), BLPS_BP_MEAS_MAX_LEN},
    // Blood Pressure Measurement Characteristic - Client Characteristic Configuration Descriptor
    [BPS_IDX_BP_MEAS_IND_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // Blood Pressure Feature Characteristic Declaration
    [BPS_IDX_BP_FEATURE_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Blood Pressure Feature Characteristic Value
    [BPS_IDX_BP_FEATURE_VAL] = {ATT_CHAR_BLOOD_PRESSURE_FEATURE, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint16_t)},

    // Intermediate Cuff Pressure Characteristic Declaration
    [BPS_IDX_INTM_CUFF_PRESS_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Intermediate Cuff Pressure Characteristic Value
    [BPS_IDX_INTM_CUFF_PRESS_VAL] =
                    {ATT_CHAR_INTERMEDIATE_CUFF_PRESSURE, PERM(NTF, ENABLE), PERM(RI, ENABLE), BLPS_BP_MEAS_MAX_LEN},
    // Intermediate Cuff Pressure Characteristic - Client Characteristic Configuration Descriptor
    [BPS_IDX_INTM_CUFF_PRESS_NTF_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the BLPS module.
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
static uint8_t blps_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct blps_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    // Service content flag
    uint32_t cfg_flag= BLPS_MANDATORY_MASK;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;


    //Set Configuration Flag Value
    if (GETB(p_params->prfl_cfg, BLPS_INTM_CUFF_PRESS_SUP))
    {
        cfg_flag |= BLPS_INTM_CUFF_PRESS_MASK;
    }

    // Add service in the database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_BLOOD_PRESSURE, (uint8_t *)&cfg_flag,
            BPS_IDX_NB, NULL, p_env->task, &blps_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate BLPS required environment variable
        struct blps_env_tag *p_blps_env =
                (struct blps_env_tag *) ke_malloc(sizeof(struct blps_env_tag), KE_MEM_ATT_DB);

        // Initialize BLPS environment
        p_env->env           = (prf_env_t *) p_blps_env;
        p_blps_env->shdl     = *p_start_hdl;
        p_blps_env->features = p_params->features;
        p_blps_env->prfl_cfg = p_params->prfl_cfg;

        p_blps_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Multi Instantiated task
        p_blps_env->prf_env.prf_task    = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id = TASK_ID_BLPS;
        blps_task_init(&(p_env->desc));

        memset(p_blps_env->prfl_ntf_ind_cfg, 0, BLE_CONNECTION_MAX);

        // Put BLS in disabled state
        ke_state_set(p_env->task, BLPS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the BLPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void blps_destroy(struct prf_task_env *p_env)
{
    struct blps_env_tag *p_blps_env = (struct blps_env_tag *) p_env->env;

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_blps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void blps_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct blps_env_tag *p_blps_env = (struct blps_env_tag *) p_env->env;
    p_blps_env->prfl_ntf_ind_cfg[conidx] = 0;
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
static void blps_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct blps_env_tag *p_blps_env = (struct blps_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    p_blps_env->prfl_ntf_ind_cfg[conidx] = 0;
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// BLPS Task interface required by profile manager
const struct prf_task_cbs blps_itf =
{
    (prf_init_fnct) blps_init,
    blps_destroy,
    blps_create,
    blps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs* blps_prf_itf_get(void)
{
    return &blps_itf;
}

uint8_t blps_pack_meas_value(uint8_t *p_packed_bp, const struct bps_bp_meas *p_meas_val)
{
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);

    uint8_t cursor;

    *(p_packed_bp) = p_meas_val->flags;

    // Blood Pressure Measurement Compound Value - Systolic
    co_write16p(p_packed_bp + 1, p_meas_val->systolic);

    // Blood Pressure Measurement Compound Value - Diastolic (mmHg)
    co_write16p(p_packed_bp + 3, p_meas_val->diastolic);

    //  Blood Pressure Measurement Compound Value - Mean Arterial Pressure (mmHg)
    co_write16p(p_packed_bp + 5, p_meas_val->mean_arterial_pressure);

    cursor = 7;

    // time flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_FLAG_TIME_STAMP))
    {
        cursor += prf_pack_date_time(p_packed_bp + cursor, &(p_meas_val->time_stamp));
    }

    // Pulse rate flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_PULSE_RATE))
    {
        co_write16p(p_packed_bp + cursor, p_meas_val->pulse_rate);
        cursor += 2;
    }

    // User ID flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_USER_ID))
    {
        *(p_packed_bp + cursor) = p_meas_val->user_id;
        cursor += 1;
    }

    // Measurement status flag set
    if (GETB(p_meas_val->flags, BPS_MEAS_MEAS_STATUS))
    {
        // To avoid modify the value pointed by p_meas_val
        uint16_t meas_status_temp = p_meas_val->meas_status;

        // If feature is not supported, the corresponding Measurement status bit(s) shall be set to 0
        if (!GETB(p_blps_env->features, BPS_F_BODY_MVMT_DETECT_SUP))
        {
            SETB(meas_status_temp, BPS_STATUS_MVMT_DETECT, 0);
        }

        if (!GETB(p_blps_env->features, BPS_F_CUFF_FIT_DETECT_SUP))
        {
            SETB(meas_status_temp, BPS_STATUS_CUFF_FIT_DETECT, 0);
        }

        if (!GETB(p_blps_env->features, BPS_F_IRREGULAR_PULSE_DETECT_SUP))
        {
            SETB(meas_status_temp, BPS_STATUS_IRREGULAR_PULSE_DETECT, 0);
        }

        if (!GETB(p_blps_env->features, BPS_F_PULSE_RATE_RANGE_DETECT_SUP))
        {
            SETB(meas_status_temp, BPS_STATUS_PR_RANGE_DETECT_LSB, 0);
            SETB(meas_status_temp, BPS_STATUS_PR_RANGE_DETECT_MSB, 0);
        }

        if (!GETB(p_blps_env->features, BPS_F_MEAS_POS_DETECT_SUP))
        {
            SETB(meas_status_temp, BPS_STATUS_MEAS_POS_DETECT, 0);
        }

        co_write16p(p_packed_bp + cursor, meas_status_temp);
        cursor += 2;
    }

    // clear unused packet data
    if (cursor < BLPS_BP_MEAS_MAX_LEN)
    {
        memset(p_packed_bp + cursor, 0, BLPS_BP_MEAS_MAX_LEN - cursor);
    }

    return cursor;
}

void blps_send_cmp_evt(struct blps_env_tag *p_blps_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct blps_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_blps_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_blps_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == BLPS_BUSY)
    {
        ke_state_set(src_id, BLPS_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(BLPS_CMP_EVT, dest_id, src_id, blps_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif /* BLE_BP_SENSOR */

/// @} BLPS
