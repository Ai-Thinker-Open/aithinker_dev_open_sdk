/**
 ****************************************************************************************
 *
 * @file htpc_task.c
 *
 * @brief Health Thermometer Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HTPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_HT_COLLECTOR)
#include "co_utils.h"
#include "gap.h"
#include "attm.h"
#include "htp/htpc/api/htpc_task.h"
#include "htpc.h"
#include "gapc.h"
#include "gapc_task.h"

#include "ke_mem.h"

/// State machine used to retrieve Health Thermometer service characteristics information
const struct prf_char_def htpc_hts_char[HTPC_CHAR_HTS_MAX] =
{
    /// Temperature Measurement
    [HTPC_CHAR_HTS_TEMP_MEAS]        = {ATT_CHAR_TEMPERATURE_MEAS,     ATT_MANDATORY, ATT_CHAR_PROP_IND},
    /// Temperature Type
    [HTPC_CHAR_HTS_TEMP_TYPE]        = {ATT_CHAR_TEMPERATURE_TYPE,     ATT_OPTIONAL,  ATT_CHAR_PROP_RD},
    /// Intermediate Temperature
    [HTPC_CHAR_HTS_INTM_TEMP]        = {ATT_CHAR_INTERMED_TEMPERATURE, ATT_OPTIONAL,  ATT_CHAR_PROP_NTF},
    /// Measurement Interval
    [HTPC_CHAR_HTS_MEAS_INTV]        = {ATT_CHAR_MEAS_INTERVAL,        ATT_OPTIONAL,  ATT_CHAR_PROP_RD},
};

/// State machine used to retrieve Health Thermometer service characteristics description information
const struct prf_char_desc_def htpc_hts_char_desc[HTPC_DESC_HTS_MAX] =
{
    /// Temperature Measurement Client Config
    [HTPC_DESC_HTS_TEMP_MEAS_CLI_CFG]        = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, HTPC_CHAR_HTS_TEMP_MEAS},
    /// Intermediate Temperature Client Config
    [HTPC_DESC_HTS_INTM_MEAS_CLI_CFG]        = {ATT_DESC_CLIENT_CHAR_CFG, ATT_OPTIONAL,  HTPC_CHAR_HTS_INTM_TEMP},
    /// Measurement Interval Client Config
    [HTPC_DESC_HTS_MEAS_INTV_CLI_CFG]        = {ATT_DESC_CLIENT_CHAR_CFG, ATT_OPTIONAL,  HTPC_CHAR_HTS_MEAS_INTV},
    /// Measurement Interval valid range
    [HTPC_DESC_HTS_MEAS_INTV_VAL_RGE]        = {ATT_DESC_VALID_RANGE,     ATT_OPTIONAL,  HTPC_CHAR_HTS_MEAS_INTV},
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HTPC_ENABLE_REQ message.
 * The handler enables the Health Thermometer Profile Collector Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int htpc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct htpc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Health Thermometer Profile Collector Role Task Environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

    if ((ke_state_get(dest_id) == HTPC_IDLE) && (p_htpc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_htpc_env->p_env[conidx] = (struct htpc_cnx_env *) ke_malloc(sizeof(struct htpc_cnx_env), KE_MEM_ATT_DB);
        memset(p_htpc_env->p_env[conidx], 0, sizeof(struct htpc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering HTS on peer
            prf_disc_svc_send(&(p_htpc_env->prf_env), conidx, ATT_SVC_HEALTH_THERMOM);

            // store context of request and go into busy state
            p_htpc_env->p_env[conidx]->p_operation = ke_param2msg(p_param);
            msg_status = KE_MSG_NO_FREE;
            ke_state_set(dest_id, HTPC_BUSY);
        }
        // normal connection, get saved att details
        else
        {
            p_htpc_env->p_env[conidx]->hts = p_param->hts;

            //send APP confirmation that can start normal connection to TH
            htpc_enable_rsp_send(p_htpc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        htpc_enable_rsp_send(p_htpc_env, conidx, status);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND message.
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
    if (ke_state_get(dest_id) == HTPC_BUSY)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

        ASSERT_INFO(p_htpc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_htpc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_htpc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve IAS characteristic
            prf_extract_svc_info(p_ind, HTPC_CHAR_HTS_MAX, &htpc_hts_char[0],
                    p_htpc_env->p_env[conidx]->hts.chars,
                    HTPC_DESC_HTS_MAX,
                    &htpc_hts_char_desc[0],
                    &(p_htpc_env->p_env[conidx]->hts.descs[0]) );

            // Even if we get multiple responses we only store 1 range
            p_htpc_env->p_env[conidx]->hts.svc.shdl = p_ind->start_hdl;
            p_htpc_env->p_env[conidx]->hts.svc.ehdl = p_ind->end_hdl;
        }

        p_htpc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HTPC_WR_MEAS_INTV_REQ message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int htpc_wr_meas_intv_req_handler(ke_msg_id_t const msgid,
                                        struct htpc_wr_meas_intv_req const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

    ASSERT_INFO(p_htpc_env != NULL, dest_id, src_id);

    if ((state == HTPC_IDLE) && (p_htpc_env->p_env[conidx] != NULL))
    {
        uint16_t val_hdl  = ATT_INVALID_HANDLE;

        if ((p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_MEAS_INTV].prop & ATT_CHAR_PROP_WR)
                != ATT_CHAR_PROP_WR)
        {
            status = PRF_ERR_NOT_WRITABLE;
        }
        else
        {
            val_hdl  = p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_MEAS_INTV].val_hdl;

            if (val_hdl != ATT_INVALID_HANDLE)
            {
                // Send GATT Write Request
                prf_gatt_write(&(p_htpc_env->prf_env), conidx, val_hdl,
                        (uint8_t *)&p_param->intv, sizeof(uint16_t), GATTC_WRITE);

                // store context of request and go into busy state
                p_htpc_env->p_env[conidx]->p_operation = ke_param2msg(p_param);
                msg_status = KE_MSG_NO_FREE;
                ke_state_set(dest_id, HTPC_BUSY);
            }
            else
            {
                // invalid handle requested
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state != HTPC_FREE)
    {
        // request cannot be performed
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send error response if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        struct htpc_wr_meas_intv_rsp *p_rsp = KE_MSG_ALLOC(HTPC_WR_MEAS_INTV_RSP, src_id,
                dest_id, htpc_wr_meas_intv_rsp);

        p_rsp->status = status;
        ke_msg_send(p_rsp);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HTPC_HEALTH_TEMP_NTF_CFG_REQ message.
 * It allows configuration of the peer ind/ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int htpc_health_temp_ntf_cfg_req_handler(ke_msg_id_t const msgid,
                                       struct htpc_health_temp_ntf_cfg_req const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Client Characteristic Configuration Descriptor Handle
    uint16_t cfg_hdl = 0x0000;
    uint8_t status = GAP_ERR_NO_ERROR;
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    switch(p_param->char_code)
    {
        case HTPC_CHAR_HTS_TEMP_MEAS: //can only IND
        {
            cfg_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_TEMP_MEAS_CLI_CFG].desc_hdl;
            if (!((p_param->cfg_val == PRF_CLI_STOP_NTFIND)||
                 (p_param->cfg_val == PRF_CLI_START_IND)))
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        case HTPC_CHAR_HTS_MEAS_INTV: //can only IND
        {
            cfg_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_MEAS_INTV_CLI_CFG].desc_hdl;
            if (!((p_param->cfg_val == PRF_CLI_STOP_NTFIND)||
                 (p_param->cfg_val == PRF_CLI_START_IND)))
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        case HTPC_CHAR_HTS_INTM_TEMP: //can only NTF
        {
            cfg_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_INTM_MEAS_CLI_CFG].desc_hdl;
            if (!((p_param->cfg_val == PRF_CLI_STOP_NTFIND)||
                 (p_param->cfg_val == PRF_CLI_START_NTF)))
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        } break;

        default:
        {
            //Let app know that one of the request p_params is incorrect
            status = PRF_ERR_INVALID_PARAM;
        } break;
    }

    //Check if the handle value exists
    if ((cfg_hdl == ATT_INVALID_HANDLE))
    {
        status = PRF_ERR_INEXISTENT_HDL;
    }

    // no error detected
    if (status == GAP_ERR_NO_ERROR)
    {
        // Send GATT Write Request
        prf_gatt_write_ntf_ind(&p_htpc_env->prf_env, conidx, cfg_hdl, p_param->cfg_val);
        // store context of request and go into busy state
        p_htpc_env->p_env[conidx]->p_operation = ke_param2msg(p_param);
        ke_state_set(dest_id, HTPC_BUSY);
        msg_status = KE_MSG_NO_FREE;
    }
    // an error occurs send back response message
    else
    {
        struct htpc_health_temp_ntf_cfg_rsp *p_rsp = KE_MSG_ALLOC(HTPC_HEALTH_TEMP_NTF_CFG_RSP,
                                                                src_id, dest_id,
                                                                htpc_health_temp_ntf_cfg_rsp);

        p_rsp->status = status;
        ke_msg_send(p_rsp);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HTPC_RD_REQ message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int htpc_rd_char_req_handler(ke_msg_id_t const msgid,
                                    struct htpc_rd_char_req const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

    ASSERT_INFO(p_htpc_env != NULL, dest_id, src_id);
    if ((state == HTPC_IDLE) && (p_htpc_env->p_env[conidx] != NULL))
    {
        uint16_t search_hdl = ATT_INVALID_HDL;

        switch (p_param->char_code)
        {
            // Read HTS Temp. Type
            case HTPC_RD_TEMP_TYPE:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_TEMP_TYPE].val_hdl;
            } break;
            // Read HTS Measurement Interval
            case HTPC_RD_MEAS_INTV:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_MEAS_INTV].val_hdl;
            } break;
            // Read HTS Temperature Measurement Client Cfg. Desc
            case HTPC_RD_TEMP_MEAS_CLI_CFG:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_TEMP_MEAS_CLI_CFG].desc_hdl;
            } break;
            // Read HTS Intermediate Temperature Client Cfg. Desc
            case HTPC_RD_INTM_TEMP_CLI_CFG:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_INTM_MEAS_CLI_CFG].desc_hdl;
            } break;
            // Read HTS Measurement Interval Client Cfg. Desc
            case HTPC_RD_MEAS_INTV_CLI_CFG:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_MEAS_INTV_CLI_CFG].desc_hdl;
            } break;
            // Read HTS Measurement Interval Client Cfg. Desc
            case HTPC_RD_MEAS_INTV_VAL_RGE:
            {
                search_hdl = p_htpc_env->p_env[conidx]->hts.descs[HTPC_DESC_HTS_MEAS_INTV_VAL_RGE].desc_hdl;
            } break;
            default:
            {
                status = GAP_ERR_INVALID_PARAM;
            } break;
        }

        //Check if handle is viable
        if (search_hdl != ATT_INVALID_HDL)
        {
            // perform read request
            prf_read_char_send(&(p_htpc_env->prf_env), conidx, p_htpc_env->p_env[conidx]->hts.svc.shdl,
                    p_htpc_env->p_env[conidx]->hts.svc.ehdl, search_hdl);

            // store context of request and go into busy state
            p_htpc_env->p_env[conidx]->p_operation = ke_param2msg(p_param);
            msg_status = KE_MSG_NO_FREE;
            ke_state_set(dest_id, HTPC_BUSY);
        }
        else if (status == GAP_ERR_NO_ERROR)
        {
            // invalid handle requested
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else if (state != HTPC_FREE)
    {
        // request cannot be performed
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send error response if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        prf_client_att_info_rsp(&(p_htpc_env->prf_env), conidx, HTPC_RD_CHAR_RSP,
                status, NULL);
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
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    // send attribute information
    prf_client_att_info_rsp(&(p_htpc_env->prf_env), conidx, HTPC_RD_CHAR_RSP, GAP_ERR_NO_ERROR, p_param);

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
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

    if (ke_state_get(dest_id) == HTPC_BUSY)
    {
        switch(p_htpc_env->p_env[conidx]->p_operation->id)
        {
            case HTPC_ENABLE_REQ:
            {
                uint8_t status;

                if (p_htpc_env->p_env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(HTPC_CHAR_HTS_MAX,
                            p_htpc_env->p_env[conidx]->hts.chars, htpc_hts_char);

                    p_htpc_env->p_env[conidx]->nb_svc = 0;
                }
                // Too many services found only one such service should exist
                else if (p_htpc_env->p_env[conidx]->nb_svc > 1)
                {
                    status = PRF_ERR_MULTIPLE_SVC;
                }
                else
                {
                    status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                }

                htpc_enable_rsp_send(p_htpc_env, conidx, status);
            } break;

            case HTPC_HEALTH_TEMP_NTF_CFG_REQ:
            {
                // send response of the notification configuration request
                struct htpc_health_temp_ntf_cfg_rsp *p_rsp = KE_MSG_ALLOC(HTPC_HEALTH_TEMP_NTF_CFG_RSP,
                        prf_dst_task_get(&(p_htpc_env->prf_env), conidx), dest_id, htpc_health_temp_ntf_cfg_rsp);

                // set error status
                p_rsp->status =p_param->status;
                ke_msg_send(p_rsp);
            } break;

            case HTPC_RD_CHAR_REQ:
            {
                if(p_param->status != GAP_ERR_NO_ERROR)
                {
                    // send attribute information
                    prf_client_att_info_rsp(&(p_htpc_env->prf_env), conidx, HTPC_RD_CHAR_RSP, p_param->status, NULL);
                }
            } break;

            case HTPC_WR_MEAS_INTV_REQ:
            {
                struct htpc_wr_meas_intv_rsp *p_rsp = KE_MSG_ALLOC(HTPC_WR_MEAS_INTV_RSP,
                        prf_dst_task_get(&(p_htpc_env->prf_env), conidx), dest_id, htpc_wr_meas_intv_rsp);

                p_rsp->status = p_param->status;
                ke_msg_send(p_rsp);
            } break;

            default: break;
        }

        //free operation
        if ((p_htpc_env->p_env[conidx] != NULL) && (p_htpc_env->p_env[conidx]->p_operation != NULL))
        {
            ke_free(p_htpc_env->p_env[conidx]->p_operation);
            p_htpc_env->p_env[conidx]->p_operation = NULL;
        }
        // set state to IDLE
        ke_state_set(dest_id, HTPC_IDLE);
    }

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
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    // Temperature Measurement Char.
    if (p_param->handle == p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_TEMP_MEAS].val_hdl)
    {
        // confirm that indication has been correctly received
        struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

        p_cfm->handle = p_param->handle;
        ke_msg_send(p_cfm);

        htpc_unpack_temp(p_htpc_env, (uint8_t *)&(p_param->value), p_param->length,
                ((p_param->type == GATTC_NOTIFY) ? HTP_TEMP_INTERM : HTP_TEMP_STABLE), conidx);
    }
    // Intermediate Temperature Measurement Char.
    else if(p_param->handle == p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_INTM_TEMP].val_hdl)
    {
        htpc_unpack_temp(p_htpc_env, (uint8_t *)&(p_param->value), p_param->length,
                ((p_param->type == GATTC_NOTIFY) ? HTP_TEMP_INTERM : HTP_TEMP_STABLE), conidx);
    }
    // Measurement Interval Char.
    else if (p_param->handle == p_htpc_env->p_env[conidx]->hts.chars[HTPC_CHAR_HTS_MEAS_INTV].val_hdl)
    {
        // confirm that indication has been correctly received
        struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

        p_cfm->handle = p_param->handle;
        ke_msg_send(p_cfm);

        struct htpc_meas_intv_ind *p_ind = KE_MSG_ALLOC(HTPC_MEAS_INTV_IND,
                                            prf_dst_task_get(&p_htpc_env->prf_env, conidx),
                                            prf_src_task_get(&p_htpc_env->prf_env, conidx),
                                            htpc_meas_intv_ind);

        memcpy(&p_ind->intv, &p_param->value[0], sizeof(uint16_t));

        ke_msg_send(p_ind);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/* Default State handlers definition. */
KE_MSG_HANDLER_TAB(htpc)
{
    {HTPC_ENABLE_REQ,               (ke_msg_func_t) htpc_enable_req_handler},
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t) gattc_sdp_svc_ind_handler},
    {HTPC_HEALTH_TEMP_NTF_CFG_REQ,  (ke_msg_func_t) htpc_health_temp_ntf_cfg_req_handler},
    {HTPC_WR_MEAS_INTV_REQ,         (ke_msg_func_t) htpc_wr_meas_intv_req_handler},
    {HTPC_RD_CHAR_REQ,              (ke_msg_func_t) htpc_rd_char_req_handler},
    {GATTC_READ_IND,                (ke_msg_func_t) gattc_read_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t) gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,           (ke_msg_func_t) gattc_event_ind_handler},
};

void htpc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct htpc_env_tag *p_htpc_env = PRF_ENV_GET(HTPC, htpc);

    p_task_desc->msg_handler_tab = htpc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(htpc_msg_handler_tab);
    p_task_desc->state           = p_htpc_env->state;
    p_task_desc->idx_max         = HTPC_IDX_MAX;
}

#endif //BLE_HT_COLLECTOR

/// @} HTPCTASK
