/**
 * @file     hal_timer.c
 * @author   BSP Team 
 * @brief    This file provides TIMER function.
 * @version  0.0.0.1
 * @date     2020-12-19
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_timer.h"

/* The TIMER3 will be used by WiFi Lib !!!*/
void hal_tim_init(uint32_t tim_x_base,tim_init_t_def *tim_init)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));

    hal_assert(IS_TIM_MODE(tim_init->tim_mode));
    hal_assert(IS_TIM_LOAD_VALUE(tim_init->tim_load_value));
    hal_assert(IS_TIM_LOAD_VALUE(tim_init->tim_load2_value));
    hal_assert(IS_TIM_DIV_VALUE(tim_init->tim_div));

    sysc_cmp_timer_gate_en_setf(1);
    switch(tim_x_base)
    {
        case TIMER0_BASE:sysc_cmp_timer1_gate_en_setf(1);break;
        case TIMER1_BASE:sysc_cmp_timer2_gate_en_setf(1);break;
        case TIMER2_BASE:sysc_cmp_timer3_gate_en_setf(1);break;
        case TIMER3_BASE:sysc_cmp_timer4_gate_en_setf(1);break;
    }
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    if(tim_init->tim_mode == TIM_FREE_RUNNING_MODE) {
        timer_timermode_setf(tim_x_base,0);
    }
    else if(tim_init->tim_mode == TIM_USER_DEF_CNT_MODE) {
        timer_timermode_setf(tim_x_base,1);
    }

    switch (tim_x_base)
    {
        case TIMER0_BASE:
            sysc_cmp_timer1_div_para_m1_setf(tim_init->tim_div);
            sysc_cmp_timer1_div_para_up_setf(1);
            break;
        case TIMER1_BASE:
            sysc_cmp_timer2_div_para_m1_setf(tim_init->tim_div);
            sysc_cmp_timer2_div_para_up_setf(1);
            break;
        case TIMER2_BASE:
            sysc_cmp_timer3_div_para_m1_setf(tim_init->tim_div);
            sysc_cmp_timer3_div_para_up_setf(1);
            break;
        case TIMER3_BASE:
            sysc_cmp_timer4_div_para_m1_setf(tim_init->tim_div);
            sysc_cmp_timer4_div_para_up_setf(1);
            break;
        default:
            break;
    }


    timer_timerloadcountregister_setf(tim_x_base,tim_init->tim_load_value);
    timer_timerloadcount2register_setf(tim_x_base,tim_init->tim_load2_value);
}

void hal_tim_deinit(void)
{
    sysc_cmp_srstn_timer_setf(0);
    sysc_cmp_srstn_timer_setf(1);
    
    sysc_cmp_timer_gate_en_setf(0);
    sysc_cmp_timer1_gate_en_setf(0);
    sysc_cmp_timer2_gate_en_setf(0);
    sysc_cmp_timer3_gate_en_setf(0);
    sysc_cmp_timer4_gate_en_setf(0);
}

uint8_t hal_tim_get_div(uint32_t tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));

    uint8_t div_val = 0;

    switch (tim_x_base)
    {
        case TIMER0_BASE:
            div_val = sysc_cmp_timer1_div_para_m1_getf();
            break;
        case TIMER1_BASE:
            div_val = sysc_cmp_timer2_div_para_m1_getf();
            break;
        case TIMER2_BASE:
            div_val = sysc_cmp_timer3_div_para_m1_getf();
            break;
        case TIMER3_BASE:
            div_val = sysc_cmp_timer4_div_para_m1_getf();
            break;
        default:
            break;
    }
    return div_val;
}

void hal_tim_en(uint32_t tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if(en == HAL_DISABLE) {
        timer_timerenable_setf(tim_x_base,0);
    }
    else if(en == HAL_ENABLE) {
        timer_timerenable_setf(tim_x_base,1);
    }
}

void hal_tim_pwm_en(uint32_t tim_x_base,hal_en_t en)
{   
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if(en == HAL_DISABLE) {
        timer_timerpwm_setf(tim_x_base,0);
    }
    else if(en == HAL_ENABLE) {
        timer_timerpwm_setf(tim_x_base,1);
    }
}

void hal_tim_set_load_value(uint32_t tim_x_base,uint32_t value)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_TIM_LOAD_VALUE(value));

    timer_timerloadcountregister_setf(tim_x_base,value);
}

uint32_t hal_tim_get_load_value(uint32_t tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    return timer_timerloadcountregister_getf(tim_x_base);
}

void hal_tim_set_load2_value(uint32_t tim_x_base,uint32_t value)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_TIM_LOAD_VALUE(value));

    timer_timerloadcount2register_setf(tim_x_base,value);
}

uint32_t hal_tim_get_load2_value(uint32_t tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    return timer_timerloadcount2register_getf(tim_x_base);
}

uint32_t hal_tim_get_current_cnt_value(uint32_t tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    return timer_timercurrentvalueregister_getf(tim_x_base);
}

void hal_tim_it_cfg(uint32_t tim_x_base,tim_it_flag_t tim_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_TIM_IT_FLAG(tim_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (tim_it_flag)
    {
        case TIM_IT_FLAG_ACTIVE:
        {
            if(en == HAL_DISABLE)
            {
                timer_timerinterruptmask_setf(tim_x_base,1);
            }
            else if(en == HAL_ENABLE)
            {
                timer_timerinterruptmask_setf(tim_x_base,0);
            }
            
            break;
        }
        
        default:
            break;
    }
}

uint8_t hal_tim_get_it_flag(uint32_t tim_x_base,tim_it_flag_t tim_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_TIM_IT_FLAG(tim_it_flag));

    switch (tim_it_flag)
    {
        case TIM_IT_FLAG_ACTIVE:
        {
            it_flag = timer_timerxinterruptstatusregister_getf(tim_x_base);
            break;
        }    
        default:
            break;
    }
    return it_flag;
}

void hal_tim_clr_it_flag(uint32_t tim_x_base,tim_it_flag_t tim_it_flag)
{
    /* check the parameters */
    hal_assert(IS_TIMER_ALL_PERIPH(tim_x_base));
    hal_assert(IS_TIM_IT_FLAG(tim_it_flag));

    switch (tim_it_flag)
    {
        case TIM_IT_FLAG_ACTIVE:
        {
            timer_timerendofinterruptregister_getf(tim_x_base);
            break;
        }    
        default:
            break;
    }
}
