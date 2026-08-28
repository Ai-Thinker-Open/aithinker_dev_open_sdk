#include "stdio.h"
#include "string.h"
#include "aiio_error.h"
#include "aiio_os_port.h"

//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"

typedef long             BaseType_t;
#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )
#define portBASE_TYPE	long
#define AIIO_OS_TIMER_OPT_BLOCK_MS_MAX		10*1000
#define AIIO_OS_WAIT_FOREVER				(0xFFFFFFFF)
#define AIIO_OS_NO_WAIT 					(0)
/*#define AIIO_OS_WAIT_MS2TICK(ms)			\
	( ((ms) == AIIO_OS_WAIT_FOREVER) ? AIIO_OS_WAIT_FOREVER : (((ms)/portTICK_PERIOD_MS)+((((ms)%portTICK_PERIOD_MS)+portTICK_PERIOD_MS-1)/portTICK_PERIOD_MS)) )*/


int32_t aiio_os_enter_critical(void)
{
    rtos_critical_enter(RTOS_CRITICAL_DEFAULT);
    return AIIO_OK;
}

int32_t aiio_os_exit_critical(void)
{
    rtos_critical_exit(RTOS_CRITICAL_DEFAULT);
    return AIIO_OK;
}

int32_t aiio_os_is_inisr(void)
{
    return rtos_critical_is_in_interrupt();
}

int32_t aiio_os_tick_dealy(uint32_t tick)
{
    rtos_time_delay_ms(tick);
    return AIIO_OK;
}

int32_t aiio_os_tick_count(uint32_t *tick)
{
    if (tick == NULL)
    {
        return AIIO_ERROR;
    }

    *tick = rtos_time_get_current_system_time_ms(); //不区分是否在中断中

    return AIIO_OK;
}

int32_t aiio_os_thread_self(aiio_os_thread_handle_t *pthread)
{
    *pthread = rtos_task_handle_get();

    if (*pthread)
    {
        return AIIO_OK;
    }
    else
    {
        return AIIO_ERROR;
    }
}

int32_t aiio_os_thread_self_name(char *pthread_name)
{
    if (pthread_name == NULL)
    {
        return AIIO_ERROR;
    }

    memcpy(pthread_name, pcTaskGetName(rtos_task_handle_get()), strlen(pcTaskGetName(rtos_task_handle_get())));
    return AIIO_OK;
}

int32_t aiio_os_mutex_create(aiio_os_mutex_handle_t *phandle)
{
    rtos_mutex_create(phandle);

    if (*phandle)
    {
        return AIIO_OK;
    }
    else
    {
        return AIIO_ERROR;
    }
}

int32_t aiio_os_mutex_delete(aiio_os_mutex_handle_t handle)
{
    rtos_mutex_delete(handle);
    return AIIO_OK;
}

