/**
 ****************************************************************************************
 *
 * @file hogprh_task.c
 *
 * @brief Hid Over Gatt Profile Report Host Task implementation.
 *
 * Copyright (C) RivieraWaves 2009-2016
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup HOGPRHTASK
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "rwip_config.h"
#include "rwprf_config.h"

#if (BLE_HID_REPORT_HOST)
#include "co_utils.h"
#include "gap.h"
#include "gattc_task.h"
#include "hogprh.h"
#include "hogprh_task.h"
#include "prf_utils.h"
#include "prf_types.h"

#include "ke_mem.h"

#include "utils/debug/log.h"



const struct prf_char_def hogprh_device_char[HOGPRH_CHAR_MAX] =
{
    [HOGPRH_INCLUDE_CHAR]                   = {ATT_DECL_INCLUDE,           ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_HID_INFO_CHAR]                  = {ATT_CHAR_HID_INFO,          ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_MAP_CHAR]                = {ATT_CHAR_REPORT_MAP,        ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_INPUT_KB_CHAR]           = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_INPUT_CONS_CTRL_CHAR]    = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_INPUT_AUDIO_DATA_CHAR]   = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_INPUT_AUDIO_CFG_CHAR]    = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_OUTPUT_AUDIO_STATE_CHAR] = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_REPORT_DIAG_CHAR]               = {ATT_CHAR_REPORT,            ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_HID_CTRL_POINT_CHAR]            = {ATT_CHAR_HID_CTNL_PT,       ATT_OPTIONAL, ATT_CHAR_PROP_WR},
};


#define     ATT_CHAR_HID_DFU_ID    {0x2D, 0x0A, 0xDE, 0xEC, 0xE3, 0x20, 0x43, 0xA0, 0x12, 0x49, 0x2C, 0x76, 0x01, 0xA0, 0xBF, 0xCF}
#define     ATT_CHAR_HID_DFU_AUTH  {0x2D, 0x0A, 0xDE, 0xEC, 0xE3, 0x20, 0x43, 0xA0, 0x12, 0x49, 0x2C, 0x76, 0x03, 0xA0, 0xBF, 0xCF} 
#define     ATT_CHAR_HID_DFU_DATA  {0x2D, 0x0A, 0xDE, 0xEC, 0xE3, 0x20, 0x43, 0xA0, 0x12, 0x49, 0x2C, 0x76, 0x02, 0xA0, 0xBF, 0xCF} 

#if 0
const struct prf_char_def_128 hogprh_device_dfu_char[HOGPRH_DFU_CHAR_MAX] =
{
    [HOGPRH_DFU_ID_CHAR]                   = {ATT_CHAR_HID_DFU_ID,         ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_DFU_AUTH_CHAR]                 = {ATT_CHAR_HID_DFU_AUTH,       ATT_OPTIONAL, ATT_CHAR_PROP_RD},
    [HOGPRH_DFU_DATA_CHAR]                 = {ATT_CHAR_HID_DFU_DATA,       ATT_OPTIONAL, ATT_CHAR_PROP_RD},
};
#endif

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
__STATIC int gattc_cmp_evt_handler(ke_msg_id_t const msgid,struct gattc_cmp_evt const *p_param,
                                               ke_task_id_t const dest_id,ke_task_id_t const src_id);

/**
 ****************************************************************************************
 * @brief Handles reception of the @ref HOGPRH_ENABLE_REQ message.
 * The handler enables the Battery 'Profile' Server Role.
 * @param[in] msgid Id of the message received (probably unused).
 * @param[in] p_param Pointer to the parameters of the message.
 * @param[in] dest_id ID of the receiving task instance (probably unused).
 * @param[in] src_id ID of the sending task instance.
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
__STATIC int hogprh_enable_req_handler(ke_msg_id_t const msgid,
                                   struct hogprh_enable_req const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

    LOG(LOG_LVL_INFO, "hogprh_enable_req_handler: con_type=%d,state=%d,p_env[conidx]=0x%x\r\n", 
                       p_param->con_type,state,p_hogprh_env->p_env[conidx]);

    ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);
    
    if ((state == HOGPRH_IDLE) && (p_hogprh_env->p_env[conidx] == NULL))
    {
        // allocate environment variable for task instance
        p_hogprh_env->p_env[conidx] = (struct hogprh_cnx_env *) ke_malloc(sizeof(struct hogprh_cnx_env), KE_MEM_ATT_DB);
        memset(p_hogprh_env->p_env[conidx], 0, sizeof(struct hogprh_cnx_env));

        // Config connection, start discovering
        if (p_param->con_type == PRF_CON_DISCOVERY)
        {
            // start discovering DIS on peer
            prf_disc_svc_send(&(p_hogprh_env->prf_env), conidx, ATT_SVC_HID/*ATT_DECL_PRIMARY_SERVICE*//*0xA000*/);

            // Go to DISCOVERING state
            ke_state_set(dest_id, HOGPRH_DISCOVERING);
        }
        // normal connection, get saved att details
        else
        {
            p_hogprh_env->p_env[conidx]->device = p_param->device;

            // send APP confirmation that can start normal connection to hid
            hogprh_enable_rsp_send(p_hogprh_env, conidx, GAP_ERR_NO_ERROR);
        }
    }
    else if (state != HOGPRH_FREE)
    {
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send an error if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        hogprh_enable_rsp_send(p_hogprh_env, conidx, status);
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
__STATIC int hogprh_rd_char_req_handler(ke_msg_id_t const msgid,
                                    struct hogprh_rd_char_cmd const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

    LOG(LOG_LVL_INFO, "hogprh_rd_char_cmd_handler code=0x%x\r\n",p_param->char_code);

    ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);
    
    if ((state == HOGPRH_IDLE) && (p_hogprh_env->p_env[conidx] != NULL))
    {
        uint16_t search_hdl = ATT_INVALID_HDL;

        // retrieve search handle
        if (p_param->char_code < HOGPRH_CHAR_MAX)
        {
            search_hdl = p_hogprh_env->p_env[conidx]->device.chars[p_param->char_code].val_hdl;
        }

        //Check if handle is viable
        if (search_hdl != ATT_INVALID_HDL)
        {
            // perform read request
            p_hogprh_env->p_env[conidx]->last_char_code = p_param->char_code;
            prf_read_char_send(&(p_hogprh_env->prf_env), conidx, p_hogprh_env->p_env[conidx]->device.svc.shdl,
                    p_hogprh_env->p_env[conidx]->device.svc.ehdl, search_hdl);

            // enter in a busy state
            ke_state_set(dest_id, HOGPRH_BUSY);
        }
        else
        {
            // invalid handle requested
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else if (state != HOGPRH_FREE)
    {
        // request cannot be performed
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send error response if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        hogprh_send_cmp_evt(p_hogprh_env, conidx, HOGPRH_RD_CHAR_CMD_OP_CODE, status);
    }

    return (KE_MSG_CONSUMED);
}

//read peer characteristic value by attribute handle.
__STATIC int hogprh_rd_req_handler(ke_msg_id_t const msgid,
                                    struct hogprh_rd_cmd const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    // Status
    uint8_t status = GAP_ERR_NO_ERROR;

    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Device Information Service Client Role Task Environment
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

    LOG(LOG_LVL_INFO, "hogprh_rd_cmd_handler handle=0x%x\r\n",p_param->handle);

    ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);
    
    if ((state == HOGPRH_IDLE) && (p_hogprh_env->p_env[conidx] != NULL))
    {
        uint16_t search_hdl = p_param->handle;

      
        //Check if handle is viable
        if (search_hdl != ATT_INVALID_HDL)
        {
            // perform read request
            //p_hogprh_env->p_env[conidx]->last_char_code = p_param->char_code;
            prf_read_char_send(&(p_hogprh_env->prf_env), conidx, p_hogprh_env->p_env[conidx]->device.svc.shdl,
                    p_hogprh_env->p_env[conidx]->device.svc.ehdl, search_hdl);

            // enter in a busy state
            ke_state_set(dest_id, HOGPRH_BUSY);
        }
        else
        {
            // invalid handle requested
            status = PRF_ERR_INEXISTENT_HDL;
        }
    }
    else if (state != HOGPRH_FREE)
    {
        // request cannot be performed
        status = PRF_ERR_REQ_DISALLOWED;
    }

    // send error response if request fails
    if (status != GAP_ERR_NO_ERROR)
    {
        hogprh_send_cmp_evt(p_hogprh_env, conidx, HOGPRH_RD_CHAR_CMD_OP_CODE, status);
    }

    return (KE_MSG_CONSUMED);
}


__STATIC int hogprh_wr_req_handler(ke_msg_id_t const msgid,
                                    struct hogprh_wr_cmd const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    uint8_t status = GAP_ERR_NO_ERROR;
    uint8_t state = ke_state_get(dest_id);
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Attribute handle
    uint16_t handle = ATT_INVALID_SEARCH_HANDLE;
    uint8_t length =0;
    // Write type
    uint8_t wr_type=0;
    
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

    LOG(LOG_LVL_INFO, "hogprh_wr_req_handler char_idx=%d, value=0x%x\r\n",p_param->char_idx, p_param->value);

    ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);

    if(p_param->char_idx == HOGPRH_REPORT_OUTPUT_AUDIO_STATE_CHAR)
    {
        handle = p_hogprh_env->p_env[conidx]->device.chars[HOGPRH_REPORT_OUTPUT_AUDIO_STATE_CHAR].val_hdl;
        length = sizeof(uint8_t);
        wr_type = GATTC_WRITE;
    }
    // Check if handle is viable
    if (handle != ATT_INVALID_SEARCH_HANDLE)
    {
        // Send the write request
        prf_gatt_write(&(p_hogprh_env->prf_env), conidx, handle, (uint8_t *)&p_param->value, length, wr_type);
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
    LOG(LOG_LVL_INFO, "hogprh_task gattc_sdp_svc_ind_handler start_hdl=%d, end_hdl=%d,uuid_len=%d, uuid=0x%02x%02x\r\n",
                       p_ind->start_hdl, p_ind->end_hdl,p_ind->uuid_len, p_ind->uuid[1],p_ind->uuid[0]);

    if (ke_state_get(dest_id) == HOGPRH_DISCOVERING)
    {
        uint8_t conidx = KE_IDX_GET(dest_id);
        // Get the address of the environment
        struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

        ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_hogprh_env->p_env[conidx] != NULL, dest_id, src_id);

        LOG(LOG_LVL_INFO, "nb_svc=%d\r\n",p_hogprh_env->p_env[conidx]->nb_svc );

        if (p_hogprh_env->p_env[conidx]->nb_svc == 0)
        {
            if(p_ind->uuid_len == ATT_UUID_16_LEN)
            {
                // Retrieve device characteristics
                prf_extract_svc_info(p_ind, HOGPRH_CHAR_MAX, &hogprh_device_char[0], 
                                    &(p_hogprh_env->p_env[conidx]->device.chars[0]), 0, NULL, NULL);
            }
#if 0
            else //DFU uuid is 128 bit
            {
                // Retrieve device characteristics
                prf_extract_svc_info_128(p_ind, HOGPRH_DFU_CHAR_MAX, &hogprh_device_dfu_char[0], 
                                    &(p_hogprh_env->p_env[conidx]->device.chars[0]), 0, NULL, NULL);
            }
#endif
                            //Even if we get multiple responses we only store 1 range
            p_hogprh_env->p_env[conidx]->device.svc.shdl = p_ind->start_hdl;
            p_hogprh_env->p_env[conidx]->device.svc.ehdl = p_ind->end_hdl;
            
        }

        p_hogprh_env->p_env[conidx]->nb_svc++;
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
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);
    uint8_t conidx = KE_IDX_GET(dest_id);

    LOG(LOG_LVL_INFO, "HOGPRH_task gattc_cmp_evt_handler: state=%d,status=%d,operation=0x%x, seq_num=0x%x,nb_svc=%d\r\n",
                        state, p_param->status,p_param->operation, p_param->seq_num,p_hogprh_env->p_env[conidx]->nb_svc);

    if (state == HOGPRH_DISCOVERING)
    {
        uint8_t status = p_param->status;

        if (p_param->status == ATT_ERR_NO_ERROR)
        {
            if (p_hogprh_env->p_env[conidx]->nb_svc ==  1)
            {
                status = prf_check_svc_char_validity(HOGPRH_CHAR_MAX, p_hogprh_env->p_env[conidx]->device.chars,
                                    hogprh_device_char);
            }
            // too much services
            else if (p_hogprh_env->p_env[conidx]->nb_svc > 1)
            {
                status = PRF_ERR_MULTIPLE_SVC;
            }
            // no services found
            else
            {
                status = PRF_ERR_STOP_DISC_CHAR_MISSING;
            }
        }

        hogprh_enable_rsp_send(p_hogprh_env, conidx, status);
    }
    else if (state == HOGPRH_BUSY)
    {
        if (p_param->operation == GATTC_READ)
        {
            hogprh_send_cmp_evt(p_hogprh_env, conidx, HOGPRH_RD_CHAR_CMD_OP_CODE, p_param->status);
        }

        ke_state_set(dest_id, HOGPRH_IDLE);
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
    LOG(LOG_LVL_INFO, "hogprh_task gattc_read_ind_handler handle=0x%x,length=%d, offset=%d\r\n", 
                       p_param->handle, p_param->length, p_param->offset);

    if (ke_state_get(dest_id) == HOGPRH_BUSY)
    {
        // Get the address of the environment
        struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

        ASSERT_INFO(p_hogprh_env != NULL, dest_id, src_id);
        ASSERT_INFO(p_hogprh_env->p_env[KE_IDX_GET(dest_id)] != NULL, dest_id, src_id);

        prf_client_att_info_rsp(&(p_hogprh_env->prf_env), KE_IDX_GET(dest_id), HOGPRH_RD_CHAR_IND,
                GAP_ERR_NO_ERROR, p_param);
    }

    return (KE_MSG_CONSUMED);
}

__STATIC int gattc_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
                                                        struct gattc_disc_char_desc_ind const *p_ind,
                                                        ke_task_id_t const dest_id,
                                                        ke_task_id_t const src_id)
{
    uint8_t i;
    uint8_t conidx = KE_IDX_GET(dest_id);
    uint8_t state = ke_state_get(dest_id);
    struct hogprh_env_tag *p_bcsc_env = PRF_ENV_GET(HOGPRH, hogprh);

    LOG(LOG_LVL_INFO, "HOGPRH_task gattc_read_ind_handler attr_hdl=0x%x,uuid_len=%d, uuid=0x%02x%02x,conidx=%d,state=%d\r\n", 
                           p_ind->attr_hdl, p_ind->uuid_len, p_ind->uuid[1],p_ind->uuid[0],conidx, state);

    return (KE_MSG_CONSUMED);
}


