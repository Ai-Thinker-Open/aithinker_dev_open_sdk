/**
 * @brief      rtc application interface.
 *
 * @file       aiio_rtc.c
 * @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note       rtc application interface.
 * @par        Change Logs:
 * <table>
 * <tr><th>Date            <th>Version     <th>Author      <th>Notes
 * <tr><td>2024/05/22      <td>1.0.0       <td>wusen       <td>add rtc api
 * </table>
 *
 */

#include <stdio.h>
#include "rtc_api.h"
#include "wait_api.h"

#include "aiio_rtc.h"
#include "aiio_error.h"

static time_t seconds;

int32_t aiio_hal_rtc_init(void)
{
    rtc_init();
    return AIIO_OK;
}

int32_t aiio_hal_rtc_deinit(void)
{
    rtc_free();
    return AIIO_OK;
}

int32_t aiio_hal_rtc_get_time(uint64_t *time_stamp)
{
    *time_stamp = rtc_read();
    
    return AIIO_OK;
}

int32_t aiio_hal_rtc_set_time(uint64_t *time_stamp)
{
    rtc_write(*((time_t*)time_stamp));
    return AIIO_OK;
}