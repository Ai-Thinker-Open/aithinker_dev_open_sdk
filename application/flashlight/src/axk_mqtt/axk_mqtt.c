#include "axk_mqtt.h"
#include "aiio_os_port.h"
#include "aiio_mqtt_client.h"
#include "aiio_log.h"
#include "aiio_autoconf.h"
#include "axk_at.h"
#include "axk_ringbuff.h"
#include "axk_device_info.h"
#include "axk_ota.h"
#include "axk_nvs_info.h"
#include "axk_uart_init.h"

#include "cJSON.h"

#include <stdarg.h>

#define MQTT_URI            CONFIG_AXK_MQTT_URI
#define MQTT_USERNAME       CONFIG_AXK_MQTT_USERNAME
#define MQTT_PASSWORD       CONFIG_AXK_MQTT_PASSWORD
#define MQTT_CLIENT_ID      CONFIG_AXK_MQTT_CLIENTID
#define MQTT_USER_TOPIC     CONFIG_AXK_MQTT_USER_TOPIC

//publish
#define DEVICE_INFO_TOPIC   "sys/active/device/info"

static int32_t axk_mqtt_init(void);
static int32_t axk_mqtt_deinit(void);
static int32_t axk_mqtt_modify_uri(uint8_t *uri);
static int32_t axk_mqtt_modify_user_name(uint8_t *username);
static int32_t axk_mqtt_modify_password(uint8_t *password);
static int32_t axk_mqtt_modify_client_id(uint8_t *id);
static int32_t axk_mqtt_modify_user_topic(uint8_t *topic);
static aiio_err_t mqtt_event_cb(aiio_mqtt_event_handle_t event);
static uint8_t axk_mqtt_get_start_flag(void);
static uint8_t axk_mqtt_get_info_modify_flag(void);
static void axk_mqtt_set_info_modify_flag(uint8_t ctrl);
static void axk_mqtt_set_info_default(void);
static axk_mqtt_info_t *axk_mqtt_get_conn_info(void);

static axk_mqtt_info_t mqtt_info = {
    .uri = {MQTT_URI},
    .user_name = {MQTT_USERNAME},
    .password = {MQTT_PASSWORD},
    // .client_id = {MQTT_CLIENT_ID},
    .flashlight_topic = {MQTT_USER_TOPIC},
};

static uint8_t mqtt_client_id[64] = {MQTT_CLIENT_ID};
static aiio_os_thread_handle_t mqtt_init_handle = NULL;
static uint8_t mqtt_start_flag = 0;
static uint8_t mqtt_info_modify_flag = 0;

static aiio_mqtt_client_handle_t g_client = NULL;

static uint8_t mqtt_connect_flag = 0;
static uint8_t publish_status = 0;
static uint8_t sub_status = 0;
static axk_mqtt_publish_t mqtt_publish_info = {0};

static aiio_mqtt_client_config_t mqtt_cfg = {
    .uri = (const char *)mqtt_info.uri,
    .username = (const char *)mqtt_info.user_name,
    .password = (const char *)mqtt_info.password,
    .client_id = (const char *)mqtt_client_id,
    .event_handle = mqtt_event_cb,
};

axk_mqtt_t g_axk_mqtt = {
    .init = axk_mqtt_init,
    .deinit = axk_mqtt_deinit,
    .modify_uri = axk_mqtt_modify_uri,
    .modify_username = axk_mqtt_modify_user_name,
    .modify_password = axk_mqtt_modify_password,
    .modify_client_id = axk_mqtt_modify_client_id,
    .modify_user_topic = axk_mqtt_modify_user_topic,
    .get_start_flag = axk_mqtt_get_start_flag,
    .get_info_modify_flag = axk_mqtt_get_info_modify_flag,
    .set_info_modify_flag = axk_mqtt_set_info_modify_flag,
    .set_info_default = axk_mqtt_set_info_default,
    .get_conn_info = axk_mqtt_get_conn_info,
};

