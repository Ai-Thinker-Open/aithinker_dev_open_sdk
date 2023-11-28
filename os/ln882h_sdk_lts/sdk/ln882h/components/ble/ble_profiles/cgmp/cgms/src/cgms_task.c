/**
 ****************************************************************************************
 *
 * @file cgms_task.c
 *
 * @brief Continuous Glucose Monitoring Profile Service Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup CGMSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CGM_SERVER)
#include "cgmp_common.h"

#include "gapc.h"
#include "gattc_task.h"
#include "attm.h"
#include "cgms.h"
#include "cgms_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief SEND RACP response indication.
 * If cp_opcode == CGMP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP
 * then sends the number of stored records response using number_of_records
 *
 * @param[in] dest_id        ID of the receiving task instance
 * @param[in] src_id         ID of the sending task instance.
 * @param[in] cp_opcode      Sent in the header of response- Mandatory
 * @param[in] req_cp_opcode  Copy the requested op_code
 * @param[in] rsp_code       Result code
 * @param[in] num_records    Used if cp_opcode == CGMP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP
 ****************************************************************************************
 */
__STATIC void cgms_racp_resp_send(ke_task_id_t const dest_id, ke_task_id_t const src_id, uint8_t cp_opcode,
        uint8_t req_cp_opcode, uint8_t rsp_code, uint16_t num_records)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_ind;
    // length of the data block
    uint16_t length;
    // handle to send the indication
    uint16_t handle;

    handle = cgms_att2handle(CGMS_IDX_RACP_VAL);
    length = 4; //OpCode -1oct,Operator=Null-1oct,Operand -2oct
    // Allocate the GATT notification message
    p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            dest_id,
            src_id,
            gattc_send_evt_cmd, length);

    // Fill in the parameter structure
    p_ind->operation = GATTC_INDICATE;
    p_ind->handle = handle;
    p_ind->length = length;
    // Fill data
    // Procedure Op Code
    p_ind->value[0] = cp_opcode;
    // Operator - Null
    p_ind->value[1] = CGMP_OP_NULL;
    if (cp_opcode == CGMP_OPCODE_NUM_OF_STRD_RECS_RSP)
    {
        co_write16p(&(p_ind->value[2]), num_records);
    }
    else
    {
        p_ind->value[2] = req_cp_opcode;
        p_ind->value[3] = rsp_code;
    }

    // send notification to peer device
    ke_msg_send(p_ind);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMS_ENABLE_REQ message.
 *
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgms_enable_req_handler(ke_msg_id_t const msgid,
                                    struct cgms_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Allocate Response message
    struct cgms_enable_rsp *p_cmp_evt = KE_MSG_ALLOC(CGMS_ENABLE_RSP, src_id, dest_id, cgms_enable_rsp);

    if (ke_state_get(dest_id) == CGMS_IDLE)
    {
        // CCCs for the current connection
        p_cgms_env->prfl_state[KE_IDX_GET(src_id)] = p_param->prfl_ntf_ind_cfg;

        status = GAP_ERR_NO_ERROR;
    }

    // send completed information to APP task that contains error status
    p_cmp_evt->status = status;

    ke_msg_send(p_cmp_evt);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMS_RD_CHAR_CFM message.
 * Read confirmation from the app with CGM Status, CGM Session Start Time, CGM Session Run Time
 *
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgms_rd_char_cfm_handler(ke_msg_id_t const msgid,
                                    struct cgms_rd_char_cfm *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == CGMS_OP_READ)
    {
        struct gattc_read_cfm *p_cfm;
        // Get the address of the environment
        struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
        // determine handle
        uint16_t handle = 0;
        // message size
        uint16_t length = 0;

        // add E2E-CRC
        if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
        {
            length += CGMP_CRC_LEN; // E2E-CRC size
        }

        switch (p_param->char_type)
        {
            case CGMS_CHAR_ID_STATUS_VAL:
            {
                handle = p_cgms_env->shdl + CGMS_IDX_STATUS_VAL;
                length += CGMS_RD_STATUS_CFM_SIZE;
            } break;

            case CGMS_CHAR_ID_SESSION_START_TIME_VAL:
            {
                handle = p_cgms_env->shdl + CGMS_IDX_SESS_ST_TIME_VAL;
                length += CGMS_RD_SESS_START_TIME_CFM_SIZE;
            } break;

            case CGMS_CHAR_ID_SESSION_RUN_TIME_VAL:
            {
                handle = p_cgms_env->shdl + CGMS_IDX_SESS_RUN_TIME_VAL;
                length += CGMS_RD_SESS_RUN_TIME_CFM_SIZE;
            } break;

            default:
              break;
        }

        // Send data to peer device
        p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM,
                KE_BUILD_ID(TASK_GATTC, KE_IDX_GET(dest_id)), dest_id,
                gattc_read_cfm, length);

        p_cfm->handle = handle;
        p_cfm->length = length;
        p_cfm->status = p_param->status;

        // Fill data per characteristic type
        switch (p_param->char_type)
        {
            case CGMS_CHAR_ID_STATUS_VAL:
            {
                co_write16p(&p_cfm->value[0], p_param->value.status.time_offset);
                p_cfm->value[2] = p_param->value.status.warning;
                p_cfm->value[3] = p_param->value.status.cal_temp;
                p_cfm->value[4] = p_param->value.status.annunc_status;
            } break;

            case CGMS_CHAR_ID_SESSION_START_TIME_VAL:
            {
                prf_pack_date_time(&p_cfm->value[0], &(p_param->value.sess_start_time.sess_start_time));
                p_cfm->value[7] = p_param->value.sess_start_time.time_zone;
                p_cfm->value[8] = p_param->value.sess_start_time.dst_offset;
            } break;

            case CGMS_CHAR_ID_SESSION_RUN_TIME_VAL:
            {
                co_write16p(&p_cfm->value[0], p_param->value.sess_run_time.run_time);
            } break;

            default:
              break;
        }

        // add E2E-CRC
        if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
        {
            uint16_t crc = prf_e2e_crc(&p_cfm->value[0],length-CGMP_CRC_LEN);

            co_write16p(&p_cfm->value[length-CGMP_CRC_LEN], crc);
        }

        ke_state_set(dest_id, CGMS_IDLE);
        // send message
        ke_msg_send(p_cfm);
    }
    // error condition
    else
    {
        // Send a command complete to the App  with error reason
        struct cgms_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(CGMS_CMP_EVT,
                src_id,
                dest_id, cgms_cmp_evt);

        // Read characteristic confirmation is not allowed
        p_cmp_evt->operation = CGMS_RD_CHAR_OP_CODE;
        p_cmp_evt->status = PRF_ERR_REQ_DISALLOWED;

        ke_msg_send(p_cmp_evt);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMS_WR_OPS_CTRL_PT_CFM message.
 * Confirmation from OPS Control Point
 *
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgms_wr_ops_ctrl_pt_cfm_handler(ke_msg_id_t const msgid,
                                    struct cgms_wr_ops_ctrl_pt_cfm *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t status = ATT_ERR_NO_ERROR;

    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (ke_state_get(dest_id) != CGMS_OP_WRITE_OPS_CTRL_PT)
    {
        // error condition
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // Only send if CCC is properly configured
    if ((status == ATT_ERR_NO_ERROR) && GETB(p_cgms_env->prfl_state[conidx], CGMS_IND_SOPS_ENABLED))
    {
        // handle to send the indication
        uint16_t handle = p_cgms_env->shdl + CGMS_IDX_SPEC_OPS_CTRL_PT_VAL;
        // length of the data block
        uint16_t length = 2 * sizeof(uint8_t); // minimum Op-Code & Response codes present
        // Allocate the GATT notification message
        struct gattc_send_evt_cmd *p_ind;

        switch (p_param->opcode)
        {
            // Communication interval in minutes                3
            case CGMP_OPS_CODE_CGM_COM_INTVL_RSP:
            // Request Op Code, Response Code Value             28
            case CGMP_OPS_CODE_RSP_CODE:
            {
                length += sizeof(uint16_t);
            } break;

            // Calibration Data                                 6
            case CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP:
            {
                if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_CALIB_SUP))
                {
                    length += CGMP_OPS_CALIBRATION_SIZE;
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            // Patient High bG value in mg/dL                   9
            case CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP:
            // Patient Low bG value in mg/dL                    12
            case CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP:
            {
                if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP))
                {
                    length += sizeof(uint16_t);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            // Hypo Alert Level value in mg/dL                  15
            case CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP:
            {
                if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_HYPO_ALERT_SUP))
                {
                    length += sizeof(uint16_t);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            // Hyper Alert Level value in mg/dL                 18
            case CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP:
            {
                if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_HYPER_ALERT_SUP))
                {
                    length += sizeof(uint16_t);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            // Rate of Decrease Alert Level value in mg/dL/min  21
            case CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP:
            // Rate of Increase Alert Level value in mg/dL/min  24
            case CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP:
            {
                if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP))
                {
                    length += sizeof(uint16_t);
                }
                else
                {
                    status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                }
            } break;

            default:
              break; //NA
        }

        // add E2E-CRC
        if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
        {
            length += CGMP_CRC_LEN; // E2E-CRC size
        }

        // Allocate the GATT notification message
        p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                KE_BUILD_ID(TASK_GATTC, conidx),
                dest_id,
                gattc_send_evt_cmd, length);

        // Fill in the parameter structure
        p_ind->operation = GATTC_INDICATE;
        p_ind->handle = handle;
        p_ind->length = length;
        // Fill data
        // Op Code
        p_ind->value[0] = p_param->opcode;
        // Response
        p_ind->value[1] = p_param->rsp;

        // operand based on op code
        switch (p_param->opcode)
        {
            // Communication interval in minutes                        3
            case CGMP_OPS_CODE_CGM_COM_INTVL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.intvl);
            } break;

            // Calibration Data Record Number                           5
            case CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.cal_data_record_num);
            } break;

            // Patient High bG value in mg/dL                           9
            case CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.pat_high_bg);
            } break;

            // Patient Low bG value in mg/dL                            12
            case CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.pat_low_bg);
            } break;

            // Hypo Alert Level value in mg/dL                          15
            case CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.hypo_alert_level);
            } break;

            // Hyper Alert Level value in mg/dL                         18
            case CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.hyper_alert_level);
            } break;

            // Rate of Decrease Alert Level value in mg/dL/min          21
            case CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.rate_decr_alert_level);
            } break;

            // Rate of Increase Alert Level value in mg/dL/min          24
            case CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.rate_incr_alert_level);
            } break;

            // Request Op Code, Response Code Value                     28
            case CGMP_OPS_CODE_RSP_CODE:
            {
                p_ind->value[2] = p_param->operand.req_rsp_value.req_op_code;
                p_ind->value[3] = p_param->operand.req_rsp_value.rsp_code;
            } break;

            // Calibration Data.                                         6
            case CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP:
            {
                co_write16p(&(p_ind->value[2]), p_param->operand.calib.cal_gluc_concent);
                co_write16p(&(p_ind->value[4]), p_param->operand.calib.cal_time);
                p_ind->value[6] = p_param->operand.calib.cal_type_spl;
                p_ind->value[7] = p_param->operand.calib.cal_spl_loc;
                co_write16p(&(p_ind->value[8]), p_param->operand.calib.next_cal_time);
                co_write16p(&(p_ind->value[10]), p_param->operand.calib.cal_record_num);
                p_ind->value[12] = p_param->operand.calib.cal_status;
            } break;

            default:
              break;
        }

        // add E2E-CRC
        if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
        {
            uint16_t crc = prf_e2e_crc(&p_ind->value[0], length-CGMP_CRC_LEN);
            co_write16p(&p_ind->value[length-CGMP_CRC_LEN], crc);
        }

        ke_state_set(dest_id, CGMS_OP_INDICATE_OPS_CTRL_PT);
        // send notification to peer device
        ke_msg_send(p_ind);
    }
    else
    {
        status = PRF_ERR_IND_DISABLED;
    }

    // in error case immediately send response to HL
    if (status != ATT_ERR_NO_ERROR)
    {
        // Send a command complete to the App  with error reason why Indicate msg could not be sent.
        struct cgms_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(CGMS_CMP_EVT,
                src_id,
                dest_id, cgms_cmp_evt);

        // Indication is Special OPS Control Point
        p_cmp_evt->operation = CGMS_OPS_CTRL_PT_OP_CODE;
        p_cmp_evt->status = status;

        ke_msg_send(p_cmp_evt);
    }

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
 *
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
    if(ke_state_get(dest_id) == CGMS_IDLE)
    {
        // Get the address of the environment
        struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
        uint8_t conidx = KE_IDX_GET(src_id);
        uint8_t att_idx = cgms_handle2att(p_param->handle);

        // Send data to peer device
        struct gattc_read_cfm *p_cfm = NULL;

        uint8_t status = ATT_ERR_NO_ERROR;

        switch(att_idx)
        {
            case CGMS_IDX_MEAS_CCC:
            case CGMS_IDX_SPEC_OPS_CTRL_PT_CCC:
            case CGMS_IDX_RACP_CCC:
            {

                // Fill data
                p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, sizeof(uint16_t));
                p_cfm->length = sizeof(uint16_t);

                switch(att_idx)
                {
                    uint16_t ccc;

                    case CGMS_IDX_MEAS_CCC:
                    {
                        // CCC for Measurement
                        ccc = (GETB(p_cgms_env->prfl_state[conidx], CGMS_NTF_MEAS_ENABLED) ? PRF_CLI_START_NTF : 0);
                        co_write16p(p_cfm->value, ccc);
                    } break;

                    case CGMS_IDX_SPEC_OPS_CTRL_PT_CCC:
                    {
                        // CCC for Specific OPS Ctrl Pt
                        ccc = (GETB(p_cgms_env->prfl_state[conidx], CGMS_IND_SOPS_ENABLED) ? PRF_CLI_START_IND : 0);
                        co_write16p(p_cfm->value, ccc);
                    } break;

                    case CGMS_IDX_RACP_CCC:
                    {
                        // CCC for RACP
                        ccc = (GETB(p_cgms_env->prfl_state[conidx], CGMS_IND_RACP_ENABLED) ? PRF_CLI_START_IND : 0);
                        co_write16p(p_cfm->value, ccc);
                    } break;

                    default:
                        break;
                }
            } break;

            case CGMS_IDX_STATUS_VAL:
            case CGMS_IDX_SESS_ST_TIME_VAL:
            case CGMS_IDX_SESS_RUN_TIME_VAL:
            {
                // need request APP
                struct cgms_rd_char_req_ind *p_ind = KE_MSG_ALLOC(CGMS_RD_CHAR_REQ_IND,
                        prf_dst_task_get(&(p_cgms_env->prf_env), conidx),
                        dest_id,
                        cgms_rd_char_req_ind);

                switch(att_idx)
                {
                    case CGMS_IDX_STATUS_VAL:
                    {
                        // Characteristic id @ref cgms_char_type
                        p_ind->char_type = CGMS_CHAR_ID_STATUS_VAL;
                    } break;

                    case CGMS_IDX_SESS_ST_TIME_VAL:
                    {
                        // Characteristic id @ref cgms_char_type
                        p_ind->char_type = CGMS_CHAR_ID_SESSION_START_TIME_VAL;
                    } break;

                    case CGMS_IDX_SESS_RUN_TIME_VAL:
                    {
                        // Characteristic id @ref cgms_char_type
                        p_ind->char_type = CGMS_CHAR_ID_SESSION_RUN_TIME_VAL;
                    } break;

                    default:
                        break;
                }

                // Send the request indication to the application
                ke_msg_send(p_ind);

                // go to busy state
                ke_state_set(dest_id, CGMS_OP_READ);
            } break;

            default:
            {
                p_cfm = KE_MSG_ALLOC(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm);
                p_cfm->length = 0;
                status = ATT_ERR_REQUEST_NOT_SUPPORTED;
            } break;
        }

        if (p_cfm != NULL)
        {
            p_cfm->handle = p_param->handle;
            p_cfm->status = status;

            // Send value to peer device.
            ke_msg_send(p_cfm);
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
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Get the address of the environment
    uint8_t att_idx = cgms_handle2att(p_param->handle);
    struct gattc_att_info_cfm *p_cfm;

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = p_param->handle;
    p_cfm->status = GAP_ERR_NO_ERROR;

    switch(att_idx)
    {
        case CGMS_IDX_MEAS_CCC:
        case CGMS_IDX_SPEC_OPS_CTRL_PT_CCC:
        case CGMS_IDX_RACP_CCC:
        {
            // check if it's a client configuration char
            // CCC attribute length = 2
            p_cfm->length = sizeof(uint16_t);
        } break;

        case CGMS_IDX_RACP_VAL:
        {
            // control point 
            p_cfm->length = CGMS_RACP_SIZE_MAX;
        } break;

        case CGMS_IDX_SESS_ST_TIME_VAL:
        {
            // control point 
            p_cfm->length = CGMP_SESS_ST_TIME_SIZE_MAX;
        } break;

        case CGMS_IDX_SPEC_OPS_CTRL_PT_VAL:
        {
            // control point 
            p_cfm->length = CGMS_OPS_CTRL_PT_SIZE_MAX;
        } break;

        default:
        {
            // not expected request
            p_cfm->length = 0;
            p_cfm->status = ATT_ERR_WRITE_NOT_PERMITTED;
        } break;
    }

    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_WRITE_REQ_IND message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                       struct gattc_write_req_ind const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    do
    {
        // Check the connection handle
        if (p_cgms_env == NULL)
        {
            break;
        }

        // check that task is in idle state
        if(ke_state_get(dest_id) == CGMS_IDLE)
        {
            uint8_t conidx = KE_IDX_GET(src_id);
            // Status
            uint8_t status = GAP_ERR_NO_ERROR;
            uint8_t att_idx = cgms_handle2att(p_param->handle);

            switch(att_idx)
            {
                case CGMS_IDX_MEAS_CCC:
                case CGMS_IDX_SPEC_OPS_CTRL_PT_CCC:
                case CGMS_IDX_RACP_CCC:
                {
                    uint16_t ntf_cfg;
                    // Inform the HL about the new configuration
                    struct cgms_wr_char_ccc_ind *p_ind = KE_MSG_ALLOC(CGMS_WR_CHAR_CCC_IND,
                            prf_dst_task_get(&(p_cgms_env->prf_env), conidx),
                            dest_id,
                            cgms_wr_char_ccc_ind);

                    // Get the value
                    co_write16p(&ntf_cfg, p_param->value[0]);

                    switch(att_idx)
                    {
                        case CGMS_IDX_MEAS_CCC:
                        {
                            // CCC for Measurement
                            p_ind->char_type = CGMS_CHAR_ID_MEAS_CCC;

                            if (ntf_cfg & PRF_CLI_START_NTF)
                            {
                                SETB(p_cgms_env->prfl_state[conidx], CGMS_NTF_MEAS_ENABLED, 1);
                            }
                        } break;

                        case CGMS_IDX_SPEC_OPS_CTRL_PT_CCC:
                        {
                            // CCC for Specific OPS Ctrl Pt
                            p_ind->char_type = CGMS_CHAR_ID_OPS_CTRL_PT_CCC;

                            if (ntf_cfg & PRF_CLI_START_IND)
                            {
                                SETB(p_cgms_env->prfl_state[conidx], CGMS_IND_SOPS_ENABLED, 1);
                            }
                        } break;

                        case CGMS_IDX_RACP_CCC:
                        {
                            // CCC for RACP
                            p_ind->char_type = CGMS_CHAR_ID_RACP_CCC;

                            if (ntf_cfg & PRF_CLI_START_IND)
                            {
                                SETB(p_cgms_env->prfl_state[conidx], CGMS_IND_RACP_ENABLED, 1);
                            }
                        } break;

                        default:
                            break;
                    }

                    p_ind->ind_cfg = ntf_cfg;

                    ke_msg_send(p_ind);

                } break;

                case CGMS_IDX_SESS_ST_TIME_VAL:
                {
                    if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                    {
                        if (p_param->length < CGMP_SESS_ST_TIME_SIZE_MAX)
                        {
                            status = CGM_ERROR_MISSING_CRC;
                        }
                        else
                        {
                            uint16_t crc = prf_e2e_crc(&p_param->value[0],
                                    CGMP_SESS_ST_TIME_SIZE_MAX - CGMP_CRC_LEN);

                            // check crc
                            if (crc != co_read16p(&(p_param->value[CGMP_SESS_ST_TIME_SIZE_MAX - CGMP_CRC_LEN])))
                            {
                                status = CGM_ERROR_INVALID_CRC;
                            }
                        }
                    }

                    if (status == GAP_ERR_NO_ERROR)
                    {
                        int8_t time_zone  = p_param->value[7];
                        uint8_t dst_offset = p_param->value[8];

                        if (!( (time_zone == CGM_TIME_ZONE_UNKNOWN) 
                            || ((time_zone >= CGM_TIME_ZONE_MIN) && (time_zone <= CGM_TIME_ZONE_MAX))))
                        {
                            // incorrect request
                            status = PRF_OUT_OF_RANGE;
                        }
                        if (!(  (dst_offset == CGM_DST_OFFSET_STANDARD_TIME) 
                              ||(dst_offset == CGM_DST_OFFSET_HALF_AN_HOUR_DAYLIGHT_TIME) 
                              ||(dst_offset == CGM_DST_OFFSET_DAYLIGHT_TIME) 
                              ||(dst_offset == CGM_DST_OFFSET_DOUBLE_DAYLIGHT_TIME) 
                              ||(dst_offset == CGM_DST_OFFSET_UNKNOWN_DAYLIGHT_TIME)))
                        {
                            // incorrect request
                            status = PRF_OUT_OF_RANGE;
                        }
                        if (status == GAP_ERR_NO_ERROR)
                        {
                            // Inform the HL about the new Time
                            struct cgms_wr_sess_start_time_ind *p_ind = KE_MSG_ALLOC(CGMS_WR_SESS_START_TIME_IND,
                                    prf_dst_task_get(&p_cgms_env->prf_env, conidx),
                                    prf_src_task_get(&p_cgms_env->prf_env, conidx),
                                    cgms_wr_sess_start_time_ind);

                            prf_unpack_date_time((uint8_t *) &p_param->value[0], &p_ind->sess_start_time);
                            p_ind->time_zone  = time_zone;
                            p_ind->dst_offset = dst_offset;

                            ke_msg_send(p_ind);
                        }
                    }
                } break;

                case CGMS_IDX_SPEC_OPS_CTRL_PT_VAL:
                {
                    if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                    {
                        uint16_t length = CGMP_CP_MIN_LEN;

                        // length of the message base on the OpCode
                        switch (p_param->value[0])
                        {
                            // Communication interval in minutes            1
                            case CGMP_OPS_CODE_SET_CGM_COM_INTVL:
                            // Communication interval in minutes            3
                            case CGMP_OPS_CODE_CGM_COM_INTVL_RSP:
                            // Calibration Data Record Number               5
                            case CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE:
                            // Patient High bG value in mg/dL               7
                            case CGMP_OPS_CODE_SET_PAT_HIGH_ALERT_LEVEL:
                            // Patient High bG value in mg/dL               9
                            case CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP:
                            // Patient Low bG value in mg/dL                10
                            case CGMP_OPS_CODE_SET_PAT_LOW_ALERT_LEVEL:
                            // Patient Low bG value in mg/dL                12
                            case CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP:
                            // Hypo Alert Level value in mg/dL              13
                            case CGMP_OPS_CODE_SET_HYPO_ALERT_LEVEL:
                            // Hypo Alert Level value in mg/dL              15
                            case CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP:
                            // Hyper Alert Level value in mg/dL             16
                            case CGMP_OPS_CODE_SET_HYPER_ALERT_LEVEL:
                            // Hyper Alert Level value in mg/dL             18
                            case CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP:
                            // value in mg/dL/min                           19
                            case CGMP_OPS_CODE_SET_RATE_OF_DECR_ALERT_LEVEL:
                            // value in mg/dL/min                           21
                            case CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP:
                            // value in mg/dL/min                           22
                            case CGMP_OPS_CODE_SET_RATE_OF_INCR_ALERT_LEVEL:
                            // value in mg/dL/min                           24
                            case CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP:
                            // Request Op Code, Response Code Value
                            case CGMP_OPS_CODE_RSP_CODE:
                            {
                                length += sizeof(uint16_t);
                            } break;

                            // defined in the Cal Value Fields              4
                            case CGMP_OPS_CODE_SET_GLUC_CALIB_VALUE:
                            // Calibration Data                             6
                            case CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP:
                            {
                                length += CGMP_OPS_CALIBRATION_SIZE;
                            } break;

                            default:
                                break;
                        }
                        if (p_param->length < length)
                        {
                            status = CGM_ERROR_MISSING_CRC;
                        }
                        else
                        {
                            uint16_t crc = prf_e2e_crc(&p_param->value[0], p_param->length - CGMP_CRC_LEN);

                            // check crc
                            if (crc != co_read16p(&(p_param->value[length])))
                            {
                                status = CGM_ERROR_INVALID_CRC;
                            }
                        }
                    }

                    if (status == GAP_ERR_NO_ERROR)
                    {
                        // Inform the HL about the new Time
                        struct cgms_wr_ops_ctrl_pt_req_ind *p_ind = KE_MSG_ALLOC(CGMS_WR_OPS_CTRL_PT_REQ_IND,
                                prf_dst_task_get(&p_cgms_env->prf_env, conidx),
                                prf_src_task_get(&p_cgms_env->prf_env, conidx),
                                cgms_wr_ops_ctrl_pt_req_ind);

                        // Fill data
                        memset(p_ind, 0, sizeof(struct cgms_wr_ops_ctrl_pt_req_ind));
                        // Op Code
                        p_ind->opcode = p_param->value[0];
                        // Response
                        p_ind->rsp = p_param->value[1];
                        // operand based on op code
                        switch (p_ind->opcode)
                        {
                            // Communication interval in minutes            1
                            case CGMP_OPS_CODE_SET_CGM_COM_INTVL:
                            // Communication interval in minutes            3
                            case CGMP_OPS_CODE_CGM_COM_INTVL_RSP:
                            {
                                p_ind->operand.intvl = co_read16p(&(p_param->value[2]));
                            } break;

                            // Calibration Data Record Number               5
                            case CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE:
                            {
                                p_ind->operand.cal_data_record_num = co_read16p(&(p_param->value[2]));
                            } break;

                            // Patient High bG value in mg/dL               7
                            case CGMP_OPS_CODE_SET_PAT_HIGH_ALERT_LEVEL:
                                // Patient High bG value in mg/dL           9
                            case CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.pat_high_bg = co_read16p(&(p_param->value[2]));
                            } break;

                            // Patient Low bG value in mg/dL                10
                            case CGMP_OPS_CODE_SET_PAT_LOW_ALERT_LEVEL:
                            // Patient Low bG value in mg/dL                12
                            case CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.pat_low_bg = co_read16p(&(p_param->value[2]));
                            } break;

                            // Hypo Alert Level value in mg/dL              13
                            case CGMP_OPS_CODE_SET_HYPO_ALERT_LEVEL:
                            // Hypo Alert Level value in mg/dL              15
                            case CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.hypo_alert_level = co_read16p(&(p_param->value[2]));
                            } break;

                            // Hyper Alert Level value in mg/dL             16
                            case CGMP_OPS_CODE_SET_HYPER_ALERT_LEVEL:
                            // Hyper Alert Level value in mg/dL             18
                            case CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.hyper_alert_level = co_read16p(&(p_param->value[2]));
                            } break;

                            // value in mg/dL/min                           19
                            case CGMP_OPS_CODE_SET_RATE_OF_DECR_ALERT_LEVEL:
                            // value in mg/dL/min                           21
                            case CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.rate_decr_alert_level = co_read16p(&(p_param->value[2]));
                            } break;

                            // value in mg/dL/min                           22
                            case CGMP_OPS_CODE_SET_RATE_OF_INCR_ALERT_LEVEL:
                            // value in mg/dL/min                           24
                            case CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP:
                            {
                                p_ind->operand.rate_incr_alert_level = co_read16p(&(p_param->value[2]));
                            } break;

                             // Request Op Code, Response Code Value        28
                            case CGMP_OPS_CODE_RSP_CODE:
                            {
                                p_ind->operand.req_rsp_value.req_op_code = p_param->value[2];
                                p_ind->operand.req_rsp_value.rsp_code = p_param->value[3];
                            } break;

                            // defined in Cal Value Fields                  4
                            case CGMP_OPS_CODE_SET_GLUC_CALIB_VALUE:
                            // Calibration Data                             6
                            case CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP:
                            {
                                p_ind->operand.calib.cal_gluc_concent          = co_read16p(&(p_param->value[2]));
                                p_ind->operand.calib.cal_time                  = co_read16p(&(p_param->value[4]));
                                p_ind->operand.calib.cal_type_spl              = p_param->value[6];
                                p_ind->operand.calib.cal_spl_loc               = p_param->value[7];
                                p_ind->operand.calib.next_cal_time             = co_read16p(&(p_param->value[8]));
                                p_ind->operand.calib.cal_record_num            = co_read16p(&(p_param->value[10]));
                                p_ind->operand.calib.cal_status                = p_param->value[12];
                            } break;

                            default:
                                break;
                        }

                        ke_msg_send(p_ind);
                        // go to busy state
                        ke_state_set(dest_id, CGMS_OP_WRITE_OPS_CTRL_PT);
                    }

                } break;

                case CGMS_IDX_RACP_VAL:
                {
                    // Op Code
                    uint8_t opcode = p_param->value[0];
                    // Operator
                    uint8_t operator = p_param->value[1];

                    // if command to RACP and RACP is not configured for indication
                    // or Command to report Records and SPOT_MEAS CCC is not configured for indication
                    if (!GETB(p_cgms_env->prfl_state[conidx], CGMS_IND_RACP_ENABLED)
                        || (( opcode == CGMP_OPCODE_REP_STRD_RECS)
                                && (!GETB(p_cgms_env->prfl_state[conidx], CGMS_NTF_MEAS_ENABLED))
                            )
                        )
                    {
                        // CPP improperly configured
                        status = PRF_CCCD_IMPR_CONFIGURED;
                    }
                    else if (GETB(p_cgms_env->prfl_state[conidx], CGMS_RACP_IN_PROG)
                             && (opcode != CGMP_OPCODE_ABORT_OP))
                    {
                        // procedure already in progress
                        status = PRF_PROC_IN_PROGRESS;
                    }
                    // Check it is the valid request
                    else if ((( opcode >= CGMP_OPCODE_REP_STRD_RECS)
                        && (opcode <= CGMP_OPCODE_REP_NUM_OF_STRD_RECS))
                        && (
                            ((opcode != CGMP_OPCODE_ABORT_OP)
                            && ((operator >= CGMP_OP_ALL_RECS) 
                            && (operator <= CGMP_OP_LAST_REC))) 
                            ||
                         ((opcode == CGMP_OPCODE_ABORT_OP) && (operator == CGMP_OP_NULL))
                          )
                        )
                    {
                        // Inform the HL about the new Control Point Command
                        struct cgms_wr_racp_ind *p_ind = KE_MSG_ALLOC(CGMS_WR_RACP_IND,
                                prf_dst_task_get(&p_cgms_env->prf_env, conidx),
                                prf_src_task_get(&p_cgms_env->prf_env, conidx),
                                cgms_wr_racp_ind);

                        // Get the value
                        p_ind->cp_opcode = opcode;
                        p_ind->cp_operator = operator;

                        // Get all Filter Parameter values no matter operator is
                        switch (operator)
                        {
                            case CGMP_OP_LT_OR_EQ:
                            {
                                p_ind->filter_type = p_param->value[2];
                                p_ind->max_time_offset = co_read16p(&(p_param->value[3]));
                            } break;

                            case CGMP_OP_GT_OR_EQ:
                            {
                                p_ind->filter_type = p_param->value[2];
                                p_ind->min_time_offset = co_read16p(&(p_param->value[3]));
                            } break;

                            case CGMP_OP_WITHIN_RANGE_OF:
                            {
                                p_ind->filter_type = p_param->value[2];
                                p_ind->min_time_offset = co_read16p(&(p_param->value[3]));
                                p_ind->max_time_offset = co_read16p(&(p_param->value[5]));
                            } break;

                            default:
                                break;
                        }

                        ke_msg_send(p_ind);

                        // set flag that procedure is started
                        SETB(p_cgms_env->prfl_state[conidx], CGMS_RACP_IN_PROG, 1);
                    }
                    else
                    {
                        //error condition
                        uint8_t rsp_code = CGMP_RSP_INV_OPERATOR;

                        if (  (opcode < CGMP_OPCODE_REP_STRD_RECS)
                            ||(opcode > CGMP_OPCODE_REP_NUM_OF_STRD_RECS))
                        { // incorrect opcode
                            rsp_code = CGMP_RSP_OP_CODE_NOT_SUP;
                        }
                        else if ((opcode != CGMP_OPCODE_ABORT_OP) && (operator > CGMP_OP_LAST_REC))
                        {
                            rsp_code = CGMP_RSP_OPERATOR_NOT_SUP;
                        }
                        else //if ((opcode != CGMP_OPCODE_ABORT_OPERATION) && (operator == CGMP_OPERATOR_NULL))
                        {
                            rsp_code = CGMP_RSP_INV_OPERATOR;
                        }

                        // procedure complete
                        SETB(p_cgms_env->prfl_state[conidx], CGMS_RACP_IN_PROG, 0);
                        cgms_racp_resp_send(src_id, dest_id, CGMP_OPCODE_RSP_CODE, opcode, rsp_code, 0);

                        // special state as we don't need response to the server
                        ke_state_set(dest_id, CGMS_OP_INDICATE_RACP_NO_CMP);
                    }
                } break;

                default:
                {
                    // not expected request
                    status = ATT_ERR_WRITE_NOT_PERMITTED;
                } break;
            }

            // Send the write response to the peer device
            struct gattc_write_cfm *p_cfm = KE_MSG_ALLOC(
                    GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);

            p_cfm->handle = p_param->handle;
            p_cfm->status = status;
            ke_msg_send(p_cfm);
        }
        else
        {
            msg_status = KE_MSG_SAVED;
        }

    } while(0);

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles @see GATT_NOTIFY_CMP_EVT message meaning that an indication
 * has been correctly sent to peer device (but not confirmed by peer device).
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);

    uint8_t state = ke_state_get(dest_id);

    do
    {
        if (p_cgms_env == NULL)
        {
            break;
        }

        if ( !((p_param->operation == GATTC_NOTIFY) || (p_param->operation == GATTC_INDICATE)) )
        {
            break;
        }

        // Measurement sent or RACP sent
        // Inform the application that a procedure has been completed
        if ((state == CGMS_OP_INDICATE_RACP )
            || (state == CGMS_OP_NOTIFY )
            || (state == CGMS_OP_INDICATE_OPS_CTRL_PT ))
        {
            // Send a command complete to the App
            struct cgms_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(CGMS_CMP_EVT,
                    prf_dst_task_get(&p_cgms_env->prf_env, conidx),
                    prf_src_task_get(&p_cgms_env->prf_env, conidx),
                    cgms_cmp_evt);

            if (state == CGMS_OP_NOTIFY)
            {
                // Continuous Measurement sent
                p_cmp_evt->operation = CGMS_MEAS_CMD_OP_CODE;
            }
            else if (state == CGMS_OP_INDICATE_RACP)
            {
                // Indication is RACP
                p_cmp_evt->operation = CGMS_RACP_CMD_OP_CODE;
            }
            else if (state == CGMS_OP_INDICATE_OPS_CTRL_PT)
            {
                // Indication is Special OPS Control Point
                p_cmp_evt->operation = CGMS_OPS_CTRL_PT_OP_CODE;
            }

            p_cmp_evt->status = p_param->status;

            ke_msg_send(p_cmp_evt);
        }
        else if (state != CGMS_OP_INDICATE_RACP_NO_CMP)
        {
            ASSERT_ERR(0);
        }

        // go to idle state
        ke_state_set(dest_id, CGMS_IDLE);

    } while (0);

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMS_MEAS_VALUE_CMD message.
 * Send MEASUREMENT INDICATION/NOTIFICATION to the connected peer case if CCC enabled
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgms_meas_value_cmd_handler(ke_msg_id_t const msgid,
                                          struct cgms_meas_value_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // check that task is in idle state
    if(ke_state_get(dest_id) == CGMS_IDLE)
    {
        // Connection index
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint16_t length;
        uint8_t status = ATT_ERR_NO_ERROR;

        // Get the address of the environment
        struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);
        uint16_t handle; // = p_cgms_env->shdl + CGMS_IDX_PRU_ALERT_VAL;
        // Allocate the GATT notification message
        struct gattc_send_evt_cmd *p_ind;
        // data pointer
        uint8_t *p_data;

        switch (p_param->operation)
        {
            case CGMS_MEAS_CMD_OP_CODE:
            {
                // check the CCC is enabled
                if (GETB(p_cgms_env->prfl_state[conidx], CGMS_NTF_MEAS_ENABLED))
                {
                    length = CGMS_MEAS_CMD_MIN_LENGTH;

                    // E2E-CRC
                    if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                    {
                        length += CGMP_CRC_LEN; // E2E-CRC size
                    }

                    // annunciation data
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN))
                    {
                        length += sizeof(uint8_t);
                    }
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP))
                    {
                        length += sizeof(uint8_t);
                    }
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS))
                    {
                        length += sizeof(uint8_t);
                    }

                    if ((GETB(p_cgms_env->cgm_feature, CGM_FEAT_CGM_TREND_INFO_SUP))
                       && (GETB(p_param->flags, CGM_MEAS_FLAGS_CGM_TREND_INFO)))
                    {
                        length += 2 * sizeof(uint8_t);
                    }
                    else
                    {
                        SETB(p_param->flags, CGM_MEAS_FLAGS_CGM_TREND_INFO, 0);
                    }

                    if ((GETB(p_cgms_env->cgm_feature, CGM_FEAT_CGM_QUALITY_SUP))
                        && (GETB(p_param->flags, CGM_MEAS_FLAGS_CGM_QUALITY)))
                    {
                        length += 2 * sizeof(uint8_t);
                    }
                    else
                    {
                        SETB(p_param->flags, CGM_MEAS_FLAGS_CGM_QUALITY, 0);
                    }

                    p_param->flags &= CGMS_MEAS_FLAGS_VALID;

                    handle = cgms_att2handle(CGMS_IDX_MEAS_VAL);
                    // Allocate the GATT notification message
                    p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, conidx),
                            dest_id,
                            gattc_send_evt_cmd, length);

                    // Fill in the parameter structure
                    p_ind->operation = GATTC_NOTIFY;
                    p_ind->handle = handle;
                    p_ind->length = length;

                    // fill data
                    //SIZE field
                    p_ind->value[0] = length;
                    // FLAGS field
                    p_ind->value[1] = p_param->flags;
                    // CGM Glucose Concentration (SFLOAT/16bit)
                    p_data = &(p_ind->value[2]);
                    co_write16p(p_data, p_param->gluc_concent);
                    p_data +=2;

                    // Time Offset
                    co_write16p(p_data, p_param->time_offset);
                    p_data +=2;

                    // annunciation data
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN))
                    {
                        // If Patient High/Low Alert is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP))
                        {
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_PAT, 0);
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_PAT, 0);
                        }

                        // If Hypo Alert is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_HYPO_ALERT_SUP))
                        {
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_HYPO, 0);
                        }

                        // If Hyper Alert is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_HYPER_ALERT_SUP))
                        {
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_HYPER, 0);
                        }

                        // If Rate of Increase/Decrease Alert is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP))
                        {
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_INCR, 0);
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RATE_DECR, 0);
                        }

                        // If Sensor Result High-Low Detection is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_SENSOR_RESULT_HIGH_LOW_DETECT_SUP))
                        {
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_LOWER_DEV, 0);
                            SETB(p_param->warn, CGM_MEAS_ANNUNC_WARN_SENSOR_RES_HIGHER_DEV, 0);
                        }

                        *p_data = p_param->warn;
                        p_data += 1;
                    }

                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP))
                    {
                        // If Calibration is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_CALIB_SUP))
                        {
                            SETB(p_param->cal_temp, CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_NOT_ALLOWED, 0);
                            SETB(p_param->cal_temp, CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_RECMD, 0);
                            SETB(p_param->cal_temp, CGM_MEAS_ANNUNC_CAL_TEMP_CALIB_REQ, 0);
                        }

                        // If Sensor Temperature High-Low Detection is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_SENSOR_TEMP_HIGH_LOW_DETECT_SUP))
                        {
                            SETB(p_param->cal_temp, CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_HIGH, 0);
                            SETB(p_param->cal_temp, CGM_MEAS_ANNUNC_CAL_TEMP_SENSOR_TEMP_LOW, 0);
                        }

                        *p_data = p_param->cal_temp;
                        p_data += 1;
                    }

                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS))
                    {
                        // If Device Specific Alert is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_DEV_SPEC_ALERT_SUP))
                        {
                            SETB(p_param->status, CGM_MEAS_ANNUNC_STATUS_DEV_SPEC_ALERT, 0);
                        }

                        // If Sensor Malfunction Detection is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_SENSOR_MALFUNC_DETEC_SUP))
                        {
                            SETB(p_param->status, CGM_MEAS_ANNUNC_STATUS_SENSOR_MALFUNC, 0);
                        }

                        // If Low Battery Detection is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_LOW_BATT_DETECT_SUP))
                        {
                            SETB(p_param->status, CGM_MEAS_ANNUNC_STATUS_DEV_BATT_LOW, 0);
                        }

                        // If Sensor Type Error Detection is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_SENSOR_TYPE_ERR_DETECT_SUP))
                        {
                            SETB(p_param->status, CGM_MEAS_ANNUNC_STATUS_SENSOR_TYPE_INCOR, 0);
                        }

                        // If General Device Fault is not supported,
                        // corresponding bit in Sensor Status Annunciation shall be set to 0
                        if (!GETB(p_cgms_env->cgm_feature, CGM_FEAT_GENERAL_DEV_FAULT_SUP))
                        {
                            SETB(p_param->status, CGM_MEAS_ANNUNC_STATUS_GEN_DEV_FAULT, 0);
                        }

                        *p_data = p_param->status;
                        p_data += 1;
                    }

                    // Trend data
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_CGM_TREND_INFO))
                    {
                        co_write16p(p_data, p_param->trend_info);
                        p_data += 2;
                    }

                    // Quality data
                    if (GETB(p_param->flags, CGM_MEAS_FLAGS_CGM_QUALITY))
                    {
                        co_write16p(p_data, p_param->quality);
                        p_data += 2;
                    }

                    // E2E-CRC
                    if (GETB(p_cgms_env->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                    {
                        uint16_t crc = prf_e2e_crc(&p_ind->value[0], length-CGMP_CRC_LEN);

                        co_write16p(p_data, crc);
                    }

                    // send notification to peer device
                    ke_msg_send(p_ind);

                    // go to busy state
                    ke_state_set(dest_id, CGMS_OP_NOTIFY);
                }
                else
                {
                    status = PRF_ERR_NTF_DISABLED;
                }
            } break;

            default:
            {
                status = PRF_ERR_INVALID_PARAM;
            } break;
        }

        // in error case immediately send response to HL
        if (status != ATT_ERR_NO_ERROR)
        {
            // Send a command complete to the App with error status indicate msg could not be sent.
            struct cgms_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(CGMS_CMP_EVT,
                    src_id,
                    dest_id, cgms_cmp_evt);

            p_cmp_evt->operation = p_param->operation;
            p_cmp_evt->status = status;

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
 * @brief Handles reception of the @see CGMS_RACP_RESP_CMD message.
 * Send MEASUREMENT INDICATION to the connected peer case if CCC enabled
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgms_racp_resp_cmd_handler(ke_msg_id_t const msgid,
                                          struct cgms_racp_resp_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    if(ke_state_get(dest_id) == CGMS_IDLE)
    {
        // Connection index
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t status = ATT_ERR_NO_ERROR;

        // Get the address of the environment
        struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);

        // do we have active procedure 
        // check the RACP procedure is ongoing
        if (GETB(p_cgms_env->prfl_state[conidx], CGMS_RACP_IN_PROG))
        {
            switch (p_param->operation)
            {
                case CGMS_RACP_CMD_OP_CODE:
                {
                    // check the CCC is enabled
                    if (GETB(p_cgms_env->prfl_state[conidx], CGMS_IND_RACP_ENABLED))
                    {
                        // procedure complete
                        SETB(p_cgms_env->prfl_state[conidx], CGMS_RACP_IN_PROG, 0);
                        cgms_racp_resp_send(
                                            KE_BUILD_ID(TASK_GATTC, conidx), 
                                            dest_id, 
                                            p_param->cp_opcode,
                                            p_param->req_cp_opcode,
                                            p_param->rsp_code,
                                            p_param->records_num);

                        // go to busy state
                        ke_state_set(dest_id, CGMS_OP_INDICATE_RACP);
                    }
                    else
                    {
                        status = PRF_ERR_IND_DISABLED;
                    }
                } break;
                
                default:
                {
                    status = PRF_ERR_INVALID_PARAM;
                } break;
            }
        }
        else
        {
            status = PRF_ERR_REQ_DISALLOWED;
        }

        // in error case immediately send response to HL
        if (status != ATT_ERR_NO_ERROR)
        {
            // Send a command complete to the App with error indicating why msg could not be sent.
            struct cgms_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(CGMS_CMP_EVT,
                    src_id,
                    dest_id, cgms_cmp_evt);

            p_cmp_evt->operation = p_param->operation;
            p_cmp_evt->status = status;

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

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(cgms)
{
    {CGMS_ENABLE_REQ,            (ke_msg_func_t) cgms_enable_req_handler},
    {CGMS_MEAS_VALUE_CMD,        (ke_msg_func_t) cgms_meas_value_cmd_handler},
    {CGMS_RACP_RESP_CMD,         (ke_msg_func_t) cgms_racp_resp_cmd_handler},
    {CGMS_RD_CHAR_CFM,           (ke_msg_func_t) cgms_rd_char_cfm_handler},
    {CGMS_WR_OPS_CTRL_PT_CFM,    (ke_msg_func_t) cgms_wr_ops_ctrl_pt_cfm_handler},
  
    {GATTC_READ_REQ_IND,         (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,     (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,        (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,              (ke_msg_func_t) gattc_cmp_evt_handler},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
void cgms_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct cgms_env_tag *p_cgms_env = PRF_ENV_GET(CGMS, cgms);

    p_task_desc->msg_handler_tab = cgms_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(cgms_msg_handler_tab);
    p_task_desc->state           = p_cgms_env->state;
    p_task_desc->idx_max         = CGMS_IDX_MAX;
}

#endif //(BLE_CGM_SERVER)

/// @} CGMSTASK
