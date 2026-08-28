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
#include "pinctrl.h"
#include "gpio.h"
#include "adc.h"
#include "adc_porting.h"
#include "common_def.h"
#include "soc_osal.h"
#include "app_init.h"


int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{
    // 初始化ADC模块
    uapi_adc_init(ADC_CLOCK_NONE);

    // 打开指定ADC通道
    adc_port_gadc_entirely_open(aiio_adc_cfg->change, true);

    // // test_afe_set_io(0);
    // int gadc_value = 0;
    // for (int i = 0; i < 100; i++) {
    //     gadc_value =  adc_port_gadc_entirely_sample(gadc_channel);
    //     aiio_log_w("gadc: %dmv\n", gadc_value);
    // }
    // // 关闭ADC通道
    // adc_port_gadc_entirely_close(gadc_channel);

    return AIIO_OK;
}

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    return adc_port_gadc_entirely_sample(change);
}

