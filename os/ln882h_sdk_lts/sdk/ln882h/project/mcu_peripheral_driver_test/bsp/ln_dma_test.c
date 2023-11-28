/**
 * @file     ln_dma_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-26
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/**
    DMA使用说明：
                1. DMA的测试函数只有内存拷贝模式，需要通过DMA操作外设的，请参考相关外设代码。
                
                2. DMA的IFCR是W1C不能使用位域的方式操作。

*/



#include "hal/hal_uart.h"
#include "hal/hal_dma.h"
#include "hal/hal_gpio.h"
#include "ln_dma_test.h"

unsigned char src_data[100];
unsigned char dst_data[100];

void ln_dma_test(void)
{

    //1. 内存拷贝模式
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));

    dma_init.dma_mem_to_mem_en = DMA_MEM_TO_MEM_EN;     //使能DMA内存拷贝模式
    
    dma_init.dma_data_num = 100;                        //设置DMA内存拷贝的数量
    dma_init.dma_dir = DMA_READ_FORM_MEM;               //设置DMA方向
    dma_init.dma_mem_addr = (uint32_t)src_data;         //设置DMA源地址
    dma_init.dma_mem_size = DMA_MEM_SIZE_8_BIT;         //设置源地址数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;           //设置源地址是否增长
    
    dma_init.dma_p_addr = (uint32_t)dst_data;           //设置DMA目的地址
    dma_init.dma_p_inc_en = DMA_P_INC_EN;               //设置DMA目的地址是否增长
    dma_init.dma_p_size = DMA_P_SIZE_8_BIT;             //设置DMA目的地址数据长度
    
    dma_init.dma_pri_lev = DMA_PRI_LEV_MEDIUM;          //设置传输优先级
    
    hal_dma_init(DMA_CH_7,&dma_init);                   //初始化DMA
    
    hal_dma_it_cfg(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);      //配置DMA传输完成中断
    NVIC_EnableIRQ(DMA_IRQn);                                       //使能DMA中断
    
    for(int i = 0; i < 100; i++)
    {
        src_data[i] = i;
    }
    
    hal_dma_en(DMA_CH_7,HAL_ENABLE);                    //使能DMA传输
    
    while(hal_dma_get_data_num(DMA_CH_7) != 0 || hal_dma_get_status_flag(DMA_CH_7,DMA_STATUS_FLAG_TRAN_COMP));  //等待DMA传输完成
    
    while(1);
    
}


void DMA_IRQHandler()
{
    if(hal_dma_get_it_flag(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)  
    {
        hal_dma_clr_it_flag(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP);
    }
}

