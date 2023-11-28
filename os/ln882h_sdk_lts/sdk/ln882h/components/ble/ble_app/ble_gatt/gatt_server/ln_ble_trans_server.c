/**
 ****************************************************************************************
 *
 * @file ln_ble_trans_server.c
 *
 * @brief gatt data trans server
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

#include "utils/debug/log.h"

#include "ln_ble_trans_server.h"
#include "ln_ble_gatt.h"
#include "ln_ble_event_manager.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_defines.h"
#include "ln_ble_app_err_code.h"


#if (BLE_DATA_TRANS_SERVER)

/*
 * Configuration Flag, each bit matches with an attribute of
 * att_db (Max: 32 attributes); if the bit is set to 1, the
 * attribute will be added in the service
*/
#define DATA_TRANS_CONFIG_MASK         (0x1FFF)

static data_trans_svr_t data_trans_svr = {0};

int ln_ble_trans_svr_add(ln_trans_svr_desc_t *param)
{
    uint16_t start_handle = 0;
    uint32_t cfg_flag = DATA_TRANS_CONFIG_MASK;
    uint8_t status = ATT_ERR_NO_ERROR;

    if(ATT_UUID_16_LEN == param->svr_uuid_len)
    {
        uint16_t data_trans_svc_uuid = (param->svr_uuid[1]<<8) | param->svr_uuid[0];
        struct attm_desc att_desc_tmp[param->att_count];

        for(int i = 0;i < param->att_count; i++)
        {
            att_desc_tmp[i].uuid        = (param->att_desc[i].uuid[1]<<8) | param->att_desc[i].uuid[0];
            att_desc_tmp[i].perm        = param->att_desc[i].perm;
            att_desc_tmp[i].ext_perm    = param->att_desc[i].ext_perm;
            att_desc_tmp[i].max_size    = param->att_desc[i].max_size;
        }

        status = attm_svc_create_db(&start_handle, data_trans_svc_uuid, (uint8_t *)&cfg_flag,
            param->att_count, NULL, TASK_APP, (const struct attm_desc *)att_desc_tmp,
            (SVC_PREM_TYPE_UUID16_MASK<<5));
    }
    else
    {
        uint8_t data_trans_svc_uuid[ATT_UUID_128_LEN] = {0};
        memcpy(data_trans_svc_uuid, param->svr_uuid, param->svr_uuid_len);

        status = attm_svc_create_db_128(&start_handle, data_trans_svc_uuid, (uint8_t *)&cfg_flag,
            param->att_count, NULL, TASK_APP, (const struct attm_desc_128 *)param->att_desc,
            (SVC_PREM_TYPE_UUID128_MASK<<5));
    }

    if(ATT_ERR_NO_ERROR != status)
    {
        LOG(LOG_LVL_DEBUG, "data_trans_svc_add err!!!\r\n");
        return BLE_ERR_INVALID_PARAM;
    }
    else
    {
        param->start_handle = start_handle;
        data_trans_svr.svr_count++;
        LOG(LOG_LVL_DEBUG, "server start handle %d\r\n", start_handle);
    }

    return BLE_ERR_NONE;
}

void ln_ble_trans_svr_ntf(ln_trans_svr_send_t *param)
{
    ln_gatt_send_evt_cmd_t send_data;

    send_data.handle = param->hdl;
    send_data.length = param->len;
    send_data.value  = param->data;
    ln_gatt_send_ntf(param->conn_idx, &send_data);

    LOG(LOG_LVL_TRACE, "ln_ble_trans_svr_notify %d\r\n", send_data.length);
}

void ln_ble_trans_svr_ind(ln_trans_svr_send_t *param)
{
    ln_gatt_send_evt_cmd_t send_data;

    send_data.handle = param->hdl;
    send_data.length = param->len;
    send_data.value  = param->data;
    ln_gatt_send_ind(param->conn_idx, &send_data);

    LOG(LOG_LVL_TRACE, "ln_ble_trans_svr_ind %d\r\n", send_data.length);
}

int ln_ble_trans_svr_init(void)
{
    memset(&data_trans_svr, 0, sizeof(data_trans_svr_t));

    return BLE_ERR_NONE;
}

#endif

