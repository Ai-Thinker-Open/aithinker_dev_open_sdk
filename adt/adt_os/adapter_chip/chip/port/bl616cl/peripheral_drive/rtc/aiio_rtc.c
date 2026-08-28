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
#include "bflb_rtc.h"

#include "aiio_rtc.h"
#include "aiio_error.h"
#include "aiio_log.h"

static struct bflb_device_s *rtc;
static uint64_t saved_timestamp=0;

int32_t aiio_hal_rtc_init(void)
{
    rtc = bflb_device_get_by_name("rtc");
    bflb_rtc_set_time(rtc,0);
    return AIIO_OK;
}

int32_t aiio_hal_rtc_deinit(void)
{
    bflb_rtc_disable(rtc);
    return AIIO_OK;
}

int32_t aiio_hal_rtc_get_time(uint64_t *time_stamp)
{
    *time_stamp = saved_timestamp + BFLB_RTC_TIME2SEC(bflb_rtc_get_time(rtc));
    
    return AIIO_OK;
}

int32_t aiio_hal_rtc_set_time(uint64_t *time_stamp)
{
    saved_timestamp = *time_stamp;
    return AIIO_OK;
}