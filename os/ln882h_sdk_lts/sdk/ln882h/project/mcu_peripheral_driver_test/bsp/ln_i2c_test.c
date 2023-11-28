/**
 * @file     ln_i2c_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/**
    I2C使用说明：
                1. 本测试分为五部分。第一部分是扫描设备地址；第二部分是读写AT24C04；第三部分是写OLED屏；第四部分是读取SHT30温湿度传感器；第五部分是读取MPU9250数据；第六部分BMP280气压计

                2. AT24C04、OLED、SHT30、MPU9250是通过I2C挂载在一起的,I2C的SCL和SDA要接上拉电阻。接线如下：
                
                                                                 LN882H             OLED                AT24C04             SHT30               MPU9250            BMP280
                                                                
                                                                 PB8         ->     D0          ->      SCL         ->      SCL         ->      SCL        ->      SCL 
                                                                 PB9         ->     D1          ->      SDA         ->      SDA         ->      SDA        ->      SDA 
                                                                 3.3v        ->     3.3V        ->      3.3V        ->      3.3V        ->      3.3V       ->      3.3V
                                                                 GND         ->     GND         ->      GND         ->      GND         ->      GND        ->      GND
                                                                 PB4         ->     RES         ->      
                                                                 GND         ->     CS          ->      WP
                                                                 GND         ->     GND         ->      A0
                                                                 GND         ->     GND         ->      A1
                                                                 GND         ->     GND         ->      A2      

                3. I2C波特率设置：      快速模式下：400k SCL周期为2.5us，假设外设工作时钟为40MHZ 所以Tpckl = 1/40M = 0.025us
                
                                        如果duty = 0;
                                        fs = 1;
                                                Thigh = CCR × TPCLK1 
                                                Tlow = 2 × CCR × TPCLK1
                                                Thigh + Tlow = 2.5us
                                                所以 3 * CCR × TPCLK1 = 2.5us  CCR = 33      
                                        
                                        如果duty = 1;
                                        fs = 1;
                                                Thigh = 9 * CCR × TPCLK1 
                                                Tlow = 16 × CCR × TPCLK1
                                                Thigh + Tlow = 2.5us
                                                所以 3 * CCR × TPCLK1 = 2.5us  CCR = 4

                                        duty    ->      I2C_CCR->duty
                                        fs      ->      I2C_CCR->fs
                                        Thigh   ->      I2C_CLK高电平时间
                                        Tlow    ->      I2C_CLK低电平时间
                                        TPCLK1  ->      外设时钟    
                                        CCR     ->      I2C_CCR->ccr
*/

#include "ln_i2c_test.h"
#include "hal/hal_gpio.h"
#include "ln_mpu9250_test.h"
#include "ln_oled_test.h"
#include "ln_sht30_test.h"
#include "ln_test_common.h"
#include "utils/debug/log.h"

#define TIMEOUT_CYCLE 4000

/**************************************I2C INIT START********************************************************/
void ln_i2c_init()
{
    hal_gpio_pin_afio_select(GPIOB_BASE, GPIO_PIN_8, I2C0_SCL); //初始化I2C引脚
    hal_gpio_pin_afio_select(GPIOB_BASE, GPIO_PIN_9, I2C0_SDA);
    hal_gpio_pin_afio_en(GPIOB_BASE, GPIO_PIN_8, HAL_ENABLE); //使能功能复用
    hal_gpio_pin_afio_en(GPIOB_BASE, GPIO_PIN_9, HAL_ENABLE);

    /* disable the i2c */
    hal_i2c_en(I2C_BASE, HAL_DISABLE); //配置参数前要先失能I2C

    /* init the i2c */
    i2c_init_t_def i2c_init;
    memset(&i2c_init, 0, sizeof(i2c_init));

    i2c_init.i2c_peripheral_clock_freq = 4;                     //时钟设置，默认4就可以（滤波使用）
    i2c_init.i2c_master_mode_sel = I2C_FM_MODE;                 //fs   = 1
    i2c_init.i2c_fm_mode_duty_cycle = I2C_FM_MODE_DUTY_CYCLE_2; //duty = 1
    /*
        快速模式下：400k SCL周期为2.5us
        外设工作时钟为80MHZ 所以Tpckl = 1/80M = 0.0125us
        
        duty = 1;fs = 1
        Thigh = 9  x CCR x TPCLK1 
        Tlow  = 16 x CCR x TPCLK1
        Thigh + Tlow = 2.5us
        ccr =  8
    */
    i2c_init.i2c_ccr = 8;
    i2c_init.i2c_trise = 0xF; //推荐默认值为0xF;

    hal_i2c_init(I2C_BASE, &i2c_init);

    hal_i2c_en(I2C_BASE, HAL_ENABLE);
}

