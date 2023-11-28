/**
 * @file     ln_drv_sht30.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_sht30.h"

uint8_t crc8_checksum(uint8_t *ptr, uint8_t len)
{
    uint8_t bit;        // bit mask
    uint8_t crc = 0xFF; // calculated checksum
    uint8_t byteCtr;    // byte counter

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

uint8_t ln_tem_hum_init()
{
    uint8_t data[2];
    uint8_t res = 0;
    data[0] = ((CMD_MEAS_PERI_2_M >> 8) & 0x00FF);
    data[1] = ((CMD_MEAS_PERI_2_M >> 0) & 0x00FF);
    res = i2c_master_7bit_write(TEM_HUM_ADDR, data, 2);
    return res;
}

uint8_t tem_hum_read_sensor_data(uint8_t *data)
{
    uint8_t addr_data[10];
    addr_data[0] = ((CMD_FETCH_DATA >> 8) & 0x00FF);
    addr_data[1] = ((CMD_FETCH_DATA >> 0) & 0x00FF);
    i2c_master_7bit_read_target_address(TEM_HUM_ADDR, addr_data, 2, data, 6);

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

