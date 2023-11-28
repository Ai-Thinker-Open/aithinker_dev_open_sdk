/**
 ****************************************************************************************
 *
 * @file htpt.c
 *
 * @brief Health Thermometer Profile Thermometer implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HTPT
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_HT_THERMOM)
#include "attm.h"
#include "htpt.h"
#include "htp/htpt/api/htpt_task.h"
#include "co_utils.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * HTPT PROFILE ATTRIBUTES
 ****************************************************************************************
 */
/// Full HTS Database Description - Used to add attributes into the database
const struct attm_desc htpt_att_db[HTS_IDX_NB] =
{
    // Health Thermometer Service Declaration
    [HTS_IDX_SVC]                 = {ATT_DECL_PRIMARY_SERVICE, PERM(RD, ENABLE), 0, 0},

    // Temperature Measurement Characteristic Declaration
    [HTS_IDX_TEMP_MEAS_CHAR]      = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Temperature Measurement Characteristic Value
    [HTS_IDX_TEMP_MEAS_VAL]       = {ATT_CHAR_TEMPERATURE_MEAS, PERM(IND, ENABLE), PERM(RI, ENABLE), 0},
    // Temperature Measurement Characteristic - Client Characteristic Configuration Descriptor
    [HTS_IDX_TEMP_MEAS_IND_CFG]   = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // Temperature Type Characteristic Declaration
    [HTS_IDX_TEMP_TYPE_CHAR]      = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Temperature Type Characteristic Value
    [HTS_IDX_TEMP_TYPE_VAL]       = {ATT_CHAR_TEMPERATURE_TYPE, PERM(RD, ENABLE), PERM(RI, ENABLE), 0},

    // Intermediate Measurement Characteristic Declaration
    [HTS_IDX_INTERM_TEMP_CHAR]    = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Intermediate Measurement Characteristic Value
    [HTS_IDX_INTERM_TEMP_VAL]     = {ATT_CHAR_INTERMED_TEMPERATURE, PERM(NTF, ENABLE), PERM(RI, ENABLE), 0},
    // Intermediate Measurement Characteristic - Client Characteristic Configuration Descriptor
    [HTS_IDX_INTERM_TEMP_CFG]     = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},

    // Measurement Interval Characteristic Declaration
    [HTS_IDX_MEAS_INTV_CHAR]      = {ATT_DECL_CHARACTERISTIC, PERM(RD, ENABLE), 0, 0},
    // Measurement Interval Characteristic Value
    [HTS_IDX_MEAS_INTV_VAL]       = {ATT_CHAR_MEAS_INTERVAL, PERM(RD, ENABLE), PERM(RI, ENABLE), HTPT_MEAS_INTV_MAX_LEN},
    // Measurement Interval Characteristic - Client Characteristic Configuration Descriptor
    [HTS_IDX_MEAS_INTV_CFG]       = {ATT_DESC_CLIENT_CHAR_CFG, PERM(RD, ENABLE)|PERM(WRITE_REQ, ENABLE), 0, 0},
    // Measurement Interval Characteristic - Valid Range Descriptor
    [HTS_IDX_MEAS_INTV_VAL_RANGE] = {ATT_DESC_VALID_RANGE, PERM(RD, ENABLE), PERM(RI, ENABLE), 0},
};

