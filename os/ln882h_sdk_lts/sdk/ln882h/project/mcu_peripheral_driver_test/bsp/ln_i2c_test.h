/**
 * @file     ln_i2c_test.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef LN_I2C_TEST
#define LN_I2C_TEST

#include "hal/hal_common.h"
#include "hal/hal_i2c.h"

void    ln_i2c_test(void);
void    hal_write_command_byte(uint32_t i2c_x_base, uint8_t cmd);
void    hal_write_data_byte(uint32_t  i2c_x_base, uint8_t data);
uint8_t hal_i2c_master_7bit_read_target_address(uint32_t i2c_x_base, uint8_t dev_addr,uint8_t *target_addr,uint8_t target_addr_len,uint8_t * buf, uint16_t buf_len);
uint8_t hal_i2c_master_7bit_write(uint32_t i2c_x_base, uint8_t dev_addr, const uint8_t * buf, uint16_t buf_len);
uint8_t hal_i2c_master_7bit_write_target_address(uint32_t i2c_x_base, uint8_t dev_addr, uint8_t *target_addr,uint8_t target_addr_len,const uint8_t * buf, uint16_t buf_len);

#endif

