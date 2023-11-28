
#include <stdlib.h>             // standard lib functions
#include <stdint.h>             // standard integer definition

#include "rwip_config.h"        // RW SW configuration
#include "rwip.h"               // RW SW initialization
#include "ke_event.h"
#include "ke_msg.h"

#include "ble_arch/arch.h"      // architectural platform definitions

//LN 
#include "utils/debug/log.h"
#include "osal/osal.h"          // For rtos thread, semphore


static OS_Thread_t    g_rw_thread = {NULL};
static OS_Semaphore_t g_rw_sch_trigger_sem = {NULL};
static OS_Semaphore_t g_ke_msg_sync_sem = {NULL};


static int  rwip_sch_trigger_wait(void);
static void rwip_sch_trigger(void);
static void ke_msg_sync_lock_init(void);
static void ke_msg_sync_lock_deinit(void);
static void ke_msg_sync_lock_acquire(void);
static void rw_main(void *params);


/**
 * ke event message sync lock initialization
*/
static void ke_msg_sync_lock_init(void)
{
    if (OS_SemaphoreCreate(&g_ke_msg_sync_sem, 1, 1) != 0)
    {
        BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error\r\n", __func__);
    }
}

static void ke_msg_sync_lock_deinit(void)
{
    if(OS_SemaphoreDelete(&g_ke_msg_sync_sem) != 0)
    {
        BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error\r\n", __func__);
    }
    g_ke_msg_sync_sem.handle = NULL;
}

static void ke_msg_sync_lock_acquire(void)
{
    // if (OS_SemaphoreWait(&g_ke_msg_sync_sem, OS_WAIT_FOREVER) != 0)
    // {
    //     BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error\r\n", __func__);
    // }
}

void ke_msg_sync_lock_release(void)
{
    // if (OS_SemaphoreRelease(&g_ke_msg_sync_sem) != 0)
    // {
    //     BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error\r\n", __func__);
    // }
}

void ln_ke_msg_send(void const *param_ptr)
{
    // ke_msg_sync_lock_acquire();
    // Send the message
    ke_msg_send(param_ptr);
}


static int rwip_sch_trigger_wait(void)
{
    return OS_SemaphoreWait(&g_rw_sch_trigger_sem, OS_WAIT_FOREVER);
}

static void rwip_sch_trigger(void)
{
    OS_SemaphoreRelease(&g_rw_sch_trigger_sem);
}

static void rw_main(void *params)
{
    LOG(LOG_LVL_DEBUG, "rw_main loop start!!\r\n");
    while(1)
    {
        // schedule all pending events
        if(OS_OK == rwip_sch_trigger_wait())
        {
            rwip_schedule();
        }
    }
}

void rw_init(uint8_t mac[6])
{
#define RW_TASK_STACK_SIZE         8*256

    OS_Status ret = OS_OK;

    ke_msg_sync_lock_init();

    ret = OS_SemaphoreCreateBinary(&g_rw_sch_trigger_sem);
    if (ret != OS_OK) {
        BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error! ret:%d\r\n", __func__, ret);
        return;
    }

    ke_event_notify_cb_set(rwip_sch_trigger);

    // 1. set mac address
    ln_ble_mac_set((const uint8_t *)mac);
    BLIB_LOG(BLIB_LOG_LVL_D, "mac %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // 2. RW SW stack initialization
    rwip_init(RESET_NO_ERROR);

    ret = OS_ThreadCreate(&g_rw_thread, "rw_task", rw_main, NULL,
            OS_PRIORITY_REAL_TIME, RW_TASK_STACK_SIZE);
    if(ret != OS_OK) {
        BLIB_LOG(BLIB_LOG_LVL_E, "[%s] Error! ret:%d\r\n", __func__, ret);
        return;
    }
}

void rw_deinit(void)
{
}
