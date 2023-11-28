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
#include "hal/hal_gpio.h"
#include "hal/hal_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

#include "ln_drv_timer.h"

void timer0_callback(void);
void timer1_callback(void);
void timer2_callback(void);
void timer3_callback(void); //通道3在使用wifi的情况下会被wifi占用，请慎重使用

int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. 设置示波器测试引脚 ***********************/
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));        //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                   //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_5;                    //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;             //设置GPIO速度
    hal_gpio_init(GPIOB_BASE,&gpio_init);          //初始化GPIO

    /****************** 3. TIMER 测试***********************/
    timer_init(TIMER_CH_0,100 ,timer0_callback);    //定时100us
    timer_init(TIMER_CH_1,1000000,timer1_callback); //定时1000000us
    timer_init(TIMER_CH_2,2000000,timer2_callback); //定时2000000us
    timer_init(TIMER_CH_3,5000000,timer3_callback); //定时5000000us,通道3在使用wifi的情况下会被wifi占用，请慎重使用

    while(1)
    {
        LOG(LOG_LVL_INFO,"%d\n",timer_get_timer_cnt_time(TIMER_CH_3));
        ln_delay_ms(1000);
    }
}

void timer0_callback()
{
    //翻转引脚，方便示波器测量时间
    hal_gpio_pin_toggle(GPIOB_BASE,GPIO_PIN_5);
    //LOG(LOG_LVL_INFO,"go in timer0 interrupt! \n");
}

void timer1_callback()
{
    LOG(LOG_LVL_INFO,"go in timer1 interrupt! \n");
}

void timer2_callback()
{
    LOG(LOG_LVL_INFO,"go in timer2 interrupt! \n");
}

void timer3_callback()
{
    LOG(LOG_LVL_INFO,"go in timer3 interrupt! \n");
}
