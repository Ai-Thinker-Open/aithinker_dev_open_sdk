/** @brief i2s demo
 *
 *  @file        main.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/08/01          <td>V1.0.0          <td>johhn           <td>demo
 *  <table>
 *
 */
#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "aiio_autoconf.h"
#include "aiio_es8388.h"

#define AIIO_I2C_SCL_PORT   CONFIG_I2C_SCL_PORT
#define AIIO_I2C_SCL_PIN    CONFIG_I2C_SCL_PIN
#define AIIO_I2C_SDA_PORT   CONFIG_I2C_SDA_PORT
#define AIIO_I2C_SDA_PIN    CONFIG_I2C_SDA_PIN
#define AIIO_I2C_PERIOD     CONFIG_I2C_PERIOD



#define AIIO_I2S_FS_PORT   CONFIG_I2S_FS_PORT
#define AIIO_I2S_FS_PIN    CONFIG_I2S_FS_PIN
#define AIIO_I2S_DI_PORT   CONFIG_I2S_DI_PORT
#define AIIO_I2S_DI_PIN    CONFIG_I2S_DI_PIN
#define AIIO_I2S_DO_PORT   CONFIG_I2S_DO_PORT
#define AIIO_I2S_DO_PIN    CONFIG_I2S_DO_PIN
#define AIIO_I2S_BCLK_PORT   CONFIG_I2S_BCLK_PORT
#define AIIO_I2S_BCLK_PIN    CONFIG_I2S_BCLK_PIN
#define AIIO_I2S_MCLK_PORT   CONFIG_I2S_MCLK_PORT
#define AIIO_I2S_MCLK_PIN    CONFIG_I2S_MCLK_PIN


void aiio_main(void *params)
{
    aiio_i2c_pin_cfg_t aiio_i2c_cfg = {
        .aiio_gpio_scl_port = AIIO_I2C_SCL_PORT,
        .aiio_gpio_scl_pin = AIIO_I2C_SCL_PIN,
        .aiio_gpio_sda_port = AIIO_I2C_SDA_PORT,
        .aiio_gpio_sda_pin = AIIO_I2C_SDA_PIN,
        .aiio_i2c_period = AIIO_I2C_PERIOD,
    };
    aiio_i2s_pin_cfg_t aiio_i2s_cfg = {
        .aiio_gpio_fs_port=AIIO_I2S_FS_PORT,
        .aiio_gpio_fs_pin=AIIO_I2S_FS_PIN,
        .aiio_gpio_di_port=AIIO_I2S_DI_PORT,
        .aiio_gpio_di_pin=AIIO_I2S_DI_PIN,
        .aiio_gpio_do_port=AIIO_I2S_DO_PORT,
        .aiio_gpio_do_pin=AIIO_I2S_DO_PIN,
        .aiio_gpio_bclk_port=AIIO_I2S_BCLK_PORT,
        .aiio_gpio_bclk_pin=AIIO_I2S_BCLK_PIN,
        .aiio_gpio_mclk_port=AIIO_I2S_MCLK_PORT,
        .aiio_gpio_mclk_pin=AIIO_I2S_MCLK_PIN,
    };

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");
     /* init ES8388 Codec */
    aiio_log_i("es8388 init");
    aiio_es8388_init(&aiio_i2c_cfg,&aiio_i2s_cfg);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
