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
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

#include "ln_drv_i2c.h"
#include "ln_drv_oled.h"


uint8_t  addr[128];
uint16_t addr_num = 0;

/*
 *  注意！！！ 
 *  在驱动例程中，有一些函数的参数是指向字符串（数组）地址的指针，函数中没有检查这些指针的有效性，
 *  没有判断指针是否为空，也没有判断指针指向的数据是否还有剩余空间，这些都需要使用者自行决定！！！
*/
int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. I2C OLED测试***********************/
    gpio_init_t_def gpio_init;                  //配置OLED RST引脚
    memset(&gpio_init, 0, sizeof(gpio_init));   //清零结构体

    gpio_init.dir = GPIO_OUTPUT;                //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_4;                 //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;          //设置GPIO速度
    hal_gpio_init(GPIOB_BASE, &gpio_init);      //初始化GPIO
    hal_gpio_pin_reset(GPIOB_BASE, GPIO_PIN_4); //RESET OLED屏幕
    ln_delay_ms(100);       
    hal_gpio_pin_set(GPIOB_BASE, GPIO_PIN_4);

    i2c_init(GPIO_B,GPIO_PIN_8,GPIO_B,GPIO_PIN_9,400000);

    i2c_address_scan(addr,&addr_num);
    
    oled_init();
    oled_display_on();
    oled_show_string(0, 40, (unsigned char *)"Test success!", 12); //显示测试成功的英文
    oled_refresh(); //刷新显示

    while(1)
    {
        LOG(LOG_LVL_INFO,"ln882H running! \n");
        
        ln_delay_ms(100);
    }
}
