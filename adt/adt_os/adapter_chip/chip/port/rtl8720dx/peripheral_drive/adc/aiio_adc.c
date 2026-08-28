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

#define AD2MV(ad,offset,gain) ((10 * ad - offset) * 1000 / gain)

static uint16_t OFFSET;
static uint16_t GAIN_DIV;

int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{
    u8 EfuseBuf[2];
    u32 index;
    u32 addressOffset = 0x1D0;
    u32 addressGain = 0x1D2;

    switch (aiio_adc_cfg->change)
    {
    case AIIO_ADC_CH6:
        /* code */
        break;
    default:
        aiio_log_e("channel not supported!");
        return AIIO_ERROR;
    }

    for (index = 0; index < 2; index++)
    {
        EFUSERead8(0, addressOffset + index, EfuseBuf + index, L25EOUTVOLTAGE);
    }
    OFFSET = EfuseBuf[1] << 8 | EfuseBuf[0];

    for (index = 0; index < 2; index++)
    {
        EFUSERead8(0, addressGain + index, EfuseBuf + index, L25EOUTVOLTAGE);
    }
    GAIN_DIV = EfuseBuf[1] << 8 | EfuseBuf[0];

    if (OFFSET == 0xFFFF)
    {
        OFFSET = 0x9B0;
    }

    if (GAIN_DIV == 0xFFFF)
    {
        GAIN_DIV = 0x2F12;
    }

    PAD_CMD(_PB_3, DISABLE);

	ADC_InitTypeDef ADC_InitStruct; 
	/* Initialize ADC to Software-trigger mode */
	ADC_StructInit(&ADC_InitStruct);
	ADC_InitStruct.ADC_OpMode = ADC_SW_TRI_MODE;
	ADC_InitStruct.ADC_CvlistLen = 0; 	/* ADC_CvlistLen should be channel number -1*/
	ADC_InitStruct.ADC_Cvlist[0] = ADC_CH6;
	// ADC_InitStruct.ADC_Cvlist[1] = ADC_CH3;
	ADC_InitStruct.ADC_ChIDEn = ENABLE; /* MSB 4bit is channel index*/
	ADC_Init(&ADC_InitStruct);
	ADC_Cmd(ENABLE);

    return AIIO_OK;
}

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    uint16_t ch_idx, data, value;
    int vol;

    /* SW trigger to switch channel and sample */
    ADC_SWTrigCmd(ENABLE);
    while(ADC_Readable()== 0);
    ADC_SWTrigCmd(DISABLE);
    value = ADC_Read();

    ch_idx = (value & BIT_MASK_DAT_CHID) >> 12;
    data = value & BIT_MASK_DAT_GLOBAL;
    vol = AD2MV(data, OFFSET, GAIN_DIV);

    aiio_log_d("AD%d:%x = %d mv\n", ch_idx, data, vol);

    return (int32_t)data;
}
