/**
 ****************************************************************************************
 *
 * @file blps_task.c
 *
 * @brief Blood Pressure Profile Sensor Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup BLPSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_BP_SENSOR)
#include "co_utils.h"

#include "gap.h"
#include "gattc_task.h"
#include "blps.h"
#include "blps_task.h"
#include "prf_utils.h"



/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BLPS_ENABLE_REQ message.
 * The handler enables the Blood Pressure Sensor Profile and initialize readable values.
 * @param[in] msgid Id of the message received (probably unused).off
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int blps_enable_req_handler(ke_msg_id_t const msgid,
                                   struct blps_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);
    struct blps_enable_rsp *p_cmp_evt;

    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if (gapc_get_conhdl(p_param->conidx) == GAP_INVALID_CONHDL)
    {
        status = PRF_ERR_DISCONNECTED;
    }
    else if (ke_state_get(dest_id) == BLPS_IDLE)
    {
        if (p_param->interm_cp_ntf_en == PRF_CLI_START_NTF)
        {
            // Enable Bonded Data
            SETB(p_blps_env->prfl_ntf_ind_cfg[p_param->conidx], BLPS_INTM_CUFF_PRESS_NTF_CFG, 1);
        }

        if (p_param->bp_meas_ind_en == PRF_CLI_START_IND)
        {
            // Enable Bonded Data
            SETB(p_blps_env->prfl_ntf_ind_cfg[p_param->conidx], BLPS_BP_MEAS_IND_CFG, 1);
        }

        status = GAP_ERR_NO_ERROR;
    }

    // send completed information to APP task that contains error status
    p_cmp_evt = KE_MSG_ALLOC(BLPS_ENABLE_RSP, src_id, dest_id, blps_enable_rsp);
    p_cmp_evt->status = status;
    p_cmp_evt->conidx = p_param->conidx;
    ke_msg_send(p_cmp_evt);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the read request from peer device
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_read_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == BLPS_IDLE)
    {
        // Get the address of the environment
        struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);
        uint8_t conidx = KE_IDX_GET(src_id);

        uint16_t value = 0;
        uint8_t status = ATT_ERR_NO_ERROR;
        struct gattc_read_cfm *p_cfm;

        switch (p_param->handle - p_blps_env->shdl)
        {
            case BPS_IDX_BP_FEATURE_VAL:
            {
                value = p_blps_env->features;
            } break;

            case BPS_IDX_BP_MEAS_IND_CFG:
            {
                value = GETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_BP_MEAS_IND_CFG)
                        ? PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND;
            } break;

            case BPS_IDX_INTM_CUFF_PRESS_NTF_CFG:
            {
                // Characteristic is profile specific
                if (GETB(p_blps_env->prfl_cfg, BLPS_INTM_CUFF_PRESS_SUP))
                {
                    // Fill data
                    value = GETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_INTM_CUFF_PRESS_NTF_CFG)
                            ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;
                }
                else
                {
                    status = ATT_ERR_ATTRIBUTE_NOT_FOUND;
                }
            } break;

            default:
            {
                status = ATT_ERR_REQUEST_NOT_SUPPORTED;
            } break;
        }

        // Send data to peer device
        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint16_t));
        p_cfm->length = sizeof(uint16_t);
        memcpy(p_cfm->value, &value, sizeof(uint16_t));
        p_cfm->handle = p_param->handle;
        p_cfm->status = status;

        // Send value to peer device.
        ke_msg_send(p_cfm);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BLPS_MEAS_SEND_CMD message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int blps_meas_send_cmd_handler(ke_msg_id_t const msgid,
                                      struct blps_meas_send_cmd const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);

    if (gapc_get_conhdl(p_param->conidx) == GAP_INVALID_CONHDL)
    {
        status = PRF_ERR_DISCONNECTED;
    }
    else if (ke_state_get(dest_id) == BLPS_IDLE)
    {

        // Intermediary blood pressure, must be notified if enabled
        if (p_param->flag_interm_cp)
        {
            // Check if supported
            if (GETB(p_blps_env->prfl_cfg, BLPS_INTM_CUFF_PRESS_SUP))
            {
                if (!GETB(p_blps_env->prfl_ntf_ind_cfg[p_param->conidx], BLPS_INTM_CUFF_PRESS_NTF_CFG))
                {
                    status = PRF_ERR_NTF_DISABLED;
                }
            }
            else
            {
                status = PRF_ERR_FEATURE_NOT_SUPPORTED;
            }
        }

        // Stable Blood Pressure Measurement, must be indicated if enabled
        else if (!GETB(p_blps_env->prfl_ntf_ind_cfg[p_param->conidx], BLPS_BP_MEAS_IND_CFG))
        {
            status = PRF_ERR_IND_DISABLED;
        }

        // Check if message can be sent
        if (status == GAP_ERR_NO_ERROR)
        {
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, p_param->conidx), dest_id,
                    gattc_send_evt_cmd, BLPS_BP_MEAS_MAX_LEN);

            // Fill event type and handle which trigger event
            if (p_param->flag_interm_cp)
            {
                p_meas_val->operation = GATTC_NOTIFY;
                p_meas_val->handle = p_blps_env->shdl + BPS_IDX_INTM_CUFF_PRESS_VAL;
            }
            else
            {
                // Fill in the p_parameter structure
                p_meas_val->operation = GATTC_INDICATE;
                p_meas_val->handle = p_blps_env->shdl + BPS_IDX_BP_MEAS_VAL;
            }

            // Pack the BP Measurement value
            p_meas_val->length = blps_pack_meas_value(p_meas_val->value, &p_param->meas_val);

            // Send the event
            ke_msg_send(p_meas_val);
            // Go to busy state
            ke_state_set(dest_id, BLPS_BUSY);
        }
    }
    else
    {
        // Save it for later
        msg_status = KE_MSG_SAVED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // Send error message to application
        blps_send_cmp_evt(p_blps_env, p_param->conidx, BLPS_MEAS_SEND_CMD_OP_CODE, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GL2C_CODE_ATT_WR_CMD_IND message.
 * The handler compares the new values with current ones and notifies them if they changed.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);
    struct gattc_write_cfm *p_cfm;
    uint8_t conidx = KE_IDX_GET(src_id);

    uint16_t value = 0x0000;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t char_code = 0;

    //Extract value before check
    memcpy(&value, &(p_param->value), sizeof(uint16_t));

    //BP Measurement Char. - Client Char. Configuration
    if (p_param->handle == (p_blps_env->shdl + BPS_IDX_BP_MEAS_IND_CFG))
    {
        if ((value == PRF_CLI_STOP_NTFIND) || (value == PRF_CLI_START_IND))
        {
            char_code = BPS_BP_MEAS_CHAR;

            if (value == PRF_CLI_STOP_NTFIND)
            {
                SETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_BP_MEAS_IND_CFG, 0);
            }
            else //PRF_CLI_START_IND
            {
                SETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_BP_MEAS_IND_CFG, 1);
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }
    else if (p_param->handle == (p_blps_env->shdl + BPS_IDX_INTM_CUFF_PRESS_NTF_CFG))
    {
        if ((value == PRF_CLI_STOP_NTFIND) || (value == PRF_CLI_START_NTF))
        {
            char_code = BPS_INTM_CUFF_MEAS_CHAR;

            if (value == PRF_CLI_STOP_NTFIND)
            {
                SETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_INTM_CUFF_PRESS_NTF_CFG, 0);
            }
            else //PRF_CLI_START_NTF
            {
                SETB(p_blps_env->prfl_ntf_ind_cfg[conidx], BLPS_INTM_CUFF_PRESS_NTF_CFG, 1);
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }

    if (status == GAP_ERR_NO_ERROR)
    {
        // Inform APP of configuration change
        struct blps_cfg_indntf_ind *p_ind = KE_MSG_ALLOC(
                BLPS_CFG_INDNTF_IND,
                prf_dst_task_get(&p_blps_env->prf_env, conidx),
                prf_src_task_get(&p_blps_env->prf_env, conidx),
                blps_cfg_indntf_ind);

        p_ind->conidx = conidx;
        p_ind->char_code = char_code;
        memcpy(&p_ind->cfg_val, &value, sizeof(uint16_t));

        ke_msg_send(p_ind);
    }

    // Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = p_param->handle;
    p_cfm->status = status;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY and GATT_INDICATE message meaning
 * that Measurement notification/indication has been correctly sent to peer device
 *
 *
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
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);
    uint8_t conidx = KE_IDX_GET(src_id);

    switch (p_param->operation)
    {
        case GATTC_NOTIFY:
        case GATTC_INDICATE:
        {
            // Send a complete event status to the application
            blps_send_cmp_evt(p_blps_env, conidx, BLPS_MEAS_SEND_CMD_OP_CODE, p_param->status);
        } break;

        default:
        {

        } break;
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(blps)
{
    {BLPS_ENABLE_REQ,       (ke_msg_func_t) blps_enable_req_handler},
    {GATTC_READ_REQ_IND,    (ke_msg_func_t) gattc_read_req_ind_handler},
    {BLPS_MEAS_SEND_CMD,    (ke_msg_func_t) blps_meas_send_cmd_handler},
    {GATTC_CMP_EVT,         (ke_msg_func_t) gattc_cmp_evt_handler},
    {GATTC_WRITE_REQ_IND,   (ke_msg_func_t) gattc_write_req_ind_handler},
};

void blps_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct blps_env_tag *p_blps_env = PRF_ENV_GET(BLPS, blps);

    p_task_desc->msg_handler_tab = blps_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(blps_msg_handler_tab);
    p_task_desc->state           = p_blps_env->state;
    p_task_desc->idx_max         = BLPS_IDX_MAX;
}

#endif /* #if (BLE_BP_SENSOR) */

/// @} BLPSTASK
