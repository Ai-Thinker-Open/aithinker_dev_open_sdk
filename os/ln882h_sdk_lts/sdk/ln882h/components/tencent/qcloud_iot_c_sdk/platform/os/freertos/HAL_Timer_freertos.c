/*
 * Tencent is pleased to support the open source community by making IoT Hub
 available.
 * Copyright (C) 2018-2020 THL A29 Limited, a Tencent company. All rights
 reserved.

 * Licensed under the MIT License (the "License"); you may not use this file
 except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software
 distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND,
 * either express or implied. See the License for the specific language
 governing permissions and
 * limitations under the License.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qcloud_iot_import.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t HAL_GetTimeMs(void)
{
    return (uint32_t)(xTaskGetTickCount() * (1000/configTICK_RATE_HZ));
}

/*Get timestamp*/
long HAL_Timer_current_sec(void)
{
    return HAL_GetTimeMs() / 1000;
}

char *HAL_Timer_current(char *time_str)
{
    long time_sec;
    time_sec = HAL_Timer_current_sec();
    memset(time_str, 0, TIME_FORMAT_STR_LEN);
    snprintf(time_str, TIME_FORMAT_STR_LEN, "%ld", time_sec);
    return time_str;
}

bool HAL_Timer_expired(Timer *timer)
{
    uint32_t now_ts = HAL_GetTimeMs();
    return (now_ts > timer->end_time) ? true : false;
}

void HAL_Timer_countdown_ms(Timer *timer, unsigned int timeout_ms)
{
    timer->end_time = HAL_GetTimeMs();
    timer->end_time += timeout_ms;
}

void HAL_Timer_countdown(Timer *timer, unsigned int timeout)
{
    timer->end_time = HAL_GetTimeMs();
    timer->end_time += timeout * 1000;
}

int HAL_Timer_remain(Timer *timer)
{
    return (int)(timer->end_time - HAL_GetTimeMs());
}

void HAL_Timer_init(Timer *timer)
{
    timer->end_time = 0;
}

#ifdef __cplusplus
}
#endif
