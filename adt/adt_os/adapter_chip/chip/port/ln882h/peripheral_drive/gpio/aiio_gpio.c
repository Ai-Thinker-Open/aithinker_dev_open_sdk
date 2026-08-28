/** @brief GPIO intermediate device port adaptation
 *
 *  @file        aiio_gpio.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/02/06          <td>V1.0.0          <td>hewm            <td>GPIO device port adaptation
 *  <table>
 *
 */

#include "stdint.h"

#include "hal/hal_common.h"
#include "hal/hal_gpio.h"

#include "aiio_gpio.h"

#define DEF_PIN_ID_MAX (15)

static aiio_hal_gpio_int_cb_t aiio_hal_gpio_int_cb[4][DEF_PIN_ID_MAX+1];

aiio_ret_t aiio_hal_gpio_init(uint32_t port, uint16_t pin)
{
    gpio_init_t_def gpio_init;
    memset(&gpio_init, 0, sizeof(gpio_init)); // 清零结构体

    gpio_init.dir = GPIO_OUTPUT;       // 配置GPIO方向为输出
    gpio_init.pin = pin;               // 配置GPIO引脚号
    gpio_init.speed = GPIO_HIGH_SPEED; // 设置GPIO速度
    hal_gpio_init(port, &gpio_init);   // 初始化GPIO
    return AIIO_SUCC;
}

aiio_ret_t aiio_hal_gpio_deinit(uint32_t port, uint16_t pin)
{
    hal_gpio_deinit(port);
    return AIIO_SUCC;
}

int32_t aiio_hal_gpio_pin_direction_set(uint32_t port, uint16_t pin, aiio_gpio_direction_t dir)
{
    hal_gpio_pin_direction_set(port, pin, dir);
    return 0;
}

int32_t aiio_hal_gpio_pin_pull_set(uint32_t port, uint16_t pin, aiio_gpio_pull_t pull)
{
    if (AIIO_GPIO_PULL_DOWN == pull)
        hal_gpio_pin_pull_set(port, pin, GPIO_PULL_DOWN);
    else
        hal_gpio_pin_pull_set(port, pin, GPIO_PULL_UP);
    return 0;
}

aiio_ret_t aiio_hal_gpio_set(uint32_t port, uint16_t pin, uint8_t OnOff)
{
    if (OnOff)
    {
        hal_gpio_pin_set(port, pin);
    }
    else
    {
        hal_gpio_pin_reset(port, pin);
    }
}

uint8_t aiio_hal_gpio_get(uint32_t port, uint16_t pin)
{
    return hal_gpio_pin_read(port, pin);
}

aiio_ret_t aiio_hal_gpio_int_enable(uint32_t port, uint16_t pin, uint8_t en)
{
    hal_gpio_pin_it_en(port, pin, en);
    return AIIO_SUCC;
}

aiio_ret_t aiio_hal_gpio_int_register(uint32_t port, uint16_t pin, aiio_hal_gpio_int_cb_t cb)
{
    if (pin > DEF_PIN_ID_MAX && port > 3)
        return AIIO_ERR_INIT_FAIL;

    aiio_hal_gpio_int_cb[port][pin] = cb;

    return AIIO_SUCC;
}