static uint16_t htpt_compute_att_table(uint16_t features);

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialization of the HTPT module.
 * This function performs all the initializations of the Profile module.
 *  - Creation of database (if it's a service)
 *  - Allocation of profile required memory
 *  - Initialization of task descriptor to register application
 *      - Task State array
 *      - Number of tasks
 *      - Default task handler
 *
 * @param[out]    p_env        Collector or Service allocated environment data.
 * @param[in|out] p_start_hdl  Service start handle (0 - dynamically allocated), only applies for services.
 * @param[in]     app_task     Application task number.
 * @param[in]     sec_lvl      Security level (AUTH, EKS and MI field of @see enum attm_value_perm_mask)
 * @param[in]     p_params     Configuration parameters of profile collector or service (32 bits aligned)
 *
 * @return status code to know if profile initialization succeed or not.
 ****************************************************************************************
 */
static uint8_t htpt_init(struct prf_task_env *p_env, uint16_t *p_start_hdl, uint16_t app_task,
                            uint8_t sec_lvl, struct htpt_db_cfg *p_params)
{
    // Service content flag
    uint16_t cfg_flag;
    // DB Creation Status
    uint8_t status = ATT_ERR_NO_ERROR;

    cfg_flag = htpt_compute_att_table(p_params->features);

    status = attm_svc_create_db(p_start_hdl, ATT_SVC_HEALTH_THERMOM, (uint8_t *)&cfg_flag,
               HTS_IDX_NB, NULL, p_env->task, &htpt_att_db[0],
              (sec_lvl & (PERM_MASK_SVC_DIS | PERM_MASK_SVC_AUTH | PERM_MASK_SVC_EKS)) | PERM(SVC_MI, DISABLE) );

    if (status == ATT_ERR_NO_ERROR)
    {
        //-------------------- allocate memory required for the profile  ---------------------
        struct htpt_env_tag *p_htpt_env =
                (struct htpt_env_tag *) ke_malloc(sizeof(struct htpt_env_tag), KE_MEM_ATT_DB);

        // allocate PROXR required environment variable
        p_env->env = (prf_env_t *) p_htpt_env;

        p_htpt_env->shdl = *p_start_hdl;
        p_htpt_env->prf_env.app_task = app_task
                        | (PERM_GET(sec_lvl, SVC_MI) ? PERM(PRF_MI, ENABLE) : PERM(PRF_MI, DISABLE));
        p_htpt_env->prf_env.prf_task = p_env->task | PERM(PRF_MI, DISABLE);

        // initialize environment variable
        p_env->id = TASK_ID_HTPT;
        htpt_task_init(&(p_env->desc));

        //Save features on the environment
        p_htpt_env->features      = p_params->features;
        p_htpt_env->meas_intv     = p_params->meas_intv;
        p_htpt_env->meas_intv_min = p_params->valid_range_min;
        p_htpt_env->meas_intv_max = p_params->valid_range_max;
        p_htpt_env->temp_type     = p_params->temp_type;
        p_htpt_env->p_operation   = NULL;
        memset(p_htpt_env->ntf_ind_cfg, 0 , sizeof(p_htpt_env->ntf_ind_cfg));

        // Update measurement interval permissions
        if (GETB(p_params->features, HTPT_MEAS_INTV_CHAR_SUP))
        {
            uint16_t perm = PERM(RD, ENABLE);

            // Check if Measurement Interval Char. supports indications
            if (GETB(p_params->features, HTPT_MEAS_INTV_IND_SUP))
            {
                perm |= PERM(IND, ENABLE);
            }

            // Check if Measurement Interval Char. is writable
            if (GETB(p_params->features, HTPT_MEAS_INTV_WR_SUP))
            {
                perm |= PERM(WP, UNAUTH)|PERM(WRITE_REQ, ENABLE);
            }

            attm_att_set_permission(HTPT_HANDLE(HTS_IDX_MEAS_INTV_VAL), perm, 0);
        }

        // service is ready, go into an Idle state
        ke_state_set(p_env->task, HTPT_IDLE);
    }

    return (status);
}

/**
 ****************************************************************************************
 * @brief Destruction of the HTPT module - due to a reset for instance.
 * This function clean-up allocated memory (attribute database is destroyed by another
 * procedure)
 *
 * @param[in|out]    p_env        Collector or Service allocated environment data.
 ****************************************************************************************
 */
static void htpt_destroy(struct prf_task_env *p_env)
{
    struct htpt_env_tag *p_htpt_env = (struct htpt_env_tag *) p_env->env;

    // free profile environment variables
    if (p_htpt_env->p_operation != NULL)
    {
        ke_free(p_htpt_env->p_operation);
    }

    p_env->env = NULL;
    ke_free(p_htpt_env);
}

/**
 ****************************************************************************************
 * @brief Handles Connection creation
 *
 * @param[in|out]    p_env      Collector or Service allocated environment data.
 * @param[in]        conidx     Connection index
 ****************************************************************************************
 */
static void htpt_create(struct prf_task_env *p_env, uint8_t conidx)
{
    // Clear configuration for this connection
    struct htpt_env_tag *p_htpt_env = (struct htpt_env_tag *) p_env->env;

    p_htpt_env->ntf_ind_cfg[conidx] = 0;
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
static void htpt_cleanup(struct prf_task_env *p_env, uint8_t conidx, uint8_t reason)
{
    // Clear configuration for this connection
    struct htpt_env_tag *p_htpt_env = (struct htpt_env_tag *) p_env->env;

    p_htpt_env->ntf_ind_cfg[conidx] = 0;
}

/**
 ****************************************************************************************
 * @brief Compute a flag allowing to know attributes to add into the database
 * @param  features     Health thermometer Feature (@see enum htpt_features)
 * @return a 16-bit flag whose each bit matches an attribute. If the bit is set to 1, the
 * attribute will be added into the database.
 ****************************************************************************************
 */
static uint16_t htpt_compute_att_table(uint16_t features)
{
    //Temperature Measurement Characteristic is mandatory
    uint16_t att_table = HTPT_TEMP_MEAS_MASK;

    //Check if Temperature Type Char. is supported
    if (GETB(features, HTPT_TEMP_TYPE_CHAR_SUP))
    {
        att_table |= HTPT_TEMP_TYPE_MASK;
    }

    //Check if Intermediate Temperature Char. is supported
    if (GETB(features, HTPT_INTERM_TEMP_CHAR_SUP))
    {
        att_table |= HTPT_INTM_TEMP_MASK;
    }

    //Check if Measurement Interval Char. is supported
    if (GETB(features, HTPT_MEAS_INTV_CHAR_SUP))
    {
        att_table |= HTPT_MEAS_INTV_MASK;

        //Check if Measurement Interval Char. supports indications
        if (GETB(features, HTPT_MEAS_INTV_IND_SUP))
        {
            att_table |= HTPT_MEAS_INTV_CCC_MASK;
        }

        //Check if Measurement Interval Char. is writable
        if (GETB(features, HTPT_MEAS_INTV_WR_SUP))
        {
            att_table |= HTPT_MEAS_INTV_VALID_RGE_MASK;
        }
    }

    return att_table;
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// HTPT Task interface required by profile manager
const struct prf_task_cbs htpt_itf =
{
    (prf_init_fnct) htpt_init,
    htpt_destroy,
    htpt_create,
    htpt_cleanup,
};


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

uint8_t htpt_get_valid_rge_offset(uint16_t features)
{
    uint8_t offset = 0;

    if (GETB(features, HTPT_MEAS_INTV_WR_SUP))
    {
        offset += 1;

        if (GETB(features, HTPT_MEAS_INTV_IND_SUP))
        {
            offset += 1;
        }
    }

    return offset;
}

uint8_t htpt_pack_temp_value(uint8_t *p_packed_temp, struct htp_temp_meas temp_meas)
{
    uint8_t cursor = 0;

    *(p_packed_temp + cursor) = temp_meas.flags;
    cursor += 1;

    co_write32p(p_packed_temp + cursor, temp_meas.temp);
    cursor += 4;

    // Time Flag Set
    if (GETB(temp_meas.flags, HTP_FLAG_TIME))
    {
        cursor += prf_pack_date_time(p_packed_temp + cursor, &(temp_meas.time_stamp));
    }

    // Type flag set
    if (GETB(temp_meas.flags, HTP_FLAG_TYPE))
    {
        *(p_packed_temp + cursor) = temp_meas.type;
        cursor += 1;
    }

    // Clear unused packet data
    if (cursor < HTPT_TEMP_MEAS_MAX_LEN)
    {
        memset(p_packed_temp + cursor, 0, (HTPT_TEMP_MEAS_MAX_LEN - cursor));
    }

    return cursor;
}

void htpt_exe_operation(void)
{
    struct htpt_env_tag *p_htpt_env = PRF_ENV_GET(HTPT, htpt);

    ASSERT_ERR(p_htpt_env->p_operation != NULL);

    bool finished = true;

    while(p_htpt_env->p_operation->cursor < BLE_CONNECTION_MAX)
    {
        // check if this type of event is enabled
        if (((p_htpt_env->ntf_ind_cfg[p_htpt_env->p_operation->cursor] & p_htpt_env->p_operation->op) != 0)
            // and event not filtered on current connection
            && (p_htpt_env->p_operation->conidx != p_htpt_env->p_operation->cursor))
        {
            // trigger the event
            struct gattc_send_evt_cmd *p_evt = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                KE_BUILD_ID(TASK_GATTC , p_htpt_env->p_operation->cursor), prf_src_task_get(&p_htpt_env->prf_env, 0),
                gattc_send_evt_cmd, p_htpt_env->p_operation->length);

            p_evt->operation = (p_htpt_env->p_operation->op !=
                    HTPT_CFG_INTERM_MEAS_NTF) ? GATTC_INDICATE : GATTC_NOTIFY;
            p_evt->length    = p_htpt_env->p_operation->length;
            p_evt->handle    = p_htpt_env->p_operation->handle;
            memcpy(p_evt->value, p_htpt_env->p_operation->data, p_evt->length);

            ke_msg_send(p_evt);

            finished = false;
            p_htpt_env->p_operation->cursor++;
            break;
        }

        p_htpt_env->p_operation->cursor++;
    }

    // check if operation is finished
    if (finished)
    {
        // do not send response if operation has been locally requested
        if (p_htpt_env->p_operation->dest_id != prf_src_task_get(&p_htpt_env->prf_env, 0))
        {
            // send response to requester
            struct htpt_meas_intv_upd_rsp *p_rsp =
                    KE_MSG_ALLOC(((p_htpt_env->p_operation->op ==
                            HTPT_CFG_MEAS_INTV_IND) ? HTPT_MEAS_INTV_UPD_RSP : HTPT_TEMP_SEND_RSP),
                            p_htpt_env->p_operation->dest_id, prf_src_task_get(&p_htpt_env->prf_env, 0),
                            htpt_meas_intv_upd_rsp);

            p_rsp->status = GAP_ERR_NO_ERROR;
            ke_msg_send(p_rsp);
        }

        // free operation
        ke_free(p_htpt_env->p_operation);
        p_htpt_env->p_operation = NULL;

        // go back to idle state
        ke_state_set(prf_src_task_get(&(p_htpt_env->prf_env), 0), HTPT_IDLE);
    }
}

uint8_t htpt_update_ntf_ind_cfg(uint8_t conidx, uint8_t cfg, uint16_t valid_val, uint16_t value)
{
    struct htpt_env_tag *p_htpt_env = PRF_ENV_GET(HTPT, htpt);
    uint8_t status = GAP_ERR_NO_ERROR;

    if ((value != valid_val) && (value != PRF_CLI_STOP_NTFIND))
    {
        status = PRF_APP_ERROR;

    }
    else if (value == valid_val)
    {
        p_htpt_env->ntf_ind_cfg[conidx] |= cfg;
    }
    else
    {
        p_htpt_env->ntf_ind_cfg[conidx] &= ~cfg;
    }

    if (status == GAP_ERR_NO_ERROR)
    {
        // inform application that notification/indication configuration has changed
        struct htpt_cfg_indntf_ind *p_ind = KE_MSG_ALLOC(HTPT_CFG_INDNTF_IND,
                prf_dst_task_get(&p_htpt_env->prf_env, conidx), prf_src_task_get(&p_htpt_env->prf_env, conidx),
                htpt_cfg_indntf_ind);

        p_ind->conidx      = conidx;
        p_ind->ntf_ind_cfg = p_htpt_env->ntf_ind_cfg[conidx];
        ke_msg_send(p_ind);
    }

    return (status);
}

const struct prf_task_cbs* htpt_prf_itf_get(void)
{
   return &htpt_itf;
}

uint16_t htpt_att_hdl_get(struct htpt_env_tag *p_htpt_env, uint8_t att_idx)
{
    uint16_t handle = p_htpt_env->shdl;

    do
    {
        // Mandatory attribute handle
        if (att_idx > HTS_IDX_TEMP_MEAS_IND_CFG)
        {
            handle += HTPT_TEMP_MEAS_ATT_NB;
        }
        else
        {
            handle += att_idx;
            break;
        }

        // Temperature Type
        if ((GETB(p_htpt_env->features, HTPT_TEMP_TYPE_CHAR_SUP)) && (att_idx > HTS_IDX_TEMP_TYPE_VAL))
        {
            handle += HTPT_TEMP_TYPE_ATT_NB;
        }
        else if (!GETB(p_htpt_env->features, HTPT_TEMP_TYPE_CHAR_SUP))
        {
            handle = ATT_INVALID_HANDLE;
            break;
        }
        else
        {
            handle += att_idx - HTS_IDX_TEMP_TYPE_CHAR;
            break;
        }

        // Intermediate Temperature Measurement
        if ((GETB(p_htpt_env->features, HTPT_INTERM_TEMP_CHAR_SUP)) && (att_idx > HTS_IDX_INTERM_TEMP_CFG))
        {
            handle += HTPT_INTERM_MEAS_ATT_NB;
        }
        else if (!GETB(p_htpt_env->features, HTPT_INTERM_TEMP_CHAR_SUP))
        {
            handle = ATT_INVALID_HANDLE;
            break;
        }
        else
        {
            handle += att_idx - HTS_IDX_INTERM_TEMP_CHAR;
            break;
        }

        // Measurement Interval
        if (!GETB(p_htpt_env->features, HTPT_MEAS_INTV_CHAR_SUP) || (att_idx >= HTS_IDX_NB))
        {
            handle = ATT_INVALID_HANDLE;
            break;
        }

        if (att_idx <= HTS_IDX_MEAS_INTV_VAL)
        {
            handle += att_idx - HTS_IDX_MEAS_INTV_CHAR;
            break;
        }
        else
        {
            handle += HTPT_MEAS_INTV_ATT_NB;
        }

        // Measurement Interval Indication
        if (att_idx == HTS_IDX_MEAS_INTV_CFG)
        {
            if (!GETB(p_htpt_env->features, HTPT_MEAS_INTV_IND_SUP))
            {
                handle = ATT_INVALID_HANDLE;
                break;
            }
        }
        // Measurement Interval Write permission
        else if (GETB(p_htpt_env->features, HTPT_MEAS_INTV_WR_SUP))
        {
            handle += HTPT_MEAS_INTV_CCC_ATT_NB;

            if (!GETB(p_htpt_env->features, HTPT_MEAS_INTV_WR_SUP))
            {
                handle = ATT_INVALID_HANDLE;
                break;
            }
        }
    } while (0);

    return handle;
}

uint8_t htpt_att_idx_get(struct htpt_env_tag *p_htpt_env, uint16_t handle)
{
    uint16_t handle_ref = p_htpt_env->shdl;
    uint8_t att_idx = ATT_INVALID_IDX;

    do
    {
        // not valid hande
        if (handle < handle_ref)
        {
            break;
        }

        // Mandatory attribute handle
        handle_ref += HTPT_TEMP_MEAS_ATT_NB;

        if (handle < handle_ref)
        {
            att_idx = HTS_IDX_TEMP_TYPE_CHAR - (handle_ref - handle);
            break;
        }

        // Temperature Type
        if (GETB(p_htpt_env->features, HTPT_TEMP_TYPE_CHAR_SUP))
        {
            handle_ref += HTPT_TEMP_TYPE_ATT_NB;

            if (handle < handle_ref)
            {
                att_idx = HTS_IDX_INTERM_TEMP_CHAR - (handle_ref - handle);
                break;
            }
        }

        // Intermediate Temperature Measurement
        if (GETB(p_htpt_env->features, HTPT_INTERM_TEMP_CHAR_SUP))
        {
            handle_ref += HTPT_INTERM_MEAS_ATT_NB;

            if (handle < handle_ref)
            {
                att_idx = HTS_IDX_MEAS_INTV_CHAR - (handle_ref - handle);
                break;
            }
        }

        // Measurement Interval
        if (GETB(p_htpt_env->features, HTPT_MEAS_INTV_CHAR_SUP))
        {
            handle_ref += HTPT_MEAS_INTV_ATT_NB;

            if (handle < handle_ref)
            {
                att_idx = HTS_IDX_MEAS_INTV_CFG - (handle_ref - handle);
                break;
            }

            if (GETB(p_htpt_env->features, HTPT_MEAS_INTV_IND_SUP))
            {
                if (handle == handle_ref)
                {
                    att_idx = HTS_IDX_MEAS_INTV_CFG;
                    break;
                }

                handle_ref += HTPT_MEAS_INTV_CCC_ATT_NB;
            }

            if (GETB(p_htpt_env->features, HTPT_MEAS_INTV_WR_SUP))
            {
                if (handle == handle_ref)
                {
                    att_idx = HTS_IDX_MEAS_INTV_VAL_RANGE;
                    break;
                }
            }
        }
    } while (0);

    return att_idx;
}

#endif //BLE_HT_THERMOM

/// @} HTPT
