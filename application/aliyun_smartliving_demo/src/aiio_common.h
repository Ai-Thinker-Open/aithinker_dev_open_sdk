/**
 * @brief   声明一些公共的变量、结构体
 * 
 * @file    aiio_common.h
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * 
 * 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Authorhor           <th>Notes
 * <tr><td>2023-06-12          <td>1.0.0            <td>Ai-Thinker             <td>
 */

#ifndef __AIIO_COMMON_H_
#define __AIIO_COMMON_H_
#include "aiio_adapter_include.h"
#include "aiio_cloud_key.h"
#include "aiio_cloud_led.h"

#define FLASH_RW_START_ADDR                 (CONFIG_FLASH_RW_START_ADDR)
#define FLASH_RW_WIFI_DATA_ADDR              (FLASH_RW_START_ADDR)
#define FLASH_RW_DEVICE_INFO_ADDR              (CONFIG_FLASH_RW_START_ADDR + CONFIG_FLASH_REGION_SIZE)
#define FLASH_RW_OTA_INFO_ADDR              (CONFIG_FLASH_RW_START_ADDR + CONFIG_FLASH_REGION_SIZE * 2)


typedef enum
{
    REV_CONFIG_DATA_EVENT,                          /*!< Get  config data event in  distribution network, it will callback when device got  config data*/
    REV_CONFIG_START_EVENT,                         /*!< Start event in  distribution network,it will callback when key is pressed*/
    REV_CONFIG_STOP_EVENT,                          /*!< Stop event in  distribution network,it will callback when key is pressed*/
    REV_BLE_DISCONNECTED_EVENT,                         /*!< BLE disconnect event, it will callback when device disconnected with APP*/
    REV_CONFIG_OK_EVENT,                            /*!< Success event in  distribution network，it will callback when device is online*/
    REV_CONFIG_FAIL_EVENT,                          /*!< FAIL event in  distribution network，it will callback when device is fail in  distribution network*/
    REV_WIFI_CONNECTED_EVENT,                           /*!< Wifi connected event，it will callback when device connected wifi*/
    REV_CLOUD_CONTRL_DATA_EVENT,                        /*!< Received to be control attribute event，it will callback when device received data from cloud*/
    REV_CLOUD_ONLINE_EVENT,                              /*!< Online event，it will callback when device report online status to cloud */
    REV_CLOUD_CONNECTED_EVENT,                          /*!< Cloud connectedc event, it will callback when device connected cloud by mqtt*/
    REV_CLOUD_DISCONNECTED,                              /*!< Cloud disconnected event, it will callback when device disconnected cloud by mqtt*/
    REV_CLOUD_OTA_INFO_EVENT,                           /*!< recevice ota information, ota information need to be stored in flash, the ota version needs to be used to determine whether the firmware upgrade is successful*/
    REV_CLOUD_OTA_FAIL_EVENT,                                 /*!< ota data download fail*/
    REV_CLOUD_OTA_SUCCESS_EVENT,                              /*!< ota data download success*/
}aiio_common_event_t;

typedef struct
{
    char ssid[33];                              /*!< The ssid of rout*/
    char passwd[65];                             /*!< The password of rout */
    char region_mqtturl[128];
}wifi_config_data_t;

typedef struct 
{
    char *queue_data;                      
    uint16_t queue_data_len;                
    aiio_common_event_t common_event;      
}aiio_rev_queue_t;


/* MAX size of client ID */
#define MAX_SIZE_OF_CLIENT_ID (80)

/* MAX size of product ID */
#define MAX_SIZE_OF_PRODUCT_ID (10)

/* MAX size of device name */
#define MAX_SIZE_OF_DEVICE_NAME (48)

/* MAX size of device secret */
#define MAX_SIZE_OF_DEVICE_SECRET (64)

/* Max size of conn Id  */
#define MAX_CONN_ID_LEN (6)

#define QCLOUD_IOT_DEVICE_SDK_APPID     "21010406"
#define QCLOUD_IOT_DEVICE_SDK_APPID_LEN (sizeof(QCLOUD_IOT_DEVICE_SDK_APPID) - 1)

/* MAX valid time when connect to MQTT server. 0: always valid */
/* Use this only if the device has accurate UTC time. Otherwise, set to 0 */
#define MAX_ACCESS_EXPIRE_TIMEOUT (0)

/* Max size of base64 encoded PSK = 64, after decode: 64/4*3 = 48*/
#define DECODE_PSK_LENGTH 48

/* default MQTT Tx buffer size, MAX: 16*1024 */
#define QCLOUD_IOT_MQTT_TX_BUF_LEN (2048)


#define PID_STRLEN_MAX              (64)
#define MID_STRLEN_MAX              (64)
#define GUIDER_SIGN_LEN             (48)
#define GUIDER_DYNAMIC_URL_LEN      (256)
#define GUIDER_URL_LEN              (128)
#define PRODUCT_KEY_MAXLEN          (20 + 1)
#define DEVICE_NAME_MAXLEN          (32 + 1)
#define DEVICE_SECRET_MAXLEN        (64 + 1)
#define PRODUCT_SECRET_MAXLEN       (64 + 1)

typedef struct 
{
    char product_key[PRODUCT_KEY_MAXLEN+1];
    char product_Secret[PRODUCT_SECRET_MAXLEN+1];
    char device_name[DEVICE_NAME_MAXLEN+1];
    char device_secret[DEVICE_SECRET_MAXLEN+1];
	uint32_t product_Id;
}iotx_device_info_t;


/* Max size of a topic name */
#define MAX_SIZE_OF_CLOUD_TOPIC ((MAX_SIZE_OF_DEVICE_NAME) + (MAX_SIZE_OF_PRODUCT_ID) + 64 + 6)

#define DEFAULT_MQTTCONNECT_PARAMS                                            \
    {                                                                         \
        NULL, NULL, NULL, {0}, {'M', 'Q', 'T', 'C'}, 0, 4, 240, 1, 1, NULL, 0 \
    }
extern aiio_mqtt_client_config_t mqtt_cfg;
extern iotx_device_info_t DeviceInfo_info;
extern aiio_mqtt_client_handle_t client;
extern QueueHandle_t    cloud_rev_queue_handle;        /*!< The receive queue data handle*/
extern wifi_config_data_t wifi_config_data;
extern breeze_apinfo_t apinfo;

uint64_t HAL_GetTimeMs(void); 
long HAL_Timer_current_sec(void);
void get_next_conn_id(char *conn_id);

int aiio_flash_get_device_info(iotx_device_info_t *device_info);
int aiio_flash_clear_device_info(iotx_device_info_t *device_info);
int aiio_flash_save_device_info(iotx_device_info_t *device_info);

int aiio_flash_get_wifi_config_data(wifi_config_data_t *config_data);
void aiio_flash_save_wifi_config_data(wifi_config_data_t *config_data);
void aiio_flash_clear_config_data(void);
int32_t AilinkFlashWrite(char *block, uint8_t *data, uint16_t data_len);
int32_t AilinkFlashRead(char *block, uint8_t *buf, uint16_t buf_max_len);
#endif