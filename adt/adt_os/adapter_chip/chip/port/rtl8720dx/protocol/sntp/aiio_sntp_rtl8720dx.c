//中间件
#include"aiio_sntp.h"
#include "aiio_error.h"

//sdk
#include "FreeRTOS.h"
#include "task.h"
#include <platform/platform_stdlib.h>
#include <sntp/sntp.h>

int32_t aiio_hal_sntp_set_operating_mode(aiio_sntp_operating_mode_t operating_mode){
    //printf("\r\n\r\n\r\n[%s()-%d]\r\n\r\n\r\n",__func__,__LINE__);
    if(AIIO_SNTP_OPT_MODE_POLL != operating_mode){
        aiio_uart_log_send("sntp mode=%d not support\r\n",operating_mode);
        return AIIO_PARAM_ERROR;
    }
    sntp_init();
    return AIIO_OK;
}

int32_t aiio_hal_sntp_set_server_name(uint8_t idx, const char *server){
    printf("\r\n\r\n\r\n[%s()-%d]idx:%d server:%s\r\n\r\n\r\n",__func__,__LINE__,idx,server);
    return AIIO_FUNC_NO_DEFINE;
}

int32_t aiio_hal_sntp_get_last_time(uint32_t *seconds, uint32_t *frags){
    unsigned int update_tick = 0;
	long update_sec = 0, update_usec = 0;
	sntp_get_lasttime(&update_sec, &update_usec, &update_tick);
    printf("\r\n\r\n\r\n[%s()-%d]update_tick=%d\r\n\r\n\r\n",__func__,__LINE__,update_tick);
	if(update_tick) {
		long tick_diff_sec, tick_diff_ms, current_sec, current_usec;
		unsigned int current_tick = xTaskGetTickCount();

		tick_diff_sec = (current_tick - update_tick) / configTICK_RATE_HZ;
		tick_diff_ms = (current_tick - update_tick) % configTICK_RATE_HZ / portTICK_RATE_MS;
		update_sec += tick_diff_sec;
		update_usec += (tick_diff_ms * 1000);
		current_sec = update_sec + update_usec / 1000000;
		current_usec = update_usec % 1000000;
		//printf("\r\n\r\n\r\n%s + %d usec\r\n\r\n\r\n", ctime(&current_sec), current_usec);
        *seconds=current_sec;
        *frags=current_usec/1000;
        //printf("\r\n\r\n\r\n[%s()-%d]seconds=%d frags=%d\r\n\r\n\r\n",__func__,__LINE__,*seconds,*frags);
	}
    return AIIO_OK;
}




