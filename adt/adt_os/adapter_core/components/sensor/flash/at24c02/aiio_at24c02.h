/** @brief      AT24C02 application interface.
 *
 *  @file       aiio_at24c02.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>I2C Init
 *  </table>
 *
 */
#ifndef __AIIO_AT24C02_H__
#define __AIIO_AT24C02_H__
#include "aiio_i2c.h"

/** @brief AT24C02 initializes
 *
 *  @param[in]      aiio_i2c_port              I2C interface
 *  @param[in]      aiio_i2c_sda_port          I2C sda pin port
 *  @param[in]      aiio_i2c_sda_num           I2C sda pin num
 *  @param[in]      aiio_i2c_scl_port          I2C scl pin port
 *  @param[in]      aiio_i2c_scl_num           I2C scl pin num
 *  @param[in]      i2c_period                 I2C period
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_at24c02_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg);

/** @brief at24c02 read byte function
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @param[in]      word_addr       at24c02 word address
 *  @param[in]      data            at24c02 return data
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_at24c02_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *data, uint32_t timeout);

/** @brief at24c02 read data function
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @param[in]      word_addr       at24c02 word address
 *  @param[in]      buf             at24c02 buf data
 *  @param[in]      buf_len         at24c02 buf lenght
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_at24c02_read(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len, uint32_t timeout);

/** @brief at24c02 write byte function
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @param[in]      word_addr       at24c02 word address
 *  @param[in]      data            at24c02 data
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_at24c02_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t data, uint32_t timeout);

/** @brief at24c02 write data function
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @param[in]      word_addr       at24c02 word address
 *  @param[in]      buf             at24c02 buf data
 *  @param[in]      buf_len         at24c02 buf lenght
 *  @param[in]      timeout         delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_at24c02_write(aiio_i2c_enum_t aiio_i2c_port, uint16_t word_addr, uint8_t *buf, uint16_t buf_len, uint32_t timeout);

#endif
