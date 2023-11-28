/**
 * @file        wifi_sta.c
 * @brief
 * @author      Jimmy (ccolacat@163.com)
 * @version     v1.0.0
 * @date        2023-07-20
 * 
 * @copyright   Copyright (c) 2020-2023 Shenzhen Ai-Thinker Technology Co., Ltd. All Rights Reserved.
 * 
 * Disclaimer: This program is for informational purposes only, please keep author information and source code license.
 * 
 */
#include "aiio_adapter_include.h"

#include "wifi_sta.h"
#include "main.h"

#include "aiio_cloud_receive.h"
#include "aiio_common.h"

static aiio_wifi_info_t wifi_info = {0};                                    /*!< Information of wifi*/
static wifi_config_data_t wifi_config_data = {
    
};                             /*!< The data of ble distribution network */


QueueHandle_t    cloud_rev_queue_handle;        /*!< The receive queue data handle*/                           
bool ble_config_start;                       /*!< The launch status in ble distribution network*/
bool wifi_config_start;
bool wifi_connect;
aiio_device_info DeviceInfo = {
    .deviceId = "JwJLrOhJVdAdft",
    .userName = "cd4KaH",
    .userPasswd = "pbE2NcR0GmeDMz",
};
bool device_init;

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS
#define EXAMPLE_AXK_WIFI_STA_CHANNEL            CONFIG_AXK_WIFI_STA_CHANNEL
aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};

