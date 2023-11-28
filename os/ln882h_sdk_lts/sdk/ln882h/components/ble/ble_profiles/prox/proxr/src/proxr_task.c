/**
 ****************************************************************************************
 *
 * @file proxr_task.c
 *
 * @brief Proximity Reporter Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup PROXRTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_PROX_REPORTER)
#include "co_utils.h"

#include "gap.h"
#include "gattc_task.h"
#include "attm.h"
#include "proxr_task.h"
#include "proxr.h"
#include "prf_utils.h"

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of write request message.
 * The handler will analyze what has been set and decide alert level
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_write_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    if (state == PROXR_IDLE)
    {
        // Allocate write confirmation message.
        struct gattc_write_cfm *p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,
                src_id, dest_id, gattc_write_cfm);

        // Fill in the p_parameter structure
        p_cfm->handle = p_param->handle;
        p_cfm->status = PRF_APP_ERROR;

        if (p_param->value[0] <= PROXR_ALERT_HIGH)
        {
            // Get the address of the environment
            struct proxr_env_tag *p_proxr_env = PRF_ENV_GET(PROXR, proxr);
            uint8_t conidx = KE_IDX_GET(src_id);
            uint8_t alert_lvl = p_param->value[0];

            // Store alert level on the environment
            if (p_param->handle == (p_proxr_env->lls_start_hdl + LLS_IDX_ALERT_LVL_VAL))
            {
                p_proxr_env->lls_alert_lvl[conidx] = alert_lvl;
                p_cfm->status = GAP_ERR_NO_ERROR;
            }
            else if (p_param->handle == (p_proxr_env->ias_start_hdl + IAS_IDX_ALERT_LVL_VAL))
            {
                // Allocate the alert value change indication
                struct proxr_alert_ind *p_ind = KE_MSG_ALLOC(PROXR_ALERT_IND,
                        prf_dst_task_get(&(p_proxr_env->prf_env), conidx),
                        dest_id, proxr_alert_ind);

                // Fill in the p_parameter structure
                p_ind->alert_lvl = alert_lvl;
                p_ind->char_code = PROXR_IAS_CHAR;
                p_ind->conidx = conidx;

                // Send the message
                ke_msg_send(p_ind);

                p_cfm->status = GAP_ERR_NO_ERROR;
            }
            else
            {
                p_cfm->status = ATT_ERR_REQUEST_NOT_SUPPORTED;
            }
        }

        // Send the message
        ke_msg_send(p_cfm);
    }
    else if (state == PROXR_BUSY)
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the read request from peer device
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_read_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;
    uint8_t state = ke_state_get(dest_id);

    if (state == PROXR_IDLE)
    {
        // Get the address of the environment
        struct proxr_env_tag *p_proxr_env = PRF_ENV_GET(PROXR, proxr);
        uint8_t conidx = KE_IDX_GET(src_id);

        uint8_t value = PROXR_ALERT_NONE;
        uint8_t status = ATT_ERR_NO_ERROR;

        if (p_param->handle == (p_proxr_env->lls_start_hdl + LLS_IDX_ALERT_LVL_VAL))
        {
            value = p_proxr_env->lls_alert_lvl[conidx];
        }
        else if (p_param->handle == (p_proxr_env->txp_start_hdl + TXPS_IDX_TX_POWER_LVL_VAL))
        {
            // send a request to read TX power level on GAPM
            struct gapm_get_dev_info_cmd *p_cmd = KE_MSG_ALLOC(GAPM_GET_DEV_INFO_CMD,
                                                                TASK_GAPM,
                                                                dest_id,
                                                                gapm_get_dev_info_cmd);

            p_cmd->operation = GAPM_GET_DEV_ADV_TX_POWER;
            ke_msg_send(p_cmd);

            // save the connection index
            p_proxr_env->conidx_saved = conidx;

            // need to wait for GAPM response, go into busy state
            ke_state_set(dest_id, PROXR_BUSY);

            status = PRF_APP_ERROR;
        }
        else
        {
            status = ATT_ERR_REQUEST_NOT_SUPPORTED;
        }

        // Don't send the confirmation if gap read request has been initiated.
        if (status != PRF_APP_ERROR)
        {
            // Send data to peer device
            struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id,
                    gattc_read_cfm, sizeof(uint8_t));

            p_cfm->length = sizeof(uint8_t);
            p_cfm->value[0] = value;
            p_cfm->handle = p_param->handle;
            p_cfm->status = status;

            // Send value to peer device.
            ke_msg_send(p_cfm);
        }
    }
    else if (state == PROXR_BUSY)
    {
        msg_status = KE_MSG_SAVED;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GAPM_DEV_ADV_TX_POWER_IND message.
 * This generic event is received for different requests, so need to keep track.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gapm_dev_adv_tx_power_ind_handler(ke_msg_id_t const msgid,
                                            struct gapm_dev_adv_tx_power_ind const *p_param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct proxr_env_tag *p_proxr_env = PRF_ENV_GET(PROXR, proxr);

    if (ke_state_get(dest_id) == PROXR_BUSY)
    {
        // Send data to peer device
        struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                                                        KE_BUILD_ID(TASK_GATTC, p_proxr_env->conidx_saved),
                                                        dest_id,
                                                        gattc_read_cfm,
                                                        sizeof(uint8_t));

        p_cfm->length = sizeof(uint8_t);
        p_cfm->value[0] = p_param->power_lvl;
        p_cfm->handle = p_proxr_env->txp_start_hdl + TXPS_IDX_TX_POWER_LVL_VAL;
        p_cfm->status = GAP_ERR_NO_ERROR;

        // Send value to peer device.
        ke_msg_send(p_cfm);

        ke_state_set(dest_id, PROXR_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GAPM_CMP_EVT message.
 * This generic event is received for different requests, so need to keep track.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gapm_cmp_evt_handler(ke_msg_id_t const msgid,
                                struct gapm_cmp_evt const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct proxr_env_tag *p_proxr_env = PRF_ENV_GET(PROXR, proxr);

    if ((ke_state_get(dest_id) == PROXR_BUSY) && (p_param->status != GAP_ERR_NO_ERROR))
    {
        // Send data to peer device
        struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM,
                                                    KE_BUILD_ID(TASK_GATTC,
                                                    p_proxr_env->conidx_saved),
                                                    dest_id,
                                                    gattc_read_cfm);

        p_cfm->length = sizeof(uint8_t);
        p_cfm->handle = p_proxr_env->txp_start_hdl + TXPS_IDX_TX_POWER_LVL_VAL;
        p_cfm->status = PRF_APP_ERROR;

        // Send value to peer device.
        ke_msg_send(p_cfm);

        ke_state_set(dest_id, PROXR_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(proxr)
{
    {GATTC_WRITE_REQ_IND,                 (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_READ_REQ_IND,                  (ke_msg_func_t) gattc_read_req_ind_handler},
    {GAPM_DEV_ADV_TX_POWER_IND,           (ke_msg_func_t) gapm_dev_adv_tx_power_ind_handler},
    {GAPM_CMP_EVT,                        (ke_msg_func_t) gapm_cmp_evt_handler},
};

void proxr_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct proxr_env_tag *p_proxr_env = PRF_ENV_GET(PROXR, proxr);

    p_task_desc->msg_handler_tab = proxr_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(proxr_msg_handler_tab);
    p_task_desc->state           = p_proxr_env->state;
    p_task_desc->idx_max         = PROXR_IDX_MAX;
}

#endif //BLE_PROX_REPORTER

/// @} PROXRTASK
