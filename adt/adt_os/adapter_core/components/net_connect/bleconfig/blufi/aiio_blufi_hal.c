/** @brief      blufi hal.
 *
 *  @file       aiio_blufi_hal.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include "aiio_blufi_export_import.h"

#include <stdio.h>
#include <stdlib.h>

#include "aiio_log.h"

static aiio_ble_default_handle_t srv_handle;
static aiio_ble_default_server_t blufi_server_tmp = {
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xFF, 0xFF),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xFF,0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xFF,0x01),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0,0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0,0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0,0x03),
};

static aiio_ble_conn_t g_blufi_conn;

static void ble_reverse_byte(uint8_t *arr, uint32_t size)
{
    uint8_t i, tmp;

    for (i = 0; i < size / 2; i++) {
        tmp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = tmp;
    }
}

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    static uint8_t peer_addr[6] = {0};

    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");
            aiio_ble_blufi_gap_disconnect_export(peer_addr);
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");
            g_blufi_conn = event->connect.conn;
            aiio_ble_blufi_gap_connect_export((void *)g_blufi_conn, peer_addr);
            break;
        case AIIO_BLE_EVT_MTU:
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            aiio_ble_blufi_gap_mtu_change_export(event->mtu.mtu);
            break;
        case AIIO_BLE_EVT_DATA:
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);
            aiio_ble_blufi_gatt_write_export(event->data.data, event->data.length);
            break;
        default:
            aiio_log_w("undef event!");
    }
}

int32_t aiio_hal_blufi_init(void)
{
    #define BLE_MIN_INTV    200
    #define BLE_MAX_INTV    220
    #define BLE_LATENCY     0
    #define BLE_TIMEOUT     400
    static aiio_ble_config_t ble_config = {
        .conn_intv = {
            .min_interval = BLE_MIN_INTV,
            .max_interval = BLE_MAX_INTV,
            .latency = BLE_LATENCY,
            .timeout = BLE_TIMEOUT,
        },
    };

    aiio_ble_register_event_cb(aiio_ble_cb);
    return aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
}

void aiio_hal_blufi_deinit(void)
{
    aiio_ble_deinit();

    return ;
}

int32_t aiio_hal_blufi_server_create_start(void)
{
    return ble_gatts_add_default_svcs(&blufi_server_tmp, &srv_handle);
}

int32_t aiio_hal_ble_gatt_blufi_notify_send(uint8_t *buf, uint16_t len)
{
    aiio_ble_send_t send_param;

    send_param.conn = g_blufi_conn;
    send_param.handle = srv_handle.tx_char_handle[0];
    send_param.type = AIIO_BLE_NOTIFY;
    send_param.data = buf;
    send_param.length = len;
    return aiio_ble_send(&send_param);
}

int32_t aiio_hal_blufi_gap_disconnect(void)
{
    /* reserve interface */
    return AIIO_OK;
}

int32_t aiio_hal_ble_adv_start(at_ble_adv_data_t *data)
{
    aiio_ble_adv_param_t param = {
        .conn_mode = AIIO_BLE_CONN_MODE_UND,
        /* BL602 unsupport */
        // .disc_mode = AIIO_BLE_DISC_MODE_NON,
        .disc_mode = AIIO_BLE_DISC_MODE_GEN,
        .interval_min = 200,
        .interval_max = 200,
    };

    return aiio_ble_adv_start(&param, data->data, data->data_len, data->rsp_data, data->rsp_data_len);
}

int32_t aiio_hal_ble_adv_stop(void)
{
    return aiio_ble_adv_stop();
}
