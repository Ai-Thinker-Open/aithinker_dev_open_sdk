/**
 ****************************************************************************************
 *
 * @file ln_ble_hid_client.c
 *
 * @brief gatt hid client
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

#include "ln_ble_hid_client.h"
#include "ln_ble_gatt.h"
#include "ln_ble_rw_app_task.h"


#if (BLE_HID_REPORT_HOST)

#include "hogprh_task.h"


gatt_hid_cli_t hid_client = {0};

static int ln_hid_cli_enable_rsp_handler(ke_msg_id_t const msgid,
        void const *p_param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct hogprh_enable_rsp *param = (struct hogprh_enable_rsp *)p_param;

    LOG(LOG_LVL_TRACE,"ln_hid_cli_enable_rsp_handler,status:0x%x\r\n", param->status);

    return (KE_MSG_CONSUMED);
}

void ln_hid_client_add(void)
{
    // Allocate the BASS_CREATE_DB_REQ
    struct gapm_profile_task_add_cmd *req = KE_MSG_ALLOC_DYN(GAPM_PROFILE_TASK_ADD_CMD,
                                            TASK_GAPM, TASK_APP,
                                            gapm_profile_task_add_cmd, 0);
    // Fill message
    req->operation   = GAPM_PROFILE_TASK_ADD;
    req->sec_lvl     = 0;//PERM(SVC_AUTH, UNAUTH);
    req->prf_task_id = TASK_ID_HOGPRH;
    req->app_task    = TASK_APP;
    req->start_hdl   = 0;

    // Send the message
    ln_ke_msg_send(req);
}

void ln_hid_cli_discover_server(uint8_t conidx)
{
    // Allocate the message
    struct hogprh_enable_req * req = KE_MSG_ALLOC(HOGPRH_ENABLE_REQ,
                                   KE_BUILD_ID(hid_client.prf_task_nb, conidx),
                                   TASK_APP,
                                   hogprh_enable_req);

    // Fill in the parameter structure
    req->con_type = PRF_CON_DISCOVERY;//PRF_CON_DISCOVERY
    // Send the message
    ln_ke_msg_send(req);
}

static struct ke_msg_handler app_hid_client_msg_tab[] =
{
    {HOGPRH_ENABLE_RSP, ln_hid_cli_enable_rsp_handler },
};

int ln_hid_client_init(void)
{
    int ret = 0;
    ret = ln_app_msg_regester(app_hid_client_msg_tab, ARRAY_LEN(app_hid_client_msg_tab));

    return ret;
}

#endif
