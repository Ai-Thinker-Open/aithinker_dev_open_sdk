/**
 * @file     ln_drv_rtc.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#ifndef __LN_DRV_RTC_H
#define __LN_DRV_RTC_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal_trng.h"


void trng_init(uint32_t range,uint32_t *data,uint32_t count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_RTC_H


