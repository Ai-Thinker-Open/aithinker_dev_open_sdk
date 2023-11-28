/**
 * @file     ln_ext_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-30
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
/*
        外部中断测试说明：
        
            1. 外部中断的引脚是芯片规定的，对应的引脚请参考 LN882H_pinlist.
            
                PA0  -> EXT_INT[0]
                PA1  -> EXT_INT[1]
                PA2  -> EXT_INT[2]
                PA3  -> EXT_INT[3]
                PA5  -> EXT_INT[4]
                PA6  -> EXT_INT[5]
                PA7  -> EXT_INT[6]
                PB9  -> EXT_INT[7]

*/


#include "hal_common.h"
#include "hal_ext.h"
#include "hal_gpio.h"
#include "ln_test_common.h"
#include "ln_ext_test.h"
#include "utils/debug/log.h"

void ln_ext_test()
{
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_0,GPIO_PULL_DOWN);    //配置下拉
    
    hal_ext_init(EXT_INT_SENSE_0,EXT_INT_POSEDEG,HAL_ENABLE);   //使能外部中断引脚,外部引脚为PA0
    
    NVIC_SetPriority(EXT_IRQn,1);                               //配置中断优先级
    NVIC_EnableIRQ(EXT_IRQn);  
    
    while(1)
    {
        ln_delay_ms(500);
    }
}

void EXT_IRQHandler()
{
    if(hal_ext_get_it_flag(EXT_INT_SENSE_0_IT_FLAG) == HAL_SET)
    {
        hal_ext_clr_it_flag(EXT_INT_SENSE_0_IT_FLAG);
        
        LOG(LOG_LVL_INFO,"KEY_WAKEUP has pressed! \r\n");
    }
}

