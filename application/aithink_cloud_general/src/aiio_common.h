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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_protocol_comm.h"
#include "aiio_rtc_timer.h"
#include "aiio_ble_config_wifi.h"
#include "aiio_cloud_key.h"
#include "aiio_cloud_led.h"
#include "aiio_cloud_rtc.h"
#include "aiio_cloud_control.h"
#include "aiio_cloud_receive.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <event_groups.h>
#include "queue.h"


#define         DEVICE_FW_VERSION           ("1.2.1")                   /*!< The version of firmware，it will report to cloud，and show on APP*/

#define _MACROSTR(x) #x
#define MACROSTR(x) _MACROSTR(x)
#define BUILDTIME MACROSTR(XUE_COMPILE_TIME)

#define         CMD_LOG_ENABLE                  0
#define         CMD_RESPONSE_ENABLE             1
#define filename(x) strrchr(x,'/')?strrchr(x,'/')+1:x
#define DEVICE_INFO_OUTPUT(x)\
				do{\
						if(CMD_RESPONSE_ENABLE)\
						{\
								if(CMD_LOG_ENABLE)\
									printf("[IOT Info : %s, %d, %s] ",  filename(__FILE__), __LINE__,__FUNCTION__);\
								aiio_uart_send_data(AIIO_UART1,(uint8_t *)x, strlen(x));\
						}\
					}while(0)



#define FLASH_RW_START_ADDR                 (CONFIG_FLASH_RW_START_ADDR)
#define FLASH_RW_WIFI_DATA_ADDR             (FLASH_RW_START_ADDR)
#define FLASH_RW_OTA_INFO_ADDR              (CONFIG_FLASH_RW_START_ADDR + CONFIG_FLASH_REGION_SIZE)

#define DEVICE_INFO_ADDR                    (CONFIG_DEVICE_INFO_ADDR)

#define  DEVICE_PARAMS_FLASH_BLOCK              "device_params"
#define DEVICE_TIME_INFO_FLASH_BLOCK            "time_info"
#define DEVICE_PAIR_INFO_FLASH_BLOCK            "pair_info"
#define   OTA_INSTALL_FLAG                      "ota_flag"
#define DEVICE_OTA_INFO_FLASH_BLOCK              "ota_info"

#define IOT_OTA_TYPE_MODULE                     "module_ota_all"
#define IOT_OTA_TYPE_MCU                        "module_mcu_all"


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
    char wificc[10];                             /*!< Country code */
    char mqttip[33];                             /*!< The ip address or doman of  remote service  */
    char token[33];                             /*!< Activaty information，it is given from cloud by app */
    char tz[10];                                /*!< time zone */
    char ts[16];                                /*!< time stamp */
    char pro_key[65];
    uint16_t  port;                             /*!< The port number of remote service*/
}wifi_config_data_t;


typedef struct 
{
    uint8_t channel;                    /**< 升级渠道，1：云端，0：APP */
    char    pointVer[10];               /**< 指定目标版本号，即指定某一个版本号可升级 */
    char    baseVer[10];                /**< 固件最低兼容版本号,例如：最低兼容版本号是2.0.1，而2.0.5该版本可升级，而1.5.0该版本不可升级 */
    char    OtaVer[10];                 /**< OTA固件待升级的版本号 */
    char    mcuBaseVer[10];             /**< 指定MCU最低兼容版本号，例如：最低兼容版本号是2.0.1，而2.0.5该版本可升级，而1.5.0该版本不可升级 */
    char    otaType[20];                /**< OTA包下载类型，如：module_ota_all为整包下载*/
    char    download_url[400];          /**< OTA包下载链接 */
    char    PubId[33];                  /**< 云端下发的发布id */
    char    md5[33];                    /**< 云端下发OTA数据对应生成的md5数据*/
    uint32_t timeout;                   /**< OTA超时时间*/
}aiio_ota_msg_t;


typedef struct 
{
    char userName[65];
    char userPasswd[65];
    char deviceId[64];
}aiio_device_info;

typedef struct
{
    uint32_t uart_bound;            /**< 设置设备上串口1的波特率 */
    uint8_t  work_mode;             /**< 设置设备的工作模式，是AT指令的工作模式，还是MCU SDK的工作模式*/

}aiio_device_params;

typedef struct 
{
    char *queue_data;                      
    uint16_t queue_data_len;                
    aiio_common_event_t common_event;      
}aiio_rev_queue_t;


