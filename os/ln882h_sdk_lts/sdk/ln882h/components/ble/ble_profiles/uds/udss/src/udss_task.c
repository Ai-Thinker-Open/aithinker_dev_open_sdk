/**
 ****************************************************************************************
 *
 * @file udss_task.c
 *
 * @brief User Data Service Profile Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup UDSSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_UDS_SERVER)
#include "uds_common.h"

#include "gapc.h"
#include "gattc_task.h"
#include "attm.h"

#include "udss.h"
#include "udss_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

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

    // check that task is in idle state
    if (ke_state_get(dest_id) == UDSS_STATE_IDLE)
    {
        // Get the address of the environment
        struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
        uint8_t conidx = KE_IDX_GET(src_id);
        uint8_t att_idx = p_param->handle - p_udss_env->shdl;
        // get the char_idx and dec
        uint8_t uds_char_id;

        switch(att_idx)
        {
            case UDSS_IDX_DB_CHG_INC_DESC_CCC:
            case UDSS_IDX_USER_CTRL_PT_DESC_CCC:
            {
                // Send data to peer device
                struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                        src_id, dest_id, gattc_read_cfm, UDSS_CCC_SIZE);

                p_cfm->handle = p_param->handle;
                p_cfm->length = UDSS_CCC_SIZE;
                p_cfm->status = GAP_ERR_NO_ERROR;

                co_write16p(p_cfm->value, att_idx == UDSS_IDX_DB_CHG_INC_DESC_CCC ?
                        p_udss_env->db_chg_inc_ccc[conidx] : p_udss_env->user_ctrl_pt_ccc[conidx]);

                ke_msg_send(p_cfm);
            } break;

            default:
            {
                // retrieve handle information
                uds_char_id = udss_handle_2_char(p_param->handle);

                if (uds_char_id != UDS_CHAR_ID_UNKNOWN)
                {
                    struct udss_rd_char_req_ind *p_ind = KE_MSG_ALLOC(UDSS_RD_CHAR_REQ_IND,
                            prf_dst_task_get(&(p_udss_env->prf_env), conidx),
                            dest_id,
                            udss_rd_char_req_ind);

                    // Characteristic idx @ref uds_char_id
                    p_ind->char_idx = uds_char_id;

                    // Send the request indication to the application
                    ke_msg_send(p_ind);

                    // go to busy state
                    ke_state_set(dest_id, UDSS_STATE_OP_READ);
                }
                else
                {
                    // Fill data
                    struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);

                    p_cfm->handle = p_param->handle;
                    p_cfm->length = 0;
                    p_cfm->status = ATT_ERR_REQUEST_NOT_SUPPORTED;

                    ke_msg_send(p_cfm);
                }
            } break;
        }
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
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
    // Get the address of the environment
    struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
    uint8_t att_idx = p_param->handle - p_udss_env->shdl;
    struct gattc_att_info_cfm *p_cfm;
    // get the char_idx and dec
    uint8_t uds_char_id = udss_handle_2_char(p_param->handle);

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = p_param->handle;

    p_cfm->status = GAP_ERR_NO_ERROR;

    switch (att_idx)
    {
        case UDSS_IDX_DB_CHG_INC_DESC_CCC:
        case UDSS_IDX_USER_CTRL_PT_DESC_CCC:
        {
            p_cfm->length = UDSS_CCC_SIZE;
        } break;

        default:
        {
            // retrieve handle information
            uds_char_id = udss_handle_2_char(p_param->handle);

            switch(uds_char_id)
            {
                // Strings: utf8s
                case UDS_CHAR_ID_FIRST_NAME   :
                case UDS_CHAR_ID_LAST_NAME    :
                case UDS_CHAR_ID_EMAIL_ADDRESS:
                case UDS_CHAR_ID_LANGUAGE     :
                {
                    p_cfm->length = UDSS_STRING_MAX_SIZE;
                } break;

                // Date
                case UDS_CHAR_ID_DATE_OF_BIRTH               :
                case UDS_CHAR_ID_DATE_OF_THRESHOLD_ASSESSMENT:
                {
                    p_cfm->length = UDSS_DATE_MAX_SIZE;
                } break;

                // uint16
                case UDS_CHAR_ID_WEIGHT             :
                case UDS_CHAR_ID_HEIGHT             :
                case UDS_CHAR_ID_WAIST_CIRCUMFERENCE:
                case UDS_CHAR_ID_HIP_CIRCUMFERENCE  :
                {
                    p_cfm->length = sizeof(uint16_t);
                } break;

                // uint8
                case UDS_CHAR_ID_AGE                                            :
                case UDS_CHAR_ID_GENDER                                         :
                case UDS_CHAR_ID_VO2_MAX                                        :
                case UDS_CHAR_ID_MAX_HEART_RATE                                 :
                case UDS_CHAR_ID_RESTING_HEART_RATE                             :
                case UDS_CHAR_ID_MAXIMUM_RECOMMENDED_HEART_RATE                 :
                case UDS_CHAR_ID_AEROBIC_THRESHOLD                             :
                case UDS_CHAR_ID_ANAEROBIC_THRESHOLD                           :
                case UDS_CHAR_ID_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS:
                case UDS_CHAR_ID_FAT_BURN_HEART_RATE_LOWER_LIMIT                :
                case UDS_CHAR_ID_FAT_BURN_HEART_RATE_UPPER_LIMIT                :
                case UDS_CHAR_ID_AEROBIC_HEART_RATE_LOWER_LIMIT                 :
                case UDS_CHAR_ID_AEROBIC_HEART_RATE_UPPER_LIMIT                 :
                case UDS_CHAR_ID_ANAEROBIC_HEART_RATE_LOWER_LIMIT               :
                case UDS_CHAR_ID_ANAEROBIC_HEART_RATE_UPPER_LIMIT               :
                case UDS_CHAR_ID_TWO_ZONE_HEART_RATE_LIMITS                     :
                {
                    p_cfm->length = sizeof(uint8_t);
                } break;

                // set
                case UDS_CHAR_ID_FIVE_ZONE_HEART_RATE_LIMITS :
                {
                    p_cfm->length = 4;
                } break;

                // set
                case UDS_CHAR_ID_THREE_ZONE_HEART_RATE_LIMITS:
                {
                    p_cfm->length = 2;
                } break;

                // Uint32
                case UDS_CHAR_ID_DB_CHG_INC:
                {
                    p_cfm->length = sizeof(uint32_t);
                } break;

                // Control Point
                case UDS_CHAR_ID_USER_CTRL_PT:
                {
                    p_cfm->length = UDS_USER_CTRL_PT_MAX_LEN;
                } break;

                default:
                {
                    // not expected request
                    p_cfm->length = 0;
                    p_cfm->status = ATT_ERR_WRITE_NOT_PERMITTED;
                } break;
            }
        } break;
    }

    ke_msg_send(p_cfm);

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
    struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
    uint8_t conidx = KE_IDX_GET(src_id);
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t att_idx = p_param->handle - p_udss_env->shdl;

    do
    {
        // Check the connection handle
        if (p_udss_env == NULL)
        {
            break;
        }

        // check that task is in idle state
        if (ke_state_get(dest_id) == UDSS_STATE_IDLE)
        {
            // get the char_idx and dec
            uint8_t uds_char_id;

            switch(att_idx)
            {
                case UDSS_IDX_DB_CHG_INC_DESC_CCC:
                case UDSS_IDX_USER_CTRL_PT_DESC_CCC:
                {
                    // Get the value
                    uint16_t ntf_cfg = co_read16p(&p_param->value[0]);
                    struct udss_wr_char_ccc_ind *p_ind;
                    struct gattc_write_cfm *p_cfm;

                    if (att_idx == UDSS_IDX_DB_CHG_INC_DESC_CCC)
                    {
                        p_udss_env->db_chg_inc_ccc[conidx] = ntf_cfg;
                    }
                    else
                    {
                        p_udss_env->user_ctrl_pt_ccc[conidx] = ntf_cfg;
                    }

                    // Inform the HL about the new configuration
                    p_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_CCC_IND,
                            prf_dst_task_get(&p_udss_env->prf_env, conidx),
                            prf_src_task_get(&p_udss_env->prf_env, conidx),
                            udss_wr_char_ccc_ind);

                    p_ind->char_idx = (att_idx == UDSS_IDX_DB_CHG_INC_DESC_CCC ?
                            UDS_CHAR_ID_DB_CHG_INC : UDS_CHAR_ID_USER_CTRL_PT);
                    p_ind->ind_cfg = ntf_cfg;

                    ke_msg_send(p_ind);

                    // Send the write response to the peer device
                    p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
                    p_cfm->handle = p_param->handle;
                    p_cfm->status = status;
                    ke_msg_send(p_cfm);
                } break;

                default:
                {
                    // retrieve handle information
                    uds_char_id = udss_handle_2_char(p_param->handle);

                    switch (uds_char_id)
                    {
                        // Strings: utf8s
                        case UDS_CHAR_ID_FIRST_NAME:
                        case UDS_CHAR_ID_LAST_NAME:
                        case UDS_CHAR_ID_EMAIL_ADDRESS:
                        case UDS_CHAR_ID_LANGUAGE:
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_utf8_req_ind *p_req_ind = KE_MSG_ALLOC_DYN(UDSS_WR_CHAR_UTF8_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_utf8_req_ind, p_param->length);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic String
                            p_req_ind->offset = p_param->offset;
                            p_req_ind->utf_name.length = p_param->length;
                            // actual data for the attribute
                            memcpy(&p_req_ind->utf_name.name[0], p_param->value, p_param->length);

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // Date
                        case UDS_CHAR_ID_DATE_OF_BIRTH:
                        case UDS_CHAR_ID_DATE_OF_THRESHOLD_ASSESSMENT:
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Date
                            p_req_ind->value.date.year  = co_read16p(&p_param->value[0]);
                            p_req_ind->value.date.month = p_param->value[2];
                            p_req_ind->value.date.day   = p_param->value[3];

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // uint16
                        case UDS_CHAR_ID_WEIGHT:
                        case UDS_CHAR_ID_HEIGHT:
                        case UDS_CHAR_ID_WAIST_CIRCUMFERENCE:
                        case UDS_CHAR_ID_HIP_CIRCUMFERENCE:
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Uint16
                            p_req_ind->value.uint16 = co_read16p(&p_param->value[0]);

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // uint8
                        case UDS_CHAR_ID_AGE:
                        case UDS_CHAR_ID_GENDER:
                        case UDS_CHAR_ID_VO2_MAX:
                        case UDS_CHAR_ID_MAX_HEART_RATE:
                        case UDS_CHAR_ID_RESTING_HEART_RATE:
                        case UDS_CHAR_ID_MAXIMUM_RECOMMENDED_HEART_RATE:
                        case UDS_CHAR_ID_AEROBIC_THRESHOLD:
                        case UDS_CHAR_ID_ANAEROBIC_THRESHOLD:
                        case UDS_CHAR_ID_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS:
                        case UDS_CHAR_ID_FAT_BURN_HEART_RATE_LOWER_LIMIT:
                        case UDS_CHAR_ID_FAT_BURN_HEART_RATE_UPPER_LIMIT:
                        case UDS_CHAR_ID_AEROBIC_HEART_RATE_LOWER_LIMIT:
                        case UDS_CHAR_ID_AEROBIC_HEART_RATE_UPPER_LIMIT:
                        case UDS_CHAR_ID_ANAEROBIC_HEART_RATE_LOWER_LIMIT:
                        case UDS_CHAR_ID_ANAEROBIC_HEART_RATE_UPPER_LIMIT:
                        case UDS_CHAR_ID_TWO_ZONE_HEART_RATE_LIMITS:
                        {
                            // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Uint8
                            p_req_ind->value.uint8 = p_param->value[0];

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // set
                        case UDS_CHAR_ID_FIVE_ZONE_HEART_RATE_LIMITS :
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Uint8
                            p_req_ind->value.set[0] =  p_param->value[0];
                            p_req_ind->value.set[1] =  p_param->value[1];
                            p_req_ind->value.set[2] =  p_param->value[2];
                            p_req_ind->value.set[3] =  p_param->value[3];

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // set
                        case UDS_CHAR_ID_THREE_ZONE_HEART_RATE_LIMITS:
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Uint8
                            p_req_ind->value.set[0] =  p_param->value[0];
                            p_req_ind->value.set[1] =  p_param->value[1];

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // Uint32
                        case UDS_CHAR_ID_DB_CHG_INC:
                        {
                             // Allocate a request indication message for the application
                            struct udss_wr_char_req_ind *p_req_ind = KE_MSG_ALLOC(UDSS_WR_CHAR_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_char_req_ind);

                            // Characteristic idx @ref uds_char_idx
                            p_req_ind->char_idx = uds_char_id;
                            // Characteristic Uint32
                            p_req_ind->value.db_chg_inc =  co_read32p(&p_param->value[0]);

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        // User Control Point
                        case UDS_CHAR_ID_USER_CTRL_PT:
                        {
                            uint8_t op_code = p_param->value[0];
                            uint16_t length = 0;
                            struct udss_wr_user_ctrl_pt_req_ind *p_req_ind;

                            if ((op_code == UDS_OP_CODE_REGISTER_NEW_USER)
                                || (op_code == UDS_OP_CODE_CONSENT)
                                || (op_code == UDS_OP_CODE_DELETE_USER_DATA))
                            {
                                length = 0;
                            }
                            else
                            {
                                // unknown op-code - let app decide
                                // reserved commands - for the future extension
                                if (p_param->length >= UDS_USER_CTRL_PT_MAX_LEN - 1) //sizeof(op-code)
                                {
                                    length = 0;
                                }
                                else if (p_param->length > 1) //op-code
                                {
                                    length = p_param->length - 1;
                                }
                            }

                            // check op-code and immediate response
                            // this would be an APP responsibility as the Indication of error condition after confirmatin message
                            // Allocate a request indication message for the application
                            p_req_ind = KE_MSG_ALLOC_DYN(UDSS_WR_USER_CTRL_PT_REQ_IND,
                                    prf_dst_task_get(&p_udss_env->prf_env, conidx),
                                    dest_id,
                                    udss_wr_user_ctrl_pt_req_ind, length);

                            // Request op-code
                            p_req_ind->op_code = p_param->value[0];
                            p_req_ind->length = length;

                            if (op_code == UDS_OP_CODE_REGISTER_NEW_USER)
                            {
                                // op-code and consent code
                                p_req_ind->consent = co_read16p(&p_param->value[1]);
                            }
                            else if (op_code == UDS_OP_CODE_CONSENT)
                            {
                                // op-code, user_id and consent code
                                p_req_ind->user_id = p_param->value[1];
                                p_req_ind->consent = co_read16p(&p_param->value[2]);
                            }
                            else if (op_code == UDS_OP_CODE_DELETE_USER_DATA)
                            {
                                // op-code
                            }
                            else
                            {
                                if (length) // copy all parameters
                                {
                                    // Characteristic Parameter
                                    memcpy(&p_req_ind->parameter[0], &p_param->value[1], length);
                                }
                            }

                            // Send the request indication to the application
                            ke_msg_send(p_req_ind);

                            // go to busy state
                            ke_state_set(dest_id, UDSS_STATE_OP_WRITE);
                        } break;

                        default: //UDS_CHAR_ID_UNKNOWN
                        {
                            // Send the write response to the peer device
                            struct gattc_write_cfm *p_cfm = KE_MSG_ALLOC(
                                    GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);

                            p_cfm->handle = p_param->handle;
                            p_cfm->status = ATT_ERR_REQUEST_NOT_SUPPORTED;

                            ke_msg_send(p_cfm);
                        } break;
                    }
                } break;
            }
        }
        else
        {
            msg_status = KE_MSG_SAVED;
        }
    } while (0);

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles @ref GATT_NOTIFY_CMP_EVT message meaning that a notification or an indication
 * has been correctly sent to peer device (but not confirmed by peer device).
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    do
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        // Get the address of the environment
        struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);

        if (p_udss_env == NULL)
        {
            break;
        }

        // Check if a connection exists
        switch (p_param->operation)
        {
            case (GATTC_NOTIFY):
            {
                struct udss_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(UDSS_CMP_EVT,
                        prf_dst_task_get(&(p_udss_env->prf_env), conidx),
                        dest_id, udss_cmp_evt);

                p_cmp_evt->operation = UDSS_NOTIFY_OP_CODE;
                p_cmp_evt->status = p_param->status;

                ke_msg_send(p_cmp_evt);

                // go to idle state
                ke_state_set(dest_id, UDSS_STATE_IDLE);
            } break;

            case (GATTC_INDICATE):
            {
                struct udss_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(UDSS_CMP_EVT,
                        prf_dst_task_get(&(p_udss_env->prf_env), conidx),
                        dest_id, udss_cmp_evt);

                p_cmp_evt->operation = UDSS_INDICATE_OP_CODE;
                p_cmp_evt->status = p_param->status;

                ke_msg_send(p_cmp_evt);

                // go to idle state
                ke_state_set(dest_id, UDSS_STATE_IDLE);
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }
    } while (0);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_ENABLE_REQ message.
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_enable_req_handler(ke_msg_id_t const msgid,
                                    struct udss_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
    struct udss_enable_rsp *p_cmp_evt;
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;

    if (ke_state_get(dest_id) == UDSS_STATE_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(src_id);

        p_udss_env->db_chg_inc_ccc[conidx] = p_param->db_chg_inc_ccc;
        p_udss_env->user_ctrl_pt_ccc[conidx] = p_param->user_ctrl_pt_ccc;
        status = GAP_ERR_NO_ERROR;
    }

    // send completed information to APP task that contains error status
    p_cmp_evt = KE_MSG_ALLOC(UDSS_ENABLE_RSP, src_id, dest_id, udss_enable_rsp);
    p_cmp_evt->status = status;

    ke_msg_send(p_cmp_evt);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_WR_CHAR_CFM message. Confirmation from the APP.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_wr_char_cfm_handler(ke_msg_id_t const msgid,
                                      struct udss_wr_char_cfm *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    if (ke_state_get(dest_id) == UDSS_STATE_OP_WRITE)
    {
        // Send data to peer device
        struct gattc_write_cfm *p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM,
                KE_BUILD_ID(TASK_GATTC, KE_IDX_GET(dest_id)),
                dest_id, 
                gattc_write_cfm);

        uint16_t handle;

        // Fill data
        udss_char_2_handle(&handle, p_param->char_idx);

        p_cfm->handle = handle;
        p_cfm->status = p_param->status;
        ke_msg_send(p_cfm);

        // go to idle state
        ke_state_set(dest_id, UDSS_STATE_IDLE);
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_DB_CHG_INC_NOTIFY_CMD message. APP request to Send IND.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_db_chg_inc_notify_cmd_handler(ke_msg_id_t const msgid,
                                          struct udss_db_chg_inc_notify_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    // check that task is in idle state
    if (ke_state_get(dest_id) == UDSS_STATE_IDLE)
    {
        // Get the address of the environment
        struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
        uint8_t conidx = KE_IDX_GET(dest_id);

        //check the CCC
        if (p_udss_env->db_chg_inc_ccc[conidx] & ATT_CCC_START_NTF)
        {
            uint16_t length = sizeof(uint32_t);
            // Allocate the GATT notification message
            struct gattc_send_evt_cmd *p_udss_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, conidx),
                    dest_id,
                    gattc_send_evt_cmd, length);

            // Fill in the p_parameter structure
            p_udss_val->operation = GATTC_NOTIFY;
            p_udss_val->handle = p_udss_env->shdl + UDSS_IDX_DB_CHG_INC_VAL;
            // pack db inc value in database
            co_write32p(&p_udss_val->value[0], p_param->value);
            p_udss_val->length = length;

            // send notification to peer device
            ke_msg_send(p_udss_val);

            // go to busy state
            ke_state_set(dest_id, UDSS_STATE_OP_NOTIFY);
        }
        else
        {
            struct udss_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(UDSS_CMP_EVT,
                    src_id,
                    dest_id, udss_cmp_evt);

            p_cmp_evt->operation = UDSS_NOTIFY_OP_CODE;
            p_cmp_evt->status = PRF_ERR_NTF_DISABLED;

            ke_msg_send(p_cmp_evt);
        }
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_USER_CTRL_PT_INDICATE_CMD message. APP request to Send IND.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_user_ctrl_pt_indicate_cmd_handler(ke_msg_id_t const msgid,
                                          struct udss_user_ctrl_pt_indicate_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    // check that task is in idle state
    if (ke_state_get(dest_id) == UDSS_STATE_IDLE)
    {
        // Get the address of the environment
        struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);
        uint8_t conidx = KE_IDX_GET(dest_id);

        //check the CCC
        if (p_udss_env->user_ctrl_pt_ccc[conidx] & ATT_CCC_START_IND)
        {
            // Mandatory: resp_op-code, req_op-code, resp_value
            uint16_t length = 3 + p_param->length;
            struct gattc_send_evt_cmd *p_udss_val;

            if (p_param->length > UDS_USER_CTRL_PT_MAX_LEN)
            {
                length = UDS_USER_CTRL_PT_MAX_LEN;
            }

            // Allocate the GATT indication message
            p_udss_val = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                    KE_BUILD_ID(TASK_GATTC, conidx),
                    dest_id,
                    gattc_send_evt_cmd, length);

            // Fill in the p_parameter structure
            p_udss_val->operation = GATTC_INDICATE;
            p_udss_val->handle = p_udss_env->shdl + UDSS_IDX_USER_CTRL_PT_VAL;
            // pack cntrl pt value in database
            p_udss_val->value[0] = p_param->resp_code;
            p_udss_val->value[1] = p_param->req_op_code;
            p_udss_val->value[2] = p_param->resp_value;

            // copy all additional parameters
            if (length > 3)
            {
                memcpy(&p_udss_val->value[3], &p_param->parameter[0], length - 3);
            }
            p_udss_val->length = length;

            // send indication to peer device
            ke_msg_send(p_udss_val);

            // go to busy state
            ke_state_set(dest_id, UDSS_STATE_OP_INDICATE);
        }
        else
        {
            struct udss_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(UDSS_CMP_EVT,
                    src_id,
                    dest_id, udss_cmp_evt);

            p_cmp_evt->operation = UDSS_INDICATE_OP_CODE;
            p_cmp_evt->status = PRF_ERR_IND_DISABLED;

            ke_msg_send(p_cmp_evt);
        }
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_RD_CHAR_UTF8_CFM message. APP read response of one of the CCC read.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_rd_char_utf8_cfm_handler(ke_msg_id_t const msgid,
                                      struct udss_rd_char_utf8_cfm *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    if (ke_state_get(dest_id) == UDSS_STATE_OP_READ)
    {
        struct gattc_read_cfm *p_cfm;
        uint16_t length = 0;
        uint16_t handle;

        if (p_param->status == GAP_ERR_NO_ERROR)
        {
            length = p_param->utf_name.length;
        }

        // Send data to peer device
        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                KE_BUILD_ID(TASK_GATTC, KE_IDX_GET(dest_id)),
                dest_id, 
                gattc_read_cfm, length);

        udss_char_2_handle(&handle, p_param->char_idx);

        p_cfm->handle = handle;
        p_cfm->length = length;
        p_cfm->status = p_param->status;

        if (length)
        {
            memcpy(&p_cfm->value[0], &p_param->utf_name.name[0], length);
        }

        ke_msg_send(p_cfm);

        // go to idle state
        ke_state_set(dest_id, UDSS_STATE_IDLE);
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSS_RD_CHAR_CFM message. APP read response of one of the CCC read.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udss_rd_char_cfm_handler(ke_msg_id_t const msgid,
                                      struct udss_rd_char_cfm *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    if (ke_state_get(dest_id) == UDSS_STATE_OP_READ)
    {
        // Send data to peer device
        struct gattc_read_cfm *p_cfm;
        uint8_t data[sizeof(union char_val)];
        uint16_t length = 0;
        uint16_t handle;

        if (p_param->status == GAP_ERR_NO_ERROR)
        {
            udss_pack_char_value(p_param->char_idx, &p_param->value, &data[0], &length);
        }

        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                KE_BUILD_ID(TASK_GATTC, KE_IDX_GET(dest_id)),
                dest_id, 
                gattc_read_cfm, length);

        udss_char_2_handle(&handle, p_param->char_idx);

        p_cfm->handle = handle;
        p_cfm->length = length;
        p_cfm->status = p_param->status;

        if (length)
        {
            memcpy(&p_cfm->value[0], &data[0], length);
        }

        ke_msg_send(p_cfm);

        // go to idle state
        ke_state_set(dest_id, UDSS_STATE_IDLE);
    }
    // else process it later
    else
    {
        msg_status = KE_MSG_SAVED;
    }

    return (msg_status);
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(udss)
{
    {GATTC_READ_REQ_IND,            (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_WRITE_REQ_IND,           (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,        (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_CMP_EVT,                 (ke_msg_func_t) gattc_cmp_evt_handler},

    {UDSS_ENABLE_REQ,               (ke_msg_func_t) udss_enable_req_handler},
    {UDSS_WR_CHAR_CFM,              (ke_msg_func_t) udss_wr_char_cfm_handler},
    {UDSS_RD_CHAR_UTF8_CFM,         (ke_msg_func_t) udss_rd_char_utf8_cfm_handler},
    {UDSS_RD_CHAR_CFM,              (ke_msg_func_t) udss_rd_char_cfm_handler},
    {UDSS_DB_CHG_INC_NOTIFY_CMD,    (ke_msg_func_t) udss_db_chg_inc_notify_cmd_handler},
    {UDSS_USER_CTRL_PT_INDICATE_CMD,(ke_msg_func_t) udss_user_ctrl_pt_indicate_cmd_handler},
};

void udss_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct udss_env_tag *p_udss_env = PRF_ENV_GET(UDSS, udss);

    p_task_desc->msg_handler_tab = udss_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(udss_msg_handler_tab);
    p_task_desc->state           = p_udss_env->state;
    p_task_desc->idx_max         = UDSS_IDX_MAX;
}

#endif //(BLE_UDS_SERVER)

/// @} UDSSTASK
