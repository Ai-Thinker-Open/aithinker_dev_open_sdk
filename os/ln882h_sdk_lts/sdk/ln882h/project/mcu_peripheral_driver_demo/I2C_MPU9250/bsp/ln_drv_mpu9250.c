/**
 * @file     ln_drv_mpu9250.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_mpu9250.h"

/**
 * @brief MPU9250 初始化
 * 
 * @return uint8_t 0,成功，1：未成功
 */
uint8_t mpu_init(void)
{
    uint8_t res=0;
    //IIC_Init();     //初始化IIC总线
    mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X80);//复位MPU9250
    ln_delay_ms(100);  //延时100ms
    mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X00);//唤醒MPU9250
    mpu_set_gyro_fsr(3);                                //陀螺仪传感器,±2000dps
    mpu_set_accel_fsr(0);                               //加速度传感器,±2g
    mpu_set_rate(50);                                   //设置采样率50Hz
    mpu_write_byte(MPU9250_ADDR,MPU_INT_EN_REG,0X00);   //关闭所有中断
    mpu_write_byte(MPU9250_ADDR,MPU_USER_CTRL_REG,0X00);//I2C主模式关闭
    ln_delay_ms(100);  //延时100ms
    mpu_write_byte(MPU9250_ADDR,MPU_INTBP_CFG_REG,0X02);//INT引脚低电平有效，开启bypass模式，可以直接读取磁力计
    ln_delay_ms(100);  //延时100ms
    mpu_write_byte(MPU9250_ADDR,MPU_FIFO_EN_REG,0X00);  //关闭FIFO
    
    res=mpu_read_byte(MPU9250_ADDR,MPU_DEVICE_ID_REG);  //读取MPU6500的ID
    if(res==MPU6500_ID1||res==MPU6500_ID2) //器件ID正确
    {
        mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT1_REG,0X01);      //设置CLKSEL,PLL X轴为参考
        mpu_write_byte(MPU9250_ADDR,MPU_PWR_MGMT2_REG,0X00);      //加速度与陀螺仪都工作
        mpu_set_rate(50);                                         //设置采样率为50Hz   
    }
    else 
    {
        return 0;
    }
 
    res=mpu_read_byte(AK8963_ADDR,MAG_WIA);                //读取AK8963 ID   
    if(res==AK8963_ID)
    {
        mpu_write_byte(AK8963_ADDR,MAG_CNTL2,0X01);        //复位AK8963
        ln_delay_ms(50);
        mpu_write_byte(AK8963_ADDR,MAG_CNTL1,0X11);        //设置AK8963为单次测量
        ln_delay_ms(50);
    }
    else 
    {
        return 0;
    }
    return 1;
}

/**
 * @brief 设置MPU9250陀螺仪传感器满量程范围
 * 
 * @param fsr 0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
 * @return uint8_t 0,成功，1：未成功
 */
uint8_t mpu_set_gyro_fsr(uint8_t fsr)
{
    return mpu_write_byte(MPU9250_ADDR,MPU_GYRO_CFG_REG,(fsr<<3)|3);//设置陀螺仪满量程范围  
}

/**
 * @brief 设置MPU9250加速度传感器满量程范围
 * 
 * @param fsr 0,±2g;1,±4g;2,±8g;3,±16g
 * @return uint8_t 0,成功，1：未成功
 */
uint8_t mpu_set_accel_fsr(uint8_t fsr)
{
    return mpu_write_byte(MPU9250_ADDR,MPU_ACCEL_CFG_REG,fsr<<3);//设置加速度传感器满量程范围  
}

/**
 * @brief 设置MPU9250的数字低通滤波器
 * 
 * @param lpf 数字低通滤波频率(Hz)
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t MPU_Set_LPF(uint16_t lpf)
{
    uint8_t data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6; 
    return mpu_write_byte(MPU9250_ADDR,MPU_CFG_REG,data);//设置数字低通滤波器  
}

/**
 * @brief 设置MPU9250的采样率(假定Fs=1KHz)
 * 
 * @param rate 4~1000(Hz)
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_set_rate(uint16_t rate)
{
    uint8_t data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=mpu_write_byte(MPU9250_ADDR,MPU_SAMPLE_RATE_REG,data);    //设置数字低通滤波器
     return MPU_Set_LPF(rate/2);    //自动设置LPF为采样率的一半
}

/**
 * @brief 得到温度值
 * 
 * @return short 温度值(扩大了100倍)
 */
