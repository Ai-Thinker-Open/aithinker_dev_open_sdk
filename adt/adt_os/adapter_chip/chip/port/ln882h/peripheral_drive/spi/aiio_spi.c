/** @brief      SPI application interface.
 *
 *  @file       aiio_spi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>spi init
 *  </table>
 *
 */
#include "aiio_spi.h"
#include "hal/hal_common.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"

#define TIMEOUT_CYCLE 1000
#define AIIO_FUNC_FAIL -3

uint32_t spi_cs_base = 0;
gpio_pin_t spi_cs_pin;

#define LN_SPI_CS_LOW hal_gpio_pin_reset(spi_cs_base, spi_cs_pin)
#define LN_SPI_CS_HIGH hal_gpio_pin_set(spi_cs_base, spi_cs_pin)

static void spi_master_init(aiio_spi_pin_cfg_t *spi_init_cfg)
{
    uint32_t gpio_base = 0;
    if (spi_init_cfg->aiio_gpio_clk_port == AIIO_GPIO_A)
        gpio_base = GPIOA_BASE;
    else if (spi_init_cfg->aiio_gpio_clk_port == AIIO_GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base, spi_init_cfg->aiio_gpio_clk_pin, SPI0_CLK);
    hal_gpio_pin_afio_en(gpio_base, spi_init_cfg->aiio_gpio_clk_pin, HAL_ENABLE);

    if (spi_init_cfg->aiio_gpio_mosi_port == AIIO_GPIO_A)
        gpio_base = GPIOA_BASE;
    else if (spi_init_cfg->aiio_gpio_mosi_port == AIIO_GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base, spi_init_cfg->aiio_gpio_mosi_pin, SPI0_MOSI);
    hal_gpio_pin_afio_en(gpio_base, spi_init_cfg->aiio_gpio_mosi_pin, HAL_ENABLE);

    if (spi_init_cfg->aiio_gpio_miso_port == AIIO_GPIO_A)
        gpio_base = GPIOA_BASE;
    else if (spi_init_cfg->aiio_gpio_miso_port == AIIO_GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base, spi_init_cfg->aiio_gpio_miso_pin, SPI0_MISO);
    hal_gpio_pin_afio_en(gpio_base, spi_init_cfg->aiio_gpio_miso_pin, HAL_ENABLE);

    if (spi_init_cfg->aiio_gpio_cs_port == AIIO_GPIO_A)
        gpio_base = GPIOA_BASE;
    else if (spi_init_cfg->aiio_gpio_cs_port == AIIO_GPIO_B)
        gpio_base = GPIOB_BASE;

    spi_cs_base = gpio_base;
    spi_cs_pin = spi_init_cfg->aiio_gpio_cs_pin;

    gpio_init_t_def gpio_init;
    memset(&gpio_init, 0, sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = spi_cs_pin;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(spi_cs_base, &gpio_init);
    hal_gpio_pin_set(spi_cs_base, spi_cs_pin);

    spi_init_type_def spi_init;
    memset(&spi_init, 0, sizeof(spi_init));

    spi_init.spi_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2;
    spi_init.spi_mode = SPI_MODE_MASTER;
    spi_init.spi_data_size = SPI_DATASIZE_8B;
    spi_init.spi_first_bit = SPI_FIRST_BIT_MSB;
    spi_init.spi_cpol = SPI_CPOL_LOW;
    spi_init.spi_cpha = SPI_CPHA_1EDGE;
    hal_spi_init(SPI0_BASE, &spi_init);

    hal_spi_en(SPI0_BASE, HAL_ENABLE);
    hal_spi_ssoe_en(SPI0_BASE, HAL_DISABLE);
}

static void spi_master_write_and_read_data(uint8_t *send_data, uint8_t *rec_data, uint32_t data_len)
{
    LN_SPI_CS_LOW;
    for (int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);

        if (hal_spi_wait_txe(SPI0_BASE, 1000000) == 1)
            hal_spi_send_data(SPI0_BASE, send_data[i]);
        if (hal_spi_wait_rxne(SPI0_BASE, 1000000) == 1)
            rec_data[i] = hal_spi_recv_data(SPI0_BASE);

        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);
    }
    LN_SPI_CS_HIGH;
}

static void spi_master_write_data_with_addr(uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len)
{
    volatile uint8_t rec_data_buffer[255];

    LN_SPI_CS_LOW;

    for (int i = 0; i < addr_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);

        if (hal_spi_wait_txe(SPI0_BASE, 1000000) == 1)
            hal_spi_send_data(SPI0_BASE, addr[i]);
        if (hal_spi_wait_rxne(SPI0_BASE, 1000000) == 1)
            rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);

        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);
    }
    for (int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);

        if (hal_spi_wait_txe(SPI0_BASE, 1000000) == 1)
            hal_spi_send_data(SPI0_BASE, send_data[i]);
        if (hal_spi_wait_rxne(SPI0_BASE, 1000000) == 1)
            rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);

        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);
    }
    LN_SPI_CS_HIGH;
}

