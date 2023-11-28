#include "clock.h"

#include "FreeRTOS.h"
#include "task.h"

uint32_t Clock_GetTimeMs( void )
{
    return ( uint32_t )xTaskGetTickCount() * 1000 / configTICK_RATE_HZ;;
}

void Clock_SleepMs( uint32_t sleepTimeMs )
{
    vTaskDelay( sleepTimeMs/portTICK_PERIOD_MS );
}
