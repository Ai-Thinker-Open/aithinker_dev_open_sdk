/**
 * @file     ln_drv_uart_dma.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-08
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_UART_DMA_H
#define __LN_DRV_UART_DMA_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal_uart.h"
#include "hal_gpio.h"
#include "hal_dma.h"

typedef enum
{
    UART_0,
    UART_1,
    UART_2,
}uart_x_t;

typedef enum
{
    GPIO_A = 0,
    GPIO_B = 1,
}gpio_port_t;

typedef struct
{
    gpio_port_t uart_tx_port;
    gpio_pin_t  uart_tx_pin;
    gpio_port_t uart_rx_port;
    gpio_pin_t  uart_rx_pin;
}uart_pin_cfg_t;

void uart_dma_init(uart_x_t uart_x ,uart_pin_cfg_t *uart_pin_cfg,uint32_t baud_rate ,
                   void (*uart_dma_recv_it_callback)(void),
                   void (*uart_dma_send_it_callback)(void));
void uart_dma_send_data(uint8_t *data,uint32_t len);
void uart_dma_recv_data(uint8_t *data,uint32_t len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_UART_DMA_H


