#include "FreeRTOS.h"
#include "task.h"
#include "utils/debug/log.h"
#include "utils/debug/ln_assert.h"

#if( configUSE_MALLOC_FAILED_HOOK == 1 )
extern void vApplicationMallocFailedHook(void);
void vApplicationMallocFailedHook(void)
{
	/* vApplicationMallocFailedHook() will only be called if malloc failed */
    LOG(LOG_LVL_EMERG,"Malloc Failed\r\n");
    LN_ASSERT(0);
}
#endif

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
extern void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName );
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    ( void ) pcTaskName;
    ( void ) pxTask;

    /* vApplicationStackOverflowHook will only be called if stackoverflow */
    LOG(LOG_LVL_EMERG,"(%s)StackOverFlow\r\n", pcTaskName);
    LN_ASSERT(0);
}
#endif

