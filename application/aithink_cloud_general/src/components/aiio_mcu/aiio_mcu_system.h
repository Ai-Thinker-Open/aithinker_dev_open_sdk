#ifndef __AIIO_MCU_SYSTEM_H_
#define __AIIO_MCU_SYSTEM_H_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "aiio_list.h"
#include "cJSON.h"

#define AIIO_UART0_BAUDRATE                         CONFIG_UART0_BAUDRATE
#define AIIO_UART0_TX_PIN                           CONFIG_UART0_TX_PIN
#define AIIO_UART0_RX_PIN                           CONFIG_UART0_RX_PIN
#define AIIO_UART0_RECEIVE_BUFFER_MAX_LEN           CONFIG_UART0_RECEIVE_BUFFER_MAX_LEN

#define AIIO_UART1_BAUDRATE                         CONFIG_UART1_BAUDRATE
#define AIIO_UART1_TX_PIN                           CONFIG_UART1_TX_PIN
#define AIIO_UART1_RX_PIN                           CONFIG_UART1_RX_PIN
#define AIIO_UART1_RECEIVE_BUFFER_MAX_LEN           CONFIG_UART1_RECEIVE_BUFFER_MAX_LEN

typedef struct
{
    char deviceid[65];                          /**< 设备ID */
    char productid[65];                         /**< 产品ID */
    char fw_version[10];                        /**< 模组固件版本号 */
    char mcu_version[10];                       /**< MCU固件版本号 */
    uint8_t pair_type;                          /**< 配置配网类型，如：AP和BLE共存配网为AILINK_PAIR_AP_BLE_TYPE */
}profile_info_t;

//=============================================================================
//数据帧字段位
//=============================================================================
#define         HEAD_H                          0
#define         HEAD_L                          1        
#define         PRO_VER                         2
#define         CMD_TYPE                        3
#define         LEN_H                           4
#define         LEN_L                           5
#define         DATA_LOCAL                      6
//=============================================================================
//数据帧类型
//=============================================================================
#define         HEART_BEAT_CMD                  0                               //心跳包
#define         PRODUCT_INFO_CMD                1                               //产品信息
#define         WORK_MODE_CMD                   2                               //查询MCU 设定的模块工作模式	
#define         WIFI_STATE_CMD                  3                               //wifi工作状态	
#define         WIFI_RESET_CMD                  4                               //重置wifi
#define         WIFI_MODE_CMD                   5                               //选择ble/AP模式	
#define         DATA_QUERT_CMD                  6                               //命令下发
#define         STATE_UPLOAD_CMD                7                               //状态上报	 
#define         STATE_QUERY_CMD                 8                               //状态查询   
#define         UPDATE_START_CMD                0x0a                            //升级开始
#define         UPDATE_TRANS_CMD                0x0b                            //升级传输
#define         GET_ONLINE_TIME_CMD             0x0c                            //获取系统时间(格林威治时间)
#define         FACTORY_MODE_CMD                0x0d                            //进入产测模式    
#define         WIFI_TEST_CMD                   0x0e                            //wifi功能测试
#define         GET_LOCAL_TIME_CMD              0x1c                            //获取本地时间
#define         WEATHER_OPEN_CMD                0x20                            //打开天气          
#define         WEATHER_DATA_CMD                0x21                            //天气数据
#define         STATE_UPLOAD_SYN_CMD            0x22                            //状态上报（同步）
#define         STATE_UPLOAD_SYN_RECV_CMD       0x23                            //状态上报结果通知（同步）
#define         GET_WIFI_RSSI_CMD               0x24                            //获取路由的rssi信号值
#define         HEAT_BEAT_STOP                  0x25                            //关闭WIFI模组心跳
#define         STREAM_TRANS_CMD                0x28                            //流数据传输
#define         GET_WIFI_STATUS_CMD             0x2b                            //获取当前wifi联网状态
#define         WIFI_CONNECT_TEST_CMD           0x2c                            //wifi功能测试(连接指定路由)
#define         GET_MAC_CMD                     0x2d                            //获取模块mac
#define         GET_IR_STATUS_CMD               0x2e                            //红外状态通知
#define         IR_TX_RX_TEST_CMD               0x2f                            //红外进入收发产测
#define         MAPS_STREAM_TRANS_CMD           0x30                            //流数据传输(支持多张地图)
#define         FILE_DOWNLOAD_START_CMD         0x31                            //文件下载启动
#define         FILE_DOWNLOAD_TRANS_CMD         0x32                            //文件下载数据传输
#define         MODULE_EXTEND_FUN_CMD           0x34                            //模块拓展服务
#define         BLE_TEST_CMD                    0x35                            //蓝牙功能性测试（扫描指定蓝牙信标）
#define         GET_VOICE_STATE_CMD             0x60                            //获取语音状态码
#define         MIC_SILENCE_CMD                 0x61                            //MIC静音设置
#define         SET_SPEAKER_VOLUME_CMD          0x62                            //speaker音量设置
#define         VOICE_TEST_CMD                  0x63                            //语音模组音频产测
#define         VOICE_AWAKEN_TEST_CMD           0x64                            //语音模组唤醒产测
#define         VOICE_EXTEND_FUN_CMD            0x65                            //语音模组扩展功能
#define         UPDATE_LAST_CMD                 0x66                            //mcu ota最后一包


