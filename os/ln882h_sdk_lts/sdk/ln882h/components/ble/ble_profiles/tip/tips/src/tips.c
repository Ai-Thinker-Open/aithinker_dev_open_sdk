/**
 ****************************************************************************************
 *
 * @file tips.c
 *
 * @brief Time Profile Server implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TIPS
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_TIP_SERVER)
#include "attm.h"
#include "tips.h"
#include "tips_task.h"
#include "prf_utils.h"
#include "co_utils.h"


#include "ke_mem.h"

/*
 * CTS, NDCS, RTUS ATTRIBUTES
 ****************************************************************************************
 */
/// Full CTS Database Description - Used to add attributes into the database
const struct attm_desc cts_att_db[CTS_IDX_NB] =
{
    // Current Time Service Declaration
    [CTS_IDX_SVC]                  = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Current Time Characteristic Declaration
    [CTS_IDX_CURRENT_TIME_CHAR]    = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Current Time Characteristic Value
    [CTS_IDX_CURRENT_TIME_VAL]     = {ATT_CHAR_CT_TIME, PERM(RD, ENABLE) | PERM(NTF, ENABLE),  PERM(RI, ENABLE), CTS_CURRENT_TIME_VAL_LEN},
    // Current Time Characteristic - Client Char. Configuration Descriptor
    [CTS_IDX_CURRENT_TIME_CFG]     = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE) | PERM(WRITE_REQ, ENABLE), 0, 0},

    // Local Time Information Characteristic Declaration
    [CTS_IDX_LOCAL_TIME_INFO_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Local Time Information Characteristic Value
    [CTS_IDX_LOCAL_TIME_INFO_VAL]  = {ATT_CHAR_LOCAL_TIME_INFO, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(struct tip_loc_time_info)},

    // Reference Time Information Characteristic Declaration
    [CTS_IDX_REF_TIME_INFO_CHAR]   = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Reference Time Info Characteristic Value
    [CTS_IDX_REF_TIME_INFO_VAL]    = {ATT_CHAR_REFERENCE_TIME_INFO, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(struct tip_ref_time_info)},
};

/// Full NDCS Database Description - Used to add attributes into the database
const struct attm_desc ndcs_att_db[NDCS_IDX_NB] =
{
    // Next DST Change Service Declaration
    [NDCS_IDX_SVC]                 = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Time with DST Characteristic Declaration
    [NDCS_IDX_TIME_DST_CHAR]       = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Time With DST Characteristic Value
    [NDCS_IDX_TIME_DST_VAL]        = {ATT_CHAR_TIME_WITH_DST, PERM(RD, ENABLE), PERM(RI, ENABLE), NDCS_TIME_DST_VAL_LEN},
};

/// Full RTUS Database Description - Used to add attributes into the database
const struct attm_desc rtus_att_db[RTUS_IDX_NB] =
{
    // Reference Time Information Service Declaration
    [RTUS_IDX_SVC]                   = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Time Update Control Point Characteristic Declaration
    [RTUS_IDX_TIME_UPD_CTNL_PT_CHAR] = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Time Update Control Point Characteristic Value
    [RTUS_IDX_TIME_UPD_CTNL_PT_VAL]  = {ATT_CHAR_TIME_UPDATE_CNTL_POINT, PERM(WRITE_COMMAND, ENABLE), PERM(RI, ENABLE), sizeof(tip_time_upd_contr_pt)},

    // Time Update State Characteristic Declaration
    [RTUS_IDX_TIME_UPD_STATE_CHAR]   = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Time Update State Characteristic Value
    [RTUS_IDX_TIME_UPD_STATE_VAL]    = {ATT_CHAR_TIME_UPDATE_STATE, PERM(RD, ENABLE), PERM(RI, ENABLE), sizeof(struct tip_time_upd_state)},
};


