/**
 * @file   ln_at_adapt.h
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#ifndef __LN_AT_ADAPT_H__
#define __LN_AT_ADAPT_H__

#include "ln_at_cfg.h"

#include <stdint.h>
#include <stdlib.h>

#include "utils/debug/log.h"
#include "utils/wrap_stdio.h"

#ifdef LN_AT_DEBUG_ENABLE
  /* Debug */
  #define LN_AT_LOG_D(...)    //LOG(LOG_LVL_DEBUG, ##__VA_ARGS__)
  #define LN_AT_LOG_I(...)    LOG(LOG_LVL_INFO,  ##__VA_ARGS__)
  #define LN_AT_LOG_E(...)    LOG(LOG_LVL_ERROR, ##__VA_ARGS__)
  #define LN_AT_LOG_HEX(string, buff, size)
#else
  #define LN_AT_LOG_D(...)
  #define LN_AT_LOG_I(...)
  #define LN_AT_LOG_E(...)
  #define LN_AT_LOG_HEX(string, buff, size)
#endif /* LN_AT_DEBUG_ENABLE */

#if defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #define __CLANG_ARM
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)  /* ARM Compiler */
  #define LN_AT_INLINE                   static __inline
  #define LN_AT_UNUSED                   __attribute__((unused))
  #define LN_AT_USED                     __attribute__((used))
  #define LN_AT_SECTION(x)               __attribute__((section(x)))
#elif defined (__IAR_SYSTEMS_ICC__)      /* for IAR Compiler */
  #define LN_AT_INLINE                   static inline
  #define LN_AT_UNUSED
  #define LN_AT_USED                     __root
  #define LN_AT_SECTION(x)               @ x
#elif defined (__GNUC__)                /* GNU GCC Compiler */
  #define LN_AT_INLINE                   static __inline
  #define LN_AT_UNUSED                   __attribute__((unused))
  #define LN_AT_USED                     __attribute__((used))
  #define LN_AT_SECTION(x)               __attribute__((section(x)))
#else
  #error not supported tool chain
#endif

typedef enum
{
    LN_AT_TIMER_ONCE     = 0,
    LN_AT_TIMER_PERIODIC = 1
} ln_at_timer_type_t;

/* mem */
void *ln_at_calloc(uint32_t num, uint32_t size);
void  ln_at_free(void *ptr);
#define LN_AT_MEMSET memset
#define LN_AT_CALLOC ln_at_calloc
#define LN_AT_FREE   ln_at_free

/* OS */
void *ln_at_task_create(const char *name, void *task_entry, void *args, uint8_t pri, uint32_t stack_size);
int   ln_at_task_delete(void *task);
void *ln_at_lock_create(void);
void  ln_at_lock(void);
void  ln_at_unlock(void);
void  ln_at_lock_delete(void);

void *ln_at_sem_create(uint32_t init_cnt, uint32_t total_cnt);
int   ln_at_sem_delete(void *sem);
int   ln_at_sem_wait(void *sem, uint32_t timeout);
int   ln_at_sem_release(void *sem);

void *ln_at_timer_create(ln_at_timer_type_t type, void *cb, void *arg, uint32_t timeout);
int   ln_at_timer_start(void *timer);
int   ln_at_timer_update_period(void *timer, uint32_t timeout);
int   ln_at_timer_stop(void *timer);
int   ln_at_timer_delete(void *timer);

#endif /* __LN_AT_ADAPT_H__ */
