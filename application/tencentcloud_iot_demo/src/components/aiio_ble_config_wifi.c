/**
 * @brief   Apply BLE distribution network protocol to define some interfaces applying BLE distribution network function
 * 
 * @file    aiio_ble_config_wifi.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        This file is mainly describing applying ble distribution network function
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface of applying ble distribution network function
 */
#include "aiio_ble_config_wifi.h"
#include "aiio_ble_config_service.h"
#include "aiio_log.h"
#include "aiio_common.h"
#include "aiio_adapter_include.h"




wifi_config_data_t wifi_config_data = {0};
static aiio_rev_queue_t  rev_queue = {0};


static void aiio_ble_iot_config_cb(aiio_ble_iot_data_t *ble_data);
static void aiio_ble_save_config_data(aiio_ble_data_t *source_data, wifi_config_data_t *destination_data);



void aiio_ble_config_start(void)
{
    aiio_ble_iot_config_t ble_iot_config = {0};

    ble_iot_config.aiio_ble_iot_config_cb = aiio_ble_iot_config_cb;
    aiio_ble_config_init_tencent(&ble_iot_config);
}



void aiio_ble_config_stop(void)
{
    aiio_log_i("aiio_ble_config_stop \r\n");
    aiio_ble_config_deinit_tencent();
    memset(&wifi_config_data, 0, sizeof(wifi_config_data_t));
}


static void aiio_ble_iot_config_cb(aiio_ble_iot_data_t *ble_data)
{
    switch (ble_data->event)
    {
        case AIIO_BLE_CONFIG_FAIL:                                                  /* The BLE distribution network failed, and the failed event was sent through the queue */
        {
            aiio_log_i("AIIO_BLE_CONFIG_FAIL \r\n");
            // rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
            // if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            // {
            //     aiio_log_i("queue send failed\r\n");
            // }
            // memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;

        case AIIO_BLE_DISCONNECED:   
        {
            aiio_log_i("AIIO_BLE_DISCONNECED \r\n");
            // rev_queue.common_event = REV_BLE_DISCONNECTED_EVENT;
            // if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            // {
            //     aiio_log_i("queue send failed\r\n");
            // }
            // memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;

        case AIIO_BLE_CONFIG_OK:                                                    /*The BLE distribution network is successful, and the successful events and distribution network data are sent through the queue */
        {
            aiio_log_i("AIIO_BLE_CONFIG_OK \r\n");
            if(ble_data->ble_data->ssid)
            {
                aiio_log_d("ssid = %s \r\n", ble_data->ble_data->ssid);
            }
            if(ble_data->ble_data->passwd)
            {
                aiio_log_d("passwd = %s \r\n", ble_data->ble_data->passwd);
            }
            if(ble_data->ble_data->mqttip)
            {
                aiio_log_d("mqttip = %s \r\n", ble_data->ble_data->mqttip);
            }
            rev_queue.queue_data = (char *)&wifi_config_data;
            rev_queue.queue_data_len = sizeof(wifi_config_data_t);
            // rev_queue.common_event = REV_CONFIG_DATA_EVENT;
            // if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            // {
            //     aiio_log_i("queue send failed\r\n");
            // }
            // memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;
        
        default:
            aiio_log_e("not find event \r\n");
            break;
    }
}




