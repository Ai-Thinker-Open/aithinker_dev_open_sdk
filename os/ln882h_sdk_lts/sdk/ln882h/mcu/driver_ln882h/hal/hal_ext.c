/**
 * @file     hal_ext.c
 * @author   BSP Team 
 * @brief    This file provides EXT function.
 * @version  0.0.0.1
 * @date     2021-02-20
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_ext.h"

void hal_ext_init(ext_int_sense_t ext_int_sense,ext_trig_mode_t ext_trig_mode,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_EXT_INT_SENSE(ext_int_sense));
    hal_assert(IS_EXT_TRIG_MODE(ext_trig_mode));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    uint8_t en_status = sysc_awo_ext_inrp_en_getf();
    uint8_t pin_status = sysc_awo_pin_is_inrp_src_getf();
    

    switch (ext_int_sense)
    {
        case EXT_INT_SENSE_0:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_0_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_0_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_0_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_0_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 0;
                pin_status|= 1 << 0;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 0);
                pin_status &= ~(1 << 0);
            }
            break;
        }
        case EXT_INT_SENSE_1:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_1_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_1_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_1_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_1_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 1;
                pin_status|= 1 << 1;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 1);
                pin_status &= ~(1 << 1);
            }
            break;
        }
        case EXT_INT_SENSE_2:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_2_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_2_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_2_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_2_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 2;
                pin_status|= 1 << 2;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 2);
                pin_status &= ~(1 << 2);
            }
            break;
        }
        case EXT_INT_SENSE_3:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_3_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_3_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_3_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_3_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 3 << 0;
                pin_status|= 3 << 0;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(3 << 0);
                pin_status &= ~(3 << 0);
            }
            break;
        }
        case EXT_INT_SENSE_4:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_4_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_4_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_4_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_4_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 4;
                pin_status|= 1 << 4;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 4);
                pin_status &= ~(1 << 4);
            }
            break;
        }
        case EXT_INT_SENSE_5:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_5_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_5_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_5_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_5_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 5;
                pin_status|= 1 << 5;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 5);
                pin_status &= ~(1 << 5);
            }
            break;
        }
        case EXT_INT_SENSE_6:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_6_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_6_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_6_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_6_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 6;
                pin_status|= 1 << 6;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 6);
                pin_status &= ~(1 << 6);
            }
            break;
        }
        case EXT_INT_SENSE_7:
        {
            switch (ext_trig_mode)
            {
                case EXT_INT_HIGH_LEVEL:
                    sysc_awo_ext_inrp_sense_7_setf(0);
                    break;
                case EXT_INT_LOW_LEVEL:
                    sysc_awo_ext_inrp_sense_7_setf(1);
                    break;
                case EXT_INT_POSEDEG:
                    sysc_awo_ext_inrp_sense_7_setf(2);
                    break;
                case EXT_INT_NEGEDGE:
                    sysc_awo_ext_inrp_sense_7_setf(3);
                    break;
                default:
                    break;
            }
            if(en == HAL_ENABLE){
                en_status |= 1 << 7;
                pin_status|= 1 << 7;
            }
            else if(en == HAL_DISABLE){
                en_status &= ~(1 << 7);
                pin_status &= ~(1 << 7);
            }
            break;
        }  
        default:
            break;
    }
    sysc_awo_ext_inrp_en_setf(en_status);
    sysc_awo_pin_is_inrp_src_setf(pin_status);
}

void hal_ext_deinit(void)
{
    sysc_awo_ext_intr_sence_set(0);
    sysc_awo_ext_intr_en_set(0);
}

uint8_t hal_ext_get_it_flag(ext_it_flag_t ext_it_flag)
{
    /* check the parameters */
    hal_assert(IS_EXT_IT_FLAG(ext_it_flag));
    uint8_t it_flag = sysc_awo_ext_inrp_value_r_getf();
    switch (ext_it_flag)
    {
        case EXT_INT_SENSE_0_IT_FLAG:
            it_flag = ( it_flag >> 0 ) & 0x01;
            break;
        case EXT_INT_SENSE_1_IT_FLAG:
            it_flag = ( it_flag >> 1 ) & 0x01;
            break;
        case EXT_INT_SENSE_2_IT_FLAG:
            it_flag = ( it_flag >> 2 ) & 0x01;
            break;
        case EXT_INT_SENSE_3_IT_FLAG:
            it_flag = ( it_flag >> 3 ) & 0x01;
            break;
        case EXT_INT_SENSE_4_IT_FLAG:
            it_flag = ( it_flag >> 4 ) & 0x01;
            break;
        case EXT_INT_SENSE_5_IT_FLAG:
            it_flag = ( it_flag >> 5 ) & 0x01;
            break;
        case EXT_INT_SENSE_6_IT_FLAG:
            it_flag = ( it_flag >> 6 ) & 0x01;
            break;
        case EXT_INT_SENSE_7_IT_FLAG:
            it_flag = ( it_flag >> 7 ) & 0x01;
            break;
        default:
            break;
    }
    return it_flag;
}

uint8_t hal_ext_get_raw_it_flag(ext_it_raw_flag_t ext_raw_it_flag)
{
    /* check the parameters */
    hal_assert(IS_EXT_RAW_IT_FLAG(ext_raw_it_flag));
    uint8_t raw_it_flag = sysc_awo_ext_rawinrp_value_getf();
    switch (ext_raw_it_flag)
    {
        case EXT_INT_SENSE_0_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 0 ) & 0x01;
            break;
        case EXT_INT_SENSE_1_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 1 ) & 0x01;
            break;
        case EXT_INT_SENSE_2_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 2 ) & 0x01;
            break;
        case EXT_INT_SENSE_3_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 3 ) & 0x01;
            break;
        case EXT_INT_SENSE_4_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 4 ) & 0x01;
            break;
        case EXT_INT_SENSE_5_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 5 ) & 0x01;
            break;
        case EXT_INT_SENSE_6_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 6 ) & 0x01;
            break;
        case EXT_INT_SENSE_7_RAW_IT_FLAG:
            raw_it_flag = ( raw_it_flag >> 7 ) & 0x01;
            break;
        default:
            break;
    }
    return raw_it_flag;
}
void hal_ext_clr_it_flag(ext_it_flag_t ext_it_flag)
{
    /* check the parameters */
    hal_assert(IS_EXT_IT_FLAG(ext_it_flag));
    uint8_t clear_bit = 0;
    switch (ext_it_flag)
    {
        case EXT_INT_SENSE_0_IT_FLAG:
            clear_bit = ( 1 << 0 ) & 0xFF;
            break;
        case EXT_INT_SENSE_1_IT_FLAG:
            clear_bit = ( 1 << 1 ) & 0xFF;
            break;
        case EXT_INT_SENSE_2_IT_FLAG:
            clear_bit = ( 1 << 2 ) & 0xFF;
            break;
        case EXT_INT_SENSE_3_IT_FLAG:
            clear_bit = ( 1 << 3 ) & 0xFF;
            break;
        case EXT_INT_SENSE_4_IT_FLAG:
            clear_bit = ( 1 << 4 ) & 0xFF;
            break;
        case EXT_INT_SENSE_5_IT_FLAG:
            clear_bit = ( 1 << 5 ) & 0xFF;
            break;
        case EXT_INT_SENSE_6_IT_FLAG:
            clear_bit = ( 1 << 6 ) & 0xFF;
            break;
        case EXT_INT_SENSE_7_IT_FLAG:
            clear_bit = ( 1 << 7 ) & 0xFF;
            break;
        default:
            break;
    }
    sysc_awo_ext_inrp_clr_r_setf(clear_bit);
}

