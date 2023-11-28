/**
 * @file     main.c
 * @author   BSP Team 
 * @brief    
 * @version  0.0.0.1
 * @date     2021-08-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "hal/hal_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

#include "ln_drv_uart_dma.h"

void uart_dma_send_callback(void);
void uart_dma_recv_callback(void);

uint8_t send_data[100] = "UART TEST";
uint8_t recv_data[100];

int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    ln_show_reg_init();

    /****************** 2. UART 测试***********************/  
    uart_pin_cfg_t  uart_pin_cfg;
    memset(&uart_pin_cfg,0,sizeof(uart_pin_cfg_t));
    
    uart_pin_cfg.uart_tx_port = GPIO_B;
    uart_pin_cfg.uart_tx_pin  = GPIO_PIN_9;
    uart_pin_cfg.uart_rx_port = GPIO_B;
    uart_pin_cfg.uart_rx_pin  = GPIO_PIN_8;
    
    uart_dma_init(UART_0,&uart_pin_cfg,115200,uart_dma_recv_callback,uart_dma_send_callback);
    uart_dma_recv_data(recv_data,10);
    uart_dma_send_data(send_data,9);
    while(1)
    {
        ln_delay_ms(1000);
    }
}

void uart_dma_send_callback()
{
    uart_dma_send_data(send_data,9);
}

void uart_dma_recv_callback()
{
    uart_dma_recv_data(recv_data,10);
}
