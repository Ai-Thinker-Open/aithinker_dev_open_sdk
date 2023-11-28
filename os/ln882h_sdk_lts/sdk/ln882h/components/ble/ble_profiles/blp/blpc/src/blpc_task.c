/**
 ****************************************************************************************
 *
 * @file blpc_task.c
 *
 * @brief Blood Pressure Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup BLPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_BP_COLLECTOR)
#include "co_utils.h"
#include "blpc_task.h"
#include "blpc.h"
#include "gap.h"
#include "att.h"
#include "gattc_task.h"
#include "gapc_task.h"
#include "ke_mem.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// State machine used to retrieve blood pressure service characteristics information
const struct prf_char_def blpc_bps_char[BLPC_CHAR_MAX] =
{
    // Blood Pressure Measurement
    [BLPC_CHAR_BP_MEAS]            = {ATT_CHAR_BLOOD_PRESSURE_MEAS,
                                   ATT_MANDATORY,
                                   ATT_CHAR_PROP_IND},
    // Intermediate Cuff pressure
    [BLPC_CHAR_CP_MEAS]            = {ATT_CHAR_INTERMEDIATE_CUFF_PRESSURE,
                                   ATT_OPTIONAL,
                                   ATT_CHAR_PROP_NTF},
    // Blood Pressure Feature
    [BLPC_CHAR_BP_FEATURE]         = {ATT_CHAR_BLOOD_PRESSURE_FEATURE,
                                   ATT_MANDATORY,
                                   ATT_CHAR_PROP_RD},
};

/// State machine used to retrieve blood pressure service characteristic description information
const struct prf_char_desc_def blpc_bps_char_desc[BLPC_DESC_MAX] =
{
    // Blood Pressure Measurement client config
    [BLPC_DESC_BP_MEAS_CLI_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, BLPC_CHAR_BP_MEAS},
    // Intermediate Cuff pressure client config
    [BLPC_DESC_IC_MEAS_CLI_CFG] = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, BLPC_CHAR_CP_MEAS},
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
    if (ke_state_get(dest_id) == BLPC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);

        struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);

        ASSERT_INFO(p_blpc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_blpc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_blpc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve BLS characteristics and descriptors
            prf_extract_svc_info(p_ind, BLPC_CHAR_MAX, &blpc_bps_char[0],  &p_blpc_env->p_env[conidx]->bps.chars[0],
                                    BLPC_DESC_MAX, &blpc_bps_char_desc[0], &p_blpc_env->p_env[conidx]->bps.descs[0]);

            // Even if we get multiple responses we only store 1 range
            p_blpc_env->p_env[conidx]->bps.svc.shdl = p_ind->start_hdl;
            p_blpc_env->p_env[conidx]->bps.svc.ehdl = p_ind->end_hdl;
        }

        p_blpc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BLPC_ENABLE_REQ message.
 * The handler enables the Blood Pressure Profile Collector Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int blpc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct blpc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get state
    uint8_t state = ke_state_get(dest_id);

    // Blood pressure Client Role Task Environment
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);

    ASSERT_INFO(p_blpc_env != NULL, dest_id, src_id);
    if ((state == BLPC_IDLE) && (p_blpc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_blpc_env->p_env[conidx] = (struct blpc_cnx_env *) ke_malloc(sizeof(struct blpc_cnx_env), KE_MEM_ATT_DB);
        memset(p_blpc_env->p_env[conidx], 0, sizeof(struct blpc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering BPS on peer
            prf_disc_svc_send(&(p_blpc_env->prf_env), conidx, ATT_SVC_BLOOD_PRESSURE);

            p_blpc_env->p_env[conidx]->last_uuid_req = ATT_SVC_BLOOD_PRESSURE;

            // Go to DISCOVERING state
            ke_state_set(dest_id, BLPC_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_blpc_env->p_env[conidx]->bps = p_param->bps;

            // send APP confirmation that can start normal connection to TH
            blpc_enable_rsp_send(p_blpc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }

    else if (state != BLPC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        blpc_enable_rsp_send(p_blpc_env, conidx, status);
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
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);

    if (p_blpc_env != NULL)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == BLPC_DISCOVERING)
        {
            // Status
            uint8_t status = p_param->status;

            if ((status == ATT_ERR_ATTRIBUTE_NOT_FOUND)||
                (status == ATT_ERR_NO_ERROR))
            {
                // Discovery
                // check characteristic validity
                if (p_blpc_env->p_env[conidx]->nb_svc == 1)
                {
                    status = prf_check_svc_char_validity(BLPC_CHAR_MAX,
                            p_blpc_env->p_env[conidx]->bps.chars,
                            blpc_bps_char);
                }
                // too much services
                else if (p_blpc_env->p_env[conidx]->nb_svc > 1)
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
                    status = prf_check_svc_char_desc_validity(BLPC_DESC_MAX,
                            p_blpc_env->p_env[conidx]->bps.descs,
                            blpc_bps_char_desc,
                            p_blpc_env->p_env[conidx]->bps.chars);
                }
            }

            blpc_enable_rsp_send(p_blpc_env, conidx, status);
        }

        else if (state == BLPC_BUSY)
        {
            switch (p_param->operation)
            {
                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    blpc_send_cmp_evt(p_blpc_env, conidx, BLPC_CFG_INDNTF_CMD_OP_CODE, p_param->status);
                } break;

                case GATTC_READ:
                {
                    blpc_send_cmp_evt(p_blpc_env, conidx, BLPC_RD_CHAR_CMD_OP_CODE, p_param->status);
                } break;

                default:
                {

                } break;
            }

            ke_state_set(prf_src_task_get(&p_blpc_env->prf_env, conidx), BLPC_IDLE);
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BLPC_RD_CHAR_CMD message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int blpc_rd_char_cmd_handler(ke_msg_id_t const msgid,
                                        struct blpc_rd_char_cmd const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Get the address of the environment
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if ((ke_state_get(dest_id) == BLPC_IDLE) && (p_blpc_env->p_env[conidx] != NULL))
    {
        uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;

        if ((p_param->char_code & BLPC_DESC_MASK) == BLPC_DESC_MASK)
        {
            search_hdl = p_blpc_env->p_env[conidx]->bps.descs[p_param->char_code & ~BLPC_DESC_MASK].desc_hdl;
        }
        else
        {
            search_hdl = p_blpc_env->p_env[conidx]->bps.chars[p_param->char_code].val_hdl;
        }

        // check if handle is viable
        if (search_hdl != ATT_INVALID_SEARCH_HANDLE)
        {
            prf_read_char_send(
                    &(p_blpc_env->prf_env), conidx,
                    p_blpc_env->p_env[conidx]->bps.svc.shdl,
                    p_blpc_env->p_env[conidx]->bps.svc.ehdl,
                    search_hdl);

            status = GAP_ERR_NO_ERROR;
            ke_state_set(dest_id, BLPC_BUSY);
        }
        else
        {
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        blpc_send_cmp_evt(p_blpc_env, conidx, BLPC_RD_CHAR_CMD_OP_CODE, status);
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
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (p_blpc_env ->p_env[conidx] != NULL)
    {
        ASSERT_ERR(ke_state_get(dest_id) == BLPC_BUSY);

        prf_client_att_info_rsp(&p_blpc_env->prf_env, KE_IDX_GET(dest_id), BLPC_RD_CHAR_IND,
                GAP_ERR_NO_ERROR, p_param);

    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref BLPC_CFG_INDNTF_CMD message.
 * It allows configuration of the peer ind/ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int blpc_cfg_indntf_cmd_handler(ke_msg_id_t const msgid,
                                       struct blpc_cfg_indntf_cmd const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if ((p_blpc_env->p_env[conidx] != NULL) && (ke_state_get(dest_id) == BLPC_IDLE))
    {
        uint16_t cfg_hdl = ATT_INVALID_SEARCH_HANDLE;

        status = PRF_ERR_INVALID_PARAM;

        // get handle of the configuration characteristic to set and check if value matches property
        switch (p_param->char_code)
        {
            case BLPC_CHAR_BP_MEAS://can only IND
            {
                if ((p_param->cfg_val == PRF_CLI_STOP_NTFIND)||
                     (p_param->cfg_val == PRF_CLI_START_IND))
                {
                    cfg_hdl = p_blpc_env->p_env[conidx]->bps.descs[BLPC_DESC_BP_MEAS_CLI_CFG].desc_hdl;
                }
            } break;

            case BLPC_CHAR_CP_MEAS://can only NTF
            {
                if ((p_param->cfg_val == PRF_CLI_STOP_NTFIND)||
                     (p_param->cfg_val == PRF_CLI_START_NTF))
                {
                    cfg_hdl = p_blpc_env->p_env[conidx]->bps.descs[BLPC_DESC_IC_MEAS_CLI_CFG].desc_hdl;
                }

            } break;

            default:
            {
                status = PRF_ERR_INEXISTENT_HDL;
            } break;
        }

        //check if the handle value exists
        if (cfg_hdl != ATT_INVALID_SEARCH_HANDLE)
        {
            // Send GATT Write Request
            prf_gatt_write_ntf_ind(&p_blpc_env->prf_env, conidx, cfg_hdl, p_param->cfg_val);
            // Set status
            status = GAP_ERR_NO_ERROR;
            ke_state_set(dest_id, BLPC_BUSY);
        }
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // Send error message to application
        blpc_send_cmp_evt(p_blpc_env, conidx, BLPC_CFG_INDNTF_CMD_OP_CODE, status);
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
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);

    if (p_blpc_env->p_env[conidx] != NULL)
    {
        switch (p_param->type)
        {
            case GATTC_INDICATE:
            {
                // confirm that indication has been correctly received
                struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

                p_cfm->handle = p_param->handle;
                ke_msg_send(p_cfm);
            }
            // no break

            case GATTC_NOTIFY:
            {
                if((p_param->handle == p_blpc_env->p_env[conidx]->bps.chars[BLPC_CHAR_CP_MEAS].val_hdl)
                    || ((p_param->handle == p_blpc_env->p_env[conidx]->bps.chars[BLPC_CHAR_BP_MEAS].val_hdl)))
                {
                    // build a BLPC_BP_MEAD_IND message with stable blood pressure value code.
                    struct blpc_bp_meas_ind * p_ind = KE_MSG_ALLOC(
                            BLPC_BP_MEAS_IND,
                            prf_dst_task_get(&(p_blpc_env->prf_env), conidx),
                            prf_src_task_get(&(p_blpc_env->prf_env), conidx),
                            blpc_bp_meas_ind);

                    // Intermediate cuff pressure value
                    p_ind->flag_interm_cp = ((p_param->type == GATTC_NOTIFY) ? 0x01 : 0x00);

                    // unpack blood pressure measurement.
                    blpc_unpack_meas_value(&(p_ind->meas_val), (uint8_t*) p_param->value);

                    ke_msg_send(p_ind);
                }
            } break;

            default:
            {

            } break;
        }
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(blpc)
{
    {GATTC_SDP_SVC_IND,             (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {BLPC_ENABLE_REQ,               (ke_msg_func_t)blpc_enable_req_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
    {BLPC_RD_CHAR_CMD,              (ke_msg_func_t)blpc_rd_char_cmd_handler},
    {GATTC_READ_IND,                (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_EVENT_IND,               (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,           (ke_msg_func_t)gattc_event_ind_handler},
    {BLPC_CFG_INDNTF_CMD,           (ke_msg_func_t)blpc_cfg_indntf_cmd_handler},
};

void blpc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct blpc_env_tag *p_blpc_env = PRF_ENV_GET(BLPC, blpc);

    p_task_desc->msg_handler_tab = blpc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(blpc_msg_handler_tab);
    p_task_desc->state           = p_blpc_env->state;
    p_task_desc->idx_max         = BLPC_IDX_MAX;
}

#endif /* (BLE_BP_COLLECTOR) */
/// @} BLPCTASK
