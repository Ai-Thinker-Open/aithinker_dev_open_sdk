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
#include "pinctrl.h"
#include "spi.h"
#include "soc_osal.h"
#include "app_init.h"

#define SPI_SLAVE_NUM 1
#define SPI_FREQUENCY 2
#define SPI_CLK_POLARITY 0
#define SPI_CLK_PHASE 0
#define SPI_FRAME_FORMAT 0
#define SPI_FRAME_FORMAT_STANDARD 0
#define SPI_TMOD 0
#define SPI_WAIT_CYCLES 0x10


static errcode_t bs21_master_spi_writeread(spi_bus_t bus, uint8_t *send_data, uint8_t *rec_data, uint32_t r_len)
{
    spi_xfer_data_t spi_data = {0};

    spi_data.tx_buff = send_data;
    spi_data.tx_bytes = r_len;
    spi_data.rx_buff = rec_data;
    spi_data.rx_bytes = r_len;

    return uapi_spi_master_writeread(bus, &spi_data, 0xFFFFFFFF); /* 发送数据 */
}

int32_t aiio_spi_master_init(aiio_spi_enum_t aiio_spi_port, aiio_spi_pin_cfg_t *aiio_spi_cfg)
{
    aiio_log_w("aiio_spi_master_init success");
    spi_attr_t config = {0};
    spi_extra_attr_t ext_config = {0};

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_cs_pin, HAL_PIO_SPI0_CS0);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_clk_pin, HAL_PIO_SPI0_SCLK);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_miso_pin, HAL_PIO_SPI0_RXD);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_mosi_pin, HAL_PIO_SPI0_TXD);

        config.is_slave = false;
        config.slave_num = SPI_SLAVE_NUM;
        config.bus_clk = SPI_CLK_FREQ;
        config.freq_mhz = SPI_FREQUENCY;
        config.clk_polarity = SPI_CLK_POLARITY;
        config.clk_phase = SPI_CLK_PHASE;
        config.frame_format = SPI_FRAME_FORMAT;
        config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;
        config.frame_size = HAL_SPI_FRAME_SIZE_8;
        config.tmod = SPI_TMOD;
        config.sste = 0;

        ext_config.qspi_param.wait_cycles = SPI_WAIT_CYCLES;

        uapi_spi_init(SPI_BUS_0, &config, &ext_config);

        break;
    case AIIO_SPI_1:
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_cs_pin, HAL_PIO_SPI1_CS0);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_clk_pin, HAL_PIO_SPI1_CLK);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_miso_pin, HAL_PIO_SPI1_RXD);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_mosi_pin, HAL_PIO_SPI1_TXD);

        config.is_slave = false;
        config.slave_num = SPI_SLAVE_NUM;
        config.bus_clk = SPI_CLK_FREQ;
        config.freq_mhz = SPI_FREQUENCY;
        config.clk_polarity = SPI_CLK_POLARITY;
        config.clk_phase = SPI_CLK_PHASE;
        config.frame_format = SPI_FRAME_FORMAT;
        config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;
        config.frame_size = HAL_SPI_FRAME_SIZE_8;
        config.tmod = SPI_TMOD;
        config.sste = 0;

        ext_config.qspi_param.wait_cycles = SPI_WAIT_CYCLES;

        uapi_spi_init(SPI_BUS_1, &config, &ext_config);
        break;
    case AIIO_SPI_2:
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_cs_pin, HAL_PIO_SPI2_CS0);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_clk_pin, HAL_PIO_SPI2_CLK);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_miso_pin, HAL_PIO_SPI2_RXD);
        uapi_pin_set_mode(aiio_spi_cfg->aiio_gpio_mosi_pin, HAL_PIO_SPI2_TXD);

        config.is_slave = false;
        config.slave_num = SPI_SLAVE_NUM;
        config.bus_clk = SPI_CLK_FREQ;
        config.freq_mhz = SPI_FREQUENCY;
        config.clk_polarity = SPI_CLK_POLARITY;
        config.clk_phase = SPI_CLK_PHASE;
        config.frame_format = SPI_FRAME_FORMAT;
        config.spi_frame_format = HAL_SPI_FRAME_FORMAT_STANDARD;
        config.frame_size = HAL_SPI_FRAME_SIZE_8;
        config.tmod = SPI_TMOD;
        config.sste = 0;

        ext_config.qspi_param.wait_cycles = SPI_WAIT_CYCLES;

        uapi_spi_init(SPI_BUS_2, &config, &ext_config);
        break;
    default:
        break;
    }
    // 引脚跟模式

    return AIIO_OK;
}

