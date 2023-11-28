/**
 * @file     ln_rtc_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-23
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/**
        RTC使用说明：
                    1. RTC会从load计数到match，所以LOAD寄存器里面的值可以理解为初值。
                    
                    2. rtc_warp_en = 1时，当计数器达到匹配值时会从0开始计数，否则计数器一直会计数下去，直到溢出。
                    
                    3. RTC可以使用SleepTimer的校准值来校准。

                    4. GPIOB5为RTC中断翻转引脚
                    
                    5. 定时时间 T = ( match - load ) * (1 / 32k) s
                    
                    6. 使用RTC定时比较长的时间时，可以使用SleepTimer进行校准。
                    
                        a. 先进行校准
                        
                        b. 使用校准值对32K进行校准
                        
                           例如设置定时 1s时，rtc_match = (32000 - 0) * (1 / 32K) * (32k_cal / 32k),32k_cal为校准值。
                           
                           
                    7. 对于SleepTimer 先暂时不用关心，后面低功耗相关会具体说明
*/ 


#include "ln_rtc_test.h"
#include "hal_rtc.h"
#include "hal/hal_gpio.h"
#include "hal/hal_misc.h"
#include "utils/debug/log.h"

void ln_rtc_test(void)
{
    //使用SleepTimer对32K的时钟进行校准
    hal_misc_awo_set_o_cpu_sleep_counter_bp(0);
    
    //hal_misc_awo_get_i_rco32k_period_ns();    //32K的校准值

    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO
    
    //RTC初始化
    rtc_init_t_def rtc_init;
    rtc_init.rtc_warp_en = RTC_WRAP_EN;                         //达到匹配值之后从0计数
    hal_rtc_init(RTC_BASE,&rtc_init);
    
    //RTC会从load计数到match，((32-0)*(1/32000)) = 1ms
    hal_rtc_set_cnt_match(RTC_BASE,32);                         //设置匹配计数器值
    hal_rtc_set_cnt_load(RTC_BASE,0);                           //设置初始计数器值
    hal_rtc_it_cfg(RTC_BASE,RTC_IT_FLAG_ACTIVE,HAL_ENABLE);     //配置中断
    
    NVIC_SetPriority(RTC_IRQn,4);
    NVIC_EnableIRQ(RTC_IRQn);
    
    hal_rtc_en(RTC_BASE,HAL_ENABLE);                            //使能RTC
    
    while(1);
}


void ln_sleeptimer_test(void)
{

    //使用之前先做校准
    hal_misc_awo_set_o_cpu_sleep_counter_bp(0);

    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    hal_misc_awo_set_o_cpu_sleep_time_l(320);      //((320-0)*(1/32000)) = 10ms
    hal_misc_awo_set_o_cpu_sleep_time_l(0);
    hal_misc_awo_set_o_cpu_sleep_inten(1);
    
    NVIC_SetPriority(RFSLP_IRQn,1);  //配置中断优先级
    NVIC_EnableIRQ(RFSLP_IRQn);  
}

void RFSLP_IRQHandler()
{
    hal_misc_awo_set_o_cpu_sleep_inten(0);
    hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);
}

void RTC_IRQHandler()
{
   if(hal_rtc_get_it_flag(RTC_BASE,RTC_IT_FLAG_ACTIVE) == 1)
   {
       hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);                  //翻转引脚
       hal_rtc_clr_it_flag(RTC_BASE,RTC_IT_FLAG_ACTIVE);      //清除标志位
   }
}

