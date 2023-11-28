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
#include "hal_timer.h"


typedef enum
{
    TIMER_CH_0,
    TIMER_CH_1,
    TIMER_CH_2,
    TIMER_CH_3,     //通道3在使用wifi的情况下会被wifi占用，请慎重使用
}timer_ch_t;

void        timer_init(timer_ch_t timer_ch,uint32_t us,void (*timer_it_callback)(void));
uint32_t    timer_get_timer_cnt_time(timer_ch_t timer_ch);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_RTC_H