static int32_t axk_mqtt_publish_device_info(aiio_mqtt_client_handle_t client)
{
    int32_t ret = -1;
    cJSON *root = cJSON_CreateObject();
    if (root == NULL)
    {
        aiio_log_e("root is null");
        goto __exit;
    }

    cJSON_AddStringToObject(root, "mac", (const char *)g_axk_device_info.mac_str);
    cJSON_AddStringToObject(root, "fv", FW_VER);
    cJSON_AddStringToObject(root, "mv", (const char *)g_axk_device_info.mcu_ver);
    cJSON_AddStringToObject(root, "pCode", (const char *)g_axk_device_info.productCode);
    cJSON_AddStringToObject(root, "dCode", (const char *)g_axk_device_info.deviceCode);

    char *json_str = cJSON_Print(root);
    aiio_log_i("device info: %s", json_str);
    aiio_mqtt_client_publish(client, DEVICE_INFO_TOPIC, (const char *)json_str, strlen(json_str), 1, 0);
    cJSON_Delete(root);
    aiio_os_free(json_str);

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_mqtt_sub_upgrade_topic(aiio_mqtt_client_handle_t client)
{
    char sub_topic[128] = {0};
    sprintf(sub_topic, "sys/ota/device/upgrade/%s", g_axk_device_info.mac_str);
    aiio_log_i("ota topic: %s", sub_topic);

    return aiio_mqtt_client_subscribe(client, sub_topic, 0);
}

// static int32_t axk_mqtt_sub_device_info_topic(aiio_mqtt_client_handle_t client)
// {
//     char sub_topic[128] = {0};
//     sprintf(sub_topic, "sys/active/device/info/%s", g_axk_device_info.mac_str);
//     aiio_log_i("device info topic: %s", sub_topic);

//     return aiio_mqtt_client_subscribe(client, sub_topic, 0);
// }

int axk_hal_mqtt_pub(const char *msg, int length, int qos, int retain)
{
    // AT+MQTTPUB=<LinkID>,<"topic">,<"data">,<qos>,<retain>
    int16_t wait_pub_result_time_ms = 10000;   // 等待10s

    aiio_log_i("msg length=%d\r\n", length);
    aiio_log_i("msg=%.*s\r\n", length, msg);
    int ret = aiio_mqtt_client_publish(g_client, (const char *)mqtt_info.flashlight_topic, msg, length, qos, retain);
    if (ret == -1)
    {
        aiio_log_e("esp_mqtt_client_publish failed\r\n");
        return -1;
    }
    else
    {
        aiio_log_i("esp_mqtt_client_publish done\r\n");
    }

    if (qos == 0)
    {
        return 0;
    }

    publish_status = 0;
    while (wait_pub_result_time_ms > 0)
    {
        if (publish_status == 1)
        {
            return 0;
        }
        
        wait_pub_result_time_ms -= 10;
        aiio_os_tick_dealy(10);
    }
    
    return -1;
}

static void _at_mqtt_rawpub_callback(char data)
{
    int ret = 0;

    /*
        {
            "pCode":"AS506",
            "dCode":"042306280819001",
            "fv":"1.0.1",
            "mv":"1.0.1",
            "tf":10,
            "seq":1,
            "data":"设备发送的原16进制数据包"
        }
        长度：11+strlen(pCode) + 11+strlen(dCode) + 13fv + 13mv + 6+(tf(2)) + 7+(seq(2)) + 9+(data) + 2({})
    */

    //totalFrame、seq
    if (mqtt_publish_info.pos < mqtt_publish_info.length)
    {
        if (mqtt_publish_info.pos == 0)
        {
            mqtt_publish_info.buff_index = sprintf((char *)mqtt_publish_info.buff, "{\"pCode\":\"%s\",\"dCode\":\"%s\",\"fv\":\"%s\",\"mv\":\"%s\",\"tf\":%d,\"seq\":",
                                                   g_axk_device_info.productCode, g_axk_device_info.deviceCode, FW_VER, g_axk_device_info.mcu_ver, data);
        }
        else if (mqtt_publish_info.pos == 1)
        {
            mqtt_publish_info.buff_index += sprintf((char *)&mqtt_publish_info.buff[mqtt_publish_info.buff_index], "%d,\"data\":\"", data);
        }
        else
        {
            mqtt_publish_info.buff[mqtt_publish_info.buff_index++] = data;
        }

        // mqtt_publish_info.buff[mqtt_publish_info.pos] = data;
        mqtt_publish_info.pos++;
    }

    if (mqtt_publish_info.pos < mqtt_publish_info.length)
    {
        return;
    }

    mqtt_publish_info.buff[mqtt_publish_info.buff_index++] = '\"';
    mqtt_publish_info.buff[mqtt_publish_info.buff_index++] = '}';

    // if (mqtt_publish_info.pos < mqtt_publish_info.length)
    // {
    //     mqtt_publish_info.buff[mqtt_publish_info.pos] = data;
    //     mqtt_publish_info.pos++;
    // }
    // if (mqtt_publish_info.pos < mqtt_publish_info.length)
    // {
    //     return;
    // }

    // ret = axk_hal_mqtt_pub((const char *)mqtt_publish_info.buff, mqtt_publish_info.length, 1, 0);
    ret = axk_hal_mqtt_pub((const char *)mqtt_publish_info.buff, mqtt_publish_info.buff_index, 0, 0);

    if (g_ringbuff_ctrl.get_size() > 0)
    {
        axk_at_printf("\r\nbusy p...\r\n");
        g_ringbuff_ctrl.flush();
    }

    if (ret != 0)
    {
        axk_at_printf("+MQTTPUB:FAIL\r\n");
    }
    else
    {
        axk_at_printf("+MQTTPUB:OK\r\n");
    }

    if (mqtt_publish_info.buff != NULL)
    {
        aiio_os_free(mqtt_publish_info.buff);
        mqtt_publish_info.buff = NULL;
    }

    at_port_exit_specific();
}

int at_setup_mqtt_rawpub_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;
    char* argvp;
    axk_mqtt_publish_t mqtt_pub = {0};

#if 0
    if (argc != 5)
    {
        aiio_log_e("argc error!!!");
        goto __exit;
    }

    if (mqtt_connect_flag != 1)
    {
        aiio_log_e("mqtt is not connected!!!");
        goto __exit;
    }

    argvp = (char*)argv[0];
    int link_id = atoi(argvp);
    if (link_id != 0)
    {
        aiio_log_e("link id error!!!");
        goto __exit;
    }
    mqtt_pub.link_id = link_id;

    argvp = (char*)argv[1];
    if (argvp[0] != '\"' && argvp[strlen(argvp) - 1] != '\"')
    {
        aiio_log_e("topic format error!!!");
        goto __exit;
    }
    memcpy(mqtt_pub.topic, &argvp[1], strlen(argvp) - 2);

    argvp = (char*)argv[2];
    mqtt_pub.length = atoi(argvp);
    if (mqtt_pub.length <= 0)
    {
        aiio_log_e("len error(%d)!!!", mqtt_pub.length);
        goto __exit;
    }

    argvp = (char*)argv[3];
    int qos = atoi(argvp);
    if (qos > 2 || qos < 0)
    {
        aiio_log_e("qos error(%d)!!!", qos);
        goto __exit;
    }
    mqtt_pub.qos = qos;

    argvp = (char*)argv[4];
    int retain = atoi(argvp);
    if (retain > 2 || retain < 0)
    {
        aiio_log_e("retain error(%d)!!!", qos);
        goto __exit;
    }
    mqtt_pub.retain = retain;

#endif
    if (argc != 1)
    {
        aiio_log_e("argc error!!!");
        goto __exit;
    }

    if (mqtt_connect_flag != 1)
    {
        aiio_log_e("mqtt is not connected!!!");
        goto __exit;
    }

    argvp = (char*)argv[0];
    mqtt_pub.length = atoi(argvp);
    if (mqtt_pub.length <= 0)
    {
        aiio_log_e("len error(%d)!!!", mqtt_pub.length);
        goto __exit;
    }

    /*
        {
            "pCode":"AS506",
            "dCode":"042306280819001",
            "fv":"1.0.1",
            "mv":"1.0.1",
            "tf":10,
            "seq":1,
            "data":"设备发送的原16进制数据包"
        }
        长度：11+strlen(pCode) + 11+strlen(dCode) + 13fv + 13mv + 6+(tf(2)) + 7+(seq(2)) + 9+(data) + 2({})
    */
    int len = 11 + strlen((const char *)g_axk_device_info.productCode) + 11 + strlen((const char *)g_axk_device_info.deviceCode) + 13 + 13 + 6 + 2 + 7 + 2 + 9 + mqtt_pub.length - 2 + 2;
    aiio_log_i("malloc len:%d", len);
    mqtt_pub.buff = aiio_os_malloc(len);
    if (mqtt_pub.buff == NULL)
    {
        aiio_log_e("malloc mqtt_pub.length error, len(%d)!!!", len);
        goto __exit;
    }


#if 0
    mqtt_pub.buff = aiio_os_malloc(mqtt_pub.length);
    if (mqtt_pub.buff == NULL)
    {
        aiio_log_e("malloc mqtt_pub.length error, len(%d)!!!", mqtt_pub.length);
        goto __exit;
    }
#endif
    memcpy(&mqtt_publish_info, &mqtt_pub, sizeof(axk_mqtt_publish_t));

    axk_at_printf("OK\r\n>");

    at_port_enter_specific(_at_mqtt_rawpub_callback);

    ret = 0;

__exit:
    return ret;
}

