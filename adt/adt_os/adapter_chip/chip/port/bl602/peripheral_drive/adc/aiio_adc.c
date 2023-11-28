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
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include <cli.h>
#include <hosal_adc.h>
#include <blog.h>
#include <bl_adc.h>
#include <hosal_dma.h>

static hosal_adc_dev_t adc0;

int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{
    int ret = 0;

    if(aiio_adc_cfg == NULL)
    {
        return AIIO_ERROR;
    }
    hosal_dma_init();

    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH0:
        adc0.port = 0;
        adc0.config.pin = 12;
        break;
    case AIIO_ADC_CH1:
        adc0.port = 0;
        adc0.config.pin = 5;
        break;
    case AIIO_ADC_CH2:
        adc0.port = 0;
        adc0.config.pin = 14;
        break;
    case AIIO_ADC_CH3:
        adc0.port = 0;
        adc0.config.pin = 8;
        break;
    case AIIO_ADC_CH4:
        adc0.port = 0;
        adc0.config.pin = 5;
        // adc0.config.pin = 4;
        break;
    case AIIO_ADC_CH5:
        aiio_log_w("AIIO_ADC_CH5 null ");
        // adc0.port = 0;
        // adc0.config.pin = 5;
        break;
    case AIIO_ADC_CH6:
        aiio_log_w("AIIO_ADC_CH6 null");
        // adc0.port = 0;
        // adc0.config.pin = 5;
        break;
    case AIIO_ADC_CH7:
        aiio_log_w("AIIO_ADC_CH7 null");
        // adc0.port = 0;
        // adc0.config.pin = 5;
        break;
    default:
        aiio_log_w("change is null");
        break;
    }
    adc0.config.sampling_freq = 340;

    switch (aiio_adc_cfg->mode)
    {
    case AIIO_ADC_ONE_SHOT:
        adc0.config.mode = HOSAL_ADC_ONE_SHOT;
        break;
    case AIIO_ADC_CONTIUE:
        adc0.config.mode = HOSAL_ADC_ONE_SHOT;
        // adc0.config.mode = HOSAL_ADC_CONTINUE;
        break;
    default:
        break;
    }

    //reserve
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

    //reserve
    switch (aiio_adc_cfg->vref)
    {
    case AIIO_ADC_VREF_0_8_V:

        break;
    case AIIO_ADC_VREF_0_8_5_V:

        break;
    case AIIO_ADC_VREF_0_9_5_V:

        break;
    case AIIO_ADC_VREF_1_0_5_V:

        break;
    case AIIO_ADC_VREF_VCC:

        break;
    default:
        break;
    }

    ret = hosal_adc_init(&adc0);
    if (ret != 0)
    {
        aiio_log_w("init adc failed. ");
        return AIIO_ERROR;
    }

    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH0:
        hosal_adc_add_channel(&adc0, 0);
        break;
    case AIIO_ADC_CH1:
        hosal_adc_add_channel(&adc0, 1);
        break;
    case AIIO_ADC_CH2:
        hosal_adc_add_channel(&adc0, 2);
        break;
    case AIIO_ADC_CH3:
        hosal_adc_add_channel(&adc0, 3);
        break;
    case AIIO_ADC_CH4:
        hosal_adc_add_channel(&adc0, 4);
        break;
    case AIIO_ADC_CH5:
        hosal_adc_add_channel(&adc0, 5);
        break;
    case AIIO_ADC_CH6:
        hosal_adc_add_channel(&adc0, 6);
        break;
    case AIIO_ADC_CH7:
        hosal_adc_add_channel(&adc0, 7);
        break;
    default:
        break;
    }
    return AIIO_OK;
}

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    int32_t ret_data = 0;

    switch (change)
    {
    case AIIO_ADC_CH0:
        ret_data = hosal_adc_value_get(&adc0, 0, 100);
        break;
    case AIIO_ADC_CH1:
        ret_data = hosal_adc_value_get(&adc0, 1, 100);
        break;
    case AIIO_ADC_CH2:
        ret_data = hosal_adc_value_get(&adc0, 2, 100);
        break;
    case AIIO_ADC_CH3:
        ret_data = hosal_adc_value_get(&adc0, 3, 100);
        break;
    case AIIO_ADC_CH4:
        ret_data = hosal_adc_value_get(&adc0, 4, 100);
        break;
    case AIIO_ADC_CH5:
        ret_data = hosal_adc_value_get(&adc0, 5, 100);
        break;
    case AIIO_ADC_CH6:
        ret_data = hosal_adc_value_get(&adc0, 6, 100);
        break;
    case AIIO_ADC_CH7:
        ret_data = hosal_adc_value_get(&adc0, 7, 100);
        break;
    default:
        break;
    }

    return ret_data;
}
