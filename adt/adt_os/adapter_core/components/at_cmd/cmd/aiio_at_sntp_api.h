/** @brief      AT sntp API .
 *
 *  @file       aiio_at_sntp_api.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/11/09      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_SNTP_API_H__
#define __AIIO_AT_SNTP_API_H__

#include "aiio_type.h"
#include <time.h>
// #include <sntp.h>
#include "lwip/apps/sntp.h"




typedef struct
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t mday;
    uint8_t mon;
    uint16_t year;
    uint8_t wday;
} aiio_timestamp_t;


int32_t aiio_sntp_get_interval(void);
int32_t aiio_sntp_set_interval(int interval);
int32_t aiio_sntp_set_servername(char *sntpServerName1,char *sntpServerName2,char *sntpServerName3);
int32_t aiio_sntp_get_serverName(char *sntpServerName1,char *sntpServerName2,char *sntpServerName3);
int32_t aiio_sntp_get_time(aiio_timestamp_t *time);
int32_t aiio_sntp_enable(uint8_t mode);
int32_t aiio_sntp_set_timezone(int timezone);
int32_t aiio_sntp_get_timezone(void);
uint8_t aiio_sntp_flush_success(void);


#define AIIO_SNTP_SERVER_NAME_SIZE	(65)	//SNTP服务器域名长度(实际长度为 AI_SNTP_SERVER_NAME_SIZE-1 最后有一个结束符)

#endif