int axk_setup_mqtt_sub_cmd(uint32_t argc, const char** argv)
{
    // AT+MQTTSUB=<"topic">,<qos>
    // 若已订阅过该 topic，则返回：ALREADY SUBSCRIBE
    // 当 AT 接收到已订阅的 topic 的 MQTT 消息时，返回：+MQTTSUBRECV:<"topic">,<data_length>,data
    int ret = -1;
    char *topic = NULL;
    int qos = 0;
    int16_t wait_sub_result_time_ms = 10000;

    if (!mqtt_connect_flag)
    {
        goto out;
    }

    if (argc < 1 || argc > 2)
    {
        goto out;
    }

    topic = argv[0];
    if (argc == 2)
    {
        qos = atoi(argv[1]);
    }

    sub_status = 0;

    // 需要把订阅成功的主题保存起来，以此来判断曾经是否订阅
    ret = aiio_mqtt_client_subscribe(g_client, topic, qos);
    if (ret == -1)
    {
        goto out;
    }

    while (wait_sub_result_time_ms > 0)
    {
        if (sub_status)
        {
            ret = 0;
            AT_RESPONSE_OK;
            goto out;
        }
        
        wait_sub_result_time_ms -= 10;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ret = -1;

out:
    return ret;
}

// AT+MQTTPUB=<LinkID>,<"topic">,<"data">,<qos>,<retain>
int axk_setup_mqtt_pub_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;
    int qos = 0, retain = 0;
    int len = 0;
    uint8_t *data = NULL;

    int16_t wait_pub_result_time_ms = 10000;   // 等待10s

    if (!mqtt_connect_flag)
    {
        goto out;
    }

    if (argc < 2)
    {
        goto out;
    }

    if (argc > 2)
    {
        qos = atoi(argv[2]);
        if (qos > 2 || qos < 0)
        {
            goto out;
        }
    }

    if (argc > 3)
    {
        retain = atoi(argv[3]);
        if (retain != 0 && retain != 1)
        {
            goto out;
        }
    }

    publish_status = 0;
     /*
        {
            "pCode":"AS506",
            "dCode":"042306280819001",
            "fv":"1.0.1",
            "mv":"1.0.1",
            "tf":10,
            "seq":1,
            "data":"设备发送的原16进制数据包"
        }
        长度：11+strlen(pCode) + 11+strlen(dCode) + 13fv + 13mv + 6+(tf(2)) + 7+(seq(2)) + 9+(data) + 2({})
    */

    len = 11 + strlen((const char *)g_axk_device_info.productCode) + 11 + strlen((const char *)g_axk_device_info.deviceCode) + 13 + 13 + 6 + 2 + 7 + 2 + 9 + strlen(argv[1]) + 2;
    data=malloc(len);
    memset(data,0,len);
    sprintf((char *)data, "{\"pCode\":\"%s\",\"dCode\":\"%s\",\"fv\":\"%s\",\"mv\":\"%s\",\"tf\":10,\"seq\":1,\"data\":\"%s\"}",
                                                   g_axk_device_info.productCode, g_axk_device_info.deviceCode, FW_VER, g_axk_device_info.mcu_ver, argv[1]);

    ret = aiio_mqtt_client_publish(g_client, argv[0], (char *)data, strlen((char *)data), qos, retain);
    if (ret == -1)
    {
        aiio_log_e("esp_mqtt_client_publish failed\r\n");
        goto out;
    }

    if (qos == 0)   // qos0 不会进入回调
    {
        ret = 0;
        AT_RESPONSE_OK;
        goto out;
    }

    while (wait_pub_result_time_ms > 0)
    {
        if (publish_status)
        {
            ret = 0;
            AT_RESPONSE_OK;
            goto out;
        }
        
        wait_pub_result_time_ms -= 10;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    return AT_ERR_FAILURE;

out:
    free(data);
    return ret;
}

static void axk_mqtt_init_task(void *param)
{
    aiio_log_i("uri:%s\r\n", mqtt_cfg.uri);
    aiio_log_i("username:%s\r\n", mqtt_cfg.username);
    aiio_log_i("password:%s\r\n", mqtt_cfg.password);
    aiio_log_i("clientID:%s\r\n", mqtt_cfg.client_id);

    g_client = aiio_mqtt_client_init(&mqtt_cfg);
    if (g_client == NULL)
    {
        aiio_log_e("mqtt g_client init failed!!!");
        goto __exit;
    }

    aiio_err_t err = aiio_mqtt_client_start(g_client);
    if (err != AIIO_OK)
    {
        aiio_log_e("mqtt g_client start failed!!!");
        goto __exit;
    }

    mqtt_start_flag = 1;

    aiio_log_i("mqtt g_client start done");

__exit:
    mqtt_init_handle = NULL;
    aiio_os_thread_delete(NULL);
}

static uint8_t axk_mqtt_get_start_flag(void)
{
    return mqtt_start_flag;
}

static int32_t axk_mqtt_init(void)
{
    int32_t ret = -1;

    // if (g_client == NULL)
    // {
    //     ret = aiio_os_thread_create(NULL, "mqtt_init", axk_mqtt_init_task, 2048, NULL, 10);
    //     if (ret != AIIO_OK)
    //     {
    //         aiio_log_e("create mqtt task failed!!!");
    //     }
    // }
    // else
    // {
    //     aiio_log_w("mqtt already initialized");
    //     ret = AIIO_OK;
    // }
    if (mqtt_init_handle != NULL)
    {
        aiio_log_e("mqtt_init_handle is not null");
        goto __exit;
    }

    if (g_client != NULL)
    {
        aiio_mqtt_client_destroy(g_client);
        g_client = NULL;
    }

    ret = aiio_os_thread_create(&mqtt_init_handle, "mqtt_init", axk_mqtt_init_task, 2048, NULL, 10);
    if (ret != AIIO_OK)
    {
        aiio_log_e("create mqtt task failed!!!");
        goto __exit;
    }

__exit:
    return ret;
}

static int32_t axk_mqtt_deinit(void)
{
    int32_t ret;

    ret = aiio_mqtt_client_destroy(g_client);

    if (ret != AIIO_OK)
    {
        aiio_log_e("mqtt destroy faied!!!");
        goto __exit;
    }

    aiio_log_i("mqtt destroy succeed!!!");
    g_client = NULL;

__exit:
    return ret;
}

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        aiio_log_e("Last error %s: 0x%x", message, error_code);
    }
}

