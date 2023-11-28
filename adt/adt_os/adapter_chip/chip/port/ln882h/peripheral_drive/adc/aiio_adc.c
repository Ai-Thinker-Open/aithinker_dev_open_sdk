/** @brief      adc application interface.
 *
 *  @file       aiio_adc.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>adc Init
 *  </table>
 *
 */
#include "aiio_adc.h"
#include "hal/hal_common.h"
#include "hal/hal_adc.h"
#include "hal/hal_gpio.h"

int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{
    adc_init_t_def adc_init;
    memset(&adc_init, 0, sizeof(adc_init));

    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH0:
        adc_init.adc_ch = ADC_CH0;
        break;
    case AIIO_ADC_CH1:
        adc_init.adc_ch = ADC_CH1;
        break;
    case AIIO_ADC_CH2:
        adc_init.adc_ch = ADC_CH2;
        break;
    case AIIO_ADC_CH3:
        adc_init.adc_ch = ADC_CH3;
        break;
    case AIIO_ADC_CH4:
        adc_init.adc_ch = ADC_CH4;
        break;
    case AIIO_ADC_CH5:
        adc_init.adc_ch = ADC_CH5;
        break;
    case AIIO_ADC_CH6:
        adc_init.adc_ch = ADC_CH6;
        break;
    case AIIO_ADC_CH7:
        adc_init.adc_ch = ADC_CH7;
        break;
    default:
        break;
    }

    switch (aiio_adc_cfg->mode)
    {
    case AIIO_ADC_ONE_SHOT:
        adc_init.adc_conv_mode = ADC_CONV_MODE_SINGLE; // 配置ADC为连续转换模式
        break;
    case AIIO_ADC_CONTIUE:
        adc_init.adc_conv_mode = ADC_CONV_MODE_CONTINUE; // 配置ADC为连续转换模式
        break;
    default:
        break;
    }

    switch (aiio_adc_cfg->resolution)
    {
    case AIIO_ADC_RESOLUTION_8:

        break;
    case AIIO_ADC_RESOLUTION_9:

        break;
    case AIIO_ADC_RESOLUTION_10:

        break;
    case AIIO_ADC_RESOLUTION_11:

        break;
    case AIIO_ADC_RESOLUTION_12:

        break;
    default:
        break;
    }

    switch (aiio_adc_cfg->vref)
    {
    case AIIO_ADC_VREF_0_8_V:
        adc_init.adc_vref_set = ADC_VREF_0_8_V;
        break;
    case AIIO_ADC_VREF_0_8_5_V:
        adc_init.adc_vref_set = ADC_VREF_0_8_5_V;
        break;
    case AIIO_ADC_VREF_0_9_5_V:
        adc_init.adc_vref_set = ADC_VREF_0_9_5_V;
        break;
    case AIIO_ADC_VREF_1_0_5_V:
        adc_init.adc_vref_set = ADC_VREF_1_0_5_V;
        break;
    case AIIO_ADC_VREF_VCC:

        break;
    default:
        break;
    }

    adc_init.adc_presc = 80;           // 设置采样周期为1M
    hal_adc_init(ADC_BASE, &adc_init); // 初始化ADC

    // 将对应引脚设置为模拟模式
    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH0:

        break;
    case AIIO_ADC_CH1:

        break;
    case AIIO_ADC_CH2:
        hal_gpio_pin_mode_set(GPIOA_BASE, GPIO_PIN_0, GPIO_MODE_ANALOG);
        break;
    case AIIO_ADC_CH3:
        hal_gpio_pin_mode_set(GPIOA_BASE, GPIO_PIN_1, GPIO_MODE_ANALOG);
        break;
    case AIIO_ADC_CH4:
        hal_gpio_pin_mode_set(GPIOA_BASE, GPIO_PIN_4, GPIO_MODE_ANALOG);
        break;
    case AIIO_ADC_CH5:
        hal_gpio_pin_mode_set(GPIOB_BASE, GPIO_PIN_3, GPIO_MODE_ANALOG);
        break;
    case AIIO_ADC_CH6:
        hal_gpio_pin_mode_set(GPIOB_BASE, GPIO_PIN_4, GPIO_MODE_ANALOG);
        break;
    case AIIO_ADC_CH7:
        hal_gpio_pin_mode_set(GPIOB_BASE, GPIO_PIN_5, GPIO_MODE_ANALOG);
        break;
    default:
        break;
    }

    hal_adc_en(ADC_BASE, HAL_ENABLE);
    hal_adc_start_conv(ADC_BASE);

    return AIIO_OK;
}

// int32_t aiio_adc_start(void)
// {
//     hal_adc_en(ADC_BASE, HAL_ENABLE);
//     hal_adc_start_conv(ADC_BASE);
//     return AIIO_OK;
// }

// int32_t aiio_adc_stop(void)
// {
//     hal_adc_en(ADC_BASE, HAL_DISABLE);
//     hal_adc_stop_conv(ADC_BASE);
//     return AIIO_OK;
// }

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    int32_t ret_data = 0;
    adc_ch_t adc_ch;

    switch (change)
    {
    case AIIO_ADC_CH0:
        adc_ch = ADC_CH0;
        break;
    case AIIO_ADC_CH1:
        adc_ch = ADC_CH1;
        break;
    case AIIO_ADC_CH2:
        adc_ch = ADC_CH2;
        break;
    case AIIO_ADC_CH3:
        adc_ch = ADC_CH3;
        break;
    case AIIO_ADC_CH4:
        adc_ch = ADC_CH4;
        break;
    case AIIO_ADC_CH5:
        adc_ch = ADC_CH5;
        break;
    case AIIO_ADC_CH6:
        adc_ch = ADC_CH6;
        break;
    case AIIO_ADC_CH7:
        adc_ch = ADC_CH7;
        break;
    default:
        break;
    }

    while (hal_adc_get_conv_status(ADC_BASE, adc_ch) != HAL_SET)
        ;

    ret_data = hal_adc_get_data(ADC_BASE, adc_ch);

    return ret_data;
}
