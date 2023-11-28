/**
 ****************************************************************************************
 *
 * @file disc_task.c
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
 * @addtogroup DISCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "rwprf_config.h"
#if (BLE_DIS_CLIENT)
#include "co_utils.h"
#include "gap.h"
#include "../api/disc_task.h"
#include "disc.h"
#include "gattc_task.h"

#include "ke_mem.h"

#include "utils/debug/log.h"


/// State machine used to retrieve Device Information Service characteristics information
const struct prf_char_def disc_dis_char[DISC_CHAR_MAX] =
{
    // Manufacturer Name
    [DISC_MANUFACTURER_NAME_CHAR] = {ATT_CHAR_MANUF_NAME,  ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // Model Number String
    [DISC_MODEL_NB_STR_CHAR]      = {ATT_CHAR_MODEL_NB,    ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // Serial Number String
    [DISC_SERIAL_NB_STR_CHAR]     = {ATT_CHAR_SERIAL_NB,   ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // Hardware Revision String
    [DISC_HARD_REV_STR_CHAR]      = {ATT_CHAR_HW_REV,      ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // Firmware Revision String
    [DISC_FIRM_REV_STR_CHAR]      = {ATT_CHAR_FW_REV,      ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // TSoftware Revision String
    [DISC_SW_REV_STR_CHAR]        = {ATT_CHAR_SW_REV,      ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // System ID
    [DISC_SYSTEM_ID_CHAR]         = {ATT_CHAR_SYS_ID,      ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // IEEE
    [DISC_IEEE_CHAR]              = {ATT_CHAR_IEEE_CERTIF, ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    // PnP ID
    [DISC_PNP_ID_CHAR]            = {ATT_CHAR_PNP_ID,      ATT_OPTIONAL, ATT_CHAR_PROP_RD},
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref DISC_ENABLE_REQ message.
 * The handler enables the Device Information Service Client Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int disc_enable_req_handler(ke_msg_id_t const msgid,
                                   struct disc_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);

    LOG(LOG_LVL_INFO, "disc_enable_req_handler: con_type=%d\r\n", p_param->con_type);

    ASSERT_INFO(p_disc_env != NULL, dest_id, src_id);
    if ((state == DISC_IDLE) && (p_disc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_disc_env->p_env[conidx] = (struct disc_cnx_env *) ke_malloc(sizeof(struct disc_cnx_env), KE_MEM_ATT_DB);
        memset(p_disc_env->p_env[conidx], 0, sizeof(struct disc_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // start discovering DIS on peer
            prf_disc_svc_send(&(p_disc_env->prf_env), conidx, ATT_SVC_DEVICE_INFO);

            // Go to DISCOVERING state
            ke_state_set(dest_id, DISC_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_disc_env->p_env[conidx]->dis = p_param->dis;

            // send APP confirmation that can start normal connection to TH
            disc_enable_rsp_send(p_disc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != DISC_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        disc_enable_rsp_send(p_disc_env, conidx, status);
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
    LOG(LOG_LVL_INFO, "gattc_sdp_svc_ind_handler\r\n");

    if (ke_state_get(dest_id) == DISC_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);

        ASSERT_INFO(p_disc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_disc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_disc_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve DIS characteristics
            prf_extract_svc_info(p_ind, DISC_CHAR_MAX, &disc_dis_char[0], &(p_disc_env->p_env[conidx]->dis.chars[0]),
                    0, NULL, NULL);

            //Even if we get multiple responses we only store 1 range
            p_disc_env->p_env[conidx]->dis.svc.shdl = p_ind->start_hdl;
            p_disc_env->p_env[conidx]->dis.svc.ehdl = p_ind->end_hdl;
        }

        p_disc_env->p_env[conidx]->nb_svc++;
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
    // Get the address of the environment
    struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);
    uint8_t conidx = KE_IDX_GET(dest_id);

    LOG(LOG_LVL_INFO, "gattc_cmp_evt_handler: state=%d,status=%d,operation=0x%x, seq_num=0x%x\r\n",
                        state, p_param->status,p_param->operation, p_param->seq_num);

    if (state == DISC_DISCOVERING)
    {
        uint8_t status = p_param->status;

        if (p_param->status == ATT_ERR_NO_ERROR)
        {
            if (p_disc_env->p_env[conidx]->nb_svc ==  1)
            {
                status = prf_check_svc_char_validity(DISC_CHAR_MAX, p_disc_env->p_env[conidx]->dis.chars,
                                    disc_dis_char);
            }
            // too much services
            else if (p_disc_env->p_env[conidx]->nb_svc > 1)
            {
                status = PRF_ERR_MULTIPLE_SVC;
            }
            // no services found
            else
            {
                status = PRF_ERR_STOP_DISC_CHAR_MISSING;
            }
        }

        disc_enable_rsp_send(p_disc_env, conidx, status);
    }
    else if (state == DISC_BUSY)
    {
        if (p_param->operation == GATTC_READ)
        {
            disc_send_cmp_evt(p_disc_env, conidx, DISC_RD_CHAR_CMD_OP_CODE, p_param->status);
        }

        ke_state_set(dest_id, DISC_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref DISC_RD_CHAR_CMD message.
 * Check if the handle exists in profile(already discovered) and send request, otherwise
 * error to APP.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int disc_rd_char_cmd_handler(ke_msg_id_t const msgid,
                                    struct disc_rd_char_cmd const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);

    LOG(LOG_LVL_INFO, "disc_rd_char_cmd_handler code=0x%x\r\n",p_param->char_code);

    ASSERT_INFO(p_disc_env != NULL, dest_id, src_id);
    if ((state == DISC_IDLE) && (p_disc_env->p_env[conidx] != NULL))
    {
        uint16_t search_hdl = ATT_INVALID_HDL;

        // retrieve search handle
        if (p_param->char_code < DISC_CHAR_MAX)
        {
            search_hdl = p_disc_env->p_env[conidx]->dis.chars[p_param->char_code].val_hdl;
        }

        //Check if handle is viable
        if (search_hdl != ATT_INVALID_HDL)
        {
            // perform read request
            p_disc_env->p_env[conidx]->last_char_code = p_param->char_code;
            prf_read_char_send(&(p_disc_env->prf_env), conidx, p_disc_env->p_env[conidx]->dis.svc.shdl,
                    p_disc_env->p_env[conidx]->dis.svc.ehdl, search_hdl);

            // enter in a busy state
            ke_state_set(dest_id, DISC_BUSY);
        }
        else
        {
            // invalid handle requested
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else if (state != DISC_FREE)
    {
        // request cannot be performed
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send error response if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        disc_send_cmp_evt(p_disc_env, conidx, DISC_RD_CHAR_CMD_OP_CODE, status);
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
    LOG(LOG_LVL_INFO, "disc_task gattc_read_ind_handler handle=0x%x,length=%d, offset=%d\r\n", 
                       p_param->handle, p_param->length, p_param->offset);

    if (ke_state_get(dest_id) == DISC_BUSY)
    {
        // Get the address of the environment
        struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);

        ASSERT_INFO(p_disc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_disc_env->p_env[KE_IDX_GET(dest_id)] != NULL, dest_id, src_id);

        prf_client_att_info_rsp(&(p_disc_env->prf_env), KE_IDX_GET(dest_id), DISC_RD_CHAR_IND,
                GAP_ERR_NO_ERROR, p_param);
    }

    return (KE_MSG_CONSUMED);
}

/// Default State handlers definition
KE_MSG_HANDLER_TAB(disc)
{
    {DISC_RD_CHAR_CMD,       (ke_msg_func_t)disc_rd_char_cmd_handler},
    {GATTC_READ_IND,         (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_SDP_SVC_IND,      (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {DISC_ENABLE_REQ,        (ke_msg_func_t)disc_enable_req_handler},
    {GATTC_CMP_EVT,          (ke_msg_func_t)gattc_cmp_evt_handler},
};

void disc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct disc_env_tag *p_disc_env = PRF_ENV_GET(DISC, disc);

    p_task_desc->msg_handler_tab = disc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(disc_msg_handler_tab);
    p_task_desc->state           = p_disc_env->state;
    p_task_desc->idx_max         = DISC_IDX_MAX;
}

#endif //BLE_DIS_CLIENT

/// @} DISCTASK