static void axk_mqtt_conn_cb(aiio_mqtt_client_handle_t client)
{
    axk_at_printf("+MQTTCONNECTED\r\n");
    mqtt_connect_flag = 1;

    axk_mqtt_publish_device_info(client);
    //订阅主题
    axk_mqtt_sub_upgrade_topic(client);
    // axk_mqtt_sub_device_info_topic(client);
    //保存mqtt信息
    if (g_axk_mqtt.get_info_modify_flag() != 0)
    {
        g_nvs_info.set_mqtt_info(&mqtt_info);
        uint8_t save_flag = 1;
        g_nvs_info.set_mqtt_save_flag(&save_flag);
        g_axk_mqtt.set_info_modify_flag(0);
    }
}

static void axk_mqtt_disconn_cb(void)
{
    axk_at_printf("+MQTTDISCONNECTED\r\n");
    mqtt_connect_flag = 0;
}

static void axk_mqtt_data_event_cb(aiio_mqtt_event_handle_t event)
{
    char topic[64] = {0};
    char upgrade_topic[64] = {0};
    snprintf(upgrade_topic, sizeof(upgrade_topic), "sys/ota/device/upgrade/%s", g_axk_device_info.mac_str);

    snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
    if (strcmp(topic, upgrade_topic) == 0)
    {
        g_axk_ota.remote_data_parse((uint8_t *)event->data, event->data_len);
    }
    else
    {
        #define OTHER_LEN        (17)   // 固定长度
        char *data = NULL;
        int data_len = event->data_len;

        if (event->current_data_offset == 0)    // 第一包
        {
            int temp_len = event->total_data_len;
            data_len += OTHER_LEN;          // 固定长度
            data_len += event->topic_len;   // 主题名称长度
            // 求数据长度这个数值的位数
            while (temp_len > 0)
            {
                data_len += 1;
                temp_len /= 10;
            }
        }

        aiio_log_d("malloc data len=%d\r\n", data_len);

        data = (char *)malloc(data_len);
        if (data == NULL)
        {
            aiio_log_e("malloc sub_recv data failed\r\n");
            return;
        }

        memset(data, 0, data_len);

        if (event->current_data_offset == 0)    // 第一包
        {
            int cnt = snprintf(data, data_len, "+MQTTSUBRECV:\"%.*s\",%d,", event->topic_len, event->topic, event->total_data_len);
            memcpy(&data[cnt], event->data, event->data_len);
        }
        else
        {
            memcpy(&data[0], event->data, event->data_len);
        }

        // vTaskEnterCritical();
        g_uart.send_data((uint8_t *)data, data_len);

        // 最后一包
        if (event->data_len + event->current_data_offset == event->total_data_len)
        {
            AT_ENTER;
        }
        // vTaskExitCritical();

        free(data);

        #undef OTHER_LEN
    }
}

