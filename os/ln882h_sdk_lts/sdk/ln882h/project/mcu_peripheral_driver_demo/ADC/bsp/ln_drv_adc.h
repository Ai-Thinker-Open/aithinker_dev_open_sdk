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

#ifndef __LN_DRV_ADC_H
#define __LN_DRV_ADC_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_adc.h"
#include "hal/hal_gpio.h"

void     adc_init(adc_ch_t adc_ch);
uint16_t adc_get_data(adc_ch_t adc_ch);
void     adc_start(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_ADC_H
