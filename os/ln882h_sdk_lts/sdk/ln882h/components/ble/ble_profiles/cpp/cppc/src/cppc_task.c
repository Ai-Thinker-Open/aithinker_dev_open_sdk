/**
 ****************************************************************************************
 *
 * @file cppc_task.c
 *
 * @brief Cycling Power Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup CPPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CP_COLLECTOR)
#include "cpp_common.h"
#include "cppc_task.h"
#include "cppc.h"

#include "gap.h"
#include "gattc_task.h"

#include "ke_timer.h"
#include "ke_mem.h"

#include "co_utils.h"

/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve Cycling Power service characteristics information
const struct prf_char_def cppc_cps_char[CPP_CPS_CHAR_MAX] =
{
    /// CP Measurement
    [CPP_CPS_MEAS_CHAR]         = {ATT_CHAR_CP_MEAS,
                                    ATT_MANDATORY,
                                    ATT_CHAR_PROP_NTF},
    /// CP Feature
    [CPP_CPS_FEAT_CHAR]         = {ATT_CHAR_CP_FEAT,
                                    ATT_MANDATORY,
                                    ATT_CHAR_PROP_RD},
    /// Sensor Location
    [CPP_CPS_SENSOR_LOC_CHAR]   = {ATT_CHAR_SENSOR_LOC,
                                    ATT_MANDATORY,
                                    ATT_CHAR_PROP_RD},
    /// Vector
    [CPP_CPS_VECTOR_CHAR]       = {ATT_CHAR_CP_VECTOR,
                                    ATT_OPTIONAL,
                                    ATT_CHAR_PROP_NTF},
    /// CP Control Point
    [CPP_CPS_CTNL_PT_CHAR]      = {ATT_CHAR_CP_CNTL_PT,
                                    ATT_OPTIONAL,
                                    ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND},
};

/// State machine used to retrieve Cycling Power service characteristic descriptor information
const struct prf_char_desc_def cppc_cps_char_desc[CPPC_DESC_MAX] =
{
    /// CP Measurement Char. - Client Characteristic Configuration
    [CPPC_DESC_CP_MEAS_CL_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,
                                        ATT_MANDATORY,
                                        CPP_CPS_MEAS_CHAR},

    /// CP Measurement Char. - Server Characteristic Configuration
    [CPPC_DESC_CP_MEAS_SV_CFG] = {ATT_DESC_SERVER_CHAR_CFG,
                                        ATT_OPTIONAL,
                                        CPP_CPS_MEAS_CHAR},

    /// CP Vector Char. - Client Characteristic Configuration
    [CPPC_DESC_VECTOR_CL_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG,
                                        ATT_OPTIONAL,
                                        CPP_CPS_VECTOR_CHAR},

    /// Control Point Char. - Client Characteristic Configuration
    [CPPC_DESC_CTNL_PT_CL_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,
                                        ATT_OPTIONAL,
                                        CPP_CPS_CTNL_PT_CHAR},
};

/*
 * HANDLER FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CPPC_ENABLE_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cppc_enable_req_handler(ke_msg_id_t const msgid,
                                    struct cppc_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    uint8_t state = ke_state_get(dest_id);
    // Cycling Power Profile Collector Role Task Environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    ASSERT_INFO(p_cppc_env != NULL, dest_id, src_id);
    if ((state == CPPC_IDLE) && (p_cppc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_cppc_env->p_env[conidx] = (struct cppc_cnx_env *) ke_malloc(sizeof(struct cppc_cnx_env), KE_MEM_ATT_DB);
        memset(p_cppc_env->p_env[conidx], 0, sizeof(struct cppc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering CPS on peer
            prf_disc_svc_send(&(p_cppc_env->prf_env), conidx, ATT_SVC_CYCLING_POWER);

            // Go to DISCOVERING state
            ke_state_set(dest_id, CPPC_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_cppc_env->p_env[conidx]->cps = p_param->cps;

            //send APP confirmation that can start normal connection to TH
            cppc_enable_rsp_send(p_cppc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != CPPC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        cppc_enable_rsp_send(p_cppc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CPPC_READ_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cppc_read_req_handler(ke_msg_id_t const msgid,
                                  struct cpps_read_cmd *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == CPPC_IDLE)
    {
        ASSERT_INFO(p_cppc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_cppc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Get the handler
            uint16_t hdl = cppc_get_read_handle_req (p_cppc_env, conidx, p_param);

            // Check if handle is viable
            if (hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                // Force the operation value
                p_param->operation = CPPC_READ_OP_CODE;

                // Store the command structure
                p_cppc_env->p_env[conidx]->p_operation = p_param;
                msg_status = KE_MSG_NO_FREE;

                // Send the read request
                prf_read_char_send( &(p_cppc_env->prf_env), conidx,
                        p_cppc_env->p_env[conidx]->cps.svc.shdl,
                        p_cppc_env->p_env[conidx]->cps.svc.ehdl,
                        hdl);

                // Go to the Busy state
                ke_state_set(dest_id, CPPC_BUSY);

                status = ATT_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CPPC_FREE)
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
        cppc_send_cmp_evt(p_cppc_env, conidx, CPPC_READ_OP_CODE, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CPPC_CFG_NTFIND_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cppc_cfg_ntfind_cmd_handler(ke_msg_id_t const msgid,
                                        struct cppc_cfg_ntfind_cmd *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    // Message status
    int msg_status = KE_MSG_CONSUMED;

    if (p_cppc_env != NULL)
    {
        // Status
        uint8_t status = PRF_ERR_REQ_DISALLOWED;
        // Handle
        uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        do
        {
            if (ke_state_get(dest_id) != CPPC_IDLE)
            {
                // Another procedure is pending, keep the command for later
                msg_status = KE_MSG_SAVED;
                break;
            }

            ASSERT_ERR(p_cppc_env->p_env[conidx] != NULL);

            // Get handle
            status = cppc_get_write_desc_handle_req (conidx, p_param, p_cppc_env, &handle);
        } while (0);

        if ((status == GAP_ERR_NO_ERROR) && (handle != ATT_INVALID_SEARCH_HANDLE))
        {
            // Set the operation code
            p_param->operation = CPPC_CFG_NTF_IND_OP_CODE;

            // Store the command structure
            p_cppc_env->p_env[conidx]->p_operation = p_param;
            msg_status = KE_MSG_NO_FREE;

            // Go to the Busy state
            ke_state_set(dest_id, CPPC_BUSY);

            // Send GATT Write Request
            prf_gatt_write_ntf_ind(&p_cppc_env->prf_env, conidx, handle, p_param->ntfind_cfg);
        }
    }
    else
    {
        cppc_send_no_conn_cmp_evt(dest_id, src_id, CPPC_CFG_NTF_IND_OP_CODE);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CPPC_CTNL_PT_CFG_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cppc_ctnl_pt_cfg_cmd_handler(ke_msg_id_t const msgid,
                                         struct cppc_ctnl_pt_cfg_cmd *p_param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    if (p_cppc_env != NULL)
    {
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Status
        uint8_t status = GAP_ERR_NO_ERROR;

        do
        {
            // State is Connected or Busy
            ASSERT_ERR(ke_state_get(dest_id) > CPPC_FREE);
            // Check the provided connection handle
            if (p_cppc_env->p_env[conidx] == NULL)
            {
                status = PRF_ERR_INVALID_PARAM;
                break;
            }

            if (ke_state_get(dest_id) != CPPC_IDLE)
            {
                // Another procedure is pending, keep the command for later
                msg_status = KE_MSG_SAVED;
                // Status is GAP_ERR_NO_ERROR, no message will be sent to the application
                break;
            }

            // Check if the characteristic has been found
            if (p_cppc_env->p_env[conidx]->cps.descs[CPPC_DESC_CTNL_PT_CL_CFG].desc_hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                // Request array declaration
                uint8_t req[CPP_CP_CNTL_PT_REQ_MAX_LEN];
                // Pack request
                uint8_t nb = cppc_pack_ctnl_pt_req(p_param, req, &status);

                if (status == GAP_ERR_NO_ERROR)
                {
                    // Set the operation code
                    p_param->operation = CPPC_CTNL_PT_CFG_WR_OP_CODE;

                    // Store the command structure
                    p_cppc_env->p_env[conidx]->p_operation   = p_param;
                    // Store the command information
                    msg_status = KE_MSG_NO_FREE;

                    // Go to the Busy state
                    ke_state_set(dest_id, CPPC_BUSY);

                    // Send the write request
                    prf_gatt_write(&(p_cppc_env->prf_env), conidx,
                                    p_cppc_env->p_env[conidx]->cps.chars[CPP_CPS_CTNL_PT_CHAR].val_hdl,
                                    (uint8_t *)&req[0], nb, GATTC_WRITE);
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
            cppc_send_cmp_evt(p_cppc_env, conidx, CPPC_CTNL_PT_CFG_WR_OP_CODE, status);
        }
    }
    else
    {
        // No connection
        cppc_send_no_conn_cmp_evt(dest_id, src_id, CPPC_CTNL_PT_CFG_WR_OP_CODE);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref CPPC_TIMEOUT_TIMER_IND message. This message is
 * received when the peer device doesn't send a SC Control Point indication within 30s
 * after reception of the write response.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cppc_timeout_timer_ind_handler(ke_msg_id_t const msgid,
                                           void const *p_param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (p_cppc_env != NULL)
    {
        ASSERT_ERR(p_cppc_env->p_env[conidx]->p_operation != NULL);
        ASSERT_ERR(((struct cppc_cmd *)p_cppc_env->p_env[conidx]->p_operation)->operation
                == CPPC_CTNL_PT_CFG_IND_OP_CODE);

        // Send the complete event message
        cppc_send_cmp_evt(p_cppc_env, conidx, CPPC_CTNL_PT_CFG_WR_OP_CODE, PRF_ERR_PROC_TIMEOUT);
    }
    // else drop the message

    return (KE_MSG_CONSUMED);
}

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
    if (ke_state_get(dest_id) == CPPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

        ASSERT_INFO(p_cppc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cppc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_cppc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve CPS characteristics and descriptors
            prf_extract_svc_info(p_ind, CPP_CPS_CHAR_MAX, &cppc_cps_char[0],  &p_cppc_env->p_env[conidx]->cps.chars[0],
                                      CPPC_DESC_MAX, &cppc_cps_char_desc[0], &p_cppc_env->p_env[conidx]->cps.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_cppc_env->p_env[conidx]->cps.svc.shdl = p_ind->start_hdl;
            p_cppc_env->p_env[conidx]->cps.svc.ehdl = p_ind->end_hdl;
        }

        p_cppc_env->p_env[conidx]->nb_svc++;
    }

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
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    if (p_cppc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == CPPC_DISCOVERING)
        {
            // Status
            uint8_t status = p_param->status;

            if ((status == ATT_ERR_ATTRIBUTE_NOT_FOUND) ||
                    (status == ATT_ERR_NO_ERROR))
            {
                // Discovery
                // check characteristic validity
                if (p_cppc_env->p_env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(CPP_CPS_CHAR_MAX,
                            p_cppc_env->p_env[conidx]->cps.chars,
                            cppc_cps_char);
                }
                // too much services
                else if (p_cppc_env->p_env[conidx]->nb_svc > 1)
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
                    status = prf_check_svc_char_desc_validity(CPPC_DESC_MAX,
                            p_cppc_env->p_env[conidx]->cps.descs,
                            cppc_cps_char_desc,
                            p_cppc_env->p_env[conidx]->cps.chars);
                }
            }

            cppc_enable_rsp_send(p_cppc_env, conidx, status);
        }
        else if (state == CPPC_BUSY)
        {
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    // Send the complete event status
                    cppc_send_cmp_evt(p_cppc_env, conidx, CPPC_READ_OP_CODE, p_param->status);
                } break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    uint8_t operation = ((struct cppc_cmd *)p_cppc_env->p_env[conidx]->p_operation)->operation;

                    if (operation == CPPC_CFG_NTF_IND_OP_CODE)
                    {
                        // Send the complete event status
                        cppc_send_cmp_evt(p_cppc_env, conidx, operation, p_param->status);
                    }

                    else if (operation == CPPC_CTNL_PT_CFG_WR_OP_CODE)
                    {
                        if (p_param->status == GAP_ERR_NO_ERROR)
                        {
                            // Start Timeout Procedure
                            ke_timer_set(CPPC_TIMEOUT_TIMER_IND, dest_id, 10*ATT_TRANS_RTX);

                            // Wait for the response indication
                            ((struct cppc_cmd *)p_cppc_env->p_env[conidx]->p_operation)->operation
                                    = CPPC_CTNL_PT_CFG_IND_OP_CODE;
                        }
                        else
                        {
                            // Send the complete event status
                            cppc_send_cmp_evt(p_cppc_env, conidx, operation, p_param->status);
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
    if (ke_state_get(dest_id) == CPPC_BUSY)
    {
        // Get the address of the environment
        struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_cppc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cppc_env->p_env[conidx] != NULL, dest_id, src_id);

        // Send the read value to the HL
        struct cppc_value_ind *p_ind = KE_MSG_ALLOC(
                CPPC_VALUE_IND,
                prf_dst_task_get(&(p_cppc_env->prf_env), conidx),
                dest_id,
                cppc_value_ind);

        switch (((struct cpps_read_cmd *)p_cppc_env->p_env[conidx]->p_operation)->read_code)
        {
            // Read CP Feature Characteristic value
            case (CPPC_RD_CP_FEAT):
            {
                p_ind->value.sensor_feat = co_read32p(&p_param->value[0]);
            } break;

            // Read Sensor Location Characteristic value
            case (CPPC_RD_SENSOR_LOC):
            {
                p_ind->value.sensor_loc = p_param->value[0];
            } break;

            // Read Client Characteristic Configuration Descriptor value
            case (CPPC_RD_WR_CP_MEAS_CL_CFG):
            case (CPPC_RD_WR_CP_MEAS_SV_CFG):
            case (CPPC_RD_WR_VECTOR_CFG):
            case (CPPC_RD_WR_CTNL_PT_CFG):
            {
                co_write16p(&p_ind->value.ntf_cfg, p_param->value[0]);
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }

        p_ind->att_code = ((struct cpps_read_cmd *)p_cppc_env->p_env[conidx]->p_operation)->read_code;

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
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    if (p_cppc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        switch (p_param->type)
        {
            case (GATTC_NOTIFY):
            {
                if (p_param->handle == p_cppc_env->p_env[conidx]->cps.chars[CPP_CPS_MEAS_CHAR].val_hdl)
                {
                    //Unpack measurement
                    cppc_unpack_meas_ind (conidx, p_param, p_cppc_env);
                }
                else if (p_param->handle == p_cppc_env->p_env[conidx]->cps.chars[CPP_CPS_VECTOR_CHAR].val_hdl)
                {
                    //Unpack vector
                    cppc_unpack_vector_ind (conidx, p_param, p_cppc_env);
                }
                else
                {
                    ASSERT_ERR(0);
                }
            } break;

            case (GATTC_INDICATE):
            {
                // confirm that indication has been correctly received
                struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

                p_cfm->handle = p_param->handle;
                ke_msg_send(p_cfm);

                // Check if we were waiting for the indication
                if (p_cppc_env->p_env[conidx]->p_operation != NULL)
                {
                    if (((struct cppc_cmd *)p_cppc_env->p_env[conidx]->p_operation)->operation
                            == CPPC_CTNL_PT_CFG_IND_OP_CODE)
                    {
                        // Stop the procedure timeout timer
                        ke_timer_clear(CPPC_TIMEOUT_TIMER_IND, dest_id);

                        // Unpack control point
                        cppc_unpack_ctln_pt_ind(conidx, p_param, p_cppc_env);

                        // Send the complete event message
                        cppc_send_cmp_evt(p_cppc_env, conidx, CPPC_CTNL_PT_CFG_WR_OP_CODE, GAP_ERR_NO_ERROR);
                    }
                    // else drop the message
                }
                // else drop the message
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
KE_MSG_HANDLER_TAB(cppc)
{
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {CPPC_READ_CMD,                 (ke_msg_func_t)cppc_read_req_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {CPPC_CFG_NTFIND_CMD,           (ke_msg_func_t)cppc_cfg_ntfind_cmd_handler},
    {CPPC_CTNL_PT_CFG_CMD,          (ke_msg_func_t)cppc_ctnl_pt_cfg_cmd_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,           (ke_msg_func_t)gattc_event_ind_handler},
    {CPPC_TIMEOUT_TIMER_IND,        (ke_msg_func_t)cppc_timeout_timer_ind_handler},
    {CPPC_ENABLE_REQ,               (ke_msg_func_t)cppc_enable_req_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
};

void cppc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct cppc_env_tag *p_cppc_env = PRF_ENV_GET(CPPC, cppc);

    p_task_desc->msg_handler_tab = cppc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(cppc_msg_handler_tab);
    p_task_desc->state           = p_cppc_env->state;
    p_task_desc->idx_max         = CPPC_IDX_MAX;
}

#endif //(BLE_CP_COLLECTOR)

/// @} CPPCTASK
