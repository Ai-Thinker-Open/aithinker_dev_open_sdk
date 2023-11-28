/**
 ****************************************************************************************
 *
 * @file plxc_task.c
 *
 * @brief Pulse Oximeter Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup PLXCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_PLX_CLIENT)
#include "plxp_common.h"
#include "plxc_task.h"
#include "plxc.h"

#include "gap.h"
#include "attm.h"
#include "gattc_task.h"
#include "ke_mem.h"
#include "co_utils.h"


/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve Pulse Oximeter Service characteristics information
const struct prf_char_def plxc_plx_char[PLXC_CHAR_MAX] =
{
    [PLXC_CHAR_SPOT_MEASUREMENT] = {ATT_CHAR_PLX_SPOT_CHECK_MEASUREMENT_LOC,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_IND) },

    [PLXC_CHAR_CONT_MEASUREMENT] = {ATT_CHAR_PLX_CONTINUOUS_MEASUREMENT_LOC,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_NTF) },

    [PLXC_CHAR_FEATURES]         = {ATT_CHAR_PLX_FEATURES_LOC,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_RD) },

    [PLXC_CHAR_RACP]             = {ATT_CHAR_REC_ACCESS_CTRL_PT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND) },
};


/// State machine used to retrieve Pulse Oximeter Service characteristic Description information
const struct prf_char_desc_def plxc_plx_char_desc[PLXC_DESC_MAX] =
{
    /// Client config
    [PLXC_DESC_SPOT_MEASUREMENT_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_OPTIONAL,
                                         PLXC_CHAR_SPOT_MEASUREMENT},
    /// Client config
    [PLXC_DESC_CONT_MEASUREMENT_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_OPTIONAL,
                                         PLXC_CHAR_CONT_MEASUREMENT},
    /// Client config
    [PLXC_DESC_RACP_CCC]             = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_OPTIONAL,
                                         PLXC_CHAR_RACP},
};


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_sdp_svc_ind const *p_ind,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == PLXC_DISCOVERING_SVC)
    {
        // Pulse Oximeter Collector Role Task Environment
        struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_plxc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_PULSE_OXIMETER))
        {
            // Retrieve PLX characteristics and descriptors
            prf_extract_svc_info(p_ind, PLXC_CHAR_MAX, &plxc_plx_char[0], &p_plxc_env->p_env[conidx]->plxc.chars[0],
                    PLXC_DESC_MAX, &plxc_plx_char_desc[0], &p_plxc_env->p_env[conidx]->plxc.descs[0]);

            // Even if we get multiple responses we only store 1 range
            p_plxc_env->p_env[conidx]->plxc.svc.shdl = p_ind->start_hdl;
            p_plxc_env->p_env[conidx]->plxc.svc.ehdl = p_ind->end_hdl;

            p_plxc_env->p_env[conidx]->nb_svc++;
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see PLXC_ENABLE_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int plxc_enable_req_handler(ke_msg_id_t const msgid,
        struct plxc_enable_req *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t state = ke_state_get(dest_id);
    // Pulse Oximeter Collector Role Task Environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);

    ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);

    if ((state == PLXC_IDLE) && (p_plxc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_plxc_env->p_env[conidx] = (struct plxc_cnx_env*) ke_malloc(sizeof(struct plxc_cnx_env),KE_MEM_ATT_DB);
        memset(p_plxc_env->p_env[conidx], 0, sizeof(struct plxc_cnx_env));

        // Start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // Start discovery with Pulse Oximeter
            prf_disc_svc_send(&(p_plxc_env->prf_env), conidx, ATT_SVC_PULSE_OXIMETER);

            // Go to DISCOVERING SERVICE state
            ke_state_set(dest_id, PLXC_DISCOVERING_SVC);
        }
        // normal connection, get saved att details
        else
        {
            uint16_t search_hdl;
            struct prf_svc *p_svc;

            p_plxc_env->p_env[conidx]->plxc = p_param->plxc;
            // Attribute Handle
            search_hdl = p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_FEATURES].val_hdl;
            // Service
            p_svc = &p_plxc_env->p_env[conidx]->plxc.svc;

            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_READ_FEATURES_AT_INIT;

                // Send the write request
                prf_read_char_send(&(p_plxc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, PLXC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
        }
    }
    else if (state != PLXC_FREE)
    {
        // The message will be forwarded towards the good task instance
       status = PRF_ERR_REQ_DISALLOWED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // The request is disallowed (profile already enabled for this connection, or not enough memory, ...)
        plxc_enable_rsp_send(p_plxc_env, conidx, status, 0, 0, 0);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see PLXC_READ_CMD message from the application.
 * @brief To read the Feature Characteristic in the peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int plxc_read_cmd_handler(ke_msg_id_t const msgid,
        struct plxc_read_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == PLXC_IDLE)
    {
        ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_plxc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_plxc_env->p_env[conidx]->plxc.svc;

            if (p_param->operation == PLXC_OP_CODE_READ_FEATURES)
            {
                search_hdl = p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_FEATURES].val_hdl;
                // Force the operation value
                p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_READ_FEATURES;
            }
            else
            {
                if (p_param->operation == PLXC_SPOT_CHECK_MEAS_CMD_OP_CODE)
                {
                    search_hdl = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_SPOT_MEASUREMENT_CCC].desc_hdl;
                }
                else if (p_param->operation == PLXC_CONTINUOUS_MEAS_CMD_OP_CODE)
                {
                    search_hdl = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_CONT_MEASUREMENT_CCC].desc_hdl;
                }
                else if (p_param->operation == PLXC_RACP_CMD_OP_CODE)
                {
                    search_hdl = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_RACP_CCC].desc_hdl;
                }

                // Force the operation value
                p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_READ_CCC;
            }

            // Check if handle is viable
            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                // Send the read request
                prf_read_char_send(&(p_plxc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, PLXC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == PLXC_FREE)
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
        plxc_send_cmp_evt(p_plxc_env, conidx, p_param->operation, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see PLXC_WRITE_RACP_CMD message from the application.
 * @brief To write command to the Records Access Control Point in the peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

static int plxc_write_racp_cmd_handler(ke_msg_id_t const msgid,
        struct plxc_write_racp_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == PLXC_IDLE)
    {
        ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_plxc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_RACP].val_hdl;
            // Service
            struct prf_svc *p_svc = &p_plxc_env->p_env[conidx]->plxc.svc;
             // Length
            uint8_t data[2];

            // fill parameters
            data[0] = p_param->cp_opcode;
            data[1] = p_param->cp_operator;

            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_WRITE_RACP;

                // Send the write request
                prf_gatt_write(&(p_plxc_env->prf_env), conidx, search_hdl, &data[0], 2, GATTC_WRITE);
                // Go to the Busy state
                ke_state_set(dest_id, PLXC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == PLXC_FREE)
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
        plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_WRITE_RACP, status);
    }

    return msg_status;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @see PLXC_CFG_CCC_CMD message.
 * Allows the application to write new CCC values to a Alert Characteristic in the peer server
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int plxc_cfg_ccc_cmd_handler(ke_msg_id_t const msgid,
                                   struct plxc_cfg_ccc_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);

    if (state == PLXC_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_plxc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = 0;

            if (p_param->operation == PLXC_SPOT_CHECK_MEAS_CMD_OP_CODE)
            {
                handle = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_SPOT_MEASUREMENT_CCC].desc_hdl;
            }
            else if (p_param->operation == PLXC_CONTINUOUS_MEAS_CMD_OP_CODE)
            {
                handle = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_CONT_MEASUREMENT_CCC].desc_hdl;
            }
            else if (p_param->operation == PLXC_RACP_CMD_OP_CODE)
            {
                handle = p_plxc_env->p_env[conidx]->plxc.descs[PLXC_DESC_RACP_CCC].desc_hdl;
            }

            // Check if handle is viable and service not NULL
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (&p_plxc_env->p_env[conidx]->plxc.svc != NULL))
            {
                // Force the operation value
                p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_WRITE_CCC;

                // Send the write request
                prf_gatt_write_ntf_ind(&(p_plxc_env->prf_env), conidx, handle, p_param->ccc);

                // Go to the Busy state
                ke_state_set(dest_id, PLXC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_WRITE_CCC, status);
        }
    }
    else if (state == PLXC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_CMP_EVT message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                 struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
    // Status
    uint8_t status = PRF_APP_ERROR;

    do
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (p_plxc_env == NULL)
        {
            break;
        }

        if (state == PLXC_DISCOVERING_SVC)
        {
            // clean state for the new discovering
            p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_NO_OP;

            if ((p_param->status == GAP_ERR_NO_ERROR) || (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND))
            {
                // Check service (if found)
                if (p_plxc_env->p_env[conidx]->nb_svc)
                {
                    // Check service (mandatory)
                    status = prf_check_svc_char_validity(PLXC_CHAR_MAX,
                            p_plxc_env->p_env[conidx]->plxc.chars,
                            plxc_plx_char);

                    // Check Descriptors (mandatory)
                    if (status == GAP_ERR_NO_ERROR)
                    {
                        status = prf_check_svc_char_desc_validity(PLXC_DESC_MAX,
                                p_plxc_env->p_env[conidx]->plxc.descs,
                                plxc_plx_char_desc,
                                p_plxc_env->p_env[conidx]->plxc.chars);
                    }
                }
            }

            // request read FEATURES
            if (status == GAP_ERR_NO_ERROR)
            {
                // Attribute Handle
                uint16_t search_hdl = p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_FEATURES].val_hdl;
                // Service
                struct prf_svc *p_svc = &p_plxc_env->p_env[conidx]->plxc.svc;

                // Check if handle is viable
                if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
                {
                    // Force the operation value
                    p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_READ_FEATURES_AT_INIT;

                    // Send the read request
                    prf_read_char_send(&(p_plxc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                    // Go to the Busy state
                    ke_state_set(dest_id, PLXC_BUSY);

                    status = GAP_ERR_NO_ERROR;
                }
            }

            // at this stage we send response only if it fail in some case
            if (status != GAP_ERR_NO_ERROR)
            {
                // Raise an PLXC_ENABLE_REQ complete event.
                ke_state_set(dest_id, PLXC_IDLE);

                plxc_enable_rsp_send(p_plxc_env, conidx, p_param->status, 0 ,0 ,0);
            }
        }
        else if (state == PLXC_BUSY)
        {
            uint8_t op_pending = p_plxc_env->p_env[conidx]->op_pending;

            status = p_param->status;
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    // Send the complete event status
                    switch (op_pending)
                    {
                        case PLXC_OP_PENDING_READ_FEATURES_AT_INIT:
                        {
                            // Raise an PLXC_ENABLE_REQ complete event.
                            plxc_enable_rsp_send(p_plxc_env, conidx, status, 0 ,0 ,0);
                        } break;

                        case PLXC_OP_PENDING_READ_FEATURES:
                        {
                            // Send read complete event.
                            plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_READ_FEATURES, status);
                        } break;

                        case PLXC_OP_PENDING_READ_CCC:
                        {
                            // Send read complete event.
                            plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_READ_CCC, status);
                        } break;

                        default:
                        {
                            ASSERT_ERR(0);
                        } break;
                    }

                    ke_state_set(dest_id, PLXC_IDLE);
                } break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    // Send the complete event status
                    plxc_send_cmp_evt(p_plxc_env, conidx,
                                (op_pending == PLXC_OP_PENDING_WRITE_RACP ?
                                        PLXC_OP_CODE_WRITE_RACP : PLXC_OP_CODE_WRITE_CCC),
                                p_param->status);

                    ke_state_set(dest_id, PLXC_IDLE);

                } break;

                case GATTC_REGISTER:
                {
                    if (status != GAP_ERR_NO_ERROR)
                    {
                        // Send the complete event error status
                        plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_GATTC_REGISTER, p_param->status);
                    }
                    // Go to connected state
                    ke_state_set(dest_id, PLXC_IDLE);
                } break;

                case GATTC_UNREGISTER:
                {
                    // Do nothing
                } break;

                default:
                {
                    ASSERT_ERR(0);
                } break;
            }

            p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_NO_OP;
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_READ_IND message.
 * Generic event received after every simple read command sent to peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_read_ind_handler(ke_msg_id_t const msgid,
                                    struct gattc_read_ind const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    do
    {
        // Get the address of the environment
        struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        uint8_t op_pending;
        uint8_t status = GAP_ERR_NO_ERROR;
        // length of data block
        uint16_t length;

        // Supported Features @see common supported_features
        uint16_t sup_feat = 0;
        // If enabled in Supported Features @see common measurement_status_supported
        uint16_t meas_stat_sup = 0;
        // If enabled in Supported Features @see common device_status_supported
        uint32_t dev_stat_sup = 0;
        // data pointer
        uint8_t const *p_data;

        // Flag to check if we need to return IDLE state
        uint8_t to_idle = 0;

        ASSERT_INFO(p_plxc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_plxc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (ke_state_get(dest_id) != PLXC_BUSY)
        {
            break;
        }

        // Find the op_pending - and check it is a valid read code.
        op_pending = p_plxc_env->p_env[conidx]->op_pending;

        length = p_param->length;

        // At least Supported Features is present
        ASSERT_INFO(length >= 2, dest_id, src_id);

        if ((op_pending == PLXC_OP_PENDING_READ_FEATURES_AT_INIT) ||
                (op_pending == PLXC_OP_PENDING_READ_FEATURES))
        {
            sup_feat = co_read16p(&p_param->value[0]);
            p_data = &p_param->value[2];

            length -= 2; // uint16 supported_features;
            if (GETB(sup_feat, PLXP_FEAT_MEAS_STATUS_SUP) && (length >= 2))
            {
                meas_stat_sup = co_read16p(p_data);
                p_data += 2; // uint16 measurement_status_supported;
                length -= 2;
            }

            if (GETB(sup_feat, PLXP_FEAT_DEV_SENSOR_STATUS_SUP) && (length >= 3))
            {
                dev_stat_sup = co_read24p(p_data);
            }
        }

        switch (op_pending)
        {
            case PLXC_OP_PENDING_READ_FEATURES_AT_INIT:
            {
                // Raise an PLXC_ENABLE_REQ complete event.
                plxc_enable_rsp_send(p_plxc_env, conidx, status,
                            sup_feat,
                            meas_stat_sup,
                            dev_stat_sup);

                to_idle = 1;
            } break;

            case PLXC_OP_PENDING_READ_FEATURES:
            {
                // Send value ind and next complete event.
                // indication Build a PLXC_VALUE_IND message
                struct plxc_value_ind *p_ind = KE_MSG_ALLOC(PLXC_VALUE_IND,
                                           prf_dst_task_get(&p_plxc_env->prf_env, conidx),
                                           prf_src_task_get(&p_plxc_env->prf_env, conidx),
                                           plxc_value_ind);

                p_ind->operation = PLXC_OP_CODE_READ_FEATURES;
                p_ind->value.features.sup_feat = sup_feat;
                p_ind->value.features.meas_stat_sup = meas_stat_sup;
                p_ind->value.features.dev_stat_sup = dev_stat_sup;

                // send indication to the application
                ke_msg_send(p_ind);

                to_idle = 1;

                plxc_send_cmp_evt(p_plxc_env, conidx, PLXC_OP_CODE_READ_FEATURES, status);
            } break;

            case PLXC_OP_PENDING_READ_CCC:
            {
                struct plxc_rd_char_ccc_ind *p_ind = KE_MSG_ALLOC(PLXC_RD_CHAR_CCC_IND,
                                            prf_dst_task_get(&(p_plxc_env->prf_env), conidx),
                                            dest_id,
                                            plxc_rd_char_ccc_ind);

                p_ind->ind_cfg = co_read16p(&p_param->value[0]);

                ke_msg_send(p_ind);

                // Keep the Busy state to be treated in gattc_cmp_evt_handler
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }

        // return to IDLE state
        if (to_idle)
        {
            p_plxc_env->p_env[conidx]->op_pending = PLXC_OP_PENDING_NO_OP;
            ke_state_set(dest_id, PLXC_IDLE);
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_EVENT_IND message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                   struct gattc_event_ind const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    do
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);
        // data pointer
        const uint8_t *p_data;
        // length of data block
        uint16_t length;
        //indication Build a PLXC_VALUE_IND message
        struct plxc_value_ind *p_ind;
        struct gattc_event_cfm *p_cfm;

        if (p_plxc_env == NULL)
        {
            break;
        }

        if ((p_param->type != GATTC_INDICATE) && (p_param->type != GATTC_NOTIFY))
        {
            break;
        }

        p_ind = KE_MSG_ALLOC(PLXC_VALUE_IND,
                       prf_dst_task_get(&p_plxc_env->prf_env, conidx),
                       prf_src_task_get(&p_plxc_env->prf_env, conidx),
                       plxc_value_ind);

        memset(p_ind, 0, sizeof(struct plxc_value_ind));

        if (p_param->handle == p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_SPOT_MEASUREMENT].val_hdl)
        {
            uint8_t spot_flags  = p_param->value[0];

            length = p_param->length;
            p_data = &p_param->value[5];

            p_ind->operation = PLXC_SPOT_CHECK_MEAS_CMD_OP_CODE;
            // decode message
            if (length >= 5)
            {
                // uint8_t  bitfield of supplied data @see common plxp_spot_meas_flag
                p_ind->value.spot_meas.spot_flags = spot_flags;
                // uint16_t Percentage with a resolution of 1
                p_ind->value.spot_meas.sp_o2 = co_read16p(&p_param->value[1]);
                // uint16_t Period (beats per minute) with a resolution of 1
                p_ind->value.spot_meas.pr = co_read16p(&p_param->value[3]);

                length -= 5;
            }
            else
            {
                //no further parameters possible
                length = 0;
            }

            // Timestamp (if present)
            if (GETB(spot_flags, PLXP_SPOT_MEAS_FLAGS_TIMESTAMP))
            {
                if (length >= 7)
                {
                    p_data += prf_unpack_date_time((uint8_t *)p_data,
                            (struct prf_date_time *)&p_ind->value.spot_meas.timestamp);

                    length -= 7;
                }
                else
                {
                    //no further parameters possible
                    length = 0;
                }
            }

            // Measurement Status (bitfield) @see common measurement_status_supported
            if (GETB(spot_flags, PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS) && (length >= 2))
            {
                p_ind->value.spot_meas.meas_stat = co_read16p(p_data);
                p_data += 2;
                length -= 2;
            }

            // Device and Sensor Status (bitfield) @see common device_status_supported
            if (GETB(spot_flags, PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS) && (length >= 2))
            {
                p_ind->value.spot_meas.dev_sensor_stat = co_read24p(p_data);
                p_data += 3;
                length -= 3;
            }

            // Pulse Amplitude Index - Unit is percentage with a resolution of 1
            if (GETB(spot_flags, PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE) && (length >= 2))
            {
                p_ind->value.spot_meas.pulse_ampl = co_read16p(p_data);
                p_data += 2;
                length -= 2;
            }
        }
        else if (p_param->handle == p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_CONT_MEASUREMENT].val_hdl)
        {
            uint8_t cont_flags  = p_param->value[0];

            length = p_param->length;
            p_data = &p_param->value[5];

            p_ind->operation = PLXC_CONTINUOUS_MEAS_CMD_OP_CODE;

            // decode message
            if (length >= 5)
            {
                // bitfield of supplied data @see common plxp_cont_meas_flag
                p_ind->value.cont_meas.cont_flags = cont_flags;
                // SpO2 - PR measurements - Normal
                p_ind->value.cont_meas.normal.sp_o2 = co_read16p(&p_param->value[1]);
                p_ind->value.cont_meas.normal.pr = co_read16p(&p_param->value[3]);
                length -= 5;
            }
            else
            {
                //no further parameters possible
                length = 0;
            }

            // SpO2 - PR measurements - Fast
            if (GETB(cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST))
            {
                if (length >=4)
                {
                    p_ind->value.cont_meas.fast.sp_o2 = co_read16p(p_data);
                    p_data += 2;
                    p_ind->value.cont_meas.fast.pr = co_read16p(p_data);
                    p_data += 2;
                    length -= 4;
                }
                else
                {
                    //no further parameters possible
                    length = 0;
                }
            }
            // SpO2 - PR measurements - Slow
            if (GETB(cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW))
            {
                if (length >= 4)
                {
                    p_ind->value.cont_meas.slow.sp_o2 = co_read16p(p_data);
                    p_data += 2;
                    p_ind->value.cont_meas.slow.pr = co_read16p(p_data);
                    p_data += 2;
                    length -= 4;
                }
                else
                {
                    //no further parameters possible
                    length = 0;
                }
            }
            // Measurement Status (bitfield) @see common measurement_status_supported
            if (GETB(cont_flags, PLXP_CONT_MEAS_FLAGS_MEAS_STATUS) && (length >= 2))
            {
                p_ind->value.cont_meas.meas_stat = co_read16p(p_data);
                p_data += 2;
                length -= 2;
            }
            // Device and Sensor Status (bitfield) @see common device_status_supported
            if (GETB(cont_flags, PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS) && (length >= 2))
            {
                p_ind->value.cont_meas.dev_sensor_stat = co_read24p(p_data);
                p_data += 3;
                length -= 3;
            }
            // Pulse Amplitude Index - Unit is percentage with a resolution of 1
            if (GETB(cont_flags, PLXP_CONT_MEAS_FLAGS_PULSE_AMPL) && (length >= 2))
            {
                p_ind->value.cont_meas.pulse_ampl = co_read16p(p_data);
            }
        }
        else if (p_param->handle == p_plxc_env->p_env[conidx]->plxc.chars[PLXC_CHAR_RACP].val_hdl)
        {
            // Control Point OpCode @see enum plxp_cp_opcodes_id
            uint8_t cp_opcode = p_param->value[0];

            p_ind->operation = PLXC_RACP_CMD_OP_CODE;
            p_ind->value.racp_resp.cp_opcode = cp_opcode;
            // RACP operator should be indicated (normally an Operator of NULL 0x00)
            p_ind->value.racp_resp.cp_operator = p_param->value[1];
            if (cp_opcode == PLXP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP)
            {
                p_ind->value.racp_resp.rec_num = co_read16p(&p_param->value[2]);
            }
            else
            {
                // Request Control Point OpCode @see enum plxp_cp_opcodes_id
                p_ind->value.racp_resp.req_cp_opcode = p_param->value[2];
                // Response Code @see enum plxp_cp_resp_code_id
                p_ind->value.racp_resp.rsp_code = p_param->value[3];
            }
        }
        else
        {
            // error case
            ASSERT_ERR(0);
        }

        // send indication to the application
        ke_msg_send(p_ind);

        // confirm that indication has been correctly received
        p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

        p_cfm->handle = p_param->handle;
        ke_msg_send(p_cfm);

    } while (0);

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(plxc)
{
    {PLXC_ENABLE_REQ,                   (ke_msg_func_t)plxc_enable_req_handler},

    {GATTC_SDP_SVC_IND,                 (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                     (ke_msg_func_t)gattc_cmp_evt_handler},

    {GATTC_READ_IND,                    (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_EVENT_IND,                   (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,               (ke_msg_func_t)gattc_event_ind_handler},

    {PLXC_READ_CMD,                     (ke_msg_func_t)plxc_read_cmd_handler},

    {PLXC_WRITE_RACP_CMD,               (ke_msg_func_t)plxc_write_racp_cmd_handler},

    {PLXC_CFG_CCC_CMD,                  (ke_msg_func_t)plxc_cfg_ccc_cmd_handler},
};

void plxc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct plxc_env_tag *p_plxc_env = PRF_ENV_GET(PLXC, plxc);

    p_task_desc->msg_handler_tab = plxc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(plxc_msg_handler_tab);
    p_task_desc->state           = p_plxc_env->state;
    p_task_desc->idx_max         = PLXC_IDX_MAX;
}

#endif //(BLE_PLX_CLIENT)

/// @} PLXCTASK
