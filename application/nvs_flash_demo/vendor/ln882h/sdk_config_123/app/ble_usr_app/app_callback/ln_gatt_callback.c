/**
 ****************************************************************************************
 *
 * @file ln_gatt_callback.c
 *
 * @brief GATT callback funcion source code
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup BLE
 *
 * @brief GATT callback funcion source code
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */



#include "rwip_config.h"             // SW configuration
#include "ln_app_gatt.h"
#include "usr_app.h"

#include <string.h>

//#include "gatt.h"                     // GATT Definition
#include "gattm_task.h"               // GATT Manager Task API
#include "gattc_task.h"               // GATT Controller Task API
#include "ln_app_callback.h"
#include "usr_ble_app.h"
//#include "co_math.h"                 // Common Maths Definition
//#include "co_utils.h"


#if (TRACE_ENABLE)
#include "utils/debug/log.h"
#endif

extern struct app_env_info_tag app_env_info;

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */


int gattm_add_svc_rsp_handler(ke_msg_id_t const msgid,
                              void const *p_param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id)
{
    struct gattm_add_svc_rsp *param = (struct gattm_add_svc_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_add_svc_rsp_handler ,start_handler:0x%x,status:0x%x\r\n",param->start_hdl,param->status);
#endif
    memcpy(&(app_env_info.add_svc_rsp_info),param,sizeof( struct gattm_add_svc_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}



int gattm_svc_get_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_svc_get_permission_rsp *param = (struct gattm_svc_get_permission_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_svc_get_permission_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.svc_get_permission_rsp_info),param,sizeof( struct gattm_svc_get_permission_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}


int gattm_svc_set_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_svc_set_permission_rsp *param = (struct gattm_svc_set_permission_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_svc_set_permission_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.svc_set_permission_rsp_info),param,sizeof( struct gattm_svc_set_permission_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}




int gattm_att_get_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_get_permission_rsp *param = (struct gattm_att_get_permission_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_get_permission_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.att_get_permission_rsp_info),param,sizeof( struct gattm_att_get_permission_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}


int gattm_att_set_permission_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_set_permission_rsp *param = (struct gattm_att_set_permission_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_set_permission_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.att_set_permission_rsp_info),param,sizeof( struct gattm_att_set_permission_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}


int gattm_att_get_value_rsp_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    struct gattm_att_get_value_rsp *param = (struct gattm_att_get_value_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_get_value_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.att_get_value_rsp_info),param,sizeof( struct gattm_att_get_value_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}



int gattm_att_set_value_rsp_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    struct gattm_att_set_value_rsp *param = (struct gattm_att_set_value_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_set_value_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    memcpy(&(app_env_info.att_set_value_rsp_info),param,sizeof( struct gattm_att_set_value_rsp));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}

int gattm_unknown_msg_ind_handler(ke_msg_id_t const msgid,
                                  void const *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{

    struct gattm_unknown_msg_ind *param = (struct gattm_unknown_msg_ind *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_unknown_msg_ind_handler msg_id:0x%x\r\n",param->unknown_msg_id);
#endif
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}


int gattm_destroy_db_rsp_handler(ke_msg_id_t const msgid,
                                 void const *p_param,
                                 ke_task_id_t const dest_id,
                                 ke_task_id_t const src_id)
{
     struct gattm_destroy_db_rsp *param = (struct gattm_destroy_db_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_destroy_db_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}

int gattm_svc_get_list_rsp_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gattm_svc_get_list_rsp *param = (struct gattm_svc_get_list_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_svc_get_list_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}


int gattm_att_get_info_rsp_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gattm_att_get_info_rsp *param = (struct gattm_att_get_info_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_get_info_rsp_handler ,status:0x%x\r\n",param->status);
#endif
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}

int gattm_att_db_hash_comp_rsp_handler(ke_msg_id_t const msgid,
                                       void const *p_param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
     struct gattm_att_db_hash_comp_rsp *param = (struct gattm_att_db_hash_comp_rsp *)p_param;
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattm_att_db_hash_comp_rsp_handler, status:0x%x\r\n",param->status);
#endif
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}






int gattc_cmp_evt_handler(ke_msg_id_t const msgid,
                          void const *p_param,
                          ke_task_id_t const dest_id,
                          ke_task_id_t const src_id)
{
    struct gattc_cmp_evt *param = (struct gattc_cmp_evt *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_cmp_evt_handler   conidx:0x%x,status:0x%x,operaton:0x%x\r\n",conidx,param->status,param->operation);
#endif
    memcpy(&(app_env_info.gattc_cmp_evt_info),param,sizeof( struct gattc_cmp_evt));
    ke_msg_sync_lock_release();
    return (KE_MSG_CONSUMED);
}




int gattc_mtu_changed_ind_handler(ke_msg_id_t const msgid,
                                  void const *p_param,
                                  ke_task_id_t const dest_id,
                                  ke_task_id_t const src_id)
{
    struct gattc_mtu_changed_ind *param = (struct gattc_mtu_changed_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_mtu_changed_ind_handler  mtu:%d conidx:0x%x\r\n",param->mtu,conidx);
#endif
    memcpy(&(app_env_info.mtu_changed_ind_info),param,sizeof( struct gattc_mtu_changed_ind));
    return (KE_MSG_CONSUMED);
}


int gattc_app_disc_svc_ind_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gattc_disc_svc_ind *param = (struct gattc_disc_svc_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_disc_svc_ind_handler   conidx:0x%x,start_hdl:0x%x,end_hdl:0x%x\r\n",conidx,param->start_hdl,param->end_hdl);
#endif
    memcpy(&(app_env_info.disc_svc_ind_info),param,sizeof( struct gattc_disc_svc_ind));
    struct ln_gattc_disc_svc *data = blib_malloc(sizeof(struct ln_gattc_disc_svc) );
    data->conidx=conidx;
    data->start_hdl=param->start_hdl;
    usr_queue_msg_send(BLE_MSG_SVR_DIS,sizeof(struct ln_gattc_disc_svc ),data);
    return (KE_MSG_CONSUMED);
}




int gattc_app_disc_svc_incl_ind_handler(ke_msg_id_t const msgid,
                                        void const *p_param,
                                        ke_task_id_t const dest_id,
                                        ke_task_id_t const src_id)
{
    struct gattc_disc_svc_incl_ind *param = (struct gattc_disc_svc_incl_ind *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_disc_svc_incl_ind_handler   conidx:0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.disc_svc_incl_info),param,sizeof( struct gattc_disc_svc_incl_ind));
    return (KE_MSG_CONSUMED);
}

int gattc_app_disc_char_ind_handler(ke_msg_id_t const msgid,
                                    void const *p_param,
                                    ke_task_id_t const dest_id,
                                    ke_task_id_t const src_id)
{
    struct gattc_disc_char_ind *param = (struct gattc_disc_char_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_disc_char_ind_handler   conidx:0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.disc_char_info),param,sizeof( struct gattc_disc_char_ind));
    return (KE_MSG_CONSUMED);
}


int gattc_app_disc_char_desc_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_disc_char_desc_ind *param = (struct gattc_disc_char_desc_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_disc_char_desc_ind_handler   conidx:0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.disc_char_desc_info),param,sizeof( struct gattc_disc_char_desc_ind));
    return (KE_MSG_CONSUMED);
}


int gattc_read_ind_handler(ke_msg_id_t const msgid,
                           void const *p_param,
                           ke_task_id_t const dest_id,
                           ke_task_id_t const src_id)
{
    struct gattc_read_ind *param = (struct gattc_read_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_read_ind_handler   conidx:0x%x,handler:0x%x,length:0x%x\r\n",conidx,param->handle,param->length);
#endif
    memcpy(&(app_env_info.read_ind_info),param,sizeof( struct gattc_read_ind));
    return (KE_MSG_CONSUMED);
}



int gattc_event_ind_handler(ke_msg_id_t const msgid,
                            void const *p_param,
                            ke_task_id_t const dest_id,
                            ke_task_id_t const src_id)
{
    struct gattc_event_ind const *param = (struct gattc_event_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_event_ind_handler   notify  \r\n");
    LOG(LOG_LVL_TRACE, " receive  ntf data   conidx:%d, handle:%d, length :%d  , data:",conidx,param->handle,param->length);
    for(int i=0; i<param->length; i++)
    {
        LOG(LOG_LVL_TRACE, "0x%02x,",param->value[i]);
    }
    LOG(LOG_LVL_TRACE, "\r\n");
#endif
    memcpy(&(app_env_info.event_ind_info),param,sizeof( struct gattc_event_ind));
    return (KE_MSG_CONSUMED);
}



int gattc_read_req_ind_handler(ke_msg_id_t const msgid,
                               void const *p_param,
                               ke_task_id_t const dest_id,
                               ke_task_id_t const src_id)
{
    struct gattc_read_req_ind const *param = (struct gattc_read_req_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_read_req_ind_handler   conidx:0x%x,handler:0x%x\r\n",conidx,param->handle);
#endif
    memcpy(&(app_env_info.read_req_info),param,sizeof( struct gattc_read_req_ind));
    struct gattc_read_cfm *p_cfm=KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, 2);
    p_cfm->handle = param->handle;
    p_cfm->status =GAP_ERR_NO_ERROR;
    p_cfm->length = 1;
    p_cfm->value[0]=0;
    ke_msg_send(p_cfm);
    return (KE_MSG_CONSUMED);
}




int gattc_write_req_ind_handler(ke_msg_id_t const msgid,
                                void const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{

    struct gattc_write_req_ind const *param = (struct gattc_write_req_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_write_req_ind_handler   conidx:0x%x,handler:0x%x,length:0x%x\r\n",conidx,param->handle,param->length);
#endif
    memcpy(&(app_env_info.write_req_info),param,sizeof( struct gattc_write_req_ind));
    struct gattc_write_cfm *  p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = param->handle;
    p_cfm->status = GAP_ERR_NO_ERROR;
    ke_msg_send(p_cfm);

//  struct ln_attc_write_req_ind *data = blib_malloc(sizeof(struct ln_attc_write_req_ind ) + param->length);
//  data->conidx=conidx;
//  data->handle = param->handle;
//  data->length = param->length;
//  data->offset = param->offset;
//  memcpy(&data->value[0],  param->value,  param->length);
//  usr_queue_msg_send(BLE_MSG_WRITE_DATA, param->length,data);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)param->value, param->length);

    return (KE_MSG_CONSUMED);
}




int gattc_att_info_req_ind_handler(ke_msg_id_t const msgid,
                                   void const *p_param,
                                   ke_task_id_t const dest_id,
                                   ke_task_id_t const src_id)
{
    struct gattc_att_info_req_ind  *param = (struct gattc_att_info_req_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_att_info_req_ind_handler   conidx:0x%x\r\n",conidx);
#endif
    memcpy(&(app_env_info.att_info_req_info),param,sizeof( struct gattc_att_info_req_ind));
    struct gattc_att_info_cfm *p_cfm= KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = param->handle;
    p_cfm->length =1;
    p_cfm->status = GAP_ERR_NO_ERROR;
    ke_msg_send(p_cfm);
    return (KE_MSG_CONSUMED);
}




int gattc_sdp_svc_ind_handler(ke_msg_id_t const msgid,
                              void const *p_param,
                              ke_task_id_t const dest_id,
                              ke_task_id_t const src_id)
{
    struct gattc_sdp_svc_ind  *param = (struct gattc_sdp_svc_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_sdp_svc_ind_handler   conidx:0x%x,start_hdl:0x%x,end_hdl:0x%x\r\n",conidx,param->start_hdl,param->end_hdl);
#endif
    memcpy(&(app_env_info.sdp_svc_info),param,sizeof( struct gattc_sdp_svc_ind));
    return (KE_MSG_CONSUMED);
}




int gattc_transaction_to_error_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_transaction_to_error_ind_handler   conidx:0x%x\r\n",conidx);
#endif
    return (KE_MSG_CONSUMED);
}



int gattc_event_req_ind_handler(ke_msg_id_t const msgid,
                                void const *p_param,
                                ke_task_id_t const dest_id,
                                ke_task_id_t const src_id)
{
    struct gattc_event_ind  *param = (struct gattc_event_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
#if (TRACE_ENABLE)
    LOG(LOG_LVL_TRACE,"gattc_event_req_ind_handler   indication  conidx:0x%x,length:%d,handler:%d\r\n",conidx,param->length,param->handle);
#endif
    memcpy(&(app_env_info.event_req_info),param,sizeof( struct gattc_event_ind));
    struct gattc_event_cfm *p_cfm= KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
    p_cfm->handle = param->handle;
    ke_msg_send(p_cfm);
    return (KE_MSG_CONSUMED);

}


/// @} APP

