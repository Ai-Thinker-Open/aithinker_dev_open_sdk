/**
 ****************************************************************************************
 *
 * @file hrps_task.c
 *
 * @brief Heart Rate Profile Sensor Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup HRPSTASK
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
#include "hrps.h"
#include "hrps_task.h"

#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HRPS_ENABLE_REQ message.
 * The handler enables the Heart Rate Sensor Profile.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hrps_enable_req_handler(ke_msg_id_t const msgid,
                                   struct hrps_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
     // Status
     uint8_t status = PRF_ERR_REQ_DISALLOWED;
     struct hrps_enable_rsp *p_cmp_evt;

     if (ke_state_get(dest_id) == HRPS_IDLE)
     {
         if ((p_param->conidx < BLE_CONNECTION_MAX) &&
                 (p_param->hr_meas_ntf <= PRF_CLI_START_NTF))
         {
             // Get the address of the environment
             struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);

             // Bonded data was not used before
             if (!HRPS_IS_SUPPORTED(p_hrps_env->hr_meas_ntf[p_param->conidx], HRP_PRF_CFG_PERFORMED_OK))
             {
                 // Save notification config
                 p_hrps_env->hr_meas_ntf[p_param->conidx] = p_param->hr_meas_ntf;
                 // Enable Bonded Data
                 p_hrps_env->hr_meas_ntf[p_param->conidx] |= HRP_PRF_CFG_PERFORMED_OK;
                 status = GAP_ERR_NO_ERROR;
             }
         }
         else
         {
             status = PRF_ERR_INVALID_PARAM;
         }
     }

     // send completed information to APP task that contains error status
     p_cmp_evt = KE_MSG_ALLOC(HRPS_ENABLE_RSP, src_id, dest_id, hrps_enable_rsp);
     p_cmp_evt->status = status;
     p_cmp_evt->conidx = p_param->conidx;
     ke_msg_send(p_cmp_evt);

     return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HRPS_MEAS_SEND_CMD message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hrps_meas_send_cmd_handler(ke_msg_id_t const msgid,
                                      struct hrps_meas_send_cmd const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;

    // State shall not be Connected or Busy
    if (ke_state_get(dest_id) == HRPS_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        // Get the address of the environment
        struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);

        if (p_param->meas_val.nb_rr_interval <= HRS_MAX_RR_INTERVAL)
        {
            // allocate and prepare data to notify
            p_hrps_env->p_operation = (struct hrps_op *) ke_malloc(sizeof(struct hrps_op), KE_MEM_KE_MSG);
            p_hrps_env->p_operation->length = hrps_pack_meas_value(p_hrps_env->p_operation->data, &p_param->meas_val);
            p_hrps_env->p_operation->cursor = 0;

            // start operation execution
            hrps_exe_operation(conidx);

            // Go to busy state
            ke_state_set(dest_id, HRPS_BUSY);
        }
        else
        {
            // Send confirmation
            hrps_send_cmp_evt(p_hrps_env, conidx, HRPS_MEAS_SEND_CMD_OP_CODE, PRF_ERR_INVALID_PARAM);
        }
    }
    else
    {
        // Save it for later
        msg_status = KE_MSG_SAVED;
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
    struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);
    struct gattc_write_cfm *p_cfm;
    uint8_t status = GAP_ERR_NO_ERROR;

    if (p_hrps_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        uint16_t value = 0x0000;

        // BP Measurement Char. - Client Char. Configuration
        if (p_param->handle == (p_hrps_env->shdl + HRS_IDX_HR_MEAS_NTF_CFG))
        {
            // Extract value before check
            memcpy(&value, &(p_param->value), sizeof(uint16_t));

            if ((value == PRF_CLI_STOP_NTFIND) || (value == PRF_CLI_START_NTF))
            {
                p_hrps_env->hr_meas_ntf[conidx] = value | HRP_PRF_CFG_PERFORMED_OK;
            }
            else
            {
                status = PRF_APP_ERROR;
            }

            if (status == GAP_ERR_NO_ERROR)
            {
                // Inform APP of configuration change
                struct hrps_cfg_indntf_ind *p_ind = KE_MSG_ALLOC(HRPS_CFG_INDNTF_IND,
                        prf_dst_task_get(&p_hrps_env->prf_env, conidx),
                        prf_src_task_get(&p_hrps_env->prf_env, conidx),
                        hrps_cfg_indntf_ind);

                p_ind->conidx = conidx;
                p_ind->cfg_val = value;

                ke_msg_send(p_ind);
            }
        }
        // HR Control Point Char. Value
        else
        {
            if (GETB(p_hrps_env->features, HRPS_ENGY_EXP_FEAT_SUP))
            {
                // Extract value
                memcpy(&value, &(p_param->value), sizeof(uint8_t));

                if (value == 0x1)
                {
                    // inform APP of configuration change
                    struct hrps_energy_exp_reset_ind *p_ind = KE_MSG_ALLOC(HRPS_ENERGY_EXP_RESET_IND,
                            prf_dst_task_get(&p_hrps_env->prf_env, conidx),
                            prf_src_task_get(&p_hrps_env->prf_env, conidx),
                            hrps_energy_exp_reset_ind);

                    p_ind->conidx = conidx;
                    ke_msg_send(p_ind);
                }
                else
                {
                    status = HRS_ERR_HR_CNTL_POINT_NOT_SUPPORTED;
                }
            }
            else
            {
                // Allowed to send Application Error if value inconvenient
                status = HRS_ERR_HR_CNTL_POINT_NOT_SUPPORTED;
            }
        }
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
    if ((ke_state_get(dest_id) == HRPS_BUSY) && (p_param->operation == GATTC_NOTIFY))
    {
        // continue operation execution
        hrps_exe_operation(KE_IDX_GET(src_id));
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
    if (ke_state_get(dest_id) == HRPS_IDLE)
    {
        // Get the address of the environment
        struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);

        uint8_t value[4];
        uint8_t value_size = 0;
        uint8_t status = ATT_ERR_NO_ERROR;
        struct gattc_read_cfm *p_cfm;

        switch (p_param->handle - p_hrps_env->shdl)
        {
            case HRS_IDX_HR_MEAS_NTF_CFG:
            {
                value_size = sizeof(uint16_t);
                co_write16p(value, p_hrps_env->hr_meas_ntf[KE_IDX_GET(src_id)] & ~HRP_PRF_CFG_PERFORMED_OK);
            } break;

            case HRS_IDX_BODY_SENSOR_LOC_VAL:
            {
                // Broadcast feature is profile specific
                if (GETB(p_hrps_env->features, HRPS_BODY_SENSOR_LOC_CHAR_SUP))
                {
                    // Fill data
                    value_size = sizeof(uint8_t);
                    value[0] = p_hrps_env->sensor_location;
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
        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, value_size);
        p_cfm->length = value_size;
        memcpy(p_cfm->value, value, value_size);
        p_cfm->handle = p_param->handle;
        p_cfm->status = status;

        // Send value to peer device.
        ke_msg_send(p_cfm);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/* Default State handlers definition. */
KE_MSG_HANDLER_TAB(hrps)
{
    {HRPS_ENABLE_REQ,        (ke_msg_func_t) hrps_enable_req_handler},
    {GATTC_WRITE_REQ_IND,    (ke_msg_func_t) gattc_write_req_ind_handler},
    {HRPS_MEAS_SEND_CMD,     (ke_msg_func_t) hrps_meas_send_cmd_handler},
    {GATTC_CMP_EVT,          (ke_msg_func_t) gattc_cmp_evt_handler},
    {GATTC_READ_REQ_IND,     (ke_msg_func_t) gattc_read_req_ind_handler},
};

void hrps_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct hrps_env_tag *p_hrps_env = PRF_ENV_GET(HRPS, hrps);

    p_task_desc->msg_handler_tab = hrps_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(hrps_msg_handler_tab);
    p_task_desc->state           = p_hrps_env->state;
    p_task_desc->idx_max         = HRPS_IDX_MAX;
}

#endif /* #if (BLE_HR_SENSOR) */

/// @} HRPSTASK
