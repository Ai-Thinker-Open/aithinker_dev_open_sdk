/**
 * @file     hal_rtc.h
 * @author   BSP Team 
 * @brief    This file contains all of RTC functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-11
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_RTC_H
#define __HAL_RTC_H

#ifdef __cplusplus
    extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "hal/hal_common.h"
#include "reg_rtc.h"

#define IS_RTC_ALL_PERIPH(PERIPH) ((PERIPH) == RTC_BASE) 

typedef enum
{
    RTC_WRAP_EN    = 0,
    RTC_WRAP_DIS   = 1,
}rtc_warp_en_t;

#define IS_RTC_WRAP_EN(EN) (((EN) == RTC_WRAP_EN) || ((EN) == RTC_WRAP_DIS))


typedef enum
{
    //this flag can generate interrupt , also can be used to read the status.
    RTC_IT_FLAG_ACTIVE           = 0x01,

    //this flag can not generate interrupt , can only be used to read the status. 
    RTC_IT_FLAG_ACTIVE_RAW       = 0x02,

}rtc_it_flag_t;

#define RTC_IT_FLAG_BIT_NUM         2
#define IS_RTC_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x03) != 0x00U) && ((((uint32_t)FLAG) & ~0x03) == 0x00U))


#define IS_RTC_CNT_MATCH_VALUE(VALUE)    ((VALUE <= 0xFFFFFFFF))
#define IS_RTC_CNT_LOAD_VALUE(VALUE)     ((VALUE <= 0xFFFFFFFF))


typedef struct
{
    rtc_warp_en_t       rtc_warp_en;            /*!< Specifies the rtc wrap enable status.
                                                This parameter can be a value of @ref RTC_Wrap_Enable_Status */
}rtc_init_t_def;

            //RTC init and config
void        hal_rtc_init(uint32_t rtc_base,rtc_init_t_def* rtc_init);
void        hal_rtc_deinit(void);
void        hal_rtc_en(uint32_t rtc_base,hal_en_t en);
void        hal_rtc_set_cnt_match(uint32_t rtc_base,uint32_t match_value);
void        hal_rtc_set_cnt_load(uint32_t rtc_base,uint32_t load_value);
uint32_t    hal_rtc_get_cnt(uint32_t rtc_base);

            //interrupt
void        hal_rtc_it_cfg(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag,hal_en_t en);
uint8_t     hal_rtc_get_it_flag(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag);
void        hal_rtc_clr_it_flag(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag);


#ifdef __cplusplus
}
#endif

#endif /* __HAL_RTC_H */



/**************************************END OF FILE********************************************/