//=============================================================================
#define         SED_VER                  0x10                                            //模块发送帧协议版本号
#define         REV_VER                  0x20                                            //MCU 发送帧协议版本号(默认)
#define         FRAME_MINI_LEN           0x07                                            //固定协议头长度
#define         FRAME_H                  0x5a                                            //帧头高位
#define         FRAME_L                  0xa5                                            //帧头低位
//============================================================================= 

//物模型数据类型
//=============================================================================
#define         DATA_TYPE_BOOL                    0x01        //bool 类型
#define         DATA_TYPE_VALUE                   0x02        //value 类型
#define         DATA_TYPE_STRING                  0x03        //string 类型
#define         DATA_TYPE_ENUM                    0x04        //enum 类型

//=============================================================================

//=============================================================================
//WIFI工作状态
//=============================================================================
#define         BLE_CONFIG_STATE                        0x00
#define         AP_CONFIG_STATE                         0x01
#define         WIFI_NOT_CONNECTED                      0x02
#define         WIFI_CONNECTED                          0x03
#define         WIFI_CONN_CLOUD                         0x04
#define         WIFI_LOW_POWER                          0x05
#define         BLE_AND_AP_STATE                        0x06
#define         WIFI_SATE_UNKNOW                        0xff
//=============================================================================

//=============================================================================
//ota 状态码
//ota 状态码
#define                 OTA_STATUS_INVAIL                                           0
#define                 OTA_STATUS_START                                            1
#define                 OTA_STATUS_MD5_ERR                                          2
#define                 OTA_STATUS_INSTALL                                          3
#define                 OTA_STATUS_OK                                               4

/**
 * @brief   模组向MCU下发命令数据
 * 
 * @param[in]   data        待下发的命令数据
 * @param[in]   dataLen     数据长度
 * @return  int8_t 
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t aiio_control_send_mcu(uint8_t *data, uint16_t dataLen);

/**
 * @brief   发送协议帧数据的串口函数
 * 
 * @note    将数据域内容、帧命令通过该函数，可将数据包装成协议帧数据，然后通过串口发送出去。
 * 
 * @param[in]   cmdType     帧命令
 * @param[in]   data        数据域内容
 * @param[in]   dataLen     数据长度
 * @return  int8_t          返回数据发送情况
 *          0：发送成功
 *          -1：发送失败
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t aiio_send_mcu_cmd(uint8_t cmdType, uint8_t *data, uint16_t dataLen);

/**
 * @brief   串口数据处理
 * 
 * @note    该函数需在while循环中循环调用
 * 
 * @return  int8_t 返回处理结果
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t UartProcessPro(void);

/**
 * @brief   串口协议初始化
 * 
 * @note    该函数需在mcu初始化时调用该函数初始化串口处理。
 *  
 * @return  int8_t      返回初始化状态 
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t UartProtocolInit(void);

/**
 * @brief   该函数主要是逐一字节接收串口数据，若是串口采用缓存方式处理串口数据，可调用UartRevStream该函数处理。
 * 
 * @param[in]   data        串口数据
 * @return  int8_t         返回数据处理情况
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t UartRevStream(uint8_t *data, uint16_t data_len);

bool McuCmdDataListInit(void);

/**
 * @brief   向云端上报MCU上传的数据
 * 
 * 
 * @author  hly
 * @date    2023-09-13
 */
