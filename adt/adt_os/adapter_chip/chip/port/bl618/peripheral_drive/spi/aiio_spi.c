/** @brief      SPI application interface.
 *
 *  @file       aiio_spi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       SPI application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/08/01      <td>1.0.0       <td>lzy         <td>Adapts to SPI interface
 *  <tr><td>2023/08/03      <td>1.0.1       <td>lzy         <td>Remove redundant programs
 *  </table>
 *
 */
#include <bflb_gpio.h>
#include "aiio_spi.h"
#include <bflb_spi.h>
#include <board.h>

#define AIIO_FUNC_FAIL   -3
#define SPI_MODE0         0                  /**< spi communication is master mode */
typedef void (*hosal_spi_irq_t)(void *parg); /**< spi irq callback function */

struct bflb_spi_config {
    uint32_t freq;                  /*SPI frequence, should be less than spi_clk/2*/
    uint8_t role;                   /*SPI role*/
    uint8_t mode;                   /*SPI mode*/
    uint8_t data_width;             /*SPI data width*/
    uint8_t bit_order;              /*SPI bit order*/
    uint8_t byte_order;             /*SPI byte order*/
    uint8_t tx_fifo_threshold;      /*SPI tx fifo threshold, should be less than 4*/
    uint8_t rx_fifo_threshold;      /*SPI rx fifo threshold, should be less than 4*/
};

typedef struct
{
    aiio_spi_enum_t aiio_spi_port;
    aiio_spi_pin_cfg_t aiio_spi_cfg;
} aiio_spi_struct;

static aiio_spi_struct aiio_spi_list[4];
static uint8_t aiio_spi_num = 0;

struct bflb_device_s *spi0_dev = NULL;
struct bflb_device_s *spi_gpio = NULL;

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
    {
        struct bflb_spi_config spi_cfg = {
            .freq = 1000000,
            .role = SPI_ROLE_MASTER,
            .mode = SPI_MODE0,
            .data_width = SPI_DATA_WIDTH_8BIT,
            .bit_order = SPI_BIT_MSB,
            .byte_order = SPI_BYTE_LSB,
            .tx_fifo_threshold = 0,
            .rx_fifo_threshold = 0,
        };

        /*spi gpio set*/
        spi_gpio = bflb_device_get_by_name("gpio");
        bflb_gpio_init(spi_gpio,aiio_spi_cfg->aiio_gpio_cs_pin, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
        bflb_gpio_init(spi_gpio,aiio_spi_cfg->aiio_gpio_clk_pin, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
        bflb_gpio_init(spi_gpio,aiio_spi_cfg->aiio_gpio_miso_pin, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
        bflb_gpio_init(spi_gpio,aiio_spi_cfg->aiio_gpio_mosi_pin, GPIO_FUNC_SPI0 | GPIO_ALTERNATE | GPIO_PULLUP | GPIO_SMT_EN | GPIO_DRV_1);
        aiio_log_d("bflb_gpio_init successful!!");
        
        spi0_dev= bflb_device_get_by_name("spi0");
        bflb_spi_init( spi0_dev, &spi_cfg);
        aiio_log_d("bflb_spi_init Successful!!\r\n");
        /*Set CS to low level*/
        bflb_spi_feature_control(spi0_dev, SPI_CMD_SET_CS_INTERVAL, 1);

        break;
    }
    case AIIO_SPI_1:
        aiio_log_w("aiio_spi_master_init function AIIO_I2C_1 is nonsupport!");
        break;

    case AIIO_SPI_2:
        aiio_log_w("aiio_spi_master_init function AIIO_I2C_2 is nonsupport!");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_w("aiio_spi_master_init function AIIO_I2C_SOFT is nonsupport!");
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
        // spi0 = bflb_device_get_by_name("spi0");
        bflb_spi_deinit(spi0_dev);
        break;

    case AIIO_SPI_1:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_1 is nonsupport!\r\n");
        break;

    case AIIO_SPI_2:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_w("aiio_spi_master_deinit function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write_and_read(aiio_spi_enum_t aiio_spi_port, uint8_t *send_data, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_and_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        
        spi0_dev = bflb_device_get_by_name("spi0");
        /* set data width */
        bflb_spi_feature_control(spi0_dev, SPI_CMD_SET_DATA_WIDTH, SPI_DATA_WIDTH_8BIT);
        bflb_spi_poll_exchange(spi0_dev,(uint8_t *)send_data,(uint8_t *)rec_data,data_len);
        break;

    case AIIO_SPI_1:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_1 is nonsupport!\r\n");
        break;

    case AIIO_SPI_2:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_2 is nonsupport!\r\n");
        break;

    case AIIO_SPI_SOFT:
        aiio_log_w("aiio_spi_master_write_and_read function AIIO_SPI_SOFT is nonsupport!\r\n");
        break;

    default:
        break;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_write(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *send_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    uint8_t *s_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    uint8_t *r_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    for (int i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }
    for (int i = 0; i < data_len; i++)
    {
        s_buf[addr_len + i] = send_data[i];
    }
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi0_dev = bflb_device_get_by_name("spi0");
        bflb_spi_poll_exchange(spi0_dev,s_buf,r_buf,data_len + addr_len);
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
    free(s_buf);
    free(r_buf);
    s_buf = NULL;
    r_buf = NULL;
    return AIIO_OK;
}

int32_t aiio_spi_master_read(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t data_len, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    uint8_t *s_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    uint8_t *r_buf = (uint8_t *)malloc(sizeof(uint8_t) * (addr_len + data_len));
    for (int i = 0; i < addr_len; i++)
    {
        s_buf[i] = addr[i];
    }
    switch (aiio_spi_port)
    {
    case AIIO_SPI_0:
        spi0_dev = bflb_device_get_by_name("spi0");
        bflb_spi_poll_exchange(spi0_dev,s_buf,r_buf,data_len + addr_len);
        for (int i = 0; i < data_len; i++)
        {
            rec_data[i] = r_buf[i + addr_len];
        }
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
    free(s_buf);
    free(r_buf);
    s_buf = NULL;
    r_buf = NULL;
    return AIIO_OK;
}

int32_t aiio_spi_master_write_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t send_data, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_write_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_write(aiio_spi_port, addr, addr_len, &send_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

int32_t aiio_spi_master_read_byte(aiio_spi_enum_t aiio_spi_port, uint8_t *addr, uint32_t addr_len, uint8_t *rec_data, uint32_t timeout)
{
    if (aiio_spi_port_cal(aiio_spi_port) == AIIO_FUNC_FAIL)
    {
        aiio_log_e("aiio_spi_master_read_byte function aiio_spi_port is nonsupport!");
        return AIIO_ERROR;
    }

    if (aiio_spi_master_read(aiio_spi_port, addr, addr_len, rec_data, 1, timeout) != AIIO_OK)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
