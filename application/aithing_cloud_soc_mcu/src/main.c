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
#include "aiio_cloud_control.h"

// #include "easyflash.h"


#define QUEUE_MAX_SIZE                      (5)             /* max number of queue*/

#define WIFI_MAX_DISCONNECT_NUMBER          (10)

#define CLOUD_RECONNECT_WAIT_MAX_TIME           (10 * 1000)
#define WIFI_RECONNECT_WAIT_MAX_TIME           (20 * 1000)


#define AIIO_UART0_BAUDRATE                         CONFIG_UART0_BAUDRATE
#define AIIO_UART0_TX_PIN                           CONFIG_UART0_TX_PIN
#define AIIO_UART0_RX_PIN                           CONFIG_UART0_RX_PIN
#define AIIO_UART0_RECEIVE_BUFFER_MAX_LEN           CONFIG_UART0_RECEIVE_BUFFER_MAX_LEN

#define AIIO_UART1_BAUDRATE                         CONFIG_UART1_BAUDRATE
#define AIIO_UART1_TX_PIN                           CONFIG_UART1_TX_PIN
#define AIIO_UART1_RX_PIN                           CONFIG_UART1_RX_PIN
#define AIIO_UART1_RECEIVE_BUFFER_MAX_LEN           CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN





static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;           /*!< handle of task thread*/
bool wifi_connect = false;                                           /*!< status of wifi connection*/

bool ble_config_start = false;                                       /*!< Launch state of ble distribution network*/
bool wifi_config_start = false;
bool device_init = false;
static bool wifi_start_connected = false;
static uint8_t wifi_disconnect_count = 0;                                   /*!< The number  of wifi disconnection*/
static aiio_wifi_info_t wifi_info = {0};                                    /*!< Information of wifi*/
static wifi_config_data_t wifi_config_data = {0};                             /*!< The data of ble distribution network */
static uint8_t  uart_ring_buffer[AIIO_UART0_RECEIVE_BUFFER_MAX_LEN] = {0};
static uint8_t  uart_receive_buffer[AIIO_UART0_RECEIVE_BUFFER_MAX_LEN] = {0};
static ring_buff_t uart_ring_buff_hd = {0};
static AIIO_TIMER cloud_wait_connect_handle = -1;
static AIIO_TIMER wifi_reconnect_timer_handle = -1;


/* Define global variable */
QueueHandle_t    cloud_rev_queue_handle = NULL;                             /*!< handle of queue*/
aiio_device_info DeviceInfo = {0};

static void iot_connect_wifi(char *ssid, char *passwd);



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
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
            aiio_turn_on_led_status(ONLY_GREEN_LED_TRIGGLE_STATUS);
#endif

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
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
            aiio_turn_on_led_status(ONLY_GREEN_LED_TURN_ON);
#endif
            aiio_TimerStop(wifi_reconnect_timer_handle);

            aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};
            aiio_wifi_ip_params_t wifi_sta_ip_praram = {0};
            uint8_t mac[MAC_LEN];
            int32_t rssi=0;

            aiio_wifi_rssi_get(&rssi);
            aiio_log_d("wifi cur_rssi = %d!!",rssi);

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


