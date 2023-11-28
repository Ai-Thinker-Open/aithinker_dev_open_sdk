/**
 ****************************************************************************************
 *
 * @file hrps.c
 *
 * @brief Heart Rate Profile Sensor implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HRPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_HR_SENSOR)
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "gapc_task.h"
#include "hrps.h"
#include "hrps_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * HTPT PROFILE ATTRIBUTES
 ****************************************************************************************
 */

/// Full HRS Database Description - Used to add attributes into the database
const struct attm_desc hrps_att_db[HRS_IDX_NB] =
{
    // Heart Rate Service Declaration
    [HRS_IDX_SVC]                  =   {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Heart Rate Measurement Characteristic Declaration
    [HRS_IDX_HR_MEAS_CHAR]         =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Heart Rate Measurement Characteristic Value
    [HRS_IDX_HR_MEAS_VAL]          =   {ATT_CHAR_HEART_RATE_MEAS, PERM(NTF, ENABLE), PERM(RI, ENABLE), HRPS_HT_MEAS_MAX_LEN},
    // Heart Rate Measurement Characteristic - Client Characteristic Configuration Descriptor
    [HRS_IDX_HR_MEAS_NTF_CFG]      =   {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // Body Sensor Location Characteristic Declaration
    [HRS_IDX_BODY_SENSOR_LOC_CHAR] =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Body Sensor Location Characteristic Value
    [HRS_IDX_BODY_SENSOR_LOC_VAL]  =   {ATT_CHAR_BODY_SENSOR_LOCATION, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},

    // Heart Rate Control Point Characteristic Declaration
    [HRS_IDX_HR_CTNL_PT_CHAR]      =   {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Heart Rate Control Point Characteristic Value
    [HRS_IDX_HR_CTNL_PT_VAL]       =   {ATT_CHAR_HEART_RATE_CNTL_POINT, PERM(WRITE_REQ, ENABLE), PERM(RI, ENABLE), sizeof(uint8_t)},
};

/**
 ****************************************************************************************
 * @brief Initialization of the HRPS module.
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
static uint8_t hrps_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct hrps_db_cfg *p_params)
{
    //------------------ create the attribute database for the profile -------------------
    // Service Configuration Flag
    uint8_t cfg_flag = HRPS_MANDATORY_MASK;
    // Database Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    // Set Configuration Flag Value
    if (GETB(p_params->features, HRPS_BODY_SENSOR_LOC_CHAR_SUP))
    {
        cfg_flag |= HRPS_BODY_SENSOR_LOC_MASK;
    }
    if (GETB(p_params->features, HRPS_ENGY_EXP_FEAT_SUP))
    {
        cfg_flag |= HRPS_HR_CTNL_PT_MASK;
    }

    // Add Service Into Database
    status = attm_svc_create_db(p_start_hdl, ATT_SVC_HEART_RATE, (uint8_t *)&cfg_flag,
            HRS_IDX_NB, NULL, p_env->task, &hrps_att_db[0],
            (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS))| PERM(SVC_MI, DISABLE));

    //-------------------- allocate memory required for the profile  ---------------------
    if (status == ATT_ERR_NO_ERROR)
    {
        // Allocate HRPS required environment variable
        struct hrps_env_tag *p_hrps_env =
                (struct hrps_env_tag *) ke_malloc(sizeof(struct hrps_env_tag), KE_MEM_ATT_DB);

        // Initialize HRPS environment
        p_env->env           = (prf_env_t*) p_hrps_env;
        p_hrps_env->shdl     = *p_start_hdl;
        p_hrps_env->features = p_params->features;
        p_hrps_env->sensor_location = p_params->body_sensor_loc;
        p_hrps_env->p_operation = NULL;

        p_hrps_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Mono Instantiated task
        p_hrps_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id = TASK_ID_HRPS;
        hrps_task_init(&(p_env->desc));

        // Put HRS in Idle state
        ke_state_set(p_env->task, HRPS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the HRPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void hrps_destroy(struct prf_task_env *p_env)
{
    struct hrps_env_tag *p_hrps_env = (struct hrps_env_tag *) p_env->env;

    // free profile environment variables
    if (p_hrps_env->p_operation != NULL)
    {
        ke_free(p_hrps_env->p_operation);
    }

    p_env->env = NULL;
    ke_free(p_hrps_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void hrps_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Nothing to do
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
static void hrps_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    // Nothing to do
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// HRPS Task interface required by profile manager
const struct prf_task_cbs hrps_itf =
{
    (prf_init_fnct) hrps_init,
    hrps_destroy,
    hrps_create,
    hrps_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *hrps_prf_itf_get(void)
{
    return &hrps_itf;
}

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void hrps_send_cmp_evt(struct hrps_env_tag *p_hrps_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct hrps_cmp_evt *p_cfm;

    ke_task_id_t src_id = prf_src_task_get(&p_hrps_env->prf_env, conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&p_hrps_env->prf_env, conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == HRPS_BUSY)
    {
        ke_state_set(src_id, HRPS_IDLE);
    }

    // Send CFM to APP that value has been sent or not
    p_cfm = KE_MSG_ALLOC(HRPS_CMP_EVT, dest_id, src_id, hrps_cmp_evt);
    p_cfm->operation = operation;
    p_cfm->status = status;

    ke_msg_send(p_cfm);
}

uint8_t hrps_pack_meas_value(uint8_t *p_packed_hr, const struct hrs_hr_meas *p_meas_val)
{
    uint8_t cursor = 0;
    uint8_t i = 0;

    // Heart Rate measurement flags
    *(p_packed_hr) = p_meas_val->flags;

    if (GETB(p_meas_val->flags, HRS_FLAG_HR_VALUE_FORMAT))
    {
        // Heart Rate Measurement Value 16 bits
        co_write16p(p_packed_hr + 1, p_meas_val->heart_rate);
        cursor += 3;
    }
    else
    {
        // Heart Rate Measurement Value 8 bits
        *(p_packed_hr + 1) = p_meas_val->heart_rate;
        cursor += 2;
    }

    if (GETB(p_meas_val->flags, HRS_FLAG_ENERGY_EXPENDED_PRESENT))
    {
        // Energy Expended present
        co_write16p(p_packed_hr + cursor, p_meas_val->energy_expended);
        cursor += 2;
    }

    if (GETB(p_meas_val->flags, HRS_FLAG_RR_INTERVAL_PRESENT))
    {
        for (i = 0; (i < (p_meas_val->nb_rr_interval)) && (i < (HRS_MAX_RR_INTERVAL)); i++)
        {
            // RR-Intervals
            co_write16p(p_packed_hr + cursor, p_meas_val->rr_intervals[i]);
            cursor += 2;
        }
    }

    // Clear unused packet data
    if (cursor < HRPS_HT_MEAS_MAX_LEN)
    {
        memset(p_packed_hr + cursor, 0, HRPS_HT_MEAS_MAX_LEN - cursor);
    }

    return cursor;
}

void hrps_exe_operation(uint8_t conidx)
{
    struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);

    ASSERT_ERR(p_hrps_env->p_operation != NULL);

    bool finished = true;

    while (p_hrps_env->p_operation->cursor < BLE_CONNECTION_MAX)
    {
        // Check if notifications are enabled
        if ((p_hrps_env->hr_meas_ntf[p_hrps_env->p_operation->cursor] & ~HRP_PRF_CFG_PERFORMED_OK) == PRF_CLI_START_NTF)
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, p_hrps_env->p_operation->cursor),
                    prf_src_task_get(&(p_hrps_env->prf_env), 0),
                    gattc_send_evt_cmd, p_hrps_env->p_operation->length);

            // Fill in the parameter structure
            p_meas_val->operation = GATTC_NOTIFY;
            p_meas_val->handle = p_hrps_env->shdl + HRS_IDX_HR_MEAS_VAL;
            p_meas_val->length = p_hrps_env->p_operation->length;
            memcpy(p_meas_val->value, p_hrps_env->p_operation->data, p_hrps_env->p_operation->length);

            // Send the event
            ke_msg_send(p_meas_val);

            finished = false;
            p_hrps_env->p_operation->cursor++;
            break;
        }

        p_hrps_env->p_operation->cursor++;
    }

    // check if operation is finished
    if (finished)
    {
        // Send confirmation
        hrps_send_cmp_evt(p_hrps_env, conidx, HRPS_MEAS_SEND_CMD_OP_CODE, GAP_ERR_NO_ERROR);

        // free operation
        ke_free(p_hrps_env->p_operation);
        p_hrps_env->p_operation = NULL;
        // go back to idle state
        ke_state_set(prf_src_task_get(&(p_hrps_env->prf_env), 0), HRPS_IDLE);
    }
}

#endif /* BLE_HR_SENSOR */

/// @} HRPS
