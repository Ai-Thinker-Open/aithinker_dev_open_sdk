/**
 ****************************************************************************************
 *
 * @file scppc_task.c
 *
 * @brief Scan Parameters Profile Client Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup SCPPCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_SP_CLIENT)

#include "scpp_common.h"
#include "scppc_task.h"
#include "scppc.h"

#include "gap.h"
#include "attm.h"
#include "gattc_task.h"
#include "prf_types.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * GLOBAL VARIABLES DEFINITIONS
 ****************************************************************************************
 */

/// State machine used to retrieve Scan Parameters Service characteristics information
const struct prf_char_def scppc_scps_char[SCPPC_CHAR_MAX] =
{
    // Scan Interval Window
    [SCPPC_CHAR_SCAN_INTV_WD]      = {ATT_CHAR_SCAN_INTV_WD,
                                      ATT_MANDATORY,
                                      ATT_CHAR_PROP_WR_NO_RESP},
    // Scan Refresh
    [SCPPC_CHAR_SCAN_REFRESH]      = {ATT_CHAR_SCAN_REFRESH,
                                      ATT_OPTIONAL,
                                      ATT_CHAR_PROP_NTF},
};

/// State machine used to retrieve Scan Parameters Service characteristic description information
const struct prf_char_desc_def scppc_scps_char_desc[SCPPC_DESC_MAX] =
{
    // Boot Keyboard Input Report Client Config
    [SCPPC_DESC_SCAN_REFRESH_CFG]  = {ATT_DESC_CLIENT_CHAR_CFG, ATT_MANDATORY, SCPPC_CHAR_SCAN_REFRESH},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SCPPC_ENABLE_REQ message.
 * The handler enables the Scan Parameters Profile Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int scppc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct scppc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Battery service Client Role Task Environment
    struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

    ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);
    if ((state == SCPPC_IDLE) && (p_scppc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_scppc_env->p_env[conidx] = (struct scppc_cnx_env *) ke_malloc(sizeof(struct scppc_cnx_env), KE_MEM_ATT_DB);
        memset(p_scppc_env->p_env[conidx], 0, sizeof(struct scppc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            //start discovering SCPP on peer
            prf_disc_svc_send(&(p_scppc_env->prf_env), conidx, ATT_SVC_SCAN_PARAMETERS);

            // Go to DISCOVERING state
            ke_state_set(dest_id, SCPPC_BUSY);
            p_scppc_env->p_env[conidx]->operation = SCPPC_OP_DISCOVER;
        }
        // normal connection, get saved att details
        else
        {
            p_scppc_env->p_env[conidx]->nb_svc = 1;
            memcpy(&(p_scppc_env->p_env[conidx]->scps), &(p_param->scps), sizeof(struct scps_content));

            // send APP confirmation that can start normal connection to TH
            scppc_enable_rsp_send(p_scppc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != SCPPC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        scppc_enable_rsp_send(p_scppc_env, conidx, status);
    }

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
    if (ke_state_get(dest_id) == SCPPC_BUSY)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_scppc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_scppc_env->p_env[conidx]->nb_svc ==  0)
        {
            // Retrieve DIS characteristics
            prf_extract_svc_info(p_ind, SCPPC_CHAR_MAX, &scppc_scps_char[0],
                    &(p_scppc_env->p_env[conidx]->scps.chars[0]),
                    SCPPC_DESC_MAX, &scppc_scps_char_desc[0],
                    &(p_scppc_env->p_env[conidx]->scps.descs[0]));

            //Even if we get multiple responses we only store 1 range
            p_scppc_env->p_env[conidx]->scps.svc.shdl = p_ind->start_hdl;
            p_scppc_env->p_env[conidx]->scps.svc.ehdl = p_ind->end_hdl;
        }

        p_scppc_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SCPPC_SCAN_INTV_WD_WR_CMD message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int scppc_scan_intv_wd_wr_cmd_handler(ke_msg_id_t const msgid,
                                             struct scppc_scan_intv_wd_wr_cmd const *p_param,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if (state == SCPPC_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);

        // environment variable not ready
        if (p_scppc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            uint16_t handle = p_scppc_env->p_env[conidx]->scps.chars[SCPPC_CHAR_SCAN_INTV_WD].val_hdl;
            status = PRF_ERR_INEXISTENT_HDL;

            if (handle != ATT_INVALID_HANDLE)
            {
                uint8_t data[2*sizeof(uint16_t)];

                status = GAP_ERR_NO_ERROR;
                co_write16p(&(data[0]), p_param->scan_intv_wd.le_scan_intv);
                co_write16p(&(data[sizeof(uint16_t)]) , p_param->scan_intv_wd.le_scan_window);

                // Send GATT Write Request
                prf_gatt_write(&p_scppc_env->prf_env, conidx, handle, data, 2*sizeof(uint16_t), GATTC_WRITE_NO_RESPONSE);
                // store context of request and go into busy state
                p_scppc_env->p_env[conidx]->operation = SCPPC_OP_WR_SCAN_INTV;
                ke_state_set(dest_id, SCPPC_BUSY);
            }
        }
    }
    // process message later
    else if (state == SCPPC_BUSY)
    {
        status = GAP_ERR_NO_ERROR;
        msg_status = KE_MSG_SAVED;
    }

    // request cannot be performed
    if (status != GAP_ERR_NO_ERROR)
    {
        struct scppc_cmp_evt *p_evt = KE_MSG_ALLOC(SCPPC_CMP_EVT,
                src_id, dest_id, scppc_cmp_evt);

        // set error status
        p_evt->status = status;
        p_evt->operation = SCPPC_SCAN_INTV_WD_WR_CMD_OP_CODE;

        ke_msg_send(p_evt);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD message.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int scppc_scan_refresh_ntf_cfg_rd_cmd_handler(ke_msg_id_t const msgid,
                                                     void const *p_param,
                                                     ke_task_id_t const dest_id,
                                                     ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if (state == SCPPC_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);

        // environment variable not ready
        if (p_scppc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            uint16_t handle = p_scppc_env->p_env[conidx]->scps.descs[SCPPC_DESC_SCAN_REFRESH_CFG].desc_hdl;

            status = PRF_ERR_INEXISTENT_HDL;

            if (handle != ATT_INVALID_HANDLE)
            {
                status = GAP_ERR_NO_ERROR;
                // read information
                prf_read_char_send(&(p_scppc_env->prf_env), conidx,
                        p_scppc_env->p_env[conidx]->scps.svc.shdl,
                        p_scppc_env->p_env[conidx]->scps.svc.ehdl,  handle);

                // store context of request and go into busy state
                p_scppc_env->p_env[conidx]->operation = SCPPC_OP_RD_NTF_CFG;
                ke_state_set(dest_id, SCPPC_BUSY);
            }
        }
    }
    // process message later
    else if (state == SCPPC_BUSY)
    {
        status = GAP_ERR_NO_ERROR;
        msg_status = KE_MSG_SAVED;
    }

    // request cannot be performed
    if (status != GAP_ERR_NO_ERROR)
    {
        struct scppc_cmp_evt *p_evt = KE_MSG_ALLOC(SCPPC_CMP_EVT,
                src_id, dest_id, scppc_cmp_evt);

        // set error status
        p_evt->status = status;
        p_evt->operation = SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD_OP_CODE;

        ke_msg_send(p_evt);
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref SCPPC_SCAN_REFRESH_NTF_CFG_CMD message.
 * It allows configuration of the peer ntf/stop characteristic for a specified characteristic.
 * Will return an error code if that cfg char does not exist.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int scppc_scan_refresh_ntf_cfg_cmd_handler(ke_msg_id_t const msgid,
                                                  struct scppc_scan_refresh_ntf_cfg_cmd const *p_param,
                                                  ke_task_id_t const dest_id,
                                                  ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if (state == SCPPC_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);

        // environment variable not ready
        if (p_scppc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            uint16_t handle = p_scppc_env->p_env[conidx]->scps.descs[SCPPC_DESC_SCAN_REFRESH_CFG].desc_hdl;

            status = PRF_ERR_INEXISTENT_HDL;

            if (handle != ATT_INVALID_HANDLE)
            {
                status = GAP_ERR_NO_ERROR;
                // Send GATT Write Request
                prf_gatt_write_ntf_ind(&p_scppc_env->prf_env, conidx, handle, p_param->ntf_cfg);
                // store context of request and go into busy state
                p_scppc_env->p_env[conidx]->operation = SCPPC_OP_WR_NTF_CFG;
                ke_state_set(dest_id, SCPPC_BUSY);
            }
        }
    }
    // process message later
    else if (state == SCPPC_BUSY)
    {
        status = GAP_ERR_NO_ERROR;
        msg_status = KE_MSG_SAVED;
    }

    // request cannot be performed
    if (status != GAP_ERR_NO_ERROR)
    {
        struct scppc_cmp_evt *p_evt = KE_MSG_ALLOC(SCPPC_CMP_EVT,
        src_id, dest_id, scppc_cmp_evt);

        // set error status
        p_evt->status = status;
        p_evt->operation = SCPPC_SCAN_REFRESH_NTF_CFG_CMD_OP_CODE;

        ke_msg_send(p_evt);
    }

    return (msg_status);
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
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid, struct gattc_cmp_evt const *p_param,
                                ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

    // sanity check
    if ((ke_state_get(dest_id) == SCPPC_BUSY) && (p_scppc_env->p_env[conidx] != NULL)
            && (p_scppc_env->p_env[conidx]->operation != SCPPC_OP_NONE))
    {
        switch (p_scppc_env->p_env[conidx]->operation)
        {
            case SCPPC_OP_DISCOVER:
            {
                uint8_t status = p_param->status;

                if (p_param->status == ATT_ERR_NO_ERROR)
                {
                    // check characteristic validity
                    if (p_scppc_env->p_env[conidx]->nb_svc == 1)
                    {
                        status = prf_check_svc_char_validity(SCPPC_CHAR_MAX,
                                                             p_scppc_env->p_env[conidx]->scps.chars,
                                                             scppc_scps_char);

                        // check descriptor validity
                        if (status == GAP_ERR_NO_ERROR)
                        {
                            status = prf_check_svc_char_desc_validity(SCPPC_DESC_MAX,
                                    p_scppc_env->p_env[conidx]->scps.descs, scppc_scps_char_desc,
                                    p_scppc_env->p_env[conidx]->scps.chars);
                        }
                    }
                    // invalid number of services
                    else if (p_scppc_env->p_env[conidx]->nb_svc > 1)
                    {
                        status = PRF_ERR_MULTIPLE_SVC;
                    }
                    // no services found
                    else
                    {
                        status = PRF_ERR_STOP_DISC_CHAR_MISSING;
                    }
                }

                scppc_enable_rsp_send(p_scppc_env, conidx, status);
            } break;

            case SCPPC_OP_RD_NTF_CFG:
            {
                scppc_send_cmp_evt(p_scppc_env, conidx, SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD_OP_CODE, p_param->status);
            } break;

            case SCPPC_OP_WR_NTF_CFG:
            {
                scppc_send_cmp_evt(p_scppc_env, conidx, SCPPC_SCAN_REFRESH_NTF_CFG_CMD_OP_CODE, p_param->status);
            } break;

            case SCPPC_OP_WR_SCAN_INTV:
            {
                scppc_send_cmp_evt(p_scppc_env, conidx, SCPPC_SCAN_INTV_WD_WR_CMD_OP_CODE, p_param->status);
            } break;

            default:
            {
                // Not Expected at all
                ASSERT_ERR(0);
            } break;
        }

        // operation is over - go back to idle state
        p_scppc_env->p_env[conidx]->operation = SCPPC_OP_NONE;
        ke_state_set(dest_id, SCPPC_IDLE);
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
__STATIC int gattc_read_ind_handler(ke_msg_id_t const msgid, struct gattc_read_ind const *p_param,
                                  ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == SCPPC_BUSY)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        ASSERT_INFO(p_scppc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_scppc_env->p_env[conidx] != NULL, dest_id, src_id);

        struct scppc_scan_refresh_ntf_cfg_rd_ind *p_rsp = KE_MSG_ALLOC(SCPPC_SCAN_REFRESH_NTF_CFG_RD_IND,
                prf_dst_task_get(&(p_scppc_env->prf_env), conidx), dest_id, scppc_scan_refresh_ntf_cfg_rd_ind);

        p_rsp->ntf_cfg = co_read16p(&(p_param->value[0]));

        ke_msg_send(p_rsp);
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
    if (ke_state_get(dest_id) != SCPPC_FREE)
    {
        // Retrieve environment data
        struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

        // inform application that service requests to refresh
        ke_msg_send_basic(SCPPC_SCAN_REFRESH_IND,
                          prf_dst_task_get(&(p_scppc_env->prf_env),
                          KE_IDX_GET(src_id)),
                          dest_id);
    }

    return (KE_MSG_CONSUMED);
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Default State handlers definition
KE_MSG_HANDLER_TAB(scppc)
{
    {SCPPC_ENABLE_REQ,                      (ke_msg_func_t)scppc_enable_req_handler},
    {GATTC_SDP_SVC_IND,                     (ke_msg_func_t)gattc_sdp_svc_ind_handler},

    {SCPPC_SCAN_INTV_WD_WR_CMD,             (ke_msg_func_t)scppc_scan_intv_wd_wr_cmd_handler},
    {SCPPC_SCAN_REFRESH_NTF_CFG_RD_CMD,     (ke_msg_func_t)scppc_scan_refresh_ntf_cfg_rd_cmd_handler},
    {SCPPC_SCAN_REFRESH_NTF_CFG_CMD,        (ke_msg_func_t)scppc_scan_refresh_ntf_cfg_cmd_handler},

    {GATTC_EVENT_IND,                       (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_READ_IND,                        (ke_msg_func_t)gattc_read_ind_handler},

    {GATTC_CMP_EVT,                         (ke_msg_func_t)gattc_cmp_evt_handler},
};

void scppc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct scppc_env_tag *p_scppc_env = PRF_ENV_GET(SCPPC, scppc);

    p_task_desc->msg_handler_tab = scppc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(scppc_msg_handler_tab);
    p_task_desc->state           = p_scppc_env->state;
    p_task_desc->idx_max         = SCPPC_IDX_MAX;
}

#endif /* (BLE_SP_CLIENT) */
/// @} SCPPCTASK
