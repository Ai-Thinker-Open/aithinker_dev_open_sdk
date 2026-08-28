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


#ifdef CONFIG_OTA_ENABLE

#define  OTA_DATA_SIZE       (CONFIG_OTA_DATA_SIZE)
#define  MCU_ACK_TIMEOUT   (5000)   //50*1000ms
#define  OTA_WAIT_TIMEOUT  (10)
#define   MAX_RETRY   (2)
#define   XMODEM_ACK_TIMEOUT   (60*1000) //ms

typedef enum
{
    XMODEM_START,
    XMODEM_EOT,
}xmodem_flag_t;


static aiio_rev_queue_t  rev_queue = {0};
static aiio_ota_msg_t   ota_info_buffer = {0};
static int ota_rate = 0;
static int last_ota_rate = 0;
static uint32_t down_size = 0;
extern aiio_ota_msg_t  OtaProInfo;
bool ota_transmit_init = false;
uint32_t flash_write_len = 0 ;
static aiio_os_thread_handle_t *mcu_ota = NULL;  
int aiio_mcu_ota_download(uint8_t *data, uint32_t length,uint32_t data_total_len);
int TransMcuDeviceUOtaData(uint32_t datalen);
static void mcu_ota_task(void *pvParameters);
static bool first_mcu_ota = true;
bool model_ota_flag = false;
static uint32_t s_mcu_ota_cache_addr = FLASH_RW_OTA_INFO_ADDR; 

__attribute__((weak)) int aiio_platform_get_ota_cache_addr(uint32_t need_size, uint32_t *out_addr)
{
    (void)need_size;
    *out_addr = FLASH_RW_OTA_INFO_ADDR;
    return AIIO_OK;
}

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
                if(strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_MCU, strlen(IOT_OTA_TYPE_MCU)) == 0
                || strncmp(OtaProInfo.otaType, IOT_OTA_TYPE_EXTEND, strlen(IOT_OTA_TYPE_EXTEND)) == 0)
                {
                        ota_rate = ((cloud_ota_event->ota_data_progress * 100) / cloud_ota_event->ota_data_total_len);
                        aiio_mcu_ota_download((uint8_t *)cloud_ota_event->ota_data, cloud_ota_event->ota_data_len,cloud_ota_event->ota_data_total_len);
                        if(ota_rate == 100)
                        {
                            down_size = cloud_ota_event->ota_data_total_len ;
                            NotifyMcuDeviceUpdate(OtaProInfo.otaType, OtaProInfo.OtaVer, OtaProInfo.otaKey, OtaProInfo.md5);
                    
                            aiio_os_thread_create(&mcu_ota, "mcu_ota", mcu_ota_task, 1024 * 4, NULL, 15);
                            
                        }
                }else{
                    if(!model_ota_flag){
                        model_ota_flag = true;
                    }
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
    ota_transmit_init = true;
   
    aiio_platform_ota_start();
    
    aiio_cloud_ota_start((char *)ota_info, &cloud_ota_config);
   
    return 0;
    
}


int aiio_ota_transmit_stop(void)
{
    aiio_platform_ota_stop();
    ota_transmit_init = false;
    model_ota_flag = false;
    // mcu_ota_start_flag = false;
    last_ota_rate = 0;
    ota_rate = 0;
    aiio_cloud_ota_stop();
    // aiio_wdt_init();  
    // aiio_TimerStart(wifi_watchdog_timer_handle, 5000);
    return 0;
}

static void aiio_mcuota_fail(void)
{
    aiio_log_d("clearmcuotastatus-------\r\n ");
    aiio_cloud_ota_report_proccess_status(AIIO_OTA_DATA_ERR, NULL, NULL);
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    aiio_ota_transmit_stop();
    IotClearOTAInfo();
    ResetMcuOTAStatus();
    ResetMcuOTAStart();
    aiio_restart();
}


static void aiio_mcuota_success(void)
{
    aiio_cloud_ota_report_download_progress(100);

    AilinkSetOtaInstallStatus(true);
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    aiio_cloud_ota_report_install_status(AIIO_OTA_SUCCESS);
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));

    aiio_ota_transmit_stop();
    ResetMcuOTAStatus();
    ResetMcuOTAStart();
    aiio_restart();
}



