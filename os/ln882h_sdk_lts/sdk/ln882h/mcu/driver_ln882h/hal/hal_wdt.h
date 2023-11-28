/**
 * @file     hal_wdt.h
 * @author   BSP Team 
 * @brief    This file contains all of WDT functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-15
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_WDT_H
#define __HAL_WDT_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_wdt.h"

#define IS_WDT_ALL_PERIPH(PERIPH) ((PERIPH) == WDT_BASE)

typedef enum
{
    WDT_RPL_2_PCLK        = 0,     
    WDT_RPL_4_PCLK        = 1,        
    WDT_RPL_8_PCLK        = 2,        
    WDT_RPL_16_PCLK       = 3,        
    WDT_RPL_32_PCLK       = 4,        
    WDT_RPL_64_PCLK       = 5,        
    WDT_RPL_128_PCLK      = 6,        
    WDT_RPL_256_PCLK      = 7,        
}wdt_rpl_t;

#define IS_WDT_RPL(RPL)   (((RPL) == WDT_RPL_2_PCLK)    || ((RPL) == WDT_RPL_4_PCLK)  || \
                           ((RPL) == WDT_RPL_8_PCLK)    || ((RPL) == WDT_RPL_16_PCLK) || \
                           ((RPL) == WDT_RPL_32_PCLK)   || ((RPL) == WDT_RPL_64_PCLK) || \
                           ((RPL) == WDT_RPL_128_PCLK)  || ((RPL) == WDT_RPL_256_PCLK) ) 


typedef enum
{
    WDT_RMOD_0     = 0,     
    WDT_RMOD_1     = 1,        
}wdt_rmod_t;

#define IS_WDT_RMOD(MODE)  (((MODE) == WDT_RMOD_0)  || ((MODE) == WDT_RMOD_1)) 


#define RESTART_VALUE                 0x76

typedef enum
{
    WDT_IT_FLAG_ACTIVE           = 0x01,
}wdt_it_flag_t;
typedef enum
{
    WDT_TOP_VALUE_0           = 0x00,       //wdt cnt value = 0xFF      Time = 0.007 s
    WDT_TOP_VALUE_1           = 0x01,       //wdt cnt value = 0x1FF     Time = 0.015 s
    WDT_TOP_VALUE_2           = 0x02,       //wdt cnt value = 0x3FF     Time = 0.031 s
    WDT_TOP_VALUE_3           = 0x03,       //wdt cnt value = 0x7FF     Time = 0.063 s
    WDT_TOP_VALUE_4           = 0x04,       //wdt cnt value = 0xFFF     Time = 0.127 s
    WDT_TOP_VALUE_5           = 0x05,       //wdt cnt value = 0x1FFF    Time = 0.255 s
    WDT_TOP_VALUE_6           = 0x06,       //wdt cnt value = 0x3FFF    Time = 0.511 s
    WDT_TOP_VALUE_7           = 0x07,       //wdt cnt value = 0x7FFF    Time = 1.023 s
    
    WDT_TOP_VALUE_8           = 0x08,       //wdt cnt value = 0xFFFF    Time = 2.047 s
    WDT_TOP_VALUE_9           = 0x09,       //wdt cnt value = 0x1FFFF   Time = 4.095 s
    WDT_TOP_VALUE_10          = 0x0A,       //wdt cnt value = 0x3FFFF   Time = 8.191 s
    WDT_TOP_VALUE_11          = 0x0B,       //wdt cnt value = 0x7FFFF   Time = 16.38 s
    WDT_TOP_VALUE_12          = 0x0C,       //wdt cnt value = 0xFFFFF   Time = 32.76 s
    WDT_TOP_VALUE_13          = 0x0D,       //wdt cnt value = 0x1FFFFF  Time = 65.53 s
    WDT_TOP_VALUE_14          = 0x0E,       //wdt cnt value = 0x3FFFFF  Time = 131.0 s
    WDT_TOP_VALUE_15          = 0x0F,       //wdt cnt value = 0x7FFFFF  Time = 262.1 s
}wdt_top_value_t;
#define IS_WDT_TOP_VALUE(VALUE)       ( ((VALUE) <= 0x0F)) 

#define WDT_IT_FLAG_BIT_NUM         1
#define IS_WDT_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x01) != 0x00U) && ((((uint32_t)FLAG) & ~0x01) == 0x00U))

typedef struct
{
    wdt_rpl_t       wdt_rpl;                /*!< Specifies the wdt reset pulse length.
                                            This parameter can be a value of @ref wdt_rpl_t */

    wdt_rmod_t      wdt_rmod;               /*!< Specifies the wdt reset mode.
                                            This parameter can be a value of @ref wdt_rmod_t */
                                            
    wdt_top_value_t top;                    /*!< Specifies the wdt reset pulse length.
                                            The range of parameters can be referred to @ref IS_WDT_TOP_VALUE */

}wdt_init_t_def;



            //WDT init and config
void        hal_wdt_init(uint32_t wdt_base,wdt_init_t_def *wdt_init);
void        hal_wdt_deinit(void);
void        hal_wdt_en(uint32_t wdt_base,hal_en_t en);
void        hal_wdt_cnt_restart(uint32_t wdt_base);
void        hal_wdt_set_top_value(uint32_t wdt_base,wdt_top_value_t value);

            //WDT Interrupt
//void        hal_wdt_it_cfg(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag ,hal_en_t en);
uint8_t     hal_wdt_get_it_flag(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag);
void        hal_wdt_clr_it_flag(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag);



#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_WDT_H__
