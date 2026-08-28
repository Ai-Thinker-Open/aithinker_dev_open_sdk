/**
 * @brief   
 * 
 * @file    aiio_common.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-09-27          <td>1.0.0            <td>hanly             <td>
 */

#include "aiio_common.h"
#include "aiio_adapter_include.h"
#include "aiio_nvs.h"
#include "aiio_flash.h"

int32_t AilinkFlashWrite(char *block, uint8_t *data, uint16_t data_len)
{
    int32_t ret = -1;
    ret = aiio_nvs_set_blob(block, data, data_len);
    return ret;
}

int32_t AilinkFlashRead(char *block, uint8_t *buf, uint16_t buf_max_len)
{
    int32_t ret = -1;
    size_t buf_len = 0;
    ret = aiio_nvs_get_blob(block, buf, buf_max_len, &buf_len);
    return ret;
}

static bool aiio_buffer_is_empty_check(char *buffer, uint32_t buffer_len)
{
    if(buffer == NULL)
    {
        aiio_log_e("param err \r\n");
        return false;
    }

    for(uint32_t num = 0; num < buffer_len; num ++)
    {
        if(buffer[num] != 0xff)
        {
            return false;
        }
    }

    return true;
}

static bool aiio_device_info_is_empty_check(iotx_device_info_t *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return true;
    }

    if(aiio_buffer_is_empty_check(device_info->device_name, sizeof(device_info->device_name)))               return true;
    if(aiio_buffer_is_empty_check(device_info->device_secret, sizeof(device_info->device_secret)))             return true;
    if(aiio_buffer_is_empty_check(device_info->product_key, sizeof(device_info->product_key)))             return true;
    if(aiio_buffer_is_empty_check(device_info->product_Secret, sizeof(device_info->product_Secret)))             return true;

    if(aiio_strlen(device_info->device_name) == 0 || aiio_strlen(device_info->device_secret) == 0 ||
        aiio_strlen(device_info->product_key) == 0 || aiio_strlen(device_info->product_Secret) == 0)
        {
            return true;
        }
    
    return false;
}


int aiio_flash_get_device_info(iotx_device_info_t *device_info)
{
    uint16_t data_len = 0;

    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }
    
    aiio_log_d("aiio_flash_read \r\n");
    data_len = sizeof(iotx_device_info_t);
    aiio_flash_read(FLASH_RW_DEVICE_INFO_ADDR, device_info, data_len);

    aiio_log_d("aiio_device_info_is_empty_check \r\n");
    if(aiio_device_info_is_empty_check(device_info))
    {
        aiio_log_e("Failed to get the data from the flash \r\n");
        return AIIO_ERROR;
    }

    aiio_log_d("device_info.device_name = %s \r\n", device_info->device_name);
    aiio_log_d("device_info.device_secret = %s \r\n", device_info->device_secret);
    aiio_log_d("device_info.product_key = %s \r\n", device_info->product_key);
    aiio_log_d("device_info.device_secret = %s \r\n", device_info->product_Secret);
    aiio_log_d("device_info.product_id = %u \r\n", device_info->product_Id);
    

    return data_len;
}

int aiio_flash_clear_device_info(iotx_device_info_t *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }

    memset(device_info, 0, sizeof(iotx_device_info_t));
    aiio_flash_erase(FLASH_RW_DEVICE_INFO_ADDR, sizeof(iotx_device_info_t));
    aiio_flash_write(FLASH_RW_DEVICE_INFO_ADDR, device_info, sizeof(iotx_device_info_t));
    aiio_log_d("nvs set data ok \r\n");

    return AIIO_OK;
}

int aiio_flash_save_device_info(iotx_device_info_t *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return 0;
    }

    aiio_flash_erase(FLASH_RW_DEVICE_INFO_ADDR, sizeof(iotx_device_info_t));
    aiio_flash_write(FLASH_RW_DEVICE_INFO_ADDR, device_info, sizeof(iotx_device_info_t));
    aiio_log_d("device info save ok \r\n");

    return AIIO_OK;
}

static bool aiio_wifi_config_data_is_empty_check(wifi_config_data_t *config_data)
{
    if(config_data == NULL)
    {
        aiio_log_e("param err \r\n");
        return true;
    }

    if(aiio_buffer_is_empty_check(config_data->ssid, sizeof(config_data->ssid)))               return true;
    if(aiio_buffer_is_empty_check(config_data->passwd, sizeof(config_data->passwd)))             return true;
    if(aiio_buffer_is_empty_check(config_data->region_mqtturl, sizeof(config_data->region_mqtturl)))             return true;

    if(aiio_strlen(config_data->ssid) == 0)
    {
        return true;
    }
    
    return false;
}

int aiio_flash_get_wifi_config_data(wifi_config_data_t *config_data)
{
    uint16_t wifi_config_data_len = 0;
    int32_t ret=0;

    if(config_data == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }

    aiio_log_d("aiio_flash_read \r\n");
    wifi_config_data_len = sizeof(wifi_config_data_t);
    aiio_flash_read(FLASH_RW_WIFI_DATA_ADDR, config_data, wifi_config_data_len);

    aiio_log_d("aiio_wifi_config_data_is_empty_check \r\n");
    if(aiio_wifi_config_data_is_empty_check(config_data))
    {
        aiio_log_e("Failed to get the data from the flash \r\n");
        return 0;
    }

    aiio_log_d("ssid = %s \r\n", config_data->ssid);
    aiio_log_d("passwd = %s \r\n", config_data->passwd);
    aiio_log_d("region_mqtturl = %s \r\n", config_data->region_mqtturl);
    
    aiio_log_d("wifi_config_data_len = %d \r\n", wifi_config_data_len);

    return wifi_config_data_len;
}

void aiio_flash_save_wifi_config_data(wifi_config_data_t *config_data)
{
    aiio_flash_erase(FLASH_RW_WIFI_DATA_ADDR, sizeof(wifi_config_data_t));
    aiio_flash_write(FLASH_RW_WIFI_DATA_ADDR, config_data, sizeof(wifi_config_data_t));
    aiio_log_d("wifi config data save ok \r\n");
}


void aiio_flash_clear_config_data(void)
{
    int32_t ret=0;
    wifi_config_data_t config_data = {0};

    aiio_flash_erase(FLASH_RW_WIFI_DATA_ADDR, sizeof(wifi_config_data_t));
    aiio_flash_write(FLASH_RW_WIFI_DATA_ADDR, &config_data, sizeof(wifi_config_data_t));
    aiio_log_d("nvs set data ok \r\n");

}