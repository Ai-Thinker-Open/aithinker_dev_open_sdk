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

#include "hosal_gpio.h"

#include "aiio_gpio.h"

#define DEF_PIN_ID_MAX (22)

typedef struct
{
    uint8_t inited : 1;  /**< 0 deinit 1 init*/
    uint8_t in_out : 1;  /**< 0 in 1 out*/
    uint8_t up_down : 1; /**< 0 down 1 up*/
    uint8_t irq_en : 1;/**< 0 disable enable*/
    uint8_t reserved : 4;
    hosal_gpio_dev_t gpio_port;
    aiio_hal_gpio_int_cb_t aiio_hal_gpio_int_cb;
} aiio_gpio_t;

static aiio_gpio_t io_port[DEF_PIN_ID_MAX + 1] = {0};

static void irq_cb(void *arg)
{
    uint8_t val1 = *(uint8_t *)arg;
    if(io_port[val1].irq_en)
    {
        (*io_port[val1].aiio_hal_gpio_int_cb)(NULL,arg);
    }
}

aiio_ret_t aiio_hal_gpio_init(uint32_t port, uint16_t pin)
{
    (void)port;

    if (io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    io_port[pin].gpio_port.port = pin;
    io_port[pin].gpio_port.config = INPUT_HIGH_IMPEDANCE;
    if (0 == hosal_gpio_init(&io_port[pin].gpio_port))
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
        hosal_gpio_irq_mask(&io_port[pin].gpio_port, pin);
    }

    if (0 == hosal_gpio_finalize(&io_port[pin].gpio_port))
    {
        io_port[pin].inited = 0;
        return AIIO_SUCC;
    }
    return AIIO_ERR_INIT_FAIL;
}

int32_t aiio_hal_gpio_pin_direction_set(uint32_t port, uint16_t pin, aiio_gpio_direction_t dir)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;
    hosal_gpio_finalize(&io_port[pin].gpio_port);
    if (AIIO_GPIO_INPUT == dir)
    {
        io_port[pin].in_out=0;
        if (io_port[pin].up_down)
        {
            io_port[pin].gpio_port.config = INPUT_PULL_UP;
        }
        else
        {
            io_port[pin].gpio_port.config = INPUT_PULL_DOWN;
        }
    }
    else
    {
        io_port[pin].in_out=1;
        if (io_port[pin].up_down)
        {
            io_port[pin].gpio_port.config = OUTPUT_OPEN_DRAIN_PULL_UP;
        }
        else
        {
            io_port[pin].gpio_port.config = OUTPUT_OPEN_DRAIN_NO_PULL;
        }
    }
    if (0 == hosal_gpio_init(&io_port[pin].gpio_port))
    {
        return AIIO_SUCC;
    }
    return AIIO_ERR_INIT_FAIL;
}

int32_t aiio_hal_gpio_pin_pull_set(uint32_t port, uint16_t pin, aiio_gpio_pull_t pull)
{

    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    hosal_gpio_finalize(&io_port[pin].gpio_port);
    if (AIIO_GPIO_PULL_DOWN == pull)
    {
        io_port[pin].up_down=0;
        if (io_port[pin].in_out)
        {
            io_port[pin].gpio_port.config = OUTPUT_OPEN_DRAIN_NO_PULL;
        }
        else
        {
            io_port[pin].gpio_port.config = INPUT_PULL_DOWN;
        }
    }
    else
    {
        io_port[pin].up_down=1;
        if (io_port[pin].in_out)
        {
            io_port[pin].gpio_port.config = OUTPUT_OPEN_DRAIN_PULL_UP;
        }
        else
        {
            io_port[pin].gpio_port.config = INPUT_PULL_UP;
        }
    }
    if (0 == hosal_gpio_init(&io_port[pin].gpio_port))
    {
        return AIIO_SUCC;
    }
    return AIIO_ERR_INIT_FAIL;
}

aiio_ret_t aiio_hal_gpio_set(uint32_t port, uint16_t pin, uint8_t OnOff)
{
    (void)port;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;
    if (OnOff)
    {
        hosal_gpio_output_set(&io_port[pin].gpio_port, 1);
    }
    else
    {
        hosal_gpio_output_set(&io_port[pin].gpio_port, 0);
    }
    return AIIO_SUCC;
}

uint8_t aiio_hal_gpio_get(uint32_t port, uint16_t pin)
{
    (void)port;
    uint8_t value;
    if (!io_port[pin].inited || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;
    hosal_gpio_input_get(&io_port[pin].gpio_port, &value);
    return value;
}

aiio_ret_t aiio_hal_gpio_int_enable(uint32_t port, uint16_t pin, uint8_t en)
{
    (void)port;
    if (!io_port[pin].inited || io_port[pin].in_out || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;
    if (!en)
    {
        io_port[pin].irq_en = 0;
        hosal_gpio_irq_mask(&io_port[pin].gpio_port, 1);
    }else{
        io_port[pin].irq_en = 1;
        hosal_gpio_irq_mask(&io_port[pin].gpio_port, 0);
    }
    return AIIO_SUCC;
}

aiio_ret_t aiio_hal_gpio_int_register(uint32_t port, uint16_t pin, aiio_hal_gpio_int_cb_t cb)
{
    (void)port;
    if (!io_port[pin].inited || io_port[pin].in_out || pin > DEF_PIN_ID_MAX)
        return AIIO_ERR_INIT_FAIL;

    // hosal_gpio_finalize(&io_port[pin].gpio_port);
    io_port[pin].aiio_hal_gpio_int_cb = cb;
    io_port[pin].gpio_port.port = pin;
    io_port[pin].irq_en = 1;
    if (io_port[pin].up_down)
    {
        hosal_gpio_irq_set(&io_port[pin].gpio_port, HOSAL_IRQ_TRIG_NEG_PULSE, irq_cb, &io_port[pin].gpio_port.port);
    }
    else
    {
        hosal_gpio_irq_set(&io_port[pin].gpio_port, HOSAL_IRQ_TRIG_POS_PULSE, irq_cb, &io_port[pin].gpio_port.port);
    }

    // hosal_gpio_init(&io_port[pin].gpio_port);
    return AIIO_SUCC;
}
