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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "qcloud_iot_export_error.h"
#include "qcloud_iot_import.h"

#include "utils/debug/log.h"

// TODO platform dependant

#ifdef WIFI_CONFIG_ENABLED
void *HAL_QueueCreate(unsigned long queue_length, unsigned long queue_item_size)
{
    QueueHandle_t queue_handle = NULL;
    queue_handle               = xQueueCreate(queue_length, queue_item_size);
    if (NULL == queue_handle) {
        return NULL;
    } else {
        return (void *)queue_handle;
    }
}

void HAL_QueueDestory(void *queue_handle)
{
    vQueueDelete(queue_handle);
}

int HAL_QueueReset(void *queue_handle)
{
    if (pdPASS == xQueueReset(queue_handle)) {
        return QCLOUD_RET_SUCCESS;
    } else {
        return QCLOUD_ERR_FAILURE;
    }
}

int HAL_QueueItemWaitingCount(void *queue_handle)
{
    return uxQueueMessagesWaiting(queue_handle);
}

int HAL_QueueItemPop(void *queue_handle, void *const item_buffer, uint32_t wait_timeout)
{
    if (pdPASS == xQueueReceive(queue_handle, item_buffer, wait_timeout)) {
        return QCLOUD_RET_SUCCESS;
    } else {
        return QCLOUD_ERR_FAILURE;
    }
}

int HAL_QueueItemPush(void *queue_handle, void *const item_buffer, uint32_t wait_timeout)
{
    if (pdPASS == xQueueSend(queue_handle, item_buffer, wait_timeout)) {
        return QCLOUD_RET_SUCCESS;
    } else {
        return QCLOUD_ERR_FAILURE;
    }
}
#endif

void HAL_SleepMs(_IN_ uint32_t ms)
{
    TickType_t ticks = ms / portTICK_PERIOD_MS;

    vTaskDelay(ticks ? ticks : 1); /* Minimum delay = 1 tick */

    return;
}

/**
 * @brief Print to LOG Serial Port.
 * 
 * @param fmt 
 * @param ... 
 */
void HAL_Printf(_IN_ const char *fmt, ...)
{
    #define TEMP_PRINT_BUF_SIZE                 (512)
    static char temp_print_buf[TEMP_PRINT_BUF_SIZE] = { 0 };
    va_list args;

    va_start(args, fmt);
    
    vsnprintf(temp_print_buf, TEMP_PRINT_BUF_SIZE, fmt, args);
    log_stdio_write(temp_print_buf, strlen(temp_print_buf));

    va_end(args);
}

int HAL_Snprintf(_IN_ char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int     rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);

    return rc;
}

int HAL_Vsnprintf(_IN_ char *str, _IN_ const int len, _IN_ const char *format, va_list ap)
{
    return vsnprintf(str, len, format, ap);
}

void *HAL_Malloc(_IN_ uint32_t size)
{
    return pvPortMalloc(size);
}

void HAL_Free(_IN_ void *ptr)
{
    if (ptr)
        vPortFree(ptr);
}

void *HAL_MutexCreate(void)
{
#ifdef MULTITHREAD_ENABLED
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    if (NULL == mutex) {
        HAL_Printf("%s: xSemaphoreCreateMutex failed\n", __FUNCTION__);
        return NULL;
    }

    return mutex;
#else
    return (void *)0xFFFFFFFF;
#endif
}

void HAL_MutexDestroy(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (xSemaphoreTake(mutex, 0) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
    }

    vSemaphoreDelete(mutex);
#else
    return;
#endif
}

void HAL_MutexLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return;
    }

    if (xSemaphoreTake(mutex, portMAX_DELAY) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

int HAL_MutexTryLock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return -1;
    }

    if (xSemaphoreTake(mutex, 0) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreTake failed\n", __FUNCTION__);
        return -1;
    }

    return 0;
#else
    return 0;
#endif
}

void HAL_MutexUnlock(_IN_ void *mutex)
{
#ifdef MULTITHREAD_ENABLED
    if (!mutex) {
        HAL_Printf("%s: invalid mutex\n", __FUNCTION__);
        return;
    }

    if (xSemaphoreGive(mutex) != pdTRUE) {
        HAL_Printf("%s: xSemaphoreGive failed\n", __FUNCTION__);
        return;
    }
#else
    return;
#endif
}

#if ((defined MULTITHREAD_ENABLED) || (defined WIFI_CONFIG_ENABLED))

// platform-dependant thread routine/entry function
static void _HAL_thread_func_wrapper_(void *ptr)
{
    ThreadParams *params = (ThreadParams *)ptr;

    params->thread_func(params->user_arg);

    vTaskDelete(NULL);
}

// platform-dependant thread create function
int HAL_ThreadCreate(ThreadParams *params)
{
    if (params == NULL)
        return QCLOUD_ERR_INVAL;

    if (params->thread_name == NULL) {
        HAL_Printf("thread name is required for FreeRTOS platform!\n");
        return QCLOUD_ERR_INVAL;
    }

    int ret = xTaskCreate(_HAL_thread_func_wrapper_, params->thread_name, params->stack_size, (void *)params,
                          params->priority, (void *)&params->thread_id);
    if (ret != pdPASS) {
        HAL_Printf("%s: xTaskCreate failed: %d\n", __FUNCTION__, ret);
        return QCLOUD_ERR_FAILURE;
    }

    return QCLOUD_RET_SUCCESS;
}

int HAL_ThreadDestroy(void *thread_t)
{
    vTaskDelete(thread_t);
    return 0;
}

#endif




void  *HAL_FileOpen(const char *filename, const char *mode)                {return NULL;};
size_t HAL_FileRead(void *ptr, size_t size, size_t nmemb, void *fp)        {return 0;   };
size_t HAL_FileWrite(const void *ptr, size_t size, size_t nmemb, void *fp) {return 0;   };
int    HAL_FileSeek(void *fp, long int offset, int whence)                 {return 0;   };
int    HAL_FileClose(void *fp)                                             {return 0;   };
int    HAL_FileRemove(const char *filename)                                {return 0;   };
int    HAL_FileRewind(void *fp)                                            {return 0;   };
int    HAL_FileRename(const char *old_filename, const char *new_filename)  {return 0;   };
int    HAL_FileEof(void *fp)                                               {return 0;   };
long   HAL_FileTell(void *fp)                                              {return 0;   };
long   HAL_FileSize(void *fp)                                              {return 0;   };
char  *HAL_FileGets(char *str, int n, void *fp)                            {return NULL;};
int    HAL_FileFlush(void *fp)                                             {return 0;   };
