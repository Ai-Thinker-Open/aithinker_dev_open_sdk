#include "aiio_adapter_include.h"
#include "aiio_common.h"
#include "aiio_ble_config_wifi.h"
#include "aiio_at_receive.h"
#include "aiio_cloud_led.h"
#include "cJSON.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS
#define EXAMPLE_AXK_WIFI_STA_CHANNEL            CONFIG_AXK_WIFI_STA_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_SSID                CONFIG_AXK_WIFI_AP_SSID
#define EXAMPLE_AXK_WIFI_AP_PASS                CONFIG_AXK_WIFI_AP_PASS
#define EXAMPLE_AXK_WIFI_MAX_STA_CONN           CONFIG_AXK_WIFI_MAX_STA_CONN
#define EXAMPLE_AXK_WIFI_AP_CHANNEL             CONFIG_AXK_WIFI_AP_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_IP                  CONFIG_AXK_WIFI_AP_IP
#ifdef CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     1
#else
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     0
#endif


#define QCLOUD_IOT_MQTT_DIRECT_DOMAIN "mqtt://XU6O5IP8LQ.iotcloud.tencentdevices.com"

#define         DEVICE_FW_VERSION           ("1.0.1")
#define         QUEUE_MAX_SIZE                      (5)             /* max number of queue*/



static uint8_t  uart_ring_buffer[AIIO_AT_RECEIVE_BUFFER_MAX_LEN] = {0};
static uint8_t  uart_receive_buffer[AIIO_AT_RECEIVE_BUFFER_MAX_LEN] = {0};
static ring_buff_t uart_ring_buff_hd = {0};

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ap_wifi_thread = NULL;
static bool mqtt_connect_flag = false;

static aiio_wifi_conf_t aiio_conf_test =
{
    .country_code = "CN",
};

typedef struct test_wifi_item {
    char ssid[32];
    uint32_t ssid_len;
    uint8_t bssid[6];
    uint8_t channel;
    uint8_t auth;
    int8_t rssi;
} test_wifi_item_t;

static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
static int32_t ap_stop_flag=0;
static uint8_t sta_num=0;
uint8_t mac[MAC_LEN];
uint8_t testmac[MAC_LEN]={0x18,0xB9,0x05,0x00,0x00,0x01};
aiio_wifi_mode_t cur_wifi_mode=AIIO_WIFI_MODE_NULL;
aiio_wifi_ip_params_t sta_ip_pra={0};
aiio_wifi_sta_basic_info_t sta_info_test[CONFIG_MAX_STA_CONN_LIMIT]={0};
aiio_wifi_ip_params_t ap_ip_pra={0};
aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};


QueueHandle_t    cloud_rev_queue_handle = NULL;                             /*!< handle of queue*/
aiio_device_info DeviceInfo_info = {0};
aiio_mqtt_client_config_t mqtt_cfg = {0};
bool wifi_paring_flag = false;
bool device_init = false; 

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        aiio_log_e("Last error %s: 0x%x", message, error_code);
    }
}

