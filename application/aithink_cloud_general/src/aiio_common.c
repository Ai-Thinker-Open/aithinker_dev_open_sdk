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

aiio_ota_msg_t  OtaProInfo = {0};

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

void AilinkSavePairInfo(wifi_config_data_t *info)
{
    char str_port[6] = {0};

    if(info == NULL)
    {
        aiio_log_e("Param err \r\n");
        return ;
    }

    AilinkFlashWrite(DEVICE_PAIR_INFO_FLASH_BLOCK, (uint8_t *)info, sizeof(wifi_config_data_t));
}

void AilinkClearPairInfo(void)
{
    wifi_config_data_t info = {0};

    AilinkFlashWrite(DEVICE_PAIR_INFO_FLASH_BLOCK, (uint8_t *)&info, sizeof(wifi_config_data_t));
}

static bool AilinkVerifyWifiInfoIsEmpty(wifi_config_data_t *info)
{
    if(info == NULL)
    {
        aiio_log_e("param is NULL \r\n");
        return true;
    }
    if((strlen(info->ssid) == 0) || (strlen(info->passwd) == 0) || (strlen(info->wificc) == 0) ||
        (strlen(info->mqttip) == 0) || (strlen(info->tz) == 0) || (strlen(info->token) == 0))
    {
        return true;
    }

    return false;
}

int AilinkLoadPairInfo(wifi_config_data_t *info)
{
    uint16_t wifi_config_data_len = 0;
    int32_t ret=0;

    if(info == NULL)
    {
        aiio_log_e("Param err \r\n");
        return 0;
    }

    wifi_config_data_len = AilinkFlashRead(DEVICE_PAIR_INFO_FLASH_BLOCK, (uint8_t *)info, sizeof(wifi_config_data_t));
    if(AilinkVerifyWifiInfoIsEmpty(info) > 0)
    {
        aiio_log_e("Failed to get the data from the flash \r\n");
        return 0;
    }
    aiio_log_i("info->ssid = %s \r\n", info->ssid);
    aiio_log_i("info->passwd = %s \r\n", info->passwd);
    aiio_log_i("info->wificc = %s \r\n", info->wificc);
    aiio_log_i("info->mqttip = %s \r\n", info->mqttip);
    aiio_log_i("info->tz = %s \r\n", info->tz);
    aiio_log_i("info->token = %s \r\n", info->token);
    aiio_log_i("info->port = %d \r\n", info->port);
    return wifi_config_data_len;
}

void iotgoGetDeviceInfo(aiio_device_info *deviceInfo)
{
    char databuf[200] = {0};
    cJSON *cjson_root   = NULL;
    cJSON *cjson_value   = NULL;
    char  *str_value = NULL;
    uint16_t read_length = 0;

    if(deviceInfo == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }
    read_length = sizeof(databuf)-1;

    aiio_flash_read(DEVICE_INFO_ADDR, (uint8_t *)databuf, &read_length);

    aiio_log_d("databuf = %s \r\n", databuf);
    cjson_root = cJSON_Parse(databuf);
    if(cjson_root == NULL)
    {
        // IOT_InfoLog("json parse err \r\n");
        return ;
    }

    cjson_value = cJSON_GetObjectItem(cjson_root, "username");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        aiio_log_i("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->userName, str_value, strlen(str_value));
        cjson_value = NULL;
    }
    
    cjson_value = cJSON_GetObjectItem(cjson_root, "password");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        aiio_log_i("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->userPasswd, str_value, strlen(str_value));
        cjson_value = NULL;
    }    
    
    cjson_value = cJSON_GetObjectItem(cjson_root, "deviceid");
    if(cjson_value)
    {
        str_value = cJSON_GetStringValue(cjson_value);
        aiio_log_i("str_value = %s \r\n", str_value);
        memcpy(deviceInfo->deviceId, str_value, strlen(str_value));
        cjson_value = NULL;
    }

    cJSON_Delete(cjson_root);
}

