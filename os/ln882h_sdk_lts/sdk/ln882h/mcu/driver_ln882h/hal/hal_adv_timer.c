/**
 * @file     hal_adv_timer.c
 * @author   BSP Team 
 * @brief    This file provides ADV-TIMER function.
 * @version  0.0.0.1
 * @date     2021-08-23
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_adv_timer.h"

void hal_adv_tim_init(uint32_t adv_tim_x_base,adv_tim_init_t_def* adv_tim_init)
{   
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));

    /* Check the adv timer parameters */
    hal_assert(IS_TIMER_CAP_EN(adv_tim_init->adv_tim_cap_en));
    hal_assert(IS_TIMER_TIMER_EN(adv_tim_init->adv_tim_tim_en));
    hal_assert(IS_TIMER_DEAD_EN(adv_tim_init->adv_tim_dead_en));
    hal_assert(IS_TIMER_CHA_EN(adv_tim_init->adv_tim_cha_en));
    hal_assert(IS_TIMER_CHB_EN(adv_tim_init->adv_tim_chb_en));
    hal_assert(IS_TIMER_CHB_IT_MODE(adv_tim_init->adv_tim_chb_it_mode));
    hal_assert(IS_TIMER_CHA_IT_MODE(adv_tim_init->adv_tim_cha_it_mode));
    hal_assert(IS_TIMER_ONCE_EN(adv_tim_init->adv_tim_once_en));
    hal_assert(IS_TIMER_CAP_MODE(adv_tim_init->adv_tim_cap_mode));
    hal_assert(IS_TIMER_CAP_EDG(adv_tim_init->adv_tim_cap_edg));
    hal_assert(IS_TIMER_TRG_CLR_EN(adv_tim_init->adv_tim_trg_clr_en));
    hal_assert(IS_TIMER_TRG_SEL(adv_tim_init->adv_tim_trg_sel));
    hal_assert(IS_TIMER_TRG_MODE(adv_tim_init->adv_tim_trg_mode));
    hal_assert(IS_TIMER_CNT_MODE(adv_tim_init->adv_tim_cnt_mode));
    hal_assert(IS_TIMER_CHB_INV_EN(adv_tim_init->adv_tim_chb_inv_en));
    hal_assert(IS_TIMER_CHA_INV_EN(adv_tim_init->adv_tim_cha_inv_en));
    hal_assert(IS_TIMER_LOAD_VALUE(adv_tim_init->adv_tim_load_value));
    hal_assert(IS_TIMER_CMPA_VALUE(adv_tim_init->adv_tim_cmp_a_value));
    hal_assert(IS_TIMER_CMPB_VALUE(adv_tim_init->adv_tim_cmp_b_value));
    hal_assert(IS_TIMER_CLK_DIV(adv_tim_init->adv_tim_clk_div));
    hal_assert(IS_TIMER_DEAD_GAP(adv_tim_init->adv_tim_dead_gap_value));
    hal_assert(IS_TIMER_TRIG_VALUE(adv_tim_init->adv_tim_trig_value));

    sysc_cmp_pwm_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    // set the adv timer parameters.
    pwm_db_setf(adv_tim_x_base,adv_tim_init->adv_tim_dead_gap_value);
    pwm_load_setf(adv_tim_x_base,adv_tim_init->adv_tim_load_value);
    pwm_cmp_b_setf(adv_tim_x_base,adv_tim_init->adv_tim_cmp_b_value);
    pwm_cmp_a_setf(adv_tim_x_base,adv_tim_init->adv_tim_cmp_a_value);
    pwm_trig_setf(adv_tim_x_base,adv_tim_init->adv_tim_trig_value);
    if (adv_tim_init->adv_tim_cap_en == HAL_DISABLE) {
        pwm_cap_en_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_cap_en == HAL_ENABLE) {
        pwm_cap_en_setf(adv_tim_x_base,1);
    }

    if (adv_tim_init->adv_tim_tim_en == HAL_DISABLE) {
        pwm_tim_en_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_tim_en == HAL_ENABLE) {
        pwm_tim_en_setf(adv_tim_x_base,1);
    }

    if (adv_tim_init->adv_tim_dead_en == HAL_DISABLE) {
        pwm_dben_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_dead_en == HAL_ENABLE) {
        pwm_dben_setf(adv_tim_x_base,1);
    }    

    if (adv_tim_init->adv_tim_cha_en == HAL_DISABLE) {
        pwm_ena_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_cha_en == HAL_ENABLE) {
        pwm_ena_setf(adv_tim_x_base,1);
    }      

    if (adv_tim_init->adv_tim_chb_en == HAL_DISABLE) {
        pwm_enb_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_chb_en == HAL_ENABLE) {
        pwm_enb_setf(adv_tim_x_base,1);
    }   

    switch (adv_tim_init->adv_tim_chb_it_mode)
    {
        case ADV_TIMER_CHB_IT_MODE_DIS:
            pwm_int_cmp_b_mod_setf(adv_tim_x_base,0);
            break;

        case ADV_TIMER_CHB_IT_MODE_INC:
            pwm_int_cmp_b_mod_setf(adv_tim_x_base,1);
            break;

        case ADV_TIMER_CHB_IT_MODE_DEC:
            pwm_int_cmp_b_mod_setf(adv_tim_x_base,2);
            break;

        case ADV_TIMER_CHB_IT_MODE_BOTH:
            pwm_int_cmp_b_mod_setf(adv_tim_x_base,3);
            break;
    
        default:
            break;
    }

    switch (adv_tim_init->adv_tim_cha_it_mode)
    {
        case ADV_TIMER_CHA_IT_MODE_DIS:
            pwm_int_cmp_a_mod_setf(adv_tim_x_base,0);
            break;

        case ADV_TIMER_CHA_IT_MODE_INC:
            pwm_int_cmp_a_mod_setf(adv_tim_x_base,1);
            break;

        case ADV_TIMER_CHA_IT_MODE_DEC:
            pwm_int_cmp_a_mod_setf(adv_tim_x_base,2);
            break;

        case ADV_TIMER_CHA_IT_MODE_BOTH:
            pwm_int_cmp_a_mod_setf(adv_tim_x_base,3);
            break;
    
        default:
            break;
    }

    if (adv_tim_init->adv_tim_once_en == HAL_DISABLE) {
        pwm_tim_once_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_once_en == HAL_ENABLE) {
        pwm_tim_once_setf(adv_tim_x_base,1);
    }  

    pwm_div_setf(adv_tim_x_base,adv_tim_init->adv_tim_clk_div);

    if (adv_tim_init->adv_tim_cap_mode == ADV_TIMER_CAP_MODE_1) {
        pwm_cap_mod_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_cap_mode == ADV_TIMER_CAP_MODE_2) {
        pwm_cap_mod_setf(adv_tim_x_base,1);
    } 

    switch (adv_tim_init->adv_tim_cap_edg)
    {
        case ADV_TIMER_EDG_DIS:
            pwm_cap_edg_setf(adv_tim_x_base,0);
            break;

        case ADV_TIMER_EDG_RISE:
            pwm_cap_edg_setf(adv_tim_x_base,1);
            break;

        case ADV_TIMER_EDG_FALL:
            pwm_cap_edg_setf(adv_tim_x_base,2);
            break;

        case ADV_TIMER_EDG_BOTH:
            pwm_cap_edg_setf(adv_tim_x_base,3);
            break;
    
        default:
            break;
    }

    if (adv_tim_init->adv_tim_trg_clr_en == HAL_DISABLE) {
        pwm_trig_clr_en_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_trg_clr_en == HAL_ENABLE) {
        pwm_trig_clr_en_setf(adv_tim_x_base,1);
    }

    switch (adv_tim_init->adv_tim_trg_mode)
    {
        case ADV_TIMER_TRG_MODE_DIS:
            pwm_trig_mod_setf(adv_tim_x_base,0);
            break;

        case ADV_TIMER_TRG_MODE_INC:
            pwm_trig_mod_setf(adv_tim_x_base,1);
            break;

        case ADV_TIMER_TRG_MODE_DEC:
            pwm_trig_mod_setf(adv_tim_x_base,2);
            break;

        case ADV_TIMER_TRG_MODE_BOTH:
            pwm_trig_mod_setf(adv_tim_x_base,3);
            break;
    
        default:
            break;
    }

    if (adv_tim_init->adv_tim_cnt_mode == ADV_TIMER_CNT_MODE_INC) {
        pwm_cnt_mode_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_cnt_mode == ADV_TIMER_CNT_MODE_DEC) {
        pwm_cnt_mode_setf(adv_tim_x_base,1);
    }
    else if (adv_tim_init->adv_tim_cnt_mode == ADV_TIMER_CNT_MODE_BOTH) {
        pwm_cnt_mode_setf(adv_tim_x_base,2);
    }

    if (adv_tim_init->adv_tim_chb_inv_en == HAL_DISABLE) {
        pwm_invb_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_chb_inv_en == HAL_ENABLE) {
        pwm_invb_setf(adv_tim_x_base,1);
    }

    if (adv_tim_init->adv_tim_cha_inv_en == HAL_DISABLE) {
        pwm_inva_setf(adv_tim_x_base,0);
    }
    else if (adv_tim_init->adv_tim_cha_inv_en == HAL_ENABLE) {
        pwm_inva_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_deinit(void)
{
    sysc_cmp_srstn_pwm_setf(0);
    sysc_cmp_srstn_pwm_setf(1);

    sysc_cmp_pwm_gate_en_setf(0);
}

void hal_adv_tim_cap_en(uint32_t adv_tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        pwm_cap_en_setf(adv_tim_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        pwm_cap_en_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_tim_en(uint32_t adv_tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        pwm_tim_en_setf(adv_tim_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        pwm_tim_en_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_dead_gap_en(uint32_t adv_tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        pwm_dben_setf(adv_tim_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        pwm_dben_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_a_en(uint32_t adv_tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        pwm_ena_setf(adv_tim_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        pwm_ena_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_b_en(uint32_t adv_tim_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        pwm_enb_setf(adv_tim_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        pwm_enb_setf(adv_tim_x_base,1);
    }
}

void hal_adv_tim_set_dead_gap(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_DEAD_GAP(value));
    pwm_db_setf(adv_tim_x_base,value);
}

/**
 * @note if you want to modify the load value,you must disable the model first.
 */
void hal_adv_tim_set_load_value(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_LOAD_VALUE(value));
    pwm_load_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_comp_b(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CMPB_VALUE(value));
    pwm_cmp_b_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_comp_a(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CMPA_VALUE(value));
    pwm_cmp_a_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_inv_a(uint32_t adv_tim_x_base,uint8_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CHA_INV_EN(value));
    pwm_inva_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_inv_b(uint32_t adv_tim_x_base,uint8_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CHB_INV_EN(value));
    pwm_invb_setf(adv_tim_x_base,value);
}


void hal_adv_tim_set_trig_value(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_TRIG_VALUE(value));
    pwm_trig_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_clock_div(uint32_t adv_tim_x_base,uint16_t value)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CLK_DIV(value));
    pwm_div_setf(adv_tim_x_base,value);
}

void hal_adv_tim_set_cap_edge(uint32_t adv_tim_x_base,adv_tim_cap_edg_t adv_tim_cap_edg)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_TIMER_CAP_EDG(adv_tim_cap_edg));
    switch (adv_tim_cap_edg)
    {
        case ADV_TIMER_EDG_DIS:
            pwm_cap_edg_setf(adv_tim_x_base,0);
            break;

        case ADV_TIMER_EDG_RISE:
            pwm_cap_edg_setf(adv_tim_x_base,1);
            break;

        case ADV_TIMER_EDG_FALL:
            pwm_cap_edg_setf(adv_tim_x_base,2);
            break;

        case ADV_TIMER_EDG_BOTH:
            pwm_cap_edg_setf(adv_tim_x_base,3);
            break;
    
        default:
            break;
    }
}

uint8_t hal_adv_tim_get_clock_div(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_div_getf(adv_tim_x_base);
}


uint8_t hal_adv_tim_get_cap_dege(uint32_t adv_tim_x_base)
{
    uint8_t reg_data = 0;
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    reg_data = pwm_cap_edg_getf(adv_tim_x_base);
    return reg_data;
}

uint16_t hal_adv_tim_get_trig_value(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_trig_getf(adv_tim_x_base);
}

uint16_t hal_adv_tim_get_load_value(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_load_getf(adv_tim_x_base);
}

uint16_t hal_adv_tim_get_count(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_count_get(adv_tim_x_base);
}

uint16_t hal_adv_tim_get_comp_b(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_cmp_b_getf(adv_tim_x_base);
}

uint16_t hal_adv_tim_get_comp_a(uint32_t adv_tim_x_base)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    return pwm_cmp_a_getf(adv_tim_x_base);
}

void hal_adv_tim_it_cfg(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_ADV_TIM_IT_FLAG(adv_tim_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (adv_tim_x_base)
    {
        case ADV_TIMER_0_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise0_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise0_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise0_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise0_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise0_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise0_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise0_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise0_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_1_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise1_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise1_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise1_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise1_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise1_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise1_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise1_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise1_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }    
        case ADV_TIMER_2_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise2_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise2_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise2_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise2_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise2_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise2_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise2_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise2_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_3_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise3_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise3_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise3_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise3_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise3_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise3_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise3_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise3_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }            
        case ADV_TIMER_4_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise4_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise4_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise4_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise4_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise4_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise4_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise4_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise4_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }        
        case ADV_TIMER_5_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    if (en == HAL_DISABLE) {
                        pwm_ise5_load_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise5_load_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    if (en == HAL_DISABLE) {
                        pwm_ise5_trig_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise5_trig_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    if (en == HAL_DISABLE) {
                        pwm_ise5_cmpa_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise5_cmpa_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    if (en == HAL_DISABLE) {
                        pwm_ise5_cmpb_setf(ADV_TIMER_IT_BASE,0);
                    }
                    else if (en == HAL_ENABLE) {
                        pwm_ise5_cmpb_setf(ADV_TIMER_IT_BASE,1);
                    }
                    break;
                    
                default:
                    break;
            }
            break;
        }   
        
        default:
            break;
    }
}

uint8_t hal_adv_tim_get_it_flag(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_ADV_TIM_IT_FLAG(adv_tim_it_flag));

    switch (adv_tim_x_base)
    {
        case ADV_TIMER_0_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr0_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr0_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr0_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr0_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_1_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr1_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr1_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr1_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr1_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_2_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr2_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr2_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr2_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr2_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_3_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr3_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr3_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr3_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr3_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_4_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr4_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr4_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr4_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr4_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_5_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    it_flag = pwm_isr5_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    it_flag = pwm_isr5_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    it_flag = pwm_isr5_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    it_flag = pwm_isr5_cmpb_getf(ADV_TIMER_IT_BASE);
                    
                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
    return it_flag;
}

void hal_adv_tim_clr_it_flag(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_ADV_TIM_IT_FLAG(adv_tim_it_flag));

    // only writting 1 to the interrupt state raw rigister can clear the it flag
    switch (adv_tim_x_base)
    {
        case ADV_TIMER_0_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_1_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 4);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 4);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 4);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 4);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_2_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 8);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 8);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 8);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 8);
                    break;
                default:
                    break;
            }
            break;
        }            
        case ADV_TIMER_3_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 12);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 12);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 12);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 12);
                    break;
                default:
                    break;
            }
            break;
        }              
        case ADV_TIMER_4_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 16);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 16);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 16);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 16);
                    
                default:
                    break;
            }
            break;
        }  
        case ADV_TIMER_5_BASE:
        {
            switch (adv_tim_it_flag)
            {
                case ADV_TIMER_IT_FLAG_LOAD:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 20);
                    break;
                    
                case ADV_TIMER_IT_FLAG_TRIG:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 20);
                    break;

                case ADV_TIMER_IT_FLAG_CMPA:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 20);
                    break;

                case ADV_TIMER_IT_FLAG_CMPB:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 20);
                    break;
                    
                default:
                    break;
            }
            break;
        }  

        default:
            break;
    }
}