void aiio_property_parse(void *data){
    cJSON *cjson_root   = NULL;
    cJSON *cjson_params = NULL;
    if(data == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    cjson_root = cJSON_Parse(data);
    if(cjson_root == NULL)
    {
        aiio_log_e("json parse err \r\n");
        return ;
    }
    cjson_params = cJSON_GetObjectItem(cjson_root, "params");
    if(cjson_params == NULL)
    {
        aiio_log_e("cjson_params is NULL \r\n");
        cJSON_Delete(cjson_root);
        return ;
    }
     if (cJSON_IsObject(cjson_params)) {  
        cJSON *power_switch_item = cJSON_GetObjectItem(cjson_params, "power_switch");  
        if (power_switch_item != NULL) {    
            aiio_log_d("Brightness: %d\n", power_switch_item->valueint);  
            if(power_switch_item->valueint == 1){
                aiio_turn_on_led_status(ALL_LED_TURN_ON);
            }else if(power_switch_item->valueint == 0){
                aiio_turn_on_led_status(ALL_LED_TURN_OFF);
            }
        } else {  
            aiio_log_e("Error: 'brightness' not found in 'cjson_params'.\n");  
        }  
    } else {  
        aiio_log_e("Error: 'cjson_params' is not an object.\n");  
    }  
    cJSON_Delete(cjson_root);
}

void aiio_event_parse(void *data){
    
}

void aiio_action_parse(void *data){
    
}

void aiio_topic_recv_parse(char *topic, void *data){
    if (strstr(topic, "$thing/down/property/") != NULL){
        aiio_property_parse(data);
    }else if(strstr(topic, "$thing/down/event/") != NULL){
        aiio_event_parse(data);
    }else if(strstr(topic, "$thing/down/action/") != NULL){
        aiio_action_parse(data);
    }
}

static aiio_err_t event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    aiio_mqtt_client_handle_t client = event->client;
    aiio_rev_queue_t  rev_queue = {0};

    event_id = event->event_id;
    aiio_log_i("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        aiio_log_i("MQTT_EVENT_CONNECTED");
        mqtt_connect_flag = true;
        aiio_log_i("wifi_paring_flag:%d", wifi_paring_flag);
        if(wifi_paring_flag == false){
            _mqtt_send_token();
            wifi_paring_flag = true;
        }
        char topic[MAX_SIZE_OF_CLOUD_TOPIC] = {0};
        snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/down/service/%s/%s", DeviceInfo_info.product_id, DeviceInfo_info.device_name);
        aiio_log_i("topic is %s\r\n",topic);
        msg_id = aiio_mqtt_client_subscribe(client, topic, 0);
        aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);
        snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/down/event/%s/%s", DeviceInfo_info.product_id, DeviceInfo_info.device_name);
        aiio_log_i("topic is %s\r\n",topic);
        msg_id = aiio_mqtt_client_subscribe(client, topic, 0);
        aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);
        snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/down/action/%s/%s", DeviceInfo_info.product_id, DeviceInfo_info.device_name);
        aiio_log_i("topic is %s\r\n",topic);
        msg_id = aiio_mqtt_client_subscribe(client, topic, 0);
        aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);
        snprintf(topic, MAX_SIZE_OF_CLOUD_TOPIC, "$thing/down/property/%s/%s", DeviceInfo_info.product_id, DeviceInfo_info.device_name);
        aiio_log_i("topic is %s\r\n",topic);
        msg_id = aiio_mqtt_client_subscribe(client, topic, 0);
        aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);
        rev_queue.common_event = REV_CLOUD_ONLINE_EVENT;
        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
        {
            aiio_log_i("queue send failed\r\n");
        }
        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        break;
    case MQTT_EVENT_DISCONNECTED:
        aiio_log_i("MQTT_EVENT_DISCONNECTED");

        break;
    case MQTT_EVENT_SUBSCRIBED:
        aiio_log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        aiio_log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        aiio_log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        aiio_log_i("MQTT_EVENT_DATA");
        aiio_log_i("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        aiio_log_i("SUBSCRIBED DATA=%.*s\r\n", event->data_len, event->data);
        aiio_topic_recv_parse(event->topic, event->data);
        break;
    case MQTT_EVENT_ERROR:
        aiio_log_i("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->aiio_tls_last_aiio_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->aiio_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->aiio_transport_sock_errno);
            aiio_log_i("Last errno string (%s)", strerror(event->error_handle->aiio_transport_sock_errno));
        }
        break;
    default:
        aiio_log_i("Other event id:%d", event->event_id);
        break;
    }
    return AIIO_OK;
}


aiio_mqtt_client_handle_t client = NULL;
static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
    int32_t ret=0;
    aiio_rev_queue_t  rev_queue = {0};
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
            if(wifi_paring_flag == false){
                bt_combo_report_wificonn_success();
            }else{
                if(!mqtt_connect_flag){
                    rev_queue.common_event = REV_CLOUD_CONNECTED_EVENT;
                    if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                    {
                        aiio_log_i("queue send failed\r\n");
                    }
                    memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                }
            }
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

static void iot_connect_wifi(char *ssid, char *passwd)
{
    if(ssid == NULL)
    {
        aiio_log_e("ssid is NULL \r\n");
        return ;
    }
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);
    aiio_wifi_config_t wifi_sta_config = {0};

    memcpy(wifi_sta_config.sta.ssid, ssid, strlen(ssid) + 1);
    if(passwd)
    {
        memcpy(wifi_sta_config.sta.password, passwd, strlen(passwd) + 1);
    }
    wifi_sta_config.sta.channel = 0;
    wifi_sta_config.sta.use_dhcp = 1;
    wifi_sta_config.sta.flags = 0;

    aiio_log_d("ssid = %s \r\n", wifi_sta_config.sta.ssid);
    aiio_log_d("password = %s \r\n", wifi_sta_config.sta.password);
    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();
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

                break;

                case REV_CONFIG_START_EVENT:
                {
                    // iot_connect_wifi("ZLM_24G", "1234568");
                    wifi_paring_flag = false;
                    aiio_ble_config_start();
                    aiio_turn_on_led_status(GREEN_RED_LED_TRIGGLE_STATUS);
                } 
                break;

                case REV_CONFIG_STOP_EVENT:
                   
                break;

                case REV_CONFIG_OK_EVENT:
                
                break;

                case REV_BLE_DISCONNECTED_EVENT:
                
                break;

                case REV_CONFIG_FAIL_EVENT:
                
                break;

                case REV_WIFI_CONNECTED_EVENT:
                {
                    aiio_log_d("REV_WIFI_CONNECTED_EVENT \r\n");
                    iot_connect_wifi(wifi_config_data.ssid, wifi_config_data.passwd);
                }
                
                break;

                case REV_CLOUD_CONNECTED_EVENT:
                {
                    mqtt_cfg.event_handle = event_cb;
                    mqtt_cfg.uri = QCLOUD_IOT_MQTT_DIRECT_DOMAIN;   
                    mqtt_cfg.port = 1883;

                    aiio_log_i("[%s()-%d]MQTT connect config\r\n",__func__,__LINE__);
                    aiio_log_i("  host:%s:%d\r\n",mqtt_cfg.uri,mqtt_cfg.port);

                    aiio_log_i("111 @@@@ mqtt_cfg.client_id:%s\r\n",mqtt_cfg.client_id);
                    aiio_log_i("111 @@@@ mqtt_cfg.username:%s\r\n",mqtt_cfg.username);
                    aiio_log_i("111 @@@@ mqtt_cfg.password:%s\r\n",mqtt_cfg.password);

                    client = aiio_mqtt_client_init(&mqtt_cfg);
                    aiio_mqtt_client_start(client);
                }
                break;

                case REV_CLOUD_ONLINE_EVENT:
                {
                    aiio_ble_config_stop();
                    aiio_turn_on_led_status(ONLY_BLUE_LED_TURN_ON);
                }
               
                break;

                case REV_CLOUD_DISCONNECTED:
                
                break;

                case REV_CLOUD_OTA_INFO_EVENT:
                
                break;

                case REV_CLOUD_OTA_FAIL_EVENT:
                
                break;

                case REV_CLOUD_OTA_SUCCESS_EVENT:
                
                break;
                
                default:
                    aiio_log_e("can't find event[%d] \r\n", rev_queue.common_event);
                    break;
            }
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(20));
    }
    


    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

