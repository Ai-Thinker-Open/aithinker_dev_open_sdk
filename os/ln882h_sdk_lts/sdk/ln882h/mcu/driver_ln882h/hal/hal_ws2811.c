/**
 * @file     hal_ws2811.c
 * @author   BSP Team 
 * @brief    This file provides ws2811 function.
 * @version  0.0.0.1
 * @date     2021-02-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/

#include "hal/hal_ws2811.h"

void  hal_ws2811_init(uint32_t ws2811_base, ws2811_init_t_def *ws2811_init)
{
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_WS2811_BR_VALUE(ws2811_init->br));

    sysc_cmp_ws2811_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    ws2811_br_setf(ws2811_base,ws2811_init->br);
}

void hal_ws2811_deinit(void)
{
    sysc_cmp_srstn_ws2811_setf(0);
    sysc_cmp_srstn_ws2811_setf(1);

    sysc_cmp_ws2811_gate_en_setf(0);
}

void  hal_ws2811_en(uint32_t ws2811_base,hal_en_t en)
{
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if(en == HAL_DISABLE){
        ws2811_ws_en_setf(ws2811_base,0);
    }
    else if(en == HAL_ENABLE){
        ws2811_ws_en_setf(ws2811_base,1);
    }
}

void hal_ws2811_set_data(uint32_t ws2811_base,uint8_t value)
{
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    ws2811_dr_set(ws2811_base,value);
}

void hal_ws2811_dma_en(uint32_t ws2811_base,hal_en_t en)
{
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if(en == HAL_DISABLE){
        ws2811_dma_en_setf(ws2811_base,0);
    }
    else if(en == HAL_ENABLE){
        ws2811_dma_en_setf(ws2811_base,1);
    }

}

uint8_t hal_ws2811_get_status_flag(uint32_t ws2811_base,ws2811_status_flag_t ws2811_status_flag)
{
    uint8_t status_flag = 0;
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_WS2811_STATUS_FLAG(ws2811_status_flag));

    if(ws2811_status_flag == WS2811_STATUS_FLAG_DR_EMPTY)
    {
        status_flag = ws2811_empty_getf(ws2811_base);
    }
    return status_flag;
}

void hal_ws2811_it_cfg(uint32_t ws2811_base,ws2811_it_flag_t ws2811_it_flag ,hal_en_t en)
{
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_WS2811_IT_FLAG(ws2811_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if(ws2811_it_flag == WS2811_IT_EMPTY_FLAG)
    {
        if(en == HAL_DISABLE){
            ws2811_int_en_setf(ws2811_base,0);
        }
        else if(en == HAL_ENABLE){
            ws2811_int_en_setf(ws2811_base,1);
        }      
    }
}

uint8_t hal_ws2811_get_it_flag(uint32_t ws2811_base,ws2811_it_flag_t ws2811_it_flag)
{
    uint8_t it_flag = 0;
    hal_assert(IS_WS2811_ALL_PERIPH(ws2811_base));
    hal_assert(IS_WS2811_IT_FLAG(ws2811_it_flag));

    if(ws2811_it_flag == WS2811_IT_EMPTY_FLAG)
    {
        it_flag = ws2811_intr_getf(ws2811_base);
    }
    return it_flag;
}



