#include <time.h>
//中间件
#include"aiio_sntp.h"
#include "aiio_error.h"

//sdk
#include "FreeRTOS.h"
#include "task.h"
#include <platform_stdlib.h>
#include <sntp.h>
#include <netdb.h>      // gethostbyname, struct hostent
#include <string.h>     // memset, 可选

int32_t aiio_hal_sntp_set_operating_mode(aiio_sntp_operating_mode_t operating_mode){
    //aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]\r\n\r\n\r\n",__func__,__LINE__);
    if(AIIO_SNTP_OPT_MODE_POLL != operating_mode){
        aiio_uart_log_send("sntp mode=%d not support\r\n",operating_mode);
        return AIIO_PARAM_ERROR;
    }
    sntp_init();
    return AIIO_OK;
}

int32_t aiio_hal_sntp_set_server_name(uint8_t idx, const char *server){
    aiio_uart_log_send("\r\n\r\n\r\n[%s()-%d]idx:%d server:%s\r\n\r\n\r\n",__func__,__LINE__,idx,server);
    sntp_setservername(idx, server);
    return AIIO_OK;
}

int32_t aiio_hal_sntp_get_last_time(uint32_t *seconds, uint32_t *frags){
    time_t now;
	time(&now);
    *seconds=now;
    *frags=0;
    return AIIO_OK;
}




