/**
 * @file     hal_i2s.c
 * @author   BSP Team 
 * @brief    This file provides I2S function.
 * @version  0.0.0.1
 * @date     2021-02-19
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_i2s.h"

void hal_i2s_init(uint32_t i2s_base,i2s_init_t_def* i2s_init)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));

    hal_assert(IS_I2S_REC_CONFIG(i2s_init->i2s_rec_config));
    hal_assert(IS_I2S_TRA_CONFIG(i2s_init->i2s_tra_config));
    hal_assert(IS_I2S_REC_FIFO_CONFIG(i2s_init->rec_fifo_trig_lev));
    hal_assert(IS_I2S_TRA_FIFO_CONFIG(i2s_init->tra_fifo_trig_lev));

    sysc_cmp_i2s_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    /* enable the i2s on the system*/
    sysc_cmp_i2s_io_en0_setf(1);

    switch (i2s_init->i2s_rec_config)
    {
        case I2S_REC_CONFIG_IGNORE_LENGTH:
            i2s_ch0_rcr_setf(0);    
            break;
        case I2S_REC_CONFIG_12_BIT:
            i2s_ch0_rcr_setf(1);    
            break;
        case I2S_REC_CONFIG_16_BIT:
            i2s_ch0_rcr_setf(2);    
            break;
        case I2S_REC_CONFIG_20_BIT:
            i2s_ch0_rcr_setf(3);    
            break;
        case I2S_REC_CONFIG_24_BIT:
            i2s_ch0_rcr_setf(4);    
            break;
        case I2S_REC_CONFIG_32_BIT:
            i2s_ch0_rcr_setf(5);    
            break;
        default:
            break;
    }

    switch (i2s_init->i2s_tra_config)
    {
        case I2S_TRA_CONFIG_IGNORE_LENGTH:
            i2s_ch0_tcr_setf(0);    
            break;
        case I2S_TRA_CONFIG_12_BIT:
            i2s_ch0_tcr_setf(1);    
            break;
        case I2S_TRA_CONFIG_16_BIT:
            i2s_ch0_tcr_setf(2);    
            break;
        case I2S_TRA_CONFIG_20_BIT:
            i2s_ch0_tcr_setf(3);    
            break;
        case I2S_TRA_CONFIG_24_BIT:
            i2s_ch0_tcr_setf(4);    
            break;
        case I2S_TRA_CONFIG_32_BIT:
            i2s_ch0_tcr_setf(5);    
            break;
        default:
            break;
    }
    i2s_ch0_rfcr_setf(i2s_init->rec_fifo_trig_lev);
    i2s_ch0_tfcr_setf(i2s_init->tra_fifo_trig_lev);
}

void hal_i2s_deinit(void)
{
    sysc_cmp_srstn_i2s_setf(0);
    sysc_cmp_srstn_i2s_setf(1);

    sysc_cmp_i2s_gate_en_setf(0);
}

void hal_i2s_rx_en(uint32_t i2s_base,hal_en_t en)     
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if(en == HAL_ENABLE){
        i2s_ch0_rer_setf(1);
        i2s_irer_setf(1);
    }
    else if(en == HAL_DISABLE){
        i2s_ch0_rer_setf(0);
        i2s_irer_setf(0);
    }
}   

void hal_i2s_tx_en(uint32_t i2s_base,hal_en_t en)           
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if(en == HAL_ENABLE){
        i2s_ch0_ter_setf(1);
        i2s_iter_setf(1);
    }
    else if(en == HAL_DISABLE){
        i2s_ch0_ter_setf(0);
        i2s_iter_setf(0);
    }
}

void hal_i2s_en(uint32_t i2s_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if(en == HAL_ENABLE){
        i2s_ier_setf(1);
    }
    else if(en == HAL_DISABLE){
        i2s_ier_setf(0);
    }
}

void hal_i2s_tx_fifo_flush(uint32_t i2s_base)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    i2s_ch0_tff_set(1);
}

void hal_i2s_rx_fifo_flush(uint32_t i2s_base)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    i2s_ch0_rff_set(1);
}

void hal_i2s_send_data(uint32_t i2s_base,uint32_t left_data,uint32_t right_data)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    i2s_ch0_lrbr_setf(left_data);
    i2s_ch0_rrbr_setf(right_data);
}
void hal_i2s_recv_data(uint32_t i2s_base,uint32_t *left_data,uint32_t *right_data)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    *left_data  = i2s_ch0_lrbr_getf();
    *right_data = i2s_ch0_rrbr_getf();
}
            
void hal_i2s_it_cfg(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_I2S_IT_FLAG(i2s_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    uint8_t it_status = i2s_ch0_imr_getf();
    switch (i2s_it_flag)
    {
        case I2S_IT_FLAG_TXFO:
            if(en == HAL_ENABLE){
                it_status &= ~0x20;
            }
            else if(en == HAL_DISABLE){
                it_status |= 0x20;
            }
            break;

        case I2S_IT_FLAG_TXFE:
            if(en == HAL_ENABLE){
                it_status &= ~0x10;
            }
            else if(en == HAL_DISABLE){
                it_status |= 0x10;
            }
            break;

        case I2S_IT_FLAG_RXFO:
            if(en == HAL_ENABLE){
                it_status &= ~0x02;
            }
            else if(en == HAL_DISABLE){
                it_status |= 0x02;
            }
            break;

        case I2S_IT_FLAG_RXDA:
            if(en == HAL_ENABLE){
                it_status &= ~0x01;
            }
            else if(en == HAL_DISABLE){
                it_status |= 0x01;
            }
            break;
        default:
            break;
    }
    i2s_ch0_imr_setf(it_status);
}

uint8_t hal_i2s_get_it_flag(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_I2S_IT_FLAG(i2s_it_flag));

    uint8_t it_flag = 0;
    switch (i2s_it_flag)
    {
        case I2S_IT_FLAG_TXFO:
            it_flag = (i2s_ch0_isr_getf() >> 5) & 0x01;
            break;
        case I2S_IT_FLAG_TXFE:
            it_flag = (i2s_ch0_isr_getf() >> 4) & 0x01;
            break;
        case I2S_IT_FLAG_RXFO:
            it_flag = (i2s_ch0_isr_getf() >> 1) & 0x01;
            break;
        case I2S_IT_FLAG_RXDA:
            it_flag = (i2s_ch0_isr_getf() >> 0) & 0x01;
            break;
        default:
            break;
    }
    return it_flag;
}

void hal_i2s_clr_it_flag(uint32_t i2s_base,i2s_it_flag_t i2s_it_flag)
{
    /* check the parameters */
    hal_assert(IS_I2S_ALL_PERIPH(i2s_base));
    hal_assert(IS_I2S_IT_FLAG(i2s_it_flag));
    switch (i2s_it_flag)
    {
        case I2S_IT_FLAG_TXFO:
            i2s_ch0_tor_getf();
            break;
        case I2S_IT_FLAG_RXFO:
            i2s_ch0_ror_getf();
            break;
        case I2S_IT_FLAG_RXDA:
        case I2S_IT_FLAG_TXFE:
            break;
        default:
            break;
    }
}
