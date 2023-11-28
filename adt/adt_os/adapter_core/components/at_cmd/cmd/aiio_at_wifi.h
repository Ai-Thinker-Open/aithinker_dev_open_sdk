/** @brief      AT wifi command.
 *
 *  @file       aiio_at_wifi.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_WIFI_H__
#define __AIIO_AT_WIFI_H__

#include "aiio_type.h"
#include "aiio_at_wifi_api.h"


CORE_API int32_t aiio_at_wifi_cmd_regist(void);
CORE_API void aiio_at_apinfo_print(uint16_t index,char *SSID,uint8_t channel,aiio_at_wifi_security_t security,int32_t rssi,uint8_t *pBssid);

#endif
