/** @brief      blufi_api.
 *
 *  @file       aiio_blufi_api.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include <stdlib.h>
#include "aiio_blufi_api.h"
#include "blufi_int.h"
#include "aiio_btc.h"
#include "blufi_prf.h"

int32_t aiio_blufi_register_callbacks(aiio_blufi_callbacks_t *callbacks)
{
    if (callbacks == NULL) {
        return AIIO_ERROR;
    }
    btc_blufi_set_callbacks(callbacks);
    return btc_profile_cb_set(callbacks->event_cb);
}

int32_t aiio_blufi_profile_init(void)
{
    btc_msg_t msg;
    msg.sig = BTC_SIG_API_CALL;
    msg.act = BTC_BLUFI_ACT_INIT;
    return btc_transfer_context(&msg, NULL, 0, NULL);

}


int32_t aiio_blufi_profile_deinit(void)
{
    btc_msg_t msg;
    msg.sig = BTC_SIG_API_CALL;
    msg.act = BTC_BLUFI_ACT_DEINIT;

    return btc_transfer_context(&msg, NULL, 0, NULL);
}

int32_t aiio_blufi_send_wifi_conn_report(aiio_wifi_mode_t opmode, aiio_blufi_sta_conn_state_t sta_conn_state, uint8_t softap_conn_num, aiio_blufi_extra_info_t *extra_info)
{
    btc_msg_t msg;
    btc_blufi_args_t arg;

    msg.sig = BTC_SIG_API_CALL;
    msg.act = BTC_BLUFI_ACT_SEND_CFG_REPORT;
    arg.wifi_conn_report.opmode = opmode;
    arg.wifi_conn_report.sta_conn_state = sta_conn_state;
    arg.wifi_conn_report.softap_conn_num = softap_conn_num;
    arg.wifi_conn_report.extra_info = extra_info;

    return btc_transfer_context(&msg, &arg, sizeof(btc_blufi_args_t), btc_blufi_call_deep_copy);
}


int32_t aiio_blufi_send_error_info(aiio_blufi_error_state_t state)
{
    btc_msg_t msg;
    btc_blufi_args_t arg;
    msg.sig = BTC_SIG_API_CALL;
    msg.act = BTC_BLUFI_ACT_SEND_ERR_INFO;
    arg.blufi_err_infor.state = state;
    return btc_transfer_context(&msg, &arg, sizeof(btc_blufi_args_t), NULL);
}


int32_t aiio_blufi_send_custom_data(uint8_t *data, uint32_t data_len)
{
    btc_msg_t msg;
    btc_blufi_args_t arg;
    if(data == NULL || data_len == 0) {
        return AIIO_ERROR;
    }

    msg.sig = BTC_SIG_API_CALL;
    msg.act = BTC_BLUFI_ACT_SEND_CUSTOM_DATA;
    arg.custom_data.data = data;
    arg.custom_data.data_len = data_len;

    return btc_transfer_context(&msg, &arg, sizeof(btc_blufi_args_t), btc_blufi_call_deep_copy);

}