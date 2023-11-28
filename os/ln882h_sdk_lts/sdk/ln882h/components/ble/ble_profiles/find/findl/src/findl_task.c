/**
 ****************************************************************************************
 *
 * @file findl_task.c
 *
 * @brief Find Me Locator Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FINDLTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_FINDME_LOCATOR)
#include "co_utils.h"
#include "findl_task.h"
#include "findl.h"
#include "gattc_task.h"
#include "prf_utils.h"
#include "gap.h"
#include "gapc.h"
#include "gapc_task.h"
#include <stdint.h>

#include "ke_mem.h"

/// table used to retrieve immediate alert service characteristics information
const struct prf_char_def findl_ias_char[FINDL_IAS_CHAR_MAX] =
{
        [FINDL_ALERT_LVL_CHAR] = {ATT_CHAR_ALERT_LEVEL,
                                  ATT_MANDATORY,
                                  ATT_CHAR_PROP_WR_NO_RESP},
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref FINDL_ENABLE_REQ message.
 * The handler enables the Find Me profile - Locator Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int findl_enable_req_handler(ke_msg_id_t const msgid,
                                    struct findl_enable_req const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Find me Locator Role Task Environment
    struct findl_env_tag *p_findl_env = PRF_ENV_GET(FINDL, findl);

    ASSERT_INFO(p_findl_env != NULL, dest_id, src_id);

    if ((state == FINDL_IDLE) && (p_findl_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_findl_env->p_env[conidx] = (struct findl_cnx_env *) ke_malloc(sizeof(struct findl_cnx_env), KE_MEM_ATT_DB);
        memset(p_findl_env->p_env[conidx], 0, sizeof(struct findl_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // start discovering IAS on peer
            prf_disc_svc_send(&(p_findl_env->prf_env), conidx, ATT_SVC_IMMEDIATE_ALERT);

            // Go to DISCOVERING state
            ke_state_set(dest_id, FINDL_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_findl_env->p_env[conidx]->ias = p_param->ias;

            // send APP confirmation that can start normal connection to TH
            findl_enable_rsp_send(p_findl_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != FINDL_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        findl_enable_rsp_send(p_findl_env, conidx, status);
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
    if (ke_state_get(dest_id) == FINDL_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct findl_env_tag *p_findl_env = PRF_ENV_GET(FINDL, findl);

        ASSERT_INFO(p_findl_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_findl_env->p_env[conidx] != NULL, dest_id, src_id);

        if (p_findl_env->p_env[conidx]->nb_svc == 0)
        {
            // Retrieve IAS characteristic
            prf_extract_svc_info(p_ind, FINDL_IAS_CHAR_MAX, &findl_ias_char[0],
                    &(p_findl_env->p_env[conidx]->ias.alert_lvl_char), 0, NULL, NULL);

            //Even if we get multiple responses we only store 1 range
            p_findl_env->p_env[conidx]->ias.svc.shdl = p_ind->start_hdl;
            p_findl_env->p_env[conidx]->ias.svc.ehdl = p_ind->end_hdl;
        }

        p_findl_env->p_env[conidx]->nb_svc++;
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref FINDL_SET_ALERT_CMD message.
 * The handler disables the Find Me profile - Target Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int findl_set_alert_cmd_handler(ke_msg_id_t const msgid,
                                       struct findl_set_alert_cmd const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    int msg_status = KE_MSG_CONSUMED;

    if (state == FINDL_IDLE)
    {
        // Status
        uint8_t status = GAP_ERR_NO_ERROR;
        // Get the address of the environment
        struct findl_env_tag *p_findl_env = PRF_ENV_GET(FINDL, findl);
        uint8_t conidx = KE_IDX_GET(dest_id);
        struct findl_cmp_evt *p_evt;

        // check if discovery performed
        if (p_findl_env->p_env[conidx] == NULL)
        {
            status = PRF_ERR_REQ_DISALLOWED;
        }
        // Check we're sending a correct value
        else if ((p_param->alert_lvl == FINDL_ALERT_NONE)  ||
                (p_param->alert_lvl == FINDL_ALERT_MILD)   ||
                (p_param->alert_lvl == FINDL_ALERT_HIGH))
        {
            if (p_findl_env->p_env[conidx]->ias.alert_lvl_char.char_hdl != 0x0000)
            {
                // Send GATT Write Request
                prf_gatt_write(&p_findl_env->prf_env, conidx,
                        p_findl_env->p_env[conidx]->ias.alert_lvl_char.val_hdl,
                        (uint8_t *)&p_param->alert_lvl, sizeof(uint8_t), GATTC_WRITE_NO_RESPONSE);

                // wait for end of write request
                ke_state_set(dest_id, FINDL_BUSY);
            }
            else
            {
                // cannot set alert level because have no char handle for it
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
        else
        {
            // wrong level - not one of the possible 3
            status = PRF_ERR_INVALID_PARAM;
        }

        p_evt = KE_MSG_ALLOC(FINDL_CMP_EVT, src_id, dest_id, findl_cmp_evt);
        // set status
        p_evt->status = status;
        p_evt->operation = FINDL_SET_ALERT_CMD_OP_CODE;

        ke_msg_send(p_evt);
    }
    else if (state != FINDL_FREE)
    {
        msg_status = KE_MSG_SAVED;;
    }
    // else nothing to do - ignore message

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
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gattc_cmp_evt const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Get the address of the environment
    struct findl_env_tag *p_findl_env = PRF_ENV_GET(FINDL, findl);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == FINDL_DISCOVERING)
    {
        uint8_t status = p_param->status;

        if (p_param->status == ATT_ERR_NO_ERROR)
        {
            if (p_findl_env->p_env[conidx]->nb_svc ==  1)
            {
                status = prf_check_svc_char_validity(FINDL_IAS_CHAR_MAX,
                        &(p_findl_env->p_env[conidx]->ias.alert_lvl_char), findl_ias_char);
            }
            // too much services
            else if (p_findl_env->p_env[conidx]->nb_svc > 1)
            {
                status = PRF_ERR_MULTIPLE_SVC;
            }
            // no services found
            else
            {
                status = PRF_ERR_INVALID_PARAM;
            }
        }

        findl_enable_rsp_send(p_findl_env, conidx, status);
    }
    else if (state == FINDL_BUSY)
    {
        ke_state_set(dest_id, FINDL_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/// Default State handlers definition
KE_MSG_HANDLER_TAB(findl)
{
    {FINDL_ENABLE_REQ,             (ke_msg_func_t)findl_enable_req_handler},
    {GATTC_CMP_EVT,                (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_SDP_SVC_IND,            (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {FINDL_SET_ALERT_CMD,          (ke_msg_func_t)findl_set_alert_cmd_handler},
};

void findl_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct findl_env_tag *p_findl_env = PRF_ENV_GET(FINDL, findl);

    p_task_desc->msg_handler_tab = findl_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(findl_msg_handler_tab);
    p_task_desc->state           = p_findl_env->state;
    p_task_desc->idx_max         = FINDL_IDX_MAX;
}

#endif //BLE_FINDME_LOCATOR

/// @} FINDLTASK