void McuPubliReportAttributes(void);

/**
 * @brief   心跳包数据回应
 * 
 * 
 * @author  hly
 * @date    2023-09-13
 */
void UartSendHeatBeat(void);

/**
 * @brief   获取模组与MCU的连接状态
 * 
 * @return  true    已连接
 * @return  false   未连接
 * 
 * @author  hly
 * @date    2023-09-13
 */
bool GetMCUConnectState(void);

/**
 * @brief   获取心跳状态
 * 
 * @return  true    心跳发送多次未回复，已超时
 * @return  false   心跳正常
 * 
 * @author  hly
 * @date    2023-09-13
 */
bool HeartBeatTimeout(void);

/**
 * @brief   设置WiFi网络状态
 * 
 * @param[in]   State  网络状态
 * 
 * @author  hly
 * @date    2023-09-13
 */
void SetWifiNetWorkState(uint8_t State);

/**
 * @brief   获取产品ID
 * 
 * @return  char*  产品ID数据
 * 
 * @author  hly
 * @date    2023-09-13
 */
char *GetProductID(void);

/**
 * @brief   获取产品标识
 * 
 * @return  char*  产品标识数据
 * 
 * @author  hly
 * @date    2023-09-13
 */
char *GetProductFlag(void);

/**
 * @brief   信号量加锁
 * 
 * @return  int32_t 
 * 
 * @author  hly
 * @date    2023-09-13
 */
static int32_t MutexLock(void);

/**
 * @brief   信号量解锁
 * 
 * @return  int32_t 
 * 
 * @author  hly
 * @date    2023-09-13
 */
static int32_t MutexUnLock(void);

/**
 * @brief   计算校验值，从帧头至协议内容尾字节累加求和后再对256取余
 * 
 * @param[in]   dataBuff        数据内容
 * @param[in]   dataLen         数据长度
 * @return  uint8_t             返回校验和
 * 
 * @author  hly
 * @date    2023-09-13
 */
uint8_t UartCheckValue(uint8_t *dataBuff, uint16_t dataLen);

/**
 * @brief   串口协议帧打包函数
 * 
 * @note   主要是处理将帧命令以及数据域数据，按照协议格式将数据打包在发送缓冲区中
 * 
 * @param[in]   packetBuff   发送缓冲区
 * @param[in]   cmdType       帧命令
 * @param[in]   data          数据内容
 * @param[in]   dataLen       数据内容长度
 * @return  uint16_t         返回打包好的数据长度
 * 
 * @author  hly
 * @date    2023-09-13
 */
static uint16_t UartProFramePacket(uint8_t *packetBuff, uint8_t cmdType, uint8_t *data, uint16_t dataLen);

/**
 * @brief   检查MCU设备是否重新启动
 * 
 * @return  true        MCU设备已重新启动
 * @return  false       MCU设备未重新启动
 * 
 * @author  hly
 * @date    2023-09-13
 */
bool CheckMcuDeviceStart(void);

/**
 * @brief   向MCU回复模组的MAC地址
 * 
 * 
 * @author  hly
 * @date    2023-09-13
 */
