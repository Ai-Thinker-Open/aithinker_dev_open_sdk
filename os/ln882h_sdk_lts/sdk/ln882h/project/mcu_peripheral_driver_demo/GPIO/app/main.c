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
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

uint32_t data_buf[100];
uint8_t pin_sta = 0;
int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. 初始化GPIO结构体 ***********************/
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    
    /****************** 3. GPIO 输出测试***********************/
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向为输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    //将引脚输出高电平
    hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_5);       
    
    //将引脚输出低电平
    hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_5);     
    
    //翻转引脚
    hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);     
    
    
    /****************** 4. GPIO 输入测试***********************/
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_INPUT;                    //配置GPIO方向为输入
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO
    
    //读取引脚电平
    pin_sta = hal_gpio_pin_read(GPIOB_BASE,GPIO_PIN_5);
    
    //配置内部上拉
    hal_gpio_pin_pull_set(GPIOB_BASE,GPIO_PIN_5,GPIO_PULL_UP);
    
    //配置内部下拉
    hal_gpio_pin_pull_set(GPIOB_BASE,GPIO_PIN_5,GPIO_PULL_DOWN);
    
    //配置引脚为输出
    hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_5,GPIO_OUTPUT);
    
    //配置引脚为输入
    hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_5,GPIO_INPUT);
    
    while(1)
    {
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(1000);
    }
}