extern QueueHandle_t    cloud_rev_queue_handle;        /*!< The receive queue data handle*/                           
extern bool ble_config_start;                       /*!< The launch status in ble distribution network*/
extern bool wifi_config_start;
extern bool wifi_connect;
extern aiio_device_info DeviceInfo;
extern aiio_device_params DeviceParams;
extern aiio_rtc_time_t UpTime;
extern bool ailinkInitstate;
extern bool waterfall_light;
extern bool waterfall_light_state;
extern uint8_t waterfall_num;
extern bool at_node_mcutest;
extern aiio_ota_msg_t  OtaProInfo;
extern AIIO_TIMER wifi_watchdog_timer_handle;
extern bool ota_transmit_init;
extern bool mcu_ota_start_flag;




/**
 * @brief   从flash中获取三元组数据
 * 
 * @param[in]   deviceInfo    待获取三元组数据的结构空间
 * 
 * @author  hly
 * @date    2023-09-27
 */
void iotgoGetDeviceInfo(aiio_device_info *deviceInfo);

/**
 * @brief   从存储三元组数据于flash
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotSaveDeviceInfo(void);

/**
 * @brief   检查三元组数据
 * 
 * @param[in]   deviceInfo    传入的三元组数据
 * @return  bool          返回数据校验情况
 *          1：校验成功
 *          0：校验失败
 * @author  hly
 * @date    2023-09-27
 */
bool iotCheckDeviceInfoIsEmpty(aiio_device_info *deviceInfo);

/**
 * @brief   存储设备参数数据
 * 
 * @param[in]   params      待存储数据参数
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotSaveDeviceParams(aiio_device_params *params);

/**
 * @brief   清除设备存储的参数数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotClearDeviceParams(void);

/**
 * @brief   读取设备存储的参数数据
 * 
 * @param[in]   params  已读取的参数数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotLoadDeviceParams(aiio_device_params *params);

/**
 * @brief   存储设备启动时的时间
 * 
 * @note    当设备启动后，通过UTC获取当前时间后，并将时间存储与flash上
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkSaveTimeInfo(void);

/**
 * @brief   清除flash存储的时间数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkClearTimeInfo(void);

/**
 * @brief   从flash中获取设备启动的时间
 * 
 * @param[in]   RtcTime    待获取的时间数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkLoadTimeInfo(aiio_rtc_time_t *RtcTime);

/**
 * @brief   存储配网信息函数
 * 
 * @note    当配网成功后，可通过该函数将配网数据存储在flash上
 * 
 * @param[in]   info    待存储的配网数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkSavePairInfo(wifi_config_data_t *info);

/**
 * @brief   清除flash存储的配网信息
 * 
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkClearPairInfo(void);

/**
 * @brief   从flash中获取配网信息函数
 * 
 * @param[in]   info    待获取配网数据的结构空间
 * 
 * @author  hly
 * @date    2023-09-27
 */
int AilinkLoadPairInfo(wifi_config_data_t *info);

/**
 * @brief   设置ota安装状态
 * 
 * @param[in]   ota_status
 *              true: ota已安装成功
 *              false：ota未安装成功
 * 
 * @author  hly
 * @date    2023-09-27
 */
void AilinkSetOtaInstallStatus(bool ota_status);

/**
 * @brief   获取ota安装状态
 * 
 * @return  true        ota已安装成功
 * @return  false       ota安装失败
 * 
 * @author  hly
 * @date    2023-09-27
 */
bool AilinkGetOtaInstallStatus(void);

/**
 * @brief   清除flash上的配网数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotClearOTAInfo(void);

/**
 * @brief   从flash读取配网数据
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotLoadOTAInfo(void);

/**
 * @brief   将配网数据存储于flash
 * 
 * @author  hly
 * @date    2023-09-27
 */
void IotSaveOTAInfo(void);

/**
 * @brief   重新进入配网函数
 * 
 * @author  hly
 * @date    2023-09-27
 */
void aiio_repair(void);

/**
 * @brief   连接路由函数
 * 
 * @author  hly
 * @date    2023-09-27
 */
void iot_connect_wifi(char *ssid, char *passwd);

/**
 * @brief   获取Tick时间差
 * 
 * @param[in]   start    开始的tick时间
 * @param[in]   end      结束的tick时间
 * @return  uint32_t     返回tick时间差
 * 
 * @author  hly
 * @date    2023-09-27
 */
uint32_t diffTimeTick(uint32_t start, uint32_t end);

#endif
