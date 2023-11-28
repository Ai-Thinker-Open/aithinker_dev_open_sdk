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
#include "ln_drv_mpu9250.h"


static float mpu_tem_data = 0;

static short mpu_gx_data = 0; //角速度信息
static short mpu_gy_data = 0;
static short mpu_gz_data = 0;

static short mpu_ax_data = 0; //加速度信息
static short mpu_ay_data = 0;
static short mpu_az_data = 0;

static short mpu_mx_data = 0; //磁力计信息
static short mpu_my_data = 0;
static short mpu_mz_data = 0;

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
    i2c_init(GPIO_B,GPIO_PIN_8,GPIO_B,GPIO_PIN_9,400000);
    mpu_init();
    while(1)
    {
       // 3. 更新九轴传感器信息
        mpu_tem_data = mpu_get_temperature();
        mpu_get_gyroscope(&mpu_gx_data, &mpu_gy_data, &mpu_gz_data);     //获取角速度信息
        mpu_get_accelerometer(&mpu_ax_data, &mpu_ay_data, &mpu_az_data); //或者加速度信息
        mpu_get_magnetometer(&mpu_mx_data, &mpu_my_data, &mpu_mz_data);  //获取磁力计信息

        LOG(LOG_LVL_INFO, "MPU_TEM = %f !\n", mpu_tem_data / 100);

        LOG(LOG_LVL_INFO, "ACC:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n", mpu_ax_data, mpu_ay_data, mpu_az_data);
        LOG(LOG_LVL_INFO, "GYR:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n", mpu_gx_data, mpu_gy_data, mpu_gz_data);
        LOG(LOG_LVL_INFO, "MAG:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n\n", mpu_mx_data, mpu_my_data, mpu_mz_data);


        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(500);
    }
}
