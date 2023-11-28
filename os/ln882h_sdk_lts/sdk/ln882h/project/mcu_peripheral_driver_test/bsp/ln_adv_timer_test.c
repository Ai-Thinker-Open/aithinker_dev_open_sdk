/**
 * @file     ln_adv_timer_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


/**
    PWM使用说明：
    
    1. 引脚说明：
            A8  ->  PWM引脚1
            A9  ->  PWM引脚2
            B7  ->  脉冲计数输入引脚

    2. 配置PWM死区的时候，会自动让pwma和pwmb形成互补，此时，如果想改变波形的占空比只能通过改变pwma的通道值，改变pwmb无效,死区时间 = dead_gap_value * 1 / (APB / DIV) s

    3. 如果想要改变PWM对齐方式，只需要改变cnt_mode这个寄存器，就可以了。

        * ADV_TIMER_CNT_MODE_INC      是计数器向上计数，对应的PWM就是左对齐。
        * ADV_TIMER_CNT_MODE_DEC      是计数器向下计数，对应的PWM就是右对齐。
        * ADV_TIMER_CNT_MODE_BOTH     是计数器先向上计数然后向下计数，对应的PWM就是中间对齐，中间对齐时要注意PWM的频率左右对齐的二分之一（相同LOAD值的情况下）。
        
    4. 请注意，load寄存器只能在PWM失能的情况下修改，否则无效。
    
    5. 设置PWM占空比的时候，请不要设置成100%，否则会出现毛刺！
    
    6. PWM的ISRR寄存器，是W1C，因此不支持使用位域的方式清除中断标志位。直接SET ISRR寄存器相应的标志位为1来清除中断标志位。
    
    7. PWM周期 p = (load) * 1 / (APB / DIV) s，占空比为 d = (load - cmp_a) / load,其中 load 为加载值，cmp_a为比较值，APB为外设时钟，DIV为高级定时器的分频。
*/


#include "ln_adv_timer_test.h"
#include "ln_test_common.h"
#include "hal/hal_adv_timer.h"
#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"
#include "utils/debug/log.h"
#include "reg_adv_timer.h"

void ln_adv_timer_test(void)
{
    __NVIC_SetPriorityGrouping(4);
    __enable_irq();

    /* pwm 引脚初始化 */
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_8,ADV_TIMER_PWM0);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_9,ADV_TIMER_PWM1);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_7,ADV_TIMER_PWM2);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_8,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_9,HAL_ENABLE);  
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_7,HAL_ENABLE);  
    
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_8;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    /* PWM参数初始化 */
    adv_tim_init_t_def adv_tim_init;
    memset(&adv_tim_init,0,sizeof(adv_tim_init));
    adv_tim_init.adv_tim_clk_div = 0;                               //设置时钟分频，0为不分频
    adv_tim_init.adv_tim_load_value =  40000 - 1;                   //设置PWM频率，40000 * 1 / PCLK(80M) / DIV(0) = 2k
    adv_tim_init.adv_tim_cmp_a_value = 40000 ;                      //设置通道a比较值，占空比为 50%
    adv_tim_init.adv_tim_cmp_b_value = 40000 ;                      //设置通道b比较值，占空比为 50%
    adv_tim_init.adv_tim_dead_gap_value = 1000;                     //设置死区时间
    adv_tim_init.adv_tim_dead_en = ADV_TIMER_DEAD_DIS;              //不开启死区
    adv_tim_init.adv_tim_cnt_mode = ADV_TIMER_CNT_MODE_INC;         //向上计数模式
    adv_tim_init.adv_tim_cha_en = ADV_TIMER_CHA_EN;                 //使能通道a
    adv_tim_init.adv_tim_chb_en = ADV_TIMER_CHB_EN;                 //使能通道b
    adv_tim_init.adv_tim_cha_it_mode = ADV_TIMER_CHA_IT_MODE_INC;   //使能通道a向上计数中断
    adv_tim_init.adv_tim_chb_it_mode = ADV_TIMER_CHB_IT_MODE_INC;   //使能通道b向上计数中断
    hal_adv_tim_init(ADV_TIMER_0_BASE,&adv_tim_init);               //初始化ADV_TIMER0


    /* 输入捕获-脉冲计数配置初始化 */
    memset(&adv_tim_init,0,sizeof(adv_tim_init));
    adv_tim_init.adv_tim_clk_div = 0;                               //设置时钟分频，0为不分频
    adv_tim_init.adv_tim_cap_edg = ADV_TIMER_EDG_RISE;              //设置捕获上升沿
    adv_tim_init.adv_tim_cap_mode = ADV_TIMER_CAP_MODE_2;           //设置捕获模式2
    adv_tim_init.adv_tim_cap_en = ADV_TIMER_CAP_EN;                 //捕获模式使能
    adv_tim_init.adv_tim_load_value = 0xFFFF;
    hal_adv_tim_init(ADV_TIMER_1_BASE,&adv_tim_init);               //初始化ADV_TIMER1
    
    /* enable the pwm interrupt */
    hal_adv_tim_it_cfg(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPA,HAL_ENABLE); //使能通道a中断
    hal_adv_tim_it_cfg(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPB,HAL_ENABLE); //使能通道b中断
    hal_adv_tim_it_cfg(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_LOAD,HAL_ENABLE); //使能加载值中断

