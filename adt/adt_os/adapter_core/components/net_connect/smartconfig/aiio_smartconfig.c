/** @brief      smartconfig api.
 *
 *  @file       aiio_smartconfig.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/04      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#include "aiio_smartconfig.h"
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include <lwip/netdb.h>
#include <string.h>

#include "aiio_wifi.h"

void wifi_sta_connect_sc(const char *ssid, const char *pwd, int channel)
{

}

int32_t aiio_smartconfig_register_cb(aiio_smartconfig_callback_t cb)
{
    return AIIO_OK;
}

void aiio_smartconfig_wifi_event(int32_t evt_code, void *param)
{

}

int32_t aiio_wifi_smartconfig_v1_start(void)
{

    return AIIO_OK;
}

int32_t aiio_wifi_smartconfig_v1_stop(void)
{

    return AIIO_OK;
}
