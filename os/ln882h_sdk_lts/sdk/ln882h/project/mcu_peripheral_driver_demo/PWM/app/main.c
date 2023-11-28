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
#include "hal/hal_timer.h"
#include "hal/hal_clock.h"

#include "ln_drv_pwm.h"


int main (int argc, char* argv[])
{  
    uint32_t pwm_duty = 0;
    
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. 外设配置 ***********************/
    pwm_init(10000,20,PWM_CHA_0,GPIO_B,GPIO_PIN_5);     //初始化PWM,设置频率为10K,占空比为20%
    pwm_init(10000,20,PWM_CHA_1,GPIO_B,GPIO_PIN_6);     //初始化PWM
    pwm_init(10000,20,PWM_CHA_2,GPIO_B,GPIO_PIN_7);     //初始化PWM
    pwm_start(PWM_CHA_0);                               //开始产生PWM
    while(1)
    {
        pwm_duty ++;
        if(pwm_duty > 100) pwm_duty = 0;
        
        pwm_set_duty(pwm_duty,PWM_CHA_0);               //配置占空比
        pwm_set_duty(pwm_duty,PWM_CHA_1); 
        pwm_set_duty(pwm_duty,PWM_CHA_2); 
        
        LOG(LOG_LVL_INFO,"ln882H running! \n");
        
        LOG(LOG_LVL_INFO,"Duty = %f\n",pwm_get_duty(PWM_CHA_0));
        
        ln_delay_ms(100);
    }
}
