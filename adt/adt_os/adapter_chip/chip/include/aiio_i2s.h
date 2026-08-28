/** @brief I2S HAL layer header file
 *
 *  @file        aiio_i2s.h
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/08/01          <td>V1.0.0          <td>johhn           <td>Define the I2S header API
 *  <table>
 *
 */

#ifndef __AIIO_HAL_I2S_H__
#define __AIIO_HAL_I2S_H__
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"
#include "stdint.h"
#ifdef __cplusplus
extern "C" {
#endif


/**
 *  @brief I2S interface enumeration
 */
typedef enum
{
    AIIO_I2S_0,    /*!< I2S interface zero */
    AIIO_I2S_1,    /*!< I2S interface one */
    AIIO_I2S_2,    /*!< I2S interface two */
    AIIO_I2S_SOFT, /*!< I2S software interface */
} aiio_i2s_enum_t;

/**
 *  @brief I2S pin config interface
 */
typedef struct
{
    uint8_t aiio_gpio_fs_port; /*!< GPIO port */
    uint32_t aiio_gpio_fs_pin; /*!< GPIO pin */
    uint8_t aiio_gpio_di_port; /*!< GPIO port */
    uint32_t aiio_gpio_di_pin; /*!< GPIO pin */
    uint8_t aiio_gpio_do_port; /*!< GPIO port */
    uint32_t aiio_gpio_do_pin; /*!< GPIO pin */
    uint8_t aiio_gpio_bclk_port; /*!< GPIO port */
    uint32_t aiio_gpio_bclk_pin; /*!< GPIO pin */
    uint8_t aiio_gpio_mclk_port; /*!< GPIO port */
    uint32_t aiio_gpio_mclk_pin; /*!< GPIO pin */
} aiio_i2s_pin_cfg_t;


/**
 * @brief API to init specific GPIO
 * @param  aiio_i2s_port The I2S port to be initialized
 * @param  aiio_i2s_cfg The configuration information of the I2S to be initialized
 * @return Status  - 0: Succeed, else: fail
 */
CHIP_API int aiio_hal_i2s_init(aiio_i2s_enum_t aiio_i2s_port, aiio_i2s_pin_cfg_t *aiio_i2s_cfg);

/**
 * @brief API to deinit specific GPIO
 * @param  aiio_i2s_port The I2S port to be initialized
 * @return Status  - 0: Succeed, else: fail
 */
CHIP_API int aiio_hal_i2s_deinit(aiio_i2s_enum_t aiio_i2s_port);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__AIIO_HAL_I2S_H__
