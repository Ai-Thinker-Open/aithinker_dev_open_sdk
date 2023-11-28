/**
 ****************************************************************************************
 *
 * @file ln_ble_trans_client.c
 *
 * @brief gatt data trans client
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
#include "gattc_task.h"
#include "gattm_task.h"

#include "utils/debug/log.h"

#include "ln_ble_trans_client.h"
#include "ln_ble_gatt.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_err_code.h"


#if (BLE_DATA_TRANS_CLIENT)

static data_trans_cli_t data_trans_cli= {0};

void ln_ble_trans_disc_svc(uint8_t conidx, uint8_t *svc_uuid, uint8_t svc_uuid_len)
{
    ln_gatt_disc_cmd_t param_ds = {0};

    param_ds.operation = GATTC_DISC_BY_UUID_SVC;
    param_ds.start_hdl = 1;
    param_ds.end_hdl   = 0xFFFF;
    param_ds.uuid_len  = svc_uuid_len;
    param_ds.uuid = svc_uuid;
    ln_gatt_discovery(conidx, &param_ds);
}

void ln_ble_trans_cli_read(uint8_t conidx, uint16_t handle, uint16_t len)
{
    ln_gatt_read_cmd_t p_param;
    p_param.operation = GATTC_READ;
    p_param.nb = 0;
    p_param.req.simple.handle = handle;
    p_param.req.simple.offset = 0;
    p_param.req.simple.length = len;

    ln_gatt_read(conidx, &p_param);
}

void ln_ble_trans_cli_write_cmd(ln_trans_cli_send_t *param)
{
    ln_gatt_write_cmd_t p_param;
    p_param.operation = GATTC_WRITE_NO_RESPONSE;
    p_param.auto_execute = true;
    p_param.handle = param->hdl;
    p_param.offset = 0;
    p_param.length = param->len;
    p_param.value = param->data;

    ln_gatt_write(param->conn_idx, &p_param);
}

void ln_ble_trans_cli_write_req(ln_trans_cli_send_t *param)
{
    ln_gatt_write_cmd_t p_param;
    p_param.operation = GATTC_WRITE;
    p_param.auto_execute = true;
    p_param.handle = param->hdl;
    p_param.offset = 0;
    p_param.length = param->len;
    p_param.value = param->data;

    ln_gatt_write(param->conn_idx, &p_param);
}

int ln_ble_trans_cli_init(void)
{
    memset(&data_trans_cli, 0, sizeof(data_trans_cli_t));

    return BLE_ERR_NONE;
}

#endif

