/**
 * @file     hal_wdt.c
 * @author   BSP Team
 * @brief    This file provides WDT function.
 * @version  0.0.0.1
 * @date     2021-08-24
 *
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */


/* Includes ------------------------------------------------------------------*/
#include "hal/hal_wdt.h"


void hal_wdt_init(uint32_t wdt_base,wdt_init_t_def *wdt_init)
{
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    hal_assert(IS_WDT_RPL(wdt_init->wdt_rpl));
    hal_assert(IS_WDT_RMOD(wdt_init->wdt_rmod));
    hal_assert(IS_WDT_TOP_VALUE(wdt_init->top));

    sysc_cmp_wdt_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    if (wdt_init->wdt_rmod == WDT_RMOD_0) {
        wdt_rmod_setf(wdt_base,0);
    }
    else if (wdt_init->wdt_rmod == WDT_RMOD_1)
    {
        wdt_rmod_setf(wdt_base,1);
    }

    switch (wdt_init->wdt_rpl)
    {
        case WDT_RPL_2_PCLK:
            wdt_rpl_setf(wdt_base,0);
            break;
        case WDT_RPL_4_PCLK:
            wdt_rpl_setf(wdt_base,1);
            break;
        case WDT_RPL_8_PCLK:
            wdt_rpl_setf(wdt_base,2);
            break;
        case WDT_RPL_16_PCLK:
            wdt_rpl_setf(wdt_base,3);
            break;
        case WDT_RPL_32_PCLK:
            wdt_rpl_setf(wdt_base,4);
            break;
        case WDT_RPL_64_PCLK:
            wdt_rpl_setf(wdt_base,5);
            break;
        case WDT_RPL_128_PCLK:
            wdt_rpl_setf(wdt_base,6);
            break;
        case WDT_RPL_256_PCLK:
            wdt_rpl_setf(wdt_base,7);
            break;
        default:
            break;
    }
    wdt_top_setf(wdt_base,wdt_init->top);
}

void hal_wdt_deinit(void)
{
    sysc_cmp_srstn_wdt_setf(0);
    sysc_cmp_srstn_wdt_setf(1);

    sysc_cmp_wdt_gate_en_setf(0);
}

void hal_wdt_en(uint32_t wdt_base,hal_en_t en)
{
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        wdt_wdt_en_setf(wdt_base,0);
        sysc_awo_o_wdt_rst_mask_setf(1);
    }
    else if(en == HAL_ENABLE)
    {
        wdt_wdt_en_setf(wdt_base,1);
        sysc_awo_o_wdt_rst_mask_setf(0);
    }
    wdt_wdt_crr_set(wdt_base,0x76);
}

void hal_wdt_cnt_restart(uint32_t wdt_base)
{
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    wdt_wdt_crr_set(wdt_base,0x76);
}

void hal_wdt_set_top_value(uint32_t wdt_base,wdt_top_value_t value)
{
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    hal_assert(IS_WDT_TOP_VALUE(value));
    wdt_top_setf(wdt_base,10);
}

/* WDT interrupt configuration conflicts with initialization configuration. You should confirm whether interrupt is generated in initialization configuration */
//void hal_wdt_it_cfg(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag ,hal_en_t en)
//{
//    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
//    hal_assert(IS_WDT_IT_FLAG(wdt_it_flag));
//    hal_assert(IS_FUNCTIONAL_STATE(en));
//    switch (wdt_it_flag)
//    {
//        case WDT_IT_FLAG_ACTIVE:
//        {
//            if (en == HAL_DISABLE ) {
//                WDT_RMOD_1f(wdt_base,0);
//            }
//            else if(en == HAL_ENABLE)
//            {
//                WDT_RMOD_1f(wdt_base,1);
//            }
//            break;
//        }
//        default:
//            break;
//    }
//}

uint8_t hal_wdt_get_it_flag(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag)
{
    uint8_t it_flag = 0;
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    hal_assert(IS_WDT_IT_FLAG(wdt_it_flag));
    switch (wdt_it_flag)
    {
        case WDT_IT_FLAG_ACTIVE:
            it_flag = wdt_interruptstatusregister_getf(wdt_base);
            break;

        default:
            break;
    }
    return it_flag;
}

void hal_wdt_clr_it_flag(uint32_t wdt_base,wdt_it_flag_t wdt_it_flag)
{
    hal_assert(IS_WDT_ALL_PERIPH(wdt_base));
    hal_assert(IS_WDT_IT_FLAG(wdt_it_flag));
    switch (wdt_it_flag)
    {
        case WDT_IT_FLAG_ACTIVE:
            wdt_interruptclearregister_getf(wdt_base);
            break;

        default:
            break;
    }
}
