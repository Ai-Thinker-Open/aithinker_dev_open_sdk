/**
 ****************************************************************************************
 *
 * @file tips_task.c
 *
 * @brief Time Profile Server Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup TIPSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_TIP_SERVER)
#include "co_utils.h"
#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "tips.h"
#include "tips_task.h"

#include "prf_utils.h"


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPS_ENABLE_REQ message.
 * The handler enables the Time Server Profile.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tips_enable_req_handler(ke_msg_id_t const msgid,
                                   struct tips_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
     uint8_t conidx = KE_IDX_GET(dest_id);
     struct tips_enable_rsp *p_rsp;
     // Status
     uint8_t status = GAP_ERR_NO_ERROR;

     if (gapc_get_conhdl(conidx) == GAP_INVALID_CONHDL)
     {
         status = PRF_ERR_DISCONNECTED;
     }
     else if (ke_state_get(dest_id) == TIPS_IDLE)
     {
         if (p_param->current_time_ntf_en == PRF_CLI_START_NTF)
         {
              // Get the address of the environment
             struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
             // Enable Bonded Data
             SETB(p_tips_env->p_env[conidx]->ntf_state, TIPS_CTS_CURRENT_TIME_CFG, 1);
         }
     }
     else
     {
         status = PRF_ERR_REQ_DISALLOWED;
     }

     // Send to APP the details of the discovered attributes on TIPS
     p_rsp = KE_MSG_ALLOC(TIPS_ENABLE_RSP, src_id, dest_id, tips_enable_rsp);

     p_rsp->status = status;
     ke_msg_send(p_rsp);

     return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPS_UPD_CURR_TIME_CMD message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tips_upd_curr_time_cmd_handler(ke_msg_id_t const msgid,
                                          struct tips_upd_curr_time_cmd const *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if ((p_tips_env->p_env[conidx] != NULL) && (ke_state_get(dest_id) == TIPS_IDLE))
    {
        // Check if Notifications are enabled
        if (GETB(p_tips_env->p_env[conidx]->ntf_state,TIPS_CTS_CURRENT_TIME_CFG))
        {
            // Check if notification can be sent
            if (GETB(p_param->current_time.adjust_reason, TIPS_FLAG_EXT_TIME_UPDATE))
            {
                if (p_param->enable_ntf_send == 0)
                {
                    status = PRF_ERR_REQ_DISALLOWED;
                }
            }

            if (status == GAP_ERR_NO_ERROR)
            {
                // Allocate the GATT notification message
                struct gattc_send_evt_cmd *p_meas_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                        KE_BUILD_ID(TASK_GATTC, conidx), dest_id,
                        gattc_send_evt_cmd, CTS_CURRENT_TIME_VAL_LEN);

                // Fill in the p_parameter structure
                p_meas_val->operation = GATTC_NOTIFY;
                p_meas_val->handle = p_tips_env->cts_shdl + CTS_IDX_CURRENT_TIME_VAL;
                // Pack the Current Time value
                p_meas_val->length =  tips_pack_curr_time_value(p_meas_val->value, &p_param->current_time);

                // Go to idle state
                ke_state_set(dest_id, TIPS_BUSY);
                // Send the event
                ke_msg_send(p_meas_val);
            }
        }
        else
        {
            status = PRF_ERR_NTF_DISABLED;
        }
    }
    else
    {
        status = GAP_ERR_INVALID_PARAM;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        tips_send_cmp_evt(p_tips_env, conidx, TIPS_UPD_CURR_TIME_CMD_OP_CODE, status);
    }

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
    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
    uint8_t conidx = KE_IDX_GET(src_id);

    if ((ke_state_get(dest_id) == TIPS_IDLE) && (p_tips_env->p_env[conidx] != NULL))
    {
        //CTS : Current Time client configuration
        if (p_param->handle == p_tips_env->cts_shdl + CTS_IDX_CURRENT_TIME_CFG)
        {
            uint16_t value = (GETB(p_tips_env->p_env[conidx]->ntf_state, TIPS_CTS_CURRENT_TIME_CFG))
                                    ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND;

            // Send data to peer device
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(
                    GATTC_READ_CFM,
                    src_id, dest_id,
                    gattc_read_cfm,
                    sizeof(uint16_t));

            p_cfm->length = sizeof(uint16_t);
            memcpy(p_cfm->value, &value, sizeof(uint16_t));
            p_cfm->handle = p_param->handle;
            p_cfm->status = ATT_ERR_NO_ERROR;

            // Send value to peer device.
            ke_msg_send(p_cfm);
        }
        else
        {
            // Inform APP about read request
            struct tips_rd_req_ind *p_ind = KE_MSG_ALLOC(
                    TIPS_RD_REQ_IND,
                    prf_dst_task_get(&(p_tips_env->prf_env), conidx),
                    prf_src_task_get(&(p_tips_env->prf_env), conidx),
                    tips_rd_req_ind);
            uint8_t status = PRF_ERR_REQ_DISALLOWED;

            // Current Time Service
            if (p_param->handle < p_tips_env->cts_shdl + CTS_IDX_NB)
            {
                // Current Time Char value
                if (p_param->handle == p_tips_env->cts_shdl + CTS_IDX_CURRENT_TIME_VAL)
                {
                    p_ind->char_code = TIPS_CTS_CURRENT_TIME;
                    status = ATT_ERR_NO_ERROR;
                }
                // Local Time Info Char value
                else if ((p_param->handle == p_tips_env->cts_shdl + CTS_IDX_LOCAL_TIME_INFO_VAL)
                        && (GETB(p_tips_env->features, TIPS_CTS_LOC_TIME_INFO_SUP)))
                {
                    p_ind->char_code = TIPS_CTS_LOCAL_TIME_INFO;
                    status = ATT_ERR_NO_ERROR;
                }
                // Reference Time Info Char value
                else if ((p_param->handle == p_tips_env->cts_shdl + CTS_IDX_REF_TIME_INFO_VAL)
                        && (GETB(p_tips_env->features, TIPS_CTS_REF_TIME_INFO_SUP)))
                {
                    p_ind->char_code = TIPS_CTS_REF_TIME_INFO;
                    status = ATT_ERR_NO_ERROR;
                }
            }
            // Time with DST Char value
            else if ((p_param->handle == p_tips_env->ndcs_shdl + NDCS_IDX_TIME_DST_VAL)
                    && (GETB(p_tips_env->features, TIPS_NDCS_SUP)))
            {
                p_ind->char_code = TIPS_NDCS_TIME_DST;
                status = ATT_ERR_NO_ERROR;
            }
            // Time Update State
            else if ((p_param->handle == p_tips_env->rtus_shdl + RTUS_IDX_TIME_UPD_STATE_VAL)
                    && (GETB(p_tips_env->features, TIPS_RTUS_SUP)))
            {
                p_ind->char_code = TIPS_RTUS_TIME_UPD_STATE_VAL;
                status = ATT_ERR_NO_ERROR;
            }
            else
            {
                // Set error code
                status = PRF_ERR_INEXISTENT_HDL;
            }

            if (status == ATT_ERR_NO_ERROR)
            {
                // Save handle
                p_tips_env->p_env[conidx]->handle = p_param->handle;
                // Send message to the APP
                ke_msg_send(p_ind);
                // Go to busy state
                ke_state_set(dest_id, TIPS_BUSY);
            }
            else
            {
                struct gattc_read_cfm *p_cfm;

                // Free message
                ke_msg_free(ke_param2msg(p_ind));

                // Send data to peer device
                p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);
                p_cfm->handle = p_param->handle;
                p_cfm->status = status;

                // Send to peer device.
                ke_msg_send(p_cfm);
            }
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPS_RD_CFM message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tips_rd_cfm_handler(ke_msg_id_t const msgid,
                                struct tips_rd_cfm const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if ((ke_state_get(dest_id) == TIPS_BUSY) && (p_tips_env->p_env[conidx] != NULL))
    {
        if (p_tips_env->p_env[conidx]->handle != ATT_INVALID_HANDLE)
        {
            uint8_t status = GAP_ERR_NO_ERROR;
            // Send data to peer device
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(
                    GATTC_READ_CFM,
                    KE_BUILD_ID(TASK_GATTC, conidx), dest_id,
                    gattc_read_cfm,
                    sizeof(struct tips_rd_cfm));

            switch (p_param->op_code)
            {
                case TIPS_CTS_CURRENT_TIME:
                {
                    // Pack Current Time
                    p_cfm->length = tips_pack_curr_time_value(p_cfm->value, &p_param->value.curr_time);
                } break;

                case TIPS_CTS_LOCAL_TIME_INFO:
                {
                    // Pack Local Time
                    p_cfm->length = sizeof(struct tip_loc_time_info);
                    memcpy(p_cfm->value, &p_param->value, p_cfm->length);
                } break;

                case TIPS_CTS_REF_TIME_INFO:
                {
                    // Pack Reference Time
                    p_cfm->length = sizeof(struct tip_ref_time_info);
                    memcpy(p_cfm->value, &p_param->value, p_cfm->length);
                } break;

                case TIPS_NDCS_TIME_DST:
                {
                    // Pack Time with DST
                    p_cfm->length = tips_pack_time_dst_value(p_cfm->value, &p_param->value.time_with_dst);
                } break;

                case TIPS_RTUS_TIME_UPD_STATE_VAL:
                {
                    // Pack Time Update State
                    p_cfm->length = sizeof(struct tip_time_upd_state);
                    memcpy(p_cfm->value, &p_param->value, p_cfm->length);
                } break;
                default:
                {
                    // Return Application error to peer
                    p_cfm->length = 0;
                    status = PRF_APP_ERROR;
                } break;
            }

            // Set parameters
            p_cfm->handle = p_tips_env->p_env[conidx]->handle;
            p_cfm->status = status;
            // Send value to peer device.
            ke_msg_send(p_cfm);
            // Go to idle state
            ke_state_set(dest_id, TIPS_IDLE);
            // Reset environment variable
            p_tips_env->p_env[conidx]->handle = ATT_INVALID_HANDLE;
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATTC_CMP_EVT for GATTC_NOTIFY message meaning that Measurement
 * notification has been correctly sent to peer device (but not confirmed by peer device).
 * *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
    uint8_t conidx = KE_IDX_GET(src_id);

    if ((p_tips_env->p_env[conidx] != NULL) && (ke_state_get(dest_id) == TIPS_BUSY))
    {
        if (p_param->operation == GATTC_NOTIFY)
        {
            tips_send_cmp_evt(p_tips_env, conidx, TIPS_UPD_CURR_TIME_CMD_OP_CODE, p_param->status);
        }
    }

    return (KE_MSG_CONSUMED);
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
    uint16_t cfg_value           = 0x0000;
    uint8_t status               = GAP_ERR_NO_ERROR;
    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);
    uint8_t conidx = KE_IDX_GET(src_id);

    struct gattc_write_cfm *p_cfm;

    // CTS : Current Time Char. - Notification Configuration
    if (p_param->handle == p_tips_env->cts_shdl + CTS_IDX_CURRENT_TIME_CFG)
    {
        // Extract value before check
        memcpy(&cfg_value, &p_param->value[0], sizeof(uint16_t));

        // only update configuration if value for stop or notification enable
        if ((cfg_value == PRF_CLI_STOP_NTFIND) || (cfg_value == PRF_CLI_START_NTF))
        {
            struct tips_current_time_ccc_ind *p_ind;

            if (cfg_value == PRF_CLI_STOP_NTFIND)
            {
                SETB(p_tips_env->p_env[conidx]->ntf_state, TIPS_CTS_CURRENT_TIME_CFG, 0);
            }
            else // PRF_CLI_START_NTF
            {
                SETB(p_tips_env->p_env[conidx]->ntf_state, TIPS_CTS_CURRENT_TIME_CFG, 1);
            }

            // Inform APP of configuration change
            p_ind = KE_MSG_ALLOC(TIPS_CURRENT_TIME_CCC_IND,
                                prf_dst_task_get(&p_tips_env->prf_env, conidx),
                                prf_src_task_get(&p_tips_env->prf_env, conidx),
                                tips_current_time_ccc_ind);

            p_ind->cfg_val = cfg_value;

            ke_msg_send(p_ind);
        }
        else
        {
            status = PRF_ERR_INVALID_PARAM;
        }
    }
    // RTUS : Time Update Control Point Char. - Val
    else if (p_param->handle == p_tips_env->rtus_shdl + RTUS_IDX_TIME_UPD_CTNL_PT_VAL)
    {
        // Check if value to write is in allowed range
        if ((p_param->value[0] == TIPS_TIME_UPD_CTNL_PT_GET) ||
            (p_param->value[0] == TIPS_TIME_UPD_CTNL_PT_CANCEL))
        {
            struct tips_time_upd_ctnl_pt_ind *p_ind;

            // Update value
            p_tips_env->time_upd_state = p_param->value[0];

            // Send APP the indication with the new value
            p_ind = KE_MSG_ALLOC(TIPS_TIME_UPD_CTNL_PT_IND,
                                prf_dst_task_get(&p_tips_env->prf_env, conidx),
                                prf_src_task_get(&p_tips_env->prf_env, conidx),
                                tips_time_upd_ctnl_pt_ind);

            // Time Update Control Point Characteristic Value
            p_ind->value = p_tips_env->time_upd_state;
            // Send message
            ke_msg_send(p_ind);
        }
        else
        {
            status = PRF_ERR_INVALID_PARAM;
        }
    }
    else
    {
        status = PRF_ERR_NOT_WRITABLE;
    }

    // Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = p_param->handle;
    p_cfm->status = status;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(tips)
{
    {TIPS_ENABLE_REQ,                   (ke_msg_func_t)tips_enable_req_handler},
    {GATTC_WRITE_REQ_IND,               (ke_msg_func_t)gattc_write_req_ind_handler},
    {TIPS_UPD_CURR_TIME_CMD,            (ke_msg_func_t)tips_upd_curr_time_cmd_handler},
    {GATTC_READ_REQ_IND,                (ke_msg_func_t)gattc_read_req_ind_handler},
    {TIPS_RD_CFM,                       (ke_msg_func_t)tips_rd_cfm_handler},
    {GATTC_CMP_EVT,                     (ke_msg_func_t)gattc_cmp_evt_handler},
};

void tips_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct tips_env_tag *p_tips_env = PRF_ENV_GET(TIPS, tips);

    p_task_desc->msg_handler_tab = tips_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(tips_msg_handler_tab);
    p_task_desc->state           = p_tips_env->state;
    p_task_desc->idx_max         = TIPS_IDX_MAX;
}

#endif //BLE_TIP_SERVER

/// @} TIPSTASK