static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
    int32_t rssi = 0;
    int32_t state = 0;
    uint8_t mac[MAC_LEN];

    int32_t ret=0;
    // uint8_t bssid[6] = {0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};
    switch (event->code) {
        case AIIO_WIFI_EVENT_WIFI_READY:
            aiio_log_d("<<<<<<<<<  WIFI INIT OK <<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_SCAN_DONE:
            aiio_log_d("<<<<<<<<<  SCAN DONE OK <<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_STA_CONNECTING:
            aiio_log_d("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_STA_CONNECTED:
            aiio_log_d("<<<<<<<<< CONNECTED<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_STA_DISCONNECTED:
            aiio_log_d("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
            if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE ||
                    event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE){
                //connect timeout
                aiio_log_d("connect timeout");
            }else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                    event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                    event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION){
                //password error
                aiio_log_d("password error");
            }else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL){
                //not found AP
                aiio_log_d("not found AP");
            }else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH)){
                //wifi disconnect
                aiio_log_d("wifi disconnect");
            }else{
                //connect error
                aiio_log_d("connect error");
            }
            break;
        case AIIO_WIFI_EVENT_STA_GOT_IP:
            aiio_log_d("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");
            aiio_wifi_ip_params_t wifi_sta_ip_praram = {0};
            
            aiio_wifi_rssi_get(&rssi);
            aiio_log_d("wifi cur_rssi = %d!!",rssi);
            aiio_wifi_sta_mac_get(mac);
            aiio_log_d("wifi mac = %02x%02x%02x%02x%02x%02x!!", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            aiio_wifi_sta_connect_ind_stat_get(&wifi_ind_stat);
            aiio_log_d("wifi sta_bssid = %02x%02x%02x%02x%02x%02x!!", wifi_ind_stat.bssid[0], wifi_ind_stat.bssid[1], wifi_ind_stat.bssid[2], wifi_ind_stat.bssid[3], wifi_ind_stat.bssid[4], wifi_ind_stat.bssid[5]);

            {
                aiio_wifi_sta_ip_get(&wifi_sta_ip_praram.ip, &wifi_sta_ip_praram.gateway, &wifi_sta_ip_praram.netmask);
                aiio_log_d("wifi ip = 0x%08x \r\n", wifi_sta_ip_praram.ip);
                aiio_log_d("wifi gateway = 0x%08x \r\n", wifi_sta_ip_praram.gateway);
                aiio_log_d("wifi netmask = 0x%08x \r\n", wifi_sta_ip_praram.netmask);
                aiio_log_d("wifi ip = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.ip));
                aiio_log_d("wifi gateway = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.gateway));
                aiio_log_d("wifi netmask = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.netmask));
            }

            wifi_info.ssid = malloc(strlen(wifi_ind_stat.ssid) + 1);
            if (wifi_info.ssid)
            {
                memset(wifi_info.ssid, 0, strlen(wifi_ind_stat.ssid) + 1);
                memcpy(wifi_info.ssid, wifi_ind_stat.ssid, strlen(wifi_ind_stat.ssid));
            }

            wifi_info.bssid = malloc(sizeof(wifi_ind_stat.bssid) * 2 + 1);
            if (wifi_info.bssid)
            {
                memset(wifi_info.bssid, 0, sizeof(wifi_ind_stat.bssid) * 2 + 1);
                snprintf(wifi_info.bssid, sizeof(wifi_ind_stat.bssid) * 2 + 1, "%02x%02x%02x%02x%02x%02x",
                         wifi_ind_stat.bssid[0], wifi_ind_stat.bssid[1], wifi_ind_stat.bssid[2], wifi_ind_stat.bssid[3], wifi_ind_stat.bssid[4], wifi_ind_stat.bssid[5]);
            }

            wifi_info.ip = malloc(strlen(ip4addr_ntoa(&wifi_sta_ip_praram.ip)) + 1);
            if(wifi_info.ip)
            {
                memset(wifi_info.ip , 0, strlen(ip4addr_ntoa(&wifi_sta_ip_praram.ip)) + 1);
                memcpy(wifi_info.ip, ip4addr_ntoa(&wifi_sta_ip_praram.ip), strlen(ip4addr_ntoa(&wifi_sta_ip_praram.ip)));
            }

            wifi_info.mask = malloc(strlen(ip4addr_ntoa(&wifi_sta_ip_praram.netmask)) + 1);
            if(wifi_info.mask)
            {
                memset(wifi_info.mask , 0, strlen(ip4addr_ntoa(&wifi_sta_ip_praram.netmask)) + 1);
                memcpy(wifi_info.mask, ip4addr_ntoa(&wifi_sta_ip_praram.netmask), strlen(ip4addr_ntoa(&wifi_sta_ip_praram.netmask)));
            }


            wifi_info.gw = malloc(strlen(ip4addr_ntoa(&wifi_sta_ip_praram.gateway)) + 1);
            if(wifi_info.gw)
            {
                memset(wifi_info.gw , 0, strlen(ip4addr_ntoa(&wifi_sta_ip_praram.gateway)) + 1);
                memcpy(wifi_info.gw, ip4addr_ntoa(&wifi_sta_ip_praram.gateway), strlen(ip4addr_ntoa(&wifi_sta_ip_praram.gateway)));
            }

            user_info.wifi_sta=1;
            
            break;
        case AIIO_WIFI_EVENT_AP_START:
            aiio_log_d("<<<<<<<<< start soft ap OK<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STOP:
            aiio_log_d("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STACONNECTED:
            aiio_log_d("<<<<<<<<< station connent ap <<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
            aiio_log_d("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
            break;
        default:
            break;
    }
}



void user_sta_wifi_entry(void)
{
    
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = EXAMPLE_AXK_WIFI_STA_SSID,
            .password = EXAMPLE_AXK_WIFI_STA_PASS,
            .channel = EXAMPLE_AXK_WIFI_STA_CHANNEL,
            .use_dhcp=1,
        },
    };
    #if STA_USE_STATIC_IP
        aiio_wifi_sta_ip_set("192.168.31.240");
        wifi_sta_config.sta.use_dhcp=0;
    #endif
    // memcpy(wifi_sta_config.sta.bssid,bssid,sizeof(bssid));

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();

}

static const char mqtt_ip[]="47.113.126.37";
static const char mqtt_token[]="f50497b3303711ee82ba00163e128231";

void test_cloud_function(void *params)
{

    aiio_log_i("cloud success\r\n");
    while (1)
    {
        if(user_info.wifi_sta)
        {
                user_info.wifi_sta = 0;
                aiio_log_d("REV_CLOUD_CONNECTED_EVENT \r\n");
                aiio_cloud_receive_t cloud_data = {0};
                // cloud_data.mqtt_host = wifi_config_data.mqttip;
                // cloud_data.device_token = wifi_config_data.token;
                // cloud_data.mqtt_port = wifi_config_data.port;
                cloud_data.mqtt_host = mqtt_ip;
                cloud_data.device_token = mqtt_token;
                cloud_data.mqtt_port = 1883;
                cloud_data.wifi_info = &wifi_info;

                aiio_user_service_init(true, &cloud_data);
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(100));
    }

}
void test_cloud_fun(void)
{
    static aiio_os_thread_handle_t *aiio_cloud_thread = NULL;

    aiio_os_thread_create(&aiio_cloud_thread, "test_cloud", test_cloud_function, 4096, NULL, 15);
    aiio_log_i("create lvgl cloud!");
}

int user_wifi_sta_mode_init(void)
{
    aiio_wifi_register_event_cb(cb_wifi_event);

    int ret = aiio_wifi_init();
    if (ret != 0)
    {
            aiio_log_e("wifi init error!!");
            return 1;
    }
    test_cloud_fun();
    return 0;
}

/************************ Copyright (c) 2023 Shenzhen Ai-Thinker Technology Co., Ltd. ************************/
