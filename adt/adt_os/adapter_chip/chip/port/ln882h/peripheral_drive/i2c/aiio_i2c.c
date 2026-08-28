/** @brief      I2C application interface.
 *
 *  @file       aiio_i2c.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       I2C application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>I2C Init
 *  </table>
 *
 */
#include "aiio_i2c.h"
#include "hal/hal_common.h"
#include "hal/hal_i2c.h"
#include "hal/hal_gpio.h"
#include "hal/hal_clock.h"

#define TIMEOUT_CYCLE 1000
#define AIIO_FUNC_FAIL -3

void i2c_init(uint8_t gpio_scl_port, gpio_pin_t gpio_scl_pin,
              uint8_t gpio_sda_port, gpio_pin_t gpio_sda_pin,
              uint32_t i2c_period)
{
    uint32_t gpio_scl_base = 0;
    uint32_t gpio_sda_base = 0;

    if (gpio_scl_port == 0)
        gpio_scl_base = GPIOA_BASE;
    else if (gpio_scl_port == 1)
        gpio_scl_base = GPIOB_BASE;

    if (gpio_sda_port == 0)
        gpio_sda_base = GPIOA_BASE;
    else if (gpio_sda_port == 1)
        gpio_sda_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_scl_base, gpio_scl_pin, I2C0_SCL);
    hal_gpio_pin_afio_select(gpio_sda_base, gpio_sda_pin, I2C0_SDA);
    hal_gpio_pin_afio_en(gpio_scl_base, gpio_scl_pin, HAL_ENABLE);
    hal_gpio_pin_afio_en(gpio_sda_base, gpio_sda_pin, HAL_ENABLE);
    hal_i2c_en(I2C_BASE, HAL_DISABLE);
    i2c_init_t_def i2c_init;
    memset(&i2c_init, 0, sizeof(i2c_init));
    i2c_init.i2c_peripheral_clock_freq = 4;
    i2c_init.i2c_master_mode_sel = I2C_FM_MODE;
    i2c_init.i2c_fm_mode_duty_cycle = I2C_FM_MODE_DUTY_CYCLE_2;
    i2c_init.i2c_ccr = (1.0f / i2c_period) / 25 * hal_clock_get_apb0_clk();
    i2c_init.i2c_trise = 0xF;
    hal_i2c_init(I2C_BASE, &i2c_init);
    hal_i2c_en(I2C_BASE, HAL_ENABLE);
}

uint8_t i2c_master_7bit_write_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,
        uint8_t *buf, uint16_t buf_len)
{
    if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(I2C_BASE);
        return HAL_RESET;
    }

    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_master_send_data(I2C_BASE, dev_addr);

    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_clear_sr(I2C_BASE);

    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, target_addr[i]);
        }
    }

    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    for (uint32_t i = 0; i < buf_len; i++)
    {
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, buf[i]);
        }
    }

    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_master_stop(I2C_BASE);
    return HAL_SET;
}

