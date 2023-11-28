/**
 * @file   freertos_debug.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018 LightningSemi Technology Co., Ltd. All rights reserved.
 */

#ifndef _FREERTOS_DEBUG_H_
#define _FREERTOS_DEBUG_H_

#include <stdio.h>
#include "utils/debug/log.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS_DBG_ON       0
#define OS_WARN_ON      1
#define OS_ERR_ON       1
#define OS_ABORT_ON     0

#define OS_HANDLE_CHECK     1
#define OS_RESOURCE_TRACE   0 /* trace OS resource or not */

#if defined(__CC_ARM)
  	
	#define arch_fiq_enable()	    __enable_fiq()  /*Enable FIQs*/
	#define arch_fiq_disable()	    __disable_fiq() /*Disable FIQs*/
    #define arch_breakpoint(value)  __breakpoint(value)
#elif defined(__GNUC__)
	#define arch_fiq_enable()	    __asm volatile("cpsie f" : : : "memory", "cc")  /* Enable FIQs*/	
	#define arch_fiq_disable()	    __asm volatile("cpsid f" : : : "memory", "cc")/* Disable FIQs*/
    #define arch_breakpoint(value)  __asm volatile ("bkpt "#value)
#endif

#define sys_abort()         \
    do {                    \
        arch_fiq_disable(); \
        arch_breakpoint(0); \
    } while (0)

#define OS_SYSLOG(...)    \
    do {                          \
        log_printf(__VA_ARGS__);  \
    } while (0)
#define OS_ABORT()      sys_abort()
#define OS_PANIC()      exception_panic(__FILE__, __func__, __LINE__)

/* Define (sn)printf formatters for some types */
#define OS_BASETYPE_F   "ld"
#define OS_HANDLE_F     "p"
#define OS_TIME_F       "u"

#define OS_LOG(flags, ...)  \
    do {                            \
        if (flags)                  \
            OS_SYSLOG(__VA_ARGS__);  \
    } while (0)

#define OS_DBG(...)     OS_LOG(OS_DBG_ON, "[os] "__VA_ARGS__)
#define OS_WARN(...)    OS_LOG(OS_WARN_ON, "[os WARN] "__VA_ARGS__)
#define OS_ERR(...)                         \
    do {                                            \
        OS_LOG(OS_ERR_ON, "[os ERR] %s():%d, ",  \
               __func__, __LINE__, __VA_ARGS__);          \
        if (OS_ABORT_ON)                            \
            OS_ABORT();                             \
    } while (0)

#define OS_HANDLE_ASSERT(exp, handle)               \
    if (OS_HANDLE_CHECK && !(exp)) {                \
        OS_ERR("handle %"OS_HANDLE_F"\r\n", handle);\
        return OS_E_PARAM;                          \
    }

extern void exception_panic(const char *file, const char *func, const int line);

#ifdef __cplusplus
}
#endif

#endif /* _FREERTOS_DEBUG_H_ */
