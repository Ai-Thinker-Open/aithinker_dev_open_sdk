#include "axk_blufi.h"
#include "axk_at.h"
#include "axk_at_cmd.h"
#include "axk_nvs_info.h"
#include "axk_wifi_init.h"
#include "axk_mqtt.h"

#include "aiio_log.h"
#include "aiio_wifi.h"
#include "aiio_blufi.h"
#include "cJSON.h"

#include <stdarg.h>

static void axk_blufi_init_task(void *param);
static int32_t axk_blufi_init(void);

static uint8_t axk_blufi_start_flag = 0;

int at_setup_blufi_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;

    if (argc > 2)
    {
        aiio_log_e("argc err(%d)", argc);
        goto __exit;
    }

    int ctrl = atoi(argv[0]);
    if (ctrl != 0 && ctrl != 1)
    {
        aiio_log_e("argv[0] err(%d)", ctrl);
        goto __exit;
    }

    if (ctrl == 0)
    {
        if (axk_blufi_start_flag == 1)
        {
            aiio_blufi_cb_t blufi_cb = {0};
            aiio_blufi_register_cb(blufi_cb);
            aiio_blufi_stop();

            axk_blufi_start_flag = 0;
        }
    }
    else
    {
        //start
        if (axk_blufi_start_flag == 0)
        {
            axk_blufi_init();
        }
    }

    AT_RESPONSE_OK;
    ret = 0;

__exit:
    return ret;
}

static int32_t axk_blufi_custom_data_parse(uint8_t *data)
{
    int32_t ret = -1;
    cJSON *Root, *uri, *topic, *name, *pwd;

    Root = cJSON_Parse((const char *)data);
    if (Root == NULL)
    {
        aiio_log_e("cJSON Parse error!!!");
        goto __exit;
    }

    uri = cJSON_GetObjectItem(Root, "uri");
    topic = cJSON_GetObjectItem(Root, "topic");
    name = cJSON_GetObjectItem(Root, "name");
    pwd = cJSON_GetObjectItem(Root, "pwd");

    if ((uri != NULL) && (topic != NULL) && (name != NULL) && (pwd != NULL))
    {
        if (strlen(uri->valuestring) > 0 && strlen(topic->valuestring) > 0 &&
            strlen(name->valuestring) > 0 && strlen(pwd->valuestring) > 0)
        {
            g_axk_mqtt.modify_uri((uint8_t *)uri->valuestring);
            g_axk_mqtt.modify_user_topic((uint8_t *)topic->valuestring);
            g_axk_mqtt.modify_username((uint8_t *)name->valuestring);
            g_axk_mqtt.modify_password((uint8_t *)pwd->valuestring);

            // set flag
            g_axk_mqtt.set_info_modify_flag(1);
        }
    }

    ret = 0;
    cJSON_Delete(Root);

__exit:
    return ret;
}

static void axk_blufi_ble_event_callback(aiio_blufi_cb_event_t event, aiio_blufi_cb_param_t *param)
{
    switch (event)
    {
    case AIIO_BLUFI_EVENT_RECV_CUSTOM_DATA:
    {
        aiio_log_i("custom data: %s", param->custom_data.data);
        axk_blufi_custom_data_parse(param->custom_data.data);
        break;
    }
    default:
        break;
    }
}

int32_t axk_blufi_wifi_callback(const char *ssid, const char *pwd)
{
    axk_sta_info_t sta_info = {0};

    g_nvs_info.set_wifi_mode(AXK_WIFI_STA);

    aiio_log_i("blufi ssid:%s", ssid);

    if (ssid != NULL)
    {
        if (strlen(ssid) < sizeof(sta_info.ssid))
        {
            strcpy((char *)sta_info.ssid, ssid);
        }
    }

    if (pwd != NULL)
    {
        if (strlen(pwd) < sizeof(sta_info.pwd))
        {
            strcpy((char *)sta_info.pwd, pwd);
        }
    }

    aiio_wifi_channel_get(&sta_info.channel);
    aiio_log_i("channel: %u", sta_info.channel);

    g_axk_wifi.set_sta_info(&sta_info);
    g_nvs_info.set_sta_info(sta_info);

    return 0;
}

static void axk_blufi_init_task(void *param)
{
    aiio_blufi_cb_t blufi_cb = {
        .ble_event_cb = axk_blufi_ble_event_callback,
        .wifi_event_cb = axk_blufi_wifi_callback,
    };

    aiio_blufi_register_cb(blufi_cb);
    aiio_blufi_start();

    aiio_os_thread_delete(NULL);
}

static int32_t axk_blufi_init(void)
{
    int32_t ret = -1;

    ret = aiio_os_thread_create(NULL, "blufi_start", axk_blufi_init_task, 2048, NULL, 15);
    if (ret != 0)
    {
        aiio_log_e("create blufi_start task failed!!!");
        goto __exit;
    }

    axk_blufi_start_flag = 1;

__exit:
    return ret;
}