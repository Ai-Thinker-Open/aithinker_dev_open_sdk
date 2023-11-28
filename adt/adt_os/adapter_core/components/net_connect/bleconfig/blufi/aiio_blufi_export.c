/** @brief      blufi_export.
 *
 *  @file       aiio_blufi_export.c
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
#include "aiio_blufi_api.h"
#include "aiio_btc.h"
#include "blufi_int.h"
#include "blufi_prf.h"

#include <string.h>

extern tBLUFI_ENV blufi_env;

int32_t aiio_ble_blufi_gap_connect_export(void *handle, uint8_t *peer_addr)
{
    blufi_env.is_connected = true;
    blufi_env.recv_seq = blufi_env.send_seq = 0;
    btc_msg_t msg;
    aiio_blufi_cb_param_t param;
    msg.sig = BTC_SIG_API_CB;
    msg.act = AIIO_BLUFI_EVENT_BLE_CONNECT;
    // param.connect.conn_id = event->connect.conn_handle;
    memcpy(param.connect.remote_bda, peer_addr, sizeof(aiio_bd_addr_t));
    btc_transfer_context(&msg, &param, sizeof(aiio_blufi_cb_param_t), NULL);
    return AIIO_OK;
}

int32_t aiio_ble_blufi_gap_disconnect_export(uint8_t *peer_addr)
{
    memcpy(blufi_env.remote_bda, peer_addr, sizeof(aiio_bd_addr_t));
    blufi_env.is_connected = false;
    blufi_env.recv_seq = blufi_env.send_seq = 0;
    blufi_env.sec_mode = 0x0;
    btc_msg_t msg;
    aiio_blufi_cb_param_t param;

    msg.sig = BTC_SIG_API_CB;
    msg.act = AIIO_BLUFI_EVENT_BLE_DISCONNECT;
    memcpy(param.disconnect.remote_bda, peer_addr, sizeof(aiio_bd_addr_t));
    btc_transfer_context(&msg, &param, sizeof(aiio_blufi_cb_param_t), NULL);
    return AIIO_OK;
}

int32_t aiio_ble_blufi_gap_mtu_change_export(uint16_t mtu)
{
    blufi_env.frag_size = (mtu < BLUFI_MAX_DATA_LEN ? mtu : BLUFI_MAX_DATA_LEN) - BLUFI_MTU_RESERVED_SIZE;
    return AIIO_OK;
}

int32_t aiio_ble_blufi_gatt_write_export(uint8_t *data, int32_t len)
{
    btc_blufi_recv_handler(data,len);
    return AIIO_OK;
}