//    hal_adv_tim_it_cfg(ADV_TIMER_1_BASE,ADV_TIMER_IT_FLAG_CMPA,HAL_ENABLE); //使能通道a中断
//    hal_adv_tim_it_cfg(ADV_TIMER_1_BASE,ADV_TIMER_IT_FLAG_CMPB,HAL_ENABLE); //使能通道b中断
//    hal_adv_tim_it_cfg(ADV_TIMER_1_BASE,ADV_TIMER_IT_FLAG_LOAD,HAL_ENABLE); //使能加载值中断
    
    NVIC_SetPriority(ADV_TIMER_IRQn,     4);
    NVIC_EnableIRQ(ADV_TIMER_IRQn);
    
    volatile uint32_t cmp_a_value = 0;       //通道a的比较值
    float    duty_value  = 0;       //占空比
    uint8_t  inc_dir     = 0;       //占空比递增/减方向
    volatile uint32_t pulse_cnt   = 0;       //脉冲计数
    
    while(1)
    {
        if(inc_dir == 0)
            duty_value += 0.01f;
        else
            duty_value -= 0.01f;
        
        if(duty_value >= 0.50f)
            inc_dir = 1;
        if(duty_value <= 0.01f)
            inc_dir = 0;
        
        hal_adv_tim_set_comp_a(ADV_TIMER_0_BASE,duty_value * 40000);
        hal_adv_tim_set_comp_b(ADV_TIMER_0_BASE,duty_value * 40000);
        
        ln_delay_ms(15);
        
        pulse_cnt = hal_adv_tim_get_trig_value(ADV_TIMER_1_BASE);
    }
}

void ADV_TIMER_IRQHandler()
{
    if(hal_adv_tim_get_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPB) == 1)
    {
        hal_adv_tim_clr_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPB);
        //hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_7);
    }
    if(hal_adv_tim_get_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPA) == 1)
    {
        hal_adv_tim_clr_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_CMPA);
        //hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_8);
    }
    if(hal_adv_tim_get_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_LOAD) == 1)
    {
        hal_adv_tim_clr_it_flag(ADV_TIMER_0_BASE,ADV_TIMER_IT_FLAG_LOAD);
        //hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_7);
    }
    
    if(hal_adv_tim_get_it_flag(ADV_TIMER_1_BASE,ADV_TIMER_IT_FLAG_LOAD) == 1)
    {
        hal_adv_tim_clr_it_flag(ADV_TIMER_1_BASE,ADV_TIMER_IT_FLAG_LOAD);
        hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_8);
    }
}

