/**
 ****************************************************************************************
 *
 * @file ln_ble_battery_client.c
 *
 * @brief gatt battery client
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
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattc_task.h"
#include "gattm_task.h"

#include "utils/debug/log.h"

#include "ln_ble_battery_client.h"
#include "ln_ble_gatt.h"
#include "ln_ble_rw_app_task.h"


#if (BLE_BATT_CLIENT)

#include "basc_task.h"


gatt_battery_cli_t batt_client = {0};

static int ln_batt_cli_enable_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct basc_enable_rsp *param = (struct basc_enable_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"basc_enable_rsp_handler,bas_nb:0x%x,status:0x%x\r\n",param->bas_nb,param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_batt_cli_read_info_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct basc_read_info_rsp *param = (struct basc_read_info_rsp *)p_param;
    LOG(LOG_LVL_TRACE,"basc_read_info_rsp_handler,info:0x%x,status:0x%x\r\n",param->info,param->status);

    return (KE_MSG_CONSUMED);
}


static int ln_batt_cli_level_ntf_cfg_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct basc_batt_level_ntf_cfg_rsp *param = (struct basc_batt_level_ntf_cfg_rsp *)p_param;
    LOG(LOG_LVL_TRACE,"basc_batt_level_ntf_cfg_rsp_handler,bas_nb:0x%x,status:0x%x\r\n",param->bas_nb,param->status);

    return (KE_MSG_CONSUMED);
}

static int ln_batt_cli_level_ind_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct basc_batt_level_ind *param = (struct basc_batt_level_ind *)p_param;
    LOG(LOG_LVL_TRACE,"basc_batt_level_ind_handler,bas_nb:0x%x,batt_level:0x%x\r\n",param->bas_nb,param->batt_level);

    return (KE_MSG_CONSUMED);
}

void ln_batt_client_add(void)
{
    struct bass_db_cfg* db_cfg;
    // Allocate the BASS_CREATE_DB_REQ
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                            TASK_GAPM, TASK_APP,
                                            gapm_profile_task_add_cmd, 0);
    // Fill message
    req->operation   = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl     = 0;//PERM(SVC_AUTH, UNAUTH);
    req->prf_task_id = TASK_ID_BASC;
    req->app_task    = TASK_APP;
    req->start_hdl   = 0;

    // Send the message
    ln_ke_msg_send(req);
}

void ln_batt_cli_discover_server(uint8_t conidx)
{
    // Allocate the message
    struct basc_enable_req * req = KE_MSG_ALLOC(BASC_ENABLE_REQ,
                                   KE_BUILD_ID(batt_client.prf_task_nb, conidx),
                                   TASK_APP,
                                   basc_enable_req);

    // Fill in the parameter structure
    req->con_type = PRF_CON_DISCOVERY;//PRF_CON_DISCOVERY
    // Send the message
    ln_ke_msg_send(req);
}

void ln_batt_cli_ntf_cfg_req(uint8_t conidx, batt_cli_ntf_cfg_req_t *p_req)
{
    // Allocate the message
    struct basc_batt_level_ntf_cfg_req * req = KE_MSG_ALLOC(BASC_BATT_LEVEL_NTF_CFG_REQ,
            KE_BUILD_ID(batt_client.prf_task_nb, conidx),
            TASK_APP,
            basc_batt_level_ntf_cfg_req);

    // Fill in the parameter structure, see @enum prf_cli_conf
    req->ntf_cfg = p_req->ntf_cfg;
    req->bas_nb  = p_req->bas_nb;
    // Send the message
    ln_ke_msg_send(req);
}

void ln_batt_cli_rd_char_req(uint8_t conidx, batt_cli_read_info_req_t * p_req)
{
    // Allocate the message
    struct basc_read_info_req * req = KE_MSG_ALLOC(BASC_READ_INFO_REQ,
                                      KE_BUILD_ID(batt_client.prf_task_nb, conidx),
                                      TASK_APP,
                                      basc_read_info_req);


    // Fill in the parameter structure
    req->info= p_req->info;
    req->bas_nb = p_req->bas_nb;

    // Send the message
    ln_ke_msg_send(req);
}

static struct ke_msg_handler app_batt_client_msg_tab[] =
{
    {BASC_ENABLE_RSP, ln_batt_cli_enable_rsp_handler },
    {BASC_READ_INFO_RSP, ln_batt_cli_read_info_rsp_handler},
    {BASC_BATT_LEVEL_NTF_CFG_RSP, ln_batt_cli_level_ntf_cfg_rsp_handler},
    {BASC_BATT_LEVEL_IND, ln_batt_cli_level_ind_handler},
};

int ln_batt_client_init(void)
{
    int ret = 0;
    ret = ln_app_msg_regester(app_batt_client_msg_tab, ARRAY_LEN(app_batt_client_msg_tab));

    return ret;
}

#endif

