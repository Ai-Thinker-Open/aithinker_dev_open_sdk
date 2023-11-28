/**
 * @file     ln_drv_trng.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_timer.h"
#include "hal/hal_clock.h"

void (*timer0_it_handler)(void);
void (*timer1_it_handler)(void);
void (*timer2_it_handler)(void);
void (*timer3_it_handler)(void);

/**
 * @brief 定时器初始化
 * 
 * @param timer_ch 选择定时器通道
 * @param us       0 ~ 16777215 us
 * @param timer_it_callback 定时器中断回调函数
 */
void timer_init(timer_ch_t timer_ch,uint32_t us,void (*timer_it_callback)(void))
{
    uint32_t timer_base = 0;
    switch(timer_ch)
    {
        case TIMER_CH_0: timer_base = TIMER0_BASE; break;
        case TIMER_CH_1: timer_base = TIMER1_BASE; break;
        case TIMER_CH_2: timer_base = TIMER2_BASE; break;
        case TIMER_CH_3: timer_base = TIMER3_BASE; break;
    }

    //Timer初始化
    tim_init_t_def tim_init;
    memset(&tim_init,0,sizeof(tim_init));
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;                      //设置定时器模式，设置为用户定义模式
    
    if(us < 1000){
        tim_init.tim_div = 0;                                      
        tim_init.tim_load_value = us * (uint32_t)(hal_clock_get_apb0_clk() / 1000000)  - 1;                       
    }else{
        tim_init.tim_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;                                
        tim_init.tim_load_value = us - 1;                           
    }
    hal_tim_init(timer_base,&tim_init);                             //初始化定时器
    hal_tim_en(timer_base,HAL_ENABLE);                              //使能定时器模块
    hal_tim_it_cfg(timer_base,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);       //配置中断

    switch(timer_ch)
    {
        case TIMER_CH_0:
        {
            NVIC_SetPriority(TIMER0_IRQn,     4);           //配置定时器优先级
            NVIC_EnableIRQ(TIMER0_IRQn);                    //使能定时器中断
            if(timer_it_callback != NULL)
                timer0_it_handler = timer_it_callback;
            break;
        }
        case TIMER_CH_1:
        {
            NVIC_SetPriority(TIMER1_IRQn,     4);           //配置定时器优先级
            NVIC_EnableIRQ(TIMER1_IRQn);                    //使能定时器中断
            if(timer_it_callback != NULL)
                timer1_it_handler = timer_it_callback;
            break;
        }
        case TIMER_CH_2:
        {
            NVIC_SetPriority(TIMER2_IRQn,     4);           //配置定时器优先级
            NVIC_EnableIRQ(TIMER2_IRQn);                    //使能定时器中断
            if(timer_it_callback != NULL)
                timer2_it_handler = timer_it_callback;
            break;
        }
        case TIMER_CH_3:
        {
            NVIC_SetPriority(TIMER3_IRQn,     4);           //配置定时器优先级
            NVIC_EnableIRQ(TIMER3_IRQn);                    //使能定时器中断
            if(timer_it_callback != NULL)
                timer3_it_handler = timer_it_callback;
            break;
        }
    }           
}

/**
 * @brief 获取当前定时器中计数器的运行时间，单位us
 * 
 * @param timer_ch      选择定时器通道
 * @return uint32_t     返回当前定时器中计数器的运行时间，单位us
 */
uint32_t timer_get_timer_cnt_time(timer_ch_t timer_ch)
{
    uint32_t timer_base = 0;
    switch(timer_ch)
    {
        case TIMER_CH_0: timer_base = TIMER0_BASE; break;
        case TIMER_CH_1: timer_base = TIMER1_BASE; break;
        case TIMER_CH_2: timer_base = TIMER2_BASE; break;
        case TIMER_CH_3: timer_base = TIMER3_BASE; break;
    }
    return (uint32_t)((hal_tim_get_load_value(timer_base) - hal_tim_get_current_cnt_value(timer_base)) 
            / ((hal_clock_get_apb0_clk() / 1000000) * 1.0f / ((hal_tim_get_div(timer_base) + 1))));
}

/**
 * @brief TIMER0中断处理函数
 */
void TIMER0_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER0_BASE,TIM_IT_FLAG_ACTIVE))
    {
        hal_tim_clr_it_flag(TIMER0_BASE,TIM_IT_FLAG_ACTIVE);
        if(timer0_it_handler != NULL)
            timer0_it_handler();
    }
}

/**
 * @brief TIMER1中断处理函数
 */
void TIMER1_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER1_BASE,TIM_IT_FLAG_ACTIVE))
    {
        hal_tim_clr_it_flag(TIMER1_BASE,TIM_IT_FLAG_ACTIVE);
        if(timer1_it_handler != NULL)
            timer1_it_handler();
    }
} 

/**
 * @brief TIMER2中断处理函数
 */
void TIMER2_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER2_BASE,TIM_IT_FLAG_ACTIVE))
    {
        hal_tim_clr_it_flag(TIMER2_BASE,TIM_IT_FLAG_ACTIVE);
        if(timer2_it_handler != NULL)
            timer2_it_handler();
    }
}

/**
 * @brief TIMER3中断处理函数
 */
void TIMER3_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE))
    {
        hal_tim_clr_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE);
        if(timer3_it_handler != NULL)
            timer3_it_handler();
    }
}
