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
#include "utils_hmac.h"
#include "tencent_base64.h"
#include "qcloud_iot_export_error.h"

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

static bool aiio_device_info_is_empty_check(aiio_device_info *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return true;
    }

    if(aiio_buffer_is_empty_check(device_info->device_name, sizeof(device_info->device_name)))               return true;
    if(aiio_buffer_is_empty_check(device_info->device_secret, sizeof(device_info->device_secret)))             return true;
    if(aiio_buffer_is_empty_check(device_info->product_id, sizeof(device_info->product_id)))             return true;

    if(aiio_strlen(device_info->device_name) == 0 || aiio_strlen(device_info->device_secret) == 0 ||
        aiio_strlen(device_info->product_id) == 0)
        {
            return true;
        }
    
    return false;
}


int aiio_flash_get_device_info(aiio_device_info *device_info)
{
    uint16_t data_len = 0;

    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }
    
    aiio_log_d("aiio_flash_read \r\n");
    data_len = sizeof(aiio_device_info);
    aiio_flash_read(FLASH_RW_DEVICE_INFO_ADDR, device_info, data_len);

    aiio_log_d("aiio_device_info_is_empty_check \r\n");
    if(aiio_device_info_is_empty_check(device_info))
    {
        aiio_log_e("Failed to get the data from the flash \r\n");
        return AIIO_ERROR;
    }

    aiio_log_d("device_info.device_name = %s \r\n", device_info->device_name);
    aiio_log_d("device_info.device_secret = %s \r\n", device_info->device_secret);
    aiio_log_d("device_info.product_id = %s \r\n", device_info->product_id);
    

    return data_len;
}

int aiio_flash_clear_device_info(aiio_device_info *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return AIIO_ERROR;
    }

    memset(device_info, 0, sizeof(aiio_device_info));
    aiio_flash_erase(FLASH_RW_DEVICE_INFO_ADDR, sizeof(aiio_device_info));
    aiio_flash_write(FLASH_RW_DEVICE_INFO_ADDR, device_info, sizeof(aiio_device_info));
    aiio_log_d("nvs set data ok \r\n");

    return AIIO_OK;
}

