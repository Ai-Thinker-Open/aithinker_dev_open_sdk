/** @brief      blufi main.
 *
 *  @file       blufi_main.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */
#include <string.h>
#include "aiio_blufi_api.h"
#include "aiio_blufi.h"
#include "blufi_int.h"
#include "aiio_blufi_export_import.h"


#include "aiio_wifi.h"
#include "aiio_ble.h"
#include "aiio_blufi.h"

aiio_wifi_sta_config_t aiio_blufi_sta_config={0};

typedef struct
{
    char ssid[32 + 1];        /**< the SSID of the target AP. */
    uint8_t bssid[6];         /**< the MAC address of the target AP */
    uint8_t channel;          /**< channel */
    int8_t rssi;              /**< signal strength. */
    uint8_t pci_en;           /**< PCI Authentication */
    uint16_t reconn_interval; /**< the interval between Wi-Fi reconnections. Unit: second. Default: 1. Maximum: 7200. */
    uint16_t listen_interval; /**< the interval of listening to the AP’s beacon. Unit: AP beacon intervals. Default: 3. Range: [1,100]. */
    uint8_t scan_mode;        /**< 0: fast scan. 1: all-channel scan */
    uint8_t pmf;              /**< Protected Management Frames. Default: 0. */
} aiio_wjap_response_param_t;


uint8_t g_blufi_init_flag=0;
static bool ble_is_connected = false;
static bool gl_sta_connected = false;

static aiio_blufi_cb_t blufi_cb = {0};

int32_t aiio_hal_wifi_conn_ap_info_get(aiio_wjap_response_param_t *cwjap_responseparam)
{
    aiio_wifi_sta_connect_ind_stat_info_t wifi_info;

    aiio_wifi_sta_connect_ind_stat_get(&wifi_info);

    strcpy(cwjap_responseparam->ssid, wifi_info.ssid);
    memcpy(cwjap_responseparam->bssid, wifi_info.bssid, 6);
    aiio_wifi_channel_get(&(cwjap_responseparam->channel));
    aiio_wifi_rssi_get(&(cwjap_responseparam->rssi));
    cwjap_responseparam->pci_en = 0;
    // cwjap_responseparam->reconn_interval
    // cwjap_responseparam->listen_interval
    // cwjap_responseparam->scan_mode
    cwjap_responseparam->pmf = 0;

    return AIIO_OK;
}

int32_t aiio_blufi_register_cb(aiio_blufi_cb_t cb)
{
    blufi_cb.wifi_event_cb = cb.wifi_event_cb;
    blufi_cb.ble_event_cb = cb.ble_event_cb;
    return AIIO_OK;
}

void aiio_blufi_wifi_event(int32_t evt_code, void *param)
{
    if(g_blufi_init_flag){
        switch (evt_code)
        {
            case AIIO_WIFI_EVENT_STA_CONNECTED:
                gl_sta_connected = true;
                break;
            case AIIO_WIFI_EVENT_STA_DISCONNECTED:
                gl_sta_connected = false;
                break;
            case AIIO_WIFI_EVENT_STA_GOT_IP:
            {
                aiio_wifi_mode_t mode;
                aiio_blufi_extra_info_t info;
                mode=AIIO_WIFI_MODE_STA;

                aiio_wjap_response_param_t cwjap_responseparam = {0};
                memset(&info, 0, sizeof(aiio_blufi_extra_info_t));
                aiio_hal_wifi_conn_ap_info_get(&cwjap_responseparam);
                memcpy(info.sta_bssid, cwjap_responseparam.bssid, 6);

                info.sta_bssid_set = true;
                info.sta_ssid = (uint8_t *)aiio_blufi_sta_config.ssid;
                info.sta_ssid_len = strlen((char *)aiio_blufi_sta_config.ssid);
                info.sta_passwd=(uint8_t *)aiio_blufi_sta_config.password;
                if (info.sta_passwd) {
                    info.sta_passwd_len = strlen((char *)aiio_blufi_sta_config.password);
                }

                if (ble_is_connected == true)
                {
                    aiio_blufi_send_wifi_conn_report(mode, AIIO_BLUFI_STA_CONN_SUCCESS, 0, &info);
                }
                else
                {
                    aiio_log_i("BLUFI BLE is not connected yet\r\n");
                }

                if (blufi_cb.wifi_event_cb)
                {
                    blufi_cb.wifi_event_cb((char *)info.sta_ssid, (char *)info.sta_passwd);
                }
            }
            break;
        default:
            aiio_log_d("[SC] Unknown code %u\r\n", evt_code);
            break;
        }
    }   
}