static void aiio_at_receive_data(uint8_t *buf, uint16_t size)
{
    ring_buff_push_data(&uart_ring_buff_hd, (char *)buf, size);
}

// static void ble_print_mac(void)
// {
//     uint8_t mac[6];

//     aiio_ble_get_mac(mac);
//     aiio_log_i("ble mac:%02x:%02x:%02x:%02x:%02x:%02x",
//                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
// }

void aiio_main(void *params){

    aiio_uart_config_t uart_config = {0};
    uint32_t uart_receive_len = 0;
    int32_t ret=0;
    aiio_rev_queue_t  rev_queue = {0};
    uint16_t led_status_tick = 0;

    aiio_log_i("device version: %s \r\n", DEVICE_FW_VERSION);

    ring_buff_init(&uart_ring_buff_hd, (char *)uart_ring_buffer, AIIO_AT_RECEIVE_BUFFER_MAX_LEN);
    uart_config.uart_num = AIIO_AT_UART;
    uart_config.uart_tx_pin = AIIO_AT_TX_PIN;
    uart_config.uart_rx_pin = AIIO_AT_RX_PIN;
    uart_config.baud_rate = AIIO_AT_BAUDRATE;
    uart_config.irq_rx_bk = aiio_at_receive_data;
    uart_config.dataBits= AIIO_AT_DATABIT;
    uart_config.stopBits= AIIO_AT_STOPBIT;
    uart_config.parity= AIIO_AT_PARITY;
    // aiio_uart_deinit(uart_config);
    aiio_uart_init(uart_config);

    aiio_flash_init();

    if(aiio_flash_get_device_info(&DeviceInfo_info) <= 0)
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
    // ble_print_mac();
    aiio_cloud_led_init();
    aiio_cloud_key_init();

    cloud_rev_queue_handle = xQueueCreate(QUEUE_MAX_SIZE, sizeof(aiio_rev_queue_t));
    AIIO_MEM_CHECK(cloud_rev_queue_handle, goto end);
    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);

    IOT_MQTT_Construct(&DeviceInfo_info);
    aiio_turn_on_led_status(ALL_LED_TRIGGLE_STATUS);
    device_init = true;

    if(aiio_flash_get_wifi_config_data(&wifi_config_data) > 0)
    {
        aiio_log_d("ssid = %s \r\n", wifi_config_data.ssid);
        aiio_log_d("passwd = %s \r\n", wifi_config_data.passwd);
        wifi_paring_flag = true;
        rev_queue.common_event = REV_WIFI_CONNECTED_EVENT;
        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
        {
            aiio_log_i("queue send failed\r\n");
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
end:
    while (1)
    {
        led_status_tick++;
        if(led_status_tick > 10 && device_init)
        {
            aiio_led_status_process();
            led_status_tick = 0;
        }

        if(ring_buff_get_size(&uart_ring_buff_hd) <= 0)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(20));
            continue;
        }
        
        ring_buff_pop_data(&uart_ring_buff_hd, (char *)&uart_receive_buffer[uart_receive_len], 1);
        uart_receive_len += 1;
        if(uart_receive_len >= AIIO_AT_RECEIVE_BUFFER_MAX_LEN)
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
            aiio_memset(uart_receive_buffer, 0, AIIO_AT_RECEIVE_BUFFER_MAX_LEN);
        }
    }
}