int32_t aiio_os_mutex_put(aiio_os_mutex_handle_t handle)
{
    int ret = rtos_mutex_give(handle);

    return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_mutex_get(aiio_os_mutex_handle_t handle, uint32_t wait_ms)
{
    int ret = rtos_mutex_recursive_take(handle, wait_ms);

    return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_thread_create(aiio_os_thread_handle_t *pthread, const char *name, aiio_os_function_return_t (*function)(void *arg), uint32_t stack_size, void *arg, int priority)
{
    if (0 == rtos_task_create(pthread,name, function,
					 arg, stack_size,priority) )
    {
        return AIIO_OK;
    }

    return AIIO_ERROR;
}

int32_t aiio_os_thread_delete(aiio_os_thread_handle_t thread)
{
    rtos_task_delete(thread);
    return AIIO_OK;
}

int32_t aiio_os_thread_start_scheduler()
{
    if (rtos_sched_get_state() != RTOS_SCHED_RUNNING)
    {
        rtos_sched_start();
    }

    return AIIO_OK;
}

uint32_t aiio_os_ms2tick(uint32_t ms)
{
    return ms;
}

uint32_t aiio_os_tick2ms(uint32_t tick)
{
    return tick;
}

uint32_t aiio_os_get_free_heap_size()
{
    return rtos_mem_get_free_heap_size();
}

uint32_t aiio_os_get_minimum_ever_free_heap_size()
{
    return rtos_mem_get_minimum_ever_free_heap_size();
}

int32_t aiio_os_queue_create(aiio_os_queue_handle_t *pqhandle, uint32_t q_len, uint32_t item_size)
{
    rtos_queue_create(pqhandle,q_len,item_size);
    if (*pqhandle)
    {
        return AIIO_OK;
    }
    else
    {
        return AIIO_ERROR;
    }
}

int32_t aiio_os_queue_recv(aiio_os_queue_handle_t qhandle, void *msg, uint32_t wait_ms)
{
    int ret = rtos_queue_receive(qhandle,msg,wait_ms);

    return RTK_SUCCESS==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_queue_delete(aiio_os_queue_handle_t qhandle)
{
    rtos_queue_delete(qhandle);
    return AIIO_OK;
}

// int32_t aiio_os_queue_send(aiio_os_queue_handle_t phandle, const void *msg, uint32_t wait_ms)
// {
//     int ret = rtos_queue_send(phandle,msg,wait_ms);

//     return TRUE==ret ? AIIO_OK : AIIO_ERROR;
// }

int32_t aiio_os_queue_send(aiio_os_queue_handle_t qhandle, const void *msg, uint32_t wait_ms)
{
    int ret;

    if (aiio_os_is_inisr())
    {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        ret = xQueueSendToBackFromISR(qhandle, msg, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        ret = xQueueSendToBack(qhandle, msg, aiio_os_ms2tick(wait_ms));
    }

    return ret == pdTRUE ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_queue_get_msgs_waiting(aiio_os_queue_handle_t qhandle)
{
    return rtos_queue_massage_waiting(qhandle);
}

int32_t aiio_os_semaphore_create_counting(aiio_os_semaphore_handle_t* pmhandle, uint32_t maxcount, uint32_t initcount)
{
    rtos_sema_create(pmhandle,initcount,maxcount);
	if (*pmhandle) {
		return AIIO_OK;
	}
	else {
		return AIIO_ERROR;
	}
}

int32_t aiio_os_semaphore_get(aiio_os_semaphore_handle_t mhandle, uint32_t wait_ms)
{
	int ret = rtos_sema_take(mhandle,wait_ms);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_semaphore_put(aiio_os_semaphore_handle_t mhandle)
{
	int ret = rtos_sema_give(mhandle);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_semaphore_delete(aiio_os_semaphore_handle_t mhandle)
{
	rtos_sema_delete(mhandle);
	return AIIO_OK;
}

int32_t aiio_os_event_create(aiio_os_event_handle_t *event)
{
    *event = xEventGroupCreate();

    if (*event)
    {
        return AIIO_OK;
    }
    else
    {
        return AIIO_ERROR;
    }
}

int32_t aiio_os_event_set_bit(aiio_os_event_handle_t event, const int32_t bits_to_set)
{
    int ret;

    if (aiio_os_is_inisr())
    {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        ret = xEventGroupSetBitsFromISR(event, bits_to_set, &xHigherPriorityTaskWoken);
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
    else
    {
        ret = xEventGroupSetBits(event, bits_to_set);
    }

    return pdTRUE==ret ? AIIO_OK : AIIO_ERROR;
}

int32_t aiio_os_event_wait_bits(aiio_os_event_handle_t event, const uint32_t bits_to_wait_for, const uint32_t clear_on_exit, const uint32_t wait_for_all_bit, uint32_t wait_ms)
{
    return xEventGroupWaitBits(event, bits_to_wait_for, clear_on_exit, wait_for_all_bit, wait_ms);
}

int32_t aiio_os_event_clear_bits(aiio_os_event_handle_t event, const uint32_t bits_to_clear)
{
    return xEventGroupClearBits(event, bits_to_clear);
}

int32_t aiio_os_event_clear_bits_from_isr(aiio_os_event_handle_t event, const uint32_t bits_to_clear)
{
    return xEventGroupClearBitsFromISR(event, bits_to_clear);
}

int32_t aiio_os_event_get_bits(aiio_os_event_handle_t event)
{
    return xEventGroupGetBits(event);
}

int32_t aiio_os_event_get_bits_from_isr(aiio_os_event_handle_t event)
{
    return xEventGroupGetBitsFromISR(event);
}

int32_t aiio_os_event_delete(aiio_os_event_handle_t event)
{
    vEventGroupDelete(event);

    return AIIO_OK;
}

int32_t aiio_os_timer_create(aiio_os_timer_handle_t* ptimer, const char *name, uint32_t ms,
		    void (*call_back)(aiio_os_timer_handle_t), void *context,
		    aiio_os_timer_reload_t reload,
		    aiio_os_timer_activate_t activate)
{
	int auto_reload = (reload == AIIO_OS_TIMER_ONE_SHOT) ? 0 : 1;

    rtos_timer_create(ptimer,name,NULL,ms,auto_reload,call_back);
	if (*ptimer == NULL)
		return AIIO_ERROR;

	if (activate == AIIO_OS_TIMER_AUTO_ACTIVATE){
		if(TRUE != rtos_timer_start(*ptimer,ms)){
			rtos_timer_stop(*ptimer,ms);
			*ptimer = NULL;
			return AIIO_ERROR;
		}
	}

	return AIIO_OK;
}

int32_t aiio_os_timer_activate(aiio_os_timer_handle_t timer)
{
	int ret =  rtos_timer_start(timer,AIIO_OS_TIMER_OPT_BLOCK_MS_MAX);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}


int32_t aiio_os_timer_change(aiio_os_timer_handle_t timer, uint32_t ms)
{
	int ret = rtos_timer_change_period(timer,ms,AIIO_OS_TIMER_OPT_BLOCK_MS_MAX);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}


int32_t aiio_os_timer_is_active(aiio_os_timer_handle_t timer)
{
	int ret = rtos_timer_is_timer_active(timer);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

void *aiio_os_timer_get_context(aiio_os_timer_handle_t timer)
{
	return (void*)rtos_timer_get_id(timer);
}

int32_t aiio_os_timer_reset(aiio_os_timer_handle_t timer)
{ 
	int32_t ret = aiio_os_timer_deactivate(timer);
    if(AIIO_OK!=ret){
        return AIIO_ERROR;
    }

	ret = aiio_os_timer_activate(timer);
    if(AIIO_OK!=ret){
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

int32_t aiio_os_timer_deactivate(aiio_os_timer_handle_t timer)
{
	int ret = rtos_timer_stop(timer,AIIO_OS_TIMER_OPT_BLOCK_MS_MAX);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}


int32_t aiio_os_timer_delete(aiio_os_timer_handle_t timer)
{
	int ret = rtos_timer_delete(timer, AIIO_OS_TIMER_OPT_BLOCK_MS_MAX);

	return TRUE==ret ? AIIO_OK : AIIO_ERROR;
}

void *aiio_os_malloc(size_t size)
{
    return rtos_mem_zmalloc(size);
}

void aiio_os_free(void *prv)
{
    if (prv)
    {
        rtos_mem_free(prv);
    }
}