int aiio_mcu_ota_download(uint8_t *data, uint32_t length,uint32_t data_total_len)
{
    if(first_mcu_ota){
        aiio_platform_get_ota_cache_addr(OTA_DATA_SIZE, &s_mcu_ota_cache_addr);
        aiio_log_d("erase flash addr %x\r\n ",s_mcu_ota_cache_addr);
        aiio_flash_erase(s_mcu_ota_cache_addr,OTA_DATA_SIZE);
        flash_write_len = 0;
        down_size = 0 ;
        first_mcu_ota = false;
    }
    
    if(data == NULL || length == 0)
    {
        aiio_log_e("param err \r\n");
        goto EXIT;
    }
    aiio_log_d("offset = %ld \r\n", length);
    if(data_total_len < OTA_DATA_SIZE)
    {
        int ret =aiio_flash_write(s_mcu_ota_cache_addr + flash_write_len, data, length);//开始往分区写
        if (ret) 
        {
            aiio_log_e("wirte failed\r\n");
            goto EXIT;
        }
        flash_write_len += length;
        aiio_log_d("offset = %ld \r\n", flash_write_len);
        return  AIIO_OK;
    }
    else
    {
        aiio_log_e("mcu ota file too big \r\n");
        goto EXIT;
    }
    
    EXIT:
    aiio_mcuota_fail();
    return  AIIO_ERROR;

}



uint8_t xmodem_checksum(uint8_t *buffer, uint32_t len)
{
    uint8_t checksum = 0;

    for (uint32_t i = 0; i < len; ++i) {
      checksum += buffer[i];
    }

    return checksum;
}

static uint8_t xdodem_wait_mcu_ack(xmodem_flag_t type)
{
     uint8_t wait_times=0;
     while(1) 
     {
        switch(type)
        {
            case XMODEM_START:
                if(GetMcuOTAStart())
                {
                    return AIIO_OK;
                }
            break;
            case XMODEM_EOT:
                if(GetMcuOTAStatus()==ACK)
                {
                    return AIIO_OK;
                }
            break;
            default:        
            break;
        }

        if(wait_times == MCU_ACK_TIMEOUT)
        {
            aiio_log_e("mcu  ack is time_out\r\n");
            return AIIO_ERROR;
        }
        else
        {
            aiio_os_tick_dealy(aiio_os_ms2tick(10));
            wait_times++;  
        } 
     }
}
 


static uint32_t get_curtime(void)
{
    uint32_t count;
    uint32_t CurTick;
    aiio_os_tick_count(&count);
    CurTick = aiio_os_tick2ms(count);
    return CurTick;
}



static void  xmodem_send(uint8_t *buffer, uint32_t len , uint32_t *sendtime )
{
     aiio_uart_send_data(AIIO_UART1, buffer, len);
     *sendtime = get_curtime(); 
     ResetMcuOTAStatus(); 
}



