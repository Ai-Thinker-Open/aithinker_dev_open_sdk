/**
 * @file     ln_drv_rtc.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_rtc.h"

void (*rtc_it_handler)(void);

static uint32_t time_ms = 0;    //定时时间

/**
 * @brief 初始化RTC
 * 
 * @param ms                设置RTC定时时间，范围：0ms - 134217727ms
 * @param rtc_it_callback   设置中断回调函数
 */
void rtc_init(uint32_t ms,void (*rtc_it_callback)(void))
{
    //使用SleepTimer对32K的时钟进行校准,该过程可以省略，不影响使用
    hal_misc_awo_set_o_cpu_sleep_counter_bp(0);
    //等待校准完成
    for(int i = 0; i < 10000; i++);
    
    //RTC初始化
    rtc_init_t_def rtc_init;
    rtc_init.rtc_warp_en = RTC_WRAP_EN;                       
    hal_rtc_init(RTC_BASE,&rtc_init);
    hal_rtc_set_cnt_match(RTC_BASE,ms * 1000000.0f / hal_misc_awo_get_i_rco32k_period_ns());                         
    hal_rtc_set_cnt_load(RTC_BASE,0);                           
    hal_rtc_it_cfg(RTC_BASE,RTC_IT_FLAG_ACTIVE,HAL_ENABLE);

    if(rtc_it_callback != NULL)
        rtc_it_handler = rtc_it_callback;
    time_ms = ms;
    
    NVIC_SetPriority(RTC_IRQn,4);
    NVIC_EnableIRQ(RTC_IRQn);
    
    hal_rtc_en(RTC_BASE,HAL_ENABLE);                       
}

/**
 * @brief 获取当前RTC计数器的时间(单位ms)  时间 = rtc_cnt * 1.0 / 32 ms
 * 
 * @return uint32_t 返回当前RTC计数器的时间(单位ms)
 */
uint32_t rtc_get_rtc_current_cnt_time()
{
    //注：RTC计数器是向上计数的
    return (uint32_t)(hal_rtc_get_cnt(RTC_BASE) * 1.0f / 32);
}

/**
 * @brief RTC中断处理函数
 * 
 */
void RTC_IRQHandler()
{
    hal_rtc_set_cnt_match(RTC_BASE,time_ms * 1000000.0f / hal_misc_awo_get_i_rco32k_period_ns());    
    hal_misc_awo_set_o_cpu_sleep_counter_bp(0);
    hal_rtc_clr_it_flag(RTC_BASE,RTC_IT_FLAG_ACTIVE);
    if(rtc_it_handler != NULL)
        rtc_it_handler();
}
