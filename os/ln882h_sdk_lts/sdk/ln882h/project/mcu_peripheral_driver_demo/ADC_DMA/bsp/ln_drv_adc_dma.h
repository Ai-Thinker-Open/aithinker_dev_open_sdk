/**
 * @file     ln_drv_adc.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-02
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_ADC_DMA_H
#define __LN_DRV_ADC_DMA_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_adc.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"

void adc_dma_init(adc_ch_t adc_ch,uint32_t *data_arr,uint16_t data_num,void (*dma_complete_it_callback)(void));
void adc_dma_start(void);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_ADC_DMA_H
