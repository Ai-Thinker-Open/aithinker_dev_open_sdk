/**
 * @brief   The application function of cloud demo
 * 
 * @file    main.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        1. Device start running, Red, blue and green led triggle
 *              2. Device entering distribution network, green and red led triggle
 *              3. Device exitting distribution network, green, blue and red triggle.
 *              4. Device distribution network fail, red led triggle.
 *              5. Device connected wifi, green led turn on.
 *              6. Device disconnected wifi, green led triggle.
 *              7. Device conneced cloud, blue led turn on
 *              8. Device disconned cloud, blue led triggle.
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-29          <td>1.0.0            <td>zhuolm             <td>
 */
#include "aiio_adapter_include.h"
#include "aiio_common.h"
#include "config.h"
#include "aiio_lan_receive.h"
#include "aiio_at_receive.h"
#include "aiio_ap_config_wifi.h"
#include "aiio_ota_transmit.h"
#include "aiio_wdt.h"
#include "aiio_mcu_protocol.h"
#include "aiio_mcu_system.h"
#include "aiio_nvs.h"


// #include "easyflash.h"


#define QUEUE_MAX_SIZE                      (5)             /* max number of queue*/

#define WIFI_MAX_DISCONNECT_NUMBER          (10)

#define CLOUD_RECONNECT_WAIT_MAX_TIME           (10 * 1000)
#define WIFI_RECONNECT_WAIT_MAX_TIME           (20 * 1000)


static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;           /*!< handle of task thread*/
bool wifi_connect = false;                                           /*!< status of wifi connection*/

bool ble_config_start = false;                                       /*!< Launch state of ble distribution network*/
bool wifi_config_start = false;
bool ailinkInitstate = false;
static bool wifi_start_connected = false;
static uint8_t wifi_disconnect_count = 0;                                   /*!< The number  of wifi disconnection*/
static aiio_wifi_info_t wifi_info = {0};                                    /*!< Information of wifi*/
static wifi_config_data_t wifi_config_data = {0};                             /*!< The data of ble distribution network */
static AIIO_TIMER cloud_wait_connect_handle = -1;
static AIIO_TIMER wifi_reconnect_timer_handle = -1;
AIIO_TIMER wifi_watchdog_timer_handle = -1;
static bool     deviceIsLaunch = false;
static bool     deviceinfo_flag = false;


/* Define global variable */
QueueHandle_t    cloud_rev_queue_handle = NULL;                             /*!< handle of queue*/
aiio_device_info DeviceInfo = {0};
aiio_rtc_time_t UpTime = {0};

static uint32_t lastTick = 0;
static uint32_t CurTick = 0;
bool waterfall_light = false;
bool waterfall_light_state = false;
uint8_t waterfall_num = 0;

bool sta_enable_flag = false;

// bool mcu_ota_start_flag = false;

static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
    int32_t ret=0;
    aiio_rev_queue_t  rev_queue = {0};

    switch (event->code) 
    {
        case AIIO_WIFI_EVENT_WIFI_READY:
        {
            aiio_log_i("<<<<<<<<<  WIFI INIT OK <<<<<<<<<<");
            // iot_connect_wifi("ZLM_24G", "12345678");
        }
        break;
        
        case AIIO_WIFI_EVENT_SCAN_DONE:
        {
            aiio_log_i("<<<<<<<<<  SCAN DONE OK <<<<<<<<<<");
        }
        break;

        case AIIO_WIFI_EVENT_STA_CONNECTING:
        {
            aiio_log_i("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
        }
        break;

        case AIIO_WIFI_EVENT_STA_CONNECTED:
        {
            aiio_log_i("<<<<<<<<< CONNECTED<<<<<<<<<<<");
        }
        break;

        case AIIO_WIFI_EVENT_STA_DISCONNECTED:
        {
            aiio_log_i("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
            wifi_disconnect_count++;
            if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE ||
                    event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE)
            {
                //connect timeout
                aiio_log_i("connect timeout");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                if(ble_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);
                }
#endif
            }
            else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                    event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                    event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION)
            {
                //password error
                aiio_log_i("password error");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                if(ble_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    aiio_ble_config_response_status(AIIO_BLE_CODE_WIFI_PAWD_ERR);
                }
#endif
            }
            else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL)
            {
                //not found AP
                aiio_log_i("not found AP");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                if(ble_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    aiio_ble_config_response_status(AIIO_BLE_CODE_WIFI_NO_SSID);
                }
#endif
            }
            else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH))
            {
                //wifi disconnect
                aiio_log_i("wifi disconnect");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                if(ble_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);
                }
