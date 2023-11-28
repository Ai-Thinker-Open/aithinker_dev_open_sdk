/**
 * @file     ln_drv_dma.c
 * @author   BSP Team 
 * @brief    本DMA DEMO只包含了内存拷贝的DMA功能，其它功能请参考相关外设。
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_dma.h"

/**
 * @brief DMA初始化
 * 
 */
void dma_init(void)
{
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));
    dma_init.dma_mem_to_mem_en = DMA_MEM_TO_MEM_EN;     //使能DMA内存拷贝模式
    dma_init.dma_dir = DMA_READ_FORM_MEM;               //设置DMA方向
    dma_init.dma_mem_size = DMA_MEM_SIZE_8_BIT;         //设置源地址数据长度
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;           //设置源地址是否增长
    dma_init.dma_p_inc_en = DMA_P_INC_EN;               //设置DMA目的地址是否增长
    dma_init.dma_p_size = DMA_P_SIZE_8_BIT;             //设置DMA目的地址数据长度
    dma_init.dma_pri_lev = DMA_PRI_LEV_MEDIUM;          //设置传输优先级
    hal_dma_init(DMA_CH_7,&dma_init);                   //初始化DMA
    hal_dma_it_cfg(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);
}

/**
 * @brief DMA内存拷贝
 * 
 * @param dst 目的地址
 * @param src 源地址
 * @param size 传输数据大小
 */
void dma_mem_cpy(uint8_t *dst,uint8_t *src,uint8_t size)
{
    hal_dma_set_mem_addr(DMA_CH_7,(uint32_t)src);
    hal_dma_set_p_addr(DMA_CH_7,(uint32_t)dst);
    hal_dma_set_data_num(DMA_CH_7,size);
    hal_dma_en(DMA_CH_7,HAL_ENABLE);
    while(hal_dma_get_status_flag(DMA_CH_7,DMA_STATUS_FLAG_TRAN_COMP) != HAL_SET);
    hal_dma_clr_it_flag(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP);
}
