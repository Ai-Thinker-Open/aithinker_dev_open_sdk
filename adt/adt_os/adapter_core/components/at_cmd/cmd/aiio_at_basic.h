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
	char route[128];	//资源地址
}aiio_at_ota_t;


typedef struct {
    aiio_at_uart_t uart;
    aiio_at_ota_t ota;
} aiio_at_basic_config_t;


//OTA相关配置
#define AI_CONFIG_OTA_THREAD_SIZE	(1024*8*4)	//OTA升级任务堆栈大小
#define AI_CONFIG_OTA_THREAD_PRIORITY	(3)	//OTA升级任务优先级(0最低)


extern aiio_at_basic_config_t at_basic_config;

CORE_API int32_t aiio_at_basic_cmd_regist(void);

#endif
