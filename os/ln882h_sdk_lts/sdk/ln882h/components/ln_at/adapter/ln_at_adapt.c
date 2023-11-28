/**
 * @file   ln_at_adapt.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-28     MurphyZhao   the first version
 */

#include "ln_at_adapt.h"
#include <stdint.h>
#include <string.h>

#include "osal/osal.h"

static OS_Semaphore_t g_at_sem;

void *ln_at_calloc(uint32_t num, uint32_t size)
{
    uint32_t _size = num * size;
    void * mem = OS_Malloc(_size);
    if (mem != NULL) {
        memset(mem, 0, _size);
    }
    return mem;
}

void  ln_at_free(void *ptr)
{
    OS_Free(ptr);
}

void *ln_at_lock_create(void)
{
    if (OS_SemaphoreCreate(&g_at_sem, 1, 1) != 0)
    {
        LN_AT_LOG_E("OS_SemaphoreCreate fail.\r\n");
        return NULL;
    }
    return &g_at_sem;
}

void ln_at_lock(void)
{
    if (OS_SemaphoreWait(&g_at_sem, OS_WAIT_FOREVER) != 0)
    {
        LN_AT_LOG_E("OS_SemaphoreWait fail.\r\n");
    }
}

void ln_at_unlock(void)
{
    if (OS_SemaphoreRelease(&g_at_sem) != 0)
    {
        LN_AT_LOG_E("OS_SemaphoreRelease fail.\r\n");
    }
}

void ln_at_lock_delete(void)
{
    OS_SemaphoreDelete(&g_at_sem);
}

void *ln_at_sem_create(uint32_t init_cnt, uint32_t total_cnt)
{
    OS_Semaphore_t *sem = LN_AT_CALLOC(1, sizeof(OS_Semaphore_t));
    if (!sem)
    {
        return NULL;
    }

    if(OS_SemaphoreCreate(sem, init_cnt, total_cnt) != 0)
    {
        LN_AT_LOG_E("OS_SemaphoreCreate fail.\r\n");
        return NULL;
    }

    return sem;
}

int ln_at_sem_delete(void *sem)
{
    OS_SemaphoreDelete(sem);
    LN_AT_FREE(sem);
    return 0;
}

/**
 * 0:success; others: failed
*/
int ln_at_sem_wait(void *sem, uint32_t timeout)
{
    return (int)OS_SemaphoreWait(sem, timeout);
}

int ln_at_sem_release(void *sem)
{
    OS_SemaphoreRelease(sem);
    return 0;
}

void *ln_at_timer_create(ln_at_timer_type_t type, void *cb, void *arg, uint32_t timeout)
{
    OS_TimerType timer_type = OS_TIMER_ONCE;
    OS_Timer_t *timer = LN_AT_CALLOC(1, sizeof(OS_Timer_t));
    if (!timer)
    {
        return NULL;
    }

    if (type == LN_AT_TIMER_PERIODIC)
    {
        timer_type = OS_TIMER_PERIODIC;
    }

    if (OS_OK != OS_TimerCreate(timer, timer_type, (OS_TimerCallback_t)cb, arg, timeout))
    {
        return NULL;
    }
    return timer;
}

int ln_at_timer_start(void *timer)
{
    if (OS_OK != OS_TimerStart((OS_Timer_t *)timer))
    {
        return 1;
    }

    return 0;
}

int ln_at_timer_update_period(void *timer, uint32_t timeout)
{
    if (OS_OK != OS_TimerChangePeriod((OS_Timer_t *)timer, timeout))
    {
        return 1;
    }

    return 0;
}

int ln_at_timer_stop(void *timer)
{
    if (OS_OK != OS_TimerStop((OS_Timer_t *)timer))
    {
        return 1;
    }

    return 0;
}

int ln_at_timer_delete(void *timer)
{
    if (OS_OK != OS_TimerDelete((OS_Timer_t *)timer))
    {
        return 1;
    }
    LN_AT_FREE(timer);

    return 0;
}

void *ln_at_task_create(const char *name, void *task_entry, void *args, uint8_t pri, uint32_t stack_size)
{
    OS_Thread_t *thr = LN_AT_CALLOC(1, sizeof(OS_Thread_t));
    if (!thr)
    {
        return NULL;
    }

    if(OS_ThreadCreate(thr, name, (OS_ThreadEntry_t)task_entry, args, (OS_Priority)pri, stack_size) != 0)
    {
        LN_AT_LOG_E("OS_ThreadCreate at_task fail.\r\n");
        return NULL;
    }
    return thr;
}

int ln_at_task_delete(void *task)
{
    if (OS_OK != OS_ThreadDelete(task))
    {
        return 1;
    }
    return 0;
}
