/**
 ****************************************************************************************
 *
 * @file ln_ble_hid_server.c
 *
 * @brief gatt hid server
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rwip_config.h"
#include "att.h"
#include "attm.h"
#include "gattm_task.h"               // GATT Manager Task API
#include "gattc_task.h"               // GATT Controller Task API
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "prf_types.h"

#include "utils/debug/log.h"

#include "ln_ble_gatt.h"
#include "ln_ble_hid_server.h"
#include "ln_ble_rw_app_task.h"

#if (BLE_HID_DEVICE)

#include "hogpd_task.h"


gatt_hid_svr_t hid_server = {0};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
static const uint8_t hid_mouse_report_map[] =
{
    0x05, 0x01,         //Usage Page (Generic Desktop)
    0x09, 0x02,         //Usage (Mouse)
    0xA1, 0x01,         //Collection (Application)
    0x85, 0x01,         //Report Id (1)
    0x09, 0x01,         //Usage (Pointer)
    0xA1, 0x00,         //Collection (Physical)
    0x05, 0x09,         //Usage Page (Button)
    0x19, 0x01,         //Usage Minimum (Button 1)
    0x29, 0x08,         //Usage Maximum (Button 8)
    0x15, 0x00,         //Logical minimum (0)
    0x25, 0x01,         //Logical maximum (1)
    0x95, 0x08,         //Report Count (8)
    0x75, 0x01,         //Report Size (1)
    0x81, 0x02,         //Input (Data,Value,Absolute,Bit Field)
    0x05, 0x01,         //Usage Page (Generic Desktop)
    0x09, 0x30,         //Usage (X)
    0x09, 0x31,         //Usage (Y)
    0x15, 0x81,         //Logical minimum (-127)
    0x25, 0x7F,         //Logical maximum (127)
    0x75, 0x08,         //Report Size (12)
    0x95, 0x02,         //Report Count (2)
    0x81, 0x06,         //Input (Data,Value,Relative,Bit Field)
    0x09, 0x38,         //Usage (Wheel)
    0x15, 0x81,         //Logical minimum (-127)
    0x25, 0x7F,         //Logical maximum (127)
    0x75, 0x08,         //Report Size (8)
    0x95, 0x01,         //Report Count (1)
    0x81, 0x06,         //Input (Data,Value,Relative,Bit Field)
    0x05, 0x0C,         //Usage Page (Consumer)
    0x0A, 0x38, 0x02,   //Usage (AC Pan)
    0x81, 0x06,         //Input (Data,Value,Relative,Bit Field)
    0xC0,               //End Collection
    0xC0,               //End Collection
    0x05, 0x01,         //Usage Page (Generic Desktop)
    0x09, 0x06,         //Usage (Keyboard)
    0xA1, 0x01,         //Collection (Application)
    0x85, 0x02,         //Report Id (2)
    0x05, 0x07,         //Usage Page (Keyboard)
    0x19, 0xE0,         //Usage Minimum (Keyboard Left Control)
    0x29, 0xE7,         //Usage Maximum (Keyboard Right GUI)
    0x15, 0x00,         //Logical minimum (0)
    0x25, 0x01,         //Logical maximum (1)
    0x75, 0x01,         //Report Size (1)
    0x95, 0x08,         //Report Count (8)
    0x81, 0x02,         //Input (Data,Value,Absolute,Bit Field)
    0x95, 0x01,         //Report Count (1)
    0x75, 0x08,         //Report Size (8)
    0x81, 0x01,         //Input (Constant,Array,Absolute,Bit Field)
    0x95, 0x06,         //Report Count (6)
    0x75, 0x08,         //Report Size (8)
    0x15, 0x00,         //Logical minimum (0)
    0x25, 0x0F,         //Logical maximum (15)
    0x05, 0x07,         //Usage Page (Keyboard)
    0x19, 0x00,         //Usage Minimum (No event indicated)
    0x29, 0x0F,         //Usage Maximum (Keyboard l and L)
    0x81, 0x00,         //Input (Data,Array,Absolute,Bit Field)
    0xC0,               //End Collection
    0x05, 0x01,         //Usage Page (Generic Desktop)
    0x09, 0x80,         //Usage (System Control)
    0xA1, 0x01,         //Collection (Application)
    0x85, 0x03,         //Report Id (3)
    0x15, 0x00,         //Logical minimum (0)
    0x25, 0x01,         //Logical maximum (1)
    0x75, 0x01,         //Report Size (1)
    0x95, 0x03,         //Report Count (3)
    0x09, 0x81,         //Usage (System Power Down)
    0x09, 0x82,         //Usage (System Sleep)
    0x09, 0x83,         //Usage (System Wake Up)
    0x81, 0x02,         //Input (Data,Value,Absolute,Bit Field)
    0x95, 0x01,         //Report Count (1)
    0x75, 0x05,         //Report Size (5)
    0x81, 0x03,         //Input (Constant,Value,Absolute,Bit Field)
    0xC0                //End Collection
};

#if 0
// mouse report map
static const uint8_t hid_keyboard_report_map[] = {
        0x05, 0x01, // Usage Page (Generic Desktop)
        0x09, 0x02, // Usage (Mouse)

        0xA1, 0x01, // Collection (Application)

        // Report ID 1: Mouse buttons + scroll/pan
        0x85, 0x01,       // Report Id 1
        0x09, 0x01,       // Usage (Pointer)
        0xA1, 0x00,       // Collection (Physical)
        0x95, 0x05,       // Report Count (3)
        0x75, 0x01,       // Report Size (1)
        0x05, 0x09,       // Usage Page (Buttons)
        0x19, 0x01,       // Usage Minimum (01)
        0x29, 0x05,       // Usage Maximum (05)
        0x15, 0x00,       // Logical Minimum (0)
        0x25, 0x01,       // Logical Maximum (1)
        0x81, 0x02,       // Input (Data, Variable, Absolute)
        0x95, 0x01,       // Report Count (1)
        0x75, 0x03,       // Report Size (3)
        0x81, 0x01,       // Input (Constant) for padding
        0x75, 0x08,       // Report Size (8)
        0x95, 0x01,       // Report Count (1)
        0x05, 0x01,       // Usage Page (Generic Desktop)
        0x09, 0x38,       // Usage (Wheel)
        0x15, 0x81,       // Logical Minimum (-127)
        0x25, 0x7F,       // Logical Maximum (127)
        0x81, 0x06,       // Input (Data, Variable, Relative)
        0x05, 0x0C,       // Usage Page (Consumer)
        0x0A, 0x38, 0x02, // Usage (AC Pan)
        0x95, 0x01,       // Report Count (1)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0xC0,             // End Collection (Physical)

        // Report ID 2: Mouse motion
        0x85, 0x02,       // Report Id 2
        0x09, 0x01,       // Usage (Pointer)
        0xA1, 0x00,       // Collection (Physical)
        0x75, 0x0C,       // Report Size (12)
        0x95, 0x02,       // Report Count (2)
        0x05, 0x01,       // Usage Page (Generic Desktop)
        0x09, 0x30,       // Usage (X)
        0x09, 0x31,       // Usage (Y)
        0x16, 0x01, 0xF8, // Logical maximum (2047)
        0x26, 0xFF, 0x07, // Logical minimum (-2047)
        0x81, 0x06,       // Input (Data, Variable, Relative)
        0xC0,             // End Collection (Physical)
        0xC0,             // End Collection (Application)

        // Report ID 3: Advanced buttons
        0x05, 0x0C,       // Usage Page (Consumer)
        0x09, 0x01,       // Usage (Consumer Control)
        0xA1, 0x01,       // Collection (Application)
        0x85, 0x03,       // Report Id (3)
        0x15, 0x00,       // Logical minimum (0)
        0x25, 0x01,       // Logical maximum (1)
        0x75, 0x01,       // Report Size (1)
        0x95, 0x01,       // Report Count (1)

        0x09, 0xCD,       // Usage (Play/Pause)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x0A, 0x83, 0x01, // Usage (AL Consumer Control Configuration)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x09, 0xB5,       // Usage (Scan Next Track)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x09, 0xB6,       // Usage (Scan Previous Track)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)

        0x09, 0xEA,       // Usage (Volume Down)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x09, 0xE9,       // Usage (Volume Up)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x0A, 0x25, 0x02, // Usage (AC Forward)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0x0A, 0x24, 0x02, // Usage (AC Back)
        0x81, 0x06,       // Input (Data,Value,Relative,Bit Field)
        0xC0              // End Collection
    };
#endif


#if 0
static const uint8_t hid_keyboard_report_map[] = {
    0x05, 0x01,
    0x09, 0x02,//mouse
    0xA1, 0x01,  //app
    0x85, 0x01,//report id 1
    0x09, 0x01,//pointer
    0xA1, 0x00,//
    0x05, 0x09, //buttons
    0x19, 0x01, //button 1
    0x29, 0x08, //button 8
    0x15, 0x00,   //0
    0x25, 0x01, //3//1
    0x95, 0x08, //report count 8
    0x75, 0x01, //report size 1
    0x81, 0x02,  //input
    0x05, 0x01,  //desktop
    0x09, 0x30,  //x
    0x09, 0x31,//y
    0x16, 0x01, 0xF8,  // min -2047
    0x26, 0xFF, 0x07, //max 2047
    0x75, 0x0C, //report 12
    0x95, 0x02, //report count 2
    0x81, 0x06, //input
    0x09, 0x38, //wheel 
    0x15, 0x81, //min -127
    0x25, 0x7F,  //max 127
    0x75, 0x08,  //report size 8
    0x95, 0x01,  //report count 1
    0x81, 0x06,  //input
    0x05, 0x0C,  //consumer
    0x0A, 0x38, 
    0x02, 
    0x81, 0x06,//input
    0xC0 
};
#endif

static void ln_hid_svr_proto_mode_req_cfm(struct hogpd_proto_mode_cfm *p_param);
static void ln_hid_svr_report_req_cfm(struct hogpd_report_cfm *p_param);

static int ln_hid_svr_enable_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogpd_enable_rsp *param = (struct hogpd_enable_rsp *)p_param;
    uint8_t conidx = param->conidx;

    LOG(LOG_LVL_INFO,"hogpd_enable_rsp_handler,status:0x%x,conidx:0x%x\r\n",param->status,conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_hid_svr_report_upd_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogpd_report_upd_rsp *param = (struct hogpd_report_upd_rsp *)p_param;
    uint8_t conidx = param->conidx;

    LOG(LOG_LVL_INFO,"hogpd_report_upd_rsp_handler,status:0x%x,conidx:0x%x\r\n",param->status,conidx);

    if( param->status == GAP_ERR_NO_ERROR)
    {
        //struct hogpd_report_upd_rsp *data = blib_malloc(sizeof(struct hogpd_report_upd_rsp ));
        //data->conidx=param->conidx;
        //usr_queue_msg_send(BLE_MSG_HOGPD_REP_UPD_RSP,sizeof(struct hogpd_report_upd_rsp ),data);
    }
    return (KE_MSG_CONSUMED);
}

static int ln_hid_svr_report_req_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogpd_report_req_ind *param = (struct hogpd_report_req_ind *)p_param;
    uint8_t conidx = param->conidx;

    LOG(LOG_LVL_INFO,"hogpd_report_req_ind_handler,opeation:0x%x,type:0x%x,hid_idx:0x%x,idx:0x%x,length:0x%x\r\n",param->operation,param->report.type,param->report.hid_idx,param->report.idx,param->report.length);

    if (param->operation == HOGPD_OP_REPORT_READ)
    {
        if(param->report.type == HOGPD_REPORT)
        {
            struct hogpd_report_cfm *req = blib_malloc(sizeof(struct hogpd_report_cfm) + 8);
            memset(req,0,sizeof(struct hogpd_report_cfm) +8);
            req->report.hid_idx = 0;
            req->report.length = 8;
            req->report.idx = 0;
            req->report.type =  HOGPD_REPORT;//HOGPD_BOOT_MOUSE_INPUT_REPORT;
            req->report.value[1] =rand()%255; 
            req->report.value[2] = rand()%255;
            req->report.value[3] = rand()%255;
            req->operation = param->operation;
            req->conidx = param->conidx;
            ln_hid_svr_report_req_cfm(req);
            blib_free(req);
        }
        if (param->report.type == HOGPD_REPORT_MAP)
        {
            struct hogpd_report_cfm *req = blib_malloc(sizeof(struct hogpd_report_cfm) + sizeof(hid_mouse_report_map));
            memset(req, 0, sizeof(struct hogpd_report_cfm) +sizeof(hid_mouse_report_map));
            req->conidx = conidx;
            req->status = GAP_ERR_NO_ERROR;
            req->operation = param->operation;
            req->report.hid_idx = param->report.hid_idx;
            req->report.type = param->report.type;   
            req->report.idx = param->report.idx;
            req->report.length = sizeof(hid_mouse_report_map);
            /// Report data
            memcpy(&req->report.value[0], &hid_mouse_report_map[0], sizeof(hid_mouse_report_map));
            ln_hid_svr_report_req_cfm(req);
            blib_free(req);
        }
        if(param->report.type == HOGPD_BOOT_KEYBOARD_INPUT_REPORT)
        {
            struct hogpd_report_cfm *req = blib_malloc(sizeof(struct hogpd_report_cfm) + 8);
            memset(req, 0, sizeof(struct hogpd_report_cfm) + 8);
            req->report.hid_idx = 0;
            req->report.length = 8;
            req->report.idx = 0;
            req->report.type =  HOGPD_BOOT_KEYBOARD_INPUT_REPORT;
            req->report.value[1] = 00;
            req->report.value[2] = 01;
            req->operation = param->operation;
            req->conidx = param->conidx;
            req->status = GAP_ERR_NO_ERROR;
            ln_hid_svr_report_req_cfm(req);
            blib_free(req);
        }
        if(param->report.type == HOGPD_BOOT_MOUSE_INPUT_REPORT)
        {
            struct hogpd_report_cfm *req = blib_malloc(sizeof(struct hogpd_report_cfm) + 4);
            memset(req, 0, sizeof(struct hogpd_report_cfm) + 4);
            req->report.hid_idx = 0;
            req->report.length = 4;
            req->report.idx = 0;
            req->report.type =  HOGPD_BOOT_MOUSE_INPUT_REPORT;
            req->report.value[1] =rand()%255; 
            req->report.value[2] = rand()%255;
            req->report.value[3] = rand()%255;
            req->operation = param->operation;
            req->conidx = param->conidx;
            ln_hid_svr_report_req_cfm(req);
            blib_free(req);
        }
    }

    return (KE_MSG_CONSUMED);
}

static int ln_hid_svr_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogpd_ntf_cfg_ind *param = (struct hogpd_ntf_cfg_ind *)p_param;
    uint8_t conidx = param->conidx;

    LOG(LOG_LVL_INFO,"hogpd_ntf_cfg_ind_handler,ntf_cfg:0x%x\r\n",param->ntf_cfg[conidx]);

    //struct hogpd_ntf_cfg_ind *data = blib_malloc(sizeof(struct hogpd_ntf_cfg_ind ));
    //data->conidx=param->conidx;
    //usr_queue_msg_send(BLE_MSG_HOGPD_NTF_CFG_IND,sizeof(struct hogpd_ntf_cfg_ind ),data);
    return (KE_MSG_CONSUMED);
}

static int ln_hid_svr_ctnl_pt_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    LOG(LOG_LVL_INFO,"hogpd_ctnl_pt_ind_handler\r\n");

    return (KE_MSG_CONSUMED);
}

static int ln_hid_svr_proto_mode_req_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogpd_proto_mode_req_ind *param = (struct hogpd_proto_mode_req_ind *)p_param;

    LOG(LOG_LVL_INFO,"hogpd_proto_mode_reqind_handler\r\n");

    struct hogpd_proto_mode_cfm cfm;
    cfm.conidx = param->conidx;
    cfm.hid_idx = param->hid_idx;
    cfm.proto_mode = param->proto_mode;
    cfm.status = 0;
    ln_hid_svr_proto_mode_req_cfm(&cfm);
    return (KE_MSG_CONSUMED);
}

static void ln_hid_svr_report_req_cfm(struct hogpd_report_cfm *p_param)
{
    struct hogpd_report_cfm *p_cmd = KE_MSG_ALLOC_DYN(HOGPD_REPORT_CFM,
                                                       KE_BUILD_ID(hid_server.prf_task_nb,p_param->conidx),
                                                       TASK_APP,
                                                         hogpd_report_cfm,p_param->report.length);
    p_cmd->conidx = p_param->conidx;
    p_cmd->operation = p_param->operation;
    p_cmd->status = p_param->status;
    memcpy(&p_cmd->report, &p_param->report, (sizeof(struct hogpd_report_info) + p_param->report.length));

    ke_msg_send(p_cmd);
}

static void ln_hid_svr_proto_mode_req_cfm(struct hogpd_proto_mode_cfm *p_param)
{
    struct hogpd_proto_mode_cfm *p_cmd = KE_MSG_ALLOC(HOGPD_REPORT_CFM,
                                                    KE_BUILD_ID(hid_server.prf_task_nb,p_param->conidx),
                                                    TASK_APP,
                                                    hogpd_proto_mode_cfm);
    p_cmd->conidx = p_param->conidx;
    p_cmd->status = p_param->status;
    p_cmd->hid_idx = p_param->hid_idx;
    p_cmd->proto_mode = p_param->proto_mode;

    ke_msg_send(p_cmd);
}

void ln_hid_server_add(void)
{
    struct hogpd_db_cfg *db_cfg;
    struct gapm_profile_task_add_cmd *p_cmd = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                                     TASK_GAPM, TASK_APP,
                                                     gapm_profile_task_add_cmd, sizeof(struct hogpd_db_cfg));

    p_cmd->operation = GAPM_PROFILE_TASK_ADD;
    p_cmd->sec_lvl = 0;
    p_cmd->prf_task_id = TASK_ID_HOGPD;
    p_cmd->app_task = TASK_APP;
    p_cmd->start_hdl = 0;
    db_cfg = (struct hogpd_db_cfg* ) p_cmd->param;
    db_cfg->hids_nb = 1;

    for (int i = 0; i < db_cfg->hids_nb; i++)
    {
        db_cfg->cfg[i].svc_features = HOGPD_CFG_MOUSE|HOGPD_CFG_KEYBOARD;
        db_cfg->cfg[i].report_nb = 3;
        for (int j = 0; j < db_cfg->cfg[i].report_nb; j++)
        {
            db_cfg->cfg[i].report_char_cfg[j] = j+1;
            db_cfg->cfg[i].report_id[j] = HOGPD_CFG_REPORT_IN;
        }
        db_cfg->cfg[i].hid_info.bcdHID = 0x100; // HID Version 1.00
        db_cfg->cfg[i].hid_info.bCountryCode = 0x00;
        db_cfg->cfg[i].hid_info.flags = HIDS_REMOTE_WAKE_CAPABLE | HIDS_NORM_CONNECTABLE;;
    }

    ln_ke_msg_send(p_cmd);
}

void ln_hid_svr_enable(int conidx)
{
    struct hogpd_enable_req *p_cmd = KE_MSG_ALLOC(HOGPD_ENABLE_REQ,
                                               KE_BUILD_ID(hid_server.prf_task_nb,conidx),
                                               TASK_APP,
                                                hogpd_enable_req);
    p_cmd->conidx = conidx;
    p_cmd->ntf_cfg[0] = HOGPD_CFG_MASK | HOGPD_CFG_REPORT_NTF_EN;

    ln_ke_msg_send(p_cmd);
}

void ln_hid_svr_report_upd_req(hid_svr_report_upd_req_t *p_param)
{
    struct hogpd_report_upd_req *p_cmd = KE_MSG_ALLOC_DYN(HOGPD_REPORT_UPD_REQ,
                                                       KE_BUILD_ID(hid_server.prf_task_nb,p_param->conidx),
                                                        TASK_APP,
                                                        hogpd_report_upd_req, p_param->length);
    p_cmd->conidx = p_param->conidx;
    p_cmd->report.hid_idx = p_param->hid_idx;
    p_cmd->report.type = p_param->type;
    p_cmd->report.length = p_param->length;
    p_cmd->report.idx = p_param->idx;
    memcpy(p_cmd->report.value, p_param->value, p_param->length);

    ln_ke_msg_send(p_cmd);
}


static struct ke_msg_handler app_hid_server_msg_tab[] =
{
    {HOGPD_ENABLE_RSP, ln_hid_svr_enable_rsp_handler },
    {HOGPD_REPORT_UPD_RSP, ln_hid_svr_report_upd_rsp_handler},
    {HOGPD_REPORT_REQ_IND, ln_hid_svr_report_req_ind_handler},
    {HOGPD_PROTO_MODE_REQ_IND, ln_hid_svr_proto_mode_req_ind_handler},

    {HOGPD_NTF_CFG_IND, ln_hid_svr_ntf_cfg_ind_handler},
    {HOGPD_CTNL_PT_IND, ln_hid_svr_ctnl_pt_ind_handler},
};

int ln_hid_server_init(void)
{
    int ret = 0;
    ret = ln_app_msg_regester(app_hid_server_msg_tab, ARRAY_LEN(app_hid_server_msg_tab));

    return ret;
}

#endif

