/**
 ****************************************************************************************
 *
 * @file bass_task.c
 *
 * @brief Battery Service Server Role Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup BASSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "rwprf_config.h"
#if (BLE_BATT_SERVER)

#include "gap.h"
#include "gattc_task.h"

#include "bass.h"
#include "bass_task.h"

#include "prf_utils.h"

#include "co_utils.h"

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BAPS_ENABLE_REQ message.
 * The handler enables the Battery 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int bass_enable_req_handler(ke_msg_id_t const msgid,
                                   struct bass_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_SAVED;

    // check state of the task
    if (ke_state_get(dest_id) == BASS_IDLE)
    {
        struct bass_env_tag *p_bass_env = PRF_ENV_GET(BASS, bass);

        // Check provided values
        if ((p_param->conidx > BLE_CONNECTION_MAX)
            || (gapc_get_conhdl(p_param->conidx) == GAP_INVALID_CONHDL))
        {
            // an error occurs, trigger it.
            struct bass_enable_rsp *p_rsp = KE_MSG_ALLOC(BASS_ENABLE_RSP, src_id,
                dest_id, bass_enable_rsp);

            p_rsp->conidx = p_param->conidx;
            p_rsp->status = (p_param->conidx > BLE_CONNECTION_MAX) ? GAP_ERR_INVALID_PARAM : PRF_ERR_REQ_DISALLOWED;
            ke_msg_send(p_rsp);

            msg_status = KE_MSG_CONSUMED;
        }
        else
        {
            // put task in a busy state
            msg_status = KE_MSG_NO_FREE;
            ke_state_set(dest_id, BASS_BUSY);
            p_bass_env->ntf_cfg[p_param->conidx] = p_param->ntf_cfg;
            p_bass_env->p_operation = ke_param2msg(p_param);
            p_bass_env->cursor = 0;

            // trigger notification
            bass_exe_operation();
        }
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BAPS_BATT_LEVEL_SEND_REQ message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int bass_batt_level_upd_req_handler(ke_msg_id_t const msgid,
                                            struct bass_batt_level_upd_req const *p_param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_SAVED;

    // check state of the task
    if (ke_state_get(dest_id) == BASS_IDLE)
    {
        struct bass_env_tag *p_bass_env = PRF_ENV_GET(BASS, bass);

        // Check provided values
        if ((p_param->bas_instance < p_bass_env->svc_nb) && (p_param->batt_level <= BAS_BATTERY_LVL_MAX))
        {
            // update the battery level value
            p_bass_env->batt_lvl[p_param->bas_instance] = p_param->batt_level;

            // put task in a busy state
            msg_status = KE_MSG_NO_FREE;
            ke_state_set(dest_id, BASS_BUSY);
            p_bass_env->p_operation = ke_param2msg(p_param);
            p_bass_env->cursor = 0;

            // trigger notification
            bass_exe_operation();
        }
        else
        {
            // an error occurs, trigger it.
            struct bass_batt_level_upd_rsp *p_rsp = KE_MSG_ALLOC(BASS_BATT_LEVEL_UPD_RSP, src_id,
                    dest_id, bass_batt_level_upd_rsp);

            p_rsp->status = PRF_ERR_INVALID_PARAM;
            ke_msg_send(p_rsp);
            msg_status = KE_MSG_CONSUMED;
        }
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_att_info_cfm *p_cfm;
    uint8_t svc_idx = 0, att_idx = 0;
    // retrieve handle information
    uint8_t status = bass_get_att_idx(p_param->handle, &svc_idx, &att_idx);

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = p_param->handle;

    if (status == GAP_ERR_NO_ERROR)
    {
        // check if it's a client configuration char
        if (att_idx == BAS_IDX_BATT_LVL_NTF_CFG)
        {
            // CCC attribute length = 2
            p_cfm->length = 2;
        }
        // not expected request
        else
        {
            p_cfm->length = 0;
            status = ATT_ERR_WRITE_NOT_PERMITTED;
        }
    }

    p_cfm->status = status;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_WRITE_REQ_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid, struct gattc_write_req_ind const *p_param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_write_cfm *p_cfm;
    uint8_t svc_idx = 0, att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = bass_get_att_idx(p_param->handle, &svc_idx, &att_idx);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        struct bass_env_tag *p_bass_env = PRF_ENV_GET(BASS, bass);
        // Extract value before check
        uint16_t ntf_cfg = co_read16p(&p_param->value[0]);

        // Only update configuration if value for stop or notification enable
        if ((att_idx == BAS_IDX_BATT_LVL_NTF_CFG)
                && ((ntf_cfg == PRF_CLI_STOP_NTFIND) || (ntf_cfg == PRF_CLI_START_NTF)))
        {
            // Inform APP of configuration change
            struct bass_batt_level_ntf_cfg_ind *p_ind;

            // Conserve information in environment
            if (ntf_cfg == PRF_CLI_START_NTF)
            {
                // Ntf cfg bit set to 1
                p_bass_env->ntf_cfg[conidx] |= (BAS_BATT_LVL_NTF_SUP << svc_idx);
            }
            else
            {
                // Ntf cfg bit set to 0
                p_bass_env->ntf_cfg[conidx] &= ~(BAS_BATT_LVL_NTF_SUP << svc_idx);
            }

            p_ind = KE_MSG_ALLOC(BASS_BATT_LEVEL_NTF_CFG_IND,
                                prf_dst_task_get(&(p_bass_env->prf_env), conidx), dest_id,
                                bass_batt_level_ntf_cfg_ind);

            p_ind->conidx = conidx;
            p_ind->handle = p_param->handle;
            p_ind->ntf_cfg = p_bass_env->ntf_cfg[conidx];

            ke_msg_send(p_ind);
        }
        else
        {
            status = PRF_APP_ERROR;
        }

    }

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = p_param->handle;
    p_cfm->status = status;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_READ_REQ_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid, struct gattc_read_req_ind const *p_param,
                                      ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    struct gattc_read_cfm *p_cfm;
    uint8_t svc_idx = 0, att_idx = 0;
    uint8_t conidx = KE_IDX_GET(src_id);
    // retrieve handle information
    uint8_t status = bass_get_att_idx(p_param->handle, &svc_idx, &att_idx);
    uint16_t length = 0;
    struct bass_env_tag *p_bass_env = PRF_ENV_GET(BASS, bass);

    // If the attribute has been found, status is GAP_ERR_NO_ERROR
    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == BAS_IDX_BATT_LVL_VAL)
        {
            length = sizeof(uint8_t);
        }
        // read notification information
        else if (att_idx == BAS_IDX_BATT_LVL_NTF_CFG)
        {
            length = sizeof(uint16_t);
        }
        else if (att_idx == BAS_IDX_BATT_LVL_PRES_FMT)
        {
            length = PRF_CHAR_PRES_FMT_SIZE;
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }

    //Send write response
    p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, length);
    p_cfm->handle = p_param->handle;
    p_cfm->status = status;
    p_cfm->length = length;

    if (status == GAP_ERR_NO_ERROR)
    {
        // read notification information
        if (att_idx == BAS_IDX_BATT_LVL_VAL)
        {
            p_cfm->value[0] = p_bass_env->batt_lvl[svc_idx];
        }
        // retrieve notification config
        else if (att_idx == BAS_IDX_BATT_LVL_NTF_CFG)
        {
            uint16_t ntf_cfg = (p_bass_env->ntf_cfg[conidx] >> svc_idx & BAS_BATT_LVL_NTF_SUP)
                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

            co_write16p(p_cfm->value, ntf_cfg);
        }
        // retrieve battery level format
        else if (att_idx == BAS_IDX_BATT_LVL_PRES_FMT)
        {
            prf_pack_char_pres_fmt(p_cfm->value, &(p_bass_env->batt_level_pres_format[svc_idx]));
        }
        else
        {
            // Not Possible
        }
    }

    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY message meaning that Measurement
 * notification has been correctly sent to peer device (but not confirmed by peer device).
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid, struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if(p_param->operation == GATTC_NOTIFY)
    {
        // continue operation execution
        bass_exe_operation();
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(bass)
{
    {BASS_ENABLE_REQ,               (ke_msg_func_t) bass_enable_req_handler},
    {BASS_BATT_LEVEL_UPD_REQ,       (ke_msg_func_t) bass_batt_level_upd_req_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
};

void bass_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct bass_env_tag *p_bass_env = PRF_ENV_GET(BASS, bass);

    p_task_desc->msg_handler_tab = bass_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(bass_msg_handler_tab);
    p_task_desc->state           = p_bass_env->state;
    p_task_desc->idx_max         = BASS_IDX_MAX;
}

#endif /* #if (BLE_BATT_SERVER) */

/// @} BASSTASK
