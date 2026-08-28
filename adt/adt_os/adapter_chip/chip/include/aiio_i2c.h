/** @brief      I2C application interface.
 *
 *  @file       aiio_i2c.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>I2C Init
 *  </table>
 *
 */
#ifndef __AIIO_I2C_H__
#define __AIIO_I2C_H__
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"

/**
 *  @brief I2C interface enumeration
 */
typedef enum
{
    AIIO_I2C_0,    /*!< I2C interface zero */
    AIIO_I2C_1,    /*!< I2C interface one */
    AIIO_I2C_2,    /*!< I2C interface two */
    AIIO_I2C_SOFT, /*!< I2C software interface */
} aiio_i2c_enum_t;

/**
 *  @brief I2C pin config interface
 */
typedef struct
{
    uint8_t aiio_gpio_sda_port; /*!< GPIO port */
    uint32_t aiio_gpio_sda_pin; /*!< GPIO pin */
    uint8_t aiio_gpio_scl_port; /*!< GPIO port */
    uint32_t aiio_gpio_scl_pin; /*!< GPIO pin */
    uint32_t aiio_i2c_period;   /*!< I2C period*/
} aiio_i2c_pin_cfg_t;

/** @brief I2C function initializes
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
CHIP_API int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg);

/** @brief I2C function fails to initialeze
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_i2c_master_deinit(aiio_i2c_enum_t aiio_i2c_port);

/** @brief I2C master write data function
 *
 *  @param[in]      aiio_i2c_port           I2C interface
 *  @param[in]      dev_addr                I2C device address
 *  @param[in]      target_addr             I2C register address
 *  @param[in]      target_addr_len         I2C register address lenght
 *  @param[in]      buf                     I2C data buf
 *  @param[in]      buf_len                 I2C data buf lenght
 *  @param[in]      timeout         I2C delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_i2c_master_write(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout);

/** @brief I2C master read data function
 *
 *  @param[in]      aiio_i2c_port           I2C interface
 *  @param[in]      dev_addr                I2C device address
 *  @param[in]      target_addr             I2C register address
 *  @param[in]      target_addr_len         I2C register address lenght
 *  @param[in]      buf                     I2C data buf
 *  @param[in]      buf_len                 I2C data buf lenght
 *  @param[in]      timeout         I2C delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout);

/** @brief I2C master write byte function
 *
 *  @param[in]      aiio_i2c_port  I2C interface
 *  @param[in]      dev_addr       I2C device address
 *  @param[in]      target_addr    I2C register address
 *  @param[in]      value          I2C data value
 *  @param[in]      timeout         I2C delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_i2c_master_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t data, uint32_t timeout);

/** @brief I2C master read byte function
 *
 *  @param[in]      aiio_i2c_port   I2C interface
 *  @param[in]      dev_addr        I2C device address
 *  @param[in]      target_addr     I2C target address
 *  @param[in]      target_addr_len I2C target address lenght
 *  @param[in]      value           I2C data value return
 *  @param[in]      timeout         I2C delay waiting time
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API int32_t aiio_i2c_master_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,uint8_t *data, uint32_t timeout);

#endif
