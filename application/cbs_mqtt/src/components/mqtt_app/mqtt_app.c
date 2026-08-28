#define __MQTT_APP_C_
#include "mqtt_app.h"
#include <string.h>

#define FLASH_RW_START_ADDR 0x150000

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_mqtt_send_thread = NULL;

extern QueueHandle_t xled_queue;

#define BROKER_URI_TCP "mqtt://wx.ai-thinker.com"
#define BROKER_PORT 1883
#define BROKER_CLIENT_ID "TEST2"

aiio_mqtt_client_handle_t my_client;
uint8_t mac_addr[6];
uint8_t top_str[100];
static uint8_t mqtt_status = 0;

void mac_to_string_with_prefix(const uint8_t mac[6], char *mac_str)
{
    // 定义前缀
    const char *prefix = "AiPi-CBS-Kit/";

    // 将前缀复制到输出字符串中
    strcpy(mac_str, prefix);

    // 将MAC地址转换为字符串并追加到前缀后面
    sprintf(mac_str + strlen(prefix), "%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        aiio_log_e("Last error %s: 0x%x", message, error_code);
    }
}

void mqtt_data_send(uint8_t *buf, uint32_t length)
{
    int msg_id;
    mac_to_string_with_prefix(mac_addr, top_str);

    aiio_log_i("top:%s\r\n", top_str);

    // qyq_frame_at_write(top_str, strlen(top_str));

    msg_id = aiio_mqtt_client_publish(my_client, top_str, buf, length, 1, 0);
    aiio_log_i("sent publish successful, msg_id=%d", msg_id);
}

static aiio_err_t event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    uint8_t value = 0;
    aiio_mqtt_client_handle_t client = event->client;

    event_id = event->event_id;
    aiio_log_i("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        aiio_log_i("MQTT_EVENT_CONNECTED");

        mqtt_status = 1;

        value = 1;

        if (xQueueSend(xled_queue, &value, portMAX_DELAY) == pdPASS)
        {
            // 成功发送数据
            printf("xled_queue is send success\r\n");
        }
        // mqtt_data_send("Hello service",0);

        // msg_id = aiio_mqtt_client_subscribe(client, "/topic/qos0", 0);
        // aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = aiio_mqtt_client_subscribe(client, "/topic/qos1", 1);
        // aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);

        // msg_id = aiio_mqtt_client_unsubscribe(client, "/topic/qos1");
        // aiio_log_i("sent unsubscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        aiio_log_i("MQTT_EVENT_DISCONNECTED");
        mqtt_status = 0;
        value = 0;

        if (xQueueSend(xled_queue, &value, portMAX_DELAY) == pdPASS)
        {
            // 成功发送数据
            printf("xled_queue is send success\r\n");
        }
        break;
    case MQTT_EVENT_SUBSCRIBED:
        aiio_log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        // msg_id = aiio_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        // aiio_log_i("sent publish successful, msg_id=%d", msg_id);
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
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
        {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->aiio_tls_last_aiio_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->aiio_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno", event->error_handle->aiio_transport_sock_errno);
            aiio_log_i("Last errno string (%s)", strerror(event->error_handle->aiio_transport_sock_errno));
        }
        break;
    default:
        aiio_log_i("Other event id:%d", event->event_id);
        break;
    }
    return AIIO_OK;
}

static void cb_wifi_event(aiio_input_event_t *event, void *data)
{
    int32_t ret = 0;
    // uint8_t bssid[6] = {0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};
    switch (event->code)
    {
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
            event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE)
        {
            // connect timeout
            aiio_log_d("connect timeout");
        }
        else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                 event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                 event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION)
        {
            // password error
            aiio_log_d("password error");
        }
        else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL)
        {
            // not found AP
            aiio_log_d("not found AP");
        }
        else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH))
        {
            // wifi disconnect
            aiio_log_d("wifi disconnect");
        }
        else
        {
            // connect error
            aiio_log_d("connect error");
        }
        mqtt_status = 0;
        break;
    case AIIO_WIFI_EVENT_STA_GOT_IP:
        aiio_log_d("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");

        aiio_mqtt_client_config_t mqtt_cfg = {
            .event_handle = event_cb,
        };
        mqtt_cfg.uri = BROKER_URI_TCP;
        mqtt_cfg.port = BROKER_PORT;
        mqtt_cfg.client_id = BROKER_CLIENT_ID;

        aiio_wifi_sta_mac_get(mac_addr);
        aiio_log_d("wifi mac = %02x%02x%02x%02x%02x%02x!!", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        // mqtt_cfg.uri = BROKER_URI_SSL;   //MQTTS

        // mqtt_cfg.host="192.168.202.10";

        aiio_log_i("[%s()-%d]MQTT connect config\r\n", __func__, __LINE__);
        aiio_log_i("  host:%s:%d\r\n", mqtt_cfg.uri, mqtt_cfg.port);
        aiio_log_i("  username:%s\r\n", mqtt_cfg.username);
        aiio_log_i("  password:%s\r\n", mqtt_cfg.password);
        aiio_log_i("  clientID:%s\r\n", mqtt_cfg.client_id);

        my_client = aiio_mqtt_client_init(&mqtt_cfg);
        aiio_mqtt_client_start(my_client);

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
    uint8_t ssid_buf[128] = {0};
    uint8_t pass_buf[128] = {0};

    bflb_flash_read(FLASH_RW_START_ADDR, ssid_buf, sizeof(ssid_buf));
    bflb_flash_read(FLASH_RW_START_ADDR + sizeof(ssid_buf), pass_buf, sizeof(pass_buf));

    if ((strlen(ssid_buf) > 120) && (strlen(pass_buf) > 120))
    {
        while (1)
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(1000));
        }
    }

    aiio_log_e("ssid:%s\r\n", ssid_buf);
    aiio_log_e("pass:%s\r\n", pass_buf);

    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            // .ssid = "zifeng02",
            // .password = "123456789",
            .channel = 6,
            .use_dhcp = 1,
            .flags = 0,
        },
    };

    memset(wifi_sta_config.sta.ssid, 0, sizeof(wifi_sta_config.sta.ssid));
    memset(wifi_sta_config.sta.password, 0, sizeof(wifi_sta_config.sta.password));

    strcpy(wifi_sta_config.sta.ssid, ssid_buf);
    strcpy(wifi_sta_config.sta.password, pass_buf);

    aiio_log_e("ssid:%s\r\n", wifi_sta_config.sta.ssid);
    aiio_log_e("pass:%s\r\n", wifi_sta_config.sta.password);

    // strcpy(wifi_sta_config.sta.ssid, "zifeng02");
    // strcpy(wifi_sta_config.sta.password, "123456789");

    aiio_wifi_set_config(AIIO_WIFI_IF_STA, &wifi_sta_config);

    aiio_wifi_start();

    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

static void mqtt_send_task(void *msg)
{
    while (1)
    {
        if (mqtt_status)
        {
            mqtt_data_send("Hello service", 0);
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    }
}

void mqtt_app_init(void)
{
    int ret = 0;
    aiio_wifi_register_event_cb(cb_wifi_event);
    ret = aiio_wifi_init();
    if (ret != 0)
    {
        aiio_log_e("wifi init error!!");
    }

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    aiio_os_thread_create(&aiio_test_mqtt_send_thread, "aiio_test_mqtt_send_thread", mqtt_send_task, 4096, NULL, 14);
}
