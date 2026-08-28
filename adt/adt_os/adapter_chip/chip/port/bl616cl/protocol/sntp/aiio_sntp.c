/**
 * @brief      SNTP application interface.
 *
 * @file       aiio_sntp.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       SNTP application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2024/05/14      <td>1.0.0       <td>wusen       <td>add sntp api
 * </table>
 *
 */

#include "stdio.h"
#include "string.h"
#include <time.h>
#include "lwip/apps/sntp.h"

#include "aiio_error.h"
#include "aiio_sntp.h"

int32_t aiio_hal_sntp_get_last_time(uint32_t *seconds, uint32_t *frags)
{
    return sntp_get_time(seconds,frags);
}

int32_t aiio_hal_sntp_set_server_name(uint8_t idx, const char *server)
{
    sntp_setservername(idx,server);
    return AIIO_OK;
}

int32_t aiio_hal_sntp_set_operating_mode(aiio_sntp_operating_mode_t operating_mode)
{
    sntp_setoperatingmode(operating_mode);
    return AIIO_OK; 
}

