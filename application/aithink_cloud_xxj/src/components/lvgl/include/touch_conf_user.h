/**
 * @file        touch_conf_user.h
 * @brief
 * @author      Jimmy (ccolacat@163.com)
 * @version     v1.0.0
 * @date        2023-06-06
 * 
 * @copyright   Copyright (c) 2020-2023 Shenzhen Ai-Thinker Technology Co., Ltd. All Rights Reserved.
 * 
 * Disclaimer:  This program is for informational purposes only, please keep author information and source code license.
 * 
 */

#pragma once

#include "stdint.h"


/* spi interface
    TOUCH_SPI_XPT2046  // Not currently supported
*/

/* i2c interface
    TOUCH_I2C_CHSC6540
    TOUCH_I2C_FT6X36
    TOUCH_I2C_GT911
*/

/* Select Touch Type */
#define TOUCH_I2C_FT6X36
#define TOUCH_I2C_CHSC6540

/* touch interface */
#define TOUCH_INTERFACE_SPI 1
#define TOUCH_INTERFACE_I2C 2

/* touch interface pin config */
#define TOUCH_I2C_SCL_PIN   GPIO_PIN_0
#define TOUCH_I2C_SDA_PIN   GPIO_PIN_1


/************************ Copyright (c) 2023 Shenzhen Ai-Thinker Technology Co., Ltd. ************************/

