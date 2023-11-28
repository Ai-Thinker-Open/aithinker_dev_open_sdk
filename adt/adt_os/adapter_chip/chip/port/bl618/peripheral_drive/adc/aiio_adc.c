/** @brief      adc application interface.
 *
 *  @file       aiio_adc.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       adc application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/08/01      <td>1.0.0       <td>liq         <td>adc Init
 *  </table>
 *
 */
#include "aiio_adc.h"
#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
#include "bflb_adc.h"
#include "bflb_mtimer.h"
#include "bflb_gpio.h"

struct bflb_device_s *adc;

int32_t aiio_adc_init(aiio_adc_cfg_t *aiio_adc_cfg)
{

    int ret = 0;
    struct bflb_adc_channel_s *chan;
    
    if(aiio_adc_cfg == NULL)
    {
        return AIIO_ERROR;
    }

    chan=malloc(sizeof(struct bflb_adc_channel_s));

    struct bflb_device_s *gpio;

    gpio = bflb_device_get_by_name("gpio");

    switch (aiio_adc_cfg->change)
    {
      case AIIO_ADC_CH0:
        chan->pos_chan=ADC_CHANNEL_0;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_20, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH1:
        chan->pos_chan=ADC_CHANNEL_1;
        chan->neg_chan=ADC_CHANNEL_GND;
         bflb_gpio_init(gpio, GPIO_PIN_19, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH2:
        chan->pos_chan=ADC_CHANNEL_2;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_2, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH3:
        chan->pos_chan=ADC_CHANNEL_3;
        chan->neg_chan=ADC_CHANNEL_GND;
         bflb_gpio_init(gpio, GPIO_PIN_3, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH4:
        chan->pos_chan=ADC_CHANNEL_4;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_14, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH5:
        chan->pos_chan=ADC_CHANNEL_5;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_13, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH6:
        chan->pos_chan=ADC_CHANNEL_6;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_12, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH7:
        chan->pos_chan=ADC_CHANNEL_7;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_10, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH8:
        chan->pos_chan=ADC_CHANNEL_8;
        chan->neg_chan=ADC_CHANNEL_GND;
         bflb_gpio_init(gpio, GPIO_PIN_1, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH9:
        chan->pos_chan=ADC_CHANNEL_9;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_0, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH10:
        chan->pos_chan=ADC_CHANNEL_10;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_27, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;
      case AIIO_ADC_CH11:
        chan->pos_chan=ADC_CHANNEL_11;
        chan->neg_chan=ADC_CHANNEL_GND;
        bflb_gpio_init(gpio, GPIO_PIN_28, GPIO_ANALOG | GPIO_SMT_EN | GPIO_DRV_0);
        break;                                                                                        
      
      default:
          aiio_log_w("change is null");
        break;
    }

    adc = bflb_device_get_by_name("adc");

    /* adc clock = XCLK / 2 / 32 */
    struct bflb_adc_config_s cfg;
    cfg.clk_div = ADC_CLK_DIV_32;
    cfg.scan_conv_mode = true;
    if(aiio_adc_cfg->mode)
    {
      cfg.continuous_conv_mode = true;
    }
    else 
    {
      cfg.continuous_conv_mode = false;
    }
    cfg.differential_mode = false;
    switch (aiio_adc_cfg->resolution)
    {
      case AIIO_ADC_RESOLUTION_12:
        cfg.resolution=ADC_RESOLUTION_12B;
        break;
      case AIIO_ADC_RESOLUTION_14:
        cfg.resolution=ADC_RESOLUTION_14B;
        break;
      case AIIO_ADC_RESOLUTION_16:
        cfg.resolution=ADC_RESOLUTION_16B;
        break;      
      default:
        aiio_log_w("non support this resolution");
        break;
    }
    switch (aiio_adc_cfg->vref)
    {
      case AIIO_ADC_VREF_3_2_V:
        cfg.vref=ADC_VREF_3P2V;
        break;
      case AIIO_ADC_VREF_2_0_V:
        cfg.vref=ADC_VREF_2P0V;
        break;
      default:
        aiio_log_w("non support this vref");
        break;
    }

    bflb_adc_init(adc, &cfg);
    bflb_adc_channel_config(adc, chan, 1);
    free(chan);
    return AIIO_OK;
}

int32_t aiio_adc_get_value(aiio_adc_change_t change)
{
    uint32_t *raw_data;
    struct bflb_adc_result_s *result;
    uint32_t Voltage;
    raw_data=malloc(sizeof(uint32_t));
    result=malloc(sizeof(struct bflb_adc_result_s));
    bflb_adc_start_conversion(adc);
    while (bflb_adc_get_count(adc) < 1) {
        bflb_mtimer_delay_ms(1);
    }
    *raw_data = bflb_adc_read_raw(adc);
    bflb_adc_parse_result(adc, raw_data, result, 1);
    bflb_adc_stop_conversion(adc);
    Voltage=result->millivolt;
    free(raw_data);
    free(result);
    return Voltage;
}
