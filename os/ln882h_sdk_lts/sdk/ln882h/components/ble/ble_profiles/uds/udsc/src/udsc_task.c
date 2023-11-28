/**
 ****************************************************************************************
 *
 * @file udsc_task.c
 *
 * @brief User Data Service Collector Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 * $ Rev $
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup UDSCTASK
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwip_config.h"

#if (BLE_UDS_CLIENT)
#include "uds_common.h"
#include "udsc_task.h"
#include "udsc.h"

#include "gap.h"
#include "attm.h"
#include "gattc_task.h"
#include "ke_mem.h"
#include "co_utils.h"


/*
 * STRUCTURES
 ****************************************************************************************
 */

/// State machine used to retrieve User Data Service Service characteristics information
const struct prf_char_def udsc_uds_char[UDSC_CHAR_UDS_MAX] =
{
    [UDSC_CHAR_UDS_DB_CHG_INC] = {ATT_CHAR_DATABASE_CHANGE_INCREMENT,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_NTF) },

    [UDSC_CHAR_UDS_USER_INDEX] = {ATT_CHAR_USER_INDEX,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_RD) },

    [UDSC_CHAR_UDS_USER_CTRL_PT]   = {ATT_CHAR_USER_CONTROL_POINT,
                                         ATT_MANDATORY,
                                        (ATT_CHAR_PROP_WR | ATT_CHAR_PROP_IND) },

    /// Strings: utf8s
    [UDSC_CHAR_UDS_FIRST_NAME]   = {ATT_CHAR_FIRST_NAME,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP) },

    [UDSC_CHAR_UDS_LAST_NAME]   = {ATT_CHAR_LAST_NAME,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP) },

    [UDSC_CHAR_UDS_EMAIL_ADDRESS]  = {ATT_CHAR_EMAIL_ADDRESS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP) },

    [UDSC_CHAR_UDS_LANGUAGE]   = {ATT_CHAR_LANGUAGE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR | ATT_CHAR_PROP_EXT_PROP) },

    /// Date
    [UDSC_CHAR_UDS_DATE_OF_BIRTH]   = {ATT_CHAR_DATE_OF_BIRTH,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_DATE_OF_THRESHOLD_ASSESSMENT]   = {ATT_CHAR_DATE_OF_THRESHOLD_ASSESSMENT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    /// uint16
    [UDSC_CHAR_UDS_WEIGHT]   = {ATT_CHAR_WEIGHT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_HEIGHT]   = {ATT_CHAR_HEIGHT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_WAIST_CIRCUMFERENCE]   = {ATT_CHAR_WAIST_CIRCUMFERENCE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_HIP_CIRCUMFERENCE]   = {ATT_CHAR_HIP_CIRCUMFERENCE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    /// uint8
    [UDSC_CHAR_UDS_AGE]   = {ATT_CHAR_AGE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_GENDER]   = {ATT_CHAR_GENDER,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_VO2_MAX]   = {ATT_CHAR_VO2_MAX,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_MAX_HEART_RATE]   = {ATT_CHAR_MAX_HEART_RATE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_RESTING_HEART_RATE]   = {ATT_CHAR_RESTING_HEART_RATE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_MAXIMUM_RECOMMENDED_HEART_RATE]   = {ATT_CHAR_MAXIMUM_RECOMMENDED_HEART_RATE,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_AEROBIC_THRESHOLD]   = {ATT_CHAR_AEROBIC_THRESHHOLD,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_ANAEROBIC_THRESHOLD]   = {ATT_CHAR_ANAEROBIC_THRESHHOLD,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS] =
                                         {ATT_CHAR_SPORT_TYPE_FOR_AEROBIC_AND_ANAEROBIC_THRESHOLDS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_LOWER_LIMIT]   = {ATT_CHAR_FAT_BURN_HEART_RATE_LOWER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_FAT_BURN_HEART_RATE_UPPER_LIMIT]   = {ATT_CHAR_FAT_BURN_HEART_RATE_UPPER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_AEROBIC_HEART_RATE_LOWER_LIMIT]   = {ATT_CHAR_AEROBIC_HEART_RATE_LOWER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_AEROBIC_HEART_RATE_UPPER_LIMIT]   = {ATT_CHAR_AEROBIC_HEART_RATE_UPPER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_ANAEROBIC_HEART_RATE_LOWER_LIMIT]   = {ATT_CHAR_ANAEROBIC_HEART_RATE_LOWER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_ANAEROBIC_HEART_RATE_UPPER_LIMIT]   = {ATT_CHAR_ANAEROBIC_HEART_RATE_UPPER_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_TWO_ZONE_HEART_RATE_LIMITS]   = {ATT_CHAR_TWO_ZONE_HEART_RATE_LIMIT,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    /// set
    [UDSC_CHAR_UDS_FIVE_ZONE_HEART_RATE_LIMITS]   = {ATT_CHAR_FIVE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS]   = {ATT_CHAR_THREE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    /// user defined
    [UDSC_CHAR_UDS_USER_DEFINED_1]   = {ATT_CHAR_FIVE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_USER_DEFINED_2]   = {ATT_CHAR_THREE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },
    [UDSC_CHAR_UDS_USER_DEFINED_3]   = {ATT_CHAR_FIVE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },

    [UDSC_CHAR_UDS_USER_DEFINED_4]   = {ATT_CHAR_THREE_ZONE_HEART_RATE_LIMITS,
                                         ATT_OPTIONAL,
                                        (ATT_CHAR_PROP_RD | ATT_CHAR_PROP_WR) },
};

/// State machine used to retrieve User Data Service Service characteristic description information
const struct prf_char_desc_def udsc_uds_char_desc[UDSC_DESC_UDS_MAX] =
{
    /// Database Change Increment Client config
    [UDSC_DESC_UDS_DB_CHG_INC_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         UDSC_CHAR_UDS_DB_CHG_INC},

    /// User Control Point Client config
    [UDSC_DESC_UDS_USER_CTRL_PT_CCC] = {ATT_DESC_CLIENT_CHAR_CFG,
                                         ATT_MANDATORY,
                                         UDSC_CHAR_UDS_USER_CTRL_PT},

    /// String Extended Properties
    [UDSC_DESC_UDS_FIRST_NAME_EXT] = {ATT_DESC_CHAR_EXT_PROPERTIES,
                                         ATT_OPTIONAL,
                                         UDSC_CHAR_UDS_FIRST_NAME},

    /// String Extended Properties
    [UDSC_DESC_UDS_LAST_NAME_EXT] = {ATT_DESC_CHAR_EXT_PROPERTIES,
                                         ATT_OPTIONAL,
                                         UDSC_CHAR_UDS_LAST_NAME},

    /// String Extended Properties
    [UDSC_DESC_UDS_EMAIL_ADDRESS_EXT] = {ATT_DESC_CHAR_EXT_PROPERTIES,
                                         ATT_OPTIONAL,
                                         UDSC_CHAR_UDS_EMAIL_ADDRESS},

    /// String Extended Properties
    [UDSC_DESC_UDS_LANGUAGE_EXT] = {ATT_DESC_CHAR_EXT_PROPERTIES,
                                         ATT_OPTIONAL,
                                         UDSC_CHAR_UDS_LANGUAGE},
};


/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_SDP_SVC_IND_HANDLER message.
 * The handler stores the found service details for service discovery.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
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
    do
    {
        // User Data Service Collector Role Task Environment
        struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_udsc_env->p_env[conidx] != NULL, dest_id, src_id);

        if (ke_state_get(dest_id) != UDSC_DISCOVERING_SVC)
        {
            break;
        }

        if (attm_uuid16_comp((unsigned char *)p_ind->uuid, p_ind->uuid_len, ATT_SVC_USER_DATA))
        {
            // Retrieve WS characteristics and descriptors
            prf_extract_svc_info(p_ind, UDSC_CHAR_UDS_MAX, &udsc_uds_char[0],  &p_udsc_env->p_env[conidx]->uds.chars[0],
                    UDSC_DESC_UDS_MAX, &udsc_uds_char_desc[0], &p_udsc_env->p_env[conidx]->uds.descs[0]);

            //Even if we get multiple responses we only store 1 range
            p_udsc_env->p_env[conidx]->uds.svc.shdl = p_ind->start_hdl;
            p_udsc_env->p_env[conidx]->uds.svc.ehdl = p_ind->end_hdl;

            p_udsc_env->p_env[conidx]->nb_svc++;
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}
/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_ENABLE_REQ message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

__STATIC int udsc_enable_req_handler(ke_msg_id_t const msgid,
        struct udsc_enable_req *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    uint8_t state = ke_state_get(dest_id);
    // User Data Service Collector Role Task Environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);

    ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
    if ((state == UDSC_IDLE) && (p_udsc_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_udsc_env->p_env[conidx] = (struct udsc_cnx_env *) ke_malloc(sizeof(struct udsc_cnx_env), KE_MEM_ATT_DB);
        memset(p_udsc_env->p_env[conidx], 0, sizeof(struct udsc_cnx_env));

        // Start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // Start discovery with User Data Service
            prf_disc_svc_send(&(p_udsc_env->prf_env), conidx, ATT_SVC_USER_DATA);

            p_udsc_env->p_env[conidx]->last_uuid_req = ATT_SVC_USER_DATA;
            p_udsc_env->p_env[conidx]->last_svc_req  = ATT_SVC_USER_DATA;

            // Go to DISCOVERING SERVICE state
            ke_state_set(dest_id, UDSC_DISCOVERING_SVC);
        }
        // normal connection, get saved att details
        else
        {
            p_udsc_env->p_env[conidx]->uds = p_param->uds;

            //send APP confirmation that can start normal connection to TH
            udsc_enable_rsp_send(p_udsc_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != UDSC_FREE)
    {
        // The message will be forwarded towards the good task instance
        status = PRF_ERR_REQ_DISALLOWED;
    }

    if (status != GAP_ERR_NO_ERROR)
    {
        // The request is disallowed (profile already enabled for this connection, or not enough memory, ...)
        udsc_enable_rsp_send(p_udsc_env, conidx, status);
    }

    return (KE_MSG_CONSUMED);
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_RD_CHAR_CMD  message from the application.
 * @brief To read the Feature Characteristic in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_rd_char_cmd_handler(ke_msg_id_t const msgid,
        struct udsc_rd_char_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == UDSC_IDLE)
    {
        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;

            if (p_param->char_idx < UDSC_CHAR_UDS_MAX)
            {
                search_hdl = p_udsc_env->p_env[conidx]->uds.chars[p_param->char_idx].val_hdl;
            }
            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_READ_CHAR_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = p_param->char_idx;
                // Send read request
                prf_read_char_send(&(p_udsc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == UDSC_FREE)
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
        udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_READ_CHAR_OP_CODE, status);
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_RD_CHAR_CCC_CMD message from the application.
 * @brief To read the CCC value of the Measurement characteristic in the peer server.
 *
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_rd_char_ccc_cmd_handler(ke_msg_id_t const msgid,
        struct udsc_rd_char_ccc_cmd *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t status = PRF_ERR_REQ_DISALLOWED;
    // Message status
    int msg_status = KE_MSG_CONSUMED;
    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);

    if (state == UDSC_IDLE)
    {
        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
            // Attribute Handle
            uint16_t search_hdl = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;

            if (p_param->char_idx == UDSC_CHAR_UDS_DB_CHG_INC)
            {
                search_hdl = p_udsc_env->p_env[conidx]->uds.descs[UDSC_DESC_UDS_DB_CHG_INC_CCC].desc_hdl;
            }
            else if (p_param->char_idx == UDSC_CHAR_UDS_USER_CTRL_PT)
            {
                search_hdl = p_udsc_env->p_env[conidx]->uds.descs[UDSC_DESC_UDS_USER_CTRL_PT_CCC].desc_hdl;
            }

            // Check if handle is viable
            if ((search_hdl != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_READ_CCC_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = p_param->char_idx;
                // Send read request
                prf_read_char_send(&(p_udsc_env->prf_env), conidx, p_svc->shdl, p_svc->ehdl, search_hdl);
                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status =  PRF_ERR_INEXISTENT_HDL;
            }
        }
    }
    else if (state == UDSC_FREE)
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
        udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_READ_CHAR_OP_CODE, status);
    }

    return msg_status;
}


/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_WR_CHAR_CCC_CMD message.
 * Allows the application to write new CCC values to a Characteristic in the peer server
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_wr_char_ccc_cmd_handler(ke_msg_id_t const msgid,
                                   struct udsc_wr_char_ccc_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Get connection index
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);

    if (state == UDSC_IDLE)
    {
        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;

            if (p_param->char_idx == UDSC_CHAR_UDS_DB_CHG_INC)
            {
                handle = p_udsc_env->p_env[conidx]->uds.descs[UDSC_DESC_UDS_DB_CHG_INC_CCC].desc_hdl;
            }
            else if (p_param->char_idx == UDSC_CHAR_UDS_USER_CTRL_PT)
            {
                handle = p_udsc_env->p_env[conidx]->uds.descs[UDSC_DESC_UDS_USER_CTRL_PT_CCC].desc_hdl;
            }

            // Check if handle is viable
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_WRITE_CCC_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = p_param->char_idx;

                // Send the write request
                prf_gatt_write_ntf_ind(&(p_udsc_env->prf_env), conidx, handle, p_param->ccc);

                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_WRITE_CCC_OP_CODE, status);
        }
    }
    else if (state == UDSC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_WR_CHAR_UTF8_CMD message.
 * Allows the application to write new CCC values to a Characteristic in the peer server
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_wr_char_utf8_cmd_handler(ke_msg_id_t const msgid,
                                   struct udsc_wr_char_utf8_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
    // Get connection index

    if (state == UDSC_IDLE)
    {
        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;
            // Length
            uint8_t length;
            uint8_t *p_data;

            length = p_param->utf_name.length;
            p_data = &p_param->utf_name.name[0];

            #ifndef USE_EXTRA_CHARS
            if (p_param->char_idx <= UDSC_CHAR_UDS_LANGUAGE)
            #endif
            {
                handle = p_udsc_env->p_env[conidx]->uds.chars[p_param->char_idx].val_hdl;
            }

            // Check if handle is viable
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_WRITE_CHAR_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = p_param->char_idx;

                // Send the write request
                prf_gatt_write(&(p_udsc_env->prf_env), conidx, handle, p_data, length, GATTC_WRITE);

                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_WRITE_CHAR_OP_CODE, status);
        }
    }
    else if (state == UDSC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_WR_CHAR_CMD message.
 * Allows the application to write new CCC values to a Characteristic in the peer server
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_wr_char_cmd_handler(ke_msg_id_t const msgid,
                                   struct udsc_wr_char_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    if (state == UDSC_IDLE)
    {
        // Get the address of the environment
        struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
        // Get connection index
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);

        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;
            // Length
            uint8_t length = 0;
            uint8_t data[sizeof(union char_val)];
            uint8_t *p_data = &data[0];
            uint8_t char_idx = p_param->char_idx;

            if (((char_idx >= UDSC_CHAR_UDS_DATE_OF_BIRTH) && (char_idx <= UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS)) ||
                (char_idx == UDSC_CHAR_UDS_DB_CHG_INC))
            {
                handle = p_udsc_env->p_env[conidx]->uds.chars[char_idx].val_hdl;
            }

            // Check if handle is viable
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_WRITE_CHAR_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = char_idx;

                if (char_idx == UDSC_CHAR_UDS_DB_CHG_INC)
                {
                    // DB CHG Inc
                    co_write32p(&data[0], p_param->value.db_chg_inc);
                    length = sizeof(uint32_t);
                }
                else if (char_idx == UDSC_CHAR_UDS_FIVE_ZONE_HEART_RATE_LIMITS)
                {
                    // Set
                    data[0] = p_param->value.set[0];
                    data[1] = p_param->value.set[1];
                    data[2] = p_param->value.set[2];
                    data[3] = p_param->value.set[3];
                    length = 4;
                }
                else if (char_idx == UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS)
                {
                    // Set
                    data[0] = p_param->value.set[0];
                    data[1] = p_param->value.set[1];
                    length = 2;
                }
                else if ((char_idx >= UDSC_CHAR_UDS_DATE_OF_BIRTH) &&
                        (char_idx <= UDSC_CHAR_UDS_DATE_OF_THRESHOLD_ASSESSMENT))
                {
                    // Date
                    co_write16p(&data[0], p_param->value.date.year);
                    data[2] = p_param->value.date.month;
                    data[3] = p_param->value.date.day;
                    length = 4;
                }
                else if ((char_idx >= UDSC_CHAR_UDS_WEIGHT) && (char_idx <= UDSC_CHAR_UDS_HIP_CIRCUMFERENCE))
                {
                    // uint16
                    co_write16p(&data[0], p_param->value.uint16);
                    length = 2;
                }
                else if (((char_idx >= UDSC_CHAR_UDS_AGE) && (char_idx <= UDSC_CHAR_UDS_TWO_ZONE_HEART_RATE_LIMITS))
                        || (char_idx == UDSC_CHAR_UDS_USER_INDEX))
                {
                    // uint8
                    data[0] = p_param->value.uint8;
                    length = 1;
                }
                else
                {
                    // other???
                    co_write32p(&data[0], p_param->value.db_chg_inc);
                    length = 4;
                }

                // Send the write request
                prf_gatt_write(&(p_udsc_env->prf_env), conidx, handle, p_data, length, GATTC_WRITE);

                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_WRITE_CHAR_OP_CODE, status);
        }
    }
    else if (state == UDSC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref UDSC_WR_USER_CTRL_PT_CMD message.
 * Allows the application to write new CCC values to a Characteristic in the peer server
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int udsc_wr_user_ctrl_pt_cmd_handler(ke_msg_id_t const msgid,
                                   struct udsc_wr_user_ctrl_pt_cmd *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;
    // Message status
    int msg_status = KE_MSG_CONSUMED;

    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
    // Get connection index

    if (state == UDSC_IDLE)
    {
        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        // environment variable not ready
        if (p_udsc_env->p_env[conidx] == NULL)
        {
            status = PRF_APP_ERROR;
        }
        else
        {
             // Attribute Handle
            uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
            // Service
            struct prf_svc *p_svc = &p_udsc_env->p_env[conidx]->uds.svc;
            // Length
            uint8_t length;
            uint8_t data[UDS_USER_CTRL_PT_MAX_LEN];
            uint8_t *p_data = &data[0];

            handle = p_udsc_env->p_env[conidx]->uds.chars[UDSC_CHAR_UDS_USER_CTRL_PT].val_hdl;

            // Check if handle is viable
            if ((handle != ATT_INVALID_SEARCH_HANDLE) && (p_svc != NULL))
            {
                // Force the operation value
                p_udsc_env->p_env[conidx]->op_pending = UDSC_WRITE_CHAR_OP_CODE;
                // keep the characteristic in operation
                p_udsc_env->p_env[conidx]->last_char_code = UDSC_CHAR_UDS_USER_CTRL_PT;

                data[0] = p_param->op_code;
                if (p_param->op_code == UDS_OP_CODE_REGISTER_NEW_USER)
                {
                    length = 3; // op-code and consent code
                    co_write16p(&data[1], p_param->consent);
                }
                else if (p_param->op_code == UDS_OP_CODE_CONSENT)
                {
                    length = 4; // op-code, user_id and consent code
                    data[1] = p_param->user_id;
                    co_write16p(&data[2], p_param->consent);
                }
                else if (p_param->op_code == UDS_OP_CODE_DELETE_USER_DATA)
                {
                    length = 1; // op-code
                }
                else
                {
                    // reserved commands - for the future extension
                    if (p_param->length >= UDS_USER_CTRL_PT_MAX_LEN - 1) //sizeof(op-code)
                    {
                        p_param->length = 0;
                    }

                    length = 1 + p_param->length;

                    if (p_param->length)
                    {
                        memcpy(&data[1], &p_param->parameter[0], p_param->length);
                    }
                }

                // Send the write request
                prf_gatt_write(&(p_udsc_env->prf_env), conidx, handle, p_data, length, GATTC_WRITE);

                // Go to the Busy state
                ke_state_set(dest_id, UDSC_BUSY);

                status = GAP_ERR_NO_ERROR;
            }
            else
            {
                status = PRF_ERR_INEXISTENT_HDL;
            }
        }

        if (status != GAP_ERR_NO_ERROR)
        {
            udsc_send_cmp_evt(p_udsc_env, conidx, UDSC_WRITE_CHAR_OP_CODE, status);
        }
    }
    else if (state == UDSC_FREE)
    {
        status = GAP_ERR_DISCONNECTED;
    }
    else
    {
        // Another procedure is pending, keep the command for later
        msg_status = KE_MSG_SAVED;
        status = GAP_ERR_NO_ERROR;
    }

    return msg_status;
}

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref GATTC_CMP_EVT message.
 * @param[in] msgid Id of the message received.
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance.
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                                 struct gattc_cmp_evt const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
    do
    {
        // Get the address of the environment
        struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
        // Status
        uint8_t status;
        uint8_t conidx = KE_IDX_GET(dest_id);
        uint8_t state = ke_state_get(dest_id);

        if (p_udsc_env == NULL)
        {
            break;
        }

        if (state == UDSC_DISCOVERING_SVC)
        {
            if ((p_param->status == GAP_ERR_NO_ERROR) || (p_param->status == ATT_ERR_ATTRIBUTE_NOT_FOUND))
            {
                // Check service (if found)
                if(p_udsc_env->p_env[conidx]->nb_svc)
                {
                    // Check service (mandatory)
                    status = prf_check_svc_char_validity(UDSC_CHAR_UDS_MAX,
                            p_udsc_env->p_env[conidx]->uds.chars,
                            udsc_uds_char);

                    // Check Descriptors (mandatory)
                    if(status == GAP_ERR_NO_ERROR)
                    {
                        status = prf_check_svc_char_desc_validity(UDSC_DESC_UDS_MAX,
                                p_udsc_env->p_env[conidx]->uds.descs,
                                udsc_uds_char_desc,
                                p_udsc_env->p_env[conidx]->uds.chars);
                    }
                }
            }

            // Raise an ESP_ENABLE_REQ complete event.
            ke_state_set(dest_id, UDSC_IDLE);

            udsc_enable_rsp_send(p_udsc_env, conidx, p_param->status);
        }
        else if (state == UDSC_BUSY)
        {
            uint8_t op_pending = p_udsc_env->p_env[conidx]->op_pending;

            status = p_param->status;
            switch (p_param->operation)
            {
                case GATTC_READ:
                {
                    // Send the complete event status
                    udsc_send_cmp_evt(p_udsc_env, conidx, op_pending, status);

                    p_udsc_env->p_env[conidx]->op_pending = 0;
                    ke_state_set(dest_id, UDSC_IDLE);
                } break;

                case GATTC_WRITE:
                case GATTC_WRITE_NO_RESPONSE:
                {
                    // Send the complete event status
                    udsc_send_cmp_evt(p_udsc_env, conidx, op_pending, p_param->status);
                    ke_state_set(dest_id, UDSC_IDLE);

                } break;

                case GATTC_REGISTER:
                {
                    if (status != GAP_ERR_NO_ERROR)
                    {
                        // Send the complete event error status
                        udsc_send_cmp_evt(p_udsc_env, conidx, GATTC_REGISTER, p_param->status);
                    }
                    // Go to connected state
                    ke_state_set(dest_id, UDSC_IDLE);
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
        }
    } while (0);

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
    do
    {
        // Get the address of the environment
        struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);
        uint8_t conidx = KE_IDX_GET(dest_id);

        ASSERT_INFO(p_udsc_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_udsc_env->p_env[conidx] != NULL, dest_id, src_id);

        // Find the op_pending -  and check it is a valid read code.
        uint8_t op_pending = p_udsc_env->p_env[conidx]->op_pending;;
        // retrieve the characteristic in operation
        uint8_t char_idx = p_udsc_env->p_env[conidx]->last_char_code;

        if (ke_state_get(dest_id) != UDSC_BUSY)
        {
            break;
        }

        if ((op_pending < UDSC_READ_CHAR_OP_CODE) || (op_pending > UDSC_READ_CCC_OP_CODE))
        {
            break;
        }

        switch (op_pending)
        {
            case UDSC_READ_CHAR_OP_CODE:
            {
                if (char_idx <= UDSC_CHAR_UDS_LANGUAGE)
                {
                    //utf8
                    uint16_t length = p_param->length;
                    struct udsc_rd_char_utf8_ind *p_ind = KE_MSG_ALLOC_DYN(UDSC_RD_CHAR_UTF8_IND,
                                                prf_dst_task_get(&(p_udsc_env->prf_env), conidx),
                                                dest_id,
                                                udsc_rd_char_utf8_ind, length);

                    p_ind->char_idx = char_idx;
                    p_ind->utf_name.length = p_param->length;

                    for (int i = 0; i < p_ind->utf_name.length; i++)
                    {
                        p_ind->utf_name.name[i] = p_param->value[i];
                    }

                    ke_msg_send(p_ind);
                    // Keep the Busy state to be treated in gattc_cmp_evt_handler
                }
                else
                {
                    struct udsc_rd_char_ind *p_ind = KE_MSG_ALLOC(UDSC_RD_CHAR_IND,
                                                prf_dst_task_get(&(p_udsc_env->prf_env), conidx),
                                                dest_id,
                                                udsc_rd_char_ind);

                    p_ind->char_idx = char_idx;

                    if (char_idx == UDSC_CHAR_UDS_DB_CHG_INC)
                    {
                        // DB CHG Inc
                        p_ind->value.db_chg_inc = co_read32p(&p_param->value[0]);
                    }
                    else if (char_idx == UDSC_CHAR_UDS_FIVE_ZONE_HEART_RATE_LIMITS)
                    {
                        // Set
                        p_ind->value.set[0] = p_param->value[0];
                        p_ind->value.set[1] = p_param->value[1];
                        p_ind->value.set[2] = p_param->value[2];
                        p_ind->value.set[3] = p_param->value[3];
                    }
                    else if (char_idx == UDSC_CHAR_UDS_THREE_ZONE_HEART_RATE_LIMITS)
                    {
                        // Set
                        p_ind->value.set[0] = p_param->value[0];
                        p_ind->value.set[1] = p_param->value[1];
                    }
                    else if ((char_idx >= UDSC_CHAR_UDS_DATE_OF_BIRTH) &&
                            (char_idx <= UDSC_CHAR_UDS_DATE_OF_THRESHOLD_ASSESSMENT))
                    {
                        // Date
                        p_ind->value.date.year  = co_read16p(&p_param->value[0]);
                        p_ind->value.date.month = p_param->value[2];
                        p_ind->value.date.day   = p_param->value[3];
                    }
                    else if ((char_idx >= UDSC_CHAR_UDS_WEIGHT) && (char_idx <= UDSC_CHAR_UDS_HIP_CIRCUMFERENCE))
                    {
                        // uint16
                        p_ind->value.uint16 = co_read16p(&p_param->value[0]);
                    }
                    else if (((char_idx >= UDSC_CHAR_UDS_AGE) &&
                            (char_idx <= UDSC_CHAR_UDS_TWO_ZONE_HEART_RATE_LIMITS)) ||
                            (char_idx == UDSC_CHAR_UDS_USER_INDEX))
                    {
                        // uint8
                        p_ind->value.uint8 = p_param->value[0];
                    }
                    else
                    {
                        // other???
                        p_ind->value.db_chg_inc = co_read32p(&p_param->value[0]);
                    }

                    ke_msg_send(p_ind);
                    // Keep the Busy state to be treated in gattc_cmp_evt_handler
                }
            } break;

            case UDSC_READ_CCC_OP_CODE :
            {
                struct udsc_rd_char_ccc_ind *p_ind = KE_MSG_ALLOC(UDSC_RD_CHAR_CCC_IND,
                                            prf_dst_task_get(&(p_udsc_env->prf_env), conidx),
                                            dest_id,
                                            udsc_rd_char_ccc_ind);

                p_ind->char_idx = char_idx;
                p_ind->ccc = co_read16p(&p_param->value[0]);

                ke_msg_send(p_ind);
                // Keep the Busy state to be treated in gattc_cmp_evt_handler
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
    do
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);

        if (p_udsc_env == NULL)
        {
            break;
        }

        switch (p_param->type)
        {
            case (GATTC_NOTIFY):
            {
                if (p_param->handle == p_udsc_env->p_env[conidx]->uds.chars[UDSC_CHAR_UDS_DB_CHG_INC].val_hdl)
                {
                    struct udsc_db_chg_inc_ind *p_ind = KE_MSG_ALLOC(UDSC_DB_CHG_INC_IND,
                                               prf_dst_task_get(&p_udsc_env->prf_env, conidx),
                                               prf_src_task_get(&p_udsc_env->prf_env, conidx),
                                               udsc_db_chg_inc_ind);

                    p_ind->value = co_read32p(&p_param->value[0]);
                    ke_msg_send(p_ind);
                }
                // else ignore the Notify as Att-Handle is invalid
            } break;

            case (GATTC_INDICATE):
            {
                struct gattc_event_cfm *p_cfm;

                if (p_param->handle == p_udsc_env->p_env[conidx]->uds.chars[UDSC_CHAR_UDS_USER_CTRL_PT].val_hdl)
                {
                    struct udsc_user_ctrl_pt_ind *p_ind;
                    uint16_t length = 0;

                    if (p_param->length > 3)
                    {
                        length = p_param->length - 3;
                    }

                    if (length >= UDS_USER_CTRL_PT_MAX_LEN - 3) //sizeof(op-code+length)
                    {
                        length = UDS_USER_CTRL_PT_MAX_LEN - 3;
                    }

                    p_ind = KE_MSG_ALLOC_DYN(UDSC_USER_CTRL_PT_IND,
                                               prf_dst_task_get(&p_udsc_env->prf_env, conidx),
                                               prf_src_task_get(&p_udsc_env->prf_env, conidx),
                                               udsc_user_ctrl_pt_ind, length);

                    // decode message
                    p_ind->resp_code = p_param->value[0]; // response op-code =0x20
                    p_ind->req_op_code = p_param->value[1]; // requested op-code
                    p_ind->resp_value = p_param->value[2];
                    p_ind->user_id = UDS_USER_ID_UNKNOWN_USER;

                    if ((p_ind->req_op_code == UDS_OP_CODE_REGISTER_NEW_USER) &&
                            (p_ind->resp_value == UDS_OP_RESPONSE_SUCCESS))
                    {
                        p_ind->user_id = p_param->value[3];

                        if (length)
                        {
                            length--;
                        }
                    }

                    if (length)
                    {
                        memcpy(&p_ind->parameter[0], &p_param->value[3], length);
                    }

                    p_ind->length = length;

                    ke_msg_send(p_ind);
                }

                // confirm that indication has been correctly received
                p_cfm = KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
                p_cfm->handle = p_param->handle;
                ke_msg_send(p_cfm);
            } break;

            default:
            {
                ASSERT_ERR(0);
            } break;
        }

    } while (0);

    return (KE_MSG_CONSUMED);
}

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Specifies the default message handlers
KE_MSG_HANDLER_TAB(udsc)
{
    {UDSC_ENABLE_REQ,                   (ke_msg_func_t)udsc_enable_req_handler},

    {GATTC_SDP_SVC_IND,                 (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_CMP_EVT,                     (ke_msg_func_t)gattc_cmp_evt_handler},

    {GATTC_READ_IND,                    (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_EVENT_IND,                   (ke_msg_func_t)gattc_event_ind_handler},
    {GATTC_EVENT_REQ_IND,               (ke_msg_func_t)gattc_event_ind_handler},

    {UDSC_RD_CHAR_CMD,                  (ke_msg_func_t)udsc_rd_char_cmd_handler},
    {UDSC_RD_CHAR_CCC_CMD,              (ke_msg_func_t)udsc_rd_char_ccc_cmd_handler},
    {UDSC_WR_CHAR_CMD,                  (ke_msg_func_t)udsc_wr_char_cmd_handler},
    {UDSC_WR_CHAR_UTF8_CMD,             (ke_msg_func_t)udsc_wr_char_utf8_cmd_handler},
    {UDSC_WR_USER_CTRL_PT_CMD,          (ke_msg_func_t)udsc_wr_user_ctrl_pt_cmd_handler},
    {UDSC_WR_CHAR_CCC_CMD,              (ke_msg_func_t)udsc_wr_char_ccc_cmd_handler},
};

void udsc_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct udsc_env_tag *p_udsc_env = PRF_ENV_GET(UDSC, udsc);

    p_task_desc->msg_handler_tab = udsc_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(udsc_msg_handler_tab);
    p_task_desc->state           = p_udsc_env->state;
    p_task_desc->idx_max         = UDSC_IDX_MAX;
}

#endif //(BLE_UDS_CLIENT)

/// @} UDSCTASK
