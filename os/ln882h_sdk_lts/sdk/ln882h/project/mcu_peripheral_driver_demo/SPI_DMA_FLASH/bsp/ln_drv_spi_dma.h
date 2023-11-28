/**
 * @file     ln_drv_spi_dma.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_SPI_DMA_H
#define __LN_DRV_SPI_DMA_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"

#include "ln_test_common.h"

typedef enum
{
    GPIO_A = 0,
    GPIO_B = 1,
}gpio_port_t;


typedef struct
{
    gpio_port_t gpio_clk_port;
    gpio_pin_t  gpio_clk_pin;
    gpio_port_t gpio_mosi_port;
    gpio_pin_t  gpio_mosi_pin;
    gpio_port_t gpio_miso_port;
    gpio_pin_t  gpio_miso_pin;
    gpio_port_t gpio_cs_port;
    gpio_pin_t  gpio_cs_pin;
}spi_pin_cfg_t;


void spi_dma_master_init(spi_pin_cfg_t *spi_pin_cfg);

void spi_dma_master_write_and_read_data(uint8_t *send_data,uint8_t *rec_data,uint32_t data_len);

void spi_dma_master_write_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *send_data,uint32_t data_len);

void spi_dma_master_read_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *rec_data,uint32_t data_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_SPI_DMA_H
