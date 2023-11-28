#include "usr_ctrl.h"
#include <stddef.h>
#include "ln_kv_api.h"

#include "ln_at.h"
#include "ln_at_cmd/ln_at_cmd_version.h"
#include "ln_utils.h"

#include "osal/osal.h"
#include "utils/debug/ln_assert.h"
#include "task.h"

#if STATICTICS_EN

//util
static void str_split(char *_str, char *_split, char **_pp, int _cnt)
{
    int i = 0;
    _pp[0] = strtok(_str, _split);
    Log_i("TOKEN[%d] == %s", 0, _pp[0]);
    for (i = 1; i < _cnt; i++)
    {
        _pp[i] = strtok(NULL, _split);
        if (_pp[i] == NULL)
            return;

        Log_i("TOKEN[%d] == %s", i, _pp[i]);
    }
}

//
static OS_Semaphore_t g_get_static;
static ln_at_err_t ln_at_task_list_get(const char *name)
{
    OS_SemaphoreRelease(&g_get_static);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(TASKLIST, ln_at_task_list_get, NULL, NULL, NULL);

static OS_Thread_t g_task_static_thread;
#define TASK_STATIC_STACK_SIZE 4 * 256 //Byte
volatile unsigned long ulHightFreqTimerTicks;
static void task_static(void *param)
{
    char *CPU_RunInfo = NULL;//保存任务运行时间信息
    char *pTask[32];
    int taskCnt = 0;
    int taskMaxId = 0;
    char id_buf[3] = {0};
    if (OS_SemaphoreCreate(&g_get_static, 0, 1024) != OS_OK)
    {
        Log_e("OS_SemaphoreCreate sem fail.");
        OS_ThreadDelete(&g_task_static_thread);
        return;
    }

    CPU_RunInfo = OS_Malloc(512);
    while (1)
    {
        memset(pTask, NULL, sizeof(char *) * 32);
        if (OS_OK != OS_SemaphoreWait(&g_get_static, OS_WAIT_FOREVER))
        {
            continue;
        }

        memset(CPU_RunInfo, 0, 512); 
        vTaskList((char *)CPU_RunInfo); 

        str_split(CPU_RunInfo, "\r\n", pTask, 32);
        for (size_t i = 0; i < 32; i++)
        {
            if (pTask[i] == NULL)
            {
                taskCnt = i;
                break;
            }
        }

        for (size_t i = 0; i < taskCnt; i++)
        {
            memset(id_buf, 0, 3);
            id_buf[0] = pTask[i][strlen(pTask[i]) - 2];
            id_buf[1] = pTask[i][strlen(pTask[i]) - 1];

            if (atoi(id_buf) > taskMaxId)
            {
                taskMaxId = atoi(id_buf);
            }
        }

        Log_i("taskCnt=%d    taskMaxId=%d", taskCnt, taskMaxId);
        ln_at_printf("---------------------------------------------\r\n");
        ln_at_printf("TASK\t\tSTAT\tPRI\tSTACK\tID\r\n");
        ln_at_printf("---------------------------------------------\r\n");
        for (size_t i = 1; i <= taskMaxId; i++)
        {
            for (size_t j = 0; j < taskCnt; j++)
            {
                memset(id_buf, 0, 3);
                id_buf[0] = pTask[j][strlen(pTask[j]) - 2];
                id_buf[1] = pTask[j][strlen(pTask[j]) - 1];
                //Log_i("current id: %s", id_buf);
                if (atoi(id_buf) == i)
                {
                    ln_at_printf("%s\r\n", pTask[j]);
                    break;
                }
            }
        }
        ln_at_printf("---------------------------------------------\r\n");

        // memset(CPU_RunInfo, 0, 400); //信息缓冲区清零
        // vTaskGetRunTimeStats((char *)&CPU_RunInfo);
        // at_printf("TASK  STATICS   USED\r\n");
        // at_printf("%s", CPU_RunInfo);
        // at_printf("---------------------------------------------\r\n\n");
        vTaskDelay(1000); /* 延时 500 个 tick */
    }
}

void create_statistics_list_task()
{
    if (OS_OK != OS_ThreadCreate(&g_task_static_thread, "task_static_thread", task_static, NULL, OS_PRIORITY_BELOW_NORMAL, TASK_STATIC_STACK_SIZE))
    {
        LN_ASSERT(1);
    }
}
#endif

#include "ln_at_sntp.h"
#include "transparent.h"
#include "ln_at_mqtt.h"
#include "ln_at_http.h"
#include "ln_at_cmd_base_plus.h"
#include "wifi_driver_port.h"
#include "sys_driver_port.h"

void at_cmd_plus_entry(void *params)
{
    ln_get_wifi_base_plus_handle()->init();
    ln_get_tcpip_handle()->_init();
    ln_get_sntp_handle()->init();
    ln_get_mqtt_handle()->init();
    ln_get_http_handle()->init();
    at_wifi_adapter_get_handle()->at_func.adapter_init_func();
    at_sys_adapter_get_handle()->sys_func.adapter_init_func();

    ln_get_wifi_drv_handle()->init();
}

void create_at_cmd_plus_task()
{
	at_cmd_plus_entry(NULL);
}

//tmr
void ln_start_tmr(OS_Timer_t *_tmr, OS_TimerType type, OS_TimerCallback_t cb, void *arg, uint32_t periodMS)
{
	if (OS_OK != OS_TimerCreate(_tmr, type, cb, arg, periodMS))
	{
		Log_e("OS_TimerCreate fail!");
		while(1);
	}
	
	if (OS_OK != OS_TimerStart(_tmr))
	{
		Log_e("OS_TimerStart fail!");
		while(1);
	}
}