uint8_t hal_adv_tim_get_status_flag(uint32_t adv_tim_x_base,adv_tim_status_flag_t adv_tim_status_flag)
{
    uint8_t status_flag = 0;
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_ADV_TIM_IT_FLAG(adv_tim_status_flag));

    switch (adv_tim_x_base)
    {
        case ADV_TIMER_0_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr0_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr0_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr0_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr0_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_1_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr1_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr1_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr1_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr1_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_2_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr2_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr2_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr2_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr2_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_3_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr3_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr3_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr3_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr3_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_4_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr4_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr4_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr4_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr4_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_5_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    status_flag = pwm_isrr5_load_getf(ADV_TIMER_IT_BASE);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    status_flag = pwm_isrr5_trig_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    status_flag = pwm_isrr5_cmpa_getf(ADV_TIMER_IT_BASE);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    status_flag = pwm_isrr5_cmpb_getf(ADV_TIMER_IT_BASE);
                    break;
                default:
                    break;
            }
            break;
        }

        default:
            break;
    }
    return status_flag;
}

void hal_adv_tim_clr_status_flag(uint32_t adv_tim_x_base,adv_tim_status_flag_t adv_tim_status_flag)
{
    /* check the parameters */
    hal_assert(IS_ADV_TIMER_ALL_PERIPH(adv_tim_x_base));
    hal_assert(IS_ADV_TIM_IT_FLAG(adv_tim_status_flag));

    // only writting 1 to the interrupt state raw rigister can clear the status flag
    switch (adv_tim_x_base)
    {
        case ADV_TIMER_0_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_1_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 4);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 4);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 4);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 4);
                    break;
                default:
                    break;
            }
            break;
        }
        case ADV_TIMER_2_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 8);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 8);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 8);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 8);
                    break;
                default:
                    break;
            }
            break;
        }            
        case ADV_TIMER_3_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 12);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 12);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 12);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 12);
                    break;
                default:
                    break;
            }
            break;
        }              
        case ADV_TIMER_4_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 16);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 16);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 16);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 16);
                    
                default:
                    break;
            }
            break;
        }  
        case ADV_TIMER_5_BASE:
        {
            switch (adv_tim_status_flag)
            {
                case ADV_TIMER_STATUS_FLAG_LOAD_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,1 << 20);
                    break;
                    
                case ADV_TIMER_STATUS_FLAG_TRIG_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,2 << 20);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPA_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,4 << 20);
                    break;

                case ADV_TIMER_STATUS_FLAG_CMPB_RAW:
                    pwm_isrr_set(ADV_TIMER_IT_BASE,8 << 20);
                    
                default:
                    break;
            }
            break;
        }  

        default:
            break;
    }
}

/************************END OF FILE*******************************/


