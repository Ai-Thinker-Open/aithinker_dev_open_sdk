/**
 * @file     ln_ws2811_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-26
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

 
/*
        WS2811外设测试说明：

        
        1. 对于WS2811来说，最重要的就是T0L,T1L,T0H,T1H的时间，下面简单叙述下这几个值如何设置。
        
           首先我们需要知道的是，我们已经人为的固定了T0H和T0L的比例为 1 ：4 ,T1H和T1L的比例为  4 : 1,(根据WS2811手册这个比例是成立的)
                               
           而这里所说的波特率就是单个0(T0)或单个1(T1)的时间，所以T0 = T0H + T0L, T1 = T1H + T1L,
           
           根据WS2811手册和T0H,T0L,T1H,T1L之间的比例，一般我们认为(以下数据为实测值，可以直接使用):
           
                T0 = 1125 ns,T0L = 900ns,T0H = 225ns    
                
                T1 = 1125 ns,T1L = 225ns,T1H = 900ns   
                
           而 T0 = T1 = (br + 1) * 5 * (1 / pclk)
           
           其中br为WS2811 BR寄存器，pclk为外设时钟，FPGA上pclk = 40M,所以 br = 8;实际芯片上pclk = 80M,所以br = 16
                               
                                
        2. 一般WS2811都是配合DMA使用，DMA使用的是通道二。
        
        3. WS2811不发送数据的时候，会默认引脚为低电平，根据WS2811的协议，总线低电平持续 280us 以上，会RESET 总线，完成传输。
        
        4. 使用WS2811 + DMA的时候，WS2811 Empty Interrupt会不响应。（中断的功能和DMA冲突，所以要把WS2811 DMA EN 关掉才行）
*/

#include "ln_ws2811_test.h"

#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"
#include "hal/hal_ws2811.h"
#include "ln_test_common.h"
#include "utils/debug/log.h"

#define LED_AMOUNT  50  

////////////          LED1                          LED2                         LED3                         ...       
////////////数据格式  R7 - R0   G7 - G0   B7 - B0  ;R7 - R0   G7 - G0   B7 - B0 ;R7 - R0   G7 - G0   B7 - B0  ...  

//          LED1                          LED2                         LED3                         ...       
//数据格式  B7 - B0   R7 - R0   G7 - G0  ;B7 - B0   R7 - R0   G7 - G0 ;B7 - B0   R7 - R0   G7 - G0  ...  
static unsigned char led_data_arr[LED_AMOUNT * 3 ] = {0xFF,0x00,0x00,  
                                                      0x00,0xFF,0x00,  
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00,
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00,
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00,
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00,
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00,
                                                      0x00,0x00,0xFF,
                                                      0xFF,0x00,0x00,
                                                      0x00,0xFF,0x00};
                                                      
static unsigned char led_data_arr1[LED_AMOUNT * 3] = {0x00,0x00,0x00,  
                                                      0x00,0x00,0x00,  
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00,
                                                      0x00,0x00,0x00};


void ln_ws2811_send_data(unsigned char *send_data,unsigned int data_len)
{
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_2,(uint32_t)send_data);
    hal_dma_set_data_num(DMA_CH_2,data_len);
    
    //开始传输。
    hal_dma_en(DMA_CH_2,HAL_ENABLE);
    
    //等待传输完成。
    while( hal_dma_get_data_num(DMA_CH_2) != 0);
    
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_2,HAL_DISABLE);
}

void ln_ws2811_init()
{
    // 1. 配置WS2811引脚复用
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_7,WS2811_OUT);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_7,HAL_ENABLE);
    
    // 2. 初始化WS2811配置
    ws2811_init_t_def ws2811_init;
    
    ws2811_init.br = 16;                                 //baud rate = (br+1)*5 * (1 / pclk)
    hal_ws2811_init(WS2811_BASE,&ws2811_init);          //初始化WS2811
    
    hal_ws2811_en(WS2811_BASE,HAL_ENABLE);              //使能WS2811
    hal_ws2811_dma_en(WS2811_BASE,HAL_ENABLE);          //使能WS2811 DMA
    
    hal_ws2811_it_cfg(WS2811_BASE,WS2811_IT_EMPTY_FLAG,HAL_ENABLE); //配置WS2811中断
    
    NVIC_EnableIRQ(WS2811_IRQn);                        //使能WS2811中断
    NVIC_SetPriority(WS2811_IRQn,     1);               //设置WS2811中断优先级
    
    // 3. 配置DMA
    dma_init_t_def dma_init;    
    memset(&dma_init,0,sizeof(dma_init));

    dma_init.dma_mem_addr = (uint32_t)led_data_arr;     //配置内存地址
    dma_init.dma_data_num = 0;                          //设置传输数量
    dma_init.dma_dir = DMA_READ_FORM_MEM;               //设置传输方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;           //使之内存是否自增
    dma_init.dma_p_addr = WS2811_DATA_REG;              //设置外设地址
    
    hal_dma_init(DMA_CH_2,&dma_init);                   //DMA初始化
    hal_dma_en(DMA_CH_2,HAL_DISABLE);                   //使能DMA
    
    while(1)
    {
        
        ln_ws2811_send_data(led_data_arr,3 * 3);                //点亮三盏灯

        ln_delay_ms(500);
        
        ln_ws2811_send_data(led_data_arr1,LED_AMOUNT * 3);      //熄灭所有灯

        ln_delay_ms(500);
    }
    
}


void ln_ws2811_test(void)
{
    ln_ws2811_init();
}

void WS2811_IRQHandler(void)
{
    hal_ws2811_set_data(WS2811_BASE,11);
}

