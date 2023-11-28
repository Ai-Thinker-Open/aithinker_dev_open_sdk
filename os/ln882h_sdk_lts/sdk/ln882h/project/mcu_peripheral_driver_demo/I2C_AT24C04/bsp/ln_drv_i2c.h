/**
 * @file     ln_drv_dma.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_I2C_H
#define __LN_DRV_I2C_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

#include "ln_test_common.h"

typedef enum
{
    GPIO_A = 0,
    GPIO_B = 1,
}gpio_port_t;

void i2c_init(gpio_port_t gpio_scl_port,gpio_pin_t gpio_scl_pin,
              gpio_port_t gpio_sda_port,gpio_pin_t gpio_sda_pin,
              uint32_t i2c_period);

uint8_t i2c_master_7bit_write_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, 
                                             uint8_t *buf, uint16_t buf_len);

uint8_t i2c_master_7bit_read_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, 
                                            uint8_t *buf, uint16_t buf_len);

void i2c_address_scan(uint8_t *address,uint16_t *address_num);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_I2C_H
