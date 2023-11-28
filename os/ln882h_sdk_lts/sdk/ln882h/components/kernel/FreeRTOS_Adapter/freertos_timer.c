/**
 * @file freertos_timer.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018 LightningSemi Technology Co., Ltd. All rights reserved.
 */

#include "./FreeRTOS_Adapter/freertos_timer.h"
#include "./FreeRTOS_Adapter/freertos_time.h"
#include "./FreeRTOS_Adapter/freertos_debug.h"

/* TODO: what block time should be used ? */
#define OS_TIMER_WAIT_FOREVER	portMAX_DELAY
#define OS_TIMER_WAIT_NONE		0

#if OS_TIMER_USE_FREERTOS_ORIG_CALLBACK

static void OS_TimerPrivCallback(TimerHandle_t xTimer)
{
	OS_TimerCallbackData_t *priv;

	priv = pvTimerGetTimerID(xTimer);
	if (priv && priv->callback) 
	{
		priv->callback(priv->argument);
	} 
	else 
	{
		OS_WARN("Invalid timer callback\r\n");
	}
}

/**
 * @brief Create and initialize a timer object
 *
 * @note Creating a timer does not start the timer running. The OS_TimerStart()
 *       and OS_TimerChangePeriod() API functions can all be used to start the
 *       timer running.
 *
 * @param[in] timer Pointer to the timer object
 * @param[in] type Timer type
 * @param[in] cb Timer expire callback function
 * @param[in] arg Argument of Timer expire callback function
 * @param[in] periodMS Timer period in milliseconds
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerCreate(OS_Timer_t *timer, OS_TimerType type,
                         OS_TimerCallback_t cb, void *arg, uint32_t periodMS)
{
	OS_TimerCallbackData_t *priv;

	OS_HANDLE_ASSERT(!OS_TimerIsValid(timer), timer->handle);

	priv = OS_Malloc(sizeof(OS_TimerCallbackData_t));
	if (priv == NULL) {
		return OS_E_NOMEM;
	}

	priv->callback = cb;
	priv->argument = arg;
	timer->handle = xTimerCreate("",
	                             OS_MSecsToTicks(periodMS),
	                             type == OS_TIMER_PERIODIC ? pdTRUE : pdFALSE,
	                             priv,
	                             OS_TimerPrivCallback);

	if (timer->handle == NULL) 
	{
		OS_ERR("err %"OS_HANDLE_F"\r\n", timer->handle);
		OS_Free(priv);
		return OS_FAIL;
	}
	timer->priv = priv;
	return OS_OK;
}

#else /* OS_TIMER_USE_FREERTOS_ORIG_CALLBACK */

/**
 * @brief Create and initialize a timer object
 *
 * @note Creating a timer does not start the timer running. The OS_TimerStart()
 *       and OS_TimerChangePeriod() API functions can all be used to start the
 *       timer running.
 *
 * @param[in] timer Pointer to the timer object
 * @param[in] type Timer type
 * @param[in] cb Timer expire callback function
 * @param[in] arg Argument of timer expire callback function
 * @param[in] periodMS Timer period in milliseconds
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerCreate(OS_Timer_t *timer, OS_TimerType type,
                         OS_TimerCallback_t cb, void *arg, uint32_t periodMS)
{
	OS_HANDLE_ASSERT(!OS_TimerIsValid(timer), timer->handle);

	timer->handle = xTimerCreate("",
	                             OS_MSecsToTicks(periodMS),
	                             type == OS_TIMER_PERIODIC ? pdTRUE : pdFALSE,
	                             arg,
	                             cb);

	if (timer->handle == NULL) 
	{
		OS_ERR("err %"OS_HANDLE_F"\r\n", timer->handle);
		return OS_FAIL;
	}
	return OS_OK;
}

#endif /* OS_TIMER_USE_FREERTOS_ORIG_CALLBACK */

/**
 * @brief Delete the timer object
 * @param[in] timer Pointer to the timer object
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerDelete(OS_Timer_t *timer)
{
	BaseType_t ret;

	OS_HANDLE_ASSERT(OS_TimerIsValid(timer), timer->handle);

	ret = xTimerDelete(timer->handle, OS_TIMER_WAIT_FOREVER);
	if (ret != pdPASS) 
	{
		OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
		return OS_FAIL;
	}

	OS_TimerSetInvalid(timer);
#if OS_TIMER_USE_FREERTOS_ORIG_CALLBACK
	OS_Free(timer->priv);
	timer->priv = NULL;
#endif
	return OS_OK;
}

/**
 * @brief Start a timer running.
 * @note If the timer is already running, this function will re-start the timer.
 * @param[in] timer Pointer to the timer object
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerStart(OS_Timer_t *timer)
{
	BaseType_t ret;
	BaseType_t taskWoken;

	OS_HANDLE_ASSERT(OS_TimerIsValid(timer), timer->handle);

	if (OS_IsISRContext()) 
	{
		taskWoken = pdFALSE;
		ret = xTimerStartFromISR(timer->handle, &taskWoken);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
		portEND_SWITCHING_ISR(taskWoken);
	} 
	else 
	{
		ret = xTimerStart(timer->handle, 5);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
	}

	return OS_OK;
}

/**
 * @brief Change the period of a timer
 *
 * If OS_TimerChangePeriod() is used to change the period of a timer that is
 * already running, then the timer will use the new period value to recalculate
 * its expiry time. The recalculated expiry time will then be relative to when
 * OS_TimerChangePeriod() was called, and not relative to when the timer was
 * originally started.

 * If OS_TimerChangePeriod() is used to change the period of a timer that is
 * not already running, then the timer will use the new period value to
 * calculate an expiry time, and the timer will start running.
 *
 * @param[in] timer Pointer to the timer object
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerChangePeriod(OS_Timer_t *timer, uint32_t periodMS)
{
	BaseType_t ret;
	BaseType_t taskWoken;

	OS_HANDLE_ASSERT(OS_TimerIsValid(timer), timer->handle);

	if (OS_IsISRContext()) 
	{
		taskWoken = pdFALSE;
		ret = xTimerChangePeriodFromISR(timer->handle, periodMS, &taskWoken);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
		portEND_SWITCHING_ISR(taskWoken);
	} 
	else 
	{
		ret = xTimerChangePeriod(timer->handle, periodMS, 5);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
	}

	return OS_OK;
}

/**
 * @brief Stop a timer running.
 * @param[in] timer Pointer to the timer object
 * @retval OS_Status, OS_OK on success
 */
OS_Status OS_TimerStop(OS_Timer_t *timer)
{
	BaseType_t ret;
	BaseType_t taskWoken;

	OS_HANDLE_ASSERT(OS_TimerIsValid(timer), timer->handle);

	if (OS_IsISRContext()) 
	{
		taskWoken = pdFALSE;
		ret = xTimerStopFromISR(timer->handle, &taskWoken);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
		portEND_SWITCHING_ISR(taskWoken);
	} 
	else 
	{
		ret = xTimerStop(timer->handle, OS_TIMER_WAIT_FOREVER);
		if (ret != pdPASS) {
			OS_ERR("err %"OS_BASETYPE_F"\r\n", ret);
			return OS_FAIL;
		}
	}

	return OS_OK;
}
