/**
 * @file     ln_wdt_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-24
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
        WDT使用说明：
                    1. WDT使用的是一个单独的32k时钟。
                    2. 通过设置WDT运行模式来设置WDT是否产生中断。0：计数器溢出直接复位； 1：计数器溢出先产生中断，如果再次溢出则复位。
                    3. 喂狗时间 = 2^(8 + TOP) * (1/32k);

*/


#include "ln_wdt_test.h"
#include "hal/hal_wdt.h"
#include "hal/hal_gpio.h"
#include "utils/debug/log.h"

void ln_wdt_test(void)
{
    /* 引脚初始化 */
    gpio_init_t_def gpio_init;
	memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = GPIO_PIN_5;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(GPIOB_BASE,&gpio_init);
    hal_gpio_pin_reset(GPIOB_BASE,GPIO_PIN_5);
    
    /* 看门狗初始化 */
    wdt_init_t_def wdt_init;
    memset(&wdt_init,0,sizeof(wdt_init));
    wdt_init.wdt_rmod = WDT_RMOD_1;         //等于0的时候，计数器溢出时直接复位，等于1的时候，先产生中断，如果再次溢出，则产生复位。
    wdt_init.wdt_rpl = WDT_RPL_32_PCLK;     //设置复位延时的时间
    wdt_init.top = WDT_TOP_VALUE_1;         //设置看门狗计数器的值,当TOP=1时，对应计数器的值为0x1FF，而看门狗是用的时钟是一个单独的32k时钟，
                                            //所以此时的喂狗时间必须在 (1/32k) * 0x1FF 内。
    hal_wdt_init(WDT_BASE,&wdt_init);
    
    /* 配置看门狗中断 */
    NVIC_SetPriority(WDT_IRQn,     4);
    NVIC_EnableIRQ(WDT_IRQn);
    
    /* 使能看门狗 */
    hal_wdt_en(WDT_BASE,HAL_ENABLE);
    
    /* 先喂下狗 */
    hal_wdt_cnt_restart(WDT_BASE);
    
    /* 测试引脚 */
    hal_gpio_pin_set(GPIOB_BASE,GPIO_PIN_5);
    while(1)
    {
        
    }
}

void WDT_IRQHandler()
{
    //注：WDT的寄存器时钟使用的32K，而CPU时钟160M，远远大于32K，所以可能出现清完WDT中断标志位后再次进入WDT中断的情形（WDT 32k操作寄存器比较慢）。
    hal_wdt_cnt_restart(WDT_BASE);              //喂狗操作
    hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);     //测试引脚翻转
    LOG(LOG_LVL_INFO,"feed dog~! \r\n");        //LOG打印
}


