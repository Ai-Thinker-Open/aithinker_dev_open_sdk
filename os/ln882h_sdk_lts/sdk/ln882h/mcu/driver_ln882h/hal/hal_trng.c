/**
 * @file     hal_trng.c
 * @author   BSP Team 
 * @brief    This file provides TRNG function.
 * @version  0.0.0.1
 * @date     2020-12-23
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_trng.h"


void hal_trng_init(uint32_t trng_base,trng_init_t_def* trng_init)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));

    hal_assert(IS_TRNG_FAST_MODE_EN_STATUS(trng_init->trng_fast_mode_en_status));
    hal_assert(IS_TRNG_GAP_VALUE(trng_init->trng_gap));

    sysc_cmp_trng_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    if (trng_init->trng_fast_mode_en_status == TRNG_FAST_MODE_DIS) {
        trng_fast_trng_setf(trng_base,0);
    }
    else if (trng_init->trng_fast_mode_en_status == TRNG_FAST_MODE_EN){
        trng_fast_trng_setf(trng_base,1);
    }
    trng_gap_setf(trng_base,trng_init->trng_gap);
}

void hal_trng_deinit(void)
{
    sysc_cmp_srstn_trng_setf(0);
    sysc_cmp_srstn_trng_setf(1);

    sysc_cmp_trng_gate_en_setf(0);
}

void hal_trng_en(uint32_t trng_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        trng_en_setf(trng_base,0);
    }
    else if (en == HAL_ENABLE) {
        trng_en_setf(trng_base,1);
    }
}

uint8_t hal_trng_get_en_status(uint32_t trng_base)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    return trng_en_getf(trng_base);
}

void hal_trng_set_gap(uint32_t trng_base,uint8_t gap)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    hal_assert(IS_TRNG_GAP_VALUE(gap));

    trng_gap_setf(trng_base,gap);
}

uint32_t hal_trng_get_data_0_31(uint32_t trng_base)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    return trng_data_1_getf(trng_base);
}

uint32_t hal_trng_get_data_32_63(uint32_t trng_base)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    return trng_data_2_getf(trng_base);
}

uint32_t hal_trng_get_data_64_95(uint32_t trng_base)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    return trng_data_3_getf(trng_base);
}

uint32_t hal_trng_get_data_96_127(uint32_t trng_base)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    return trng_data_4_getf(trng_base);
}

void hal_trng_it_cfg(uint32_t trng_base,trng_it_flag_t trng_it_flag ,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    hal_assert(IS_TRNG_IT_FLAG(trng_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (trng_it_flag)
    {
        case TRNG_IT_FLAG_TRNG_DONE:
        {
            if (en == HAL_DISABLE) {
                trng_ie_setf(trng_base,0);
            }
            else if(en == HAL_ENABLE) {
                trng_ie_setf(trng_base,1);
            }
            break;
        }
        default:
            break;
    }
}

uint8_t hal_trng_get_it_flag(uint32_t trng_base,trng_it_flag_t trng_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    hal_assert(IS_TRNG_IT_FLAG(trng_it_flag));

    switch (trng_it_flag)
    {
        case TRNG_IT_FLAG_TRNG_DONE:
        {
            it_flag = trng_done_getf(trng_base);
            break;
        }
        default:
            break;
    } 
    return it_flag;
}

void hal_trng_clr_it_flag(uint32_t trng_base,trng_it_flag_t trng_it_flag)
{
    /* check the parameters */
    hal_assert(IS_TRNG_ALL_PERIPH(trng_base));
    hal_assert(IS_TRNG_IT_FLAG(trng_it_flag));

    switch (trng_it_flag)
    {
        case TRNG_IT_FLAG_TRNG_DONE:
        {
            trng_done_getf(trng_base);
            break;
        }
        default:
            break;
    }
}

