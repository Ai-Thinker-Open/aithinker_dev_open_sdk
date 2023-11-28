/**
 * @file     hal_adc.c
 * @author   BSP Team 
 * @brief    This file provides ADC function.
 * @version  0.0.0.1
 * @date     2021-06-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


/* Includes ------------------------------------------------------------------*/
#include "hal/hal_adc.h"
#include "hal/hal_efuse.h"


static int8_t adc_cal_param_1 = 0;
static int8_t adc_cal_param_2 = 0;
static int8_t adc_cal_param_3 = 0;
static int8_t adc_cal_param_4 = 0;
static float  adc_cal_param_5 = 0;
//static float  adc_cal_param_6 = 0;

/* The ADC_CH0 will be used by WiFi CAL !!!*/
void hal_adc_init(uint32_t adc_base,adc_init_t_def* adc_init)
{
    uint8_t reg_value = 0;
    uint8_t tem_val_1 = 0;
    int16_t tem_val_2 = 0;
    uint32_t tem_val_3 = 0;

    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));

    hal_assert(IS_AWD_ADC_CH(adc_init->adc_awd_ch));
    hal_assert(IS_ADC_AWD_CH_SEL(adc_init->adc_awd_sgl));
    hal_assert(IS_ADC_AUTO_OFF_MODE(adc_init->adc_auto_off_mode));
    hal_assert(IS_ADC_WAIT_CONV_MODE_EN_STATUS(adc_init->adc_wait_conv_mode_en));
    hal_assert(IS_ADC_CONV_MODE(adc_init->adc_conv_mode));
    hal_assert(IS_ADC_DATA_ALIGN_MODE(adc_init->adc_data_align_mode));
    hal_assert(IS_ADC_SPEC_EXT_TRIG_EN_STATUS(adc_init->adc_spec_ext_trig_en));
    hal_assert(IS_ADC_SPEC_EXT_TRIG_EVENT(adc_init->adc_spec_ext_trig_event_sel));
    hal_assert(IS_ADC_EXT_TRIG_EN_STATUS(adc_init->adc_ext_trig_en));
    hal_assert(IS_ADC_EXT_TRIG_EVENT(adc_init->adc_ext_trig_sel));
    hal_assert(IS_ADC_STAB_TIME(adc_init->adc_stab_time));
    hal_assert(IS_ADC_OVER_SAMPLING_RATIO(adc_init->adc_ov_smp_ratio));
    hal_assert(IS_ADC_OVER_SAMPLING_EN_STATUS(adc_init->adc_ov_smp_ratio_en));
    hal_assert(IS_ADC_HIGH_THOD(adc_init->adc_ht));
    hal_assert(IS_ADC_LOW_THOD(adc_init->adc_lt));
    hal_assert(IS_ADC_SPE_CH(adc_init->adc_spe_ch));
    hal_assert(IS_ADC_CH(adc_init->adc_ch));
    hal_assert(IS_ADC_PRESCALE_VALUE(adc_init->adc_presc));
    hal_assert(IS_ADC_VREF_VALUE(adc_init->adc_vref_set));
    
    sysc_cmp_adcc_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();
    
    /*get efuse data*/
    tem_val_3 = hal_efuse_read_corrent_reg(EFUSE_REG_1);
    /*****************modify by liushanbiao 202201004***********************/
    if((tem_val_3 >> 26) & 0x01){
       adc_cal_param_1 = -((tem_val_3 >> 22) & 0x0F) * 6;
    } else {
       adc_cal_param_1 = ((tem_val_3 >> 22) & 0x0F) * 6;
    }
   /**********************************************************************/

    tem_val_1 = (int8_t)(tem_val_3 & 0xFF);
    if (tem_val_1 & 0x80) {
        adc_cal_param_2 = - (tem_val_1 & 0x7F);
    } else {
        adc_cal_param_2 = tem_val_1 & 0x7F;
    }

    tem_val_1 = (int8_t)((tem_val_3 >> 8) & 0xFF);
    if (tem_val_1 & 0x80) {
        adc_cal_param_3 = - (tem_val_1 & 0x7F);
    } else {
        adc_cal_param_3 = tem_val_1 & 0x7F;
    }
    adc_cal_param_5 = 1 - 1.0f * adc_cal_param_3 / 512;                      //modify by liushanbiao
    //adc_cal_param_6 = adc_cal_param_2 * adc_cal_param_5;
    tem_val_2 = 750 - adc_cal_param_1;   //modify by liushanbiao
    tem_val_2 = (uint16_t)((tem_val_2 - adc_cal_param_2) / adc_cal_param_5); //modify by liushanbiao
    adc_cal_param_4 = 750 - tem_val_2; //modify by liushanbiao


    if (adc_init->adc_awd_sgl == ADC_AWD_ALL_CH) {
        adc_awdsgl_setf(adc_base,0);
    }
    else if (adc_init->adc_awd_sgl == ADC_AWD_SGL_CH) {
        adc_awdsgl_setf(adc_base,1);
    }

    if (adc_init->adc_auto_off_mode == ADC_AUTO_OFF_MODE_DIS) {
        adc_autoff_setf(adc_base,0);
    }
    else if (adc_init->adc_auto_off_mode == ADC_AUTO_OFF_MODE_EN) {
        adc_autoff_setf(adc_base,1);
    }

    if (adc_init->adc_wait_conv_mode_en == ADC_WAIT_CONV_MODE_DIS) {
        adc_waitm_setf(adc_base,0);
    }
    else if (adc_init->adc_wait_conv_mode_en == ADC_WAIT_CONV_MODE_EN) {
        adc_waitm_setf(adc_base,1);
    }

    if (adc_init->adc_conv_mode == ADC_CONV_MODE_SINGLE) {
        adc_cont_setf(adc_base,0);
    }
    else if (adc_init->adc_conv_mode == ADC_CONV_MODE_CONTINUE) {
        adc_cont_setf(adc_base,1);
    }

    if (adc_init->adc_data_align_mode == ADC_DATA_ALIGN_RIGHT) {
        adc_align_setf(adc_base,0);
    }
    else if (adc_init->adc_data_align_mode == ADC_DATA_ALIGN_LEFT) {
        adc_align_setf(adc_base,1);
    }

    if (adc_init->adc_spec_ext_trig_en == ADC_SPEC_EXT_TRIG_DIS) {
        adc_spec_exten_setf(adc_base,0);
    }
    else if (adc_init->adc_spec_ext_trig_en == ADC_SPEC_EXT_TRIG_EN) {
        adc_spec_exten_setf(adc_base,1);
    }

    switch (adc_init->adc_spec_ext_trig_event_sel)
    {
        case ADC_SPEC_EXT_TRIG_0:
            adc_spec_extsel_setf(adc_base,0);
            break;

        case ADC_SPEC_EXT_TRIG_1:
            adc_spec_extsel_setf(adc_base,1);
            break;

        case ADC_SPEC_EXT_TRIG_2:
            adc_spec_extsel_setf(adc_base,2);
            break;

        case ADC_SPEC_EXT_TRIG_3:
            adc_spec_extsel_setf(adc_base,3);
            break;
        
        default:
            break;
    }

    switch (adc_init->adc_ext_trig_en)
    {
        case ADC_EXT_TRIG_DIS:
            adc_exten_setf(adc_base,0);
            break;

        case ADC_EXT_TRIG_RISING:
            adc_exten_setf(adc_base,1);
            break;

        case ADC_EXT_TRIG_FALLING:
            adc_exten_setf(adc_base,2);
            break;

        case ADC_EXT_TRIG_RISING_AND_FALLING:
            adc_exten_setf(adc_base,3);
            break;
        
        default:
            break;
    }

    switch (adc_init->adc_ext_trig_sel)
    {
        case ADC_EXT_TRIG_0:
            adc_extsel_setf(adc_base,0);
            break;

        case ADC_EXT_TRIG_1:
            adc_extsel_setf(adc_base,1);
            break;

        case ADC_EXT_TRIG_2:
            adc_extsel_setf(adc_base,2);
            break;

        case ADC_EXT_TRIG_3:
            adc_extsel_setf(adc_base,3);
            break;
        
        default:
            break;
    }

    adc_stab_setf(adc_base,adc_init->adc_stab_time);


    switch (adc_init->adc_ov_smp_ratio)
    {
        case ADC_OVER_SAMPLING_RATIO_X2:
            adc_ovsr_setf(adc_base,0);
            break;

        case ADC_OVER_SAMPLING_RATIO_X4:
            adc_ovsr_setf(adc_base,1);
            break;

        case ADC_OVER_SAMPLING_RATIO_X5:
            adc_ovsr_setf(adc_base,2);
            break;

        case ADC_OVER_SAMPLING_RATIO_X16:
            adc_ovsr_setf(adc_base,3);
            break;
            
        case ADC_OVER_SAMPLING_RATIO_X32:
            adc_ovsr_setf(adc_base,4);
            break;

        case ADC_OVER_SAMPLING_RATIO_X64:
            adc_ovsr_setf(adc_base,5);
            break;
        
        default:
            break;
    }

    adc_ovse_setf(adc_base,adc_init->adc_ov_smp_ratio_en);
    adc_ht_setf(adc_base,adc_init->adc_ht);
    adc_lt_setf(adc_base,adc_init->adc_lt);

    switch (adc_init->adc_spe_ch)
    {
        case ADC_SPE_CH0:
            adc_spec_ch_setf(adc_base,0);
            break;

        case ADC_SPE_CH1:
            adc_spec_ch_setf(adc_base,1);
            break;

        case ADC_SPE_CH2:
            adc_spec_ch_setf(adc_base,2);
            break;

        case ADC_SPE_CH3:
            adc_spec_ch_setf(adc_base,3);
            break;
            
        case ADC_SPE_CH4:
            adc_spec_ch_setf(adc_base,4);
            break;

        case ADC_SPE_CH5:
            adc_spec_ch_setf(adc_base,5);
            break;

        case ADC_SPE_CH6:
            adc_spec_ch_setf(adc_base,6);
            break;

        case ADC_SPE_CH7:
            adc_spec_ch_setf(adc_base,7);
            break;
        
        default:
            break;
    }

    reg_value = adc_chselx_getf(adc_base);
    reg_value |= adc_init->adc_ch;
    
    adc_chselx_setf(adc_base,reg_value);
    adc_presc_setf(adc_base,adc_init->adc_presc);
}

