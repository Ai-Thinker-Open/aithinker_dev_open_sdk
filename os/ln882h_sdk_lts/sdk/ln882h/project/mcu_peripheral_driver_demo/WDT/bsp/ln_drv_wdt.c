/**
 * @file     ln_drv_wdt.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-09
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_wdt.h"


/**
 * @brief 看门狗初始化
 * 
 * @param wdt_top_value 设置看门狗定时器时间
 * 
 * 注：WDT_RMOD_1 = 1时，当看门狗定时器达到设定时间后，会先产生中断，如果中断中没有喂狗，同时定时器再次溢出，则会产生复位。
 */
void wdt_init(wdt_top_value_t wdt_top_value)
{
    /* 看门狗初始化 */
    wdt_init_t_def wdt_init;
    memset(&wdt_init,0,sizeof(wdt_init));
    wdt_init.wdt_rmod = WDT_RMOD_1;         //等于0的时候，计数器溢出时直接复位，等于1的时候，先产生中断，如果再次溢出，则产生复位。
    wdt_init.wdt_rpl = WDT_RPL_32_PCLK;     //设置复位延时的时间
    wdt_init.top = wdt_top_value;           //设置看门狗计数器的值,当TOP=1时，对应计数器的值为0x1FF，而看门狗是用的时钟是一个单独的32k时钟，
                                            //所以此时的喂狗时间必须在 (1/32k) * 0x1FF 内。
    hal_wdt_init(WDT_BASE,&wdt_init);
    
    /* 配置看门狗中断 */
    NVIC_SetPriority(WDT_IRQn,     4);
    NVIC_EnableIRQ(WDT_IRQn);
    
    /* 使能看门狗，注：一旦打开看门狗，除非断电重启，否则无法关闭看门狗*/
    hal_wdt_en(WDT_BASE,HAL_ENABLE);
    
    /* 先喂下狗 */
    hal_wdt_cnt_restart(WDT_BASE);
}

void wdt_feed_dog()
{
    hal_wdt_cnt_restart(WDT_BASE);
}

void WDT_IRQHandler()
{
     //hal_wdt_cnt_restart(WDT_BASE);                  //喂狗操作
}
