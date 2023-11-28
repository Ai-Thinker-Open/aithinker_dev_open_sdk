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
#include "hal/hal_misc.h"

#include "hal_rtc.h"

void        rtc_init(uint32_t ms,void (*rtc_it_callback)());
uint32_t    rtc_get_rtc_current_cnt_time(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_RTC_H


