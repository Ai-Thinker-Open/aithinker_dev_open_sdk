/**
 * @file     ln_gpio_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-30
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
        GPIO测试说明
        
        1. 
*/

 
 
#include "hal_common.h"
#include "hal_gpio.h"
#include "ln_test_common.h"
#include "ln_gpio_test.h"
#include "utils/debug/log.h"

void ln_gpio_test()
{
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体

//    /* 引脚输出测试 */
//    gpio_init.dir   = GPIO_OUTPUT;                 //配置GPIO方向，输入或者输出
//    gpio_init.pin   = GPIO_PIN_5;                  //配置GPIO引脚号
//    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
//    gpio_init.mode  = GPIO_MODE_DIGITAL;           //设置引脚为数字引脚
//    gpio_init.pull  = GPIO_PULL_UP;                //设置引脚为上拉
//    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO
//    
//    /* 引脚输出测试 */
//    gpio_init.dir   = GPIO_OUTPUT;                 //配置GPIO方向，输入或者输出
//    gpio_init.pin   = GPIO_PIN_7;                  //配置GPIO引脚号
//    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
//    gpio_init.mode  = GPIO_MODE_DIGITAL;           //设置引脚为数字引脚
//    gpio_init.pull  = GPIO_PULL_UP;                //设置引脚为上拉
//    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    /* 引脚输入中断测试 */
    gpio_init.dir   = GPIO_INPUT;                  //配置GPIO方向，输入或者输出
    gpio_init.pin   = GPIO_PIN_5;                  //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    gpio_init.mode  = GPIO_MODE_DIGITAL;           //设置引脚为数字引脚
    gpio_init.pull  = GPIO_PULL_UP;                //设置引脚为上拉
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO
    
    gpio_init.dir   = GPIO_INPUT;                  //配置GPIO方向，输入或者输出
    gpio_init.pin   = GPIO_PIN_6;                  //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    gpio_init.mode  = GPIO_MODE_DIGITAL;           //设置引脚为数字引脚
    gpio_init.pull  = GPIO_PULL_UP;                //设置引脚为上拉
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    /* 设置下降沿中断 */
    hal_gpio_pin_it_cfg(GPIOB_BASE,GPIO_PIN_6,GPIO_INT_FALLING);
    hal_gpio_pin_it_en(GPIOB_BASE,GPIO_PIN_6,HAL_ENABLE);    

    hal_gpio_pin_it_cfg(GPIOB_BASE,GPIO_PIN_5,GPIO_INT_FALLING);
    hal_gpio_pin_it_en(GPIOB_BASE,GPIO_PIN_5,HAL_ENABLE);         
    NVIC_SetPriority(GPIOB_IRQn,1);                //配置中断优先级
    NVIC_EnableIRQ(GPIOB_IRQn);  

    while(1)
    {
        ln_delay_ms(500);
        //hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);    //翻转引脚
    }
}


void GPIOB_IRQHandler()
{
    if(hal_gpio_pin_get_it_flag(GPIOB_BASE,GPIO_PIN_5) == HAL_SET)
    {
        hal_gpio_pin_clr_it_flag(GPIOB_BASE,GPIO_PIN_5);
        LOG(LOG_LVL_INFO,"KEY_1 has pressed! \r\n");
    }
    
    if(hal_gpio_pin_get_it_flag(GPIOB_BASE,GPIO_PIN_6) == HAL_SET)
    {
        hal_gpio_pin_clr_it_flag(GPIOB_BASE,GPIO_PIN_6);
        LOG(LOG_LVL_INFO,"KEY_2 has pressed! \r\n");
    }
}