short mpu_get_temperature(void)
{
    uint8_t buf[2]; 
    short raw;
    float temp;
    mpu_read_len(MPU9250_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((uint16_t)buf[0]<<8)|buf[1];  
    temp=21+((double)raw)/333.87;  
    return temp*100;;
}

/**
 * @brief 得到陀螺仪值(原始值)
 * 
 * @param gx 陀螺仪x轴的原始读数(带符号)
 * @param gy 陀螺仪y轴的原始读数(带符号)
 * @param gz 陀螺仪z轴的原始读数(带符号)
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_get_gyroscope(short *gx,short *gy,short *gz)
{
    uint8_t buf[6],res; 
    res=mpu_read_len(MPU9250_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
    if(res==1)
    {
        *gx=((uint16_t)buf[0]<<8)|buf[1];  
        *gy=((uint16_t)buf[2]<<8)|buf[3];  
        *gz=((uint16_t)buf[4]<<8)|buf[5];
    }     
    return res;;
}

/**
 * @brief 得到加速度值(原始值)
 * 
 * @param ax 陀螺仪x轴的原始读数(带符号)
 * @param ay 陀螺仪y轴的原始读数(带符号)
 * @param az 陀螺仪z轴的原始读数(带符号)
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_get_accelerometer(short *ax,short *ay,short *az)
{
    uint8_t buf[6],res;  
    res=mpu_read_len(MPU9250_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==1)
    {
        *ax=((uint16_t)buf[0]<<8)|buf[1];  
        *ay=((uint16_t)buf[2]<<8)|buf[3];  
        *az=((uint16_t)buf[4]<<8)|buf[5];
    }     
    return res;;
}

/**
 * @brief 得到磁力计值(原始值)
 * 
 * @param mx 磁力计x轴的原始读数(带符号)
 * @param my 磁力计y轴的原始读数(带符号)
 * @param mz 磁力计z轴的原始读数(带符号)
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_get_magnetometer(short *mx,short *my,short *mz)
{
    uint8_t buf[6],res; 
    res=mpu_read_len(AK8963_ADDR,MAG_XOUT_L,6,buf);
    if(res==1)
    {
        *mx=((uint16_t)buf[1]<<8)|buf[0];  
        *my=((uint16_t)buf[3]<<8)|buf[2];  
        *mz=((uint16_t)buf[5]<<8)|buf[4];
    }      
    mpu_write_byte(AK8963_ADDR,MAG_CNTL1,0X11); //AK8963每次读完以后都需要重新设置为单次测量模式
    return res;;
}

/**
 * @brief IIC连续写
 * 
 * @param addr 器件地址
 * @param reg  寄存器地址
 * @param len  写入长度
 * @param buf  数据区
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_write_len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{
    if(i2c_master_7bit_write(addr,buf,len) == 0)
        return 0;
    else
        return 1;
} 

/**
 * @brief IIC连续读
 * 
 * @param addr 器件地址
 * @param reg  要读取的寄存器地址
 * @param len  要读取的长度
 * @param buf  取到的数据存储区
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_read_len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
{   
    if(i2c_master_7bit_read_target_address(addr,&reg,1,buf,len) == 0)
        return 0; 
    else
        return 1;
}

/**
 * @brief IIC写一个字节 
 * 
 * @param addr 器件IIC地址
 * @param reg  寄存器地址
 * @param data 数据
 * @return uint8_t 0,成功，1：未成功 
 */
uint8_t mpu_write_byte(uint8_t addr,uint8_t reg,uint8_t data)
{

    if(i2c_master_7bit_write_target_address(addr,&reg,1,&data,1) == 0)
        return 0;
    else
        return 1;
}

/**
 * @brief IIC读一个字节 
 * 
 * @param addr 设备地址
 * @param reg  寄存器地址 
 * @return uint8_t 读到的数据
 */
uint8_t mpu_read_byte(uint8_t addr,uint8_t reg)
{
    unsigned char res = 0;
    return i2c_master_7bit_read_target_address(addr,&reg,1,&res,1); 
}
