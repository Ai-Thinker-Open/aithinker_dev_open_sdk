/**
 ****************************************************************************************
 *
 * @file ln_ble_gatt_ind_handler.c
 *
 * @brief GATT callback funcion source code
 *
 * Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "rwip_config.h"             // SW configuration
#include "gattm_task.h"               // GATT Manager Task API
#include "gattc_task.h"               // GATT Controller Task API

#include "utils/debug/log.h"

#include "ln_ble_gatt_ind_handler.h"
#include "ln_ble_gatt.h"
#include "ln_ble_event_manager.h"


/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static int ln_gatt_svc_get_perm_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_svc_get_permission_rsp *param = (struct gattm_svc_get_permission_rsp *)p_param;
    LOG(LOG_LVL_TRACE,"ln_gatt_svc_get_perm_rsp_handler ,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_svc_set_perm_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_svc_set_permission_rsp *param = (struct gattm_svc_set_permission_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_svc_set_perm_rsp_handler, status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_get_perm_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_get_permission_rsp *param = (struct gattm_att_get_permission_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_get_perm_rsp_handler ,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_set_perm_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_set_permission_rsp *param = (struct gattm_att_set_permission_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_set_perm_rsp_handler ,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_get_value_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_get_value_rsp *param = (struct gattm_att_get_value_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_get_value_rsp_handler, status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_set_value_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_set_value_rsp *param = (struct gattm_att_set_value_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_set_value_rsp_handler, status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_mgr_unknown_msg_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_unknown_msg_ind *param = (struct gattm_unknown_msg_ind *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_mgr_unknown_msg_ind_handler msg_id:0x%x\r\n",param->unknown_msg_id);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_destroy_db_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
     struct gattm_destroy_db_rsp *param = (struct gattm_destroy_db_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_destroy_db_rsp_handler, status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_svc_get_list_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_svc_get_list_rsp *param = (struct gattm_svc_get_list_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_svc_get_list_rsp_handler ,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_get_info_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattm_att_get_info_rsp *param = (struct gattm_att_get_info_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_get_info_rsp_handler ,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_db_hash_comp_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
     struct gattm_att_db_hash_comp_rsp *param = (struct gattm_att_db_hash_comp_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_gatt_att_db_hash_comp_rsp_handler, status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_cmp_evt_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_cmp_evt *param = (struct gattc_cmp_evt *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);
    LOG(LOG_LVL_TRACE,"ln_gatt_cmp_evt_handler conidx:0x%x, status:0x%x,operaton:0x%x\r\n",conidx,param->status,param->operation);

    if(GATTC_INDICATE == param->operation)
    {
        //LOG(LOG_LVL_TRACE,"ln gatt indicate\r\n");
    }
    if(GATTC_NOTIFY == param->operation)
    {
        //LOG(LOG_LVL_TRACE,"ln gatt notify\r\n");
    }
    if(GATTC_WRITE_NO_RESPONSE == param->operation)
    {
        //LOG(LOG_LVL_TRACE,"ln gatt write command\r\n");
    }
    if(GATTC_EXEC_WRITE == param->operation)
    {
        //(LOG_LVL_TRACE,"ln gatt write request\r\n");
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_mtu_changed_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_mtu_changed_ind *param = (struct gattc_mtu_changed_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_INFO,"ln_gatt_mtu_changed_ind_handler  mtu:%d conidx:0x%x\r\n",param->mtu,conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_discover_svc_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_disc_svc_ind *param = (struct gattc_disc_svc_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_discover_svc_ind_handler conidx:0x%x,start_hdl:0x%x,end_hdl:0x%x\r\n",conidx,param->start_hdl,param->end_hdl);

    ble_evt_gatt_svr_disc_t *p_evt_gatt_svr_disc = blib_malloc(sizeof(ble_evt_gatt_svr_disc_t) + param->uuid_len);
    if(p_evt_gatt_svr_disc != NULL)
    {
        p_evt_gatt_svr_disc->conidx = conidx;
        p_evt_gatt_svr_disc->start_hdl = param->start_hdl;
        p_evt_gatt_svr_disc->end_hdl = param->end_hdl;
        p_evt_gatt_svr_disc->uuid_len = param->uuid_len;
        memcpy(p_evt_gatt_svr_disc->uuid, param->uuid, param->uuid_len);
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_DISC_SVC, p_evt_gatt_svr_disc);
        blib_free(p_evt_gatt_svr_disc);
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_discover_include_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gattc_disc_svc_incl_ind *param = (struct gattc_disc_svc_incl_ind *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_discover_include_ind_handler conidx:0x%x\r\n",conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_discover_charac_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gattc_disc_char_ind *param = (struct gattc_disc_char_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_discover_charac_ind_handler conidx:0x%x\r\n",conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_discover_charac_desc_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gattc_disc_char_desc_ind *param = (struct gattc_disc_char_desc_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_discover_charac_desc_ind_handler conidx:0x%x\r\n",conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_read_req_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_read_req_ind const *param = (struct gattc_read_req_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_read_req_ind_handler conidx:0x%x,handler:0x%x\r\n", conidx, param->handle);

    ble_evt_gatt_read_req_t p_evt_gatt_read = {0};

    ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_READ_REQ, &p_evt_gatt_read);

    struct gattc_read_cfm *p_cfm=KE_MSG_ALLOC_DYN(GATTC_READ_CFM, src_id, dest_id, gattc_read_cfm, p_evt_gatt_read.length);
    p_cfm->handle = param->handle;
    p_cfm->status =GAP_ERR_NO_ERROR;
    if(!p_evt_gatt_read.length && NULL != p_evt_gatt_read.value)
    {
        p_cfm->length = p_evt_gatt_read.length;
        memcpy(p_cfm->value, p_evt_gatt_read.value, p_evt_gatt_read.length);
    }
    ke_msg_send(p_cfm);

    if(NULL != p_evt_gatt_read.value)
        blib_free(p_evt_gatt_read.value);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_write_req_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_write_req_ind const *param = (struct gattc_write_req_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_INFO,"ln_gatt_write_req_ind_handler conidx:0x%x,handler:0x%x,length:0x%x\r\n", conidx, param->handle, param->length);

    ble_evt_gatt_write_req_t *p_evt_gatt_write = blib_malloc(sizeof(ble_evt_gatt_write_req_t) + param->length);
    if(p_evt_gatt_write != NULL)
    {
        p_evt_gatt_write->conidx = conidx;
        p_evt_gatt_write->handle = param->handle;
        p_evt_gatt_write->length = param->length;
        p_evt_gatt_write->offset = param->offset;
        memcpy(&p_evt_gatt_write->value[0], param->value, param->length);
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_WRITE_REQ, p_evt_gatt_write);
        blib_free(p_evt_gatt_write);
    }

    struct gattc_write_cfm *  p_cfm = KE_MSG_ALLOC(GATTC_WRITE_CFM, src_id, dest_id, gattc_write_cfm);
    p_cfm->handle = param->handle;
    p_cfm->status = GAP_ERR_NO_ERROR;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_att_info_req_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_att_info_req_ind  *param = (struct gattc_att_info_req_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_att_info_req_ind_handler conidx:0x%x\r\n", conidx);

    struct gattc_att_info_cfm *p_cfm= KE_MSG_ALLOC(GATTC_ATT_INFO_CFM, src_id, dest_id, gattc_att_info_cfm);
    p_cfm->handle = param->handle;
    p_cfm->length =1;
    p_cfm->status = GAP_ERR_NO_ERROR;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_sdp_svc_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_sdp_svc_ind  *param = (struct gattc_sdp_svc_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_sdp_svc_ind_handler conidx:0x%x,start_hdl:0x%x,end_hdl:0x%x\r\n", conidx, param->start_hdl, param->end_hdl);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_trans_error_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_transaction_to_error_ind_handler conidx:0x%x\r\n", conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_read_rsp_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_read_ind *param = (struct gattc_read_ind *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_read_rsp_ind_handler conidx:0x%x,handler:0x%x,length:0x%x\r\n",conidx,param->handle,param->length);

    ble_evt_gatt_read_rsp_t *p_evt_gatt_read_rsp = blib_malloc(sizeof(ble_evt_gatt_read_rsp_t) + param->length);
    if(p_evt_gatt_read_rsp != NULL)
    {
        p_evt_gatt_read_rsp->conidx = conidx;
        p_evt_gatt_read_rsp->handle = param->handle;
        p_evt_gatt_read_rsp->length = param->length;
        p_evt_gatt_read_rsp->offset = param->offset;
        memcpy(&p_evt_gatt_read_rsp->value[0], param->value, param->length);
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_READ_RSP, p_evt_gatt_read_rsp);
        blib_free(p_evt_gatt_read_rsp);
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gatt_notify_event_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_event_ind const *param = (struct gattc_event_ind const *)p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    ble_evt_gatt_notify_t *p_evt_gatt_nf = blib_malloc(sizeof(ble_evt_gatt_notify_t) + param->length);
    if(p_evt_gatt_nf != NULL)
    {
        p_evt_gatt_nf->conidx = conidx;
        p_evt_gatt_nf->handle = param->handle;
        p_evt_gatt_nf->length = param->length;
        memcpy(&p_evt_gatt_nf->value[0], param->value, param->length);
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_NOTIFY, p_evt_gatt_nf);
        blib_free(p_evt_gatt_nf);
    }


    return (KE_MSG_CONSUMED);
}

static int ln_gatt_indication_event_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gattc_event_ind  *param = (struct gattc_event_ind  *) p_param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gatt_indication_event_ind_handler indication  conidx:0x%x,length:%d,handler:%d\r\n",conidx, param->length, param->handle);

    ble_evt_gatt_indicate_t *p_evt_gatt_ind = blib_malloc(sizeof(ble_evt_gatt_indicate_t) + param->length);
    if(p_evt_gatt_ind != NULL)
    {
        p_evt_gatt_ind->conidx = conidx;
        p_evt_gatt_ind->handle = param->handle;
        p_evt_gatt_ind->length = param->length;
        memcpy(&p_evt_gatt_ind->value[0], param->value, param->length);
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_INDICATE, p_evt_gatt_ind);
        blib_free(p_evt_gatt_ind);
    }

    struct gattc_event_cfm *p_cfm= KE_MSG_ALLOC(GATTC_EVENT_CFM, src_id, dest_id, gattc_event_cfm);
    p_cfm->handle = param->handle;
    ke_msg_send(p_cfm);

    return (KE_MSG_CONSUMED);
}

struct ke_msg_handler app_gatt_general_msg_tab[] =
{
    /* gatt common interface*/
    {GATTC_CMP_EVT,                  ln_gatt_cmp_evt_handler },
    {GATTC_MTU_CHANGED_IND,          ln_gatt_mtu_changed_ind_handler},

    /*gatt client interface*/
    {GATTM_SVC_GET_PERMISSION_RSP,   ln_gatt_svc_get_perm_rsp_handler},
    {GATTM_SVC_SET_PERMISSION_RSP,   ln_gatt_svc_set_perm_rsp_handler},
    {GATTM_ATT_GET_PERMISSION_RSP,   ln_gatt_att_get_perm_rsp_handler},
    {GATTM_ATT_SET_PERMISSION_RSP,   ln_gatt_att_set_perm_rsp_handler},
    {GATTM_ATT_GET_VALUE_RSP,        ln_gatt_att_get_value_rsp_handler},
    {GATTM_ATT_SET_VALUE_RSP,        ln_gatt_att_set_value_rsp_handler},
    {GATTM_UNKNOWN_MSG_IND,          ln_gatt_mgr_unknown_msg_ind_handler},
    {GATTM_DESTROY_DB_RSP,           ln_gatt_destroy_db_rsp_handler},
    {GATTM_SVC_GET_LIST_RSP,         ln_gatt_svc_get_list_rsp_handler},
    {GATTM_ATT_GET_INFO_RSP,         ln_gatt_att_get_info_rsp_handler},
    {GATTM_ATT_DB_HASH_COMP_RSP,     ln_gatt_att_db_hash_comp_rsp_handler},
    {GATTC_SDP_SVC_IND,              ln_gatt_sdp_svc_ind_handler},
    {GATTC_DISC_SVC_IND,             ln_gatt_discover_svc_ind_handler},
    {GATTC_DISC_SVC_INCL_IND,        ln_gatt_discover_include_ind_handler},
    {GATTC_DISC_CHAR_IND,            ln_gatt_discover_charac_ind_handler},
    {GATTC_DISC_CHAR_DESC_IND,       ln_gatt_discover_charac_desc_ind_handler},
    {GATTC_READ_IND,                 ln_gatt_read_rsp_ind_handler},
    {GATTC_ATT_INFO_REQ_IND,         ln_gatt_att_info_req_ind_handler},
    {GATTC_TRANSACTION_TO_ERROR_IND, ln_gatt_trans_error_ind_handler},
    {GATTC_EVENT_IND,                ln_gatt_notify_event_ind_handler},
    {GATTC_EVENT_REQ_IND,            ln_gatt_indication_event_ind_handler},
        

    /*gatt server interface*/
    {GATTC_READ_REQ_IND,             ln_gatt_read_req_ind_handler},
    {GATTC_WRITE_REQ_IND,            ln_gatt_write_req_ind_handler},
};

int app_gatt_msg_size(void)
{
    return sizeof((app_gatt_general_msg_tab))/sizeof((app_gatt_general_msg_tab)[0]);
}

