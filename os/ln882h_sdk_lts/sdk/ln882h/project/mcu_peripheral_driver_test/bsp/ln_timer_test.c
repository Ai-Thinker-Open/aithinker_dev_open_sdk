/**
 * @file     ln_timer_test.c
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2021-08-20
 *
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

/*
    Timer使用说明：

    1. Timer Mode寄存器使用方式：
                                0 ： 使用自由运行模式时，计数器会自动加载最大值。
                                1 ： 使用用户定义模式时，计数器会自动加载我们设定的计数值。


    2. 不使用PWM也不分频的情况下，定时时间为 ((1 / pclk0) * tim_load_value) s

    3. 使用PWM时，配置PWM使能位，然后配置tim_cnt2,则PWM周期为  ((1 / pclk0) * (tim_load_value + 1 + tim_load2_value + 1))s，

       其中 tim_load_value + 1 的时间为低电平时间，tim_load2_value + 1 的时间为高电平时间。使用PWM时，定时器的中断会有两次。

    4. PWM占空比不可设置为0%或者100%,否则会出现毛刺，原因是因为设置的定时都是 tim_load_value + 1的，所以最终还是有一点点跳变的。

    5. 测试代码中，Timer0，Timer2，Timer3作为定时器使用，Timer1作为PWM使用

    6. 测试引脚：
                    PB5     Timer0 中断翻转IO
                    PB6     Timer2 中断翻转IO
                    PB7     Timer3 中断翻转IO
                    PB8     Timer1 PWM IO

    7. PWM的占空比控制在while(1)循环中，每隔500ms改变一次。


*/


#include "ln_timer_test.h"
#include "hal/hal_timer.h"
#include "hal/hal_gpio.h"
#include "ln_test_common.h"
#include "utils/debug/log.h"

void ln_timer_test(void)
{
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    gpio_init.pin = GPIO_PIN_6;                    //配置GPIO引脚号
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    gpio_init.pin = GPIO_PIN_7;                    //配置GPIO引脚号
    hal_gpio_init(GPIOB_BASE,&gpio_init);

    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_8,TIMER1_PWM);     //配置TIMER1 PWM引脚复用
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_ENABLE);         //使能引脚复用



    //Timer初始化
    tim_init_t_def tim_init;
    memset(&tim_init,0,sizeof(tim_init));

    tim_init.tim_load_value = 80000 - 1;                   //设置加载值，定时时间 = 1 / 80000000 * 80000 = 1ms
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;      //设置定时器模式，设置为用户定义模式

    tim_init.tim_div = 0;                           //设置Timer不分频

    //Timer0初始化
    hal_tim_init(TIMER0_BASE,&tim_init);            //初始化定时器
    hal_tim_en(TIMER0_BASE,HAL_ENABLE);             //使能定时器模块
    NVIC_SetPriority(TIMER0_IRQn,     4);           //配置定时器优先级
    NVIC_EnableIRQ(TIMER0_IRQn);                    //使能定时器中断
    hal_tim_it_cfg(TIMER0_BASE,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);      //配置中断

    //Timer2初始化
    hal_tim_init(TIMER2_BASE,&tim_init);            //初始化定时器
    hal_tim_en(TIMER2_BASE,HAL_ENABLE);             //使能定时器模块
    NVIC_SetPriority(TIMER2_IRQn,     4);           //配置定时器优先级
    NVIC_EnableIRQ(TIMER2_IRQn);                    //使能定时器中断
    hal_tim_it_cfg(TIMER2_BASE,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);      //配置中断

    //Timer3初始化
    hal_tim_init(TIMER3_BASE,&tim_init);            //初始化定时器
    hal_tim_en(TIMER3_BASE,HAL_ENABLE);             //使能定时器模块
    NVIC_SetPriority(TIMER3_IRQn,     4);           //配置定时器优先级
    NVIC_EnableIRQ(TIMER3_IRQn);                    //使能定时器中断
    hal_tim_it_cfg(TIMER3_BASE,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);      //配置中断

    //Timer PWM初始化
    tim_init.tim_load2_value = 80000 - 1;                  //高电平时间 = 1 / 80000000 * 80000 = 1ms，所以PWM高电平时间=1ms,低电平时间 = 1ms.
    hal_tim_init(TIMER1_BASE,&tim_init);            //初始化定时器
    hal_tim_en(TIMER1_BASE,HAL_ENABLE);             //使能定时器模块
    hal_tim_pwm_en(TIMER1_BASE,HAL_ENABLE);         //使能PWM
    NVIC_SetPriority(TIMER1_IRQn,     1);           //配置定时器优先级
    NVIC_EnableIRQ(TIMER1_IRQn);                    //使能定时器中断
    hal_tim_it_cfg(TIMER1_BASE,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);      //配置中断

    float duty = 0.01f;
    uint8_t cnt_dir = 0;
    while(1)
    {

        /*占空比实时改变，CNT值为0时生效*/
        if(cnt_dir == 0)
            duty += 0.01f;
        else
            duty -= 0.01f;

        if(duty >= 0.98f)
            cnt_dir = 1;
        if(duty <= 0.02f)
            cnt_dir = 0;

        hal_tim_set_load_value(TIMER1_BASE,duty * 160000);
        hal_tim_set_load2_value(TIMER1_BASE,(1-duty) * 160000);

        hal_tim_set_load_value(TIMER0_BASE,duty * 160000);
        hal_tim_set_load2_value(TIMER0_BASE,(1-duty) * 160000);
        ln_delay_ms(500);
    }
}

void TIMER0_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER0_BASE,TIM_IT_FLAG_ACTIVE) == 1)
    {
        hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);                   //翻转GPIO，便于测试
        hal_tim_clr_it_flag(TIMER0_BASE,TIM_IT_FLAG_ACTIVE);      //清除标志位
    }
}

void TIMER1_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER1_BASE,TIM_IT_FLAG_ACTIVE) == 1)
    {
        //hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_6);                //翻转GPIO，便于测试
        hal_tim_clr_it_flag(TIMER1_BASE,TIM_IT_FLAG_ACTIVE);      //清除标志位
    }
}

void TIMER2_IRQHandler()
{
    if(hal_tim_get_it_flag(TIMER2_BASE,TIM_IT_FLAG_ACTIVE) == 1)
    {
        hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_6);                   //翻转GPIO，便于测试
        hal_tim_clr_it_flag(TIMER2_BASE,TIM_IT_FLAG_ACTIVE);      //清除标志位
    }
}

void TIMER3_IRQHandler(void)
{
    if(hal_tim_get_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE) == 1)
    {
        hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_7);                   //翻转GPIO，便于测试
        hal_tim_clr_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE);      //清除标志位
    }
}