__STATIC int gattc_event_ind_handler(ke_msg_id_t const msgid,
                                         struct gattc_event_ind const *p_param,
                                         ke_task_id_t const dest_id,
                                         ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(dest_id);
    // Get the address of the environment
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);
    // data pointer
    const uint8_t *p_data;
    // length of data block
    uint16_t length;
    //indication Build a PLXC_VALUE_IND message
    struct gattc_event_cfm *p_cfm;

    LOG(LOG_LVL_INFO, "hogprh_task gattc_event_ind_handler handle=0x%x,length=%d, type=%d,hogprh_state=%d\r\n", 
                           p_param->handle, p_param->length, p_param->type, ke_state_get(dest_id));

    if ((p_param->type != GATTC_INDICATE) && (p_param->type != GATTC_NOTIFY))
    {
        return (KE_MSG_CONSUMED);
    }

    //if (ke_state_get(dest_id) != HOGPRH_FREE)
    {
        struct hogprh_notify_ind *p_ind = KE_MSG_ALLOC_DYN(HOGPRH_NTY_IND,
                                            prf_dst_task_get(&(p_hogprh_env->prf_env), conidx), dest_id,
                                            hogprh_notify_ind, p_param->length);
                                            
        memcpy(p_ind->value, p_param->value, p_param->length);
        p_ind->length = p_param->length;

        if (p_param->handle == p_hogprh_env->p_env[conidx]->device.chars[HOGPRH_REPORT_INPUT_KB_CHAR].val_hdl)
        {
            p_ind->type = HOGPRH_REPORT_INPUT_KB_CHAR;
        }
        else if (p_param->handle == p_hogprh_env->p_env[conidx]->device.chars[HOGPRH_REPORT_INPUT_CONS_CTRL_CHAR].val_hdl)
        {
            p_ind->type = HOGPRH_REPORT_INPUT_CONS_CTRL_CHAR;
        }
        else if (p_param->handle == p_hogprh_env->p_env[conidx]->device.chars[HOGPRH_REPORT_INPUT_AUDIO_DATA_CHAR].val_hdl)
        {
            p_ind->type = HOGPRH_REPORT_INPUT_AUDIO_DATA_CHAR;
        }
        else if (p_param->handle == p_hogprh_env->p_env[conidx]->device.chars[HOGPRH_REPORT_INPUT_AUDIO_CFG_CHAR].val_hdl)
        {
            p_ind->type = HOGPRH_REPORT_INPUT_AUDIO_CFG_CHAR;
        }
        
        //Send value to APP
        ke_msg_send(p_ind);
    }

    return (KE_MSG_CONSUMED);
}


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// Default State handlers definition
KE_MSG_HANDLER_TAB(hogprh)
{
    {HOGPRH_ENABLE_REQ,        (ke_msg_func_t)hogprh_enable_req_handler},
    {HOGPRH_RD_CHAR_REQ,       (ke_msg_func_t)hogprh_rd_char_req_handler},
    {HOGPRH_RD_REQ,            (ke_msg_func_t)hogprh_rd_req_handler},
    {HOGPRH_WR_REQ,            (ke_msg_func_t)hogprh_wr_req_handler},
    {GATTC_READ_IND,           (ke_msg_func_t)gattc_read_ind_handler},
    {GATTC_SDP_SVC_IND,        (ke_msg_func_t)gattc_sdp_svc_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND, (ke_msg_func_t)gattc_disc_char_desc_ind_handler},
    {GATTC_CMP_EVT,            (ke_msg_func_t)gattc_cmp_evt_handler},
    {GATTC_EVENT_IND,          (ke_msg_func_t)gattc_event_ind_handler},
};

void hogprh_task_init(struct ke_task_desc *p_task_desc)
{
    // Get the address of the environment
    struct hogprh_env_tag *p_hogprh_env = PRF_ENV_GET(HOGPRH, hogprh);

    p_task_desc->msg_handler_tab = hogprh_msg_handler_tab;
    p_task_desc->msg_cnt         = ARRAY_LEN(hogprh_msg_handler_tab);
    p_task_desc->state           = p_hogprh_env->state;
    p_task_desc->idx_max         = HOGPRH_IDX_MAX;
}

#endif //BLE_HID_REPORT_HOST

/// @} HOGPRHTASK

