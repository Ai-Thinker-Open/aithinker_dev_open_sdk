/**
 * @file     ln_mpu9250_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-03-31
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
/*
        MPU9020使用说明：

                    1. 因为本测试使用的是I2C接口，所以接线和函数调用请参考ln_i2c_test()函数。
*/


 

#include "ln_mpu9250_test.h"

#include "ln_test_common.h" 

//初始化MPU9250
//返回值:0,成功
//    其他,错误代码
uint8_t ln_mpu_init(void)
{
    uint8_t res=0;
    //IIC_Init();     //初始化IIC总线
    ln_mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X80);//复位MPU9250
    ln_delay_ms(100);  //延时100ms
    ln_mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X00);//唤醒MPU9250
    ln_mpu_set_gyro_fsr(3);                                //陀螺仪传感器,±2000dps
    ln_mpu_set_accel_fsr(0);                               //加速度传感器,±2g
    ln_mpu_set_rate(50);                                   //设置采样率50Hz
    ln_mpu_write_byte(MPU9250_ADDR,MPU_INT_EN_REG,0X00);   //关闭所有中断
    ln_mpu_write_byte(MPU9250_ADDR,MPU_USER_CTRL_REG,0X00);//I2C主模式关闭
    ln_delay_ms(100);  //延时100ms
    ln_mpu_write_byte(MPU9250_ADDR,MPU_INTBP_CFG_REG,0X02);//INT引脚低电平有效，开启bypass模式，可以直接读取磁力计
    ln_delay_ms(100);  //延时100ms
    ln_mpu_write_byte(MPU9250_ADDR,MPU_FIFO_EN_REG,0X00);  //关闭FIFO
    
    res=ln_mpu_read_byte(MPU9250_ADDR,MPU_DEVICE_ID_REG);  //读取MPU6500的ID
    if(res==MPU6500_ID1||res==MPU6500_ID2) //器件ID正确
    {
        ln_mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X01);      //设置CLKSEL,PLL X轴为参考
        ln_mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT2_REG,0X00);      //加速度与陀螺仪都工作
        ln_mpu_set_rate(50);                                         //设置采样率为50Hz   
    }
    else 
    {
        return 0;
    }
 
    res=ln_mpu_read_byte(AK8963_ADDR,MAG_WIA);                //读取AK8963 ID   
    if(res==AK8963_ID)
    {
        ln_mpu_write_byte(AK8963_ADDR,MAG_CNTL2,0X01);        //复位AK8963
        ln_delay_ms(50);
        ln_mpu_write_byte(AK8963_ADDR,MAG_CNTL1,0X11);        //设置AK8963为单次测量
        ln_delay_ms(50);
    }
    else 
    {
        return 0;
    }
    return 1;
}

//设置MPU9250陀螺仪传感器满量程范围
//fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败 
uint8_t ln_mpu_set_gyro_fsr(uint8_t fsr)
{
    return ln_mpu_write_byte(MPU9250_ADDR,MPU_GYRO_CFG_REG,(fsr<<3)|3);//设置陀螺仪满量程范围  
}
//设置MPU9250加速度传感器满量程范围
//fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败 
uint8_t ln_mpu_set_accel_fsr(uint8_t fsr)
{
    return ln_mpu_write_byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}

//设置MPU9250的数字低通滤波器
//lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败 
uint8_t MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6; 
    return ln_mpu_write_byte(MPU9250_ADDR,MPU_CFG_REG,data);//设置数字低通滤波器  
}

//设置MPU9250的采样率(假定Fs=1KHz)
//rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败 
uint8_t ln_mpu_set_rate(uint16_t rate)
{
    uint8_t data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=ln_mpu_write_byte(MPU9250_ADDR,MPU_SAMPLE_RATE_REG,data);    //设置数字低通滤波器
     return MPU_Set_LPF(rate/2);    //自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
short ln_mpu_get_temperature(void)
{
    uint8_t buf[2]; 
    short raw;
    float temp;
    ln_mpu_read_len(MPU9250_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((uint16_t)buf[0]<<8)|buf[1];  
    temp=21+((double)raw)/333.87;  
    return temp*100;;
}
//得到陀螺仪值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t ln_mpu_get_gyroscope(short *gx,short *gy,short *gz)
{
    uint8_t buf[6],res; 
    res=ln_mpu_read_len(MPU9250_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
    if(res==1)
    {
        *gx=((uint16_t)buf[0]<<8)|buf[1];  
        *gy=((uint16_t)buf[2]<<8)|buf[3];  
        *gz=((uint16_t)buf[4]<<8)|buf[5];
    }     
    return res;;
}
//得到加速度值(原始值)
//gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t ln_mpu_get_accelerometer(short *ax,short *ay,short *az)
{
    uint8_t buf[6],res;  
    res=ln_mpu_read_len(MPU9250_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==1)
    {
        *ax=((uint16_t)buf[0]<<8)|buf[1];  
        *ay=((uint16_t)buf[2]<<8)|buf[3];  
        *az=((uint16_t)buf[4]<<8)|buf[5];
    }     
    return res;;
}

//得到磁力计值(原始值)
//mx,my,mz:磁力计x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t ln_mpu_get_magnetometer(short *mx,short *my,short *mz)
{
    uint8_t buf[6],res; 
    res=ln_mpu_read_len(AK8963_ADDR,MAG_XOUT_L,6,buf);
    if(res==1)
    {
        *mx=((uint16_t)buf[1]<<8)|buf[0];  
        *my=((uint16_t)buf[3]<<8)|buf[2];  
        *mz=((uint16_t)buf[5]<<8)|buf[4];
    }      
    ln_mpu_write_byte(AK8963_ADDR,MAG_CNTL1,0X11); //AK8963每次读完以后都需要重新设置为单次测量模式
    return res;;
}

//IIC连续写
//addr:器件地址 
//reg:寄存器地址
//len:写入长度
//buf:数据区
//返回值:0,正常
//    其他,错误代码
uint8_t ln_mpu_write_len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    if(hal_i2c_master_7bit_write(I2C_BASE,addr,buf,len) == 0)
        return 0;
    else
        return 1;
} 

//IIC连续读
//addr:器件地址
//reg:要读取的寄存器地址
//len:要读取的长度
//buf:读取到的数据存储区
//返回值:0,正常
//    其他,错误代码
uint8_t ln_mpu_read_len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{   
    if(hal_i2c_master_7bit_read_target_address(I2C_BASE,addr,&reg,1,buf,len) == 0)
        return 0; 
    else
        return 1;
}

//IIC写一个字节 
//devaddr:器件IIC地址
//reg:寄存器地址
//data:数据
//返回值:0,正常
//    其他,错误代码
uint8_t ln_mpu_write_byte(uint8_t addr,uint8_t reg,uint8_t data)
{

    if(hal_i2c_master_7bit_write_target_address(I2C_BASE,addr,&reg,1,&data,1) == 0)
        return 0;
    else
        return 1;
}

//IIC读一个字节 
//reg:寄存器地址 
//返回值:读到的数据
uint8_t ln_mpu_read_byte(uint8_t addr,uint8_t reg)
{
    unsigned char res = 0;
    return hal_i2c_master_7bit_read_target_address(I2C_BASE,addr,&reg,1,&res,1); 
}
