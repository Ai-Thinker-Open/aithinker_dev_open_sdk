/** @brief      adc application interface.
 *
 *  @file       aiio_adc.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>adc Init
 *  </table>
 *
 */
#ifndef __AIIO_ADC_H_
#define __AIIO_ADC_H_

#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"

// 通道
typedef enum
{
    AIIO_ADC_CH0 = 0x00,
    AIIO_ADC_CH1 = 0x01,
    AIIO_ADC_CH2 = 0x02,
    AIIO_ADC_CH3 = 0x03,
    AIIO_ADC_CH4 = 0x04,
    AIIO_ADC_CH5 = 0x05,
    AIIO_ADC_CH6 = 0x06,
    AIIO_ADC_CH7 = 0x07,
    AIIO_ADC_CH8 = 0x08,
    AIIO_ADC_CH9 = 0x09,
    AIIO_ADC_CH10 = 0x0a,        
    AIIO_ADC_CH11 = 0x0b,
} aiio_adc_change_t;

// 基准电压
typedef enum
{
    AIIO_ADC_VREF_0_8_V = 0x00,     //LN882H
    AIIO_ADC_VREF_0_8_5_V = 0x01,   //LN882H
    AIIO_ADC_VREF_0_9_5_V = 0x02,   //LN882H
    AIIO_ADC_VREF_1_0_5_V = 0x03,   //LN882H
    AIIO_ADC_VREF_2_0_V = 0x04,   //BL616/618
    AIIO_ADC_VREF_3_2_V = 0x05,     //BL616/618
    AIIO_ADC_VREF_VCC = 0xFF,
} aiio_adc_vref_t;

// 模式
typedef enum
{
    AIIO_ADC_ONE_SHOT,
    AIIO_ADC_CONTIUE
} aiio_adc_mode_t;

// 分辨率
typedef enum
{
    AIIO_ADC_RESOLUTION_8 = 0x00,
    AIIO_ADC_RESOLUTION_9 = 0x01,
    AIIO_ADC_RESOLUTION_10 = 0x02,
    AIIO_ADC_RESOLUTION_11 = 0x03,
    AIIO_ADC_RESOLUTION_12 = 0x04,
    AIIO_ADC_RESOLUTION_14 = 0x05,
    AIIO_ADC_RESOLUTION_16 = 0x06,
} aiio_adc_resolution_t;

/**
 *  @brief adc pin config interface
 */
typedef struct
{
    aiio_adc_change_t change;
    aiio_adc_mode_t mode;
    aiio_adc_resolution_t resolution;
    aiio_adc_vref_t vref;
} aiio_adc_cfg_t;

CHIP_API int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg);

// CHIP_API int32_t aiio_adc_start(void);

// CHIP_API int32_t aiio_adc_stop(void);

CHIP_API int32_t aiio_adc_get_value(aiio_adc_change_t change);

#endif
