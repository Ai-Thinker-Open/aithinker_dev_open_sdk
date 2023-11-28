/**
 * @file     ln_drv_dma.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_DMA_H
#define __LN_DRV_DMA_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_dma.h"


void dma_init(void);
void dma_mem_cpy(uint8_t *dst,uint8_t *src,uint8_t size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_DMA_H