uint8_t i2c_master_7bit_read_target_address(uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len)
{
    if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
    {
        hal_i2c_master_reset(I2C_BASE);
        return HAL_RESET;
    }

    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_master_send_data(I2C_BASE, dev_addr);

    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_clear_sr(I2C_BASE);

    for (uint16_t i = 0; i < target_addr_len; i++)
    {
        if (hal_i2c_wait_txe(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            return HAL_RESET;
        }
        else
        {
            hal_i2c_master_send_data(I2C_BASE, target_addr[i]);
        }
    }

    if (hal_i2c_wait_btf(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_master_reset(I2C_BASE);

    if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_master_send_data(I2C_BASE, dev_addr + 1);

    if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        return HAL_RESET;

    hal_i2c_clear_sr(I2C_BASE);
    hal_i2c_master_recv_data(I2C_BASE);

    for (int i = buf_len; i > 0; i--)
    {
        if (buf_len == 1)
        {
            hal_i2c_ack_en(I2C_BASE, HAL_DISABLE);

            if (hal_i2c_wait_rxne(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(I2C_BASE);
            }
        }
        else
        {
            hal_i2c_ack_en(I2C_BASE, HAL_ENABLE);

            if (hal_i2c_wait_rxne(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            {
                return HAL_RESET;
            }
            else
            {
                *buf = hal_i2c_master_recv_data(I2C_BASE);
            }
        }

        buf_len--;
        buf++;
    }

    hal_i2c_master_stop(I2C_BASE);
    return HAL_SET;
}

void i2c_address_scan(uint8_t *address, uint16_t *address_num)
{
    hal_i2c_ack_en(I2C_BASE, HAL_DISABLE);

    for (uint32_t dev_addr = 0; dev_addr <= 254; dev_addr = dev_addr + 2)
    {
        if (hal_i2c_wait_bus_idle(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
        {
            hal_i2c_master_reset(I2C_BASE);
            ln_delay_ms(20);
            continue;
        }

        if (hal_i2c_master_start(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            continue;

        hal_i2c_master_send_data(I2C_BASE, dev_addr);

        if (hal_i2c_master_wait_addr(I2C_BASE, TIMEOUT_CYCLE) == HAL_RESET)
            continue;

        ln_delay_ms(1);

        if (hal_i2c_get_status_flag(I2C_BASE, I2C_STATUS_FLAG_AF) == HAL_SET)
        {
            hal_i2c_master_reset(I2C_BASE);
            hal_i2c_clear_sr(I2C_BASE);
            hal_i2c_master_stop(I2C_BASE);
            ln_delay_ms(20);
        }
        else
        {
            hal_i2c_master_reset(I2C_BASE);
            hal_i2c_clear_sr(I2C_BASE);
            address[*address_num] = dev_addr;
            (*address_num)++;
            ln_delay_ms(20);
        }
    }

    hal_i2c_clear_sr(I2C_BASE);
    hal_i2c_master_stop(I2C_BASE);
}

struct aiio_i2c_struct
{
    aiio_i2c_enum_t aiio_i2c_port;
    uint8_t aiio_i2c_sda_port;
    uint8_t aiio_i2c_scl_port;
    uint32_t aiio_i2c_sda_pin;
    uint32_t aiio_i2c_scl_pin;
    uint32_t i2c_period;
};

struct aiio_i2c_struct aiio_i2c_list[4];
static uint8_t aiio_i2c_num = 0;

static int32_t aiio_i2c_port_cal(aiio_i2c_enum_t aiio_i2c_port)
{
    uint8_t i = 0;

    for (i = 0; i < aiio_i2c_num; i++)
    {
        if (aiio_i2c_list[i].aiio_i2c_port == aiio_i2c_port)
        {
            return i;
        }
    }

    return AIIO_FUNC_FAIL;
}

int32_t aiio_i2c_master_init(aiio_i2c_enum_t aiio_i2c_port, aiio_i2c_pin_cfg_t *aiio_i2c_cfg)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) != AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_init function aiio_i2c_port is repetition!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        i2c_init(aiio_i2c_cfg->aiio_gpio_scl_port, aiio_i2c_cfg->aiio_gpio_scl_pin, aiio_i2c_cfg->aiio_gpio_sda_port, aiio_i2c_cfg->aiio_gpio_sda_pin, aiio_i2c_cfg->aiio_i2c_period);
        break;

    case AIIO_I2C_1:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_I2C_2:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!");
        break;

    default:
        break;
    }

    aiio_i2c_list[aiio_i2c_num].aiio_i2c_port = aiio_i2c_port;
    aiio_i2c_list[aiio_i2c_num].aiio_i2c_sda_port = aiio_i2c_cfg->aiio_gpio_scl_port;
    aiio_i2c_list[aiio_i2c_num].aiio_i2c_scl_port = aiio_i2c_cfg->aiio_gpio_scl_pin;
    aiio_i2c_list[aiio_i2c_num].aiio_i2c_sda_pin = aiio_i2c_cfg->aiio_gpio_sda_port;
    aiio_i2c_list[aiio_i2c_num].aiio_i2c_scl_pin = aiio_i2c_cfg->aiio_gpio_sda_pin;
    aiio_i2c_list[aiio_i2c_num].i2c_period = aiio_i2c_cfg->aiio_i2c_period;
    aiio_i2c_num++;
    return AIIO_OK;
}

int32_t aiio_i2c_master_deinit(aiio_i2c_enum_t aiio_i2c_port)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_0 is nonsupport!\r\n");
        break;

    case AIIO_I2C_1:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!\r\n");
        break;

    case AIIO_I2C_2:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!\r\n");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_write(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        if (HAL_SET != i2c_master_7bit_write_target_address(dev_addr, target_addr, target_addr_len, buf, buf_len))
        {
            aiio_log_e("aiio_i2c_master_write function write fail!");
            return AIIO_ERROR;
        }

        break;

    case AIIO_I2C_1:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_I2C_2:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_e("aiio_i2c_master_write function AIIO_I2C_SOFT is nonsupport!");
        break;

    default:
        break;
    }

    return AIIO_OK;
}


int32_t aiio_i2c_master_write_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t data, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_i2c_master_write(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_read(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len, uint8_t *buf, uint16_t buf_len, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_i2c_port)
    {
    case AIIO_I2C_0:
        if (HAL_SET != i2c_master_7bit_read_target_address(dev_addr, target_addr, target_addr_len, buf, buf_len))
        {
            aiio_log_e("aiio_i2c_master_read function read fail!");
            return AIIO_ERROR;
        }

        break;

    case AIIO_I2C_1:
        aiio_log_e("aiio_i2c_master_read function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_I2C_2:
        aiio_log_e("aiio_i2c_master_read function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_I2C_SOFT:
        aiio_log_e("aiio_i2c_master_read function AIIO_I2C_SOFT is nonsupport!");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_i2c_master_read_byte(aiio_i2c_enum_t aiio_i2c_port, uint8_t dev_addr, uint8_t *target_addr, uint8_t target_addr_len,uint8_t *data, uint32_t timeout)
{
    if (aiio_i2c_port_cal(aiio_i2c_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_i2c_deinit function aiio_i2c_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_i2c_master_read(aiio_i2c_port, dev_addr, target_addr, target_addr_len, data,1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