int aiio_flash_save_device_info(aiio_device_info *device_info)
{
    if(device_info == NULL)
    {
        aiio_log_e("param err \r\n");
        return 0;
    }

    aiio_flash_erase(FLASH_RW_DEVICE_INFO_ADDR, sizeof(aiio_device_info));
    aiio_flash_write(FLASH_RW_DEVICE_INFO_ADDR, device_info, sizeof(aiio_device_info));
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

uint64_t HAL_GetTimeMs(void) 
{
    uint32_t tickCount = xTaskGetTickCount();  // 获取当前 tick 数  
    uint32_t tickDuration = configTICK_RATE_HZ / 1000;  // 每个 tick 对应的毫秒数  
    return tickCount * tickDuration;  // 返回毫秒值  
}

/*Get timestamp*/
long HAL_Timer_current_sec(void)
{
    return HAL_GetTimeMs() / 1000;
}

void get_next_conn_id(char *conn_id)
{
    int i;
    srand((unsigned)HAL_GetTimeMs());
    for (i = 0; i < MAX_CONN_ID_LEN - 1; i++) {
        int flag = rand() % 3;
        switch (flag) {
            case 0:
                conn_id[i] = (rand() % 26) + 'a';
                break;
            case 1:
                conn_id[i] = (rand() % 26) + 'A';
                break;
            case 2:
                conn_id[i] = (rand() % 10) + '0';
                break;
        }
    }

    conn_id[MAX_CONN_ID_LEN - 1] = '\0';
}

// void aiio_mqtt_connect_packet(void)
// {
//     char conn_id[MAX_CONN_ID_LEN];
//     unsigned char psk_decode[DECODE_PSK_LENGTH]; 

//     aiio_log_i("device_info->product_id is %s\r\n",DeviceInfo.product_id);
//     aiio_log_i("device_info->device_name is %s\r\n",DeviceInfo.device_name);
//     aiio_log_i("device_info->device_secret is %s\r\n",DeviceInfo.device_secret);

//     size_t src_len = strlen(DeviceInfo.device_secret);
//     printf("@@ src_len is %d\r\n",src_len);
//     size_t len;
//     size_t device_secret_len;

//     memset(psk_decode, 0x00, DECODE_PSK_LENGTH);

//     aiio_log_i("000 @@@@ mqtt_cfg.client_id:%s\r\n",mqtt_cfg.client_id);
//     aiio_log_i("000 @@@@ mqtt_cfg.username:%s\r\n",mqtt_cfg.username);
//     aiio_log_i("000 @@@@ mqtt_cfg.password:%s\r\n",mqtt_cfg.password);
//     aiio_log_i("000 @@@@ device_secret:%s\r\n",psk_decode);

//     qcloud_iot_utils_base64decode(psk_decode, DECODE_PSK_LENGTH, &len,(unsigned char *)DeviceInfo.device_secret, src_len);

//     long cur_timesec = HAL_Timer_current_sec() + MAX_ACCESS_EXPIRE_TIMEOUT / 1000;
//     if (cur_timesec <= 0 || MAX_ACCESS_EXPIRE_TIMEOUT <= 0) {
//         cur_timesec = LONG_MAX;
//     }

//     get_next_conn_id(conn_id);

//     mqtt_cfg.client_id = malloc(MAX_SIZE_OF_CLIENT_ID);  
//     if (mqtt_cfg.client_id == NULL) {  
//     }  

//     int username_len  = strlen(mqtt_cfg.client_id) + QCLOUD_IOT_DEVICE_SDK_APPID_LEN + MAX_CONN_ID_LEN + 20;
//     mqtt_cfg.username = malloc(username_len);  
//     if (mqtt_cfg.username == NULL) {  
//     } 

//     mqtt_cfg.password = (char *)malloc(51);  
//     if (mqtt_cfg.password == NULL) {  
//     } 

//     sprintf(mqtt_cfg.client_id, "%s%s", DeviceInfo.product_id, DeviceInfo.device_name);
//     sprintf(mqtt_cfg.username, "%s;%s;%s;%ld", mqtt_cfg.client_id, QCLOUD_IOT_DEVICE_SDK_APPID,
//                 conn_id, cur_timesec);

//     if (psk_decode != NULL && mqtt_cfg.username != NULL) {
//         char sign[41] = {0};
//         utils_hmac_sha1(mqtt_cfg.username, strlen(mqtt_cfg.username), sign, psk_decode,device_secret_len);
//         sprintf(mqtt_cfg.password,"%s;hmacsha1", sign);
//     }
//     aiio_log_i("222 @@@@ mqtt_cfg.client_id:%s\r\n",mqtt_cfg.client_id);
//     aiio_log_i("222 @@@@ mqtt_cfg.username:%s\r\n",mqtt_cfg.username);
//     aiio_log_i("222 @@@@ mqtt_cfg.password:%s\r\n",mqtt_cfg.password);
//     aiio_log_i("222 @@@@ device_secret:%s\r\n",psk_decode);

//     // free(mqtt_cfg.client_id);  
//     // free(mqtt_cfg.username); 
//     // free(mqtt_cfg.password);  
// }

void IOT_MQTT_Construct(aiio_device_info *pParams)
{
    aiio_log_i("device_info->product_id is %s\r\n",pParams->product_id);
    aiio_log_i("device_info->device_name is %s\r\n",pParams->device_name);
    aiio_log_i("device_info->device_secret is %s\r\n",pParams->device_secret);

    Qcloud_IoT_Client *mqtt_client = NULL;
    char              *client_id   = NULL;

    // create and init MQTTClient
    if ((mqtt_client = (Qcloud_IoT_Client *)malloc(sizeof(Qcloud_IoT_Client))) == NULL) {
        aiio_log_e("malloc MQTTClient failed");
        // return NULL;
    }

    // int rc = qcloud_iot_mqtt_init(mqtt_client, pParams);
    // if (rc != QCLOUD_RET_SUCCESS) {
    //     aiio_log_e("mqtt init failed: %d", rc);
    //     free(mqtt_client);
    //     return NULL;
    // }

    MQTTConnectParams connect_params = DEFAULT_MQTTCONNECT_PARAMS;
    client_id                        = malloc(MAX_SIZE_OF_CLIENT_ID + 1);
    if (client_id == NULL) {
        aiio_log_i("malloc client_id failed");
        // free(mqtt_client);
        // return NULL;
    }
    memset(client_id, 0, MAX_SIZE_OF_CLIENT_ID + 1);
    snprintf(client_id, MAX_SIZE_OF_CLIENT_ID, "%s%s", pParams->product_id, pParams->device_name);

    connect_params.client_id = client_id;
    // Upper limit of keep alive interval is (11.5 * 60) seconds
    // connect_params.keep_alive_interval = Min(pParams->keep_alive_interval_ms / 1000, 690);
    // connect_params.clean_session       = pParams->clean_session;
    // connect_params.auto_connect_enable = pParams->auto_connect_enable;

    if (pParams->device_secret == NULL) {
        aiio_log_i("Device secret is null!");
        // qcloud_iot_mqtt_fini(mqtt_client);
        // free(mqtt_client);
        // free(client_id);
        // return NULL;
    }
    size_t src_len = strlen(pParams->device_secret);
    size_t len;
    memset(mqtt_client->psk_decode, 0x00, DECODE_PSK_LENGTH);
    int rc                               = qcloud_iot_utils_base64decode(mqtt_client->psk_decode, DECODE_PSK_LENGTH, &len,
                                                                     (unsigned char *)pParams->device_secret, src_len);
    connect_params.device_secret     = (char *)mqtt_client->psk_decode;
    connect_params.device_secret_len = len;
    if (rc != QCLOUD_RET_SUCCESS) {
        aiio_log_i("Device secret decode err, secret:%s", pParams->device_secret);
        // qcloud_iot_mqtt_fini(mqtt_client);
        // HAL_Free(mqtt_client);
        // HAL_Free(client_id);
        // return NULL;
    }

    aiio_log_i("111 @@@@ connect_params.client_id:%s\r\n",connect_params.client_id);
    aiio_log_i("111 @@@@ connect_params.username:%s\r\n",connect_params.username);
    aiio_log_i("111 @@@@ connect_params.password:%s\r\n",connect_params.password);
    aiio_log_i("111 @@@@ connect_params.device_secret:%s\r\n",connect_params.device_secret);
    rc = qcloud_iot_mqtt_connect(mqtt_client, &connect_params);
    if (rc != QCLOUD_RET_SUCCESS) {
        // aiio_log_e("mqtt connect with id: %s failed: %d", mqtt_client->options.conn_id, rc);
        // qcloud_iot_mqtt_fini(mqtt_client);
        // HAL_Free(mqtt_client);
        // HAL_Free(client_id);
        // return NULL;
    } else {
        // aiio_log_i("mqtt connect with id: %s success", mqtt_client->options.conn_id);
    }

// #ifdef LOG_UPLOAD
//     // log subscribe topics
//     if (is_log_uploader_init()) {
//         set_log_mqtt_client((void *)mqtt_client);
//         int log_level;
//         rc = qcloud_get_log_level(&log_level);
//         if (rc < 0) {
//             aiio_log_e("client get log topic failed: %d", rc);
//         }

//         IOT_Log_Upload(true);
//     }
// #endif
    // return mqtt_client;
}