/**
 * @brief I2C 写数据（设备地址7bit）
 * 
 * @param i2c_x_base I2C寄存器基址
 * @param dev_addr   设备地址
 * @param buf        要写入的数据
 * @param buf_len    要写入数据的长度
 * @return uint8_t   是否写入成功，0：不成功，1：成功
 */
uint8_t hal_i2c_master_7bit_write(uint32_t i2c_x_base, uint8_t dev_addr, const uint8_t *buf, uint16_t buf_len)
{
    //1. check busy
    if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(i2c_x_base);
        return HAL_RESET;
    }

    //2. send start
    if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //3. send addr
    hal_i2c_master_send_data(i2c_x_base, dev_addr);

    //4. wait send complete
    if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //5. clear addr flag
    hal_i2c_clear_sr(i2c_x_base);

    //6. send data
    for (uint32_t i = 0; i < buf_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(i2c_x_base, buf[i]);
        }
    }

    //7. wait send complete.
    if (hal_i2c_wait_btf(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //8. stop the i2c.
    hal_i2c_master_stop(i2c_x_base);

    return HAL_SET;
}

/**
 * @brief I2C 写数据到目标设备的寄存器（设备地址7bit）
 * 
 * @param i2c_x_base      I2C寄存器基址
 * @param dev_addr        设备地址
 * @param target_addr     目标设备寄存器地址
 * @param target_addr_len 寄存器地址长度
 * @param buf             要写入的数据
 * @param buf_len         要写入数据的长度
 * @return uint8_t        是否写入成功，0：不成功，1：成功
 */
uint8_t hal_i2c_master_7bit_write_target_address(uint32_t i2c_x_base, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, const uint8_t *buf, uint16_t buf_len)
{
    //1. check busy
    if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(i2c_x_base);
        return HAL_RESET;
    }

    //2. send start
    if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //3. send addr
    hal_i2c_master_send_data(i2c_x_base, dev_addr);

    //4. wait send complete
    if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //5. clear addr flag
    hal_i2c_clear_sr(i2c_x_base);

    //6. send target address
    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(i2c_x_base, target_addr[i]);
        }
    }

    //7. wait send complete.
    if (hal_i2c_wait_btf(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //8. send data
    for (uint32_t i = 0; i < buf_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(i2c_x_base, buf[i]);
        }
    }

    //9. wait send complete.
    if (hal_i2c_wait_btf(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //10. stop the i2c.
    hal_i2c_master_stop(i2c_x_base);

    return HAL_SET;
}

/**
 * @brief I2C 读目标设备的寄存器（设备地址7bit）
 * 
 * @param i2c_x_base        I2C寄存器基址      
 * @param dev_addr          设备地址
 * @param target_addr       目标设备寄存器地址
 * @param target_addr_len   目标设备寄存器地址长度
 * @param buf               读取的数据
 * @param buf_len           读取的数据长度
 * @return uint8_t          是否读取成功，0：不成功，1：成功
 */
uint8_t hal_i2c_master_7bit_read_target_address(uint32_t i2c_x_base, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len)
{
    //1. check busy
    if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(i2c_x_base);
        return HAL_RESET;
    }

    //2. send start
    if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //3. send addr
    hal_i2c_master_send_data(i2c_x_base, dev_addr);

    //4. wait send complete
    if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //5. clear addr flag
    hal_i2c_clear_sr(i2c_x_base);

    //6. send target address
    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        //wait tx empty flag
        if (hal_i2c_wait_txe(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(i2c_x_base, target_addr[i]);
        }
    }

    //7. wait send complete.
    if (hal_i2c_wait_btf(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //8. reset the bus,if not do this,the bus can not run start operation
    hal_i2c_master_reset(i2c_x_base);

    //9. send start
    if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //10. send addr (+1 is read operation)
    hal_i2c_master_send_data(i2c_x_base, dev_addr + 1);

    //11. Wait for an ack after sending the address
    if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    //12. clear addr flag
    hal_i2c_clear_sr(i2c_x_base);

    //13. clear the DR
    hal_i2c_master_recv_data(i2c_x_base);

    //14. receive data
    for (int i = buf_len; i > 0; i--)
    {
        //when reading the last byte,do not send the ack
        if (buf_len == 1)
        {
            //do not send the ack
            hal_i2c_ack_en(i2c_x_base, HAL_DISABLE);

            ///wait rx not empty
            if (hal_i2c_wait_rxne(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(i2c_x_base);
            }
            //read data
        }
        else
        {
            //send ack
            hal_i2c_ack_en(i2c_x_base, HAL_ENABLE);

            //wait rx not empty
            if (hal_i2c_wait_rxne(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(i2c_x_base);
            }
        }
        buf_len--;
        buf++;
    }

    //15. stop the i2c.
    hal_i2c_master_stop(i2c_x_base);

    return HAL_SET;
}

/**************************************I2C INIT END********************************************************/

//第一部分，I2C扫描设备地址
/**************************************I2C SCAN START********************************************************/

static unsigned int address[128];    //存储扫描到的地址
static unsigned int address_num = 0; //扫描到的地址个数

void hal_i2c_address_scan(uint32_t i2c_x_base)
{
    hal_i2c_ack_en(i2c_x_base, HAL_DISABLE);

    //scan the iic address
    for (uint32_t dev_addr = 0; dev_addr <= 254; dev_addr = dev_addr + 2)
    {
        //1. check busy
        if (hal_i2c_wait_bus_idle(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
        {
            hal_i2c_master_reset(i2c_x_base);
            ln_delay_ms(200);
            continue;
        }

        //2. send start
        if (hal_i2c_master_start(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
            continue;

        //3. send addr
        hal_i2c_master_send_data(i2c_x_base, dev_addr);

        //4. wait send complete
        if (hal_i2c_master_wait_addr(i2c_x_base, TIMEOUT_CYCLE) == HAL_RESET)
            continue;
            
        ln_delay_ms(1);

        //5. check ack
        if (hal_i2c_get_status_flag(i2c_x_base,I2C_STATUS_FLAG_AF) == HAL_SET)
        {
            hal_i2c_master_reset(i2c_x_base);
            hal_i2c_clear_sr(i2c_x_base);
            hal_i2c_master_stop(i2c_x_base);
            ln_delay_ms(20);
            
        }
        else
        {
            hal_i2c_master_reset(i2c_x_base);
            hal_i2c_clear_sr(i2c_x_base);

            address[address_num] = dev_addr;
            address_num++;

            ln_delay_ms(20);
        }
    }
    // clear addr flag
    hal_i2c_clear_sr(i2c_x_base);

    hal_i2c_master_stop(i2c_x_base);
}

/**************************************I2C SCAN END********************************************************/

//第二部分，读写24C04
/**************************************I2C R/W AT24C04 START********************************************************/
#define AT24C04_ADDR_W 0xA0
#define AT24C04_WRITE_DELAY 50000

/**
 * @brief 读取24C04存储的数据
 * 
 * @param word_addr   要读取的位置
 * @param buf         读出的数据存放地址
 * @param buf_len     要读出数据的长度
 * @return int        返回读取的结果，0：失败  1：成功
 */
int hal_24c04_read(uint16_t word_addr, uint8_t *buf, uint16_t buf_len)
{
    uint8_t bufer = 0;

    //因为24C04一共有 32页 * 16 字节  数据，当想要读取大于 255地址的数据时，就必须通过增加设备地址来翻页，
    //地址0XA0的范围  0 - 255，地址0xA2的范围 256 - 512.
    if (word_addr + buf_len >= 256)
    {
        if (word_addr >= 256)
        {
            bufer = word_addr - 256;
            if (HAL_SET != hal_i2c_master_7bit_read_target_address(I2C_BASE, AT24C04_ADDR_W | 0x02, &bufer, 1, buf, buf_len)){
                return HAL_RESET;
            }
        }
        else
        {
            bufer = word_addr;
            if (HAL_SET != hal_i2c_master_7bit_read_target_address(I2C_BASE, AT24C04_ADDR_W, &bufer, 1, buf, 256 - word_addr)){
                return HAL_RESET;
            }
            bufer = 0;
            if (HAL_SET != hal_i2c_master_7bit_read_target_address(I2C_BASE, AT24C04_ADDR_W | 0x02, &bufer, 1, buf + 256 - word_addr, buf_len - (256 - word_addr))){
                return HAL_RESET;
            }
        }
    }
    else
    {
        bufer = word_addr;
        if (HAL_SET != hal_i2c_master_7bit_read_target_address(I2C_BASE, AT24C04_ADDR_W, &bufer, 1, buf, buf_len))
        {
            return HAL_RESET;
        }
    }
    return HAL_SET;
}

/**
 * @brief 写一个字节到24c04
 * 
 * @param word_addr  要写入的位置
 * @param data       要写入的数据
 * @return int       返回写入的结果，0：失败  1：成功
 */
static int hal_24c04_write_byte(uint16_t word_addr, uint8_t data)
{
    uint8_t buf[2];
    uint8_t drv_addr = AT24C04_ADDR_W;

    buf[0] = word_addr & 0xFF;
    buf[1] = data;
    //因为24C04一共有 32页 * 16 字节  数据，当想要写入大于 255地址的数据时，就必须通过增加设备地址来翻页，
    //地址0XA0的范围  0 - 255，地址0xA2的范围 256 - 512.
    if (word_addr > 255)
        drv_addr = (AT24C04_ADDR_W | 0x02);
    if (HAL_SET != hal_i2c_master_7bit_write_target_address(I2C_BASE, drv_addr, buf, 1, buf + 1, 1)){
        return HAL_RESET;
    }
    for (volatile uint32_t i = 0; i < AT24C04_WRITE_DELAY; i++){}; //max 5ms

    return HAL_SET;
}

/**
 * @brief 写数据到24c04
 * 
 * @param word_addr 要写入的位置
 * @param buf       要写入的数据
 * @param buf_len   返回写入的结果，0：失败  1：成功
 * @return int 
 */
int hal_24c04_write(uint16_t word_addr, const uint8_t *buf, uint16_t buf_len)
{
    uint16_t i = 0;
    for (i = 0; i < buf_len; i++)
    {
        if (HAL_SET != hal_24c04_write_byte(word_addr + i, buf[i]))
        {
            return HAL_RESET;
        }
    }
    return HAL_SET;
}

/**************************************I2C R/W AT24C04 END********************************************************/

//第三部分，写OLED屏
/**************************************I2C W OLED START********************************************************/
unsigned char cmd_arr[2];
unsigned char data_arr[2];
#define OLED_ADDR 0x78
void hal_write_command_byte(uint32_t base, uint8_t cmd)
{
    cmd_arr[0] = 0x00;
    cmd_arr[1] = cmd;
    hal_i2c_master_7bit_write(base, OLED_ADDR, cmd_arr, 2);
};
void hal_write_data_byte(uint32_t base, uint8_t data)
{
    data_arr[0] = 0x40;
    data_arr[1] = data;
    hal_i2c_master_7bit_write(base, OLED_ADDR, data_arr, 2);
};

/**************************************I2C W OLED END********************************************************/

//第四部分，读取SHT30温湿度传感器
/**************************************I2C SHT30 INIT********************************************************/
unsigned char crc8_checksum(unsigned char *ptr, unsigned char len)
{
    unsigned char bit;        // bit mask
    unsigned char crc = 0xFF; // calculated checksum
    unsigned char byteCtr;    // byte counter

    // calculates 8-Bit checksum with given polynomial
    for (byteCtr = 0; byteCtr < len; byteCtr++)
    {
        crc ^= (ptr[byteCtr]);
        for (bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x131;
            else
                crc = (crc << 1);
        }
    }

    return crc;
}

unsigned char ln_tem_hum_init()
{
    unsigned char data[2];
    unsigned char res = 0;
    data[0] = ((CMD_MEAS_PERI_2_M >> 8) & 0x00FF);
    data[1] = ((CMD_MEAS_PERI_2_M >> 0) & 0x00FF);
    res = hal_i2c_master_7bit_write(I2C_BASE, TEM_HUM_ADDR, data, 2);
    return res;
}

unsigned char tem_hum_read_sensor_data(unsigned char *data)
{
    unsigned char addr_data[10];
    addr_data[0] = ((CMD_FETCH_DATA >> 8) & 0x00FF);
    addr_data[1] = ((CMD_FETCH_DATA >> 0) & 0x00FF);
    hal_i2c_master_7bit_read_target_address(I2C_BASE, TEM_HUM_ADDR, addr_data, 2, data, 6);

    if (data[2] != crc8_checksum(data, 2))
    {
        return 1;
    }

    if (data[5] != crc8_checksum(data + 3, 2))
    {
        return 1;
    }

    return 0;
}

/**************************************I2C SHT30 END********************************************************/

//第五部分，读写九轴陀螺仪
//该部分代码在ln_mpu_9250中

/**************************************I2C BMP280 START********************************************************/
//第六部分，读取BMP280气压计数据

#define BMP280_I2C_ADDR 0xEC
#define BMP280_ID_ADDR 0XD0
#define BMP280_CTRL_MEAS_ADDR 0XF4
#define BMP280_PRESS_DATA_ADDR 0XF7
#define BMP280_TEM_DATA_ADDR 0XFA

/*******************************下面是用来计算补偿值相关**********************************/
typedef long signed int BMP280_S32_t;   //有符号 64位！
typedef long unsigned int BMP280_U32_t; //无符号 32位！
typedef long long signed int BMP280_S64_t;

typedef struct
{
    /* T1~P9 为补偿系数 */
    uint16_t T1;
    int16_t T2;
    int16_t T3;
    uint16_t P1;
    int16_t P2;
    int16_t P3;
    int16_t P4;
    int16_t P5;
    int16_t P6;
    int16_t P7;
    int16_t P8;
    int16_t P9;
} BMP280_t;

BMP280_t BMP280;

#define dig_T1 BMP280.T1
#define dig_T2 BMP280.T2
#define dig_T3 BMP280.T3

#define dig_P1 BMP280.P1
#define dig_P2 BMP280.P2
#define dig_P3 BMP280.P3
#define dig_P4 BMP280.P4
#define dig_P5 BMP280.P5
#define dig_P6 BMP280.P6
#define dig_P7 BMP280.P7
#define dig_P8 BMP280.P8
#define dig_P9 BMP280.P9

volatile double pressure_data = 0;        //压力数据
volatile double pressure_tem_data = 0;    //压力传感器的温度数据
static BMP280_S32_t pressure_adc = 0;     //压力ADC数据
static BMP280_S32_t pressure_tem_adc = 0; //压力传感器的温度ADC数据

/**************************传感器值转定点值*************************************/
BMP280_S32_t t_fine; //用于计算补偿
//我用浮点补偿
#ifdef USE_FIXED_POINT_COMPENSATE
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BMP280_S32_t bmp280_compensate_T_int32(BMP280_S32_t adc_T)
{
    BMP280_S32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((BMP280_S32_t)dig_T1 << 1))) * ((BMP280_S32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((BMP280_S32_t)dig_T1)) * ((adc_T >> 4) - ((BMP280_S32_t)dig_T1))) >> 12) *
            ((BMP280_S32_t)dig_T3)) >>
           14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa
BMP280_U32_t bmp280_compensate_P_int64(BMP280_S32_t adc_P)
{
    BMP280_S64_t var1, var2, p;
    var1 = ((BMP280_S64_t)t_fine) - 128000;
    var2 = var1 * var1 * (BMP280_S64_t)dig_P6;
    var2 = var2 + ((var1 * (BMP280_S64_t)dig_P5) << 17);
    var2 = var2 + (((BMP280_S64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (BMP280_S64_t)dig_P3) >> 8) + ((var1 * (BMP280_S64_t)dig_P2) << 12);
    var1 = (((((BMP280_S64_t)1) << 47) + var1)) * ((BMP280_S64_t)dig_P1) >> 33;
    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((BMP280_S64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((BMP280_S64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((BMP280_S64_t)dig_P7) << 4);
    return (BMP280_U32_t)p;
}

/***********************************CUT*************************************/
#else
/**************************传感器值转定点值*************************************/
// Returns temperature in DegC, double precision. Output value of “51.23” equals 51.23 DegC.
// t_fine carries fine temperature as global value
double bmp280_compensate_T_double(BMP280_S32_t adc_T)
{
    double var1, var2, T;
    var1 = (((double)adc_T) / 16384.0 - ((double)dig_T1) / 1024.0) * ((double)dig_T2);
    var2 = ((((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0) *
            (((double)adc_T) / 131072.0 - ((double)dig_T1) / 8192.0)) *
           ((double)dig_T3);
    t_fine = (BMP280_S32_t)(var1 + var2);
    T = (var1 + var2) / 5120.0;
    return T;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double bmp280_compensate_P_double(BMP280_S32_t adc_P)
{
    double var1, var2, p;
    var1 = ((double)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((double)dig_P6) / 32768.0;
    var2 = var2 + var1 * ((double)dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((double)dig_P4) * 65536.0);
    var1 = (((double)dig_P3) * var1 * var1 / 524288.0 + ((double)dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((double)dig_P1);
    if (var1 == 0.0)
    {
        return 0; // avoid exception caused by division by zero
    }
    p = 1048576.0 - (double)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((double)dig_P9) * p * p / 2147483648.0;
    var2 = p * ((double)dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((double)dig_P7)) / 16.0;
    return p;
}
#endif

unsigned char pressure_init()
{
    unsigned char addr_data[1];
    unsigned char data[30];
    unsigned char res = 0;

    addr_data[0] = BMP280_ID_ADDR;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 1);

    data[0] = 0xB7;
    addr_data[0] = BMP280_CTRL_MEAS_ADDR;
    hal_i2c_master_7bit_write_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 1);

    addr_data[0] = BMP280_PRESS_DATA_ADDR;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 3);
    pressure_adc = (data[0] << 12) + (data[1] << 4) + ((data[2] & 0xF0) >> 4);

    addr_data[0] = BMP280_TEM_DATA_ADDR;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 3);
    pressure_tem_adc = (data[0] << 12) + (data[1] << 4) + ((data[2] & 0xF0) >> 4);

    addr_data[0] = 0x88;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 26);

    BMP280.T1 = *(uint16_t *)(data);
    BMP280.T2 = *(int16_t *)(data + 2);
    BMP280.T3 = *(int16_t *)(data + 4);

    BMP280.P1 = *(uint16_t *)(data + 6);
    BMP280.P2 = *(int16_t *)(data + 8);
    BMP280.P3 = *(int16_t *)(data + 10);
    BMP280.P4 = *(int16_t *)(data + 12);
    BMP280.P5 = *(int16_t *)(data + 14);
    BMP280.P6 = *(int16_t *)(data + 16);
    BMP280.P7 = *(int16_t *)(data + 18);
    BMP280.P8 = *(int16_t *)(data + 20);
    BMP280.P9 = *(int16_t *)(data + 22);

    pressure_data = bmp280_compensate_T_double(pressure_tem_adc);

    pressure_tem_data = bmp280_compensate_P_double(pressure_adc);

    return res;
}

unsigned char pressure_read_sensor_data(void)
{
    unsigned char addr_data[1];
    unsigned char data[30];
    unsigned char res = 0;

    addr_data[0] = BMP280_PRESS_DATA_ADDR;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 3);
    pressure_adc = (data[0] << 12) + (data[1] << 4) + ((data[2] & 0xF0) >> 4);

    addr_data[0] = BMP280_TEM_DATA_ADDR;
    hal_i2c_master_7bit_read_target_address(I2C_BASE, BMP280_I2C_ADDR, addr_data, 1, data, 3);
    pressure_tem_adc = (data[0] << 12) + (data[1] << 4) + ((data[2] & 0xF0) >> 4);

    pressure_data = bmp280_compensate_T_double(pressure_tem_adc);

    pressure_tem_data = bmp280_compensate_P_double(pressure_adc);

    return res;
}

/**************************************I2C BMP280 END********************************************************/

/*******************************************************************************************************/

static unsigned char tx_data[100]; //测试BUFFER
static unsigned char rx_data[100];
static unsigned char sensor_data[6];
static unsigned int err_cnt = 0;

static volatile double tem_data = 0; //温度数据
static volatile double hum_data = 0; //湿度数据

static volatile double mpu_tem_data = 0;

static short mpu_gx_data = 0; //角速度信息
static short mpu_gy_data = 0;
static short mpu_gz_data = 0;

static short mpu_ax_data = 0; //加速度信息
static short mpu_ay_data = 0;
static short mpu_az_data = 0;

static short mpu_mx_data = 0; //磁力计信息
static short mpu_my_data = 0;
static short mpu_mz_data = 0;

void ln_i2c_test()
{
    // I2C初始化
    ln_i2c_init();

    /*************** 1. 开始扫描地址 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c scan start! \n");

    hal_i2c_address_scan(I2C_BASE);

    if (address_num == 0)
    {
        LOG(LOG_LVL_INFO, "There is no device on I2C bus! \n");
    }
    else
    {
        for (int i = address_num; i > 0; i--)
        {
            LOG(LOG_LVL_INFO, "Found device %d,address : 0x%x! \n", address_num - i, address[address_num - i]);
        }
        LOG(LOG_LVL_INFO, "Scan end. \n");
    }

    for (int i = 0; i < 100; i++)
    {
        tx_data[i] = i;
    }

    /*************** 2. 开始读写AT24C04 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c + 24c04 test start! \n");

    hal_24c04_write(256, tx_data, 100);
    hal_24c04_read(256, rx_data, 100);

    for (int i = 0; i < 100; i++)
    {
        if (tx_data[i] != rx_data[i])
        {
            err_cnt++;
        }
    }
    if (err_cnt != 0)
        LOG(LOG_LVL_INFO, "LN882H i2c + 24c04 test fail! \n");
    else
        LOG(LOG_LVL_INFO, "LN882H i2c + 24c04 test success! \n");

    

    /*************** 3. 开始控制OLED显示屏 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c + oled test start! \n");
    gpio_init_t_def gpio_init;                  //配置OLED RST引脚
    memset(&gpio_init, 0, sizeof(gpio_init));   //清零结构体
    gpio_init.dir = GPIO_OUTPUT;                //配置GPIO方向，输入或者输出
    gpio_init.pin = GPIO_PIN_4;                 //配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED;          //设置GPIO速度
    hal_gpio_init(GPIOB_BASE, &gpio_init);      //初始化GPIO
    hal_gpio_pin_reset(GPIOB_BASE, GPIO_PIN_4); //RESET OLED屏幕
    ln_delay_ms(100);       
    hal_gpio_pin_set(GPIOB_BASE, GPIO_PIN_4);

    ln_oled_init(I2C_BASE);
    ln_oled_display_on();

    ln_oled_show_string(0, 40, (unsigned char *)"Test success!", 12); //显示测试成功的英文
    LOG(LOG_LVL_INFO, "LN882H i2c + oled test success! \n");

    ln_oled_refresh(); //刷新显示
    ln_delay_ms(1000);
    ln_oled_clear();

    /*************** 4. 初始化温湿度传感器 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c + sht30 test start! \n");
    ln_tem_hum_init();

    /*************** 5. 初始化九轴传感器 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c + MPU9250 test start! \n");
    ln_mpu_init();

    /*************** 6. 初始化BMP280气压计 *****************/
    LOG(LOG_LVL_INFO, "LN882H i2c + BMP280 test start! \n");
    pressure_init();

    while (1)
    {
        unsigned char display_buffer[16];

        // 1. 获得温湿度信息
        if (tem_hum_read_sensor_data(sensor_data) == 0) 
        {

            memset(display_buffer, ' ', 16);
            tem_data = -45 + (175 * (sensor_data[0] * 256 + sensor_data[1]) * 1.0 / (65535 - 1)); //转换为摄氏度
            hum_data = 100 * ((sensor_data[3] * 256 + sensor_data[4]) * 1.0 / (65535 - 1));       //转换为%

            sprintf((char *)display_buffer, "TEM : %0.2f", tem_data); //oled显示和串口打印数据
            ln_oled_show_string_with_len(0, 32, display_buffer, 16, 15);
            memset(display_buffer, ' ', 16);
            sprintf((char *)display_buffer, "HUM : %0.2f", hum_data);
            ln_oled_show_string_with_len(0, 48, display_buffer, 16, 15);
        }

        // 2. 更新压力计信息
        pressure_read_sensor_data(); 

        // 3. 更新九轴传感器信息
        mpu_tem_data = ln_mpu_get_temperature();
        ln_mpu_get_gyroscope(&mpu_gx_data, &mpu_gy_data, &mpu_gz_data);     //获取角速度信息
        ln_mpu_get_accelerometer(&mpu_ax_data, &mpu_ay_data, &mpu_az_data); //或者加速度信息
        ln_mpu_get_magnetometer(&mpu_mx_data, &mpu_my_data, &mpu_mz_data);  //获取磁力计信息

        // 4. 显示和打印LOG。
        memset(display_buffer, ' ', 16);
        sprintf((char *)display_buffer, "x:%0.1f", mpu_ax_data * 1.0 * 0.005625);
        ln_oled_show_string_with_len(0, 0, display_buffer, 16, 8);

        memset(display_buffer, ' ', 16);
        sprintf((char *)display_buffer, "y:%0.1f", mpu_ay_data * 1.0 * 0.005625);
        ln_oled_show_string_with_len(64, 0, display_buffer, 16, 7);

        memset(display_buffer, ' ', 16);
        sprintf((char *)display_buffer, "z:%0.1f", mpu_az_data * 1.0 * 0.005625);
        ln_oled_show_string_with_len(0, 16, display_buffer, 16, 8);

        LOG(LOG_LVL_INFO, "Tem = %0.2f  ", tem_data);
        LOG(LOG_LVL_INFO, "Hum = %0.2f%\n", hum_data);

        LOG(LOG_LVL_INFO, "Pressure = %0.2f  ", pressure_data);
        LOG(LOG_LVL_INFO, "Pressure Tem = %0.2f  \n", pressure_tem_data);

        LOG(LOG_LVL_INFO, "ACC:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n", mpu_ax_data, mpu_ay_data, mpu_az_data);
        LOG(LOG_LVL_INFO, "GYR:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n", mpu_gx_data, mpu_gy_data, mpu_gz_data);
        LOG(LOG_LVL_INFO, "MAG:  X: %0.3d    Y: %0.3d    Z: %0.3d  \n\n", mpu_mx_data, mpu_my_data, mpu_mz_data);

        ln_oled_refresh();  //刷新OLED显示

        ln_delay_ms(30);
    }
}
