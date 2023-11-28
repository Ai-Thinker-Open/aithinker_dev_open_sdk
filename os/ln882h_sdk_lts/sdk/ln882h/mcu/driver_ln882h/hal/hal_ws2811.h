/**
 * @file     hal_ws2811.h
 * @author   BSP Team 
 * @brief    This file contains all of ws2811 functions prototypes.
 * @version  0.0.0.1
 * @date     2021-02-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


#ifndef __HAL_WS2811_H
#define __HAL_WS2811_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "reg_ws2811.h"
#include "hal/hal_common.h"

#define IS_WS2811_ALL_PERIPH(PERIPH) ((PERIPH) == WS2811_BASE)

typedef enum
{
    WS2811_STATUS_FLAG_DR_EMPTY     = 0x01, 
}ws2811_status_flag_t;

#define WS2811_STATUS_FLAG_BIT_NUM         1
#define IS_WS2811_STATUS_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x01) != 0x00U) && ((((uint32_t)FLAG) & ~0x01) == 0x00U))

typedef enum
{
    WS2811_IT_EMPTY_FLAG            = 0x01,     //write dr clear
}ws2811_it_flag_t;

#define WS2811_IT_FLAG_BIT_NUM         1
#define IS_WS2811_IT_FLAG(FLAG)        (((((uint32_t)FLAG) & 0x01) != 0x00U) && ((((uint32_t)FLAG) & ~0x01) == 0x00U))

#define IS_WS2811_BR_VALUE(VALUE)      (((VALUE) <= 0xFF))

typedef struct
{
    uint8_t         br;                     /*!< Specifies the ws2811 baud rate.  pclk num for one bit: br*4
                                            The range of parameters can be referred to @ref IS_WS2811_BR_VALUE */
}ws2811_init_t_def;

            //WS2811 init and config
void        hal_ws2811_init(uint32_t ws2811_base, ws2811_init_t_def *ws2811_init);
void        hal_ws2811_deinit(void);
void        hal_ws2811_en(uint32_t ws2811_base,hal_en_t en);
void        hal_ws2811_set_data(uint32_t ws2811_base,uint8_t value);
void        hal_ws2811_dma_en(uint32_t ws2811_base,hal_en_t en);
uint8_t     hal_ws2811_get_status_flag(uint32_t ws2811_base,ws2811_status_flag_t ws2811_status_flag);

            //WS2811 Interrupt
void        hal_ws2811_it_cfg(uint32_t ws2811_base,ws2811_it_flag_t ws2811_it_flag ,hal_en_t en);
uint8_t     hal_ws2811_get_it_flag(uint32_t ws2811_base,ws2811_it_flag_t ws2811_it_flag);




#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_WS2811_H