void hal_adc_deinit(void)
{
    sysc_cmp_srstn_adcc_setf(0);
    sysc_cmp_srstn_adcc_setf(1);

    sysc_cmp_adcc_gate_en_setf(0);
}

void hal_adc_dma_en(uint32_t adc_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        adc_dma_en_setf(adc_base,0);
    }
    else if (en == HAL_ENABLE) {
        adc_dma_en_setf(adc_base,1);
    }
}

void hal_adc_awd_en(uint32_t adc_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        adc_awden_setf(adc_base,0);
    }
    else if (en == HAL_ENABLE) {
        adc_awden_setf(adc_base,1);
    }    
}

void hal_adc_en(uint32_t adc_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_DISABLE) {
        adc_aden_setf(adc_base,0);
    }
    else if (en == HAL_ENABLE) {
        adc_aden_setf(adc_base,1);
    }  
}

void hal_adc_start_conv(uint32_t adc_base)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    adc_adstart_setf(adc_base,1);
}

void hal_adc_stop_conv(uint32_t adc_base)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    adc_adstp_setf(adc_base,1);
}

void hal_adc_spe_sw_start(uint32_t adc_base)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    adc_spec_sw_start_setf(adc_base,1);
}

uint16_t hal_adc_get_data(uint32_t adc_base,adc_ch_t ch)
{   
    uint16_t data = 0;
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_CH(ch));

    switch (ch)
    {
        case ADC_CH0:
            data = adc_data_getf(adc_base);
            break;
        case ADC_CH1:
            data = adc_data1_getf(adc_base);
            break;
        case ADC_CH2:
            data = adc_data2_getf(adc_base);
            break;
        case ADC_CH3:
            data = adc_data3_getf(adc_base);
            break;
        case ADC_CH4:
            data = adc_data4_getf(adc_base);
            break;
        case ADC_CH5:
            data = adc_data5_getf(adc_base);
            break;
        case ADC_CH6:
            data = adc_data6_getf(adc_base);
            break;
        case ADC_CH7:
            data = adc_data7_getf(adc_base);
            break;
        default:
            break;
    }
    // data = (uint16_t)((data * adc_cal_param_5 + adc_cal_param_6)); //
    data = (uint16_t)((data - adc_cal_param_2) / adc_cal_param_5); //modify by liushanbiao 20220104

    if(ch == ADC_CH0){
        data += adc_cal_param_4;
        if(data > 4095)
            data = 4095;
    }

    return data;
}

