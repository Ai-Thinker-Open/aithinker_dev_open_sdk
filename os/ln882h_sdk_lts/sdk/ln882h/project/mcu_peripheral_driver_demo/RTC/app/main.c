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
#include "hal/hal_gpio.h"

#include "ln_drv_rtc.h"

void rtc_callback(void);
uint32_t running_time = 0;

int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. RTC 测试,可以通过示波器测量PB5引脚来确定RTC精度***********************/
    //设置RTC定时为1s
    rtc_init(1000,rtc_callback);
    
    //设置示波器测试引脚
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    while(1)
    {
        LOG(LOG_LVL_INFO,"running time = %d:%d:%d:%d \n",running_time / 3600,running_time % 3600 / 60,running_time % 60,rtc_get_rtc_current_cnt_time());
        ln_delay_ms(1000);
    }
}

void rtc_callback(void) 
{
    //翻转引脚，方便示波器测量时间
    hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);
    running_time += 1;
}
