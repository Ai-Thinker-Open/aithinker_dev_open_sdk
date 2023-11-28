/**
 * @file   osal.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018 LightningSemi Technology Co., Ltd. All rights reserved.
 */

#ifndef __OSAL_H__
#define __OSAL_H__
#include "proj_config.h"

#if (defined(__CONFIG_OS_KERNEL))
  #include "./FreeRTOS_Adapter/freertos_common.h"
  #include "./FreeRTOS_Adapter/freertos_cpuusage.h"
  #include "./FreeRTOS_Adapter/freertos_errno.h"
  #include "./FreeRTOS_Adapter/freertos_mutex.h"
  #include "./FreeRTOS_Adapter/freertos_queue.h"
  #include "./FreeRTOS_Adapter/freertos_semaphore.h"
  #include "./FreeRTOS_Adapter/freertos_thread.h"
  #include "./FreeRTOS_Adapter/freertos_time.h"
  #include "./FreeRTOS_Adapter/freertos_timer.h"
  #include "./FreeRTOS_Adapter/freertos_debug.h"
#else 
  #error "No OS Kernel defined!"
#endif

#endif /* __OSAL_H__ */
