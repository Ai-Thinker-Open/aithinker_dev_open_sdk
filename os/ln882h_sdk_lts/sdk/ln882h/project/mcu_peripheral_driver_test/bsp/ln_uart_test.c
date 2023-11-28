/**
 * @file     ln_uart_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-18
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
   UART测试说明：
   
    1. UART使用的是UART0，波特率115200，8位数据长度，一位停止位，无校验位，引脚如下：
   
                                    PA10     UART0_TX 
                                    PA11     UART0_RX   
                                    PA12     UART0_RTS 
                                    PA13     UART0_CTS   
                                    
                                  
    2. 由于UART0的中断和原本系统的LOG打印使用的中断相冲突，所以使用本测试文件的时候需要先在serial_hw.c 232行注释掉 UART0_IRQHandler函数。
    
       同时在本文件的最下方取消掉UART0_IRQHandler的注释。(DMA中断也可能会有类似的问题，请根据实际情况调整)
       

    3. 本测试有两种情况，一种是普通的UART发送接收，一种是UART+DMA,可以通过使能或者失能 "LN_UART_DMA_SEND_EN" 和 "LN_UART_DMA_RECV_EN"，来打开或者关闭UART DMA 接收和发送。
    
      
    4. DMA接收，使用了双缓冲BUFFER，便于提升效率

*/

#include "hal/hal_uart.h"
#include "hal/hal_dma.h"
#include "hal/hal_gpio.h"
#include "ln_test_common.h"
#include "ln_uart_test.h"
#include "hal/hal_common.h"

#define LN_UART_DMA_SEND_EN     1          //uart发送DMA使能
#define LN_UART_DMA_RECV_EN     1          //uart接收DMA使能


static volatile unsigned char tx_data[100];         //uart发送buffer
static volatile unsigned char rx_data_1[100];       //uart接收buffer1
static volatile unsigned char rx_data_2[100];       //uart接收buffer2

static volatile unsigned int tx_data_cnt = 0;       //发送计数
static volatile unsigned int rx_data_cnt = 0;       //接收计数
static volatile unsigned int int_cnt_1 = 0;                  //中断计数器，调试使用
static volatile unsigned int int_cnt_2 = 0;                  //中断计数器，调试使用

static volatile unsigned int   data_num = 100;         //DMA传输次数
static volatile unsigned char  data_sel = 0;           //双缓冲BUFFER选择
static volatile unsigned char  data_comp= 0;           //数据接收完成标志位


void ln_uart_init(void)
{
    /* 1. 初始化UART0引脚 */
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_11,UART0_TX);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_12,UART0_RX);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_3,UART0_RTS);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_4,UART0_CTS);

    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_11,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_12,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_3,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_4,HAL_ENABLE);


    /* 2. 初始化UART0配置 */
    uart_init_t_def uart_init_struct;
    memset(&uart_init_struct,0,sizeof(uart_init_struct));
    
    uart_init_struct.baudrate = 115200;                 //设置波特率
    uart_init_struct.parity = UART_PARITY_NONE;         //设置校验位
    uart_init_struct.stop_bits = UART_STOP_BITS_1;      //设置停止位
    uart_init_struct.word_len = UART_WORD_LEN_8;        //设置数据长度
    
    hal_uart_init(UART0_BASE,&uart_init_struct);        //初始化UART寄存器
    hal_uart_rx_mode_en(UART0_BASE,HAL_ENABLE);     //使能发送模式
    hal_uart_tx_mode_en(UART0_BASE,HAL_ENABLE);     //使能接收模式
    hal_uart_en(UART0_BASE,HAL_ENABLE);             //使能UART模块

    //uart_hardware_flow_rts_enable(UART0_BASE,HAL_ENABLE);     //UART0 RTS配置
    //uart_hardware_flow_cts_enable(UART0_BASE,HAL_ENABLE);     //UART0 CTS配置
    
    /* 3. 初始化UART0中断 */
    NVIC_SetPriority(UART0_IRQn,     4);
    NVIC_EnableIRQ(UART0_IRQn);
    hal_uart_it_en(UART0_BASE,USART_IT_RXNE);       //使能接收中断
}

void ln_uart_dma_init()
{
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));

#if LN_UART_DMA_SEND_EN == 1

    /* 1.初始化UART发送DMA */
    dma_init.dma_data_num = 0;                  //设置DMA数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;   //设置DMA内存地址是否增长
    dma_init.dma_mem_addr = (uint32_t)tx_data;  //设置DMA内存地址
    dma_init.dma_dir = DMA_READ_FORM_MEM;       //设置DMA传输方向
    dma_init.dma_p_addr = UART0_TX_DATA_REG;    //设置DMA物理地址
    hal_dma_init(DMA_CH_6,&dma_init);           //初始化DMA

    hal_uart_dma_en(UART0_BASE,USART_DMA_REQ_TX,HAL_ENABLE);        //设置UART 发送 DMA使能
