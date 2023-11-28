/**
 ****************************************************************************************
 *
 * @file ln_ble_battery_server.c
 *
 * @brief gatt battery server
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

#include "ln_ble_battery_server.h"
#include "ln_ble_gatt.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_event_manager.h"

#if (BLE_BATT_SERVER)

#include "bass_task.h"


gatt_batt_svr_t batt_server = {0};

const static struct prf_char_pres_fmt batt_lvl_pres_fmt =
{
    /* percentage */
    .unit = ATT_UNIT_PERCENTAGE,
    /* first */
    .description = 0x0001,
    /* unsigned 8-bit integer */
    .format = ATT_FORMAT_UINT8,
    /* actual value = Characteristic Value * 10Exp */
    .exponent = 0,
    /* Bluetooth SIG */
    .name_space = 0x01,
};

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */

static int ln_batt_svr_enable_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct bass_enable_rsp *param = (struct bass_enable_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"bass_enable_rsp_handler,conidx:%d,status:0x%x\r\n",param->conidx,param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_batt_svr_level_upd_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct bass_batt_level_upd_rsp *param = (struct bass_batt_level_upd_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"bass_batt_level_upd_rsp_handler,status:0x%x\r\n",param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_batt_svr_level_ntf_cfg_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct bass_batt_level_ntf_cfg_ind *param = (struct bass_batt_level_ntf_cfg_ind *)p_param;

    LOG(LOG_LVL_TRACE,"bass_batt_level_ntf_cfg_ind_handler,conidx:%d,ntf_cfg:0x%x\r\n",param->conidx,param->ntf_cfg);
#if 0
    uint8_t *ntf_cfg = blib_malloc(sizeof(uint8_t));

    if(data != NULL)
    {
        *ntf_cfg = param->ntf_cfg;
        usr_queue_msg_send(BLE_MSG_BATT_LVL_NTF_CFG, sizeof(uint8_t), ntf_cfg);
    }
#endif
    ble_evt_gatt_write_req_t *p_evt_gatt_write = blib_malloc(sizeof(ble_evt_gatt_write_req_t) + 1);
    if(p_evt_gatt_write != NULL)
    {
        p_evt_gatt_write->conidx = param->conidx;
        p_evt_gatt_write->handle = param->handle;
        p_evt_gatt_write->length = 1;
        p_evt_gatt_write->offset = 0;
        p_evt_gatt_write->value[0] = param->ntf_cfg;
        ln_ble_evt_mgr_process(BLE_EVT_ID_GATT_WRITE_REQ, p_evt_gatt_write);
        blib_free(p_evt_gatt_write);
    }

    return (KE_MSG_CONSUMED);
}

void ln_batt_server_add(void)
{
    struct bass_db_cfg* db_cfg;
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                            TASK_GAPM, TASK_APP,
                                            gapm_profile_task_add_cmd, sizeof(struct bass_db_cfg));
    // Fill message
    req->operation   = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl     = 0;//PERM(SVC_AUTH, UNAUTH);
    req->prf_task_id = TASK_ID_BASS;
    req->app_task    = TASK_APP;
    req->start_hdl   = 0;

    // Set parameters
    db_cfg           = (struct bass_db_cfg* ) req->param;
    // Add a BAS instance
    db_cfg->bas_nb   = 1;

    // Add a BAS instance
    for(int i=0; i<db_cfg->bas_nb; i++)
    {
        db_cfg->features[i] = BAS_BATT_LVL_NTF_SUP;
        memcpy(&db_cfg->batt_level_pres_format[i],&batt_lvl_pres_fmt,sizeof(struct prf_char_pres_fmt));
    }
    // Send the message
    ln_ke_msg_send(req);
}

void ln_batt_svr_enable_prf(batt_svr_enable_req_t * p_req)
{
    // Allocate the message
    struct bass_enable_req * req = KE_MSG_ALLOC(BASS_ENABLE_REQ,
                                   KE_BUILD_ID(batt_server.prf_task_nb, p_req->conidx),
                                   TASK_APP,
                                   bass_enable_req);

    // Fill in the parameter structure
    req->conidx             = p_req->conidx;
    // NTF initial status - Disabled
    req->ntf_cfg           = p_req->ntf_cfg;
    for(int i=0; i<BASS_NB_BAS_INSTANCES_MAX; i++)
    {
        req->old_batt_lvl[i]=p_req->old_batt_lvl[i];
    }
    // Send the message
    ln_ke_msg_send(req);
}

void ln_batt_svr_send_lvl(uint8_t conidx, batt_svr_level_upd_req_t *p_req)
{
    ASSERT_ERR(p_req->batt_level <= BAS_BATTERY_LVL_MAX);
    // Allocate the message
    struct bass_batt_level_upd_req * req = KE_MSG_ALLOC(BASS_BATT_LEVEL_UPD_REQ,
                                           KE_BUILD_ID(batt_server.prf_task_nb, conidx),
                                           TASK_APP,
                                           bass_batt_level_upd_req);
    // Fill in the parameter structure
    req->bas_instance = p_req->bas_instance;
    req->batt_level   = p_req->batt_level;
    // Send the message
    ln_ke_msg_send(req);
}

static struct ke_msg_handler app_batt_server_msg_tab[] =
{
    {BASS_ENABLE_RSP, ln_batt_svr_enable_rsp_handler },
    {BASS_BATT_LEVEL_UPD_RSP, ln_batt_svr_level_upd_rsp_handler},
    {BASS_BATT_LEVEL_NTF_CFG_IND, ln_batt_svr_level_ntf_cfg_ind_handler},
};

int ln_batt_server_init(void)
{
    int ret = 0;
    ret = ln_app_msg_regester(app_batt_server_msg_tab, ARRAY_LEN(app_batt_server_msg_tab));

    return ret;
}

#endif

