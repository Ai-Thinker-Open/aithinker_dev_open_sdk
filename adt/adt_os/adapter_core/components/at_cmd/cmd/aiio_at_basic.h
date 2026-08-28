/** @brief      AT basic command.
 *
 *  @file       aiio_at_basic.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_BASIC_H__
#define __AIIO_AT_BASIC_H__

#include "aiio_type.h"

// #define AIIO_CONFIG_IPV6 1

typedef struct __AI_UartConfig{
    uint8_t dataBits;
    uint8_t stopBits;
    uint8_t parity;
    uint32_t baudrate;
} aiio_at_uart_t;

//OTA升级方式
typedef enum {
	AIIO_OTA_MODE_HTTP=1,	//使用HTPP方式升级
	AIIO_OTA_MODE_HTTPS=2,	//使用HTPPS方式升级
}aiio_ota_mode_t;

//定义OTA升级相关参数
typedef struct {
	aiio_ota_mode_t mode;	//请求方式
	char host_name[128];	//服务器域名
	int port;	//服务器端口
	char route[256];	//资源地址
}aiio_at_ota_t;


//系统URC数据打印掩码
#define AIIO_SYS_MSG_WIFI_DISCONNECT_MASK	(1<<0)	//WIFI断开
#define AIIO_SYS_MSG_WIFI_CONNECT_MASK	(1<<1)	//wifi连接
#define AIIO_SYS_MSG_WIFI_APCLIENTDISCONNECT_MASK	(1<<2)	//AP模式客户端断开连接
#define AIIO_SYS_MSG_WIFI_APCLIENTCONNECTED_MASK	(1<<3)	//AP模式有新的客户端连接
#define AIIO_SYS_MSG_SOCKET_DOWN_MASK	(1<<4)	//收到socket数据
#define AIIO_SYS_MSG_SOCKET_SEED_MASK	(1<<5)	//收到新的客户端连接
#define AIIO_SYS_MSG_SOCKET_DISCONNECT_MASK	(1<<6)	//socket断开
#define AIIO_SYS_MSG_SOCKET_RECONNECT_MASK	(1<<7)	//socket断开自动重连成功
#define AIIO_SYS_MSG_SOCKET_AUTO_DEL_MASK	(1<<8)	//socket自动删除
#define AIIO_SYS_MSG_MQTT_CONNECT_MASK	(1<<9)	//MQTT连接成功
#define AIIO_SYS_MSG_MQTT_DISCONNECT_MASK	(1<<10)	//MQTT连接断开
#define AIIO_SYS_MSG_MQTT_SUB_MASK	(1<<11)	//接收到了订阅消息
#define AIIO_SYS_MSG_BLE_DISCONNECT_MASK	(1<<12)	//蓝牙断开
#define AIIO_SYS_MSG_BLE_CONNECTED_MASK	(1<<13)	//蓝牙连接成功
#define AIIO_SYS_MSG_BLE_DATA_MASK	(1<<14)	//主机模式下收到蓝牙透传数据
#define AIIO_SYS_MSG_WIFI_GOT_IP_MASK	(1<<15)	//STA获取ip
#define AIIO_SYS_MSG_WIFI_SCAN_DON_MASK	(1<<16)	//wifi扫描结束
#define AIIO_SYS_MSG_SNTP_SYNC_DON_MASK	(1<<17)	//SNTP时间同步成功

typedef struct {
    aiio_at_uart_t uart;
    aiio_at_ota_t ota;
    uint8_t aiio_ipv6_flag;
    uint8_t aiio_ate_flag;
    //系统信息打印标记位，标记透传模式哪些URC数据需要打印
	uint32_t sysMsgPrintInTTMode;
} aiio_at_basic_config_t;

//OTA相关配置
#define AI_CONFIG_OTA_THREAD_SIZE	(1024*8*4)	//OTA升级任务堆栈大小
#define AI_CONFIG_OTA_THREAD_PRIORITY	(3)	//OTA升级任务优先级(0最低)


extern aiio_at_basic_config_t at_basic_config;

CORE_API int32_t aiio_at_basic_cmd_regist(void);

#endif
