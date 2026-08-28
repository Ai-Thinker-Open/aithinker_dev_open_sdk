/** @brief GPIO intermediate device port adaptation
 *
 *  @file        aiio_gpio.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/05/30          <td>V1.0.0          <td>hewm            <td>GPIO device port adaptation
 *  <table>
 *
 */

#include "stdint.h"
#include "aiio_error.h"
#include "aiio_gpio.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"
#include "pinctrl.h"
#include "gpio.h"

#define DEF_PIN_ID_MAX (22)

typedef struct
{
    uint8_t inited : 1;  /**< 0 deinit 1 init*/
    uint8_t in_out : 1;  /**< 0 in 1 out*/
    uint8_t up_down : 1; /**< 0 down 1 up*/
    uint8_t irq_en : 1;  /**< 0 disable enable*/
    uint8_t reserved : 4;
    uint16_t gpio_pin;
    aiio_hal_gpio_int_cb_t aiio_hal_gpio_int_cb;
} aiio_gpio_t;

static aiio_gpio_t io_port[DEF_PIN_ID_MAX + 1] = {0};

aiio_ret_t aiio_hal_gpio_init(uint32_t port, uint16_t pin)
{
    (void)port;

    if (io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    io_port[pin].gpio_pin = pin;

    if (ERRCODE_SUCC == uapi_pin_set_mode(io_port[pin].gpio_pin, (pin_mode_t)HAL_PIO_FUNC_GPIO))
    {
        io_port[pin].inited = 1;
        return AIIO_SUCC;
    }
    return AIIO_ERR_INIT_FAIL;
}

aiio_ret_t aiio_hal_gpio_deinit(uint32_t port, uint16_t pin)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    if (io_port[pin].irq_en)
    {
        io_port[pin].irq_en = 0;
        uapi_gpio_unregister_isr_func(pin);
    }

    io_port[pin].inited = 0;
    return AIIO_SUCC;
}

int32_t aiio_hal_gpio_pin_direction_set(uint32_t port, uint16_t pin, aiio_gpio_direction_t dir)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    switch (dir)
    {
    case AIIO_GPIO_INPUT:
        io_port[pin].in_out=0;
        uapi_gpio_set_dir(pin, GPIO_DIRECTION_INPUT);
        break;
    case AIIO_GPIO_OUTPUT:
        io_port[pin].in_out=1;
        uapi_gpio_set_dir(pin, GPIO_DIRECTION_OUTPUT);
        break;
    default:
        break;
    }

    return AIIO_SUCC;
}

int32_t aiio_hal_gpio_pin_pull_set(uint32_t port, uint16_t pin, aiio_gpio_pull_t pull)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    switch (pull)
    {
    case AIIO_GPIO_PULL_DOWN:
        io_port[pin].up_down=0;
        uapi_pin_set_pull(pin, PIN_PULL_DOWN);
        break;
    case AIIO_GPIO_PULL_UP:
        io_port[pin].up_down=1;
        uapi_pin_set_pull(pin, PIN_PULL_UP);
        break;
    case AIIO_GPIO_PULL_NONE:
        uapi_pin_set_pull(pin, PIN_PULL_NONE);
        break;
    default:
        break;
    }

    return AIIO_SUCC;
}

aiio_ret_t aiio_hal_gpio_set(uint32_t port, uint16_t pin, uint8_t OnOff)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    switch (OnOff)
    {
    case 0:
        uapi_gpio_set_val(pin, GPIO_LEVEL_LOW);
        break;
    case 1:
        uapi_gpio_set_val(pin, GPIO_LEVEL_HIGH);
        break;
    default:
        break;
    }

    return AIIO_SUCC;
}

uint8_t aiio_hal_gpio_get(uint32_t port, uint16_t pin)
{
    (void)port;

    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    return (uint8_t)uapi_gpio_get_val(pin);
}

aiio_ret_t aiio_hal_gpio_int_enable(uint32_t port, uint16_t pin, uint8_t en)
{
    (void)port;
    if (!io_port[pin].inited || io_port[pin].in_out || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    if (!en)
    {
        io_port[pin].irq_en = 0;
        uapi_gpio_disable_interrupt(pin);
    }else{
        io_port[pin].irq_en = 1;
        uapi_gpio_enable_interrupt(pin);
    }

    return AIIO_SUCC;
}

aiio_ret_t aiio_hal_gpio_int_register(uint32_t port, uint16_t pin, aiio_hal_gpio_int_cb_t cb)
{
    (void)port;
    if (!io_port[pin].inited || io_port[pin].in_out || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    io_port[pin].irq_en = 1;
    if (io_port[pin].up_down)
    {
        uapi_gpio_register_isr_func(pin, GPIO_INTERRUPT_RISING_EDGE,(gpio_callback_t)cb);
    }
    else
    {
        uapi_gpio_register_isr_func(pin, GPIO_INTERRUPT_FALLING_EDGE,(gpio_callback_t)cb);
    }

    return AIIO_SUCC;
}

// aiio_ret_t aiio_at_get_gpio_by_map(uint8_t at_pin,uint32_t *p_port, uint16_t *p_pin)
// {
//     *p_port=AIIO_GPIO_B;
//     *p_pin=at_pin;
//     return AIIO_OK;
// }
