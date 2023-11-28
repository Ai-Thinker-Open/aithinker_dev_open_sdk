/**
 ****************************************************************************************
 *
 * @file hrpc_task.c
 *
 * @brief Heart Rate Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HRPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"
#include "hrp_common.h"

#if (BLE_HR_COLLECTOR)
#include "co_utils.h"
#include "gap.h"
#include "attm.h"
#include "hrpc_task.h"
#include "hrpc.h"
#include "gattc_task.h"

#include "ke_mem.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// State machine used to retrieve heart rate service characteristics information
const struct prf_char_def hrpc_hrs_char[HRPC_CHAR_MAX] =
{
    // Heart Rate Measurement
    [HRPC_CHAR_HR_MEAS]              = {ATT_CHAR_HEART_RATE_MEAS,
                                        ATT_MANDATORY,
                                        ATT_CHAR_PROP_NTF},
    // Body Sensor Location
    [HRPC_CHAR_BODY_SENSOR_LOCATION] = {ATT_CHAR_BODY_SENSOR_LOCATION,
                                        ATT_OPTIONAL,
                                        ATT_CHAR_PROP_RD},
    // Heart Rate Control Point
    [HRPC_CHAR_HR_CNTL_POINT]        = {ATT_CHAR_HEART_RATE_CNTL_POINT,
                                        ATT_OPTIONAL,
                                        ATT_CHAR_PROP_WR},
};

/// State machine used to retrieve heart rate service characteristic description information
const struct prf_char_desc_def hrpc_hrs_char_desc[HRPC_DESC_MAX] =
{
    // Heart Rate Measurement client config
    [HRPC_DESC_HR_MEAS_CLI_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, HRPC_CHAR_HR_MEAS},
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_ind Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_sdp_svc_ind const *p_ind,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == HRPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);

        ASSERT_INFO(p_hrpc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_hrpc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_hrpc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve HRS characteristics and descriptors
            prf_extract_svc_info(p_ind, HRPC_CHAR_MAX, &hrpc_hrs_char[0], &p_hrpc_env->p_env[conidx]->hrs.chars[0],
                                      HRPC_DESC_MAX, &hrpc_hrs_char_desc[0], &p_hrpc_env->p_env[conidx]->hrs.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_hrpc_env->p_env[conidx]->hrs.svc.shdl = p_ind->start_hdl;
            p_hrpc_env->p_env[conidx]->hrs.svc.ehdl = p_ind->end_hdl;
        }

        p_hrpc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HRPC_ENABLE_REQ message.
 * The handler enables the Heart Rate Profile Collector Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hrpc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct hrpc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Heart Rate Profile Client Role Task Environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);;

    ASSERT_INFO(p_hrpc_env != NULL, dest_id, src_id);
    // Config connection, start discovering
    if ((ke_state_get(dest_id) == HRPC_IDLE) && (p_hrpc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_hrpc_env->p_env[conidx] = (struct hrpc_cnx_env *) ke_malloc(sizeof(struct hrpc_cnx_env), KE_MEM_ATT_DB);
        memset(p_hrpc_env->p_env[conidx], 0, sizeof(struct hrpc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // start discovering HRS on peer
            prf_disc_svc_send(&(p_hrpc_env->prf_env), conidx, ATT_SVC_HEART_RATE);

            p_hrpc_env->p_env[conidx]->last_uuid_req = ATT_SVC_HEART_RATE;

            // Go to DISCOVERING state
            ke_state_set(dest_id, HRPC_DISCOVERING);
        }
        //normal connection, get saved att details
        else
        {
            p_hrpc_env->p_env[conidx]->hrs = p_param->hrs;
            //send APP confirmation that can start normal connection to TH
            hrpc_enable_rsp_send(p_hrpc_env, conidx, GAP_ERR_NO_ERROR);
        }

        status = GAP_ERR_NO_ERROR;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        hrpc_enable_rsp_send(p_hrpc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 * This generic event is received for different requests, so need to keep track.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);

    if (p_hrpc_env != NULL)
    {
        // Status
        uint8_t status = p_param->status;
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == HRPC_DISCOVERING)
        {
            if ((status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||
                (status == ATT_ERR_NO_ERROR))
            {
                // Discovery
                // check characteristic validity
                if (p_hrpc_env->p_env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(HRPC_CHAR_MAX,
                            p_hrpc_env->p_env[conidx]->hrs.chars,
                            hrpc_hrs_char);
                }
                // too much services
                else if (p_hrpc_env->p_env[conidx]->nb_svc > 1)
                {
                    status = PRF_ERR_MULTIPLE_SVC;
                }
                // no services found
                else
                {
                    status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                }

                // check descriptor validity
                if (status == GAP_ERR_NO_ERROR)
                {
                    status = prf_check_svc_char_desc_validity(HRPC_DESC_MAX,
                            p_hrpc_env->p_env[conidx]->hrs.descs,
                            hrpc_hrs_char_desc,
                            p_hrpc_env->p_env[conidx]->hrs.chars);
                }
            }

            hrpc_enable_rsp_send(p_hrpc_env, conidx, status);
        }

        else if (state == HRPC_BUSY)
        {
            switch (p_param->operation)
            {
                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    uint8_t operation;

                    if (p_hrpc_env->p_env[conidx]->last_char_code == HRPC_DESC_HR_MEAS_CLI_CFG)
                    {
                        operation = HRPC_CFG_INDNTF_CMD_OP_CODE;
                    }
                    else
                    {
                        operation = HRPC_WR_CNTL_POINT_CMD_OP_CODE;
                    }

                    hrpc_send_cmp_evt(p_hrpc_env, conidx, operation, status);
                } break;

                case GATTC_READ:
                {
                    hrpc_send_cmp_evt(p_hrpc_env, conidx, HRPC_RD_CHAR_CMD_OP_CODE, status);
                } break;

                default:
                {

                } break;
            }

            ke_state_set(prf_src_task_get(&p_hrpc_env->prf_env, conidx), HRPC_IDLE);
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HRPC_RD_CHAR_CMD message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hrpc_rd_char_cmd_handler(ke_msg_id_t const msgid,
                                        struct hrpc_rd_char_cmd const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == HRPC_IDLE)
    {
        ASSERT_INFO(p_hrpc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_hrpc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;

            if (((p_param->char_code & HRPC_DESC_MASK) == HRPC_DESC_MASK) &&
               ((p_param->char_code & ~HRPC_DESC_MASK) < HRPC_DESC_MAX))
            {
                search_hdl = p_hrpc_env->p_env[conidx]->hrs.descs[p_param->char_code & ~HRPC_DESC_MASK].desc_hdl;
            }
            else if (p_param->char_code < HRPC_CHAR_MAX)
            {
                search_hdl = p_hrpc_env->p_env[conidx]->hrs.chars[p_param->char_code].val_hdl;
            }

            // check if handle is viable
            if (search_hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                // Store the command
                p_hrpc_env->p_env[conidx]->last_char_code = p_param->char_code;
                // Send the read request
                prf_read_char_send(&(p_hrpc_env->prf_env), conidx,
                        p_hrpc_env->p_env[conidx]->hrs.svc.shdl,
                        p_hrpc_env->p_env[conidx]->hrs.svc.ehdl,
                        search_hdl);

                // Go to the Busy state
                ke_state_set(dest_id, HRPC_BUSY);

                status = ATT_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == HRPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        hrpc_send_cmp_evt(p_hrpc_env, conidx, HRPC_RD_CHAR_CMD_OP_CODE, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_READ_IND message.
 * Generic event received after every simple read command sent to peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_ind_handler(ke_msg_id_t const msgid,
                                    struct gattc_read_ind const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (p_hrpc_env->p_env[conidx] != NULL)
    {
        ASSERT_ERR(ke_state_get(dest_id) == HRPC_BUSY);

        prf_client_att_info_rsp(&p_hrpc_env->prf_env, conidx, HRPC_RD_CHAR_IND,
                GAP_ERR_NO_ERROR, p_param);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HRPC_CFG_INDNTF_CMD message.
 * It allows configuration of the peer ind/ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hrpc_cfg_indntf_cmd_handler(ke_msg_id_t const msgid,
                                       struct hrpc_cfg_indntf_cmd const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == HRPC_IDLE)
    {
        if (p_hrpc_env->p_env[conidx] != NULL)
        {
            // Status
            status = PRF_ERR_INVALID_PARAM;
            // Check if p_parameter is OK
            if ((p_param->cfg_val == PRF_CLI_STOP_NTFIND) ||
                    (p_param->cfg_val == PRF_CLI_START_NTF))
            {
                // Get handle of the client configuration
                uint16_t cfg_hdl =
                        p_hrpc_env->p_env[conidx]->hrs.descs[HRPC_DESC_HR_MEAS_CLI_CFG].desc_hdl;

                // Status
                status = PRF_ERR_INEXISTENT_HDL;
                //check if the handle value exists
                if (cfg_hdl != ATT_INVALID_SEARCH_HANDLE)
                {
                    p_hrpc_env->p_env[conidx]->last_char_code = HRPC_DESC_HR_MEAS_CLI_CFG;

                    status = GAP_ERR_NO_ERROR;
                    // Go to the Busy state
                    ke_state_set(dest_id, HRPC_BUSY);
                    // Send GATT Write Request
                    prf_gatt_write_ntf_ind(&p_hrpc_env->prf_env, conidx, cfg_hdl, p_param->cfg_val);
                }
            }
        }
        else
        {
            status = PRF_APP_ERROR;
        }
    }
    else if (state == HRPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        hrpc_send_cmp_evt(p_hrpc_env, conidx, HRPC_CFG_INDNTF_CMD_OP_CODE, status);
    }

    return (msg_status);
}

/**
****************************************************************************************
* @brief Handles reception of the @ref HRPC_WR_CNTL_POINT_CMD message.
* Check if the handle exists in profile(already discovered) and send request, otherwise
* error to APP.
* @param[in] msgid Id of the message received (probably unused).
* @param[in] p_param Pointer to the parameters of the message.
* @param[in] dest_id ID of the receiving task instance (probably unused).
* @param[in] src_id ID of the sending task instance.
* @return If the message was consumed or not.
****************************************************************************************
*/
__STATIC int hrpc_wr_cntl_point_cmd_handler(ke_msg_id_t const msgid,
                                       struct hrpc_wr_cntl_point_cmd const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    uint8_t msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == HRPC_IDLE)
    {
        do
        {
            // this is mandatory readable if it is included in the peer's DB
            if (p_hrpc_env->p_env[conidx] == NULL)
            {
                status = PRF_APP_ERROR;

                break;
            }

            // this is mandatory readable if it is included in the peer's DB
            if (p_hrpc_env->p_env[conidx]->hrs.chars[HRPC_CHAR_HR_CNTL_POINT].char_hdl == ATT_INVALID_SEARCH_HANDLE)
            {
                // send app error indication for inexistent handle for this characteristic
                status = PRF_ERR_INEXISTENT_HDL;

                break;
            }

            if ((p_hrpc_env->p_env[conidx]->hrs.chars[HRPC_CHAR_HR_CNTL_POINT].prop & ATT_CHAR_PROP_WR)
                    != ATT_CHAR_PROP_WR)
            {
                //write not allowed, so no point in continuing
                status = PRF_ERR_NOT_WRITABLE;

                break;
            }

            p_hrpc_env->p_env[conidx]->last_char_code = HRPC_CHAR_HR_CNTL_POINT;

            // Send GATT Write Request
            prf_gatt_write(&p_hrpc_env->prf_env,
                    conidx, p_hrpc_env->p_env[conidx]->hrs.chars[HRPC_CHAR_HR_CNTL_POINT].val_hdl,
                    (uint8_t *)&p_param->val, sizeof(uint8_t), GATTC_WRITE);

            // Go to the Busy state
            ke_state_set(dest_id, HRPC_BUSY);

        } while (0);
    }
    else if (state == HRPC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        hrpc_send_cmp_evt(p_hrpc_env, conidx, HRPC_WR_CNTL_POINT_CMD_OP_CODE, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_EVENT_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                        struct gattc_event_ind const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);

    if (p_hrpc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        if ((p_param->handle == p_hrpc_env->p_env[conidx]->hrs.chars[HRPC_CHAR_HR_MEAS].val_hdl) &&
                (p_param->type == GATTC_NOTIFY))
        {
            // build a HRPC_HR_MEAS_IND message with stable heart rate value code.
            struct hrpc_hr_meas_ind *p_ind = KE_MSG_ALLOC(
                    HRPC_HR_MEAS_IND,
                    prf_dst_task_get(&(p_hrpc_env->prf_env), conidx),
                    prf_src_task_get(&(p_hrpc_env->prf_env), conidx),
                    hrpc_hr_meas_ind);

            // unpack heart rate measurement.
            hrpc_unpack_meas_value(&(p_ind->meas_val), (uint8_t *) p_param->value, p_param->length);

            ke_msg_send(p_ind);
        }
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(hrpc)
{
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {HRPC_ENABLE_REQ,               (ke_msg_func_t)hrpc_enable_req_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {HRPC_CFG_INDNTF_CMD,           (ke_msg_func_t)hrpc_cfg_indntf_cmd_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {HRPC_RD_CHAR_CMD,              (ke_msg_func_t)hrpc_rd_char_cmd_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {HRPC_WR_CNTL_POINT_CMD,        (ke_msg_func_t)hrpc_wr_cntl_point_cmd_handler},
};

void hrpc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct hrpc_env_tag *p_hrpc_env = PRF_ENV_GET(HRPC, hrpc);

    p_task_desc->msg_handler_tab = hrpc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(hrpc_msg_handler_tab);
    p_task_desc->state           = p_hrpc_env->state;
    p_task_desc->idx_max         = HRPC_IDX_MAX;
}

#endif /* (BLE_HR_COLLECTOR) */
/// @} HRPCTASK
