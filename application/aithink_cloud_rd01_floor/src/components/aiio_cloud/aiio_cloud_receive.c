/**
 * @brief   Define some interface about cloud, such as Lauch function of cloud or some callback function
 * 
 * @file    aiio_cloud_receive.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note        This file is mainly desctrbing cloud interface
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-06-16          <td>1.0.0            <td>zhuolm             <td> The interface about cloud
 */
#include "aiio_adapter_include.h"
#include "aiio_wan_service.h"
#include "aiio_ble_config.h"
#include "aiio_common.h"
#include "config.h"
#include "aiio_ota_transmit.h"


#ifdef CONFIG_WAN_NETWORK_MODULE


#define         DEVICE_TYPE                 CONFIG_DEVICE_TYPE                   /*!< 设置该固件运行于哪个芯片平台上 */



static bool service_init_state = false;
static bool cloud_connect = false;
bool device_host_slave = false;



/* The callback function of receive mqtt data*/
static void aiio_service_receive_data(aiio_service_rev_event_t *rev_event)
{
    aiio_rev_queue_t rev_queue = {0};

    if(rev_event == NULL)
    {
        aiio_log_i("receive event is NULL \r\n");
        return ;
    }

    if(rev_event->data)
    {
        aiio_log_i("receive data: %s \r\n", rev_event->data);
    }

    switch (rev_event->service_event)
    {
        case AIIO_SERVICE_LANCH_FAIL_EVNET:                                             /* mqtt connection fail*/
        {
            aiio_log_i("AIIO_SERVICE_LANCH_FAIL_EVNET \r\n");

#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(wifi_config_start)
            {
                if(ble_config_start)        aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);
                
                rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_i("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            }
#endif
        }
        break;

        case AIIO_SERVICE_SUBSCRIBE_TIMEOUT:                                            /* The mqtt subscription is timeout*/
        {
            aiio_log_i("AIIO_SERVICE_SUBSCRIBE_TIMEOUT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(wifi_config_start)
            {
                if(ble_config_start)        aiio_ble_config_response_status(AIIO_BLE_CODE_FAIL);

                rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_i("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            }
#endif
        }
        break;

        case AIIO_SERVICE_MQTT_DISCONNECT:                                                            
        {
            aiio_log_i("AIIO_SERVICE_MQTT_DISCONNECT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(wifi_config_start && !cloud_connect)
            {
                if(ble_config_start)        aiio_ble_config_response_status(AIIO_BLE_CODE_MQTT_CONN_ERR);

                rev_queue.common_event = REV_CONFIG_FAIL_EVENT;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_i("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            }
            else
#endif
            {
                rev_queue.common_event = REV_CLOUD_DISCONNECTED;
                if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                {
                    aiio_log_i("queue send failed\r\n");
                }
                memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            }
            aiio_log_i("AIIO_SERVICE_MQTT_DISCONNECT \r\n");
            cloud_connect = false;
        }
        break;

        case AIIO_SERVICE_MQTT_CONNECTING:                                                     
        {
            aiio_log_i("AIIO_SERVICE_MQTT_CONNECTING \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(ble_config_start)
            {
                aiio_ble_config_response_status(AIIO_BLE_CODE_MQTT_CONNING);
            }
#endif
        }
        break;

        case AIIO_SERVICE_MQTT_CONNECTED:                                                
        {
            aiio_log_i("AIIO_SERVICE_MQTT_CONNECTED \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            aiio_ble_config_response_status(AIIO_BLE_CODE_MQTT_CONN_OK);
#endif
        }
        break;

        case AIIO_SERVICE_ACTIVITY_EVENT:                                                               /*When the device is in the distribution network, it should report the activation data to the cloud. The event is callback cause the device has sent the activation data to the cloud */
        {
            aiio_log_i("AIIO_SERVICE_ACTIVITY_EVENT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(ble_config_start)
            {
                aiio_ble_config_response_status(AIIO_BLE_CODE_ACTIVITY_REQ);
            }
#endif
        }
        break;

        case AIIO_SERVICE_RESTORE_EVENT:                                                    
        {
            aiio_log_i("AIIO_SERVICE_RESTORE_EVENT \r\n");

            aiio_flash_clear_config_data();
            rev_queue.common_event = REV_CONFIG_START_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_e("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            cloud_connect = false;
        }
        break;

        case AIIO_SERVICE_REBOOT_EVENT:                                                             
        {
            aiio_log_i("AIIO_SERVICE_REBOOT_EVENT \r\n");
            aiio_restart();
        }
        break;

        case AIIO_SERVICE_INFO_EVENT:                                                           /* The requestion command of device information come from cloud, the device needs to response device information to cloud*/
        {
            aiio_log_i("AIIO_SERVICE_INFO_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_CONFIG_EVENT:                                                             /* The configuration command come from cloud, the device needs to be configured for the configuration data*/
        {
            aiio_log_i("AIIO_SERVICE_CONFIG_EVENT \r\n");
        }
        break;

        case AIIO_SERVICE_UPGRADE_EVENT:                                                          /* The ota command come from cloud, device needs to execure ota function*/
        {
            aiio_log_i("AIIO_SERVICE_UPGRADE_EVENT \r\n");
#ifdef CONFIG_OTA_MODULE
            aiio_ota_transmit_start((uint8_t *)rev_event->data);
#endif
        }
        break;

        case AIIO_SERVICE_QUERY_EVENT:                                                                      /* The query command of attribute status come from cloud, device needs to report all attribute status to cloud*/
        {
            aiio_log_i("AIIO_SERVICE_QUERY_EVENT \r\n");
            aiio_report_all_attibute_status(rev_event->msgMid, rev_event->from);
        }
        break;

        case AIIO_SERVICE_CONTROL_EVENT:                                                            
        {
            aiio_log_i("AIIO_SERVICE_CONTROL_EVENT \r\n");

            if(rev_event->data)
            {
                aiio_parse_control_data(rev_event->msgMid, rev_event->from, rev_event->data);
            }
        }
        break;

        // case AIIO_SERVICE_LINKAGE_EVENT:                                                            
        // {
        //     aiio_log_i("AIIO_SERVICE_LINKAGE_EVENT \r\n");
        //     if(!device_host_slave){
        //         if(rev_event->data)
        //         {
        //             aiio_parse_control_data(rev_event->msgMid, rev_event->from, rev_event->data);
        //         }
        //     }
        // }
        // break;

        case AIIO_SERVICE_UTC_EVENT:                                                                
        {
            aiio_log_i("AIIO_SERVICE_UTC_EVENT \r\n");
            if(rev_event->data)
            {
                aiio_log_i("receive data: %s \r\n", rev_event->data);
                aiio_online_update_local_time(rev_event->data, aiio_strlen(rev_event->data));
            }
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            aiio_ble_config_response_status(AIIO_BLE_CODE_UTC_REQ);
#endif
        }
        break;

        case AIIO_SERVICE_ONLINE_EVENT:                                                        
        {
            aiio_log_i("AIIO_SERVICE_ONLINE_EVENT \r\n");
#ifdef CONFIG_BLE_DISTRIBUTION_NETWORK
            if(ble_config_start)
            {
                aiio_ble_config_response_status(AIIO_BLE_CODE_ONLINE_REQ);
            }
#endif
            rev_queue.common_event = REV_CLOUD_ONLINE_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            cloud_connect = true;
        }
        break;
        
        default:
            aiio_log_i("event id err \r\n");
            break;
    }
}



void aiio_user_service_init(bool activity, aiio_cloud_receive_t *cloud_data)
{
    uint8_t mac[6];
    char mac_str[20] = {0};
    aiio_service_config_t  service_config = {0};

    if(service_init_state)
    {
        aiio_log_e("service already init \r\n");
        return ;
    }

    aiio_wifi_sta_mac_get((uint8_t *)mac);
    aiio_log_i("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    snprintf(mac_str, 20, "%02x%02x%02x%02x%02x%02x", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    aiio_log_i("mac_str: %s\r\n", mac_str);

    service_config.service_data.deviceid = DeviceInfo.deviceId;
    service_config.service_data.productid = PRODUCT_ID;
    service_config.service_data.user_name = DeviceInfo.userName;
    service_config.service_data.passwd = DeviceInfo.userPasswd;
    service_config.dev_info.fwVer = DEVICE_FW_VERSION;
    // service_config.dev_info.mcuVer = mcuVer;
    service_config.dev_info.devType = DEVICE_TYPE;
    service_config.dev_info.mac = mac_str;

    memcpy(&service_config.wifi_info, cloud_data->wifi_info, sizeof(aiio_wifi_info_t));
    service_config.service_netif.host = cloud_data->mqtt_host;
    service_config.service_netif.port = cloud_data->mqtt_port;
    service_config.service_netif.keepalive = 50;
    service_config.service_netif.transport = MQTT_TRANSPORT_OVER_SSL;
    service_config.activity = activity;
    service_config.token = cloud_data->device_token;
    service_config.rev_service_data_cb = aiio_service_receive_data;

    aiio_service_init(&service_config);
    service_init_state = true;
}



void aiio_user_service_deinit(void)
{
    if(!service_init_state)
    {
        aiio_log_e("service already deinit \r\n");
        return ;
    }
    aiio_service_deinit();
    service_init_state = false;
}


#endif