static void example_event_callback(aiio_blufi_cb_event_t event, aiio_blufi_cb_param_t *param)
{
    /* actually, should post to blufi_task handle the procedure,
     * now, as a example, we do it more simply */
    if (blufi_cb.ble_event_cb)
    {
        blufi_cb.ble_event_cb(event, param);
    }

    switch (event)
    {
    case AIIO_BLUFI_EVENT_INIT_FINISH:
        aiio_log_i("BLUFI init finish\n");

        aiio_blufi_adv_start();
        break;
    case AIIO_BLUFI_EVENT_DEINIT_FINISH:
        aiio_log_i("BLUFI deinit finish\n");
        break;
    case AIIO_BLUFI_EVENT_BLE_CONNECT:
        aiio_log_i("BLUFI ble connect\n");
        ble_is_connected = true;
        aiio_blufi_adv_stop();
        // blufi_security_init();
        break;
    case AIIO_BLUFI_EVENT_BLE_DISCONNECT:
        aiio_log_i("BLUFI ble disconnect\n");
        ble_is_connected = false;
        //    blufi_security_deinit();
        if(g_blufi_init_flag != 0){
            aiio_blufi_adv_start();
        }
        break;
    case AIIO_BLUFI_EVENT_SET_WIFI_OPMODE:
        aiio_log_i("BLUFI Set WIFI opmode %d\n", param->wifi_mode.op_mode);
        // if (axk_hal_wifi_mode_set(AT_WIFIMODE_STA, 0) != AT_ERR_SUCCESS)
        // {
        //     aiio_log_e("BLUFI axk_hal_wifi_mode_set fail\r\n");
        //     break;
        // }
        // g_at_cmd_config.wifi.cwmode = AT_WIFIMODE_STA;
        break;
    case AIIO_BLUFI_EVENT_REQ_CONNECT_TO_AP:
    {
        aiio_log_i("BLUFI requset wifi connect to AP\n");
        
        aiio_wifi_disconnect();
        
        aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

        aiio_wifi_config_t wifi_sta_config = {
            .sta = {
                .channel = 0,
                .use_dhcp=1,
                .flags=0,
            },
        };
        memcpy(wifi_sta_config.sta.ssid,aiio_blufi_sta_config.ssid,sizeof(aiio_blufi_sta_config.ssid));
        memcpy(wifi_sta_config.sta.password,aiio_blufi_sta_config.password,sizeof(aiio_blufi_sta_config.password));
        memcpy(wifi_sta_config.sta.bssid,aiio_blufi_sta_config.bssid,sizeof(aiio_blufi_sta_config.bssid));

        aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

        if (aiio_wifi_start() != AIIO_OK)
        {
            aiio_log_i("BLUFI axk_hal_conn_ap_info_set fail\r\n");
            break;
        }
        // g_at_cmd_config.wifi.sta.state = AT_WIFI_STATE_CONNECTING;
    }

    break;
    case AIIO_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
        aiio_log_i("BLUFI requset wifi disconnect from AP\n");
        aiio_wifi_disconnect();
        break;
    case AIIO_BLUFI_EVENT_REPORT_ERROR:
        aiio_log_e("BLUFI report error, error code %d\n", param->report_error.state);
        aiio_blufi_send_error_info(param->report_error.state);
        break;
    case AIIO_BLUFI_EVENT_GET_WIFI_STATUS:
    {
        aiio_wifi_mode_t mode;
        aiio_blufi_extra_info_t info;
        mode = AIIO_WIFI_MODE_STA;

        if (gl_sta_connected)
        {
            aiio_wjap_response_param_t cwjap_responseparam = {0};
            memset(&info, 0, sizeof(aiio_blufi_extra_info_t));
            aiio_hal_wifi_conn_ap_info_get(&cwjap_responseparam);
            memcpy(info.sta_bssid, cwjap_responseparam.bssid, 6);
            info.sta_bssid_set = true;
            info.sta_ssid = (uint8_t *)aiio_blufi_sta_config.ssid;
            info.sta_ssid_len = strlen((char *)aiio_blufi_sta_config.ssid);
            aiio_blufi_send_wifi_conn_report(mode, AIIO_BLUFI_STA_CONN_SUCCESS, 0, &info);
        }
        else
        {
            aiio_blufi_send_wifi_conn_report(mode, AIIO_BLUFI_STA_CONN_FAIL, 0, NULL);
        }
        aiio_log_i("BLUFI get wifi status from AP\n");

        break;
    }
    case AIIO_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
        aiio_log_i("blufi close a gatt connection\r\n");
        aiio_blufi_disconnect();
        break;
    case AIIO_BLUFI_EVENT_DEAUTHENTICATE_STA:
        /* TODO */
        break;
    case AIIO_BLUFI_EVENT_RECV_STA_BSSID:
        memset(aiio_blufi_sta_config.bssid, 0, 6);
        memcpy((char *)aiio_blufi_sta_config.bssid, param->sta_bssid.bssid, 6);
        // sta_config.sta.bssid_set = 1;
        // aiio_wifi_set_config(WIFI_IF_STA, &sta_config);
        aiio_log_i("Recv STA BSSID %s\r\n", param->sta_bssid.bssid);
        break;
    case AIIO_BLUFI_EVENT_RECV_STA_SSID:
        memset(aiio_blufi_sta_config.ssid, 0, 33);
        strncpy((char *)aiio_blufi_sta_config.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);

        aiio_log_i("Recv STA SSID %s\r\n", (char *)aiio_blufi_sta_config.ssid);
        break;
    case AIIO_BLUFI_EVENT_RECV_STA_PASSWD:
        memset(aiio_blufi_sta_config.password, 0, 64);
        strncpy((char *)aiio_blufi_sta_config.password, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
        aiio_log_i("Recv STA PASSWORD %s\r\n", (char *)aiio_blufi_sta_config.password);
        break;
    case AIIO_BLUFI_EVENT_RECV_SOFTAP_SSID:
        break;
    case AIIO_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
        break;
    case AIIO_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
        break;
    case AIIO_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
        break;
    case AIIO_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
        break;
    case AIIO_BLUFI_EVENT_GET_WIFI_LIST:
        break;
    case AIIO_BLUFI_EVENT_RECV_CUSTOM_DATA:
        aiio_log_i("Recv Custom Data %d\n", param->custom_data.data_len);
        // aiio_log_buffer_hex("Custom Data", param->custom_data.data, param->custom_data.data_len);
        break;
    case AIIO_BLUFI_EVENT_RECV_USERNAME:
        /* Not handle currently */
        break;
    case AIIO_BLUFI_EVENT_RECV_CA_CERT:
        /* Not handle currently */
        break;
    case AIIO_BLUFI_EVENT_RECV_CLIENT_CERT:
        /* Not handle currently */
        break;
    case AIIO_BLUFI_EVENT_RECV_SERVER_CERT:
        /* Not handle currently */
        break;
    case AIIO_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
        /* Not handle currently */
        break;
        ;
    case AIIO_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
        /* Not handle currently */
        break;
    default:
        break;
    }
}

static aiio_blufi_callbacks_t example_callbacks = {
    .event_cb = example_event_callback,
    // .negotiate_data_handler = blufi_dh_negotiate_data_handler,
    // .encrypt_func = blufi_aes_encrypt,
    // .decrypt_func = blufi_aes_decrypt,
    // .checksum_func = blufi_crc_checksum,
};

int32_t aiio_blufi_start(void)
{

    int32_t ret;
    aiio_hal_blufi_init();

    ret = aiio_blufi_host_and_cb_init(&example_callbacks);
    if (ret!=AIIO_OK)
    {
        aiio_log_e("%s initialise failed: %d\n", __func__, ret);
    }else{
        g_blufi_init_flag=1;
    }
    return ret;

}

void aiio_blufi_stop(void)
{
    aiio_blufi_profile_deinit();
    
    aiio_blufi_adv_stop();
    g_blufi_init_flag=0;
    // axk_hal_ble_role_set(AT_ROLE_DEINIT);
    aiio_hal_blufi_deinit();
}