int TransMcuDeviceUOtaData(uint32_t datalen)
{
    uint32_t ota_rate = 0;
    uint32_t last_ota_rate = 0; 
    uint32_t sender_time = 0;
    uint16_t pack_num = 1;
    uint32_t current_time = 0;
    bool wait_reply = false;
    uint32_t sendlen=0;
    // uint8_t retry_times =0;
    uint16_t xmodem_packet_len = 0;
    uint16_t xmodem_data_len = 0;
    uint32_t total_len = datalen ;
    xmodem_packet_len = (ota_transmit_mode == XMODEM_128) ? XMODEM_PACKET_LEN : XMODEM_1k_PACKET_LEN;
    xmodem_data_len = (ota_transmit_mode == XMODEM_128) ? XMODEM_DATA_LEN : XMODEM_1k_DATA_LEN;

    uint8_t *packet = malloc(xmodem_packet_len * sizeof(uint8_t));
    if(packet == NULL)
    {
        aiio_log_e("malloc fail \r\n");
        aiio_mcuota_fail();
        return AIIO_ERROR; 
    }
    if(xdodem_wait_mcu_ack(XMODEM_START) !=AIIO_OK)
    {
       goto EXIT;
    }
    if(total_len  > OTA_DATA_SIZE)
    {
        aiio_log_e("size it to big  %d\r\n ",total_len);
        goto EXIT;
    }
    aiio_log_i("start xmodem----------- xmodem data len [%d] packet len [%d]\r\n",xmodem_data_len,xmodem_packet_len);
    while(sendlen < total_len )
    {
        if(!wait_reply)
        {
            if(total_len-sendlen >= xmodem_data_len)
            {
                if (aiio_flash_read(s_mcu_ota_cache_addr + sendlen,packet, xmodem_data_len) == 0)
                {  
                    memmove(packet + XMODEM_HEAD_LEN, packet, xmodem_data_len); 
                     
                } 
            }
            else
            {
                if (aiio_flash_read(s_mcu_ota_cache_addr + sendlen,packet, total_len-sendlen) == 0)
                {  
                    memmove(packet + XMODEM_HEAD_LEN, packet, total_len - sendlen);  
                    memset(&packet[XMODEM_HEAD_LEN + total_len - sendlen], 0x1A, xmodem_data_len-(total_len - sendlen));
                }   
            }

            packet[0] = (ota_transmit_mode == XMODEM_128) ? SOH : STX;
            packet[1] = pack_num & 0xFF;
            packet[2] = ~(pack_num & 0xFF);
            uint8_t checksum = xmodem_checksum(&packet[XMODEM_HEAD_LEN], xmodem_data_len);
            packet[xmodem_data_len + XMODEM_HEAD_LEN] = checksum;
            xmodem_send(packet,xmodem_packet_len,&sender_time);
            aiio_log_d("xmodemsendlen [%d] [%d] [%d] \r\n",sendlen,total_len,total_len - sendlen);
            wait_reply = true;
        }
        else
        {
            switch(GetMcuOTAStatus())
            {
                case ACK:
                    if(total_len - sendlen >= xmodem_data_len)
                    {
                        sendlen += xmodem_data_len;
                    }
                    else
                    {
                        sendlen += total_len - sendlen;
                    }
                    ota_rate = (sendlen * 100) / total_len;
                    if(ota_rate - last_ota_rate >= 25)
                    {
                        aiio_cloud_ota_report_download_progress(ota_rate);
                        aiio_os_tick_dealy(aiio_os_ms2tick(200));
                        last_ota_rate = ota_rate;
                    }
                    memset(packet,0,xmodem_packet_len);
                    wait_reply = false;
                    // retry_times=0;
                    pack_num ++;
                break;
                case NAK:
                    // aiio_log_e("mcu reply nck \r\n");
                    // if(retry_times > MAX_RETRY)
                    // {
                    //     packet[0] = CAN;
                    //     xmodem_send(packet, XMODEM_ACK_LEN ,&sender_time);
                    //     goto EXIT;
                    // }
                    xmodem_send(packet,xmodem_packet_len,&sender_time);
                    // retry_times++;
                break;
                case CAN:
                    aiio_log_e("mcu  reply can \r\n");
                    goto EXIT;
                break;
                default:
                        break;
            }
        }
        current_time= get_curtime();
        aiio_os_tick_dealy(aiio_os_ms2tick(10));
        if(diffTimeTick(sender_time, current_time) > XMODEM_ACK_TIMEOUT)
        {
            aiio_log_e("xmodem ack time out sender_time is %d current_time is %d \r\n",sender_time,current_time);
            // packet[0] = CAN;
            // xmodem_send(packet, XMODEM_ACK_LEN ,&sender_time);
            goto EXIT;
        }
    }
    if(total_len == sendlen)
    {
        aiio_log_e("xmodem send EOT\r\n");
        packet[0] = EOT;
        xmodem_send(packet, XMODEM_ACK_LEN ,&sender_time);
        if(xdodem_wait_mcu_ack(XMODEM_EOT) !=AIIO_OK)
        {
            goto EXIT;
        }
        aiio_log_e("xmodem sueccss-----------\r\n");
        aiio_mcuota_success();
        free(packet);
        return AIIO_OK;
    }
    
    EXIT:
    free(packet);
    aiio_mcuota_fail();
    return AIIO_ERROR; 
}


static void mcu_ota_task(void *pvParameters)
{
    
    aiio_log_d("ota download size is %d\r\n", down_size);
    TransMcuDeviceUOtaData(down_size);
    aiio_os_thread_delete(NULL);
}


#endif


