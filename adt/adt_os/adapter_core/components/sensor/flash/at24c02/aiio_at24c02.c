/** @brief      AT24C02 application interface.
 *
 *  @file       aiio_at24c02.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>I2C Init
 *  </table>
 *
 */
#include "aiio_at24c02.h"

#define AT24C04_ADDR_W 0xA0
#define AT24C04_WRITE_DELAY 50000

int32_t aiio_at24c02_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    if (aiio_i2c_master_init(aiio_i2c_port, aiio_i2c_cfg) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_at24c02_read(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    uint8_t bufer = 0;

    if (word_addr + buf_len >= 256)
    {
        if (word_addr >= 256)
        {
            bufer = word_addr - 256;

            if (AIIO_OK != aiio_i2c_master_read(aiio_i2c_port, AT24C04_ADDR_W | 0x02, &bufer, 1, buf, buf_len,timeout))
            {
                return AIIO_ERROR;
            }
        }
        else
        {
            bufer = word_addr;

            if (AIIO_OK != aiio_i2c_master_read(aiio_i2c_port, AT24C04_ADDR_W, &bufer, 1, buf, 256 - word_addr,timeout))
            {
                return AIIO_ERROR;
            }

            bufer = 0;

            if (AIIO_OK != aiio_i2c_master_read(aiio_i2c_port, AT24C04_ADDR_W | 0x02, &bufer, 1, buf + 256 - word_addr, buf_len - (256 - word_addr),timeout))
            {
                return AIIO_ERROR;
            }
        }
    }
    else
    {
        bufer = word_addr;

        if (AIIO_OK != aiio_i2c_master_read(aiio_i2c_port, AT24C04_ADDR_W, &bufer, 1, buf, buf_len,timeout))
        {
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}

int32_t aiio_at24c02_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *data, uint32_t timeout)
{
    if (AIIO_OK != aiio_at24c02_read(aiio_i2c_port, word_addr, data, 1,timeout))
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_at24c02_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t data, uint32_t timeout)
{
    uint8_t buf[2];
    uint8_t drv_addr = AT24C04_ADDR_W;
    buf[0] = word_addr & 0xFF;
    buf[1] = data;

    if (word_addr > 255)
        drv_addr = (AT24C04_ADDR_W | 0x02);

    if (AIIO_OK != aiio_i2c_master_write(aiio_i2c_port, drv_addr, buf, 1, buf + 1, 1,timeout))
    {
        return AIIO_ERROR;
    }

    for (volatile uint32_t i = 0; i < AT24C04_WRITE_DELAY; i++)
    {
    };

    return AIIO_OK;
}

int32_t aiio_at24c02_write(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    uint16_t i = 0;

    for (i = 0; i < buf_len; i++)
    {
        if (AIIO_OK != aiio_at24c02_write_byte(aiio_i2c_port, word_addr + i, buf[i],timeout))
        {
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}
