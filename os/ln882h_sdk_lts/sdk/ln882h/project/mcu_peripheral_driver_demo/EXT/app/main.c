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

#include "ln_drv_ext.h"


void ext_it_callback(void);

int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. EXT 测试***********************/
    ext_init(EXT_INT_SENSE_6,EXT_INT_NEGEDGE,ext_it_callback);

    while(1)
    {
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(1000);
    }
}


void ext_it_callback(void)
{
    LOG(LOG_LVL_INFO,"GO IN EXT IT! \n");
}