static void spi_master_read_data_with_addr(uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len)
{
    volatile uint8_t buffer[255];
    LN_SPI_CS_LOW;
    for (int i = 0; i < addr_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);

        if (hal_spi_wait_txe(SPI0_BASE, 1000000) == 1)
            hal_spi_send_data(SPI0_BASE, addr[i]);
        if (hal_spi_wait_rxne(SPI0_BASE, 1000000) == 1)
            buffer[0] = hal_spi_recv_data(SPI0_BASE);

        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);
    }
    for (int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);

        if (hal_spi_wait_txe(SPI0_BASE, 1000000) == 1)
            hal_spi_send_data(SPI0_BASE, 0xFF);
        if (hal_spi_wait_rxne(SPI0_BASE, 1000000) == 1)
            rec_data[i] = hal_spi_recv_data(SPI0_BASE);

        hal_spi_wait_bus_idle(SPI0_BASE, 1000000);
    }
    LN_SPI_CS_HIGH;
}

typedef struct
{
    aiio_spi_enum_t aiio_spi_port;
    aiio_spi_pin_cfg_t aiio_spi_cfg;
} aiio_spi_struct;

static aiio_spi_struct aiio_spi_list[4];
static uint8_t aiio_spi_num = 0;

static int32_t aiio_spi_cfg_copy(aiio_spi_pin_cfg_t *aiio_spi_source_cfg, aiio_spi_pin_cfg_t *aiio_spi_target_cfg)
{
    if ((aiio_spi_source_cfg == NULL) || (aiio_spi_target_cfg == NULL))
    {
        return AIIO_ERROR;
    }

    aiio_spi_source_cfg->aiio_gpio_clk_port = aiio_spi_target_cfg->aiio_gpio_clk_port;
    aiio_spi_source_cfg->aiio_gpio_clk_pin = aiio_spi_target_cfg->aiio_gpio_clk_pin;
    aiio_spi_source_cfg->aiio_gpio_mosi_port = aiio_spi_target_cfg->aiio_gpio_mosi_port;
    aiio_spi_source_cfg->aiio_gpio_mosi_pin = aiio_spi_target_cfg->aiio_gpio_mosi_pin;
    aiio_spi_source_cfg->aiio_gpio_miso_port = aiio_spi_target_cfg->aiio_gpio_miso_port;
    aiio_spi_source_cfg->aiio_gpio_miso_pin = aiio_spi_target_cfg->aiio_gpio_miso_pin;
    aiio_spi_source_cfg->aiio_gpio_cs_port = aiio_spi_target_cfg->aiio_gpio_cs_port;
    aiio_spi_source_cfg->aiio_gpio_cs_pin = aiio_spi_target_cfg->aiio_gpio_cs_pin;

    return AIIO_OK;
}

static int32_t aiio_spi_port_cal(aiio_spi_enum_t aiio_spi_port)
{
    uint8_t i = 0;

    for (i = 0; i < aiio_spi_num; i++)
    {
        if (aiio_spi_list[i].aiio_spi_port == aiio_spi_port)
        {
            return i;
        }
    }

    return AIIO_FUNC_FAIL;
}

int32_t aiio_spi_master_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg)
{
    if (aiio_spi_port_cal(aiio_spi_port) != AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_init function aiio_spi_port is repetition!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi_master_init(aiio_spi_cfg);
        break;

    case AIIO_SPI_1:
        aiio_log_e("aiio_spi_master_init function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_init function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_init function AIIO_I2C_SOFT is nonsupport!");
        break;

    default:
        break;
    }

    aiio_spi_list[aiio_spi_num].aiio_spi_port = aiio_spi_port;
    aiio_spi_cfg_copy(&aiio_spi_list[aiio_spi_num], aiio_spi_cfg);
    aiio_spi_num++;

    return AIIO_OK;
}

int32_t aiio_spi_master_deinit(aiio_spi_enum_t aiio_spi_port)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_deinit function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        aiio_log_e("aiio_spi_master_deinit function AIIO_SPI_0 is nonsupport!\r\n");
        break;

    case AIIO_SPI_1:
        aiio_log_e("aiio_spi_master_deinit function AIIO_SPI_1 is nonsupport!\r\n");
        break;

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_deinit function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_deinit function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data, uint8_t *rec_data, uint32_t data_len,uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_and_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi_master_write_and_read_data(send_data, rec_data, data_len);
        break;

    case AIIO_SPI_1:
        aiio_log_e("aiio_spi_master_write_and_read function AIIO_SPI_1 is nonsupport!\r\n");
        break;

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_write_and_read function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_write_and_read function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len,uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi_master_write_data_with_addr(addr,addr_len,send_data,data_len);
        break;

    case AIIO_SPI_1:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_1 is nonsupport!\r\n");
        break;

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_write function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t send_data,uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_write(aiio_spi_port, addr, addr_len, &send_data, 1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len,uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi_master_read_data_with_addr(addr,addr_len,rec_data,data_len);
        break;

    case AIIO_SPI_1:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_1 is nonsupport!");
        break;

    case AIIO_SPI_2:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_2 is nonsupport!");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;

    default:
        aiio_log_e("aiio_spi_master_read function AIIO_SPI_SOFT is nonsupport!");
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_read_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data,uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_read(aiio_spi_port, addr, addr_len, rec_data, 1,timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