static void iot_connect_wifi(char *ssid, char *passwd)
{
    if(ssid == NULL)
    {
        aiio_log_e("ssid is NULL \r\n");
        return ;
    }
    
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {0};

    memcpy(wifi_sta_config.sta.ssid, ssid, strlen(ssid));
    if(passwd)
    {
        memcpy(wifi_sta_config.sta.password, passwd, strlen(passwd));
    }
    wifi_sta_config.sta.channel = 0;
    wifi_sta_config.sta.use_dhcp = 1;
    wifi_sta_config.sta.flags = 0;

    aiio_log_d("ssid = %s \r\n", wifi_sta_config.sta.ssid);
    aiio_log_d("password = %s \r\n", wifi_sta_config.sta.password);
    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();
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
                    aiio_log_d("REV_CONFIG_DATA_EVENT \r\n");
                    // aiio_turn_on_led_status(GREEN_RED_LED_TURN_ON);
                    memcpy(&wifi_config_data, rev_queue.queue_data, rev_queue.queue_data_len);
                    aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
                    aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
                    aiio_log_d("mqttip = %s \r\n", wifi_config_data.mqttip);
                    aiio_log_d("token = %s \r\n", wifi_config_data.token);
                    aiio_log_d("time zone = %s \r\n", wifi_config_data.tz);
                    aiio_update_rtc_time_with_zone(wifi_config_data.tz, strlen(wifi_config_data.tz));
                    aiio_flash_save_wifi_config_data(&wifi_config_data);
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

                    aiio_TimerStop(wifi_reconnect_timer_handle);
                    aiio_os_tick_dealy(aiio_os_ms2tick(2000));
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
                    aiio_ble_config_start();
                    ble_config_start = true;
#endif
#ifdef CONFIG_AP_DISTRIBUTION_NETWORK
                    aiio_ap_config_start();
#endif
                    aiio_TimerStop(cloud_wait_connect_handle);
                    
                    wifi_config_start = true;
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
                    aiio_turn_on_led_status(GREEN_RED_LED_TRIGGLE_STATUS);
#else
                    aiio_turn_on_led_status(ONLY_BLUE_LED_TRIGGLE_STATUS);
#endif
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

                        if(aiio_flash_get_wifi_config_data(&wifi_config_data) > 0)
                        {
                            aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
                            aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
                            iot_connect_wifi(wifi_config_data.ssid, wifi_config_data.passwd);
                        }
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
                        aiio_turn_on_led_status(ALL_LED_TRIGGLE_STATUS);
#endif
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
                        aiio_flash_clear_config_data();

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
                    aiio_flash_clear_config_data();
#ifndef CONFIG_CLOUD_CONTROL_LED_MODULE
                    aiio_turn_on_led_status(ONLY_RED_LED_TRIGGLE_STATUS);
#else
                    aiio_turn_on_led_status(ONLY_BLUE_LED_TRIGGLE_STATUS);
#endif
                    ble_config_start = false;
                    wifi_config_start = false;
                }
                break;

                case REV_WIFI_CONNECTED_EVENT:
                {
                    aiio_log_d("REV_WIFI_CONNECTED_EVENT \r\n");
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
                    aiio_report_bool_attibute_status(NULL, NULL, CMD_POWERSTATE, aiio_get_blue_led_status());
#ifdef CONFIG_OTA_MODULE
                    aiio_ota_msg_t ota_info = {0};
                    if(aiio_flash_get_ota_info(&ota_info) > 0)
                    {
                        aiio_log_d("channel = %d \r\n", ota_info.channel);
                        aiio_log_d("otaVer = %s \r\n", ota_info.otaVer);
                        aiio_log_d("otaType = %s \r\n", ota_info.otaType);
                        aiio_log_d("download_url = %s \r\n", ota_info.download_url);
                        aiio_log_d("PubId = %s \r\n", ota_info.PubId);
                        aiio_log_d("md5 = %s \r\n", ota_info.md5);
                        aiio_cloud_ota_report_proccess_status(AIIO_OTA_SUCCESS, ota_info.otaVer, ota_info.PubId);
                        aiio_flash_clear_ota_info(&ota_info);
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
                    aiio_log_d("channel = %d \r\n", ota_info->channel);
                    aiio_log_d("otaVer = %s \r\n", ota_info->otaVer);
                    aiio_log_d("otaType = %s \r\n", ota_info->otaType);
                    aiio_log_d("download_url = %s \r\n", ota_info->download_url);
                    aiio_log_d("PubId = %s \r\n", ota_info->PubId);
                    aiio_log_d("md5 = %s \r\n", ota_info->md5);
                    aiio_flash_save_ota_info(ota_info);
#endif
                }
                break;

                case REV_CLOUD_OTA_FAIL_EVENT:
                {
                    aiio_log_i("REV_CLOUD_OTA_FAIL_EVENT \r\n");
#ifdef CONFIG_OTA_MODULE
                    aiio_ota_transmit_stop();

                    aiio_ota_msg_t ota_info = {0};
                    aiio_flash_clear_ota_info(&ota_info);
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




static void aiio_uart0_receive_data(uint8_t *buf, uint16_t size)
{
    ring_buff_push_data(&uart_ring_buff_hd, (char *)buf, size);
}



void aiio_main(void *params){

    int32_t ret=0;
    aiio_rev_queue_t  rev_queue = {0};
    aiio_uart_config_t uart_config = {0};
    uint16_t led_status_tick = 0;
    uint16_t led_control_tick = 0;
    uint32_t uart_receive_len = 0;

    aiio_log_i("build date: %s \r\n", __DATE__);
    aiio_log_i("build time: %s \r\n", __TIME__);
    aiio_log_i("device version: %s \r\n", DEVICE_FW_VERSION);
    aiio_log_i("product ID: %s \r\n", PRODUCT_ID);

    uart_config.uart_num = AIIO_UART0;
    uart_config.uart_tx_pin = AIIO_UART0_TX_PIN;
    uart_config.uart_rx_pin = AIIO_UART0_RX_PIN;
    uart_config.baud_rate = AIIO_UART0_BAUDRATE;
    uart_config.irq_rx_bk = aiio_uart0_receive_data;
    aiio_uart_init(uart_config);

    ring_buff_init(&uart_ring_buff_hd, (char *)uart_ring_buffer, AIIO_UART0_RECEIVE_BUFFER_MAX_LEN);

    aiio_flash_init();
    // easyflash_init();

    if(aiio_flash_get_device_info(&DeviceInfo) <= 0)
    {
        aiio_log_e("device isn't init, it need to save device info \r\n");
        goto end;
    }

    aiio_wifi_register_event_cb(cb_wifi_event);
    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
        goto end;
    }

    aiio_rtc_time_init();
    aiio_TimerInit();
    aiio_cloud_led_init();
    aiio_cloud_key_init();

    cloud_wait_connect_handle = aiio_TimerAdd(AIIO_SINGLE_TIMER);
    if(cloud_wait_connect_handle == -1)
    {
        aiio_log_e("softtimner add fail!!");
        goto end;
    }
    aiio_log_d("cloud_wait_connect_handle = %d \r\n", cloud_wait_connect_handle);
    aiio_TimerStop(cloud_wait_connect_handle);

    wifi_reconnect_timer_handle = aiio_TimerAdd(AIIO_REPEAT_TIMER);
    if(wifi_reconnect_timer_handle == -1)
    {
        aiio_log_e("softtimner add fail!!");
        goto end;
    }
    aiio_log_d("wifi_reconnect_timer_handle = %d \r\n", wifi_reconnect_timer_handle);
    aiio_TimerStop(wifi_reconnect_timer_handle);

    cloud_rev_queue_handle = xQueueCreate(QUEUE_MAX_SIZE, sizeof(aiio_rev_queue_t));
    AIIO_MEM_CHECK(cloud_rev_queue_handle, goto end);

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);


    if(aiio_flash_get_wifi_config_data(&wifi_config_data) > 0)
    {
        aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
        aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
        aiio_log_d("time zone = %s \r\n", wifi_config_data.tz);
        aiio_update_rtc_time_with_zone(wifi_config_data.tz, strlen(wifi_config_data.tz));
        rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
        {
            aiio_log_i("queue send failed\r\n");
        }
        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
    }
#ifdef CONFIG_CLOUD_CONTROL_LED_MODULE
    else
    {
        rev_queue.common_event = REV_CONFIG_START_EVENT;
        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
        {
            aiio_log_e("queue send failed\r\n");
        }
        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
    }
#else
    aiio_turn_on_led_status(ALL_LED_TRIGGLE_STATUS);
#endif

    
    device_init = true;

end:
    while (1)
    {
        led_status_tick++;
        if(led_status_tick > 10 && device_init)
        {
            aiio_led_status_process();
            led_status_tick = 0;
        }

        if(control_led_count > 0)      led_control_tick++;
        if(led_control_tick >= 1000)
        {
            aiio_log_i("################# led_control_tick = %d ###################\r\n", led_control_tick);
            aiio_log_i("################# control_led_count = %d ###################\r\n", control_led_count);
            control_led_count = 0;
            led_control_tick = 0;
        }

        if(ring_buff_get_size(&uart_ring_buff_hd) <= 0)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(20));
            continue;
        }
        
        ring_buff_pop_data(&uart_ring_buff_hd, (char *)&uart_receive_buffer[uart_receive_len], 1);
        uart_receive_len += 1;
        if(uart_receive_len >= AIIO_UART0_RECEIVE_BUFFER_MAX_LEN)
        {
            ring_buff_flush(&uart_ring_buff_hd);
            uart_receive_len = 0;
            aiio_memset(uart_receive_buffer, 0, 100);
            continue;
        }

        if((uart_receive_len >= 4) && ('\r' == uart_receive_buffer[uart_receive_len-2]) && ('\n' == uart_receive_buffer[uart_receive_len-1]))
        {
            aiio_log_d("rev:%s \r\n", uart_receive_buffer);
            aiio_log_d("cmdlen:%d \r\n", uart_receive_len);
            uart_receive_buffer[uart_receive_len-2] = '\0';
            atCmdExecute((char *)uart_receive_buffer);
            uart_receive_len = 0;
            aiio_memset(uart_receive_buffer, 0, AIIO_UART0_RECEIVE_BUFFER_MAX_LEN);
        }
        
    }
}

