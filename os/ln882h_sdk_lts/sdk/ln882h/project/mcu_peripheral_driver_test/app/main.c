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
#include "hal/hal_gpio.h"
#include "ln_test_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"


#include "hal_adc.h"
#include "hal_timer.h"
#include "hal_clock.h"
#include "hal_adv_timer.h"


int main (int argc, char* argv[])
{  
    /* 1. 系统初始化 */
    SetSysClock();
    
    log_init();
    
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    
    ln_show_reg_init();
    
    while(1)
    {
        LOG(LOG_LVL_INFO,"ln882H running! \n");
        ln_delay_ms(100);
    }
}
