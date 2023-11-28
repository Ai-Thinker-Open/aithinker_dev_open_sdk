/**
 ****************************************************************************************
 *
 * @file lans_task.c
 *
 * @brief Location and Navigation Profile Sensor Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup LANSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_LN_SENSOR)
#include "lan_common.h"

#include "gapc.h"
#include "gattc.h"
#include "gattc_task.h"
#include "attm.h"
#include "lans.h"
#include "lans_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
#include "co_math.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref LANS_ENABLE_REQ message.
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int lans_enable_req_handler(ke_msg_id_t const msgid,
                                    struct lans_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    struct lans_enable_rsp *p_cmp_evt;

    if (ke_state_get(dest_id) == LANS_IDLE)
    {
        // Bonded data was not used before
        if (!GETB(p_lans_env->env[p_param->conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_PERFORMED_OK))
        {
            status = GAP_ERR_NO_ERROR;
            p_lans_env->env[p_param->conidx].prfl_ntf_ind_cfg = p_param->prfl_ntf_ind_cfg;
        }
        // Enable Bonded Data
        SETB(p_param->conidx, LANS_PRF_CFG_PERFORMED_OK, 1);
    }

    // send completed information to APP task that contains error status
    p_cmp_evt = KE_MSG_ALLOC(LANS_ENABLE_RSP, src_id, dest_id, lans_enable_rsp);
    p_cmp_evt->status = status;
    p_cmp_evt->conidx = p_param->conidx;
    ke_msg_send(p_cmp_evt);

    return (KE_MSG_CONSUMED);
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
    if (ke_state_get(dest_id) == LANS_IDLE)
    {
        // Pointer to the response message
        struct gattc_read_cfm *p_cfm = NULL;

        // Get the address of the environment
        struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
        uint8_t conidx = KE_IDX_GET(src_id);
        uint8_t att_idx = LANS_IDX(p_param->handle);

        uint8_t status = ATT_ERR_NO_ERROR;

        switch (att_idx)
        {
            case LNS_IDX_LN_FEAT_VAL:
            {
                // Allocate structure
                p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint32_t));
                p_cfm->length = sizeof(uint32_t);
                // Fill data
                co_write32p(p_cfm->value, p_lans_env->features);
            } break;

            case LNS_IDX_POS_Q_VAL:
            {
                // Check if Position Quality Char. has been added in the database
                if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_POS_Q_MASK))
                {
                    // Allocate structure
                    p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(struct lanp_posq));
                    // Fill data
                    p_cfm->length = lans_pack_posq(p_cfm->value);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }

            } break;

            case LNS_IDX_LOC_SPEED_NTF_CFG:
            {
                // allocate structure
                p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint16_t));
                p_cfm->length = sizeof(uint16_t);
                // Fill data
                co_write16p(p_cfm->value,
                        GETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF)
                        ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND);
            } break;

            case LNS_IDX_NAVIGATION_NTF_CFG:
            {
                // Check if Navigation Char. has been added in the database
                if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK))
                {
                    // allocate structure
                    p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint16_t));
                    p_cfm->length = sizeof(uint16_t);
                    co_write16p(p_cfm->value,
                            GETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_NAVIGATION_NTF)
                            ? PRF_CLI_START_NTF : PRF_CLI_STOP_NTFIND);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            case LNS_IDX_LN_CTNL_PT_IND_CFG:
            {
                // Check if LN control point has been added in the database
                if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_LN_CTNL_PT_MASK))
                {
                    // allocate structure
                    p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint16_t));
                    p_cfm->length = sizeof(uint16_t);
                    // Fill data
                    co_write16p(p_cfm->value,
                            GETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND)
                            ? PRF_CLI_START_IND : PRF_CLI_STOP_NTFIND);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            default:
            {
                status = ATT_ERR_REQUEST_NOT_SUPPORTED;
            } break;
        }

        if (status != ATT_ERR_NO_ERROR)
        {
            p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);
            p_cfm->length = 0;
        }

        p_cfm->handle = p_param->handle;
        p_cfm->status = status;

        // Send value to peer device.
        ke_msg_send(p_cfm);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the attribute info request message.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == LANS_IDLE)
    {
        // Get the address of the environment
        struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
        uint8_t att_idx = LANS_IDX(p_param->handle);
        struct gattc_att_info_cfm *p_cfm;

        //Send write response
        p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
        p_cfm->handle = p_param->handle;

        // check if it's a client configuration char
        if ((att_idx == LNS_IDX_LOC_SPEED_NTF_CFG)
                || (att_idx == LNS_IDX_NAVIGATION_NTF_CFG)
                || (att_idx == LNS_IDX_LN_CTNL_PT_IND_CFG))
        {
            // CCC attribute length = 2
            p_cfm->length = 2;
            p_cfm->status = GAP_ERR_NO_ERROR;
        }
        else if (att_idx == LNS_IDX_LN_CTNL_PT_VAL)
        {
            // force length to zero to reject any write starting from something != 0
            p_cfm->length = 0;
            p_cfm->status = GAP_ERR_NO_ERROR;
        }
        // not expected request
        else
        {
            p_cfm->length = 0;
            p_cfm->status = ATT_ERR_WRITE_NOT_PERMITTED;
        }

        ke_msg_send(p_cfm);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref LANS_NTF_LN_MEAS_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int lans_ntf_loc_speed_cmd_handler(ke_msg_id_t const msgid,
                                          struct lans_ntf_loc_speed_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_NO_FREE;

    // State shall be Connected or Busy
    if (ke_state_get(dest_id) == LANS_IDLE)
    {
        // Get the address of the environment
        struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

        // Configure the environment
        p_lans_env->operation = LANS_NTF_LOC_SPEED_OP_CODE;
        // allocate operation data
        p_lans_env->p_op_data = (struct lans_op *) ke_malloc(sizeof(struct lans_op), KE_MEM_KE_MSG);
        p_lans_env->p_op_data->p_cmd         = ke_param2msg(p_param);
        p_lans_env->p_op_data->cursor      = 0;
        p_lans_env->p_op_data->p_ntf_pending = NULL;

        // Go to busy state
        ke_state_set(dest_id, LANS_BUSY);

        // start operation execution
        lans_exe_operation(KE_IDX_GET(src_id));
    }
    else
    {
        // Save it for later
        msg_status = KE_MSG_SAVED;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref LANS_UPD_POS_Q_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int lans_upd_pos_q_req_handler(ke_msg_id_t const msgid,
                                          struct lans_upd_pos_q_req *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    // Allocate response message
    struct lans_upd_pos_q_rsp *p_rsp = KE_MSG_ALLOC(LANS_UPD_POS_Q_RSP,
            src_id, dest_id, lans_upd_pos_q_rsp);

    p_rsp->status = PRF_ERR_FEATURE_NOT_SUPPORTED;

    // Check if Position Quality Char. has been added in the database
    if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_POS_Q_MASK))
    {
        if (p_lans_env->p_posq != NULL)
        {
            // Update position quality
            lans_upd_posq(p_param->parameters);
            // Set status value
            p_rsp->status = GAP_ERR_NO_ERROR;
        }
        else
        {
            // Set status value
            p_rsp->status = PRF_APP_ERROR;
        }
    }

    // Send response to the application
    ke_msg_send(p_rsp);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref LANS_NTF_NAVIGATION_CMD message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int lans_ntf_navigation_cmd_handler(ke_msg_id_t const msgid,
                                          struct lans_ntf_navigation_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_NO_FREE;

    // State shall be Connected or Busy
    if (ke_state_get(dest_id) == LANS_IDLE)
    {
        // Get the address of the environment
        struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

        // Configure the environment
        p_lans_env->operation = LANS_NTF_NAVIGATION_OP_CODE;
        // allocate operation data
        p_lans_env->p_op_data = (struct lans_op*) ke_malloc(sizeof(struct lans_op), KE_MEM_KE_MSG);
        p_lans_env->p_op_data->p_cmd         = ke_param2msg(p_param);
        p_lans_env->p_op_data->cursor      = 0;
        p_lans_env->p_op_data->p_ntf_pending = NULL;

        // Go to busy state
        ke_state_set(dest_id, LANS_BUSY);

        // start operation execution
        lans_exe_operation(KE_IDX_GET(src_id));
    }
    else
    {
        // Save it for later
        msg_status = KE_MSG_SAVED;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref LANS_LN_CTNL_PT_CFM message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int lans_ln_ctnl_pt_cfm_handler(ke_msg_id_t const msgid,
                                          struct lans_ln_ctnl_pt_cfm *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    uint8_t status = GAP_ERR_NO_ERROR;

    if (ke_state_get(dest_id) == LANS_BUSY)
    {
        do
        {
            struct gattc_send_evt_cmd *p_ctl_pt_rsp;

            // Check the current operation
            if (p_lans_env->operation < LANS_SET_CUMUL_VALUE_OP_CODE)
            {
                // The confirmation has been sent without request indication, ignore
                break;
            }

            // The LN Control Point Characteristic must be supported if we are here
            if (!LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_LN_CTNL_PT_MASK))
            {
                status = PRF_ERR_REQ_DISALLOWED;
                break;
            }

            // Check if sending of indications has been enabled
            if (!GETB(p_lans_env->env[p_param->conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND))
            {
                // CCC improperly configured
                status = PRF_CCCD_IMPR_CONFIGURED;
                break;
            }

            // Allocate the GATT notification message
            p_ctl_pt_rsp = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, p_param->conidx), dest_id,
                    gattc_send_evt_cmd, LANP_LAN_LN_CNTL_PT_RSP_MAX_LEN);

            // Fill in the p_parameter structure
            p_ctl_pt_rsp->operation = GATTC_INDICATE;
            p_ctl_pt_rsp->handle = LANS_HANDLE(LNS_IDX_LN_CTNL_PT_VAL);
            // Pack Control Point confirmation
            p_ctl_pt_rsp->length = lans_pack_ln_ctnl_point_cfm (p_param->conidx, p_param, p_ctl_pt_rsp->value);

            // Send the event
            ke_msg_send(p_ctl_pt_rsp);

        } while (0);

        if (status != GAP_ERR_NO_ERROR)
        {
            // Inform the application that a procedure has been completed
            lans_send_cmp_evt(p_param->conidx,
                    prf_src_task_get(&p_lans_env->prf_env, p_param->conidx),
                    prf_dst_task_get(&p_lans_env->prf_env, p_param->conidx),
                    p_lans_env->operation, status);
        }
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_WRITE_REQ_IND message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                       struct gattc_write_req_ind const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);
    uint8_t conidx = KE_IDX_GET(src_id);
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Control Point Status
    uint8_t ctl_pt_status = LANP_LN_CTNL_PT_RESP_SUCCESS;
    struct gattc_write_cfm *p_cfm;

    if (p_lans_env != NULL)
    {
        // Location and speed Characteristic, Client Characteristic Configuration Descriptor
        if (p_param->handle == (LANS_HANDLE(LNS_IDX_LOC_SPEED_NTF_CFG)))
        {
            // Update value
            status = lans_update_characteristic_config(conidx, LANS_PRF_CFG_FLAG_LOC_SPEED_NTF_BIT, p_param);
        }
        // Navigation Characteristic, Client Characteristic Configuration Descriptor
        else if (p_param->handle == (LANS_HANDLE(LNS_IDX_NAVIGATION_NTF_CFG)))
        {
            if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_NAVI_MASK))
            {
                // Update value
                status = lans_update_characteristic_config(conidx, LANS_PRF_CFG_FLAG_NAVIGATION_NTF_BIT, p_param);
            }
            else
            {
                status = ATT_ERR_ATTRIBUTE_NOT_FOUND;
            }
        }
        // LN Control Point Characteristic
        else
        {
            if (LANS_IS_FEATURE_SUPPORTED(p_lans_env->prfl_cfg, LANS_LN_CTNL_PT_MASK))
            {
                // LN Control Point, Client Characteristic Configuration Descriptor
                if (p_param->handle == (LANS_HANDLE(LNS_IDX_LN_CTNL_PT_IND_CFG)))
                {
                    // Update value
                    status = lans_update_characteristic_config(conidx, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND_BIT, p_param);
                }
                // LN Control Point Characteristic
                else if (p_param->handle == (LANS_HANDLE(LNS_IDX_LN_CTNL_PT_VAL)))
                {
                    do
                    {
                        // Check if sending of indications has been enabled
                        if (!GETB(p_lans_env->env[conidx].prfl_ntf_ind_cfg, LANS_PRF_CFG_FLAG_LN_CTNL_PT_IND))
                        {
                            // CCC improperly configured
                            status = PRF_CCCD_IMPR_CONFIGURED;
                            break;
                        }

                        if (p_lans_env->operation >= LANS_SET_CUMUL_VALUE_OP_CODE)
                        {
                            // A procedure is already in progress
                            status = LAN_ERROR_PROC_IN_PROGRESS;
                            break;
                        }

                        if (p_lans_env->operation == LANS_NTF_LOC_SPEED_OP_CODE)
                        {
                            // Keep the message until the end of the current procedure
                            msg_status = KE_MSG_SAVED;
                            break;
                        }

                        // Unpack Control Point parameters
                        ctl_pt_status = lans_unpack_ln_ctnl_point_ind (conidx, p_param);

                    } while (0);
                }
                else
                {
                    ASSERT_ERR(0);
                }
            }
            else
            {
                status = ATT_ERR_ATTRIBUTE_NOT_FOUND;
            }
        }
    }

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = p_param->handle;
    p_cfm->status = status;
    ke_msg_send(p_cfm);

    // Check if control point failed
    if ((p_param->handle == (LANS_HANDLE(LNS_IDX_LN_CTNL_PT_VAL))) &&
            (ctl_pt_status != LANP_LN_CTNL_PT_RESP_SUCCESS))
    {
        // Send response to peer
        lans_send_rsp_ind(conidx, p_param->value[0], ctl_pt_status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATT_NOTIFY_CMP_EVT message meaning that a notification or an indication
 * has been correctly sent to peer device (but not confirmed by peer device).
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    uint8_t conidx = KE_IDX_GET(src_id);

    // Check if a connection exists
    if (ke_state_get(dest_id) == LANS_BUSY)
    {
        switch (p_param->operation)
        {
            case (GATTC_NOTIFY):
            {
                lans_exe_operation(conidx);
                // else ignore the message
            } break;

            case (GATTC_INDICATE):
            {

                ASSERT_ERR(p_lans_env->operation >= LANS_SET_CUMUL_VALUE_OP_CODE);

                // Inform the application that a procedure has been completed
                lans_send_cmp_evt(conidx, prf_src_task_get(&(p_lans_env->prf_env), conidx),
                                   prf_dst_task_get(&(p_lans_env->prf_env), conidx),
                                   p_lans_env->operation, p_param->status);
            } break;
            // else ignore the message

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
KE_MSG_HANDLER_TAB(lans)
{
    {LANS_ENABLE_REQ,               (ke_msg_func_t)lans_enable_req_handler},
    {GATTC_READ_REQ_IND,            (ke_msg_func_t)gattc_read_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t)gattc_att_info_req_ind_handler},
    {LANS_NTF_LOC_SPEED_CMD,        (ke_msg_func_t)lans_ntf_loc_speed_cmd_handler},
    {LANS_UPD_POS_Q_REQ,            (ke_msg_func_t)lans_upd_pos_q_req_handler},
    {LANS_NTF_NAVIGATION_CMD,       (ke_msg_func_t)lans_ntf_navigation_cmd_handler},
    {LANS_LN_CTNL_PT_CFM,           (ke_msg_func_t)lans_ln_ctnl_pt_cfm_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t)gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t)gattc_cmp_evt_handler},
};

void lans_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct lans_env_tag *p_lans_env = PRF_ENV_GET(LANS, lans);

    p_task_desc->msg_handler_tab = lans_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(lans_msg_handler_tab);
    p_task_desc->state           = p_lans_env->state;
    p_task_desc->idx_max         = LANS_IDX_MAX;
}

#endif //(BLE_LN_SENSOR)

/// @} LANSTASK