static void UartSendMac(void);

/**
 * @brief   向MCU回复本地时间
 * 
 * 
 * @author  hly
 * @date    2023-09-13
 */
static void UartSendLocalTime(void);

/**
 * @brief   处理MCU发送的产品信息是否齐全
 * 
 * @return  true           产品信息未接收齐全
 * @return  false          产品信息已接收齐全
 * 
 * @author  hly
 * @date    2023-09-13
 */
static bool ProductInfoIsEmpty(void);

/**
 * @brief   获取MCU版本号
 * 
 * @return  char* MCU版本号数据
 * 
 * @author  hly
 * @date    2023-09-13
 */
char *GetMcuVersion(void);

/**
 * @brief   获取profile信息
 * 
 * @return  profile_info_t*  返回获取profile存储的信息
 * 
 * @author  hly
 * @date    2023-09-13
 */
profile_info_t *AilinkGetProfileInfo(void);

/**
 * @brief   解析产品信息
 * 
 * @param[in]   info            从MCU上报的json字符串
 * @param[in]   data_len        json字符串数据长度
 * 
 * @author  hly
 * @date    2023-09-13
 */
static void ParseProductInfo(uint8_t *info, uint16_t data_len);

/**
 * @brief   回复测试WiFi连接情况
 * 
 * @param[in]   state
 *              true：WiFi已连接成功
 *              false: WiFi未连接成功
 * 
 * @author  hly
 * @date    2023-09-13
 */
static void ResponseWifiTestState(bool state);

/**
 * @brief   数据帧处理，执行帧命令
 * 
 * @param[in]   DataCmd     数据帧
 * @param[in]   data_len    数据帧长度
 * @return  uint8_t         返回执行结果
 *          0：执行成功
 *          -1：执行失败
 * 
 * @author  hly
 * @date    2023-09-13
 */
uint8_t ProcessUartCmd(uint8_t *DataCmd, uint16_t data_len);

/**
 * @brief   接收串口缓存数据
 * 
 * @param[in]   data            待处理的串口数据
 * @param[in]   data_len        串口数据大小
 * @return  int8_t             返回数据处理情况
 * 
 * @author  hly
 * @date    2023-09-13
 */
int8_t UartRevStream(uint8_t *data, uint16_t data_len);

/**
 * @brief   判断接收缓冲区是否为空
 * 
 * @return  true  接收缓冲区为空
 * @return  false 接收缓冲区不为空
 * 
 * @author  hly
 * @date    2023-09-13
 */
bool revBuffIsEmpty(void);

/**
 * @brief   从接收缓冲区获取数据
 * 
 * @return  uint8_t  已获取到的数据
 * 
 * @author  hly
 * @date    2023-09-13
 */
uint8_t TakeDataFromBuff(void);

/**
 * @brief 发送MCU设备可升级的OTA信息
 * 
 * @param ver   待升级的OTA版本号
 * @param md5   待升级OTA固件的md5数据
 */
void NotifyMcuDeviceUpdate(char *ver, char *md5);

/**
 * @brief 发送给MCU设备的OTA数据，OTA数据传输时，主要是分包进行传输
 * 
 * @param data      OTA数据内容
 * @param data_len  OTA数据内容长度
 */
void TransferMcuDeviceUOtaData(char *data, uint16_t data_len);

/**
 * @brief 获取 MCU设备处理OTA状态
 * 
 * @return uint8_t 
 *      OTA_STATUS_INVAIL：无效状态
 *      OTA_STATUS_START：MCU设备已准备好开始处理OTA数据
 *      OTA_STATUS_MD5_ERR：MCU设备md5校验OTA数据错误
 *      OTA_STATUS_INSTALL：MCU设备安装固件中
 *      OTA_STATUS_OK：MCU设备已OTA完成
 */
uint8_t GetMcuOTAStatus(void);

/**
 * @brief 重置OTA状态
 * 
 */
void ResetMcuOTAStatus(void);
#endif