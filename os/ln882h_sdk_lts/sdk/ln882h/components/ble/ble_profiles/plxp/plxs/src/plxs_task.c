/**
 ****************************************************************************************
 *
 * @file plxs_task.c
 *
 * @brief Pulse Oximeter Profile Service Task Implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup PLXSTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_PLX_SERVER)
#include "plxp_common.h"

#include "gapc.h"
#include "gattc_task.h"
#include "attm.h"
#include "plxs.h"
#include "plxs_task.h"
#include "prf_utils.h"

#include "ke_mem.h"
#include "co_utils.h"
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Send RACP response indication.
 * @param[in] dest_id           ID of the receiving task instance
 * @param[in] src_id            ID of the sending task instance.
 * @param[in] cp_opcode         Control Point Opcode @see enum plxp_cp_opcodes_id
 * @param[in] req_cp_opcode     Request Control Point OpCode @see enum plxp_cp_opcodes_id
 * @param[in] rsp_code          Response Code @see enum plxp_cp_resp_code_id
 * @param[in] rec_num           Number of Records
 ****************************************************************************************
 */
static void plxs_racp_resp_send(ke_task_id_t const dest_id, ke_task_id_t const src_id,
        uint8_t cp_opcode, uint8_t req_cp_opcode, uint8_t rsp_code, uint16_t rec_num)
{
    // Allocate the GATT notification message
    struct gattc_send_evt_cmd *p_ind;
    // length of the data block
    uint16_t length;
    // handle to send the indication
    uint16_t handle;

    handle = plxs_att2handle(PLXS_IDX_RACP_VAL);
    // OpCode -1oct,Operator=Null-1oct,Operand -2oct
    length = 4;
    // Allocate the GATT notification message
    p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
            dest_id,
            src_id,
            gattc_send_evt_cmd, length);

    // Fill in the p_parameter structure
    p_ind->operation = GATTC_INDICATE;
    p_ind->handle = handle;
    p_ind->length = length;
    // Fill data
    // Procedure Op Code
    p_ind->value[0] = cp_opcode;
    // Operator - Null
    p_ind->value[1] = PLXP_OPERATOR_NULL;

    if (cp_opcode == PLXP_OPCODE_NUMBER_OF_STORED_RECORDS_RESP)
    {
        co_write16p(&(p_ind->value[2]), rec_num);
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
 * @brief Handles reception of the @see PLXS_ENABLE_REQ message.
 * @param[in] msgid Id of the message received
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int plxs_enable_req_handler(ke_msg_id_t const msgid,
                                    struct plxs_enable_req *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    struct plxs_enable_rsp *cmp_evt;

    if (ke_state_get(dest_id) == PLXS_IDLE)
    {
        uint8_t conidx = KE_IDX_GET(src_id);
        // Get the address of the environment
        struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

        // CCCs for the current connection
        p_plxs_env->prfl_ind_cfg_spot[conidx] = p_param->ind_cfg_spot_meas;
        p_plxs_env->prfl_ind_cfg_cont[conidx] = p_param->ind_cfg_cont_meas;
        p_plxs_env->prfl_ind_cfg_racp[conidx] = p_param->ind_cfg_racp_meas;
        status = GAP_ERR_NO_ERROR;
    }

    // send completed information to APP task that contains error status
    cmp_evt = KE_MSG_ALLOC(PLXS_ENABLE_RSP, src_id, dest_id, plxs_enable_rsp);
    cmp_evt->status = status;

    ke_msg_send(cmp_evt);

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
static int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                                      struct gattc_read_req_ind const *p_param,
                                      ke_task_id_t const dest_id,
                                      ke_task_id_t const src_id)
{
    int msg_status = KE_MSG_CONSUMED;

    // check that task is in idle state
    if (ke_state_get(dest_id) == PLXS_IDLE)
    {
        // Get the address of the environment
        struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);
        uint8_t conidx = KE_IDX_GET(src_id);
        uint8_t att_idx = plxs_handle2att(p_param->handle);
        uint8_t status = ATT_ERR_NO_ERROR;

        // Send data to peer device
        struct gattc_read_cfm *p_cfm = KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id,
                dest_id, gattc_read_cfm, sizeof(uint16_t));

        switch(att_idx)
        {
            case PLXS_IDX_SPOT_MEASUREMENT_CCC:
            {
                // CCC for Spot-Measurement
                co_write16p(p_cfm->value, p_plxs_env->prfl_ind_cfg_spot[conidx]);
            } break;

            case PLXS_IDX_CONT_MEASUREMENT_CCC:
            {
                // CCC for Cont-Measurement
                co_write16p(p_cfm->value, p_plxs_env->prfl_ind_cfg_cont[conidx]);
            } break;

            case PLXS_IDX_RACP_CCC:
            {
                // CCC for RACP
                co_write16p(p_cfm->value, p_plxs_env->prfl_ind_cfg_racp[conidx]);
            } break;

            default:
            {
                status = ATT_ERR_REQUEST_NOT_SUPPORTED;
            } break;
        }

        // Fill data
        p_cfm->length = sizeof(uint16_t);
        p_cfm->handle = p_param->handle;
        p_cfm->status = status;

        // Send value to peer device.
        ke_msg_send(p_cfm);
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
static int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
        struct gattc_att_info_req_ind *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t att_idx = plxs_handle2att(p_param->handle);
    struct gattc_att_info_cfm *p_cfm;

    //Send write response
    p_cfm = KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = p_param->handle;

    switch(att_idx)
    {
        case PLXS_IDX_SPOT_MEASUREMENT_CCC:
        case PLXS_IDX_CONT_MEASUREMENT_CCC:
        case PLXS_IDX_RACP_CCC:
        {
            // check if it's a client configuration char
            // CCC attribute length = 2
            p_cfm->length = sizeof(uint16_t);
            p_cfm->status = GAP_ERR_NO_ERROR;
        } break;

        case PLXS_IDX_RACP_VAL:
        {
            // control point 
            p_cfm->length = 4;
            p_cfm->status = GAP_ERR_NO_ERROR;
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
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                       struct gattc_write_req_ind const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{

    // Message status
    int msg_status = KE_MSG_CONSUMED;

    do
    {
        // Get the address of the environment
        struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

        // Check the connection handle
        if (p_plxs_env == NULL)
        {
            break;
        }

        // check that task is in idle state
        if (ke_state_get(dest_id) == PLXS_IDLE)
        {
            // Status
            uint8_t status = GAP_ERR_NO_ERROR;
            uint8_t att_idx = plxs_handle2att(p_param->handle);
            uint8_t conidx = KE_IDX_GET(src_id);
            struct gattc_write_cfm *p_cfm;

            switch(att_idx)
            {
                case PLXS_IDX_SPOT_MEASUREMENT_CCC:
                case PLXS_IDX_CONT_MEASUREMENT_CCC:
                case PLXS_IDX_RACP_CCC:
                {
                    uint16_t ntf_cfg;
                    // Inform the HL about the new configuration
                    struct plxs_wr_char_ccc_ind *p_ind = KE_MSG_ALLOC(PLXS_WR_CHAR_CCC_IND,
                            prf_dst_task_get(&p_plxs_env->prf_env, conidx),
                            prf_src_task_get(&p_plxs_env->prf_env, conidx),
                            plxs_wr_char_ccc_ind);

                    // Get the value
                    co_write16p(&ntf_cfg, p_param->value[0]);

                    switch(att_idx)
                    {
                        case PLXS_IDX_SPOT_MEASUREMENT_CCC:
                        {
                            // CCC for Spot-Measurement
                            p_ind->char_type = PLXS_SPOT_CHECK_MEAS_CODE;
                            p_plxs_env->prfl_ind_cfg_spot[conidx] = ntf_cfg;
                        } break;

                        case PLXS_IDX_CONT_MEASUREMENT_CCC:
                        {
                            // CCC for Cont-Measurement
                            p_ind->char_type = PLXS_CONTINUOUS_MEAS_CODE;
                            p_plxs_env->prfl_ind_cfg_cont[conidx] = ntf_cfg;
                        } break;

                        case PLXS_IDX_RACP_CCC:
                        {
                            // CCC for RACP
                            p_ind->char_type = PLXS_RACP_CODE;
                            p_plxs_env->prfl_ind_cfg_racp[conidx] = ntf_cfg;
                        } break;

                        default:
                        {

                        } break;
                    }

                    p_ind->ind_cfg = ntf_cfg;

                    ke_msg_send(p_ind);
                } break;

                case PLXS_IDX_RACP_VAL:
                {
                    // if command to RACP and RACP is not configured for indication
                    // or Command to report Records and SPOT_MEAS CCC is not configured for indication
                    if (((p_plxs_env->prfl_ind_cfg_racp[conidx] & PRF_CLI_START_IND) != PRF_CLI_START_IND) ||
                        (( p_param->value[0] == PLXP_OPCODE_REPORT_STORED_RECORDS) &&
                        ((p_plxs_env->prfl_ind_cfg_spot[conidx] & PRF_CLI_START_IND) != PRF_CLI_START_IND)))
                    {
                        // CPP improperly configured
                        status = PRF_CCCD_IMPR_CONFIGURED;
                    }
                    else if ((p_plxs_env->racp_in_progress[conidx]) && (p_param->value[0] != PLXP_OPCODE_ABORT_OPERATION))
                    {
                        // procedure already in progress
                        status = PRF_PROC_IN_PROGRESS;
                    }
                    // Check it is the valid request
                    else if ((( p_param->value[0] >= PLXP_OPCODE_REPORT_STORED_RECORDS) &&
                            (p_param->value[0] <= PLXP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS)) &&
                            (((p_param->value[0] != PLXP_OPCODE_ABORT_OPERATION) &&
                            (p_param->value[1] == PLXP_OPERATOR_ALL_RECORDS)) ||
                            ((p_param->value[0] == PLXP_OPCODE_ABORT_OPERATION) &&
                            (p_param->value[1] == PLXP_OPERATOR_NULL)))
                        )
                    {
                        // Inform the HL about the new Control Point Command
                        struct plxs_wr_racp_ind *p_ind = KE_MSG_ALLOC(PLXS_WR_RACP_IND,
                                prf_dst_task_get(&p_plxs_env->prf_env, conidx),
                                prf_src_task_get(&p_plxs_env->prf_env, conidx),
                                plxs_wr_racp_ind);

                        // Get the value
                        p_ind->cp_opcode = p_param->value[0];
                        p_ind->cp_operator = p_param->value[1];

                        ke_msg_send(p_ind);
                        // set flag that procedure is started
                        p_plxs_env->racp_in_progress[conidx] = true;
                    }
                    else
                    {
                        //error condition
                        uint8_t response_code = PLXP_RESP_INVALID_OPERATOR;

                        if ((p_param->value[0]==0) ||
                                (p_param->value[0]>PLXP_OPCODE_REPORT_NUMBER_OF_STORED_RECORDS))
                        {
                            // incorrect opcode
                            response_code = PLXP_RESP_OP_CODE_NOT_SUPPORTED;
                        }
                        else if ((p_param->value[0] != PLXP_OPCODE_ABORT_OPERATION) &&
                                (p_param->value[1] > PLXP_OPERATOR_ALL_RECORDS))
                        {
                            response_code = PLXP_RESP_OPERATOR_NOT_SUPPORTED;
                        }
                        else // if ((p_param->value[0] != PLXP_OPCODE_ABORT_OPERATION) &&
                            // (p_param->value[1] == PLXP_OPERATOR_NULL))
                        {
                            response_code = PLXP_RESP_INVALID_OPERATOR;
                        }

                        // procedure complete
                        p_plxs_env->racp_in_progress[conidx] = 0;
                        plxs_racp_resp_send(src_id, dest_id, PLXP_OPCODE_RESPONSE_CODE,
                                p_param->value[0], response_code, 0);

                        // special state as we don't need response to the server
                        ke_state_set(dest_id, PLXS_OP_INDICATE_RACP_NO_CMP);
                    }
                } break;

                default:
                {
                    // not expected request
                    status = ATT_ERR_WRITE_NOT_PERMITTED;
                } break;
            }

            // Send the write response to the peer device
            p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
            p_cfm->handle = p_param->handle;
            p_cfm->status = status;
            ke_msg_send(p_cfm);
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
 * @brief Handles @see GATT_NOTIFY_CMP_EVT message meaning that an indication
 * has been correctly sent to peer device (but not confirmed by peer device).
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param     Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance
 * @param[in] src_id    ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int gattc_cmp_evt_handler(ke_msg_id_t const msgid,  struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    do
    {
        // Get the address of the environment
        struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

        if (p_plxs_env == NULL)
        {
            break;
        }

        // Check if a connection exists
        switch (p_param->operation)
        {
            case (GATTC_NOTIFY):
            case (GATTC_INDICATE):
            {
                uint8_t state = ke_state_get(dest_id);

                // Spot-Measurement sent or RACP sent
                // Inform the application that a procedure has been completed
                if ((state == PLXS_OP_INDICATE_MEAS) ||
                        (state == PLXS_OP_INDICATE_RACP) || (state == PLXS_OP_NOTIFY))
                {
                    uint8_t conidx = KE_IDX_GET(src_id);
                    // Send a command complete to the App indicate msg could not be sent.
                    struct plxs_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(PLXS_CMP_EVT,
                            prf_dst_task_get(&p_plxs_env->prf_env, conidx),
                            prf_src_task_get(&p_plxs_env->prf_env, conidx),
                            plxs_cmp_evt);

                    if (state == PLXS_OP_NOTIFY)
                    {
                        // Continuous Measurement sent
                        p_cmp_evt->operation = PLXS_CONTINUOUS_MEAS_CMD_OP_CODE;
                    }
                    else
                    {
                        // Indication is Spot-Meas Record or RACP
                        p_cmp_evt->operation = (state == PLXS_OP_INDICATE_MEAS ?
                                PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE : PLXS_RACP_CMD_OP_CODE);
                    }
                    p_cmp_evt->status = p_param->status;

                    ke_msg_send(p_cmp_evt);

                    // go to idle state
                    ke_state_set(dest_id, PLXS_IDLE);
                }
                else if (state == PLXS_OP_INDICATE_RACP_NO_CMP )
                {
                    // go to idle state
                    ke_state_set(dest_id, PLXS_IDLE);
                }
            } break;
                // else ignore the message
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
 * @brief Handles reception of the @see PLXS_MEAS_VALUE_CMD message.
 * @brief Send MEASUREMENT INDICATION/NOTIFICATION to the connected peer case if CCC enabled
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int plxs_meas_value_cmd_handler(ke_msg_id_t const msgid,
                                          struct plxs_meas_value_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;

     // check that task is in idle state
    if (ke_state_get(dest_id) == PLXS_IDLE)
    {
        // Allocate the GATT notification message
        struct gattc_send_evt_cmd *p_ind;
        // = p_plxs_env->shdl + PLXS_IDX_PRU_ALERT_VAL;
        uint16_t handle;
        // data pointer
        uint8_t *p_data;
        uint8_t status = ATT_ERR_NO_ERROR;
        uint16_t length;
        // Connection index
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

        switch (p_param->operation)
        {
            case PLXS_CONTINUOUS_MEAS_CMD_OP_CODE:
            {
                if ((p_plxs_env->prfl_ind_cfg_cont[conidx] & PRF_CLI_START_NTF)
                        && (p_plxs_env->optype != PLXS_OPTYPE_SPOT_CHECK_ONLY))
                {
                    handle = plxs_att2handle(PLXS_IDX_CONT_MEASUREMENT_VAL);
                    // Flags-1oct SpO2PR-Normal-4oct
                    length = 5;

                    // check the sup_feat vs Flags settings
                    // mask off unsupported features
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_SPO2PR_FAST_SUP))
                    {
                        SETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_SPO2PR_SLOW_SUP))
                    {
                        SETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_MEAS_STATUS_SUP))
                    {
                        SETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_MEAS_STATUS, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_DEV_SENSOR_STATUS_SUP))
                    {
                        SETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_PULSE_AMPL_SUP))
                    {
                        SETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_PULSE_AMPL, 0);
                    }
                    // allow only valid flags
                    p_param->value.cont_meas.cont_flags &= PLXP_CONT_MEAS_FLAGS_VALID_MASK;

                    // calculate size of data record
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST))
                    {
                        length += 4; // SpO2PR-Fast-4oct
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW))
                    {
                        length += 4; // SpO2PR-Slow-4oct
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_MEAS_STATUS))
                    {
                        length += 2; // Measurement Status
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS))
                    {
                        length += 3; // Device & Sensor Status
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_PULSE_AMPL))
                    {
                        length += 2; // Pulse Amplitude Index
                    }

                    // Allocate the GATT notification message
                    p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, conidx),
                            dest_id,
                            gattc_send_evt_cmd, length);

                    // Fill in the p_parameter structure
                    p_ind->operation = GATTC_NOTIFY;
                    p_ind->handle = handle;
                    p_ind->length = length;
                    // fill data
                    p_ind->value[0] = p_param->value.cont_meas.cont_flags;
                    p_data = &(p_ind->value[1]);
                    co_write16p(p_data,p_param->value.cont_meas.normal.sp_o2);
                    p_data +=2;
                    co_write16p(p_data,p_param->value.cont_meas.normal.pr);
                    p_data +=2;

                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_FAST))
                    {
                        // SpO2PR-Fast-4oct
                        co_write16p(p_data,p_param->value.cont_meas.fast.sp_o2);
                        p_data +=2;
                        co_write16p(p_data,p_param->value.cont_meas.fast.pr);
                        p_data +=2;
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_SPO2PR_SLOW))
                    {
                        // SpO2PR-Slow-4oct
                        co_write16p(p_data,p_param->value.cont_meas.slow.sp_o2);
                        p_data +=2;
                        co_write16p(p_data,p_param->value.cont_meas.slow.pr);
                        p_data +=2;
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_MEAS_STATUS))
                    {
                        // If bit of the Measurement Status field is not supported in the Feature,
                        // it shall always be set to 0 in its characteristic field
                        p_param->value.cont_meas.meas_stat &= p_plxs_env->meas_stat_sup;

                        // Measurement Status
                        co_write16p(p_data,p_param->value.cont_meas.meas_stat);
                        p_data +=2;
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_DEV_SENSOR_STATUS))
                    {
                        // If bit of the Device and Sensor Status field is not supported in the Feature,
                        // it shall always be set to 0 in its characteristic field
                        p_param->value.cont_meas.dev_sensor_stat &= p_plxs_env->dev_stat_sup;

                        // Device & Sensor Status
                        co_write24p(p_data,p_param->value.cont_meas.dev_sensor_stat);
                        p_data +=3;
                    }
                    if (GETB(p_param->value.cont_meas.cont_flags, PLXP_CONT_MEAS_FLAGS_PULSE_AMPL))
                    {
                        // Pulse Amplitude Index
                        co_write16p(p_data,p_param->value.cont_meas.pulse_ampl);
                        p_data +=2;
                    }

                    // send notification to peer device
                    ke_msg_send(p_ind);

                    // go to busy state
                    ke_state_set(dest_id, PLXS_OP_NOTIFY);
                }
                else
                {
                    status = PRF_ERR_IND_DISABLED;
                }
            } break;

            case PLXS_SPOT_CHECK_MEAS_CMD_OP_CODE:
            {
                if ((p_plxs_env->prfl_ind_cfg_spot[conidx] & PRF_CLI_START_IND) &&
                        (p_plxs_env->optype != PLXS_OPTYPE_CONTINUOUS_ONLY))
                {
                    handle = plxs_att2handle(PLXS_IDX_SPOT_MEASUREMENT_VAL);
                    length = 5; // Flags-1oct SpO2PR-Spot-Check-4oct
                    // check the sup_feat vs Flags settings
                    // mask off unsupported features
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_TIMESTAMP_SUP))
                    {
                        SETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_TIMESTAMP, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_MEAS_STATUS_SUP))
                    {
                        SETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_DEV_SENSOR_STATUS_SUP))
                    {
                        SETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS, 0);
                    }
                    if (!GETB(p_plxs_env->sup_feat, PLXP_FEAT_PULSE_AMPL_SUP))
                    {
                        SETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE, 0);
                    }

                    // allow only valid flags
                    p_param->value.spot_meas.spot_flags &= PLXP_SPOT_MEAS_FLAGS_VALID_MASK;

                    // calculate size of data record
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_TIMESTAMP))
                    {
                        length += 7; // Timestamp
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS))
                    {
                        length += 2; // Measurement Status
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS))
                    {
                        length += 3; // Device & Sensor Status
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE))
                    {
                        length += 2; // Pulse Amplitude Index
                    }

                    // Allocate the GATT notification message
                    p_ind = KE_MSG_ALLOC_DYN(GATTC_SEND_EVT_CMD,
                            KE_BUILD_ID(TASK_GATTC, conidx),
                            dest_id,
                            gattc_send_evt_cmd, length);

                    // Fill in the p_parameter structure
                    p_ind->operation = GATTC_INDICATE;
                    p_ind->handle = handle;
                    p_ind->length = length;
                    // fill data
                    p_ind->value[0] = p_param->value.spot_meas.spot_flags;
                    p_data = &(p_ind->value[1]);
                    co_write16p(p_data,p_param->value.spot_meas.sp_o2);
                    p_data +=2;
                    co_write16p(p_data,p_param->value.spot_meas.pr);
                    p_data +=2;

                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_TIMESTAMP))
                    {
                        // Timestamp-7oct
                        p_data +=prf_pack_date_time(p_data, &p_param->value.spot_meas.timestamp);
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_MEAS_STATUS))
                    {
                        // If bit of the Measurement Status field is not supported in the Feature,
                        // it shall always be set to 0 in its characteristic field
                        p_param->value.spot_meas.meas_stat &= p_plxs_env->meas_stat_sup;

                        // Measurement Status
                        co_write16p(p_data, p_param->value.spot_meas.meas_stat);
                        p_data +=2;
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_DEV_SENSOR_STATUS))
                    {
                        // If bit of the Device and Sensor Status field is not supported in the Feature,
                        // it shall always be set to 0 in its characteristic field
                        p_param->value.spot_meas.dev_sensor_stat &= p_plxs_env->dev_stat_sup;

                        // Device & Sensor Status
                        co_write24p(p_data,p_param->value.spot_meas.dev_sensor_stat);
                        p_data +=3;
                    }
                    if (GETB(p_param->value.spot_meas.spot_flags, PLXP_SPOT_MEAS_FLAGS_PULSE_AMPLITUDE))
                    {
                        // Pulse Amplitude Index
                        co_write16p(p_data,p_param->value.spot_meas.pulse_ampl);
                        p_data +=2;
                    }

                    // send notification to peer device
                    ke_msg_send(p_ind);

                    // go to busy state
                    ke_state_set(dest_id, PLXS_OP_INDICATE_MEAS);
                }
                else
                {
                    status = PRF_ERR_IND_DISABLED;
                }
            } break;

            default:
            {
                status = PRF_ERR_IND_DISABLED;
            } break;
        }

        // in error case immediately send response to HL
        if (status != ATT_ERR_NO_ERROR)
        {
            // Send a command complete to the App indicate msg could not be sent.
            struct plxs_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(PLXS_CMP_EVT,
                    src_id,
                    dest_id, plxs_cmp_evt);

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
 * @brief Handles reception of the @see PLXS_RACP_RESP_CMD message.
 * @brief Send MEASUREMENT INDICATION to the connected peer case if CCC enabled
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static int plxs_racp_resp_cmd_handler(ke_msg_id_t const msgid,
                                          struct plxs_racp_resp_cmd *p_param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    // Message status
    int msg_status = KE_MSG_CONSUMED;

     // check that task is in idle state
    if (ke_state_get(dest_id) == PLXS_IDLE)
    {
        uint8_t status = ATT_ERR_NO_ERROR;

        switch (p_param->operation)
        {
            case PLXS_RACP_CMD_OP_CODE:
            {
                // Connection index
                uint8_t conidx = KE_IDX_GET(dest_id);
                // Get the address of the environment
                struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

                if ((p_plxs_env->prfl_ind_cfg_racp[conidx] & PRF_CLI_START_IND) &&
                        (p_plxs_env->optype != PLXS_OPTYPE_CONTINUOUS_ONLY))
                {
                    // procedure complete
                    p_plxs_env->racp_in_progress[conidx] = 0;
                    plxs_racp_resp_send(KE_BUILD_ID(TASK_GATTC, conidx),
                                        dest_id, 
                                        p_param->cp_opcode,
                                        p_param->req_cp_opcode,
                                        p_param->rsp_code,
                                        p_param->rec_num);

                    // go to busy state
                    ke_state_set(dest_id, PLXS_OP_INDICATE_RACP);
                }
                else
                {
                    status = PRF_ERR_IND_DISABLED;
                }
            } break;

            default:
            {
                status = PRF_ERR_IND_DISABLED;
            } break;
        }

        // in error case immediately send response to HL
        if (status != ATT_ERR_NO_ERROR)
        {
            // Send a command complete to the App indicate msg could not be sent.
            struct plxs_cmp_evt *p_cmp_evt = KE_MSG_ALLOC(PLXS_CMP_EVT,
                    src_id,
                    dest_id, plxs_cmp_evt);

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
KE_MSG_HANDLER_TAB(plxs)
{
    {PLXS_ENABLE_REQ,          (ke_msg_func_t) plxs_enable_req_handler},
    {PLXS_MEAS_VALUE_CMD,      (ke_msg_func_t) plxs_meas_value_cmd_handler},
    {PLXS_RACP_RESP_CMD,       (ke_msg_func_t) plxs_racp_resp_cmd_handler},

    {GATTC_READ_REQ_IND,       (ke_msg_func_t) gattc_read_req_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,   (ke_msg_func_t) gattc_att_info_req_ind_handler},
    {GATTC_WRITE_REQ_IND,      (ke_msg_func_t) gattc_write_req_ind_handler},
    {GATTC_CMP_EVT,            (ke_msg_func_t) gattc_cmp_evt_handler},
};

void plxs_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct plxs_env_tag *p_plxs_env = PRF_ENV_GET(PLXS, plxs);

    p_task_desc->msg_handler_tab = plxs_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(plxs_msg_handler_tab);
    p_task_desc->state           = p_plxs_env->state;
    p_task_desc->idx_max         = PLXS_IDX_MAX;
}

#endif //(BLE_PLX_SERVER)

/// @} PLXSTASK
