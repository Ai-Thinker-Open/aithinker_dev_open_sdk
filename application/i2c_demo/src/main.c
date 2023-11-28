#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"
#include "aiio_autoconf.h"

uint8_t str[100] = "Welcome to the Ai-Thinker AT24C02 component!";
uint8_t str1[100];

#define AIIO_I2C_SCL_PORT   CONFIG_I2C_SCL_PORT
#define AIIO_I2C_SCL_PIN    CONFIG_I2C_SCL_PIN
#define AIIO_I2C_SDA_PORT   CONFIG_I2C_SDA_PORT
#define AIIO_I2C_SDA_PIN    CONFIG_I2C_SDA_PIN
#define AIIO_I2C_PERIOD     CONFIG_I2C_PERIOD

void aiio_main(void *params)
{
    // ln882h
    aiio_i2c_pin_cfg_t aiio_i2c_cfg = {
        .aiio_gpio_scl_port = AIIO_I2C_SCL_PORT,
        .aiio_gpio_scl_pin = AIIO_I2C_SCL_PIN,
        .aiio_gpio_sda_port = AIIO_I2C_SDA_PORT,
        .aiio_gpio_sda_pin = AIIO_I2C_SDA_PIN,
        .aiio_i2c_period = AIIO_I2C_PERIOD,
    };

    // //bl602
    // aiio_i2c_pin_cfg_t aiio_i2c_cfg = {
    //     .aiio_gpio_scl_port = AIIO_GPIO_B,
    //     .aiio_gpio_scl_pin = AIIO_GPIO_NUM_4,
    //     .aiio_gpio_sda_port = AIIO_GPIO_B,
    //     .aiio_gpio_sda_pin = AIIO_GPIO_NUM_17,
    //     .aiio_i2c_period = 100000,
    // };
    aiio_at24c02_init(AIIO_I2C_0, &aiio_i2c_cfg);

    while (1)
    {
        aiio_at24c02_write(AIIO_I2C_0, 0, str, strlen((const char *)str), 1000);
        aiio_at24c02_read(AIIO_I2C_0, 0, str1, strlen((const char *)str), 1000);
        aiio_log_w("%s", str);
        aiio_log_w("%s", str1);
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