/**
 ****************************************************************************************
 * @brief Initialization of the TIPS module.
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
static uint8_t tips_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
        uint8_t sec_lvl, struct tips_db_cfg *p_params)
{
    // Service content flag
    uint32_t cfg_flag= TIPS_CTS_CURRENT_TIME_MASK;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;
    // Total number of attributes
    uint8_t tot_nb_att = CTS_IDX_NB;

    // Allocate TIPS required environment variable
    struct tips_env_tag *p_tips_env =
            (struct tips_env_tag *) ke_malloc(sizeof(struct tips_env_tag), KE_MEM_ATT_DB);

    p_env->env = (prf_env_t *) p_tips_env;

    //------------------ create the attribute database for the profile -------------------

    // Check supported attributes
    if (GETB(p_params->features, TIPS_NDCS_SUP))
    {
        tot_nb_att += NDCS_IDX_NB;
    }

    if (GETB(p_params->features, TIPS_RTUS_SUP))
    {
        tot_nb_att += RTUS_IDX_NB;
    }

    // Check that attribute list can be allocated.
    status = attm_reserve_handle_range(p_start_hdl, tot_nb_att);

    if (status == ATT_ERR_NO_ERROR)
    {
        // Set Configuration Flag Value
        if (GETB(p_params->features, TIPS_CTS_LOC_TIME_INFO_SUP))
        {
            cfg_flag |= TIPS_CTS_LOC_TIME_INFO_MASK;
        }

        if (GETB(p_params->features, TIPS_CTS_REF_TIME_INFO_SUP))
        {
            cfg_flag |= TIPS_CTS_REF_TIME_INFO_MASK;
        }

        /*---------------------------------------------------*
         * Current Time Service Creation
         *---------------------------------------------------*/
        status = attm_svc_create_db(p_start_hdl, ATT_SVC_CURRENT_TIME, (uint8_t *)&cfg_flag,
                CTS_IDX_NB, &p_tips_env->cts_att_tbl[0], p_env->task, &cts_att_db[0],
                (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, ENABLE));

        // Update index
        p_tips_env->cts_shdl = *p_start_hdl;
        *p_start_hdl += CTS_IDX_NB;

        // All attributes are mandatory for NDCS and RTUS
        cfg_flag = 0xFF;

        if ((status == ATT_ERR_NO_ERROR) && (GETB(p_params->features, TIPS_NDCS_SUP)))
        {
            /*---------------------------------------------------*
             * Next DST Change Service Creation
             *---------------------------------------------------*/
            status = attm_svc_create_db(p_start_hdl, ATT_SVC_NEXT_DST_CHANGE, (uint8_t *)&cfg_flag,
                    NDCS_IDX_NB, &p_tips_env->ndcs_att_tbl[0], p_env->task, &ndcs_att_db[0],
                    (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, ENABLE));

            // Update index
            p_tips_env->ndcs_shdl = *p_start_hdl;
            *p_start_hdl += NDCS_IDX_NB;
        }

        /*---------------------------------------------------*
         * Reference Time Update Service Creation
         *---------------------------------------------------*/
        if ((status == ATT_ERR_NO_ERROR) && (GETB(p_params->features, TIPS_RTUS_SUP)))
        {
            status = attm_svc_create_db(p_start_hdl, ATT_SVC_REF_TIME_UPDATE, (uint8_t *)&cfg_flag,
                    RTUS_IDX_NB, &p_tips_env->rtus_att_tbl[0], p_env->task, &rtus_att_db[0],
                    (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, ENABLE));

            // Update index
            p_tips_env->rtus_shdl = *p_start_hdl;
            *p_start_hdl += RTUS_IDX_NB;
        }

        // Initialize TIPS environment
        p_tips_env->features = p_params->features;

        p_tips_env->prf_env.app_task    = app_task
                | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        // Multi Instantiated task
        p_tips_env->prf_env.prf_task    = p_env->task | PERM(PRF_MI, ENABLE);

        // initialize environment variable
        p_env->id                     = TASK_ID_TIPS;
        tips_task_init(&(p_env->desc));

        for (uint8_t idx = 0; idx < BLE_CONNECTION_MAX; idx++)
        {
            p_tips_env->p_env[idx] = NULL;
        }

        // Put TIS in disabled state
        ke_state_set(p_env->task, TIPS_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the TIPS module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void tips_destroy(struct prf_task_env *p_env)
{
    uint8_t idx;
    struct tips_env_tag* p_tips_env = (struct tips_env_tag *) p_env->env;

    // cleanup environment variable for each task instances
    for (idx = 0; idx < BLE_CONNECTION_MAX; idx++)
    {
        if (p_tips_env->p_env[idx] != NULL)
        {
            ke_free(p_tips_env->p_env[idx]);
        }
    }

    // free profile environment variables
    p_env->env = NULL;
    ke_free(p_tips_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void tips_create(struct prf_task_env *p_env, uint8_t conidx)
{
    struct tips_env_tag *p_tips_env = (struct tips_env_tag *) p_env->env;

    p_tips_env->p_env[conidx] = (struct tips_cnx_env *)
            ke_malloc(sizeof(struct tips_cnx_env), KE_MEM_ATT_DB);

    memset(p_tips_env->p_env[conidx], 0, sizeof(struct tips_cnx_env));

    // Put TIS in idle state
    ke_state_set(KE_BUILD_ID(p_env->task, conidx), TIPS_IDLE);
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
static void tips_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    struct tips_env_tag *p_tips_env = (struct tips_env_tag *) p_env->env;

    // clean-up environment variable allocated for task instance
    if (p_tips_env->p_env[conidx] != NULL)
    {
        ke_free(p_tips_env->p_env[conidx]);
        p_tips_env->p_env[conidx] = NULL;
    }
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// TIPS Task interface required by profile manager
const struct prf_task_cbs tips_itf =
{
    (prf_init_fnct) tips_init,
    tips_destroy,
    tips_create,
    tips_cleanup,
};

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

const struct prf_task_cbs *tips_prf_itf_get(void)
{
    return &tips_itf;
}

uint8_t tips_pack_curr_time_value(uint8_t *p_pckd_time, const struct tip_curr_time *p_curr_time_val)
{
    // Date-Time
    prf_pack_date_time(p_pckd_time, &(p_curr_time_val->date_time));

    //Day of Week
    *(p_pckd_time + 7) = p_curr_time_val->day_of_week;

    //Fraction 256
    *(p_pckd_time + 8) = p_curr_time_val->fraction_256;

    //Adjust Reason
    *(p_pckd_time + 9) = p_curr_time_val->adjust_reason;

    return 10;
}

uint8_t tips_pack_time_dst_value(uint8_t *p_pckd_time_dst, const struct tip_time_with_dst *p_time_dst_val)
{
    // Date-Time
    prf_pack_date_time(p_pckd_time_dst, &(p_time_dst_val->date_time));

    // DST Offset
    *(p_pckd_time_dst + 7) = p_time_dst_val->dst_offset;

    return 8;
}

void tips_send_cmp_evt(struct tips_env_tag *p_tips_env, uint8_t conidx, uint8_t operation, uint8_t status)
{
    struct tips_cmp_evt *p_evt;
    ke_task_id_t src_id = prf_src_task_get(&(p_tips_env->prf_env), conidx);
    ke_task_id_t dest_id = prf_dst_task_get(&(p_tips_env->prf_env), conidx);

    // Come back to the Connected state if the state was busy.
    if (ke_state_get(src_id) == TIPS_BUSY)
    {
        ke_state_set(src_id, TIPS_IDLE);
    }

    // Send the message to the application
    p_evt = KE_MSG_ALLOC(TIPS_CMP_EVT, dest_id, src_id, tips_cmp_evt);

    p_evt->operation   = operation;
    p_evt->status      = status;

    ke_msg_send(p_evt);
}

#endif //BLE_TIP_SERVER

/// @} TIPS
