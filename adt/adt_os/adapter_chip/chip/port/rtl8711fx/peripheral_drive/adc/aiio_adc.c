/** @brief      adc application interface.
 *
 *  @file       aiio_adc.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/04/08      <td>1.0.0       <td>hongjz      <td>adc api
 *  </table>
 *
 */
#include "aiio_adc.h"
#include "aiio_log.h"

#include "device.h"
#include "analogin_api.h"
#include "sys_api.h"

int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{
    static u8 ADC_CHAN;
    static u8 ADC_PAD;

    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH0:
        ADC_CHAN = ADC_CH0;
        ADC_PAD = _PB_19;
        break;

    case AIIO_ADC_CH1:
        ADC_CHAN = ADC_CH1;
        ADC_PAD = _PB_18;
        break;

    case AIIO_ADC_CH2:
        ADC_CHAN = ADC_CH2;
        ADC_PAD = _PB_17;
        break;

    case AIIO_ADC_CH3:
        ADC_CHAN = ADC_CH3;
        ADC_PAD = _PB_16;
        break;

    case AIIO_ADC_CH4:
        ADC_CHAN = ADC_CH4;
        ADC_PAD = _PB_15;
        break;

    case AIIO_ADC_CH5:
        ADC_CHAN = ADC_CH5;
        ADC_PAD = _PB_14;
        break;

    case AIIO_ADC_CH6:
        ADC_CHAN = ADC_CH6;
        ADC_PAD = _PB_13;
        break;

    default:
        aiio_log_e("channel not supported!");
        return AIIO_ERROR;
    }
	/* Enable ADC clock and function */
	RCC_PeriphClockCmd(APBPeriph_ADC, APBPeriph_ADC_CLOCK, ENABLE);

	/* Initialize ADC pad */
    Pinmux_Config(ADC_PAD, PINMUX_FUNCTION_ADC);
    PAD_PullCtrl(ADC_PAD, GPIO_PuPd_NOPULL);
    PAD_SleepPullCtrl(ADC_PAD, GPIO_PuPd_NOPULL);
    PAD_InputCtrl(ADC_PAD, DISABLE);


	ADC_InitTypeDef ADC_InitStruct; 
	/* Initialize ADC to Software-trigger mode */
	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_OpMode = ADC_SW_TRI_MODE;
	ADC_InitStruct.ADC_CvlistLen = 0; 	/* ADC_CvlistLen should be channel number -1*/
	ADC_InitStruct.ADC_Cvlist[0] = ADC_CHAN;

	// ADC_InitStruct.ADC_Cvlist[1] = ADC_CH3;
	//ADC_InitStruct.ADC_ChIDEn = ENABLE; /* MSB 4bit is channel index*/
	ADC_Init(&ADC_InitStruct);
	ADC_Cmd(ENABLE);

    return AIIO_OK;
}

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    u32 adc_idx, adc_data, value;
	s32 adc_vol;

	ADC_ClearFIFO();

	ADC_AutoCSwCmd(ENABLE);

    while (ADC_Readable() == 0);
    value = ADC_Read();
	ADC_AutoCSwCmd(DISABLE);

    adc_idx = ADC_GET_CH_NUM_GLOBAL(value);
    adc_data = ADC_GET_DATA_GLOBAL(value);
    adc_vol = ADC_GetVoltage(adc_data);

    aiio_log_e("AD%lu:%ld => %ldmV\n", adc_idx, adc_data, adc_vol);

    return (int32_t)adc_data;
}
