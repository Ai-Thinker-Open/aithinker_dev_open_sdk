#include "axk_http_server_data_parse.h"
#include "axk_mqtt.h"
#include "axk_wifi_init.h"

#include "cJSON.h"
#include "aiio_log.h"

static int32_t axk_http_server_mqtt_data_parse(const char *data);
static int32_t axk_http_server_wifi_data_parse(const char *data);

axk_http_server_data_parse_t g_axk_http_server_data_parse = {
    .http_server_mqtt_data_parse = axk_http_server_mqtt_data_parse,
    .http_server_wifi_data_parse = axk_http_server_wifi_data_parse,
};


static int32_t axk_http_server_mqtt_data_parse(const char *data)
{
    int32_t ret = -1;
    cJSON *Root, *uri, *topic, *username, *password;

    Root = cJSON_Parse(data);
    if (Root == NULL)
    {
        aiio_log_e("cJSON Parse error!!!");
        goto __exit;
    }

    uri = cJSON_GetObjectItem(Root, "mqttUrl");
    topic = cJSON_GetObjectItem(Root, "topic");
    username = cJSON_GetObjectItem(Root, "mqtt_username");
    password = cJSON_GetObjectItem(Root, "mqtt_password");

    if (uri != NULL)
    {
        g_axk_mqtt.modify_uri((uint8_t *)uri->valuestring);
    }

    if (topic != NULL)
    {
        g_axk_mqtt.modify_user_topic((uint8_t *)topic->valuestring);
    }

    if (username != NULL)
    {
        g_axk_mqtt.modify_username((uint8_t *)username->valuestring);
    }

    if (password != NULL)
    {
        g_axk_mqtt.modify_password((uint8_t *)password->valuestring);
    }

    cJSON_Delete(Root);
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_http_server_wifi_data_parse(const char *data)
{
    int32_t ret = -1;
    cJSON *Root, *ssid, *password, *mqtt_uri, *mqtt_topic, *mqtt_username, *mqtt_password;
    axk_sta_info_t wifi_info = {0};

    Root = cJSON_Parse(data);
    if (Root == NULL)
    {
        aiio_log_e("cJSON Parse error!!!");
        goto __exit;
    }

    ssid = cJSON_GetObjectItem(Root, "Network");
    password = cJSON_GetObjectItem(Root, "Password");

    if (ssid == NULL)
    {
        aiio_log_e("ssid is null!!!");
        goto __param_err;
    }

    if (strlen(ssid->valuestring) >= sizeof(wifi_info.ssid))
    {
        aiio_log_e("ssid is too long!!!");
        goto __param_err;
    }

    strcpy((char *)wifi_info.ssid, ssid->valuestring);

    if (password != NULL)
    {
        if (strlen(password->valuestring) < sizeof(wifi_info.pwd))
        {
            strcpy((char *)wifi_info.pwd, password->valuestring);
        }
    }

    //mqtt info
    mqtt_uri = cJSON_GetObjectItem(Root, "mqttUrl");
    mqtt_topic = cJSON_GetObjectItem(Root, "topic");
    mqtt_username = cJSON_GetObjectItem(Root, "mqtt_username");
    mqtt_password = cJSON_GetObjectItem(Root, "mqtt_password");

    if ((mqtt_uri != NULL) && (mqtt_topic != NULL) && (mqtt_username != NULL) && (mqtt_password != NULL))
    {
        if (strlen(mqtt_uri->valuestring) > 0 && strlen(mqtt_topic->valuestring) > 0 &&
            strlen(mqtt_username->valuestring) > 0 && strlen(mqtt_password->valuestring) > 0)
        {
            g_axk_mqtt.modify_uri((uint8_t *)mqtt_uri->valuestring);
            g_axk_mqtt.modify_user_topic((uint8_t *)mqtt_topic->valuestring);
            g_axk_mqtt.modify_username((uint8_t *)mqtt_username->valuestring);
            g_axk_mqtt.modify_password((uint8_t *)mqtt_password->valuestring);

            // set flag
            g_axk_mqtt.set_info_modify_flag(1);
        }
    }

    g_axk_wifi.set_sta_info((const axk_sta_info_t *)&wifi_info);

    ret = 0;

__param_err:
    cJSON_Delete(Root);

__exit:
    return ret;
}


