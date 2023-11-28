/**
 ****************************************************************************************
 *
 * @file tipc_task.c
 *
 * @brief Time Profile Client Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup TIPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_TIP_CLIENT)
#include "co_utils.h"
#include "tipc_task.h"
#include "tipc.h"
#include "gap.h"
#include "attm.h"
#include "gattc_task.h"

#include "ke_mem.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/// State machine used to retrieve Current Time service characteristics information
const struct prf_char_def tipc_cts_char[TIPC_CHAR_CTS_MAX] =
{
    // Current Time
    [TIPC_CHAR_CTS_CURR_TIME]        = {ATT_CHAR_CT_TIME,
                                        ATT_MANDATORY,
                                        ATT_CHAR_PROP_RD|ATT_CHAR_PROP_NTF},
    // Local Time Info
    [TIPC_CHAR_CTS_LOCAL_TIME_INFO]  = {ATT_CHAR_LOCAL_TIME_INFO,
                                        ATT_OPTIONAL,
                                        ATT_CHAR_PROP_RD},
    // Reference Time Info
    [TIPC_CHAR_CTS_REF_TIME_INFO]    = {ATT_CHAR_REFERENCE_TIME_INFO,
                                        ATT_OPTIONAL,
                                        ATT_CHAR_PROP_RD},
};

/// State machine used to retrieve Current Time service characteristic description information
const struct prf_char_desc_def tipc_cts_char_desc[TIPC_DESC_CTS_MAX] =
{
    // Current Time client config
    [TIPC_DESC_CTS_CURR_TIME_CLI_CFG] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         TIPC_CHAR_CTS_CURR_TIME},
};

/// State machine used to retrieve Next DST Change service characteristics information
const struct prf_char_def tipc_ndcs_char[TIPC_CHAR_NDCS_MAX] =
{
    // Current Time
    [TIPC_CHAR_NDCS_TIME_WITH_DST]    = {ATT_CHAR_TIME_WITH_DST,
                                         ATT_MANDATORY,
                                         ATT_CHAR_PROP_RD},
};

/// State machine used to retrieve Reference Time Update service characteristics information
const struct prf_char_def tipc_rtus_char[TIPC_CHAR_RTUS_MAX] =
{
    // Time Update Control Point
    [TIPC_CHAR_RTUS_TIME_UPD_CTNL_PT] = {ATT_CHAR_TIME_UPDATE_CNTL_POINT,
                                         ATT_MANDATORY,
                                         ATT_CHAR_PROP_WR_NO_RESP},
    // Time Update State
    [TIPC_CHAR_RTUS_TIME_UPD_STATE]   = {ATT_CHAR_TIME_UPDATE_STATE,
                                         ATT_MANDATORY,
                                         ATT_CHAR_PROP_RD},
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
    if (ke_state_get(dest_id) == TIPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);

        ASSERT_INFO(p_tipc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_tipc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_CURRENT_TIME))
        {
            // Retrieve TIS characteristics and descriptors
            prf_extract_svc_info(p_ind, TIPC_CHAR_CTS_MAX, &tipc_cts_char[0],  &p_tipc_env->p_env[conidx]->cts.chars[0],
                    TIPC_DESC_CTS_MAX, &tipc_cts_char_desc[0], &p_tipc_env->p_env[conidx]->cts.descs[0]);

            // Even if we get multiple responses we only store 1 range
            p_tipc_env->p_env[conidx]->cts.svc.shdl = p_ind->start_hdl;
            p_tipc_env->p_env[conidx]->cts.svc.ehdl = p_ind->end_hdl;
        }

        if (attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_NEXT_DST_CHANGE))
        {
            // Retrieve NDCS characteristics and descriptors
            prf_extract_svc_info(p_ind, TIPC_CHAR_NDCS_MAX, &tipc_ndcs_char[0],  &p_tipc_env->p_env[conidx]->ndcs.chars[0],
                    0, NULL, NULL);

            //Even if we get multiple responses we only store 1 range
            p_tipc_env->p_env[conidx]->ndcs.svc.shdl = p_ind->start_hdl;
            p_tipc_env->p_env[conidx]->ndcs.svc.ehdl = p_ind->end_hdl;
        }

        if (attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_REF_TIME_UPDATE))
        {
            // Retrieve RTUS characteristics and descriptors
            prf_extract_svc_info(p_ind, TIPC_CHAR_RTUS_MAX, &tipc_rtus_char[0],  &p_tipc_env->p_env[conidx]->rtus.chars[0],
                    0, NULL, NULL);

            //Even if we get multiple responses we only store 1 range
            p_tipc_env->p_env[conidx]->rtus.svc.shdl = p_ind->start_hdl;
            p_tipc_env->p_env[conidx]->rtus.svc.ehdl = p_ind->end_hdl;
        }

        // Increment number of services
        p_tipc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPC_ENABLE_REQ message.
 * The handler enables the Time Profile Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tipc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct tipc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get state
    uint8_t state = ke_state_get(dest_id);

    // Time Service Client Role Task Environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);

    ASSERT_INFO(p_tipc_env != NULL, dest_id, src_id);
    if ((state == TIPC_IDLE) && (p_tipc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_tipc_env->p_env[conidx] = (struct tipc_cnx_env *) ke_malloc(sizeof(struct tipc_cnx_env), KE_MEM_ATT_DB);
        memset(p_tipc_env->p_env[conidx], 0, sizeof(struct tipc_cnx_env));

        // config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // start discovering CTS on peer
            prf_disc_svc_send(&(p_tipc_env->prf_env), conidx, ATT_SVC_CURRENT_TIME);

            p_tipc_env->p_env[conidx]->last_uuid_req = ATT_SVC_CURRENT_TIME;
            p_tipc_env->p_env[conidx]->last_svc_req  = ATT_SVC_CURRENT_TIME;

            // Go to DISCOVERING state
            ke_state_set(dest_id, TIPC_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_tipc_env->p_env[conidx]->cts = p_param->cts;
            p_tipc_env->p_env[conidx]->ndcs = p_param->ndcs;
            p_tipc_env->p_env[conidx]->rtus = p_param->rtus;

            // send APP confirmation that can start normal connection
            tipc_enable_rsp_send(p_tipc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }

    else if (state != TIPC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        tipc_enable_rsp_send(p_tipc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPC_RD_CHAR_CMD message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tipc_rd_char_cmd_handler(ke_msg_id_t const msgid,
                                        struct tipc_rd_char_cmd const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t status = GAP_ERR_NO_ERROR;

    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if ((ke_state_get(dest_id) == TIPC_IDLE) && (p_tipc_env->p_env[conidx] != NULL))
    {
        // Service
        struct prf_svc *p_svc = NULL;
        // Attribute Handle
        uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;

        // Next DST Change Service Characteristic
        if (((p_param->char_code & TIPC_CHAR_NDCS_MASK) == TIPC_CHAR_NDCS_MASK) &&
            ((p_param->char_code & ~TIPC_CHAR_NDCS_MASK) < TIPC_CHAR_NDCS_MAX))
        {
            p_svc = &p_tipc_env->p_env[conidx]->ndcs.svc;
            search_hdl = p_tipc_env->p_env[conidx]->ndcs.
                    chars[p_param->char_code & ~TIPC_CHAR_NDCS_MASK].val_hdl;
        }
        // Reference Time Update Service Characteristic
        else if (((p_param->char_code & TIPC_CHAR_RTUS_MASK) == TIPC_CHAR_RTUS_MASK) &&
                 ((p_param->char_code & ~TIPC_CHAR_RTUS_MASK) < TIPC_CHAR_RTUS_MAX))
        {
            p_svc = &p_tipc_env->p_env[conidx]->rtus.svc;
            search_hdl = p_tipc_env->p_env[conidx]->rtus.
                    chars[p_param->char_code & ~TIPC_CHAR_RTUS_MASK].val_hdl;
        }
        else
        {
            p_svc = &p_tipc_env->p_env[conidx]->cts.svc;

            // Current Time Characteristic Descriptor
            if (((p_param->char_code & TIPC_DESC_CTS_MASK) == TIPC_DESC_CTS_MASK) &&
                ((p_param->char_code & ~TIPC_DESC_CTS_MASK) < TIPC_DESC_CTS_MAX))
            {
                search_hdl = p_tipc_env->p_env[conidx]->cts.
                        descs[p_param->char_code & ~TIPC_DESC_CTS_MASK].desc_hdl;
            }
            // Current Time Service Characteristic
            else if (p_param->char_code < TIPC_CHAR_CTS_MAX)
            {
                search_hdl = p_tipc_env->p_env[conidx]->cts.
                        chars[p_param->char_code].val_hdl;
            }
        }

        // Check if handle is viable
        if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
        {
            // Save char code
            p_tipc_env->p_env[conidx]->last_char_code = p_param->char_code;
            // Send read request
            prf_read_char_send(&(p_tipc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);

            ke_state_set(dest_id, TIPC_BUSY);
        }
        else
        {
            status =  PRF_ERR_INEXISTENT_HDL;
        }
    }
    else
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_RD_CHAR_CMD_OP_CODE, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPC_CT_NTF_CFG_CMD message.
 * It allows configuration of the peer ind/ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tipc_ct_ntf_cfg_cmd_handler(ke_msg_id_t const msgid,
                                       struct tipc_ct_ntf_cfg_cmd const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if ((p_tipc_env->p_env[conidx] != NULL) && (ke_state_get(dest_id) == TIPC_IDLE))
    {
        // Only NTF
        if ((p_param->cfg_val == PRF_CLI_STOP_NTFIND) || (p_param->cfg_val == PRF_CLI_START_NTF))
        {
            uint16_t cfg_hdl = p_tipc_env->p_env[conidx]->cts.descs[TIPC_DESC_CTS_CURR_TIME_CLI_CFG].desc_hdl;

            // check if the handle value exists
            if (cfg_hdl != ATT_INVALID_SEARCH_HANDLE)
            {
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&p_tipc_env->prf_env, conidx, cfg_hdl, p_param->cfg_val);
                status = GAP_ERR_NO_ERROR;

                ke_state_set(dest_id, TIPC_BUSY);
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
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_CT_NTF_CFG_CMD_OP_CODE, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref TIPC_WR_TIME_UPD_CTNL_PT_CMD message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int tipc_wr_time_upd_ctnl_pt_cmd_handler(ke_msg_id_t const msgid,
                                                struct tipc_wr_time_upd_ctnl_pt_cmd const *p_param,
                                                ke_task_id_t const dest_id,
                                                ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if ((ke_state_get(dest_id) == TIPC_IDLE) && (p_tipc_env->p_env[conidx] != NULL))
    {
        // Check provided parameters
        if ((p_param->value == TIPS_TIME_UPD_CTNL_PT_GET) ||
                (p_param->value == TIPS_TIME_UPD_CTNL_PT_CANCEL))
        {
            if (p_tipc_env->p_env[conidx]->cts.chars[TIPC_CHAR_RTUS_TIME_UPD_CTNL_PT].char_hdl !=
                    ATT_INVALID_SEARCH_HANDLE)
            {
                // Send GATT Write Request
                prf_gatt_write(&p_tipc_env->prf_env, conidx,
                        p_tipc_env->p_env[conidx]->rtus.chars[TIPC_CHAR_RTUS_TIME_UPD_CTNL_PT].val_hdl,
                        (uint8_t *)&p_param->value, sizeof(uint8_t), GATTC_WRITE_NO_RESPONSE);

                status = GAP_ERR_NO_ERROR;

                ke_state_set(dest_id, TIPC_BUSY);
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
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_WR_TIME_UPD_CTNL_PT_CMD_OP_CODE, status);
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
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t status = PRF_ERR_STOP_DISC_CHAR_MISSING;
    bool finished = false;

    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);

    if ((state == TIPC_DISCOVERING) && (p_tipc_env->p_env[conidx] != NULL))
    {
        if ((p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||
                (p_param->status == ATT_ERR_NO_ERROR))
        {
            // check characteristic validity
            if (p_tipc_env->p_env[conidx]->nb_svc <= 1)
            {
                switch (p_tipc_env->p_env[conidx]->last_svc_req)
                {
                    case ATT_SVC_CURRENT_TIME:
                    {
                        // Check service (mandatory)
                        status = prf_check_svc_char_validity(TIPC_CHAR_CTS_MAX,
                                p_tipc_env->p_env[conidx]->cts.chars,
                                tipc_cts_char);

                        // Check Descriptors (mandatory)
                        if (status == GAP_ERR_NO_ERROR)
                        {
                            status = prf_check_svc_char_desc_validity(TIPC_DESC_CTS_MAX,
                                    p_tipc_env->p_env[conidx]->cts.descs,
                                    tipc_cts_char_desc,
                                    p_tipc_env->p_env[conidx]->cts.chars);
                            // Prepare to discovery next service
                            p_tipc_env->p_env[conidx]->last_svc_req = ATT_SVC_NEXT_DST_CHANGE;
                        }
                    } break;

                    case ATT_SVC_NEXT_DST_CHANGE:
                    {
                        // Check service (if found)
                        if (p_tipc_env->p_env[conidx]->nb_svc)
                        {
                            status = prf_check_svc_char_validity(TIPC_CHAR_NDCS_MAX,
                                    p_tipc_env->p_env[conidx]->ndcs.chars,
                                    tipc_ndcs_char);
                        }
                        else
                        {
                            status = GAP_ERR_NO_ERROR;
                        }
                        // Prepare to discovery next service
                        p_tipc_env->p_env[conidx]->last_svc_req = ATT_SVC_REF_TIME_UPDATE;
                    } break;

                    case ATT_SVC_REF_TIME_UPDATE:
                    {
                        // Check service (if found)
                        if (p_tipc_env->p_env[conidx]->nb_svc)
                        {
                            status = prf_check_svc_char_validity(TIPC_CHAR_RTUS_MAX,
                                    p_tipc_env->p_env[conidx]->rtus.chars,
                                    tipc_rtus_char);
                        }
                        else
                        {
                            status = GAP_ERR_NO_ERROR;
                        }

                        if (status == GAP_ERR_NO_ERROR)
                        {
                            // send app the details about the discovered TIPS DB to save
                            tipc_enable_rsp_send(p_tipc_env, conidx, GAP_ERR_NO_ERROR);
                            // Discovery is finished
                            finished = true;
                        }
                    } break;

                    default:
                    {

                    } break;
                }
            }
            // too many services
            else
            {
                status = PRF_ERR_MULTIPLE_SVC;
            }

            if (status == GAP_ERR_NO_ERROR)
            {
                // reset number of services
                p_tipc_env->p_env[conidx]->nb_svc = 0;

                if (!finished)
                {
                    // start discovering following service on peer
                    prf_disc_svc_send(&(p_tipc_env->prf_env), conidx, p_tipc_env->p_env[conidx]->last_svc_req);
                }
            }
            else
            {
                // stop discovery procedure
                tipc_enable_rsp_send(p_tipc_env, conidx, status);
            }
        }
    }
    else if (state == TIPC_BUSY)
    {
        switch (p_param->operation)
        {
            case GATTC_WRITE:
            {
                tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_CT_NTF_CFG_CMD_OP_CODE, p_param->status);
            } break;

            case GATTC_READ:
            {
                tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_RD_CHAR_CMD_OP_CODE, p_param->status);
            } break;

            case GATTC_WRITE_NO_RESPONSE:
            {
                tipc_send_cmp_evt(p_tipc_env, conidx, TIPC_WR_TIME_UPD_CTNL_PT_CMD_OP_CODE, p_param->status);
            } break;

            default:
            {

            } break;
        }
    }

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
    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    // Build the message
    struct tipc_rd_char_ind *p_ind = KE_MSG_ALLOC(
            TIPC_RD_CHAR_IND,
            prf_dst_task_get(&(p_tipc_env->prf_env), conidx),
            dest_id,
            tipc_rd_char_ind);

    // Current Time Characteristic
    if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_CTS_CURR_TIME)
    {
        p_ind->op_code = TIPC_RD_CTS_CURR_TIME;
        // Unpack Current Time Value.
        tipc_unpack_curr_time_value(&(p_ind->value.curr_time), (uint8_t *) p_param->value);
    }
    // Local Time Information Characteristic
    else if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_CTS_LOCAL_TIME_INFO)
    {
        p_ind->op_code = TIPC_RD_CTS_LOCAL_TIME_INFO;
        // Local Time Information Value
        p_ind->value.loc_time_info.time_zone         = p_param->value[0];
        p_ind->value.loc_time_info.dst_offset        = p_param->value[1];
    }
    // Reference Time Information Characteristic
    else if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_CTS_REF_TIME_INFO)
    {
        p_ind->op_code = TIPC_RD_CTS_REF_TIME_INFO;
        // Reference Time Information Value
        p_ind->value.ref_time_info.time_source      = p_param->value[0];
        p_ind->value.ref_time_info.time_accuracy    = p_param->value[1];
        p_ind->value.ref_time_info.days_update      = p_param->value[2];
        p_ind->value.ref_time_info.hours_update     = p_param->value[3];
    }
    // Time with DST Characteristic
    else if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_NDCS_TIME_WITH_DST)
    {
        p_ind->op_code = TIPC_RD_NDCS_TIME_WITH_DST;
        // Time with DST Value
        tipc_unpack_time_dst_value(&p_ind->value.time_with_dst, (uint8_t *) p_param->value);
    }
    // Time Update State Characteristic
    else if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_RTUS_TIME_UPD_STATE)
    {
        p_ind->op_code = TIPC_RD_RTUS_TIME_UPD_STATE;
        // Reference Time Information Value
        p_ind->value.time_upd_state.current_state     = p_param->value[0];
        p_ind->value.time_upd_state.result            = p_param->value[1];
    }
    // Current Time Characteristic - Client Characteristic Configuration Descriptor
    else if (p_tipc_env->p_env[conidx]->last_char_code == TIPC_RD_CTS_CURR_TIME_CLI_CFG)
    {
        p_ind->op_code = TIPC_RD_CTS_CURR_TIME_CLI_CFG;
        // Notification Configuration
        memcpy(&p_ind->value.ntf_cfg, &p_param->value[0], sizeof(uint16_t));
    }

    ke_msg_send(p_ind);

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
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (p_tipc_env->p_env[conidx] != NULL)
    {
        if (p_param->handle == p_tipc_env->p_env[conidx]->cts.chars[TIPC_CHAR_CTS_CURR_TIME].val_hdl)
        {
            // Build a TIPC_CT_IND message
            struct tipc_ct_ind *p_ind = KE_MSG_ALLOC(
                    TIPC_CT_IND,
                    prf_dst_task_get(&(p_tipc_env->prf_env), conidx),
                    prf_src_task_get(&(p_tipc_env->prf_env), conidx),
                    tipc_ct_ind);

            // Unpack Current Time Value.
            tipc_unpack_curr_time_value(&(p_ind->ct_val), (uint8_t*) p_param->value);

            ke_msg_send(p_ind);
        }
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(tipc)
{
    {TIPC_ENABLE_REQ,               (ke_msg_func_t)tipc_enable_req_handler},
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {TIPC_CT_NTF_CFG_CMD,           (ke_msg_func_t)tipc_ct_ntf_cfg_cmd_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {TIPC_RD_CHAR_CMD,              (ke_msg_func_t)tipc_rd_char_cmd_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {TIPC_WR_TIME_UPD_CTNL_PT_CMD,  (ke_msg_func_t)tipc_wr_time_upd_ctnl_pt_cmd_handler},
};

void tipc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct tipc_env_tag *p_tipc_env = PRF_ENV_GET(TIPC, tipc);

    p_task_desc->msg_handler_tab = tipc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(tipc_msg_handler_tab);
    p_task_desc->state           = p_tipc_env->state;
    p_task_desc->idx_max         = TIPC_IDX_MAX;
}

#endif /* (BLE_TIP_CLIENT) */
/// @} TIPCTASK
