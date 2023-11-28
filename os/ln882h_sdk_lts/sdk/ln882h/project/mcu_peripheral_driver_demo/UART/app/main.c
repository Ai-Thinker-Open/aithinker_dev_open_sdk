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

#include "ln_drv_uart.h"

void uart0_recv_callback(void);
void uart1_recv_callback(void);
void uart2_recv_callback(void);

char send_data[100] = "UART TEST";
char recv_data[100];
uint32_t recv_cnt = 0;
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
    
    uart_init(UART_0,&uart_pin_cfg,115200,uart0_recv_callback);

    while(1)
    {
        for(int i = 0; i < strlen(send_data); i ++){
            while(uart_get_tx_empty_flag(UART_0) != HAL_SET);
            uart_send_data(UART_0,send_data[i]);
        }
        ln_delay_ms(1000);
    }
}

void uart0_recv_callback(void)
{
    if(uart_get_rx_not_empty_flag(UART_0) == HAL_SET){
        recv_data[recv_cnt++] = uart_recv_data(UART_0);
    }
}

void uart1_recv_callback(void)
{
   
}

void uart2_recv_callback(void)
{
   
}