#endif
            }
            else
            {
                //connect error
                aiio_log_i("connect error");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                if(ble_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);
                }
#endif
            }

            if(wifi_config_start && wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
            {
                rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_i("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                wifi_disconnect_count = 0;
            }
            else
            {
                if(wifi_disconnect_count > WIFI_MAX_DISCONNECT_NUMBER)
                {
                    wifi_disconnect_count = 0;
                    rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
                    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                    {
                        aiio_log_i("queue send failed\r\n");
                    }
                    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                }
            }
            wifi_connect = false;
            wifi_start_connected = false;
            aiio_turn_on_led_status(ONLY_GREEN_LED_TRIGGLE_STATUS);

#ifdef CONFIG_LAN_NETWORK_MODULE
            aiio_user_lan_service_deinit();

#endif

        }
        break;
        case AIIO_WIFI_EVENT_STA_GOT_IP:
        {
            aiio_log_i("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            aiio_ble_config_response_status(AIIO_BLE_CODE_WIFI_CONN_OK);
#endif
            wifi_connect = true;
            aiio_turn_on_led_status(ONLY_GREEN_LED_TURN_ON);
            aiio_TimerStop(wifi_reconnect_timer_handle);

            aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};
            aiio_wifi_ip_params_t wifi_sta_ip_praram = {0};
            uint8_t mac[MAC_LEN];
            int32_t rssi=0;

            aiio_wifi_rssi_get(&rssi);
            uint8_t rssi1 = (uint8_t)(-rssi);
            aiio_log_d("wifi cur_rssi = %d!!",rssi1);
            aiio_send_mcu_cmd(GET_WIFI_RSSI_CMD, &rssi1, sizeof(rssi1));

            wifi_info.rssi = rssi;

            aiio_wifi_sta_mac_get(mac);
            aiio_log_d("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            aiio_wifi_sta_connect_ind_stat_get(&wifi_ind_stat);
            aiio_log_d("wifi ssid = %s \r\n", wifi_ind_stat.ssid);
            aiio_log_d("wifi passphr = %s \r\n", wifi_ind_stat.passphr);
            aiio_log_d("wifi sta_bssid = %02x%02x%02x%02x%02x%02x!!",wifi_ind_stat.bssid[0],wifi_ind_stat.bssid[1],wifi_ind_stat.bssid[2],wifi_ind_stat.bssid[3],wifi_ind_stat.bssid[4],wifi_ind_stat.bssid[5]);

            
            aiio_wifi_sta_ip_get(&wifi_sta_ip_praram.ip, &wifi_sta_ip_praram.gateway, &wifi_sta_ip_praram.netmask);
            aiio_log_d("wifi ip = 0x%08x \r\n", wifi_sta_ip_praram.ip);
            aiio_log_d("wifi gateway = 0x%08x \r\n", wifi_sta_ip_praram.gateway);
            aiio_log_d("wifi netmask = 0x%08x \r\n", wifi_sta_ip_praram.netmask);
            aiio_log_d("wifi ip = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.ip));
            aiio_log_d("wifi gateway = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.gateway));
            aiio_log_d("wifi netmask = %s \r\n", ip4addr_ntoa(&wifi_sta_ip_praram.netmask));

            wifi_info.ssid = malloc(strlen(wifi_ind_stat.ssid) + 1);
            if(wifi_info.ssid)
            {
                memset(wifi_info.ssid , 0, strlen(wifi_ind_stat.ssid) + 1);
                memcpy(wifi_info.ssid, wifi_ind_stat.ssid, strlen(wifi_ind_stat.ssid));
            }

            wifi_info.bssid = malloc(sizeof(wifi_ind_stat.bssid) * 2 + 1);
            if(wifi_info.bssid)
            {
                memset(wifi_info.bssid , 0, sizeof(wifi_ind_stat.bssid) * 2 + 1);
                snprintf(wifi_info.bssid, sizeof(wifi_ind_stat.bssid) * 2 + 1, "%02x%02x%02x%02x%02x%02x", 
                        wifi_ind_stat.bssid[0],wifi_ind_stat.bssid[1],wifi_ind_stat.bssid[2],wifi_ind_stat.bssid[3],wifi_ind_stat.bssid[4],wifi_ind_stat.bssid[5]);
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

            
            rev_queue.common_event = REV_CLOUD_CONNECTED_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            wifi_disconnect_count = 0;
            wifi_start_connected = false;
        }
        break;

        case AIIO_WIFI_EVENT_AP_START:
        {
            aiio_log_i("<<<<<<<<< start soft ap OK<<<<<<<<<<<");
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
            aiio_ap_config_notify(AIIO_WIFI_SOFT_AP_START_EVENT);
#endif
        }
        break;

        case AIIO_WIFI_EVENT_AP_STOP:
        {
            aiio_log_i("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
            aiio_ap_config_notify(AIIO_WIFI_SOFT_AP_STOP_EVENT);
#endif
        }
        break;

        case AIIO_WIFI_EVENT_AP_STACONNECTED:
        {
            aiio_log_i("<<<<<<<<< station connent ap <<<<<<<<<<<");
        }
        break;
            
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
        {
            aiio_log_i("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
        }
        break;

        default:
            break;
    }
}


void iot_connect_wifi(char *ssid, char *passwd)
{
    if(ssid == NULL)
    {
        aiio_log_e("ssid is NULL \r\n");
        return ;
    }

    aiio_wifi_config_t wifi_sta_config = {0};

    memcpy(wifi_sta_config.sta.ssid, ssid, strlen(ssid));
    if(passwd)
    {
        memcpy(wifi_sta_config.sta.password, passwd, strlen(passwd));
    }
    wifi_sta_config.sta.channel = 0;
    wifi_sta_config.sta.use_dhcp = 1;
    wifi_sta_config.sta.flags = 0;

    aiio_log_i("ssid = %s \r\n", wifi_sta_config.sta.ssid);
    aiio_log_i("password = %s \r\n", wifi_sta_config.sta.password);
    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);
    printf("sta_enable_flag is %d\r\n",sta_enable_flag);
    if(!sta_enable_flag){
        aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);
        aiio_wifi_start();
        sta_enable_flag = true;
    }else{
        aiio_wifi_connect();
    }
    
    wifi_disconnect_count = 0;
    wifi_start_connected = true;
}

static void test_sta_wifi_entry(void *pvParameters)
{
    int msg_id;
    aiio_rev_queue_t  rev_queue = {0};

    while (1)
    {

        if(xQueueReceive(cloud_rev_queue_handle, &rev_queue, 0) == pdPASS)
        {
            switch (rev_queue.common_event)
            {
                case REV_CONFIG_DATA_EVENT:
                {
                    SetWifiNetWorkState(WIFI_NOT_CONNECTED);
                    aiio_log_d("REV_CONFIG_DATA_EVENT \r\n");
                    // aiio_turn_on_led_status(GREEN_RED_LED_TURN_ON);
                    memcpy(&wifi_config_data, rev_queue.queue_data, rev_queue.queue_data_len);
                    aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
                    aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
                    aiio_log_d("mqttip = %s \r\n", wifi_config_data.mqttip);
                    aiio_log_d("token = %s \r\n", wifi_config_data.token);
                    aiio_log_d("time zone = %s \r\n", wifi_config_data.tz);
                    aiio_update_rtc_time_with_zone(wifi_config_data.tz, strlen(wifi_config_data.tz));
                    AilinkSavePairInfo(&wifi_config_data);
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_stop();
#endif
                    rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
                    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                    {
                        aiio_log_i("queue send failed\r\n");
                    }
                    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                }
                break;

                case REV_CONFIG_START_EVENT:
                {
                    aiio_log_d("REV_CONFIG_START_EVENT \r\n");
                    if(wifi_connect)
                    {
#ifdef CONFIG_WAN_NETWORK_MODULE
                        aiio_user_service_deinit();
#endif
#ifdef CONFIG_LAN_NETWORK_MODULE
                        aiio_user_lan_service_deinit();

#endif
                        aiio_wifi_disconnect();
                    }
                    else if(wifi_start_connected)
                    {
                        aiio_wifi_disconnect();
                    }

                    aiio_os_tick_dealy(aiio_os_ms2tick(50));
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                    aiio_ble_config_start();
                    ble_config_start = true;
                    SetWifiNetWorkState(BLE_CONFIG_STATE);
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_start();
                    SetWifiNetWorkState(AP_CONFIG_STATE);
#endif
                    aiio_TimerStop(cloud_wait_connect_handle);
                    
                    wifi_config_start = true;
                    aiio_turn_on_led_status(GREEN_RED_LED_TRIGGLE_STATUS);
                }
                break;

                case REV_CONFIG_STOP_EVENT:
                {
                    if(wifi_config_start)
                    {
                        aiio_log_d("REV_CONFIG_STOP_EVENT \r\n");
                        

#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                        aiio_ble_config_stop();
                        ble_config_start = false;
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                        aiio_ap_config_stop();
#endif
                        wifi_config_start = false;

                        if(AilinkLoadPairInfo(&wifi_config_data) > 0)
                        {
                            aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
                            aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
                            iot_connect_wifi(wifi_config_data.ssid, wifi_config_data.passwd);
                        }

                        aiio_turn_on_led_status(ALL_LED_TRIGGLE_STATUS);
                    }
                }
                break;

                case REV_CONFIG_OK_EVENT:
                {
                    aiio_log_d("REV_CONFIG_OK_EVENT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                    aiio_ble_config_response_status(AIIO_BLE_CODE_OK);
                    aiio_os_tick_dealy(aiio_os_ms2tick(2000));
                    aiio_ble_config_stop();
                    ble_config_start = false;
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_stop();
#endif
                    
                    wifi_config_start = false;
                }
                break;

                case REV_BLE_DISCONNECTED_EVENT:
                {
                    aiio_log_d("REV_BLE_DISCONNECTED_EVENT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                    if(ble_config_start)
                    {
                        aiio_ble_config_stop();
                        AilinkClearPairInfo();

                        rev_queue.common_event = REV_CONFIG_START_EVENT;
                        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                        {
                            aiio_log_i("queue send failed\r\n");
                        }
                        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                    }
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_stop();
#endif
                }
                break;

                case REV_CONFIG_FAIL_EVENT:
                {
                    aiio_log_d("REV_CONFIG_FAIL_EVENT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                    aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);
                    aiio_os_tick_dealy(aiio_os_ms2tick(2000));
                    aiio_ble_config_stop();
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_stop();
#endif
                    AilinkClearPairInfo();
                    aiio_turn_on_led_status(ONLY_RED_LED_TRIGGLE_STATUS);
                    ble_config_start = false;
                    wifi_config_start = false;
                }
                break;

                case REV_WIFI_CONNECTED_EVENT:
                {
                    aiio_log_d("REV_WIFI_CONNECTED_EVENT \r\n");
                    SetWifiNetWorkState(WIFI_CONNECTED);
                    if(wifi_connect || wifi_start_connected)
                    {
                        aiio_log_d("wifi disconnect \r\n");
                        aiio_wifi_disconnect();
                    }
                    iot_connect_wifi(wifi_config_data.ssid, wifi_config_data.passwd);
                    aiio_TimerStart(wifi_reconnect_timer_handle, WIFI_RECONNECT_WAIT_MAX_TIME);
                }
                break;

                case REV_CLOUD_CONNECTED_EVENT:
                {
                    SetWifiNetWorkState(WIFI_CONN_CLOUD);
                    aiio_log_d("REV_CLOUD_CONNECTED_EVENT \r\n");
                    aiio_cloud_receive_t cloud_data = {0};
                    cloud_data.mqtt_host = wifi_config_data.mqttip;
                    cloud_data.device_token = wifi_config_data.token;
                    cloud_data.mqtt_port = wifi_config_data.port;
                    cloud_data.wifi_info = &wifi_info;

#ifdef CONFIG_WAN_NETWORK_MODULE
                    if(!wifi_config_start)
                    {
                        aiio_user_service_init(true, &cloud_data);
                    }
                    else
                    {
                        aiio_user_service_init(false, &cloud_data);
                    }
#endif

#ifdef CONFIG_LAN_NETWORK_MODULE
                    aiio_user_lan_config_t lan_config = {0};

                    lan_config.device_id = DeviceInfo.deviceId;
                    lan_config.local_ip = wifi_info.ip;
                    aiio_user_lan_service_init(&lan_config);
#endif
                }
                break;

                case REV_CLOUD_ONLINE_EVENT:
                {
                    aiio_log_d("REV_CLOUD_ONLINE_EVENT \r\n");

                    if(wifi_config_start)
                    {
                        rev_queue.common_event = REV_CONFIG_OK_EVENT;
                        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                        {
                            aiio_log_i("queue send failed\r\n");
                        }
                        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                    }
                    aiio_turn_on_led_status(ONLY_BLUE_LED_TURN_ON);
                    if(!deviceIsLaunch)
                    {
                        deviceIsLaunch = true;
                        AilinkSaveTimeInfo();
                    }
#ifdef CONFIG_OTA_MODULE
                    if(AilinkGetOtaInstallStatus())
                    {
                        IotLoadOTAInfo();
                        aiio_cloud_ota_report_proccess_status(AIIO_OTA_SUCCESS, OtaProInfo.OtaVer, OtaProInfo.PubId);
                        AilinkSetOtaInstallStatus(false);
                    }
#endif
                }
                break;

                case REV_CLOUD_DISCONNECTED:
                {
                    aiio_log_d("REV_CLOUD_DISCONNECTED \r\n");
                    if(!wifi_config_start && wifi_connect)
                    {
                        aiio_TimerStart(cloud_wait_connect_handle, CLOUD_RECONNECT_WAIT_MAX_TIME);
                    }
#ifdef CONFIG_WAN_NETWORK_MODULE

                    aiio_user_service_deinit();
#endif
                    aiio_turn_on_led_status(ONLY_BLUE_LED_TRIGGLE_STATUS);

                    if(!wifi_config_start && !wifi_connect)
                    {
                        rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
                        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                        {
                            aiio_log_i("queue send failed\r\n");
                        }
                        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                    }
                }
                break;

                case REV_CLOUD_OTA_INFO_EVENT:
                {
                    aiio_log_d("REV_CLOUD_OTA_INFO_EVENT \r\n");
#ifdef CONFIG_OTA_MODULE
                    aiio_ota_msg_t *ota_info = NULL;

                    // memcpy(&ota_info, rev_queue.queue_data, rev_queue.queue_data_len);
                    ota_info = rev_queue.queue_data;
                    memcpy(&OtaProInfo, ota_info, sizeof(aiio_ota_msg_t));
                    aiio_log_d("channel = %d \r\n", OtaProInfo.channel);
                    aiio_log_d("otaVer = %s \r\n", OtaProInfo.OtaVer);
                    aiio_log_d("otaType = %s \r\n", OtaProInfo.otaType);
                    aiio_log_d("download_url = %s \r\n", OtaProInfo.download_url);
                    aiio_log_d("PubId = %s \r\n", OtaProInfo.PubId);
                    aiio_log_d("md5 = %s \r\n", OtaProInfo.md5);
                    IotSaveOTAInfo();
                    printf("strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)) is %d\r\n",strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)));
                    if(strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)) == 0)
                    {
                        NotifyMcuDeviceUpdate(OtaProInfo.OtaVer, OtaProInfo.md5);
                    }
#endif
                }
                break;

                case REV_CLOUD_OTA_FAIL_EVENT:
                {
                    aiio_log_i("REV_CLOUD_OTA_FAIL_EVENT \r\n");
#ifdef CONFIG_OTA_MODULE
                    aiio_ota_transmit_stop();

                    IotClearOTAInfo();
#endif
                }
                break;

                case REV_CLOUD_OTA_SUCCESS_EVENT:
                {
                    aiio_log_i("REV_CLOUD_OTA_SUCCESS_EVENT \r\n");
#ifdef CONFIG_OTA_MODULE 
                    aiio_ota_transmit_stop();
                    aiio_restart();
#endif
                }
                break;
                
                default:
                    aiio_log_e("can't find event[%d] \r\n", rev_queue.common_event);
                    break;
            }
        }

        if(aiio_CheckTimerTimeout(cloud_wait_connect_handle) == AIIO_TIMER_TIMEOUT)
        {
            aiio_log_d("reconneceing cloud \r\n");
            rev_queue.common_event = REV_CLOUD_CONNECTED_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }

        if(aiio_CheckTimerTimeout(wifi_reconnect_timer_handle) == AIIO_TIMER_TIMEOUT)
        {
            aiio_log_d("reconneceing WIFI \r\n");
            rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }

        aiio_os_tick_dealy(aiio_os_ms2tick(20));
    }
    


    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

void aiio_main(void *params){

    int32_t ret=0;
    aiio_rev_queue_t  rev_queue = {0};
    uint16_t led_status_tick = 0;
    uint32_t count = 0;
    char DataBuff[100] = {0};

    aiio_log_i("build date: %s \r\n", __DATE__);
    aiio_log_i("build time: %s \r\n", __TIME__);
    aiio_log_i("device version: %s \r\n", DEVICE_FW_VERSION);
    
    aiio_TimerInit();

    aiio_cloud_led_init();
    aiio_cloud_key_init();

    aiio_nvs_init();
    aiio_flash_init();

    aiio_mcu_process_init();          /* 初始化MCU通信处理 */

    iotgoGetDeviceInfo(&DeviceInfo);    
    if(iotCheckDeviceInfoIsEmpty(&DeviceInfo) == 0)                   
    {
        aiio_log_e("device isn't init, it need to save device info \r\n");
    }else{
        deviceinfo_flag = true;
        wifi_watchdog_timer_handle = aiio_TimerAdd(AIIO_REPEAT_TIMER);
        if(wifi_watchdog_timer_handle == -1)
        {
            aiio_log_e("watchdog softtimner add fail!!");
        }
        aiio_log_d("wifi_watchdog_timer_handle = %d \r\n", wifi_watchdog_timer_handle);
        aiio_wdt_init();  
        aiio_TimerStart(wifi_watchdog_timer_handle, 5000);
        aiio_turn_on_led_status(ALL_LED_TRIGGLE_STATUS);
    }
    
    aiio_rtc_time_init();

    DEVICE_INFO_OUTPUT("############################## Device info  ##################################\r\n");
    snprintf(DataBuff, sizeof(DataBuff), "build time: %s, build date: %s, fw version: %s \r\n", __TIME__, __DATE__,DEVICE_FW_VERSION);
    DEVICE_INFO_OUTPUT(DataBuff);
    memset(DataBuff, 0, sizeof(DataBuff));
    snprintf(DataBuff, sizeof(DataBuff), "userName: %s\r\nuserPasswd: %s\r\ndeviceId: %s\r\n", DeviceInfo.userName, DeviceInfo.userPasswd, DeviceInfo.deviceId);
    DEVICE_INFO_OUTPUT(DataBuff);
    memset(DataBuff, 0, sizeof(DataBuff));
    DEVICE_INFO_OUTPUT("############################## end  ##################################\r\n");

    while (1)
    {
        if(deviceinfo_flag){
            if(aiio_CheckTimerTimeout(wifi_watchdog_timer_handle) == AIIO_TIMER_TIMEOUT)
            {
                aiio_log_d("feed dog \r\n");
                aiio_wdt_reload(); 
            }
            if(GetMCUConnectState() && !ailinkInitstate)            /* MCU与模组握手成功后，方可执行以下内容 */
            {
                if(GetProductID() == NULL || strlen(GetProductID()) == 0)
                {
                    continue;
                }
                if(GetProductFlag() == NULL || strlen(GetProductFlag()) == 0)
                {
                    continue;
                }
                aiio_log_d("product ID: %s \r\n", GetProductID());
                aiio_log_d("productflag = %s \r\n", GetProductFlag());

                aiio_wifi_register_event_cb(cb_wifi_event);
                ret=aiio_wifi_init();
                if(ret!=0){
                    aiio_log_e("wifi init error!!");
                    continue;
                }

                cloud_wait_connect_handle = aiio_TimerAdd(AIIO_SINGLE_TIMER);
                if(cloud_wait_connect_handle == -1)
                {
                    aiio_log_e("softtimner add fail!!");
                    continue;
                }
                aiio_log_d("cloud_wait_connect_handle = %d \r\n", cloud_wait_connect_handle);
                aiio_TimerStop(cloud_wait_connect_handle);

                wifi_reconnect_timer_handle = aiio_TimerAdd(AIIO_REPEAT_TIMER);
                if(wifi_reconnect_timer_handle == -1)
                {
                    aiio_log_e("softtimner add fail!!");
                    continue;
                }
                aiio_log_d("wifi_reconnect_timer_handle = %d \r\n", wifi_reconnect_timer_handle);
                aiio_TimerStop(wifi_reconnect_timer_handle);


                cloud_rev_queue_handle = xQueueCreate(QUEUE_MAX_SIZE, sizeof(aiio_rev_queue_t));
                AIIO_MEM_CHECK(cloud_rev_queue_handle, continue);

                aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);

                printf("AilinkLoadPairInfo(&wifi_config_data) is %d\r\n",AilinkLoadPairInfo(&wifi_config_data));
                if(AilinkLoadPairInfo(&wifi_config_data) > 0)
                {
                    aiio_log_i("ssid = %s \r\n", wifi_config_data.ssid);
                    aiio_log_i("passwd = %s \r\n", wifi_config_data.passwd);
                    aiio_log_d("time zone = %s \r\n", wifi_config_data.tz);
                    aiio_update_rtc_time_with_zone(wifi_config_data.tz, strlen(wifi_config_data.tz));
                    rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
                    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                    {
                        aiio_log_e("queue send failed\r\n");
                    }
                    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                }else{
                    rev_queue.common_event = REV_CONFIG_START_EVENT;
                    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                    {
                        aiio_log_e("queue send failed\r\n");
                    }
                    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                }
                ailinkInitstate = true;
            } 
        }   
        if(ota_transmit_init && strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)) == 0){
            switch (GetMcuOTAStatus())
            {
                case OTA_STATUS_START:
                {
                    aiio_log_e("OTA_STATUS_START \r\n");
                    // mcu_ota_start_flag = true;
                    ResetMcuOTAStatus();
                }
                break;

                case OTA_STATUS_MD5_ERR:
                {
                    aiio_log_e("OTA_STATUS_MD5_ERR \r\n");
                    aiio_cloud_ota_report_proccess_status(AIIO_OTA_DATA_ERR, NULL, NULL);
                    ResetMcuOTAStatus();
                    aiio_ota_transmit_stop();
                }
                break;

                case OTA_STATUS_INSTALL:
                {
                    aiio_log_e("OTA_STATUS_INSTALL \r\n");
                    aiio_cloud_ota_report_install_status(AIIO_OTA_SUCCESS);
                    ResetMcuOTAStatus();
                }
                break;

                case OTA_STATUS_OK:
                {
                    aiio_log_e("OTA_STATUS_OK \r\n");
                    aiio_cloud_ota_report_proccess_status(AIIO_OTA_SUCCESS, OtaProInfo.OtaVer, OtaProInfo.PubId);
                    ResetMcuOTAStatus();
                    aiio_ota_transmit_stop();
                }
                break;
                
                default:
                    break;
            }
        }                           
        if(waterfall_light)
        {
            if(!waterfall_light_state)
            {
                aiio_os_tick_count(&count);
                lastTick = aiio_os_tick2ms(count);
                waterfall_light_state = true;
            }

            aiio_os_tick_count(&count);
            CurTick = aiio_os_tick2ms(count);
            if(diffTimeTick(lastTick, CurTick) > 1000)
            {
                lastTick = CurTick;
                
                switch (waterfall_num)
                {
                    case 0:
                    {
                        aiio_turn_on_led_status(ONLY_BLUE_LED_TURN_ON);
                        aiio_log_d("led_b on, led_c off \r\n");
                        waterfall_num++;
                    }
                    break;

                    case 1:
                    {
                        aiio_turn_on_led_status(ONLY_RED_LED_TURN_ON);
                        aiio_log_d("led_r on, led_b off \r\n");
                        waterfall_num++;
                    }
                    break;

                    case 2:
                    {
                        aiio_turn_on_led_status(ONLY_GREEN_LED_TURN_ON);
                        aiio_log_d("led_g on, led_r off \r\n");
                        waterfall_num=0;
                    }
                    break;
                    
                    default:
                        break;
                }
            }

        }  
        led_status_tick++;
        if(led_status_tick > 10 && ailinkInitstate)
        {
            aiio_led_status_process();
            led_status_tick = 0;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(20));
    }
}

