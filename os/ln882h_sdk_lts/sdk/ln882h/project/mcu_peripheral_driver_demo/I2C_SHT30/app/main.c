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
#include "ln_drv_sht30.h"


uint8_t  addr[128];
uint16_t addr_num = 0;
uint8_t  sensor_data[20];

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

    /****************** 2. I2C SHT30测试***********************/
    ln_tem_hum_init();
    while(1)
    {
        if (tem_hum_read_sensor_data(sensor_data) == 0) 
        {
            float tem_data = -45 + (175 * (sensor_data[0] * 256 + sensor_data[1]) * 1.0 / (65535 - 1)); //转换为摄氏度
            float hum_data = 100 * ((sensor_data[3] * 256 + sensor_data[4]) * 1.0 / (65535 - 1));       //转换为%

            LOG(LOG_LVL_INFO,"TEM = %f , HUM + %f % \n",tem_data,hum_data);
        }
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(500);
    }
}
