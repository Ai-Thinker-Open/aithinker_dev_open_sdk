/**
 ****************************************************************************************
 *
 * @file findt_task.c
 *
 * @brief Find Me Target Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FINDTTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"

#if (BLE_FINDME_TARGET)
#include "co_utils.h"
#include "gattc_task.h"
#include "findt_task.h"
#include "findt.h"
#include "attm.h"
#include "prf_utils.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref  GATTC_WRITE_REQ_IND message.
 * The message is redirected from TASK_SVC because at profile enable, the ATT handle is
 * register for TASK_FINDT. In the handler, an ATT Write Response/Error Response should
 * be sent for ATT protocol, but Alert Level Characteristic only supports WNR so no
 * response PDU is needed.
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
    // Allocate write confirmation message.
    struct gattc_write_cfm *cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,
            src_id, dest_id, gattc_write_cfm);

    // Get the address of the environment
    struct findt_env_tag *p_findt_env = PRF_ENV_GET(FINDT, findt);
    uint8_t att_idx = FINDT_IDX(p_param->handle);

    // Fill in the parameter structure
    cfm->handle = p_param->handle;
    cfm->status = PRF_APP_ERROR;

    // Check if Alert Level is valid
    if ((att_idx == FINDT_IAS_IDX_ALERT_LVL_VAL) && (p_param->value[0] <= FINDT_ALERT_HIGH))
    {
        struct findt_alert_ind *p_ind;
        uint8_t alert_lvl = 0x0000;
        uint8_t conidx = KE_IDX_GET(src_id);

        alert_lvl = p_param->value[0];
        cfm->status = GAP_ERR_NO_ERROR;

        // Allocate the alert value change indication
        p_ind = KE_MSG_ALLOC(FINDT_ALERT_IND,
                prf_dst_task_get(&(p_findt_env->prf_env), conidx),
                dest_id, findt_alert_ind);
        // Fill in the parameter structure
        p_ind->alert_lvl = alert_lvl;
        p_ind->conidx = conidx;

        // Send the message
        ke_msg_send(p_ind);
    }

    // Send the message
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(findt)
{
    {GATTC_WRITE_REQ_IND,                 (ke_msg_func_t) gattc_write_req_ind_handler},
};

void findt_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct findt_env_tag *p_findt_env = PRF_ENV_GET(FINDT, findt);

    p_task_desc->msg_handler_tab = findt_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(findt_msg_handler_tab);
    p_task_desc->state           = p_findt_env->state;
    p_task_desc->idx_max         = FINDT_IDX_MAX;
}

#endif //BLE_FINDME_TARGET

/// @} FINDTTASK