int32_t aiio_spi_master_deinit(aiio_spi_enum_t aiio_spi_port)
{
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        uapi_spi_deinit(SPI_BUS_0);
        break;
    case AIIO_SPI_1:
        uapi_spi_deinit(SPI_BUS_1);
        break;
    case AIIO_SPI_2:
        uapi_spi_deinit(SPI_BUS_2);
        break;
    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{
    timeout = timeout;

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        if (bs21_master_spi_writeread(SPI_BUS_0, send_data, rec_data, data_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    case AIIO_SPI_1:
        if (bs21_master_spi_writeread(SPI_BUS_1, send_data, rec_data, data_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    case AIIO_SPI_2:
        if (bs21_master_spi_writeread(SPI_BUS_2, send_data, rec_data, data_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len, uint32_t timeout)
{
    uint8_t *s_buf = (uint8_t *)osal_kmalloc(sizeof(uint8_t) * (addr_len + data_len),0);
    uint8_t *r_buf = (uint8_t *)osal_kmalloc(sizeof(uint8_t) * (addr_len + data_len),0);
    for (uint32_t i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }
    for (uint32_t i = 0; i < data_len; i++)
    {
        s_buf[addr_len + i] = send_data[i];
    }
    timeout = timeout;

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        if (bs21_master_spi_writeread(SPI_BUS_0, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    case AIIO_SPI_1:
        if (bs21_master_spi_writeread(SPI_BUS_1, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    case AIIO_SPI_2:
        if (bs21_master_spi_writeread(SPI_BUS_2, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        break;
    default:
        break;
    }

    osal_kfree(s_buf);
    osal_kfree(r_buf);

    return AIIO_OK;
}

int32_t aiio_spi_master_write_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t send_data, uint32_t timeout)
{
    if (aiio_spi_master_write(aiio_spi_port, addr, addr_len, &send_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{

    uint8_t *s_buf = (uint8_t *)osal_kmalloc(sizeof(uint8_t) * (addr_len + data_len),0);
    uint8_t *r_buf = (uint8_t *)osal_kmalloc(sizeof(uint8_t) * (addr_len + data_len),0);
    for (uint32_t i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }
    timeout = timeout;

    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        if (bs21_master_spi_writeread(SPI_BUS_0, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        for (uint32_t i = 0; i < data_len; i++)
        {
            rec_data[i] = r_buf[i + addr_len];
        }
        break;
    case AIIO_SPI_1:
        if (bs21_master_spi_writeread(SPI_BUS_1, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        for (uint32_t i = 0; i < data_len; i++)
        {
            rec_data[i] = r_buf[i + addr_len];
        }
        break;
    case AIIO_SPI_2:
        if (bs21_master_spi_writeread(SPI_BUS_2, (uint8_t *)s_buf, (uint8_t *)r_buf, data_len + addr_len) != ERRCODE_SUCC)
        {
            return AIIO_ERROR;
        }
        for (uint32_t i = 0; i < data_len; i++)
        {
            rec_data[i] = r_buf[i + addr_len];
        }
        break;
    default:
        break;
    }

    osal_kfree(s_buf);
    osal_kfree(r_buf);

    return AIIO_OK;
}

int32_t aiio_spi_master_read_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t timeout)
{
    if (aiio_spi_master_read(aiio_spi_port, addr, addr_len, rec_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
