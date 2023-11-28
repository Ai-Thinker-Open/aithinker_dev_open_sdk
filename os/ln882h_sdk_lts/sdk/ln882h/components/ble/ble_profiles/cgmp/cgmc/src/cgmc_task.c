/**
 ****************************************************************************************
 *
 * @file cgmc_task.c
 *
 * @brief Continuous Glucose Monitoring Profile Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2017
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup CGMCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_CGM_CLIENT)
#include "cgmp_common.h"
#include "cgmc_task.h"
#include "cgmc.h"

#include "gap.h"
#include "attm.h"
#include "gattc_task.h"
#include "ke_mem.h"
#include "ke_timer.h"
#include "co_utils.h"


/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve Continuous Glucose Monitoring Service characteristics information
const struct prf_char_def cgmc_cgm_char[CGMC_CHAR_MAX] =
{
    [CGMC_CHAR_MEAS]              = {ATT_CHAR_CGM_MEASUREMENT,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_NTF) },

    [CGMC_CHAR_FEAT]              = {ATT_CHAR_CGM_FEATURES,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_RD) },

    [CGMC_CHAR_STATUS]            = {ATT_CHAR_CGM_STATUS,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_RD) },

    [CGMC_CHAR_SESS_ST_TIME]      = {ATT_CHAR_CGM_SESSION_START,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_WR | ATT_CHAR_PROP_RD) },

    [CGMC_CHAR_SESS_RUN_TIME]     = {ATT_CHAR_CGM_SESSION_RUN,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_RD) },

    [CGMC_CHAR_RACP]              = {ATT_CHAR_REC_ACCESS_CTRL_PT,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND) },

    [CGMC_CHAR_SPEC_OPS_CTRL_PT]  = {ATT_CHAR_CGM_SPECIFIC_OPS_CTRL_PT,
                                    ATT_MANDATORY,
                                    (ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND) },
};

/// State machine used to retrieve Continuous Glucose Monitoring Service characteristic Description information
const struct prf_char_desc_def cgmc_cgm_char_desc[CGMC_DESC_MAX] =
{
    /// Client config
    [CGMC_DESC_MEAS_CCC]                 = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         CGMC_CHAR_MEAS},
    /// Client config
    [CGMC_DESC_RACP_CCC]                 = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         CGMC_CHAR_RACP},
    /// Client config
    [CGMC_DESC_SPEC_OPS_CTRL_PT_CCC]     = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         CGMC_CHAR_SPEC_OPS_CTRL_PT},
};

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_ind Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                                             struct gattc_sdp_svc_ind const *p_ind,
                                             ke_task_id_t const dest_id,
                                             ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == CGMC_DISCOVERING_SVC)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Continuous Glucose Monitoring Collector Role Task Environment
        struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cgmc_env->env[conidx] != NULL, dest_id, src_id);

        if(attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_CONTINUOUS_GLUCOSE_MONITORING))
        {
            // Retrieve CGMS characteristics and descriptors
            prf_extract_svc_info(p_ind, CGMC_CHAR_MAX, &cgmc_cgm_char[0],  &p_cgmc_env->env[conidx]->cgmc.chars[0],
                    CGMC_DESC_MAX, &cgmc_cgm_char_desc[0], &p_cgmc_env->env[conidx]->cgmc.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_cgmc_env->env[conidx]->cgmc.svc.shdl = p_ind->start_hdl;
            p_cgmc_env->env[conidx]->cgmc.svc.ehdl = p_ind->end_hdl;

            p_cgmc_env->env[conidx]->nb_svc++;
        }

    }

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_ENABLE_REQ message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int cgmc_enable_req_handler(ke_msg_id_t const msgid,
        struct cgmc_enable_req *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Status
    uint8_t status     = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t state = ke_state_get(dest_id);
    // Continuous Glucose Monitoring Collector Role Task Environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
    if ((state == CGMC_IDLE) && (p_cgmc_env->env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_cgmc_env->env[conidx] = (struct cgmc_cnx_env*) ke_malloc(sizeof(struct cgmc_cnx_env),KE_MEM_ATT_DB);
        memset(p_cgmc_env->env[conidx], 0, sizeof(struct cgmc_cnx_env));

        // Start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // Start discovery with Continuous Glucose Monitoring
            prf_disc_svc_send(&(p_cgmc_env->prf_env), conidx, ATT_SVC_CONTINUOUS_GLUCOSE_MONITORING);

            // Go to DISCOVERING SERVICE state
            ke_state_set(dest_id, CGMC_DISCOVERING_SVC);
        }
        // normal connection, get saved att details
        else
        {
            uint16_t search_hdl;
            struct prf_svc *p_svc;

            p_cgmc_env->env[conidx]->cgmc = p_param->cgmc;

            // start reading features
            // send APP confirmation that can start normal connection to TH
            // Attribute Handle
            search_hdl = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_FEAT].val_hdl;
            // Service
            p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

            // Check if handle is viable
            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                // Force the operation value
                p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_FEATURES_AT_INIT;

                // Send the write request
                prf_read_char_send(&(p_cgmc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
        }
    }
    else if (state != CGMC_FREE)
    {
        // The message will be forwarded towards the good task instance
        status = PRF_ERR_REQ_DISALLOWED;
    }

    if(status != GAP_ERR_NO_ERROR)
    {
        // The request is disallowed (profile already enabled for this connection, or not enough memory, ...)
        cgmc_enable_rsp_send(p_cgmc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_READ_CMD message from the application.
 * To read the Feature Characteristic in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int cgmc_read_cmd_handler(ke_msg_id_t const msgid,
        struct cgmc_read_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

            if ((p_svc != NULL) && (p_svc->shdl != ATT_INVALID_SEARCH_HANDLE))
            {
                if (p_param->operation == CGMC_OP_CODE_READ_FEATURES)
                {
                    // immediate results as features kept in the profile
                    // Send value ind and next complete event.
                    // indication Build a CGMC_VALUE_IND message
                    struct cgmc_value_ind *p_ind = KE_MSG_ALLOC(CGMC_VALUE_IND,
                                               prf_dst_task_get(&p_cgmc_env->prf_env, conidx),
                                               prf_src_task_get(&p_cgmc_env->prf_env, conidx),
                                               cgmc_value_ind);

                    p_ind->operation = CGMC_OP_CODE_READ_FEATURES;
                    p_ind->value.feat.cgm_feat     = p_cgmc_env->env[conidx]->cgm_feature;
                    p_ind->value.feat.type_samp    = p_cgmc_env->env[conidx]->type_sample;
                    p_ind->value.feat.samp_loc     = p_cgmc_env->env[conidx]->sample_location;

                    // send indication to the application
                    ke_msg_send(p_ind);

                    status = GAP_ERR_NO_ERROR;
                    cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_READ_FEATURES, status);
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_NO_OP;

                    return msg_status;
                }
                else if (p_param->operation == CGMC_OP_CODE_READ_STATUS)
                {
                    handle = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_STATUS].val_hdl;
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_STATUS;
                }
                else if (p_param->operation == CGMC_OP_CODE_READ_SESSION_START_TIME)
                {
                    handle = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_SESS_ST_TIME].val_hdl;
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_SESSION_START_TIME;
                }
                else if (p_param->operation == CGMC_OP_CODE_READ_SESSION_RUN_TIME)
                {
                    handle = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_SESS_RUN_TIME].val_hdl;
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_SESSION_RUN_TIME;
                }
            }

            // Check if handle is viable
            if (handle != ATT_INVALID_SEARCH_HANDLE)
            {
                // Send the read request
                prf_read_char_send(&(p_cgmc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, handle);
                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, p_param->operation, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_READ_CCC_CMD message.
 * Allows the application to read CCC values in the peer server
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgmc_read_ccc_cmd_handler(ke_msg_id_t const msgid,
                                   struct cgmc_read_ccc_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status     = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

            if (p_param->operation == CGMC_OP_CODE_MEASUREMENT)
            {
                search_hdl = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_MEAS_CCC].desc_hdl;
            }
            else if (p_param->operation == CGMC_OP_CODE_RACP)
            {
                search_hdl = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_RACP_CCC].desc_hdl;
            }
            else if (p_param->operation == CGMC_OP_CODE_SPECIFIC_OPS_CTRL_PT)
            {
                search_hdl = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_SPEC_OPS_CTRL_PT_CCC].desc_hdl;
            }

            // Check if handle is viable
            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                // Force the operation value
                p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_CCC;

                // Send read request
                prf_read_char_send(&(p_cgmc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);

                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_READ_CCC, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_WRITE_SESS_START_TIME_CMD message from the application.
 * To write command to the Records Access Control Point in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int cgmc_write_sess_start_time_cmd_handler(ke_msg_id_t const msgid,
        struct cgmc_write_sess_start_time_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_SESS_ST_TIME].val_hdl;
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

            // Check if handle is viable
            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                 // Length (7 -packed data 1 - zone 1- offset, 2-CRC)
                uint8_t length = CGMP_SESS_ST_TIME_SIZE_MAX;
                uint8_t data[CGMP_SESS_ST_TIME_SIZE_MAX];

                // fill parameters
                prf_pack_date_time(&data[0], &p_param->sess_start_time);
                data[7] = p_param->time_zone;
                data[8] = p_param->dst_offset;

                // add E2E-CRC
                if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                {
                    uint16_t crc = prf_e2e_crc(&data[0],length-CGMP_CRC_LEN);
                    co_write16p(&data[length-CGMP_CRC_LEN], crc);
                }
                else
                {
                    // less the crc 16bit
                    length -= sizeof(uint16_t);
                }

                // Force the operation value
                p_cgmc_env->env[conidx]->op_pending = CGMC_OP_CODE_WRITE_SESSION_START_TIME;

                // Send the write request
                prf_gatt_write(&(p_cgmc_env->prf_env), conidx, search_hdl, &data[0], length, GATTC_WRITE);
                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_WRITE_CTRL_PT, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_WRITE_RACP_CMD message from the application.
 * To write command to the Records Access Control Point in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int cgmc_write_racp_cmd_handler(ke_msg_id_t const msgid,
        struct cgmc_write_racp_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;
            // Attribute Handle
            uint16_t search_hdl = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_RACP].val_hdl;

            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                 // Length: op_code+operand + filter +2x2byte time offset
                uint8_t length = 2+5;
                uint8_t data[2+5];

                // fill parameters
                data[0] = p_param->cp_opcode;
                data[1] = p_param->cp_operator;

                switch (p_param->cp_opcode)
                {
                    case CGMP_OPCODE_REP_STRD_RECS:
                    case CGMP_OPCODE_REP_NUM_OF_STRD_RECS:
                    case CGMP_OPCODE_DEL_STRD_RECS:
                    {
                        switch (p_param->cp_operator)
                        {
                            case CGMP_OP_LT_OR_EQ:
                            {
                                // filter type is normally: CGMP_FILTER_TIME_OFFSET
                                data[2] = p_param->filter_type;
                                co_write16p(&data[3], p_param->max_time_offset);
                                length = 5;
                            } break;

                            case CGMP_OP_GT_OR_EQ:
                            {
                                // filter type is normally: CGMP_FILTER_TIME_OFFSET
                                data[2] = p_param->filter_type;
                                co_write16p(&data[3], p_param->min_time_offset);
                                length = 5;
                            } break;

                            case CGMP_OP_WITHIN_RANGE_OF:
                            {
                                // filter type is normally: CGMP_FILTER_TIME_OFFSET
                                data[2] = p_param->filter_type;
                                co_write16p(&data[3], p_param->min_time_offset);
                                co_write16p(&data[5], p_param->max_time_offset);
                                length = 7;
                            } break;

                            case CGMP_OP_ALL_RECS:
                            case CGMP_OP_FIRST_REC:
                            case CGMP_OP_LAST_REC:
                            default:
                            {
                                // send across anyway
                                length = 2;
                            } break;
                        }
                    } break;

                    case CGMP_OPCODE_ABORT_OP:
                    {
                        // fill parameters
                        data[1] = CGMP_OP_NULL; // only 
                        length = 2;
                    } break;

                    default:
                    {
                        // send across anyway
                        length = 2;
                    } break;
                }

                // Force the operation value
                p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_WRITE_RACP;

                // Send the write request
                prf_gatt_write(&(p_cgmc_env->prf_env), conidx, search_hdl, &data[0], length, GATTC_WRITE);
                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);
                // start the timer; will destroy the link if it expires
                ke_timer_set(CGMC_CP_REQ_TIMEOUT, dest_id, 10*CGMC_CP_TIMEOUT);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_WRITE_CTRL_PT, status);
    }

    return msg_status;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_WRITE_OPS_CTRL_PT_CMD message from the application.
 * To write command to the Specific Ops Control Point in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int cgmc_write_ops_ctrl_pt_cmd_handler(ke_msg_id_t const msgid,
        struct cgmc_write_ops_ctrl_pt_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // data pointer
    uint8_t  *p_data;

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if(p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_SPEC_OPS_CTRL_PT].val_hdl;
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

             // Length
            uint8_t length= 2 * sizeof(uint8_t); //min size response consist of opcode & response
            uint8_t data[(2 + CGMP_CALIBR_SIZE + CGMP_CRC_LEN)];

            // Check if handle is viable
            if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
            {
                // treat request as correct
                status = GAP_ERR_NO_ERROR;
                // fill parameters
                data[0] = p_param->opcode;
                data[1] = p_param->response;

                p_data = &(data[2]);
                switch(p_param->opcode)
                {
                    case CGMP_OPS_CODE_SET_CGM_COM_INTVL: // 1,
                    {
                        co_write16p(p_data, p_param->operand.intvl);
                        p_data +=2;
                        length += 2;
                    } break;
                    case CGMP_OPS_CODE_SET_GLUC_CALIB_VALUE: // 4,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_CALIB_SUP))
                        {
                            // Glucose Concentration at Calibration
                            co_write16p(p_data, p_param->operand.calib.cal_gluc_concent);
                            p_data +=2;
                            length += 2;
                            // Calibration Time in minutes
                            co_write16p(p_data, p_param->operand.calib.cal_time);
                            p_data +=2;
                            length += 2;
                            // Calibration Type of Sample @see common enum cgmp_type_sample_id
                            *p_data++ = p_param->operand.calib.cal_type_spl;
                            length++;
                            // Calibration Location of Sample @see common enum cgmp_sample_location_id
                            *p_data++ = p_param->operand.calib.cal_spl_loc;
                            length++;
                            // Next Calibration Time in minutes
                            co_write16p(p_data, p_param->operand.calib.next_cal_time);
                            p_data +=2;
                            length += 2;
                            // Calibration Record Number
                            co_write16p(p_data, p_param->operand.calib.cal_record_num);
                            p_data +=2;
                            length += 2;
                            // Calibration Status - bitfield @see common enum cgmp_calib_status_bf
                            *p_data++ = p_param->operand.calib.cal_status;
                            length++;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_GET_GLUC_CALIB_VALUE: // 5,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_CALIB_SUP))
                        {
                            // Calibration Record Number
                            co_write16p(p_data, p_param->operand.cal_data_record_num);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_PAT_HIGH_ALERT_LEVEL: // 7,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.pat_high_bg);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_PAT_LOW_ALERT_LEVEL: // 10,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.pat_low_bg);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_HYPO_ALERT_LEVEL: // 13,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_HYPO_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.hypo_alert_level);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_HYPER_ALERT_LEVEL: // 16,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_HYPER_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.hyper_alert_level);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_RATE_OF_DECR_ALERT_LEVEL: // 19,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.rate_decr_alert_level);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_SET_RATE_OF_INCR_ALERT_LEVEL: // 22,
                    {
                        if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP))
                        {
                            co_write16p(p_data, p_param->operand.rate_incr_alert_level);
                            p_data +=2;
                            length += 2;
                        }
                        else
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    // valid codes with no parameters
                    case CGMP_OPS_CODE_GET_CGM_COM_INTVL: // 2,
                    case CGMP_OPS_CODE_START_SESS: // 26,
                    case CGMP_OPS_CODE_STOP_SESS: // 27,
                    {

                    } break;

                    case CGMP_OPS_CODE_GET_PAT_HIGH_ALERT_LEVEL: // 8,
                    case CGMP_OPS_CODE_GET_PAT_LOW_ALERT_LEVEL: // 11,
                    {
                        if (!GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_PAT_HIGH_LOW_ALERT_SUP))
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_GET_HYPO_ALERT_LEVEL: // 14,
                    {
                        if (!GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_HYPO_ALERT_SUP))
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_GET_HYPER_ALERT_LEVEL: // 17,
                    {
                        if (!GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_HYPER_ALERT_SUP))
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_GET_RATE_OF_DECR_ALERT_LEVEL: // 20,
                    case CGMP_OPS_CODE_GET_RATE_OF_INCR_ALERT_LEVEL: // 23,
                    {
                        if (!GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_RATE_OF_INCR_DECR_ALERT_SUP))
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    case CGMP_OPS_CODE_RESET_DEV_SPEC_ALERT: // 25,
                    {
                        if (!GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_DEV_SPEC_ALERT_SUP))
                        {
                            status = PRF_ERR_FEATURE_NOT_SUPPORTED;
                        }
                    } break;

                    // invalid codes for request
                    case CGMP_OPS_CODE_CGM_COM_INTVL_RSP: // 3,
                    case CGMP_OPS_CODE_GLUC_CALIB_VALUE_RSP: // 6,
                    case CGMP_OPS_CODE_PAT_HIGH_ALERT_LEVEL_RSP: // 9,
                    case CGMP_OPS_CODE_PAT_LOW_ALERT_LEVEL_RSP: // 12,
                    case CGMP_OPS_CODE_HYPO_ALERT_LEVEL_RSP: // 15,
                    case CGMP_OPS_CODE_HYPER_ALERT_LEVEL_RSP: // 18,
                    case CGMP_OPS_CODE_RATE_OF_DECR_ALERT_LEVEL_RSP: // 21,
                    case CGMP_OPS_CODE_RATE_OF_INCR_ALERT_LEVEL_RSP: // 24,
                    case CGMP_OPS_CODE_RSP_CODE: // 28,
                    default :
                    {
                        status = PRF_ERR_INVALID_PARAM;
                    } break;
                }

                if (status == GAP_ERR_NO_ERROR)
                {
                    // add E2E-CRC if applicable
                    if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                    {
                        uint16_t crc = prf_e2e_crc(&data[0],length);
                        co_write16p(p_data, crc);
                        p_data +=CGMP_CRC_LEN;
                        length += CGMP_CRC_LEN;
                    }

                    // Force the operation value
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_WRITE_SPECIFIC_OPS_CTRL_PT;

                    // Send the write request
                    prf_gatt_write(&(p_cgmc_env->prf_env), conidx, search_hdl, &data[0], length, GATTC_WRITE);
                    // Go to the Busy state
                    ke_state_set(dest_id, CGMC_BUSY);
                    // start the timer; will destroy the link if it expires
                    ke_timer_set(CGMC_CP_REQ_TIMEOUT, dest_id, 10*CGMC_CP_TIMEOUT);
                }
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        if (p_cgmc_env->env[conidx]->op_pending == CGMC_OP_PENDING_WRITE_SPECIFIC_OPS_CTRL_PT)
        {
            // Another Control Point request is pending
            status = PRF_PROC_IN_PROGRESS;
        }
        else
        {
            // Another procedure is pending, keep the command for later
            msg_status = KE_MSG_SAVED;
            status = GAP_ERR_NO_ERROR;
        }
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_WRITE_CTRL_PT, status);
    }

    return msg_status;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @see CGMC_CFG_CCC_CMD message.
 * Allows the application to write new CCC values to a Alert Characteristic in the peer server
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int cgmc_cfg_ccc_cmd_handler(ke_msg_id_t const msgid,
                                   struct cgmc_cfg_ccc_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status     = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    if (state == CGMC_IDLE)
    {
        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_cgmc_env->env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;

            if (p_param->operation == CGMC_OP_CODE_MEASUREMENT)
            {
                handle = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_MEAS_CCC].desc_hdl;
            }
            else if (p_param->operation == CGMC_OP_CODE_RACP)
            {
                handle = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_RACP_CCC].desc_hdl;
            }
            else if (p_param->operation == CGMC_OP_CODE_SPECIFIC_OPS_CTRL_PT)
            {
                handle = p_cgmc_env->env[conidx]->cgmc.descs[CGMC_DESC_SPEC_OPS_CTRL_PT_CCC].desc_hdl;
            }

            // Check if handle is viable
            if ((p_svc != NULL) && (handle != ATT_INVALID_SEARCH_HANDLE))
            {
                // Force the operation value
                p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_WRITE_CCC;

                // Send the write request
                prf_gatt_write_ntf_ind(&(p_cgmc_env->prf_env), conidx, handle, p_param->ccc);

                // Go to the Busy state
                ke_state_set(dest_id, CGMC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == CGMC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_WRITE_CCC, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_CMP_EVT message.
 *
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                 struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    if (p_cgmc_env != NULL)
    {
        // Status
        uint8_t status = PRF_APP_ERROR;
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (state == CGMC_DISCOVERING_SVC)
        {
            // clean state for the new discovering
            p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_NO_OP;

            if ((p_param->status == GAP_ERR_NO_ERROR) || (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND))
            {
                // Check service (if found)
                if (p_cgmc_env->env[conidx]->nb_svc)
                {
                    // Check service (mandatory)
                    status = prf_check_svc_char_validity(CGMC_CHAR_MAX,
                            p_cgmc_env->env[conidx]->cgmc.chars,
                            cgmc_cgm_char);

                    // Check Descriptors (mandatory)
                    if (status == GAP_ERR_NO_ERROR)
                    {
                        status = prf_check_svc_char_desc_validity(CGMC_DESC_MAX,
                                p_cgmc_env->env[conidx]->cgmc.descs,
                                cgmc_cgm_char_desc,
                                p_cgmc_env->env[conidx]->cgmc.chars);
                    }
                }
            }

            // request read FEATURES
            if (status == GAP_ERR_NO_ERROR)
            {
                // Attribute Handle
                uint16_t search_hdl = p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_FEAT].val_hdl;
                // Service
                struct prf_svc *p_svc = &p_cgmc_env->env[conidx]->cgmc.svc;
                
                // Check if handle is viable
                if ((p_svc != NULL) && (search_hdl != ATT_INVALID_SEARCH_HANDLE))
                {
                    // Force the operation value
                    p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_READ_FEATURES_AT_INIT;

                    // Send the read request
                    prf_read_char_send(&(p_cgmc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                    // Go to the Busy state
                    ke_state_set(dest_id, CGMC_BUSY);

                }
                else
                {
                    // Raise an CGMC_ENABLE_REQ complete event.
                    // Was an issue reading features
                    status = ATT_ERR_INVALID_HANDLE;
                }
            }

            // at this stage we send response only if it fail in some case
            if (status != GAP_ERR_NO_ERROR)
            {
                // Raise an CGMC_ENABLE_REQ complete event.
                ke_state_set(dest_id, CGMC_IDLE);

                cgmc_enable_rsp_send(p_cgmc_env, conidx, p_param->status);
            }
        }
        else if (state == CGMC_BUSY)
        {
            uint8_t op_pending = p_cgmc_env->env[conidx]->op_pending;

            status = p_param->status;
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    // Send the complete event status
                    switch(op_pending)
                    {
                        case CGMC_OP_PENDING_READ_FEATURES_AT_INIT:
                        {
                           // Raise an CGMC_ENABLE_REQ complete event.
                           // Was an issue reading features
                           cgmc_enable_rsp_send(p_cgmc_env, conidx, status);
                        } break;

                        case CGMC_OP_PENDING_READ_FEATURES:
                        {
                            // Send read complete event.
                            cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_READ_FEATURES, status);
                        } break;

                        case CGMC_OP_PENDING_READ_CCC:
                        {
                            // Send read complete event.
                            cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_READ_CCC, status);
                        } break;

                        default:
                            break;
                    }
                    ke_state_set(dest_id, CGMC_IDLE);
                }
                break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    uint8_t operation = CGMC_OP_PENDING_NO_OP;

                    switch(op_pending)
                    {
                        case CGMC_OP_PENDING_WRITE_RACP:
                        {
                            operation = CGMC_OP_CODE_WRITE_CTRL_PT;
                        } break;
                        case CGMC_OP_CODE_WRITE_SESSION_START_TIME:
                        {
                            operation = CGMC_OP_CODE_WRITE_SESSION_START_TIME;
                        } break;
                        case CGMC_OP_PENDING_WRITE_CCC:
                        {
                            operation = CGMC_OP_CODE_WRITE_CCC;
                        } break;
                        case CGMC_OP_PENDING_WRITE_SPECIFIC_OPS_CTRL_PT:
                        {
                            operation = CGMC_OP_CODE_WRITE_OPS_CTRL_PT;
                        } break;
                        default:
                            break;
                    }

                    // Send the complete event status
                    cgmc_send_cmp_evt(p_cgmc_env, conidx, operation, p_param->status);
                    ke_state_set(dest_id, CGMC_IDLE);

                } break;

                case GATTC_REGISTER:
                {
                    if (status != GAP_ERR_NO_ERROR)
                    {
                        // Send the complete event error status
                        cgmc_send_cmp_evt(p_cgmc_env, conidx, CGMC_OP_CODE_GATTC_REGISTER, p_param->status);
                    }
                    // Go to connected state
                    ke_state_set(dest_id, CGMC_IDLE);
                } break;

                case GATTC_UNREGISTER:
                {
                    // Do nothing
                } break;

                default:
                {
                    ASSERT_ERR(0);
                } break;
            }
            p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_NO_OP;
        }
    }
    // else ignore the message

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_READ_IND message.
 * Generic event received after every simple read command sent to peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_read_ind_handler(ke_msg_id_t const msgid,
                                    struct gattc_read_ind const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    if (ke_state_get(dest_id) == CGMC_BUSY)
    {
        // Get the address of the environment
        struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Service
        uint8_t op_pending;
        uint8_t status = GAP_ERR_NO_ERROR;

        // E2E crc field
        uint16_t  feature_crc;
        // calculated crc
        // e2e crc to protect the fields
        // predefined value when crc is not supported
        // this field exist in all the cases
        uint16_t crc = 0xFFFF;

        // Send value ind and next complete event.
        // indication in a CGMC_VALUE_IND message
        struct cgmc_value_ind *p_ind;

        // Flag to check if we need to return IDLE state
        uint8_t to_idle = 0;

        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cgmc_env->env[conidx] != NULL, dest_id, src_id);

        // Find the op_pending -  and check it is a valid read code.
        op_pending = p_cgmc_env->env[conidx]->op_pending;

        switch (op_pending)
        {
            case CGMC_OP_PENDING_READ_FEATURES_AT_INIT:
            case CGMC_OP_PENDING_READ_FEATURES:
            {
                // If enabled in CGM Features @see common cgm_features
                uint32_t cgm_feature;

                cgm_feature = co_read24p(&p_param->value[0]);

                // check if E2E-CRC is supported
                if (GETB(cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                {
                    crc = prf_e2e_crc(&p_param->value[0], 3+1); // 24bits 4bit 4bits
                    feature_crc = co_read16p(&p_param->value[4]);

                    // only take reading if crc match
                    if (feature_crc != crc)
                    {
                        status = CGM_ERROR_INVALID_CRC;
                    }
                }

                p_cgmc_env->env[conidx]->cgm_feature     = cgm_feature;
                p_cgmc_env->env[conidx]->type_sample     = (p_param->value[3]) & 0x0f;
                p_cgmc_env->env[conidx]->sample_location = (p_param->value[3] >> 4) & 0x0f;

                if (op_pending == CGMC_OP_PENDING_READ_FEATURES_AT_INIT)
                {
                    // Raise an CGMC_ENABLE_RSP complete event.
                    cgmc_enable_rsp_send(p_cgmc_env, conidx, status);
                    // no need for complete event at the end
                    status = GAP_ERR_NO_ERROR;
                }

                to_idle = 1;
            } break;

            case CGMC_OP_PENDING_READ_STATUS:
            case CGMC_OP_PENDING_READ_SESSION_START_TIME:
            case CGMC_OP_PENDING_READ_SESSION_RUN_TIME:
            {
                // Offset
                uint8_t offs = 0;
                // Operation code for CMP EVT
                uint8_t op_code = CGMC_NO_OP;

                // Send value ind and next complete event.
                p_ind = KE_MSG_ALLOC(CGMC_VALUE_IND,
                                           prf_dst_task_get(&p_cgmc_env->prf_env, conidx),
                                           prf_src_task_get(&p_cgmc_env->prf_env, conidx),
                                           cgmc_value_ind);
                if (op_pending == CGMC_OP_PENDING_READ_STATUS)
                {
                    p_ind->operation = CGMC_OP_CODE_READ_STATUS;
                    p_ind->value.rd_status.time_offset     = co_read16p(&p_param->value[0]);
                    p_ind->value.rd_status.warning         = p_param->value[2];
                    p_ind->value.rd_status.cal_temp        = p_param->value[3];
                    p_ind->value.rd_status.annunc_status   = p_param->value[4];
                    offs = 5;

                    op_code = CGMC_OP_CODE_READ_STATUS;
                }
                else if (op_pending == CGMC_OP_PENDING_READ_SESSION_START_TIME)
                {
                    p_ind->operation = CGMC_OP_CODE_READ_SESSION_START_TIME;
                    offs = prf_unpack_date_time((uint8_t *) &(p_param->value[0]),
                            &(p_ind->value.start_time.sess_start_time));
                    p_ind->value.start_time.time_zone         = p_param->value[offs];
                    p_ind->value.start_time.dst_offset        = p_param->value[offs+1];
                    offs += 2;

                    op_code = CGMC_OP_CODE_READ_SESSION_START_TIME;
                }
                else if (op_pending == CGMC_OP_PENDING_READ_SESSION_RUN_TIME)
                {
                    p_ind->operation = CGMC_OP_CODE_READ_SESSION_RUN_TIME;
                    p_ind->value.run_time.run_time     = co_read16p(&p_param->value[0]);
                    offs = 2;

                    op_code = CGMC_OP_CODE_READ_SESSION_RUN_TIME;
                }

                // check E2E-CRC
                if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
                {
                    if (p_param->length < (offs+2)) // 16bit 24bits +16bit CRC
                    {
                        status = CGM_ERROR_MISSING_CRC;
                    }
                    else
                    {
                        feature_crc     = co_read16p(&p_param->value[offs]);
                        crc = prf_e2e_crc(&p_param->value[0], offs); // 16bit 24bits
                        // only take reading if crc match
                        if (feature_crc != crc)
                        {
                            status = CGM_ERROR_INVALID_CRC;
                        }
                    }
                }

                if (status == GAP_ERR_NO_ERROR)
                {
                    // send indication to the application
                    ke_msg_send(p_ind);
                }
                else
                {
                    // No need to send prepared message
                    KE_MSG_FREE(p_ind);
                }

                to_idle = 1;

                // event complete sent as a result of operation command request
                cgmc_send_cmp_evt(p_cgmc_env, conidx, op_code, status);

            } break;

            case CGMC_OP_PENDING_READ_CCC:
            {
                struct cgmc_rd_char_ccc_ind *p_ind = KE_MSG_ALLOC(CGMC_RD_CHAR_CCC_IND,
                                            prf_dst_task_get(&(p_cgmc_env->prf_env), conidx),
                                            dest_id,
                                            cgmc_rd_char_ccc_ind);

                p_ind->ind_cfg = co_read16p(&p_param->value[0]);

                ke_msg_send(p_ind);

                // Keep the Busy state to be treated in gattc_cmp_evt_handler
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }

        // return to IDLE state
        if (to_idle)
        {
            // translation from op_pending to NO_OP opcode
            p_cgmc_env->env[conidx]->op_pending = CGMC_OP_PENDING_NO_OP;

            ke_state_set(dest_id, CGMC_IDLE);
        }
    }
    // else drop the message

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @see GATTC_EVENT_IND message.
 *
 * @param[in] msgid    Id of the message received (probably unused).
 * @param[in] p_param  Pointer to the parameters of the message.
 * @param[in] dest_id  ID of the receiving task instance (probably unused).
 * @param[in] src_id   ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                   struct gattc_event_ind const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    do
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // data pointer
        const uint8_t *p_data;
        // length of data block
        uint16_t length = p_param->length;
        // E2E crc field
        uint16_t  feature_crc;
        // calculated crc
        // e2e crc to protect the fields
        uint16_t crc;
        //indication Build a CGMC_VALUE_IND message
        struct cgmc_value_ind *p_ind = NULL;

        if (p_cgmc_env == NULL)
        {
            break;
        }

        if ((p_param->type != GATTC_INDICATE) && (p_param->type != GATTC_NOTIFY))
        {
            break;
        }

        if (p_param->handle == p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_RACP].val_hdl)
        {
            // no crc for legacy reasons
            p_ind = KE_MSG_ALLOC(CGMC_VALUE_IND,
                                       prf_dst_task_get(&p_cgmc_env->prf_env, conidx),
                                       prf_src_task_get(&p_cgmc_env->prf_env, conidx),
                                       cgmc_value_ind);
            memset(p_ind, 0, sizeof(struct cgmc_value_ind));

            p_ind->operation = CGMC_OP_CODE_RACP;
            // RACP response carry number of records or Requested OpCode+ Response code
            p_ind->value.racp_rsp.cp_opcode = p_param->value[0];
            // RACP operator should be indicated (normally an Operator of NULL 0x00)
            p_ind->value.racp_rsp.cp_operator = p_param->value[1];

            if (p_param->value[0] == CGMP_OPCODE_NUM_OF_STRD_RECS_RSP)
            {
                p_ind->value.racp_rsp.records_num = co_read16p(&p_param->value[2]);
            }
            else
            {
                // Request Control Point OpCode @see enum cgmp_cp_opcodes_id
                p_ind->value.racp_rsp.req_cp_opcode = p_param->value[2];
                // Response Code @see enum cgmp_cp_resp_code_id
                p_ind->value.racp_rsp.rsp_code = p_param->value[3];
            }
            // stop timer.
            ke_timer_clear(CGMC_CP_REQ_TIMEOUT, dest_id);
        }
        else
        {
            //check E2E crc
            if (GETB(p_cgmc_env->env[conidx]->cgm_feature, CGM_FEAT_E2E_CRC_SUP))
            {
                feature_crc  = co_read16p(&p_param->value[length-CGMP_CRC_LEN]);

                // calc E2E-CRC
                crc = prf_e2e_crc(&p_param->value[0], length-CGMP_CRC_LEN);

                // only take reading if crc match
                if (feature_crc != crc)
                {
                    uint8_t operation = CGMC_OP_CODE_SPECIFIC_OPS_CTRL_PT;

                    if (p_param->handle == p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_MEAS].val_hdl)
                    {
                        operation = CGMC_OP_CODE_MEASUREMENT;
                    }
                    // Send the complete event status
                    cgmc_send_cmp_evt(p_cgmc_env, conidx, operation, CGM_ERROR_INVALID_CRC);
                }
            }

            p_ind = KE_MSG_ALLOC(CGMC_VALUE_IND,
                               prf_dst_task_get(&p_cgmc_env->prf_env, conidx),
                               prf_src_task_get(&p_cgmc_env->prf_env, conidx),
                               cgmc_value_ind);
            memset(p_ind, 0, sizeof(struct cgmc_value_ind));

            if (p_param->handle == p_cgmc_env->env[conidx]->cgmc.chars[CGMC_CHAR_MEAS].val_hdl)
            {
                uint8_t flags = p_param->value[1];

                p_ind->operation = CGMC_OP_CODE_MEASUREMENT;
                p_ind->value.meas.size = p_param->value[0];
                p_ind->value.meas.flags = flags;
                // CGM Glucose Concentration (SFLOAT/16bit)
                p_data = &(p_param->value[2]);
                p_ind->value.meas.gluc_concent = co_read16p(p_data);
                p_data +=2;
                // Time Offset
                p_ind->value.meas.time_offset = co_read16p(p_data);
                p_data +=2;

                // verify flags vs supported_features
                // annunciation data
                if (GETB(flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_WARN))
                {
                    p_ind->value.meas.warn = *(p_data++);
                }
                if (GETB(flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_CAL_TEMP))
                {
                    p_ind->value.meas.cal_temp = *(p_data++);
                }
                if (GETB(flags, CGM_MEAS_FLAGS_SENSOR_STATUS_ANNUNC_STATUS))
                {
                    p_ind->value.meas.status = *(p_data++);
                }

                // Trend data
                if (GETB(flags, CGM_MEAS_FLAGS_CGM_TREND_INFO))
                {
                    p_ind->value.meas.trend_info = co_read16p(p_data);
                    p_data += 2;
                }

                // Quality data
                if (GETB(flags, CGM_MEAS_FLAGS_CGM_QUALITY))
                {
                    p_ind->value.meas.quality = co_read16p(p_data);
                    p_data += 2;
                }

                // start the timer; will destroy the link if it expires
                ke_timer_set(CGMC_CP_REQ_TIMEOUT, dest_id, 10*CGMC_CP_TIMEOUT);
            }
            else if(p_param->handle == p_cgmc_env->env[conidx]->
                    cgmc.chars[CGMC_CHAR_SPEC_OPS_CTRL_PT].val_hdl)
            {
                // Control Point OpCode @see enum cgmp_cp_opcodes_id
                uint8_t cp_opcode = p_param->value[0];
                union cgmp_ops_operand *p_operand = &p_ind->value.ops_cp_rsp.operand;

                p_ind->operation = CGMC_OP_CODE_SPECIFIC_OPS_CTRL_PT;

                // Op Code
                p_ind->value.ops_cp_rsp.opcode = p_param->value[0];
                // Response
                p_ind->value.ops_cp_rsp.response = p_param->value[1];
                //
                p_data = &(p_param->value[2]);

                // operand based on op code
                switch (cp_opcode)
                {
                case  1://     Communication interval in minutes
                case  3://     Communication interval in minutes
                    p_operand->intvl = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case  5://     Calibration Data Record Number
                    p_operand->cal_data_record_num = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case  7://     Patient High bG value in mg/dL
                case  9://     Patient High bG value in mg/dL
                    p_operand->pat_high_bg = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 10://     Patient Low bG value in mg/dL
                case 12://     Patient Low bG value in mg/dL
                    p_operand->pat_low_bg = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 13://     Hypo Alert Level value in mg/dL
                case 15://     Hypo Alert Level value in mg/dL
                    p_operand->hypo_alert_level = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 16://     Hyper Alert Level value in mg/dL
                case 18://     Hyper Alert Level value in mg/dL
                    p_operand->hyper_alert_level = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 19://     Rate of Decrease Alert Level value in mg/dL/min
                case 21://     Rate of Decrease Alert Level value in mg/dL/min
                    p_operand->rate_decr_alert_level = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 22://     Rate of Increase Alert Level value in mg/dL/min
                case 24://     Rate of Increase Alert Level value in mg/dL/min
                    p_operand->rate_incr_alert_level = co_read16p(&(p_param->value[2]));
                    p_data += 2;
                    break;
                case 28://     Request Op Code, Response Code Value
                    p_operand->req_rsp_value.req_op_code = p_param->value[2];
                    p_operand->req_rsp_value.rsp_code = p_param->value[3];
                    p_data += 2;
                    break;
                case 4://     Operand value as defined in the Calibration Value Fields.
                case 6://     Calibration Data .  ???
                {
                    struct cgm_calib_operand *p_calibr =
                            &p_ind->value.ops_cp_rsp.operand.calib;

                    *p_calibr = (struct cgm_calib_operand){
                        .cal_gluc_concent = co_read16p(&(p_param->value[2])),
                        .cal_time = co_read16p(&(p_param->value[4])),
                        .cal_type_spl = p_param->value[6],
                        .cal_spl_loc = p_param->value[7],
                        .next_cal_time = co_read16p(&(p_param->value[8])),
                        .cal_record_num = co_read16p(&(p_param->value[10])),
                        .cal_status = p_param->value[12]
                    };
                    p_data += CGMP_CALIBR_SIZE;
                } break;
                default: break; //NA
                }
                // stop timer.
                ke_timer_clear(CGMC_CP_REQ_TIMEOUT, dest_id);
            }

        }
        if (p_ind != NULL)
        {
            // confirm that indication has been correctly received
            struct gattc_event_cfm *p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);

            // send indication to the application
            ke_msg_send(p_ind);

            p_cfm->handle = p_param->handle;
            ke_msg_send(p_cfm);
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}


/**
 ****************************************************************************************
 * @brief Control Point request not executed by peer device or is freezed.
 * It can be a connection problem, disconnect the link.
 *
 * @param[in] msgid     Id of the message received.
 * @param[in] p_param   Pointer to the parameters of the message.
 * @param[in] dest_id   ID of the receiving task instance (TASK_GATTC).
 * @param[in] src_id    ID of the sending task instance.
 *
 * @return If the message was consumed or not.
 *
 ****************************************************************************************
 */