char *PacketDeviceInfoToJson(void)
{
    cJSON  *json_root = NULL;
    char  *json_str = NULL;

    json_root = cJSON_CreateObject();
    if (!json_root)
    {
        aiio_log_e("json create object fail \r\n");
        return NULL;
    }

    cJSON_AddStringToObject(json_root, "username", DeviceInfo.userName);
    cJSON_AddStringToObject(json_root, "password", DeviceInfo.userPasswd);
    cJSON_AddStringToObject(json_root, "deviceid", DeviceInfo.deviceId);

    json_str = cJSON_PrintUnformatted(json_root);
    if(json_str == NULL)
    {
        aiio_log_e("json create str fail \r\n");
        cJSON_Delete(json_root);
        return NULL;
    }

    cJSON_Delete(json_root);
    return json_str;
}

void IotSaveDeviceInfo(void)
{
    char *deviceinfo = NULL;

    deviceinfo = PacketDeviceInfoToJson();
    aiio_log_d("deviceinfo = %s \r\n", deviceinfo);

    aiio_flash_erase(DEVICE_INFO_ADDR, strlen(deviceinfo) + 1);
    aiio_flash_write(DEVICE_INFO_ADDR, (uint8_t *)deviceinfo, strlen(deviceinfo));
}

bool iotCheckDeviceInfoIsEmpty(aiio_device_info *deviceInfo)
{
    if(deviceInfo == NULL)
    {
        return false;
    }
    if((strlen(deviceInfo->userName) == 0) || (strlen(deviceInfo->userPasswd) == 0) || (strlen(deviceInfo->deviceId) == 0))
    {
        return false;
    }

    return true;
}

void IotSaveDeviceParams(aiio_device_params *params)
{
    if(params == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }
    DeviceParams.uart_bound = params->uart_bound;
    DeviceParams.work_mode = params->work_mode;

    AilinkFlashWrite(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&DeviceParams, sizeof(aiio_device_params));
}

void IotClearDeviceParams(void)
{
    memset(&DeviceParams, 0, sizeof(aiio_device_params));
    AilinkFlashWrite(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&DeviceParams, sizeof(aiio_device_params));
}

