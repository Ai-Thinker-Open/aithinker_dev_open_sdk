/**
 * @file     ln_drv_24c04.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_24c04.h"

//读写24C04
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
int i2c_24c04_read(uint16_t word_addr, uint8_t *buf, uint16_t buf_len)
{
    uint8_t bufer = 0;

    //因为24C04一共有 32页 * 16 字节  数据，当想要读取大于 255地址的数据时，就必须通过增加设备地址来翻页，
    //地址0XA0的范围  0 - 255，地址0xA2的范围 256 - 512.
    if (word_addr + buf_len >= 256)
    {
        if (word_addr >= 256)
        {
            bufer = word_addr - 256;
            if (HAL_SET != i2c_master_7bit_read_target_address(AT24C04_ADDR_W | 0x02, &bufer, 1, buf, buf_len)){
                return HAL_RESET;
            }
        }
        else
        {
            bufer = word_addr;
            if (HAL_SET != i2c_master_7bit_read_target_address(AT24C04_ADDR_W, &bufer, 1, buf, 256 - word_addr)){
                return HAL_RESET;
            }
            bufer = 0;
            if (HAL_SET != i2c_master_7bit_read_target_address(AT24C04_ADDR_W | 0x02, &bufer, 1, buf + 256 - word_addr, buf_len - (256 - word_addr))){
                return HAL_RESET;
            }
        }
    }
    else
    {
        bufer = word_addr;
        if (HAL_SET != i2c_master_7bit_read_target_address(AT24C04_ADDR_W, &bufer, 1, buf, buf_len))
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
static int i2c_24c04_write_byte(uint16_t word_addr, uint8_t data)
{
    uint8_t buf[2];
    uint8_t drv_addr = AT24C04_ADDR_W;

    buf[0] = word_addr & 0xFF;
    buf[1] = data;
    //因为24C04一共有 32页 * 16 字节  数据，当想要写入大于 255地址的数据时，就必须通过增加设备地址来翻页，
    //地址0XA0的范围  0 - 255，地址0xA2的范围 256 - 512.
    if (word_addr > 255)
        drv_addr = (AT24C04_ADDR_W | 0x02);
    if (HAL_SET != i2c_master_7bit_write_target_address(drv_addr, buf, 1, buf + 1, 1)){
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
int i2c_24c04_write(uint16_t word_addr, const uint8_t *buf, uint16_t buf_len)
{
    uint16_t i = 0;
    for (i = 0; i < buf_len; i++)
    {
        if (HAL_SET != i2c_24c04_write_byte(word_addr + i, buf[i]))
        {
            return HAL_RESET;
        }
    }
    return HAL_SET;
}

