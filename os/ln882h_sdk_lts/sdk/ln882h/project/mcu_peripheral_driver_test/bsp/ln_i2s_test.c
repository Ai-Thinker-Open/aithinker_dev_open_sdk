/**
 * @file     ln_i2s_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-25
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
    I2S调试说明：

        1. 引脚     
                    LN882H          STM32
                    A8(WS)          PB12             
                    A9(CLK)         PB13
                    A10(SDO)        PB15(SD)
                    A6(SDI)
                    
            注：STM32的I2S的DR是接收发送一体的，所以只有一个SD寄存器,同时LN882H的I2S引脚是固定的，通过CMP里面的I2S_O_EN0来选择是否打开
                而我们是全双工的，A10输出数据，A6接收数据
        
        2. 从机接收数据的时候可能会缺少帧头和帧尾的数据，发送数据的时候不会有这个问题。
        
        3. I2S不支持DMA。
        
        4. 本测试中，使用中断来进行I2S的接收和发送。

        

*/

#include "hal_i2s.h"
#include "ln_i2s_test.h"
#include "reg_sysc_cmp.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

static unsigned short tx_data_left[256];
static unsigned short tx_data_right[256];
static unsigned short rx_data_left[256];
static unsigned short rx_data_right[256];
static unsigned int data_index = 0;
static unsigned int tx_data_index = 0;      
static unsigned int rx_data_index = 0;


void ln_i2s_test(void)
{
    //使能I2S IO
    sysc_cmp_i2s_io_en0_setf(1);
   
    for(int i = 0;i < 256; i++)
    {
        tx_data_left[i] = i+1 ;
        tx_data_right[i] = i+10;
    }
    
    __NVIC_SetPriorityGrouping(4);
    __enable_irq();
    NVIC_SetPriority(I2S_IRQn,     4);
    NVIC_EnableIRQ(I2S_IRQn);
    
    i2s_init_t_def i2s_init;
    memset(&i2s_init,0,sizeof(i2s_init));
    i2s_init.tra_fifo_trig_lev = 4;				        //为了让FIFO中同时有左声道和右声道数据，此处设为4（只有一个声道时可能会不发数据 ）。
    i2s_init.rec_fifo_trig_lev = 0;                     //设置接收FIFO深度门限（就是能触发门限中断的值）
    i2s_init.i2s_tra_config = I2S_TRA_CONFIG_16_BIT;    //设置发送数据宽度
    i2s_init.i2s_rec_config = I2S_REC_CONFIG_16_BIT;    //设置接收数据宽度
    hal_i2s_init(I2S_BASE,&i2s_init);                   //初始化I2S
    
    //tx
    hal_i2s_tx_fifo_flush(I2S_BASE);                        //清除发送FIFO
    hal_i2s_tx_en(I2S_BASE,HAL_ENABLE);                     //使能I2S发送
    hal_i2s_it_cfg(I2S_BASE,I2S_IT_FLAG_TXFE,HAL_ENABLE);   //使能发送中断（fifo为空产生中断）
    hal_i2s_en(I2S_BASE,HAL_ENABLE);                    

    //rx
    hal_i2s_rx_fifo_flush(I2S_BASE);                        //清除接收FIFO
    hal_i2s_rx_en(I2S_BASE,HAL_ENABLE);                     //使能I2S接收
    hal_i2s_it_cfg(I2S_BASE,I2S_IT_FLAG_RXDA,HAL_ENABLE);   //使能接收中断（fifo达到门限值产生中断）
    hal_i2s_en(I2S_BASE,HAL_ENABLE);                        //I2S模块使能

    while(1)
    {
        LOG(LOG_LVL_INFO,"running...\r\n");
        ln_delay_ms(1000);
    }

}

void I2S_IRQHandler(void)
{
    //tx
    if(hal_i2s_get_it_flag(I2S_BASE,I2S_IT_FLAG_TXFE) == 1)
    {
        hal_i2s_send_data(I2S_BASE,tx_data_left[tx_data_index],tx_data_right[tx_data_index]);
        
        tx_data_index++;
        if(tx_data_index >= 255)
            hal_i2s_en(I2S_BASE,HAL_DISABLE);               //发送一定数量的数据之后，关闭I2S，否则可能会卡死（因为一直处于中断中）
    }

    //rx
    if(hal_i2s_get_it_flag(I2S_BASE,I2S_IT_FLAG_RXDA) == 1)
    {
        //接收数据，接收左声道和右声道数据
        hal_i2s_recv_data(I2S_BASE,(uint32_t *)(rx_data_left + rx_data_index),(uint32_t *)(rx_data_right + rx_data_index));
        
        rx_data_index++;
        if(rx_data_index >= 250)
            rx_data_index = 0;
        data_index++;
    }

}


