/**
 * @file    freertos_debug.c
 * @author  LightningSemi WLAN Team
 * Copyright (C) 2018 LightningSemi Technology Co., Ltd. All rights reserved.
 */

#include "./FreeRTOS_Adapter/freertos_debug.h"
#include "ln_utils.h"

#if OS_RESOURCE_TRACE

extern int g_xr_msgqueue_cnt;
extern int g_xr_mutex_cnt;
extern int g_xr_semaphore_cnt;
extern int g_xr_thread_cnt;
extern int g_xr_timer_cnt;

void xr_os_resource_info(void)
{
	OS_LOG(1, "<<< xr os resource info >>>\r\n");
	OS_LOG(1, "g_xr_msgqueue_cnt  %d\r\n", g_xr_msgqueue_cnt);
	OS_LOG(1, "g_xr_mutex_cnt     %d\r\n", g_xr_mutex_cnt);
	OS_LOG(1, "g_xr_semaphore_cnt %d\r\n", g_xr_semaphore_cnt);
	OS_LOG(1, "g_xr_thread_cnt    %d\r\n", g_xr_thread_cnt);
	OS_LOG(1, "g_xr_timer_cnt     %d\r\n", g_xr_timer_cnt);
}

#endif /* OS_RESOURCE_TRACE */
