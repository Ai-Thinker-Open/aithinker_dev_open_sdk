#include "aiio_log.h"
#include "aiio_os_port.h"
#include "aiio_radar.h"
#include "aiio_wifi.h"
#include "aiio_mqtt_client.h"

#include "stdio.h"

static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
uint8_t mac[6];

aiio_mqtt_client_handle_t radar_client = NULL;
static uint8_t mqtt_conn_flag = 0;
aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};

static void rd01_data_output_cb(uint8_t *data, uint16_t len)
{
    static uint8_t body_status = 0;

    for (size_t i = 0; i < len; i++)
    {
        printf("%02X ", data[i]);
    }
    printf("\r\n");

    switch (data[8])
    {
    case 0x00:
        if (body_status != 0)
        {
            aiio_log_i(" nobody");
            body_status = 0;
            if (mqtt_conn_flag)
            {
                aiio_log_i("report nobody");
                aiio_mqtt_client_publish(radar_client, "/topic/radar", "{\"状态\":\"无人\"}", strlen("{\"状态\":\"无人\"}"), 0, 0);
            }
        }
        break;
    case 0x01:
    case 0x02:
    case 0x03:
        if (body_status == 0)
        {
            aiio_log_i(" body");
            body_status = 1;
            if (mqtt_conn_flag)
            {
                aiio_log_i("report body");
                aiio_mqtt_client_publish(radar_client, "/topic/radar", "{\"状态\":\"有人\"}", strlen("{\"状态\":\"有人\"}"), 0, 0);
            }
        }
        break; 
    default:
        break;
    }
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        aiio_log_e("Last error %s: 0x%x", message, error_code);
    }
}

static aiio_err_t event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    aiio_mqtt_client_handle_t client = event->client;

    event_id = event->event_id;
    aiio_log_i("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        aiio_log_i("MQTT_EVENT_CONNECTED");
        mqtt_conn_flag = 1;
        // msg_id = aiio_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
        // aiio_log_i("sent publish successful, msg_id=%d", msg_id);

        // msg_id = aiio_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = aiio_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = aiio_mqtt_client_unsubscribe(client, "/topic/qos1");
        // aiio_log_i("sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        aiio_log_i("MQTT_EVENT_DISCONNECTED");
        mqtt_conn_flag = 0;

        break;
    case MQTT_EVENT_SUBSCRIBED:
        aiio_log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = aiio_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
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
        aiio_log_i("DATA=%.*s\r\n", event->data_len, event->data);
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

static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
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
            
            aiio_mqtt_client_config_t mqtt_cfg = {
                .event_handle = event_cb,
            };
            mqtt_cfg.uri ="mqtt://192.168.3.165:1883";
            // mqtt_cfg.uri = BROKER_URI_SSL;   //MQTTS

            // mqtt_cfg.host="192.168.202.10";
            
            aiio_log_i("[%s()-%d]MQTT connect config\r\n",__func__,__LINE__);
            aiio_log_i("  host:%s:%d\r\n",mqtt_cfg.uri,mqtt_cfg.port);
            aiio_log_i("  username:%s\r\n",mqtt_cfg.username);
            aiio_log_i("  password:%s\r\n",mqtt_cfg.password);
            aiio_log_i("  clientID:%s\r\n",mqtt_cfg.client_id);

            radar_client = aiio_mqtt_client_init(&mqtt_cfg);
            aiio_mqtt_client_start(radar_client);            
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

static void test_sta_wifi_entry(void *pvParameters)
{
    
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = "specter",
            .password = "12345678909",
            .channel = 0,
            .use_dhcp=1,
        },
    };

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();

    aiio_os_thread_delete(NULL);
}

void aiio_main(void *params)
{
    aiio_rd01_data_output_cb_register(rd01_data_output_cb);
    aiio_rd01_init();

    aiio_wifi_register_event_cb(cb_wifi_event);
    aiio_wifi_init();

    aiio_os_thread_create(NULL, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    
    aiio_os_thread_delete(NULL);
}

