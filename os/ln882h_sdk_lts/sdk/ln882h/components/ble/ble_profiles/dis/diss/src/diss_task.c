/**
 ****************************************************************************************
 *
 * @file diss_task.c
 *
 * @brief Device Information Service Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup DISSTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "rwprf_config.h"
#if (BLE_DIS_SERVER)
#include "co_utils.h"
#include "gap.h"
#include "gattc_task.h"
#include "diss.h"
#include "diss_task.h"
#include "prf_utils.h"

#include "ke_mem.h"

#include "utils/debug/log.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref DISS_SET_VALUE_REQ message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int diss_set_value_req_handler(ke_msg_id_t const msgid,
                                      struct diss_set_value_req const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    // Request status
    uint8_t status;
    struct diss_set_value_rsp *p_rsp;

    LOG(LOG_LVL_INFO,"diss_set_value_req_handler: value=0x%x\r\n", p_param->value);

    // Check Characteristic Code
    if (p_param->value < DIS_CHAR_MAX)
    {
        struct diss_env_tag *p_diss_env = PRF_ENV_GET(DISS, diss);

        // Characteristic Declaration attribute handle
        uint16_t handle = diss_value_to_handle(p_diss_env, p_param->value);

        // Check if the Characteristic exists in the database
        if (handle != ATT_INVALID_HDL)
        {
            // Check the value length
            status = diss_check_val_len(p_param->value, p_param->length);

            if (status == GAP_ERR_NO_ERROR)
            {
                // Check value in already present in service
                struct diss_val_elmt *p_val = (struct diss_val_elmt *) co_list_pick(&(p_diss_env->values));

                // loop until value found
                while (p_val != NULL)
                {
                    // if value already present, remove old one
                    if (p_val->value == p_param->value)
                    {
                        co_list_extract(&(p_diss_env->values), &(p_val->hdr));
                        ke_free(p_val);
                        break;
                    }

                    p_val = (struct diss_val_elmt *)p_val->hdr.next;
                }

                // allocate value data
                p_val = (struct diss_val_elmt *) ke_malloc(sizeof(struct diss_val_elmt)
                        + p_param->length, KE_MEM_ATT_DB);

                p_val->value = p_param->value;
                p_val->length = p_param->length;
                memcpy(p_val->data, p_param->data, p_param->length);
                // insert value into the list
                co_list_push_back(&(p_diss_env->values), &(p_val->hdr));
            }
        }
        else
        {
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else
    {
        status = PRF_ERR_INVALID_PARAM;
    }

    // send response to application
    p_rsp = KE_MSG_ALLOC(DISS_SET_VALUE_RSP, src_id, dest_id, diss_set_value_rsp);
    p_rsp->value = p_param->value;
    p_rsp->status = status;

    ke_msg_send(p_rsp);

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
    int msg_status = KE_MSG_CONSUMED;
    ke_state_t state = ke_state_get(dest_id);

    LOG(LOG_LVL_INFO,"gattc_read_req_ind_handler: handle=0x%x,state=%d\r\n", p_param->handle,state);

    if (state == DISS_IDLE)
    {
        struct diss_env_tag *p_diss_env = PRF_ENV_GET(DISS, diss);
        // retrieve value attribute
        uint8_t value = diss_handle_to_value(p_diss_env, p_param->handle);

        // Check Characteristic Code
        if (value < DIS_CHAR_MAX)
        {
            // Check value in already present in service
            struct diss_val_elmt *p_val = (struct diss_val_elmt *) co_list_pick(&(p_diss_env->values));

            // loop until value found
            while (p_val != NULL)
            {
                // value is present in service
                if (p_val->value == value)
                {
                    break;
                }

                p_val = (struct diss_val_elmt *)p_val->hdr.next;
            }

            if (p_val != NULL)
            {
                // Send value to peer device.
                struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id,
                        dest_id, gattc_read_cfm, p_val->length);

                p_cfm->handle = p_param->handle;
                p_cfm->status = ATT_ERR_NO_ERROR;
                p_cfm->length = p_val->length;
                memcpy(p_cfm->value, p_val->data, p_val->length);
                ke_msg_send(p_cfm);
            }
            else
            {
                struct diss_value_req_ind *p_req_ind;

                // request value to application
                p_diss_env->req_val = value;
                p_diss_env->req_conidx = KE_IDX_GET(src_id);

                p_req_ind = KE_MSG_ALLOC(DISS_VALUE_REQ_IND,
                        prf_dst_task_get(&(p_diss_env->prf_env), KE_IDX_GET(src_id)),
                        dest_id, diss_value_req_ind);

                p_req_ind->value = value;
                ke_msg_send(p_req_ind);

                // Put Service in a busy state
                ke_state_set(dest_id, DISS_BUSY);
            }
        }
        else
        {
            // application error, value cannot be retrieved
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);

            p_cfm->handle = p_param->handle;
            p_cfm->status = ATT_ERR_APP_ERROR;
            ke_msg_send(p_cfm);
        }
    }
    // postpone request if profile is in a busy state - required for multipoint
    else if (state == DISS_BUSY)
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the value confirmation from application
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int diss_value_cfm_handler(ke_msg_id_t const msgid,
                                      struct diss_value_cfm const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"diss_value_cfm_handler: handle=0x%x,length=%d\r\n", p_param->value,p_param->length);

    if (ke_state_get(dest_id) == DISS_BUSY)
    {
        struct diss_env_tag *p_diss_env = PRF_ENV_GET(DISS, diss);
        // retrieve value attribute
        uint16_t handle = diss_value_to_handle(p_diss_env, p_diss_env->req_val);

        // check if application provide correct value
        if (p_diss_env->req_val == p_param->value)
        {
            // Send value to peer device.
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                    KE_BUILD_ID(TASK_GATTC, p_diss_env->req_conidx), dest_id, gattc_read_cfm, p_param->length);

            p_cfm->handle = handle;
            p_cfm->status = ATT_ERR_NO_ERROR;
            p_cfm->length = p_param->length;
            memcpy(p_cfm->value, p_param->data, p_param->length);
            ke_msg_send(p_cfm);
        }
        else
        {
            // application error, value provided by application is not the expected one
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM,
                    KE_BUILD_ID(TASK_GATTC, p_diss_env->req_conidx), dest_id, gattc_read_cfm);

            p_cfm->handle = handle;
            p_cfm->status = ATT_ERR_APP_ERROR;
            ke_msg_send(p_cfm);
        }

        // return to idle state
        ke_state_set(dest_id, DISS_IDLE);
    }
    // else ignore request if not in busy state

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(diss)
{
    {DISS_SET_VALUE_REQ,      (ke_msg_func_t)diss_set_value_req_handler},
    {GATTC_READ_REQ_IND,      (ke_msg_func_t)gattc_read_req_ind_handler},
    {DISS_VALUE_CFM,          (ke_msg_func_t)diss_value_cfm_handler},
};

void diss_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct diss_env_tag *p_diss_env = PRF_ENV_GET(DISS, diss);

    p_task_desc->msg_handler_tab = diss_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(diss_msg_handler_tab);
    p_task_desc->state           = p_diss_env->state;
    p_task_desc->idx_max         = DISS_IDX_MAX;
}

#endif //BLE_DIS_SERVER

/// @} DISSTASK
