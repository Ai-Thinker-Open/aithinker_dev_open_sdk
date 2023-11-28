/**
 ****************************************************************************************
 *
 * @file bcsc_task.c
 *
 * @brief Body Composition Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup BCSCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BCS_CLIENT)
#include "bcs_common.h"
#include "bcsc_task.h"
#include "bcsc.h"

#include "gap.h"
#include "attm.h"
#include "gattc_task.h"
#include "ke_mem.h"
#include "co_utils.h"


/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve Body Composition Service characteristics information
const struct prf_char_def bcsc_bcs_char[BCSC_CHAR_BCS_MAX] =
{
    [BCSC_CHAR_BCS_FEATURE] = {ATT_CHAR_BODY_COMPOSITION_FEATURE,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_RD ) },

    [BCSC_CHAR_BCS_MEAS]    = {ATT_CHAR_BODY_COMPOSITION_MEASUREMENT,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_IND) },
};

/// State machine used to retrieve Body Composition Service characteristic description information
const struct prf_char_desc_def bcsc_bcs_char_desc[BCSC_DESC_BCS_MAX] =
{
    /// Client config
    [BCSC_DESC_BCS_MEAS_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         BCSC_CHAR_BCS_MEAS},
};


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_ind Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_sdp_svc_ind const *p_ind,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == BCSC_DISCOVERING_SVC)
    {
        // Body Composition Client Role Task Environment
        struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_bcsc_env->env[conidx] != NULL, dest_id, src_id);

        if(attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_BODY_COMPOSITION))
        {
            // Retrieve BCS characteristics and descriptors
            prf_extract_svc_info(p_ind, BCSC_CHAR_BCS_MAX, &bcsc_bcs_char[0],  &p_bcsc_env->env[conidx]->bcs.chars[0],
                    BCSC_DESC_BCS_MAX, &bcsc_bcs_char_desc[0], &p_bcsc_env->env[conidx]->bcs.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_bcsc_env->env[conidx]->bcs.svc.shdl = p_ind->start_hdl;
            p_bcsc_env->env[conidx]->bcs.svc.ehdl = p_ind->end_hdl;

            p_bcsc_env->env[conidx]->nb_svc++;
        }

    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_IND message.
 * The handler stores the found characteristic details.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_ind Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int gattc_disc_char_ind_handler(ke_msg_id_t const msgid,
                    struct gattc_disc_char_ind const *p_ind,
                    ke_task_id_t const dest_id,
                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == BCSC_DISCOVERING_CHARS)
    {
        uint8_t i;
        uint8_t conidx = KE_IDX_GET(dest_id);
        struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_bcsc_env->env[conidx] != NULL, dest_id, src_id);

        for(i = 0; i < BCSC_CHAR_BCS_MAX; i++)
        {
            uint16_t uuid = co_read16p(&p_ind->uuid[0]);

            if ((bcsc_bcs_char[i].uuid == uuid) && (p_ind->prop == bcsc_bcs_char[i].prop_mand))
            {
                p_bcsc_env->env[conidx]->bcs.chars[i].char_hdl = p_ind->attr_hdl;
                p_bcsc_env->env[conidx]->bcs.chars[i].val_hdl = p_ind->pointer_hdl;
                p_bcsc_env->env[conidx]->bcs.chars[i].prop = p_ind->prop;

                // If Body Composition Feature - then End Handle == val_hdl.
                // The end handle for the Body Composition Measurement is written after the discovery of the CCC.
                if (uuid == ATT_CHAR_BODY_COMPOSITION_FEATURE)
                {
                    p_bcsc_env->env[conidx]->bcs.chars[BCSC_CHAR_BCS_FEATURE].char_ehdl_off =
                            p_ind->pointer_hdl - p_ind->attr_hdl;
                }
            }
        }
    }

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_DISC_CHAR_DESC_IND  message.
 * The handler stores the found descriptor details for a given characteristic.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_ind Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int gattc_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
                    struct gattc_disc_char_desc_ind const *p_ind,
                    ke_task_id_t const dest_id,
                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == BCSC_DISCOVERING_CHAR_DESC)
    {
        uint8_t i;
        uint8_t conidx = KE_IDX_GET(dest_id);
        struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_bcsc_env->env[conidx] != NULL, dest_id, src_id);

        for (i = 0; i < BCSC_DESC_BCS_MAX; i++)
        {
            uint16_t uuid = co_read16p(&p_ind->uuid[0]);

            if (bcsc_bcs_char_desc[i].uuid == uuid)
            {
                p_bcsc_env->env[conidx]->bcs.descs[i].desc_hdl = p_ind->attr_hdl;

                if (uuid == ATT_DESC_CLIENT_CHAR_CFG)
                {
                    // This is the end handle of the Body Measurement Characteristic
                    if (p_ind->attr_hdl)
                    {
                        p_bcsc_env->env[conidx]->bcs.chars[BCSC_CHAR_BCS_MEAS].char_ehdl_off =
                                p_ind->attr_hdl - p_bcsc_env->env[conidx]->bcs.chars[BCSC_CHAR_BCS_MEAS].char_hdl;
                    }
                }
            }
        }
    }

    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSC_ENABLE_REQ message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int bcsc_enable_req_handler(ke_msg_id_t const msgid,
        struct bcsc_enable_req *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t state = ke_state_get(dest_id);
    // Body Composition Collector Role Task Environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

    ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
    if ((state == BCSC_IDLE) && (p_bcsc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_bcsc_env->env[conidx] = (struct bcsc_cnx_env *) ke_malloc(sizeof(struct bcsc_cnx_env), KE_MEM_ATT_DB);
        memset(p_bcsc_env->env[conidx], 0, sizeof(struct bcsc_cnx_env));

        // Start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            if (p_param->svc_type != PRF_SECONDARY_SERVICE)
            {
                // Start discovery with Body Composition
                prf_disc_svc_send(&(p_bcsc_env->prf_env), conidx, ATT_SVC_BODY_COMPOSITION);

                // Go to DISCOVERING SERVICE state
                ke_state_set(dest_id, BCSC_DISCOVERING_SVC);
            }
            else
            {
                // The Start and End Handle of the service are provided as Params.
                if ((p_param->bcs.svc.shdl) && (p_param->bcs.svc.ehdl))
                {
                    struct gattc_disc_cmd * p_char_disc = NULL;

                    p_bcsc_env->env[conidx]->bcs.svc.shdl = p_param->bcs.svc.shdl;
                    p_bcsc_env->env[conidx]->bcs.svc.ehdl = p_param->bcs.svc.ehdl;

                    // Need to Kick Start off the - Discovery of the Chars and Descriptors.
                    // Send a Characteristic Discovery
                    p_char_disc = KE_MSG_ALLOC_DYN(GATTC_DISC_CMD,
                                KE_BUILD_ID(TASK_GATTC, conidx),
                                prf_src_task_get(&(p_bcsc_env->prf_env), conidx),
                                gattc_disc_cmd, ATT_UUID_16_LEN);

                    p_char_disc->operation = GATTC_DISC_ALL_CHAR;
                    p_char_disc->start_hdl = p_bcsc_env->env[conidx]->bcs.svc.shdl;
                    p_char_disc->end_hdl = p_bcsc_env->env[conidx]->bcs.svc.ehdl;
                    p_char_disc->uuid_len  = ATT_UUID_16_LEN;
                    memset(p_char_disc->uuid, 0, ATT_UUID_16_LEN);

                    ke_msg_send(p_char_disc);
                    // Go to DISCOVERING state
                    ke_state_set(dest_id, BCSC_DISCOVERING_CHARS);
                }
                else
                {
                    bcsc_enable_rsp_send(p_bcsc_env, conidx, PRF_ERR_INEXISTENT_HDL);
                }
            }
        }
        // normal connection, get saved att details
        else
        {
            p_bcsc_env->env[conidx]->bcs = p_param->bcs;
            p_bcsc_env->env[conidx]->meas_evt_msg = 0;
            bcsc_enable_rsp_send(p_bcsc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != BCSC_FREE)
    {
        // The message will be forwarded towards the good task instance
        status = PRF_ERR_REQ_DISALLOWED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // The request is disallowed (profile already enabled for this connection, or not enough memory, ...)
        bcsc_enable_rsp_send(p_bcsc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSC_RD_FEAT_CMD message from the application.
 * @brief To read the Feature Characteristic in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int bcsc_rd_feat_cmd_handler(ke_msg_id_t const msgid,
        struct bcsc_rd_feat_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == BCSC_IDLE)
    {
        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_bcsc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = p_bcsc_env->env[conidx]->bcs.chars[BCSC_CHAR_BCS_FEATURE].val_hdl;
            // Service
            struct prf_svc *p_svc = &p_bcsc_env->env[conidx]->bcs.svc;

            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_bcsc_env->env[conidx]->op_pending = BCSC_READ_FEAT_OP_CODE;
                // Send read request
                prf_read_char_send(&(p_bcsc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, BCSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == BCSC_FREE)
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
        bcsc_send_cmp_evt(p_bcsc_env, conidx, BCSC_READ_FEAT_OP_CODE, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSC_RD_MEAS_CCC_CMD  message from the application.
 * @brief To read the CCC value of the Measurement characteristic in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int bcsc_rd_meas_ccc_cmd_handler(ke_msg_id_t const msgid,
        struct bcsc_rd_meas_ccc_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == BCSC_IDLE)
    {
        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_bcsc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = p_bcsc_env->env[conidx]->bcs.descs[BCSC_DESC_BCS_MEAS_CCC].desc_hdl;
            // Service
            struct prf_svc *p_svc = &p_bcsc_env->env[conidx]->bcs.svc;

            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_bcsc_env->env[conidx]->op_pending = BCSC_READ_CCC_OP_CODE;
                // Send read request
                prf_read_char_send(&(p_bcsc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, BCSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            bcsc_send_cmp_evt(p_bcsc_env, conidx, BCSC_READ_CCC_OP_CODE, status);
        }
    }
    else if (state == BCSC_FREE)
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
        bcsc_send_cmp_evt(p_bcsc_env, conidx, BCSC_READ_CCC_OP_CODE, status);
    }

    return (msg_status);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BCSC_WR_MEAS_CCC_CMD message.
 * Allows the application to write new CCC values to a Characteristic in the peer server
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_cmd Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int bcsc_wr_meas_ccc_cmd_handler(ke_msg_id_t const msgid,
                                   struct bcsc_wr_meas_ccc_cmd *p_cmd,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);
    // Get connection index

    if (state == BCSC_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_bcsc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t handle = p_bcsc_env->env[conidx]->bcs.descs[BCSC_DESC_BCS_MEAS_CCC].desc_hdl;
            // Service
            struct prf_svc *p_svc = &p_bcsc_env->env[conidx]->bcs.svc;

            // Check if handle is viable
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_bcsc_env->env[conidx]->op_pending = BCSC_WRITE_CCC_OP_CODE;

                // Send the write request
                prf_gatt_write_ntf_ind(&(p_bcsc_env->prf_env), conidx, handle, p_cmd->ccc);

                // Go to the Busy state
                ke_state_set(dest_id, BCSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            bcsc_send_cmp_evt(p_bcsc_env, conidx, BCSC_WRITE_CCC_OP_CODE, status);
        }
    }
    else if (state == BCSC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return (msg_status);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                 struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

    if (p_bcsc_env != NULL)
    {
        // Status
        uint8_t status;
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == BCSC_DISCOVERING_SVC)
        {
            if ((p_param->status == GAP_ERR_NO_ERROR) || (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND))
            {
                // Check service (if found)
                if(p_bcsc_env->env[conidx]->nb_svc)
                {
                    // Check service (mandatory)
                    status = prf_check_svc_char_validity(BCSC_CHAR_BCS_MAX,
                            p_bcsc_env->env[conidx]->bcs.chars,
                            bcsc_bcs_char);

                    // Check Descriptors (mandatory)
                    if(status == GAP_ERR_NO_ERROR)
                    {
                        status = prf_check_svc_char_desc_validity(BCSC_DESC_BCS_MAX,
                                p_bcsc_env->env[conidx]->bcs.descs,
                                bcsc_bcs_char_desc,
                                p_bcsc_env->env[conidx]->bcs.chars);
                    }
                }
            }

            // Raise an ESP_ENABLE_REQ complete event.
            ke_state_set(dest_id, BCSC_IDLE);

            bcsc_enable_rsp_send(p_bcsc_env, conidx, p_param->status);
        }
        else if (state == BCSC_DISCOVERING_CHARS)
        {
            // Finished Characteristic Discovery - move onto Descriptor Discovery
            if ((p_param->status == GAP_ERR_NO_ERROR) || (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND))
            {
                // Send a Descriptor Discovery
                struct gattc_disc_cmd *p_desc_disc =
                    KE_MSG_ALLOC_DYN(GATTC_DISC_CMD,
                    KE_BUILD_ID(TASK_GATTC, conidx),
                    prf_src_task_get(&(p_bcsc_env->prf_env), conidx),
                    gattc_disc_cmd, ATT_UUID_16_LEN);

                p_desc_disc->operation = GATTC_DISC_DESC_CHAR;
                p_desc_disc->start_hdl = p_bcsc_env->env[conidx]->bcs.svc.shdl+1;
                p_desc_disc->end_hdl = p_bcsc_env->env[conidx]->bcs.svc.ehdl;
                p_desc_disc->uuid_len  = ATT_UUID_16_LEN;

                memset(p_desc_disc->uuid, 0, ATT_UUID_16_LEN);

                // Reset the Char_Idx back to zero. So Descriptor discovery begins at the first char.
                //envc_env->env[conidx]->char_idx = 0;
                ke_msg_send(p_desc_disc);

                ke_state_set(dest_id, BCSC_DISCOVERING_CHAR_DESC);
            }
            else
            {
                // Send an BCSC_ENABLE_RSP .
                ke_state_set(dest_id, BCSC_IDLE);
                bcsc_enable_rsp_send(p_bcsc_env, conidx, p_param->status);
            }
        }
        else if (state == BCSC_DISCOVERING_CHAR_DESC)
        {
            // Raise an ESP_ENABLE_REQ complete event.
            ke_state_set(dest_id, BCSC_IDLE);

            if (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND)
            {
                bcsc_enable_rsp_send(p_bcsc_env, conidx, GAP_ERR_NO_ERROR);
            }
            else
            {
                bcsc_enable_rsp_send(p_bcsc_env, conidx, p_param->status);
            }
        }
        else if (state == BCSC_BUSY)
        {
            uint8_t op_pending = p_bcsc_env->env[conidx]->op_pending;

            status = p_param->status;
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    switch(op_pending)
                    {
                        case BCSC_READ_FEAT_OP_CODE:
                        case BCSC_READ_CCC_OP_CODE:
                        {
                            bcsc_send_cmp_evt(p_bcsc_env, conidx, op_pending, status);
                        }
                        break;

                        default:
                        {
                            break;
                        }
                    }
                    ke_state_set(dest_id, BCSC_IDLE);
                } break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    // Send the complete event status
                    bcsc_send_cmp_evt(p_bcsc_env, conidx, op_pending, status);
                    ke_state_set(dest_id, BCSC_IDLE);

                } break;

                case GATTC_REGISTER:
                {
                    if (status != GAP_ERR_NO_ERROR)
                    {
                        // Send the complete event error status
                        bcsc_send_cmp_evt(p_bcsc_env, conidx, GATTC_REGISTER, status);
                    }
                    // Go to connected state
                    ke_state_set(dest_id, BCSC_IDLE);
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
        }
    }
    // else ignore the message

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_READ_IND message.
 *
 * Generic event received after every simple read command sent to peer server.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_ind_handler(ke_msg_id_t const msgid,
                                    struct gattc_read_ind const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == BCSC_BUSY)
    {
        // Get the address of the environment
        struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t op_pending;

        ASSERT_INFO(p_bcsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_bcsc_env->env[conidx] != NULL, dest_id, src_id);

        // Find the op_pending -  and check it is a valid read code.
        op_pending = p_bcsc_env->env[conidx]->op_pending;
        if ((op_pending == BCSC_READ_FEAT_OP_CODE) || (op_pending == BCSC_READ_CCC_OP_CODE))
        {
            switch (op_pending)
            {
                case BCSC_READ_FEAT_OP_CODE:
                {
                    struct bcsc_feat_ind *p_ind = KE_MSG_ALLOC(BCSC_FEAT_IND,
                                                prf_dst_task_get(&(p_bcsc_env->prf_env), conidx),
                                                dest_id,
                                                bcsc_feat_ind);

                    p_ind->feature = co_read32p(&p_param->value[0]);

                    ke_msg_send(p_ind);
                }
                break;

                case BCSC_READ_CCC_OP_CODE :
                {
                    struct bcsc_meas_ccc_ind *p_ind = KE_MSG_ALLOC(BCSC_MEAS_CCC_IND,
                                                prf_dst_task_get(&(p_bcsc_env->prf_env), conidx),
                                                dest_id,
                                                bcsc_meas_ccc_ind);

                    p_ind->ccc = co_read16p(&p_param->value[0]);

                    ke_msg_send(p_ind);
                }
                break;

                default:
                {
                    ASSERT_ERR(0);
                } break;
            }
        }
    }
    // else drop the message

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_EVENT_IND message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                   struct gattc_event_ind const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

    if (p_bcsc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        switch (p_param->type)
        {
            case (GATTC_INDICATE):
            {
                // confirm that indication has been correctly received
                struct gattc_event_cfm *p_cfm = NULL;

                if (p_param->handle == p_bcsc_env->env[conidx]->bcs.chars[BCSC_CHAR_BCS_MEAS].val_hdl)
                {
                    //indication Build a BCSC_MEAS_IND message
                    uint8_t *p_val;
                    struct bcsc_meas_ind *p_ind = NULL;
                    uint16_t flags;
                    uint8_t second_segment_of_multipacket = 0;

                    p_val = (uint8_t *)&p_param->value[0];
                    flags =  co_read16p(p_val) & BCS_MEAS_FLAGS_VALID;

                    if (GETB(flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS) && (p_bcsc_env->env[conidx]->meas_evt_msg != 0))
                    {
                        p_ind = ke_msg2param(p_bcsc_env->env[conidx]->meas_evt_msg);
                        second_segment_of_multipacket = 0x1;
                    }
                    else if (p_bcsc_env->env[conidx]->meas_evt_msg == 0)
                    {
                        p_ind = KE_MSG_ALLOC(BCSC_MEAS_IND,
                                prf_dst_task_get(&p_bcsc_env->prf_env, conidx),
                                prf_src_task_get(&p_bcsc_env->prf_env, conidx),
                                bcsc_meas_ind);

                        if (GETB(flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS))
                        {
                            p_bcsc_env->env[conidx]->meas_evt_msg = ke_param2msg(p_ind);
                        }
                    }

                    if (p_ind != NULL)
                    {
                        // decode message
                        // Flags is 16 bit
                        p_val += sizeof(uint16_t);

                        // Body Fat Percentage is mandatory
                        p_ind->body_fat_percent = co_read16p(p_val);
                        p_val += sizeof(uint16_t);

                        // shall not be considered if it's in the second segement of multipacket
                        if (!second_segment_of_multipacket)
                        {
                            // Measurement Units
                            p_ind->meas_u = *p_val++;
                        }

                        // Time Stamp if present
                        if (GETB(flags, BCS_MEAS_FLAGS_TIMESTAMP_PRESENT))
                        {
                            p_val += prf_unpack_date_time(p_val, &p_ind->time_stamp);
                        }
                        // User Id if present
                        if (GETB(flags, BCS_MEAS_FLAGS_USER_ID_PRESENT))
                        {
                            p_ind->user_id = *p_val++;
                        }

                        // Basal Metabolism if present
                        if (GETB(flags, BCS_MEAS_FLAGS_BASAL_METAB_PRESENT))
                        {
                            p_ind->basal_metab = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Muscle Percentage if present
                        if (GETB(flags, BCS_MEAS_FLAGS_MUSCLE_PERCENT_PRESENT))
                        {
                            p_ind->muscle_percent = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Muscle Mass if present
                        if (GETB(flags, BCS_MEAS_FLAGS_MUSCLE_MASS_PRESENT))
                        {
                            p_ind->muscle_mass = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Fat Free Mass if present
                        if (GETB(flags, BCS_MEAS_FLAGS_FAT_FREE_MASS_PRESENT))
                        {
                            p_ind->fat_free_mass = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Soft Lean Mass if present
                        if (GETB(flags, BCS_MEAS_FLAGS_SOFT_LEAN_MASS_PRESENT))
                        {
                            p_ind->soft_lean_mass = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Body Water Mass if present
                        if (GETB(flags, BCS_MEAS_FLAGS_BODY_WATER_MASS_PRESENT))
                        {
                            p_ind->body_water_mass = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Impedance if present
                        if (GETB(flags, BCS_MEAS_FLAGS_IMPEDANCE_PRESENT))
                        {
                            p_ind->impedance = co_read16p(p_val);
                            p_val += sizeof(uint16_t);
                        }

                        // Weight if present
                        if (GETB(flags, BCS_MEAS_FLAGS_WEIGHT_PRESENT))
                        {
                            p_ind->weight = co_read16p(p_val);
                            p_val += sizeof(uint16_t);

                            // Mass resolution
                            p_ind->mass_resol = *p_val++;
                        }

                        // Height if present
                        if (GETB(flags, BCS_MEAS_FLAGS_HEIGHT_PRESENT))
                        {
                            p_ind->height = co_read16p(p_val);
                            p_val += sizeof(uint16_t);

                            // Height resolution
                            p_ind->hght_resol = *p_val++;
                        }

                        // if Multipacket Measurement present
                        if (GETB(flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS))
                        {
                            if (second_segment_of_multipacket)
                            {
                                SETB(flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS, 0);
                                p_ind->flags |= flags;

                                p_bcsc_env->env[conidx]->meas_evt_msg = 0;
                                ke_msg_send(p_ind);
                            }
                            else
                            {
                                SETB(flags, BCS_MEAS_FLAGS_MULTIPACKET_MEAS, 0);
                                p_ind->flags = flags;
                            }
                        }
                        else
                        {
                            p_ind->flags = flags;
                            p_bcsc_env->env[conidx]->meas_evt_msg = 0;
                            ke_msg_send(p_ind);
                        }
                    }
                }

                p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
                p_cfm->handle = p_param->handle;
                ke_msg_send(p_cfm);

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
KE_MSG_HANDLER_TAB(bcsc)
{
    {BCSC_ENABLE_REQ,                   (ke_msg_func_t)bcsc_enable_req_handler},

    {GATTC_SDP_SVC_IND,                 (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_DISC_CHAR_IND,               (ke_msg_func_t)gattc_disc_char_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND,          (ke_msg_func_t)gattc_disc_char_desc_ind_handler},
    {GATTC_CMP_EVT,                     (ke_msg_func_t)gattc_cmp_evt_handler},

    {GATTC_READ_IND,                    (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_EVENT_IND,                   (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {BCSC_RD_FEAT_CMD,                  (ke_msg_func_t)bcsc_rd_feat_cmd_handler},
    {BCSC_RD_MEAS_CCC_CMD,              (ke_msg_func_t)bcsc_rd_meas_ccc_cmd_handler},
    {BCSC_WR_MEAS_CCC_CMD,              (ke_msg_func_t)bcsc_wr_meas_ccc_cmd_handler},

};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void bcsc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct bcsc_env_tag *p_bcsc_env = PRF_ENV_GET(BCSC, bcsc);

    p_task_desc->msg_handler_tab = bcsc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(bcsc_msg_handler_tab);
    p_task_desc->state           = p_bcsc_env->state;
    p_task_desc->idx_max         = BCSC_IDX_MAX;
}

#endif //(BLE_BCS_CLIENT)

/// @} BCSCTASK
