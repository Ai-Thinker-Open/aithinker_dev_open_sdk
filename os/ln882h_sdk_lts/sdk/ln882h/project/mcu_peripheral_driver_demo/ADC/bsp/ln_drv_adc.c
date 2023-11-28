/**
 * @file     ln_drv_adc.c
 * @author   BSP Team 
 * @brief    ADC驱动例程
 * @version  0.0.0.1
 * @date     2021-11-02
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_adc.h"

/*
    引脚和ADC通道对应关系

    GPIOA0  ->  ADC2
    GPIOA1  ->  ADC3
    GPIOA4  ->  ADC4
    GPIOB3  ->  ADC5

    GPIOB4  ->  ADC6
    GPIOB5  ->  ADC7
*/

/**
 * @brief 初始化ADC
 * 
 * @param adc_ch 选择ADC通道
 */
void adc_init(adc_ch_t adc_ch)
{
    adc_init_t_def adc_init;
    memset(&adc_init,0,sizeof(adc_init));
    
    adc_init.adc_ch = adc_ch;                               //配置通道
    adc_init.adc_conv_mode = ADC_CONV_MODE_CONTINUE;        //配置ADC为连续转换模式
    adc_init.adc_presc = 80;                                //设置采样周期为1M
    hal_adc_init(ADC_BASE,&adc_init);                       //初始化ADC

    //将对应引脚设置为模拟模式
    switch(adc_ch)
    {
        case ADC_CH2:  hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_0,GPIO_MODE_ANALOG);  break;
        case ADC_CH3:  hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_1,GPIO_MODE_ANALOG);  break;

        case ADC_CH4:  hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_4,GPIO_MODE_ANALOG);  break;
        case ADC_CH5:  hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_3,GPIO_MODE_ANALOG);  break;
        case ADC_CH6:  hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_4,GPIO_MODE_ANALOG);  break;
        case ADC_CH7:  hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_5,GPIO_MODE_ANALOG);  break;
        default:
            break;
    }
}

/**
 * @brief ADC开始转换
 * 
 * @param adc_ch 选择ADC通道
 */
void adc_start(void)
{
    hal_adc_en(ADC_BASE,HAL_ENABLE);
    hal_adc_start_conv(ADC_BASE);
}


/**
 * @brief 获取ADC数据,如果一直没有获取到ADC数据，则会阻塞在这个函数中，此处应注意！
 * 
 * @param ch 选择ADC通道
 * @return uint16_t 返回ADC数据
 */
uint16_t adc_get_data(adc_ch_t adc_ch)
{
    uint16_t ret_data = 0;

    while(hal_adc_get_conv_status(ADC_BASE,adc_ch) != HAL_SET);

    ret_data = hal_adc_get_data(ADC_BASE,adc_ch);
    
    return ret_data;
}