void IotLoadDeviceParams(aiio_device_params *params)
{
    if(params == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    memset(&DeviceParams, 0, sizeof(aiio_device_params));
    AilinkFlashRead(DEVICE_PARAMS_FLASH_BLOCK, (uint8_t *)&DeviceParams, sizeof(aiio_device_params));
    aiio_log_i("DeviceParams.uart_bound = %d \r\n", DeviceParams.uart_bound);
    aiio_log_i("DeviceParams.work_mode = %d \r\n", DeviceParams.work_mode);

    memcpy(params, &DeviceParams, sizeof(aiio_device_params));
}


static bool aiio_ota_info_is_empty_check(aiio_ota_msg_t *ota_info)
{
    if(ota_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return true;
    }

    if(aiio_buffer_is_empty_check(ota_info->OtaVer, sizeof(ota_info->OtaVer)))                           return true;
    if(aiio_buffer_is_empty_check(ota_info->otaType, sizeof(ota_info->otaType)))                         return true;
    if(aiio_buffer_is_empty_check(ota_info->download_url, sizeof(ota_info->download_url)))               return true;
    if(aiio_buffer_is_empty_check(ota_info->PubId, sizeof(ota_info->PubId)))                             return true;
    if(aiio_buffer_is_empty_check(ota_info->md5, sizeof(ota_info->md5)))                                 return true;


    if(aiio_strlen(ota_info->OtaVer) == 0 || aiio_strlen(ota_info->otaType) == 0 || aiio_strlen(ota_info->download_url) == 0 ||  
        aiio_strlen(ota_info->PubId) == 0 ||  aiio_strlen(ota_info->md5) == 0 || ota_info->channel == 0)
        {
            return true;
        }
    
    return false;
}

void AilinkSetOtaInstallStatus(bool ota_status)
{
    char status[3] = {0};

    if(ota_status)
    {
        memcpy(status, "ok", 2);
        aiio_log_i("set ota status \r\n");
        AilinkFlashWrite(OTA_INSTALL_FLAG, (uint8_t *)status, sizeof(status));
    }
    else
    {
        memset(status, 0, sizeof(status));
        aiio_log_i("clear ota status \r\n");
        AilinkFlashWrite(OTA_INSTALL_FLAG, (uint8_t *)status, sizeof(status));
    }
}

bool AilinkGetOtaInstallStatus(void)
{
    char status[3] = {0};

    AilinkFlashRead(OTA_INSTALL_FLAG, (uint8_t *)status, 2);
    if(strcmp(status, "ok") == 0)
    {
        aiio_log_i("ota already install ok \r\n");
        return true;
    }
    else
    {
        aiio_log_i("not ota install \r\n");
        return false;
    }
}

void IotSaveOTAInfo(void)
{
    AilinkFlashWrite(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(aiio_ota_msg_t));
}

void IotClearOTAInfo(void)
{
    memset(&OtaProInfo, 0, sizeof(OtaProInfo));
    AilinkFlashWrite(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(aiio_ota_msg_t));
}

void IotLoadOTAInfo(void)
{
    memset(&OtaProInfo, 0, sizeof(aiio_ota_msg_t));
    AilinkFlashRead(DEVICE_OTA_INFO_FLASH_BLOCK, (uint8_t *)&OtaProInfo, sizeof(aiio_ota_msg_t));
    aiio_log_i("OtaProInfo.pointVer = %s \r\n", OtaProInfo.pointVer);
    aiio_log_i("OtaProInfo.baseVer = %s \r\n", OtaProInfo.baseVer);
    aiio_log_i("OtaProInfo.OtaVer = %s \r\n", OtaProInfo.OtaVer);
    aiio_log_i("OtaProInfo.mcuBaseVer = %s \r\n", OtaProInfo.mcuBaseVer);
    aiio_log_i("OtaProInfo.otaType = %s \r\n", OtaProInfo.otaType);
    aiio_log_i("OtaProInfo.download_url = %s \r\n", OtaProInfo.download_url);
    aiio_log_i("OtaProInfo.channel = %d \r\n", OtaProInfo.channel);
}

void AilinkSaveTimeInfo(void)
{
    aiio_rtc_time_t *RtcTime = NULL;

    RtcTime = aiio_get_rtc_time();
    if(RtcTime == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    AilinkFlashWrite(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)RtcTime, sizeof(aiio_rtc_time_t));
}

void AilinkClearTimeInfo(void)
{
    aiio_rtc_time_t RtcTime = {0};

    AilinkFlashWrite(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)&RtcTime, sizeof(aiio_rtc_time_t));
}

void AilinkLoadTimeInfo(aiio_rtc_time_t *RtcTime)
{
    if(RtcTime == NULL)
    {
        aiio_log_e("Param err \r\n");
        return ;
    }

    AilinkFlashRead(DEVICE_TIME_INFO_FLASH_BLOCK, (uint8_t *)RtcTime, sizeof(aiio_rtc_time_t));
    aiio_log_i("the Date : %02d:%02d:%02d \r\n", RtcTime->year, RtcTime->mon, RtcTime->day);
    aiio_log_i("the time : %02d:%02d:%02d \r\n", RtcTime->hour, RtcTime->minute, RtcTime->second);

}

void aiio_repair(void)
{
    aiio_log_i("go to pair wifi \r\n");
    AilinkClearPairInfo();
}

uint32_t diffTimeTick(uint32_t start, uint32_t end)
{
    return ((end >= start) ? (end-start) : ((0xFFFFFFFF-start) + end));
}