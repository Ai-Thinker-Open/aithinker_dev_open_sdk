/** @brief      I2C application interface.
 *
 *  @file       aiio_i2c.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/04/09      <td>1.0.0       <td>hongjz      <td>I2C API
 *  </table>
 *
 */
#include "aiio_i2c.h"

#include "i2c_api.h"
#include "i2c_ex_api.h"
#include "PinNames.h"

#define PIN_CALCULATE(a, b) ((a)<<5|(b))

static i2c_t s_i2c_dev;

int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    (void)aiio_i2c_port;

    PinName sda, scl;

    sda = PIN_CALCULATE(aiio_i2c_cfg->aiio_gpio_sda_port, aiio_i2c_cfg->aiio_gpio_sda_pin);
    scl = PIN_CALCULATE(aiio_i2c_cfg->aiio_gpio_scl_port, aiio_i2c_cfg->aiio_gpio_scl_pin);

    i2c_init(&s_i2c_dev, sda ,scl);
    i2c_frequency(&s_i2c_dev, aiio_i2c_cfg->aiio_i2c_period);

    //enable restart
    i2c_restart_enable(&s_i2c_dev);
    return AIIO_OK;
}

int32_t aiio_i2c_master_deinit(aiio_i2c_enum_t aiio_i2c_port)
{
    i2c_reset(&s_i2c_dev);
    return AIIO_OK;
}

int32_t aiio_i2c_master_write(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    (void)aiio_i2c_port;
    (void)timeout;

    dev_addr >>= 1;

	i2c_write(&s_i2c_dev, dev_addr, target_addr, target_addr_len, 0);
	i2c_write(&s_i2c_dev, dev_addr, buf, buf_len, 1);

    return AIIO_OK;
}

int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    (void)aiio_i2c_port;
    (void)timeout;

    dev_addr >>= 1;

	i2c_write(&s_i2c_dev, dev_addr, target_addr, target_addr_len, 0);
	i2c_read(&s_i2c_dev, dev_addr, buf, buf_len, 1);

    return AIIO_OK;
}

int32_t aiio_i2c_master_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t data, uint32_t timeout)
{
    return aiio_i2c_master_write(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data, 1, timeout);
}

int32_t aiio_i2c_master_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,uint8_t *data, uint32_t timeout)
{
    return aiio_i2c_master_read(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout);
}