static aiio_err_t mqtt_event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    aiio_mqtt_client_handle_t client = event->client;

    event_id = event->event_id;
    aiio_log_i("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        aiio_log_i("MQTT_EVENT_CONNECTED");
        axk_mqtt_conn_cb(client);
        break;
    case MQTT_EVENT_DISCONNECTED:
        aiio_log_i("MQTT_EVENT_DISCONNECTED");
        axk_mqtt_disconn_cb();
        break;
    case MQTT_EVENT_SUBSCRIBED:
        aiio_log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        sub_status = 1;
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        aiio_log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:  //qos0 无此回调
        aiio_log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        publish_status = 1;
        break;
    case MQTT_EVENT_DATA:
        aiio_log_i("MQTT_EVENT_DATA");
        aiio_log_i("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        aiio_log_i("DATA=%.*s\r\n", event->data_len, event->data);
        axk_mqtt_data_event_cb(event);
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

static int32_t axk_mqtt_modify_uri(uint8_t *uri)
{
    int32_t ret = -1;

    if (uri == NULL)
    {
        goto __exit;
    }

    if (strlen((const char *)uri) > sizeof(mqtt_info.uri))
    {
        goto __exit;
    }

    strcpy((char *)mqtt_info.uri, (const char *)uri);
    aiio_log_i("mqtt uri:%s", mqtt_info.uri);
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_mqtt_modify_user_name(uint8_t *username)
{
    int32_t ret = -1;

    if (username == NULL)
    {
        goto __exit;
    }

    if (strlen((const char *)username) > sizeof(mqtt_info.user_name))
    {
        goto __exit;
    }

    strcpy((char *)mqtt_info.user_name, (const char *)username);
    aiio_log_i("mqtt username:%s", mqtt_info.user_name);
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_mqtt_modify_password(uint8_t *password)
{
    int32_t ret = -1;

    if (password == NULL)
    {
        goto __exit;
    }

    if (strlen((const char *)password) > sizeof(mqtt_info.password))
    {
        goto __exit;
    }

    strcpy((char *)mqtt_info.password, (const char *)password);
    aiio_log_i("mqtt password:%s", mqtt_info.password);
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_mqtt_modify_client_id(uint8_t *id)
{
    int32_t ret = -1;

    if (id == NULL)
    {
        goto __exit;
    }

    if (strlen((const char *)id) > sizeof(mqtt_client_id))
    {
        goto __exit;
    }

    strcpy((char *)mqtt_client_id, (const char *)id);
    aiio_log_i("mqtt clientid:%s", mqtt_client_id);
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_mqtt_modify_user_topic(uint8_t *topic)
{
    int32_t ret = -1;

    if (topic == NULL)
    {
        goto __exit;
    }

    if (strlen((const char *)topic) > sizeof(mqtt_info.flashlight_topic))
    {
        goto __exit;
    }

    strcpy((char *)mqtt_info.flashlight_topic, (const char *)topic);
    aiio_log_i("mqtt user topic:%s", mqtt_info.flashlight_topic);
    ret = 0;

__exit:
    return ret;
}

static uint8_t axk_mqtt_get_info_modify_flag(void)
{
    return mqtt_info_modify_flag;
}

static void axk_mqtt_set_info_modify_flag(uint8_t ctrl)
{
    mqtt_info_modify_flag = ctrl;
}

static void axk_mqtt_set_info_default(void)
{
    strncpy((char *)mqtt_info.uri, MQTT_URI, sizeof(mqtt_info.uri) - 1);
    strncpy((char *)mqtt_info.user_name, MQTT_USERNAME, sizeof(mqtt_info.user_name) - 1);
    strncpy((char *)mqtt_info.password, MQTT_PASSWORD, sizeof(mqtt_info.password) - 1);
    strncpy((char *)mqtt_info.flashlight_topic, MQTT_USER_TOPIC, sizeof(mqtt_info.flashlight_topic) - 1);
}

static axk_mqtt_info_t *axk_mqtt_get_conn_info(void)
{
    return &mqtt_info;
}
