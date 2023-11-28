/**
 * @file     ln_drv_wdt.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-09
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#ifndef __LN_DRV_WDT_H
#define __LN_DRV_WDT_H


#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal_wdt.h"


void wdt_init(wdt_top_value_t wdt_top_value);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_WDT_H


