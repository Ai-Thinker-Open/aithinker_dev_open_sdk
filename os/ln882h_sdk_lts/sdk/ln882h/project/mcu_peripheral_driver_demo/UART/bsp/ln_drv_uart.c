/**
 * @file     ln_drv_uart.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-08
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_uart.h"

void (*uart0_recv_it_handler)(void);
void (*uart1_recv_it_handler)(void);
void (*uart2_recv_it_handler)(void);

/**
 * @brief UART 初始化
 * 
 * @param uart_x            选择UART通道
 * @param uart_pin_cfg      配置UART引脚
 * @param baud_rate         设置UART波特率
 * @param uart_it_callback  设置UART接收中断回调函数
 */
void uart_init(uart_x_t uart_x,uart_pin_cfg_t *uart_pin_cfg,uint32_t baud_rate,void (*uart_recv_it_callback)(void))
{
    uint32_t uart_base = 0;
    uint32_t gpio_base = 0;
    switch(uart_x)
    {
        case UART_0: uart_base = UART0_BASE; break;
        case UART_1: uart_base = UART1_BASE; break;
        case UART_2: uart_base = UART2_BASE; break;
    }

    if(uart_pin_cfg->uart_tx_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(uart_pin_cfg->uart_tx_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    if(uart_x == UART_0)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_tx_pin,(afio_function_t)(UART0_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_tx_pin,HAL_ENABLE);
    }
    else if(uart_x == UART_1)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_tx_pin,(afio_function_t)(UART1_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_tx_pin,HAL_ENABLE);
    }
    else if(uart_x == UART_2)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_tx_pin,(afio_function_t)(UART2_TX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_tx_pin,HAL_ENABLE);
    }

    if(uart_pin_cfg->uart_rx_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(uart_pin_cfg->uart_rx_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    if(uart_x == UART_0)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_rx_pin,(afio_function_t)(UART0_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_rx_pin,HAL_ENABLE);
    }
    else if(uart_x == UART_1)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_rx_pin,(afio_function_t)(UART1_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_rx_pin,HAL_ENABLE);
    }
    else if(uart_x == UART_2)
    {
        hal_gpio_pin_afio_select(gpio_base,uart_pin_cfg->uart_rx_pin,(afio_function_t)(UART2_RX));
        hal_gpio_pin_afio_en(gpio_base,uart_pin_cfg->uart_rx_pin,HAL_ENABLE);
    }

    uart_init_t_def uart_init_struct;
    memset(&uart_init_struct,0,sizeof(uart_init_struct));
    
    uart_init_struct.baudrate = baud_rate;              //设置波特率
    uart_init_struct.parity = UART_PARITY_NONE;         //设置校验位
    uart_init_struct.stop_bits = UART_STOP_BITS_1;      //设置停止位
    uart_init_struct.word_len = UART_WORD_LEN_8;        //设置数据长度
    
    hal_uart_init(uart_base,&uart_init_struct);         //初始化UART寄存器
    hal_uart_rx_mode_en(uart_base,HAL_ENABLE);          //使能发送模式
    hal_uart_tx_mode_en(uart_base,HAL_ENABLE);          //使能接收模式
    hal_uart_en(uart_base,HAL_ENABLE);                  //使能UART模块
    
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + uart_x), 4);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + uart_x));
    hal_uart_it_en(uart_base,USART_IT_RXNE);           //使能接收中断

    switch(uart_x)
    {
        case UART_0: if(uart_recv_it_callback != NULL) uart0_recv_it_handler = uart_recv_it_callback; break;
        case UART_1: if(uart_recv_it_callback != NULL) uart1_recv_it_handler = uart_recv_it_callback; break;
        case UART_2: if(uart_recv_it_callback != NULL) uart2_recv_it_handler = uart_recv_it_callback; break;
    }
}

/**
 * @brief UART发送数据
 * 
 * @param uart_x 选择UART通道
 * @param data   设置发送指针
 */
void uart_send_data(uart_x_t uart_x,uint8_t data)
{
    uint32_t uart_base = 0;
    switch(uart_x)
    {
        case UART_0: uart_base = UART0_BASE; break;
        case UART_1: uart_base = UART1_BASE; break;
        case UART_2: uart_base = UART2_BASE; break;
    }
    hal_uart_send_data(uart_base,data);
}

/**
 * @brief 设置UART接收指针
 * 
 * @param uart_x 选择UART通道
 */
uint16_t uart_recv_data(uart_x_t uart_x)
{
    uint32_t uart_base = 0;
    switch(uart_x)
    {
        case UART_0: uart_base = UART0_BASE; break;
        case UART_1: uart_base = UART1_BASE; break;
        case UART_2: uart_base = UART2_BASE; break;
    }
    return hal_uart_recv_data(uart_base);
}

/**
 * @brief 获取UART RX缓冲区非空标志位
 * 
 * @param uart_x 
 * @return uint8_t 
 */
uint8_t uart_get_rx_not_empty_flag(uart_x_t uart_x)
{
    uint32_t uart_base = 0;
    switch(uart_x)
    {
        case UART_0: uart_base = UART0_BASE; break;
        case UART_1: uart_base = UART1_BASE; break;
        case UART_2: uart_base = UART2_BASE; break;
    }
    return hal_uart_flag_get(uart_base,USART_FLAG_RXNE);
}

/**
 * @brief 获取UART TX缓冲区为空标志位
 * 
 * @param uart_x 
 * @return uint8_t 
 */
uint8_t uart_get_tx_empty_flag(uart_x_t uart_x)
{
    uint32_t uart_base = 0;
    switch(uart_x)
    {
        case UART_0: uart_base = UART0_BASE; break;
        case UART_1: uart_base = UART1_BASE; break;
        case UART_2: uart_base = UART2_BASE; break;
    }
    return hal_uart_flag_get(uart_base,USART_FLAG_TXE);
}

/**
 * @brief UART0中断服务函数
 */
void UART0_IRQHandler()
{
    if(hal_uart_flag_get(UART0_BASE,USART_FLAG_RXNE) && hal_uart_it_en_status_get(UART0_BASE,USART_IT_RXNE))
    {
        if(uart0_recv_it_handler != NULL)
            uart0_recv_it_handler();
    }
}

/**
 * @brief UART1中断服务函数
 */
void UART1_IRQHandler()
{
    if(hal_uart_flag_get(UART1_BASE,USART_FLAG_RXNE) && hal_uart_it_en_status_get(UART1_BASE,USART_IT_RXNE))
    {
        if(uart1_recv_it_handler != NULL)
            uart1_recv_it_handler();
    }
}

/**
 * @brief UART2中断服务函数
 */
void UART2_IRQHandler()
{
    if(hal_uart_flag_get(UART2_BASE,USART_FLAG_RXNE) && hal_uart_it_en_status_get(UART2_BASE,USART_IT_RXNE))
    {
        if(uart2_recv_it_handler != NULL)
            uart2_recv_it_handler();
    }
}
