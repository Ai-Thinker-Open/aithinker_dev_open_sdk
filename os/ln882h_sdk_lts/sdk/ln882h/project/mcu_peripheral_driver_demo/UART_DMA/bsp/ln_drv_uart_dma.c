/**
 * @file     ln_drv_uart_dma.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-08
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_uart_dma.h"

void (*uart_dma_recv_it_handler)(void);
void (*uart_dma_send_it_handler)(void);

/**
 * @brief UART初始化
 * 
 * @param uart_x       选择UART通道
 * @param uart_pin_cfg 设置UART引脚
 * @param baud_rate    设置UART波特率
 * @param uart_dma_recv_it_callback 设置DMA接收完成中断回调函数
 * @param uart_dma_send_it_callback 设置DMA发送完成中断回调函数
 */
void uart_dma_init(uart_x_t uart_x ,uart_pin_cfg_t *uart_pin_cfg,uint32_t baud_rate ,
                   void (*uart_dma_recv_it_callback)(void),
                   void (*uart_dma_send_it_callback)(void))
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
    
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + uart_x),     4);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + uart_x));
    hal_uart_it_en(uart_base,USART_IT_RXNE);            //使能接收中断

    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));

    dma_init.dma_data_num = 0;                  //设置DMA数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;   //设置DMA内存地址是否增长
    dma_init.dma_mem_addr = 0;                  //设置DMA内存地址
    dma_init.dma_dir = DMA_READ_FORM_MEM;       //设置DMA传输方向
    switch(uart_x)
    {
        case UART_0: dma_init.dma_p_addr = UART0_TX_DATA_REG;  break;
        case UART_1: dma_init.dma_p_addr = UART1_TX_DATA_REG;  break;
        case UART_2: dma_init.dma_p_addr = UART2_TX_DATA_REG;  break;
    }
    hal_dma_init(DMA_CH_6,&dma_init);           //初始化DMA

    hal_uart_dma_en(UART0_BASE,USART_DMA_REQ_TX,HAL_ENABLE);        //设置UART 发送 DMA使能

    memset(&dma_init,0,sizeof(dma_init));
    dma_init.dma_data_num = 0;                  //设置DMA数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;   //设置DMA内存地址是否增长
    dma_init.dma_mem_addr = 0;                  //设置DMA内存地址
    dma_init.dma_dir = DMA_READ_FORM_P;         //设置DMA传输方向
    switch(uart_x)
    {
        case UART_0: dma_init.dma_p_addr = UART0_RX_DATA_REG;  break;
        case UART_1: dma_init.dma_p_addr = UART1_RX_DATA_REG;  break;
        case UART_2: dma_init.dma_p_addr = UART2_RX_DATA_REG;  break;
    }
    hal_dma_init(DMA_CH_5,&dma_init);           //初始化DMA

    hal_uart_dma_en(UART0_BASE,USART_DMA_REQ_RX,HAL_ENABLE);       //设置UART 接收 DMA使能
    hal_dma_it_cfg(DMA_CH_5,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);     //使能DMA传输完成中断
    hal_dma_it_cfg(DMA_CH_6,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);     //使能DMA传输完成中断
    NVIC_EnableIRQ(DMA_IRQn);                                      //使能DMA中断 

    if(uart_dma_recv_it_callback != NULL) 
        uart_dma_recv_it_handler = uart_dma_recv_it_callback;
    if(uart_dma_send_it_callback != NULL) 
        uart_dma_send_it_handler = uart_dma_send_it_callback;
}

/**
 * @brief UART发送数据
 * 
 * @param data 设置UART发送数据指针
 * @param len  设置要数据的数据长度
 */
void uart_dma_send_data(uint8_t *data,uint32_t len)
{
    hal_dma_en(DMA_CH_6,HAL_DISABLE);  
    hal_dma_set_data_num(DMA_CH_6,len);
    hal_dma_set_mem_addr(DMA_CH_6,(uint32_t)data);
    hal_dma_en(DMA_CH_6,HAL_ENABLE);  
}

/**
 * @brief UART接收数据
 * 
 * @param data 设置接收数据指针
 * @param len  设置要接收的数据长度
 */
void uart_dma_recv_data(uint8_t *data,uint32_t len)
{
    hal_dma_en(DMA_CH_5,HAL_DISABLE);  
    hal_dma_set_data_num(DMA_CH_5,len);
    hal_dma_set_mem_addr(DMA_CH_5,(uint32_t)data);
    hal_dma_en(DMA_CH_5,HAL_ENABLE);  
}

void DMA_IRQHandler()
{
    if(hal_dma_get_it_flag(DMA_CH_5,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)
    {
        hal_dma_clr_it_flag(DMA_CH_5,DMA_IT_FLAG_TRAN_COMP);
        if(uart_dma_recv_it_handler != NULL)
            uart_dma_recv_it_handler();
    }
    if(hal_dma_get_it_flag(DMA_CH_6,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)
    {
        hal_dma_clr_it_flag(DMA_CH_6,DMA_IT_FLAG_TRAN_COMP);
        if(uart_dma_send_it_handler != NULL)
            uart_dma_send_it_handler();
    }

}

