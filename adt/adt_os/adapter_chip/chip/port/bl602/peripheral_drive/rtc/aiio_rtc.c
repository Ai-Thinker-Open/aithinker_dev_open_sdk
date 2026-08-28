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
#include <hosal_rtc.h>

#include "aiio_rtc.h"
#include "aiio_error.h"

static hosal_rtc_dev_t rtc;

int32_t aiio_hal_rtc_init(void)
{
    rtc.port = 0;
    rtc.config.format = 0; 
    return hosal_rtc_init(&rtc);
}

int32_t aiio_hal_rtc_deinit(void)
{
    return hosal_rtc_finalize(&rtc);
}

int32_t aiio_hal_rtc_get_time(uint64_t *time_stamp)
{
    return hosal_rtc_get_count(&rtc,time_stamp);
}

int32_t aiio_hal_rtc_set_time(uint64_t *time_stamp)
{
    aiio_hal_rtc_deinit();
    aiio_hal_rtc_init();
    return hosal_rtc_set_count(&rtc,time_stamp);
}