//Check that the conversion is complete
uint8_t hal_adc_get_conv_status(uint32_t adc_base, adc_ch_t ch)
{
    uint8_t conv_status = 0;
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_CH(ch));
    
    switch (ch)
    {
        case ADC_CH0:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_0);
            break;

        case ADC_CH1:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_1);
            break;

        case ADC_CH2:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_2);
            break;

        case ADC_CH3:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_3);
            break;

        case ADC_CH4:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_4);
            break;

        case ADC_CH5:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_5);
            break;
    
        case ADC_CH6:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_6);
            break;

        case ADC_CH7:
            conv_status = hal_adc_get_it_flag(adc_base,ADC_IT_FLAG_EOC_7);
            break;

        default:
            break;
    }
    return conv_status;
}

void hal_adc_clr_conv_status(uint32_t adc_base, adc_ch_t ch)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_CH(ch));
    
    switch (ch)
    {
        case ADC_CH0:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_0);
            break;

        case ADC_CH1:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_1);
            break;

        case ADC_CH2:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_2);
            break;

        case ADC_CH3:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_3);
            break;

        case ADC_CH4:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_4);
            break;

        case ADC_CH5:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_5);
            break;
    
        case ADC_CH6:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_6);
            break;

        case ADC_CH7:
            hal_adc_clr_it_flag(adc_base,ADC_IT_FLAG_EOC_7);
            break;

        default:
            break;
    }
}

void hal_adc_it_cfg(uint32_t adc_base,adc_it_flag_t adc_it_flag ,hal_en_t en)
{
    uint32_t value = 0;
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_IT_FLAG_VALUE(adc_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    
    value = adc_ier_get(adc_base);
    if(en == HAL_ENABLE)
    {
        value |= adc_it_flag;
    }
    else
    {
        value &= ~adc_it_flag;
    }
    adc_ier_set(adc_base,value);
}

uint8_t hal_adc_get_it_flag(uint32_t adc_base,adc_it_flag_t adc_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_IT_FLAG_VALUE(adc_it_flag));

    it_flag = ((adc_isr_get(adc_base) & adc_it_flag) == adc_it_flag) ? HAL_SET : HAL_RESET;
    
    return it_flag;
}

void hal_adc_clr_it_flag(uint32_t adc_base,adc_it_flag_t adc_it_flag)
{
    /* check the parameters */
    hal_assert(IS_ADC_ALL_PERIPH(adc_base));
    hal_assert(IS_ADC_IT_FLAG_VALUE(adc_it_flag));

    adc_isr_set(adc_base,adc_it_flag);
}
