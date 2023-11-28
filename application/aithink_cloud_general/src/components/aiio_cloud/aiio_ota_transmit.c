/**
 * @brief   Define ota launch interface and ota data transmit interface
 * 
 * @file    aiio_ota_transmit.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-08-04          <td>1.0.0            <td>zhuolm             <td> The ota launch interface and ota data transmit interface
 */
#include "aiio_ota_transmit.h"
#include "aiio_cloud_ota.h"
#include "aiio_protocol_comm.h"
#include "aiio_adapter_include.h"
#include "config.h"
#include "aiio_common.h"
#include "aiio_mcu_system.h"


#ifdef CONFIG_OTA_MODULE

static aiio_rev_queue_t  rev_queue = {0};
static aiio_ota_msg_t   ota_info_buffer = {0};
static int ota_rate = 0;
static int last_ota_rate = 0;
bool ota_transmit_init = false;



static void aiio_get_ota_info(aiio_ota_msg_t *ota_info, aiio_ota_protocol_msg_t *ota_msg)
{
    if(ota_info == NULL || ota_msg == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    if(ota_msg->otaVer)
    {
        aiio_strncpy(ota_info->OtaVer, ota_msg->otaVer, aiio_strlen(ota_msg->otaVer));
    }

    if(ota_msg->otaType)
    {
        aiio_strncpy(ota_info->otaType, ota_msg->otaType, aiio_strlen(ota_msg->otaType));
    }

    if(ota_msg->download_url)
    {
        aiio_strncpy(ota_info->download_url, ota_msg->download_url, aiio_strlen(ota_msg->download_url));
    }

    if(ota_msg->PubId)
    {
        aiio_strncpy(ota_info->PubId, ota_msg->PubId, aiio_strlen(ota_msg->PubId));
    }

    if(ota_msg->md5)
    {
        aiio_strncpy(ota_info->md5, ota_msg->md5, aiio_strlen(ota_msg->md5));
    }

    ota_info->channel = ota_msg->channel;

}

static void aiio_cloud_ota_receive_event(aiio_cloud_ota_event_t *cloud_ota_event)
{
    int ret = 0;

    switch (cloud_ota_event->ota_event_id)
    {
        case AIIO_CLOUD_OTA_INFO_PARSE_OK:
        {
            aiio_log_d("AIIO_CLOUD_OTA_INFO_PARSE_OK \r\n");

            aiio_get_ota_info(&ota_info_buffer, cloud_ota_event->ota_protocol_msg);

            rev_queue.queue_data = &ota_info_buffer;
            rev_queue.queue_data_len = sizeof(aiio_ota_msg_t);
            rev_queue.common_event = REV_CLOUD_OTA_INFO_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
        }
        break;

        case AIIO_CLOUD_OTA_INFO_PARSE_FAILE:
        {
            aiio_log_d("AIIO_CLOUD_OTA_INFO_PARSE_FAILE \r\n");
            rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            aiio_cloud_ota_report_proccess_status(AIIO_OTA_PROTOCOL_DATA_ERR, NULL, NULL);
            
        }
        break;

        case AIIO_CLOUD_OTA_HARDWARE_ERR:
        {
            aiio_log_d("AIIO_CLOUD_OTA_HARDWARE_ERR \r\n");
            rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            aiio_cloud_ota_report_proccess_status(AIIO_OTA_DEVICE_ERR, NULL, NULL);
            
        }
        break;

        case AIIO_CLOUD_OTA_HTTP_ERR:
        {
            aiio_log_d("AIIO_CLOUD_OTA_HTTP_ERR \r\n");
            rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            aiio_cloud_ota_report_proccess_status(AIIO_OTA_DOWNLOAD_FAIL, NULL, NULL);
            
        }
        break;

        case AIIO_CLOUD_OTA_REV_DATA:
        {
            aiio_log_d("AIIO_CLOUD_OTA_REV_DATA \r\n");

            aiio_log_d("ota_data_progress = %d \r\n", cloud_ota_event->ota_data_progress);
            aiio_log_d("ota_data_total_len = %d \r\n", cloud_ota_event->ota_data_total_len);
            if(ota_transmit_init)
            {   
                if(strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)) == 0){
                    // if(mcu_ota_start_flag){
                        TransferMcuDeviceUOtaData((char *)cloud_ota_event->ota_data, cloud_ota_event->ota_data_len);
                         ota_rate = ((cloud_ota_event->ota_data_progress * 100) / cloud_ota_event->ota_data_total_len);
                        // if((ota_rate % 10 == 0) && (last_ota_rate != ota_rate))
                        if((ota_rate - last_ota_rate >= 10) || ota_rate == 100)
                        {
                            aiio_log_i("ota_rate = %d \r\n", ota_rate);
                            last_ota_rate = ota_rate;
                            aiio_cloud_ota_report_download_progress(ota_rate);
                        }
                        if(ota_rate == 100){
                            aiio_send_mcu_cmd(UPDATE_LAST_CMD, NULL, 0);
                        }
                    // }
                    // else{
                    //     aiio_ota_transmit_stop();
                    // }
                }else{
                     ota_rate = ((cloud_ota_event->ota_data_progress * 100) / cloud_ota_event->ota_data_total_len);
                    if((ota_rate % 10 == 0) && (last_ota_rate != ota_rate))
                    {
                        aiio_log_i("ota_rate = %d \r\n", ota_rate);
                        last_ota_rate = ota_rate;
                        aiio_cloud_ota_report_download_progress(ota_rate);
                    }
                    ret = aiio_platform_ota_install((uint8_t *)cloud_ota_event->ota_data, cloud_ota_event->ota_data_len, cloud_ota_event->ota_data_total_len);
                    if(ret == AIIO_ERROR)
                    {
                        aiio_log_e("ota install err \r\n");
                        rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
                        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                        {
                            aiio_log_e("queue send failed\r\n");
                        }
                        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                        aiio_cloud_ota_report_proccess_status(AIIO_OTA_DATA_ERR, NULL, NULL);
                        
                    }
                    else if(ret == AIIO_OTA_INSTALL_SUCCESS)
                    {
                        AilinkSetOtaInstallStatus(true);
                        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
                        aiio_cloud_ota_report_install_status(AIIO_OTA_SUCCESS);
                        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
                        rev_queue.common_event = REV_CLOUD_OTA_SUCCESS_EVENT;
                        if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
                        {
                            aiio_log_i("queue send failed\r\n");
                        }
                        memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
                    }
                }
            }

        }
        break;

        case AIIO_CLOUD_OTA_DATA_CHECK_ERR:
        {
            aiio_log_d("AIIO_CLOUD_OTA_DATA_CHECK_ERR \r\n");
            rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            aiio_cloud_ota_report_proccess_status(AIIO_OTA_DATA_ERR, NULL, NULL);
            
        }
        break;

        case AIIO_CLOUD_OTA_REV_DATA_TIMEOUT:
        {
            aiio_log_d("AIIO_CLOUD_OTA_REV_DATA_TIMEOUT \r\n");
            rev_queue.common_event = REV_CLOUD_OTA_FAIL_EVENT;
            if (xQueueSendToBack(cloud_rev_queue_handle, &rev_queue, 100) != pdPASS)
            {
                aiio_log_i("queue send failed\r\n");
            }
            memset(&rev_queue, 0, sizeof(aiio_rev_queue_t));
            aiio_cloud_ota_report_proccess_status(AIIO_OTA_DOWNLOAD_FAIL, NULL, NULL);
            
        }
        break;

        case AIIO_CLOUD_OTA_FINISH:
        {
            aiio_log_d("AIIO_CLOUD_OTA_FINISH \r\n");
            
        }
        break;
    
    default:
        break;
    }

    aiio_os_tick_dealy(aiio_os_ms2tick(10));
}


int aiio_ota_transmit_start(uint8_t *ota_info)
{
    aiio_cloud_ota_config_t  cloud_ota_config = {0};

    cloud_ota_config.device_fw_version = DEVICE_FW_VERSION;
    cloud_ota_config.device_product_id = PRODUCT_ID;
    cloud_ota_config.is_compatible_type = true;
    cloud_ota_config.cloud_ota_rev_eve_cb = aiio_cloud_ota_receive_event;

    aiio_platform_ota_start();
    ota_transmit_init = true;
    return aiio_cloud_ota_start((char *)ota_info, &cloud_ota_config);
}


int aiio_ota_transmit_stop(void)
{
    aiio_platform_ota_stop();
    ota_transmit_init = false;
    // mcu_ota_start_flag = false;
    last_ota_rate = 0;
    ota_rate = 0;
    return aiio_cloud_ota_stop();
    aiio_wdt_init();  
    aiio_TimerStart(wifi_watchdog_timer_handle, 5000);
}

#endif