__STATIC int cgmc_cp_req_timeout_handler(ke_msg_id_t const msgid, void const *p_param,
                                         ke_task_id_t const dest_id, ke_task_id_t const src_id)
{
    if(ke_state_get(dest_id) != CGMC_BUSY)
    {
        uint8_t op_pending;
        uint8_t operation;
        uint8_t conidx = KE_IDX_GET(dest_id);
        struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

        ASSERT_INFO(p_cgmc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_cgmc_env->env[conidx] != NULL, dest_id, src_id);

        // inform that racp execution is into a timeout state
        op_pending = p_cgmc_env->env[conidx]->op_pending;
        operation = CGMC_OP_PENDING_NO_OP;

        switch(op_pending)
        {
            case CGMC_OP_PENDING_WRITE_RACP:
            {
                operation = CGMC_OP_CODE_WRITE_CTRL_PT;
            } break;

            case CGMC_OP_PENDING_WRITE_SPECIFIC_OPS_CTRL_PT:
            {
                operation = CGMC_OP_CODE_WRITE_OPS_CTRL_PT;
            } break;

            default:
                break;
        }
        // Send the complete event status
        cgmc_send_cmp_evt(p_cgmc_env, conidx, operation, PRF_ERR_PROC_TIMEOUT);

        ke_state_set(dest_id, CGMC_IDLE);
    }

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(cgmc)
{
    {CGMC_ENABLE_REQ,                   (ke_msg_func_t)cgmc_enable_req_handler},

    {GATTC_SDP_SVC_IND,                 (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                     (ke_msg_func_t)gattc_cmp_evt_handler},

    {GATTC_READ_IND,                    (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_EVENT_IND,                   (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,               (ke_msg_func_t)gattc_event_ind_handler},

    {CGMC_READ_CMD,                     (ke_msg_func_t)cgmc_read_cmd_handler},
    {CGMC_READ_CCC_CMD,                 (ke_msg_func_t)cgmc_read_ccc_cmd_handler},

    {CGMC_WRITE_SESS_START_TIME_CMD,    (ke_msg_func_t)cgmc_write_sess_start_time_cmd_handler},

    {CGMC_WRITE_RACP_CMD,               (ke_msg_func_t)cgmc_write_racp_cmd_handler},

    {CGMC_WRITE_OPS_CTRL_PT_CMD,        (ke_msg_func_t)cgmc_write_ops_ctrl_pt_cmd_handler},

    {CGMC_CFG_CCC_CMD,                  (ke_msg_func_t)cgmc_cfg_ccc_cmd_handler},
    {CGMC_CP_REQ_TIMEOUT,               (ke_msg_func_t)cgmc_cp_req_timeout_handler},
};

/*
 * GLOBAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
void cgmc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct cgmc_env_tag *p_cgmc_env = PRF_ENV_GET(CGMC, cgmc);

    p_task_desc->msg_handler_tab = cgmc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(cgmc_msg_handler_tab);
    p_task_desc->state           = p_cgmc_env->state;
    p_task_desc->idx_max         = CGMC_IDX_MAX;
}


#endif //(BLE_CGM_CLIENT)

/// @} CGMCTASK