#endif
    
#if LN_UART_DMA_RECV_EN == 1
    /* 2.初始化UART接收DMA */
    memset(&dma_init,0,sizeof(dma_init));
    dma_init.dma_data_num = data_num;           //设置DMA数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;   //设置DMA内存地址是否增长
    dma_init.dma_mem_addr = (uint32_t)rx_data_1;//设置DMA内存地址
    dma_init.dma_dir = DMA_READ_FORM_P;         //设置DMA传输方向
    dma_init.dma_p_addr = UART0_RX_DATA_REG;    //设置DMA物理地址
    hal_dma_init(DMA_CH_5,&dma_init);           //初始化DMA

    hal_uart_dma_en(UART0_BASE,USART_DMA_REQ_RX,HAL_ENABLE);   //设置UART 接收 DMA使能
    hal_dma_it_cfg(DMA_CH_5,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);     //使能DMA传输完成中断
    NVIC_EnableIRQ(DMA_IRQn);                                      //使能DMA中断 
    
    hal_dma_en(DMA_CH_5,HAL_ENABLE);  
#endif

    /* 3.初始化双缓冲buffer设置 */
    data_sel = 2;                               //设置双缓冲下一个BUFFER
    data_comp = 0;                              //设置传输完成标志位
}

void ln_uart_test()
{
    ln_uart_init();         //初始化UART
    ln_uart_dma_init();     //初始化UART DMA

    /* 给要发送的数据buff赋值 */
    for(int i = 0;i < 100; i++)
    {
        tx_data[i] = i ;
    }

    while(1)
    {

#if (LN_UART_DMA_SEND_EN == 1)
        //DMA方式发送数据
        {
            //先失能DMA，然后才能配置DMA相关参数。
            hal_dma_en(DMA_CH_6,HAL_DISABLE);                               //失能DMA（为了配置参数）
            hal_dma_set_mem_addr(DMA_CH_6,(uint32_t)(tx_data));             //重新设置DMA地址
            hal_dma_set_data_num(DMA_CH_6,data_num);                        //设置DMA数据长度
            hal_dma_en(DMA_CH_6,HAL_ENABLE);                                //设置DMA使能
            while(hal_dma_get_data_num(DMA_CH_6));                          //等待数据发送完成
            while(hal_dma_get_it_flag(DMA_CH_6,DMA_IT_FLAG_TRAN_COMP));     //等待数据发送完成
        }
#else
        //普通方式发送数据
        {
            for(int i = 0; i < 100; i++)
            {
                while(!hal_uart_flag_get(UART0_BASE,USART_FLAG_TXE));       //等待发送完成
                hal_uart_send_data(UART0_BASE,tx_data[i]);                  //发送数据
            }
        }
#endif
        ln_delay_ms(2000);

    }

}

#if LN_UART_DMA_RECV_EN == 1

//DMA双缓冲接收数据
//void DMA_IRQHandler()
//{
//    if(hal_dma_get_it_flag(DMA_CH_5,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)
//    {
//        if(data_sel == 1)
//        {
//            int_cnt_1++;
//            data_sel = 2;
//            hal_dma_en(DMA_CH_5,HAL_DISABLE);                                 //失能DMA（为了配置参数）
//            hal_dma_set_mem_addr(DMA_CH_5,(uint32_t)(rx_data_1));             //重新设置DMA地址
//            hal_dma_set_data_num(DMA_CH_5,data_num);                          //设置DMA数据长度
//            hal_dma_en(DMA_CH_5,HAL_ENABLE);                                  //设置DMA使能
//        }
//        else
//        {
//            int_cnt_2++;
//            data_sel = 1;
//            hal_dma_en(DMA_CH_5,HAL_DISABLE);                                 //失能DMA（为了配置参数）
//            hal_dma_set_mem_addr(DMA_CH_5,(uint32_t)(rx_data_2));             //重新设置DMA地址
//            hal_dma_set_data_num(DMA_CH_5,data_num);                          //设置DMA数据长度
//            hal_dma_en(DMA_CH_5,HAL_ENABLE);                                  //设置DMA使能
//        }
//        data_comp = 1; 
//    }
//}

#else

//普通方式接收数据
void UART0_IRQHandler()
{
   if(hal_uart_flag_get(UART0_BASE,USART_FLAG_RXNE) == 1 && hal_uart_it_en_status_get(UART0_BASE,USART_IT_RXNE))
   {
       rx_data_1[rx_data_cnt++] = hal_uart_recv_data(UART0_BASE);
       if(rx_data_cnt >= 99)rx_data_cnt = 0;
   }
}

#endif
