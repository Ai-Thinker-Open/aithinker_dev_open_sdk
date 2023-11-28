/**
 ****************************************************************************************
 *
 * @file cscpc_task.c
 *
 * @brief Cycling Speed and Cadence Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup CSCPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CSC_COLLECTOR)
#include "cscp_common.h"
#include "cscpc_task.h"
#include "cscpc.h"

#include "gap.h"
#include "gattc_task.h"

#include "ke_timer.h"
#include "ke_mem.h"
#include "co_utils.h"

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve Cycling Speed and Cadence service characteristics information
const struct prf_char_def cscpc_cscs_char[CSCP_CSCS_CHAR_MAX] =
{
    // CSC Measurement
    [CSCP_CSCS_CSC_MEAS_CHAR]      = {ATT_CHAR_CSC_MEAS,
                                      ATT_MANDATORY,
                                      ATT_CHAR_PROP_NTF},
    // CSC Feature
    [CSCP_CSCS_CSC_FEAT_CHAR]      = {ATT_CHAR_CSC_FEAT,
                                      ATT_MANDATORY,
                                      ATT_CHAR_PROP_RD},
    // Sensor Location
    [CSCP_CSCS_SENSOR_LOC_CHAR]    = {ATT_CHAR_SENSOR_LOC,
                                      ATT_OPTIONAL,
                                      ATT_CHAR_PROP_RD},
    // SC Control Point
    [CSCP_CSCS_SC_CTNL_PT_CHAR]    = {ATT_CHAR_SC_CNTL_PT,
                                      ATT_OPTIONAL,
                                      ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND},
};

/// State machine used to retrieve Cycling Speed and Cadence service characteristic descriptor information
const struct prf_char_desc_def cscpc_cscs_char_desc[CSCPC_DESC_MAX] =
{
    // CSC Measurement Char. - Client Characteristic Configuration
    [CSCPC_DESC_CSC_MEAS_CL_CFG]    = {ATT_DESC_CLIENT_CHAR_CFG,
                                       ATT_MANDATORY,
                                       CSCP_CSCS_CSC_MEAS_CHAR},

    // SC Control Point Char. - Client Characteristic Configuration
    [CSCPC_DESC_SC_CTNL_PT_CL_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG,
                                       ATT_OPTIONAL,
                                       CSCP_CSCS_SC_CTNL_PT_CHAR},
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
 * @param[in] p_param Pointer to the parameters of the message.
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
    if (ke_state_get(dest_id) == CSCPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

        ASSERT_INFO(p_cscpc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cscpc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_cscpc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve CSCS characteristics and descriptors
            prf_extract_svc_info(p_ind, CSCP_CSCS_CHAR_MAX, &cscpc_cscs_char[0],
                                      &p_cscpc_env->p_env[conidx]->cscs.chars[0],
                                      CSCPC_DESC_MAX, &cscpc_cscs_char_desc[0],
                                      &p_cscpc_env->p_env[conidx]->cscs.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_cscpc_env->p_env[conidx]->cscs.svc.shdl = p_ind->start_hdl;
            p_cscpc_env->p_env[conidx]->cscs.svc.ehdl = p_ind->end_hdl;
        }

        p_cscpc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CSCPC_ENABLE_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cscpc_enable_req_handler(ke_msg_id_t const msgid,
                                    struct cscpc_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status     = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    uint8_t state = ke_state_get(dest_id);
    // Cycling Speed and Cadence Profile Collector Role Task Environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    ASSERT_INFO(p_cscpc_env != NULL, dest_id, src_id);
    if ((state == CSCPC_IDLE) && (p_cscpc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_cscpc_env->p_env[conidx] = (struct cscpc_cnx_env *) ke_malloc(sizeof(struct cscpc_cnx_env), KE_MEM_ATT_DB);
        memset(p_cscpc_env->p_env[conidx], 0, sizeof(struct cscpc_cnx_env));

        // Start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            prf_disc_svc_send(&(p_cscpc_env->prf_env), conidx, ATT_SVC_CYCLING_SPEED_CADENCE);

            // Go to DISCOVERING state
            ke_state_set(dest_id, CSCPC_DISCOVERING);
        }
        // Bond information are provided
        else
        {
            p_cscpc_env->p_env[conidx]->cscs = p_param->cscs;

            //send APP confirmation that can start normal connection to TH
            cscpc_enable_rsp_send(p_cscpc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != CSCPC_FREE)
    {
        // The message will be forwarded towards the good task instance
        status = PRF_ERR_REQ_DISALLOWED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // The request is disallowed (profile already enabled for this connection, or not enough memory, ...)
        cscpc_enable_rsp_send(p_cscpc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CSCPC_READ_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cscpc_read_cmd_handler(ke_msg_id_t const msgid,
                                  struct cscpc_read_cmd *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == CSCPC_IDLE)
    {
        ASSERT_INFO(p_cscpc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_cscpc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;

            switch (p_param->read_code)
            {
                // Read CSC Feature
                case (CSCPC_RD_CSC_FEAT):
                {
                    handle = p_cscpc_env->p_env[conidx]->cscs.chars[CSCP_CSCS_CSC_FEAT_CHAR].val_hdl;
                } break;

                // Read Sensor Location
                case (CSCPC_RD_SENSOR_LOC):
                {
                    handle = p_cscpc_env->p_env[conidx]->cscs.chars[CSCP_CSCS_SENSOR_LOC_CHAR].val_hdl;
                } break;

                // Read CSC Measurement Characteristic Client Char. Cfg. Descriptor Value
                case (CSCPC_RD_WR_CSC_MEAS_CFG):
                {
                    handle = p_cscpc_env->p_env[conidx]->cscs.descs[CSCPC_DESC_CSC_MEAS_CL_CFG].desc_hdl;
                } break;

                // Read Unread Alert Characteristic Client Char. Cfg. Descriptor Value
                case (CSCPC_RD_WR_SC_CTNL_PT_CFG):
                {
                    handle = p_cscpc_env->p_env[conidx]->cscs.descs[CSCPC_DESC_SC_CTNL_PT_CL_CFG].desc_hdl;
                } break;

                default:
                {
                    status = PRF_ERR_INVALID_PARAM;
                } break;
            }

            // Check if handle is viable
            if (handle != ATT_INVALID_SEARCH_HANDLE)
            {
                // Force the operation value
                p_param->operation = CSCPC_READ_OP_CODE;

                // Store the command structure
                p_cscpc_env->p_env[conidx]->p_operation = p_param;
                msg_status = KE_MSG_NO_FREE;

                // Send the read request
                prf_read_char_send(&(p_cscpc_env->prf_env), conidx,
                        p_cscpc_env->p_env[conidx]->cscs.svc.shdl,
                        p_cscpc_env->p_env[conidx]->cscs.svc.ehdl,
                        handle);

                // Go to the Busy state
                ke_state_set(dest_id, CSCPC_BUSY);

                status = ATT_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }

    else if (state == CSCPC_FREE)
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
        // Send the complete event message to the task id stored in the environment
        cscpc_send_cmp_evt(p_cscpc_env, conidx, CSCPC_READ_OP_CODE, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CSCPC_CFG_NTFIND_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cscpc_cfg_ntfind_cmd_handler(ke_msg_id_t const msgid,
                                        struct cscpc_cfg_ntfind_cmd *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    // Message status
    int msg_status = KE_MSG_CONSUMED;

    if (p_cscpc_env != NULL)
    {
        // Status
        uint8_t status = PRF_ERR_REQ_DISALLOWED;
        // Handle
        uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        do
        {
            if (ke_state_get(dest_id) != CSCPC_IDLE)
            {
                // Another procedure is pending, keep the command for later
                msg_status = KE_MSG_SAVED;
                // Status is GAP_ERR_NO_ERROR, no message will be sent to the application
                break;
            }

            // state = CSCPC_CONNECTED
            ASSERT_ERR(p_cscpc_env->p_env[conidx] != NULL);

            switch(p_param->desc_code)
            {
                // Write CSC Measurement Characteristic Client Char. Cfg. Descriptor Value
                case (CSCPC_RD_WR_CSC_MEAS_CFG):
                {
                    if (p_param->ntfind_cfg <= PRF_CLI_START_NTF)
                    {
                        handle = p_cscpc_env->p_env[conidx]->cscs.descs[CSCPC_DESC_CSC_MEAS_CL_CFG].desc_hdl;
                        // The descriptor is mandatory
                        ASSERT_ERR(handle != ATT_INVALID_SEARCH_HANDLE);
                        status = GAP_ERR_NO_ERROR;
                    }
                    else
                    {
                        status = PRF_ERR_INVALID_PARAM;
                    }
                } break;

                // Write SC Control Point Characteristic Client Char. Cfg. Descriptor Value
                case (CSCPC_RD_WR_SC_CTNL_PT_CFG):
                {
                    if ((p_param->ntfind_cfg == PRF_CLI_STOP_NTFIND) ||
                        (p_param->ntfind_cfg == PRF_CLI_START_IND))
                    {
                        handle = p_cscpc_env->p_env[conidx]->cscs.descs[CSCPC_DESC_SC_CTNL_PT_CL_CFG].desc_hdl;

                        status = GAP_ERR_NO_ERROR;
                        if (handle == ATT_INVALID_SEARCH_HANDLE)
                        {
                            // The descriptor has not been found.
                            status = PRF_ERR_INEXISTENT_HDL;
                        }
                    }
                    else
                    {
                        status = PRF_ERR_INVALID_PARAM;
                    }
                } break;

                default:
                {
                    status = PRF_ERR_INVALID_PARAM;
                } break;
            }
        } while (0);

        if ((status == GAP_ERR_NO_ERROR) && (handle != ATT_INVALID_SEARCH_HANDLE))
        {
            // Set the operation code
            p_param->operation = CSCPC_CFG_NTF_IND_OP_CODE;

            // Store the command structure
            p_cscpc_env->p_env[conidx]->p_operation   = p_param;
            msg_status = KE_MSG_NO_FREE;

            // Go to the Busy state
            ke_state_set(dest_id, CSCPC_BUSY);

            // Send GATT Write Request
            prf_gatt_write_ntf_ind(&p_cscpc_env->prf_env, conidx, handle, p_param->ntfind_cfg);
        }
    }
    else
    {
        cscpc_send_no_conn_cmp_evt(dest_id, src_id, CSCPC_CFG_NTF_IND_OP_CODE);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CSCPC_CFG_NTFIND_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cscpc_ctnl_pt_cfg_cmd_handler(ke_msg_id_t const msgid,
                                         struct cscpc_ctnl_pt_cfg_cmd *p_param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    if (p_cscpc_env != NULL)
    {
        // Status
        uint8_t status = GAP_ERR_NO_ERROR;
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        do
        {
            // State is Connected or Busy
            ASSERT_ERR(ke_state_get(dest_id) > CSCPC_FREE);

            // Check the provided connection handle
            if (p_cscpc_env->p_env[conidx] == NULL)
            {
                status = PRF_ERR_INVALID_PARAM;
                break;
            }

            if (ke_state_get(dest_id) != CSCPC_IDLE)
            {
                // Another procedure is pending, keep the command for later
                msg_status = KE_MSG_NO_FREE;
                // Status is GAP_ERR_NO_ERROR, no message will be sent to the application
                break;
            }

            // Check if the characteristic has been found
            if (p_cscpc_env->p_env[conidx]->cscs.descs[CSCPC_DESC_SC_CTNL_PT_CL_CFG].desc_hdl
                    != ATT_INVALID_SEARCH_HANDLE)
            {
                // Packed request
                uint8_t req[CSCP_SC_CNTL_PT_REQ_MAX_LEN];
                // Request Length
                uint8_t req_len = CSCP_SC_CNTL_PT_REQ_MIN_LEN;

                // Set the operation code
                req[0] = p_param->sc_ctnl_pt.op_code;

                // Fulfill the message according to the operation code
                switch (p_param->sc_ctnl_pt.op_code)
                {
                    case (CSCP_CTNL_PT_OP_SET_CUMUL_VAL):
                    {
                        // Set the cumulative value
                        co_write32p(&req[req_len], p_param->sc_ctnl_pt.value.cumul_val);
                        // Update length
                        req_len += 4;
                    } break;

                    case (CSCP_CTNL_PT_OP_UPD_LOC):
                    {
                        // Set the sensor location
                        req[req_len] = p_param->sc_ctnl_pt.value.sensor_loc;
                        // Update length
                        req_len++;
                    } break;

                    case (CSCP_CTNL_PT_OP_RESERVED):
                    case (CSCP_CTNL_PT_OP_START_CALIB):
                    case (CSCP_CTNL_PT_OP_REQ_SUPP_LOC):
                    {
                        // Nothing more to do
                    } break;

                    default:
                    {
                        status = PRF_ERR_INVALID_PARAM;
                    } break;
                }

                if (status == GAP_ERR_NO_ERROR)
                {
                    // Set the operation code
                    p_param->operation = CSCPC_CTNL_PT_CFG_WR_OP_CODE;

                    // Store the command structure
                    p_cscpc_env->p_env[conidx]->p_operation = p_param;
                    // Store the command information
                    msg_status = KE_MSG_NO_FREE;

                    // Go to the Busy state
                    ke_state_set(dest_id, CSCPC_BUSY);

                    // Send the write request
                    prf_gatt_write(&(p_cscpc_env->prf_env), conidx,
                                   p_cscpc_env->p_env[conidx]->cscs.chars[CSCP_CSCS_SC_CTNL_PT_CHAR].val_hdl,
                                   (uint8_t *)&req[0], req_len, GATTC_WRITE);
                }
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        } while (0);

        if (status != GAP_ERR_NO_ERROR)
        {
            // Send a complete event status to the application
            cscpc_send_cmp_evt(p_cscpc_env, conidx, CSCPC_CTNL_PT_CFG_WR_OP_CODE, status);
        }
    }
    else
    {
        // No connection
        cscpc_send_no_conn_cmp_evt(dest_id, src_id, CSCPC_CTNL_PT_CFG_WR_OP_CODE);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CSCPC_TIMEOUT_TIMER_IND message. This message is
 * received when the peer device doesn't send a SC Control Point indication within 30s
 * after reception of the write response.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cscpc_timeout_timer_ind_handler(ke_msg_id_t const msgid,
                                           void const *p_param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    if (p_cscpc_env != NULL)
    {
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_ERR(p_cscpc_env->p_env[conidx]->p_operation != NULL);
        ASSERT_ERR(((struct cscpc_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->operation ==
                CSCPC_CTNL_PT_CFG_IND_OP_CODE);

        // Send the complete event message
        cscpc_send_cmp_evt(p_cscpc_env, conidx, CSCPC_CTNL_PT_CFG_WR_OP_CODE, PRF_ERR_PROC_TIMEOUT);
    }
    // else drop the message

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
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
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);


    if (p_cscpc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == CSCPC_DISCOVERING)
        {
            // Status
            uint8_t status = p_param->status;

            if ((status == ATT_ERR_ATTRIBUTE_NOT_FOUND) ||
                    (status == ATT_ERR_NO_ERROR))
            {
                // Discovery
                // check characteristic validity
                if(p_cscpc_env->p_env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(CSCP_CSCS_CHAR_MAX,
                            p_cscpc_env->p_env[conidx]->cscs.chars,
                            cscpc_cscs_char);
                }
                // too much services
                else if (p_cscpc_env->p_env[conidx]->nb_svc > 1)
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
                    status = prf_check_svc_char_desc_validity(CSCPC_DESC_MAX,
                            p_cscpc_env->p_env[conidx]->cscs.descs,
                            cscpc_cscs_char_desc,
                            p_cscpc_env->p_env[conidx]->cscs.chars);
                }
            }

            cscpc_enable_rsp_send(p_cscpc_env, conidx, status);
        }

        else if (state == CSCPC_BUSY)
        {
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    // Send the complete event status
                    cscpc_send_cmp_evt(p_cscpc_env, conidx, CSCPC_READ_OP_CODE, p_param->status);
                } break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    uint8_t operation = ((struct cscpc_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->operation;

                    if (operation == CSCPC_CFG_NTF_IND_OP_CODE)
                    {
                        // Send the complete event status
                        cscpc_send_cmp_evt(p_cscpc_env, conidx, operation, p_param->status);
                    }

                    else if (operation == CSCPC_CTNL_PT_CFG_WR_OP_CODE)
                    {
                        if (p_param->status == GAP_ERR_NO_ERROR)
                        {
                            // Start Timeout Procedure
                            ke_timer_set(CSCPC_TIMEOUT_TIMER_IND, dest_id, 10*ATT_TRANS_RTX);

                            // Wait for the response indication
                            ((struct cscpc_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->operation =
                                    CSCPC_CTNL_PT_CFG_IND_OP_CODE;
                        }
                        else
                        {
                            // Send the complete event status
                            cscpc_send_cmp_evt(p_cscpc_env, conidx, operation, p_param->status);
                        }
                    }
                } break;

                case GATTC_REGISTER:
                case GATTC_UNREGISTER:
                {
                    // Do nothing
                } break;

                default:
                {
                    ASSERT_ERR(0);
                } break;
            }
        }
    }
    // else ignore the message

    return (KE_MSG_CONSUMED);
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
    if (ke_state_get(dest_id) == CSCPC_BUSY)
    {
        // Get the address of the environment
        struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_cscpc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cscpc_env->p_env[conidx] != NULL, dest_id, src_id);

        // Send the read value to the HL
        struct cscpc_value_ind *p_ind = KE_MSG_ALLOC(CSCPC_VALUE_IND,
                prf_dst_task_get(&(p_cscpc_env->prf_env), conidx),
                dest_id,
                cscpc_value_ind);

        switch (((struct cscpc_read_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->read_code)
        {
            // Read CSC Feature Characteristic value
            case (CSCPC_RD_CSC_FEAT):
            {
                // Mask the reserved bits
                p_ind->value.sensor_feat = co_read16p(p_param->value);
            } break;

            // Read Sensor Location Characteristic value
            case (CSCPC_RD_SENSOR_LOC):
            {
                p_ind->value.sensor_loc = p_param->value[0];
            } break;

            // Read Client Characteristic Configuration Descriptor value
            case (CSCPC_RD_WR_CSC_MEAS_CFG):
            case (CSCPC_RD_WR_SC_CTNL_PT_CFG):
            {
                co_write16p(&p_ind->value.ntf_cfg, p_param->value[0]);
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }

        p_ind->att_code = ((struct cscpc_read_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->read_code;

        // Send the message to the application
        ke_msg_send(p_ind);
    }
    // else drop the message

    return (KE_MSG_CONSUMED);
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
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    if (p_cscpc_env != NULL)
    {
        switch (p_param->type)
        {
            case (GATTC_NOTIFY):
            {
                // Offset
                uint8_t offset = CSCP_CSC_MEAS_MIN_LEN;

                // CSC Measurement value has been received
                struct cscpc_value_ind *p_ind = KE_MSG_ALLOC(CSCPC_VALUE_IND,
                                                           prf_dst_task_get(&(p_cscpc_env->prf_env), conidx),
                                                           prf_src_task_get(&(p_cscpc_env->prf_env), conidx),
                                                           cscpc_value_ind);
                // Attribute code
                p_ind->att_code = CSCPC_NTF_CSC_MEAS;

                /*----------------------------------------------------
                 * Unpack Measurement --------------------------------
                 *----------------------------------------------------*/

                // Flags
                p_ind->value.csc_meas.flags = p_param->value[0];

                // Cumulative Wheel Revolutions
                // Last Wheel Event Time
                if (GETB(p_param->value[0], CSCP_MEAS_WHEEL_REV_DATA_PRESENT))
                {
                    // Cumulative Wheel Revolutions
                    p_ind->value.csc_meas.cumul_wheel_rev = co_read32p(&p_param->value[offset]);
                    offset += 4;

                    // Last Wheel Event Time
                    p_ind->value.csc_meas.last_wheel_evt_time = co_read16p(&p_param->value[offset]);
                    offset += 2;
                }

                // Cumulative Crank Revolutions
                // Last Crank Event Time
                if (GETB(p_param->value[0], CSCP_MEAS_CRANK_REV_DATA_PRESENT))
                {
                    // Cumulative Crank Revolutions
                    p_ind->value.csc_meas.cumul_crank_rev = co_read16p(&p_param->value[offset]);
                    offset += 2;

                    // Last Crank Event Time
                    p_ind->value.csc_meas.last_crank_evt_time = co_read16p(&p_param->value[offset]);
                }

                ke_msg_send(p_ind);
            } break;

            case (GATTC_INDICATE):
            {
                // confirm that indication has been correctly received
                struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

                p_cfm->handle = p_param->handle;
                ke_msg_send(p_cfm);

                do
                {
                    struct cscpc_ctnl_pt_ind *p_ind;

                    // Check if we were waiting for the indication
                    if (p_cscpc_env->p_env[conidx]->p_operation == NULL)
                    {
                        break;
                    }

                    if (((struct cscpc_cmd *)p_cscpc_env->p_env[conidx]->p_operation)->operation
                                != CSCPC_CTNL_PT_CFG_IND_OP_CODE)
                    {
                        break;
                    }

                    // Stop the procedure timeout timer
                    ke_timer_clear(CSCPC_TIMEOUT_TIMER_IND, dest_id);

                    // CSC Measurement value has been received
                    p_ind = KE_MSG_ALLOC(CSCPC_CTNL_PT_IND,
                                           prf_dst_task_get(&(p_cscpc_env->prf_env), conidx),
                                           prf_src_task_get(&(p_cscpc_env->prf_env), conidx),
                                           cscpc_ctnl_pt_ind);

                    p_ind->ctnl_pt_rsp.cp_opcode   = p_param->value[0];
                    // Requested operation code
                    p_ind->ctnl_pt_rsp.req_op_code = p_param->value[1];
                    // Response value
                    p_ind->ctnl_pt_rsp.resp_value  = p_param->value[2];

                    // Get the list of supported sensor locations if needed
                    if ((p_ind->ctnl_pt_rsp.req_op_code == CSCP_CTNL_PT_OP_REQ_SUPP_LOC) &&
                        (p_ind->ctnl_pt_rsp.resp_value == CSCP_CTNL_PT_RESP_SUCCESS)     &&
                        (p_param->length > 3))
                    {
                        // Get the number of supported locations that have been received
                        uint8_t nb_supp_loc = (p_param->length - 3);
                        // Counter
                        uint8_t counter;
                        // Location
                        uint8_t loc;

                        for (counter = 0; counter < nb_supp_loc; counter++)
                        {
                            loc = p_param->value[counter + 3];

                            // Check if valid
                            if (loc < CSCP_LOC_MAX)
                            {
                                p_ind->ctnl_pt_rsp.supp_loc |= (1 << loc);
                            }
                        }
                    }

                    // Send the message
                    ke_msg_send(p_ind);

                    // Send the complete event message
                    cscpc_send_cmp_evt(p_cscpc_env, conidx, CSCPC_CTNL_PT_CFG_WR_OP_CODE, GAP_ERR_NO_ERROR);

                } while (0);
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(cscpc)
{
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {CSCPC_ENABLE_REQ,              (ke_msg_func_t)cscpc_enable_req_handler},
    {CSCPC_READ_CMD,                (ke_msg_func_t)cscpc_read_cmd_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {CSCPC_CFG_NTFIND_CMD,          (ke_msg_func_t)cscpc_cfg_ntfind_cmd_handler},
    {CSCPC_CTNL_PT_CFG_CMD,         (ke_msg_func_t)cscpc_ctnl_pt_cfg_cmd_handler},
    {CSCPC_TIMEOUT_TIMER_IND,       (ke_msg_func_t)cscpc_timeout_timer_ind_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,           (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
};

void cscpc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct cscpc_env_tag *p_cscpc_env = PRF_ENV_GET(CSCPC, cscpc);

    p_task_desc->msg_handler_tab = cscpc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(cscpc_msg_handler_tab);
    p_task_desc->state           = p_cscpc_env->state;
    p_task_desc->idx_max         = CSCPC_IDX_MAX;
}

#endif //(BLE_CSC_COLLECTOR)

/// @} CSCPCTASK
