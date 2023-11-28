/**
 * @file     main.c
 * @author   BSP Team 
 * @brief    
 * @version  0.0.0.1
 * @date     2021-08-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "hal/hal_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

#include "ln_drv_trng.h"

uint32_t data_buf[100];

int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. RTC 测试***********************/
    trng_init( 10000, data_buf, 100);

    for(int i = 0; i < 100; i ++)
    {
        LOG(LOG_LVL_INFO,"DATA[%d] =  %u \n",i,data_buf[i]);
    }

    while(1)
    {
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(1000);
    }
}
