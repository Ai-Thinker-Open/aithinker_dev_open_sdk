/**
 * @brief   
 * 
 * @file    aiio_ap_config_wifi.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-13          <td>1.0.0            <td>zhuolm             <td>
 */
#include "aiio_ap_config.h"
#include "aiio_log.h"
#include "aiio_common.h"
#include "aiio_adapter_include.h"
#include "config.h"



#ifdef CONFIG_AP_DISTRIBUTION_NETWORK


static wifi_config_data_t wifi_config_data = {0};
static aiio_rev_queue_t  rev_queue = {0};


static void aiio_ap_iot_config_receive_data(aiio_ap_iot_data_t *ap_data);
static void aiio_wifi_soft_ap_start(aiio_wifi_ap_info_config_t *wifi_ap_config);
static void aiio_wifi_soft_ap_stop(void);




void aiio_ap_config_start(void)
{
    aiio_ap_iot_config_t ap_iot_config = {0};

    ap_iot_config.deviceid = DeviceInfo.deviceId;
    ap_iot_config.productid = PRODUCT_ID;
    ap_iot_config.wifi_flag = PRODUCT_FLAG;
    ap_iot_config.aiio_ap_iot_config_cb = aiio_ap_iot_config_receive_data;
    ap_iot_config.aiio_wifi_ap_start = aiio_wifi_soft_ap_start;
    ap_iot_config.aiio_wifi_ap_stop = aiio_wifi_soft_ap_stop;
    aiio_ap_config_init(&ap_iot_config);
}


void aiio_ap_config_stop(void)
{
    aiio_log_d("aiio_ap_config_deinit \r\n");
    aiio_ap_config_deinit();
    aiio_log_d("aiio_ap_config_deinit \r\n");
}


void aiio_ap_config_notify(uint8_t event)
{
    aiio_wifi_ap_event_notify(event);
}


static void aiio_ap_save_config_data(aiio_ap_data_t *source_data, wifi_config_data_t *destination_data)
{
    int ret = 0;

    if(source_data == NULL || destination_data == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    memset(destination_data, 0, sizeof(wifi_config_data_t));
    if(source_data->ssid)
    {
        memcpy(destination_data->ssid, source_data->ssid, strlen(source_data->ssid));
    }

    if(source_data->passwd)
    {
        memcpy(destination_data->passwd, source_data->passwd, strlen(source_data->passwd));
    }

    if(source_data->wificc)
    {
        memcpy(destination_data->wificc, source_data->wificc, strlen(source_data->wificc));
    }

    if(source_data->mqttip)
    {
        memcpy(destination_data->mqttip, source_data->mqttip, strlen(source_data->mqttip));
    }

    if(source_data->token)
    {
        memcpy(destination_data->token, source_data->token, strlen(source_data->token));
    }

    if(source_data->tz)
    {
        memcpy(destination_data->tz, source_data->tz, strlen(source_data->tz));
    }

    destination_data->ts = source_data->ts;
    destination_data->port = source_data->port;

    aiio_log_d("ssid = %s \r\n", destination_data->ssid);
    aiio_log_d("passwd = %s \r\n", destination_data->passwd);
}



static void aiio_ap_iot_config_receive_data(aiio_ap_iot_data_t *ap_iot_data)
{
    switch (ap_iot_data->event)
    {
        case AIIO_AP_CONFIG_FAIL:
        {
            aiio_log_d("AIIO_AP_CONFIG_FAIL \r\n");

            rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;

        case AIIO_AP_CONFIG_OK:
        {
            aiio_log_d("AIIO_AP_CONFIG_OK \r\n");

            if(ap_iot_data->ap_data->ssid)
            {
                aiio_log_d("ssid = %s \r\n", ap_iot_data->ap_data->ssid);
            }
            if(ap_iot_data->ap_data->passwd)
            {
                aiio_log_d("passwd = %s \r\n", ap_iot_data->ap_data->passwd);
            }
            if(ap_iot_data->ap_data->mqttip)
            {
                aiio_log_d("mqttip = %s \r\n", ap_iot_data->ap_data->mqttip);
            }
            aiio_ap_save_config_data(ap_iot_data->ap_data, &wifi_config_data);
            rev_queue.queue_data = (char *)&wifi_config_data;
            rev_queue.queue_data_len = sizeof(wifi_config_data_t);
            rev_queue.common_event = REV_CONFIG_DATA_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;
        
        default:
            aiio_log_e("can't find event \r\n");
            break;
    }
}


static void aiio_wifi_soft_ap_start(aiio_wifi_ap_info_config_t *wifi_ap_info_config)
{
    int32_t ret=0;
    aiio_wifi_config_t wifi_ap_config = {
                .ap = {
                    .channel=5,
                    .max_connection=5,
                    .ssid_hidden=0,
                    .ssid_len=strlen((char *)wifi_ap_info_config->ssid),
                    .use_dhcp=1,
                },
            };

        memcpy(wifi_ap_config.ap.ssid, wifi_ap_info_config->ssid, strlen((char *)wifi_ap_info_config->ssid));
        memcpy(wifi_ap_config.ap.password, wifi_ap_info_config->password, strlen((char *)wifi_ap_info_config->password));

    ret=aiio_wifi_ap_ip_set("192.168.4.1");
    if(ret != 0)
    {
        aiio_log_e("wifi ap_ip_set error!!");
        return ;
    }

    aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);

    aiio_wifi_set_config(AIIO_WIFI_IF_AP,&wifi_ap_config);

    aiio_wifi_start();
    aiio_log_d("aiio_wifi_start \r\n");
}


static void aiio_wifi_soft_ap_stop(void)
{
    aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);
    aiio_wifi_stop();
    aiio_log_d("aiio_wifi_stop \r\n");
}



#endif




