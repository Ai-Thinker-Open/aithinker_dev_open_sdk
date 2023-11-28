/**
 * @file     ln_drv_adc_dma.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_adc_dma.h"

/*
    引脚和ADC通道对应关系

    GPIOA0  ->  ADC2
    GPIOA1  ->  ADC3
    GPIOA4  ->  ADC4
    GPIOB3  ->  ADC5

    GPIOB4  ->  ADC6
    GPIOB5  ->  ADC7
*/

void (*dma_complete_it_handler)(void);

/**
 * @brief 
 * 
 * @param adc_ch   配置ADC通道
 * @param data_arr 配置DMA传输的目的地址（存放数据的内存地址）
 * @param data_num 配置DMA要传输的数据大小
 * @param dma_complete_it_callback DMA传输完成的回调函数
 */
void adc_dma_init(adc_ch_t adc_ch,uint32_t *data_arr,uint16_t data_num,void (*dma_complete_it_callback)(void) )
{
    adc_init_t_def adc_init;
    memset(&adc_init,0,sizeof(adc_init));
    
    adc_init.adc_ch = adc_ch;                               //配置通道
    adc_init.adc_conv_mode = ADC_CONV_MODE_CONTINUE;        //配置ADC为连续转换模式
    adc_init.adc_presc = 80;                                //设置采样周期为1M
    hal_adc_init(ADC_BASE,&adc_init);                       //初始化ADC

    /* init the adc rx dma */
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));

    //将对应引脚设置为模拟模式
    switch(adc_ch)
    {
        case ADC_CH2:  
            hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_0,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_2,HAL_ENABLE);       
            dma_init.dma_p_addr = ADC_DATA_REG_2;           
            break;
        case ADC_CH3:  
            hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_1,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_3,HAL_ENABLE);     
            dma_init.dma_p_addr = ADC_DATA_REG_3;    
            break;
        case ADC_CH4:  
            hal_gpio_pin_mode_set(GPIOA_BASE,GPIO_PIN_4,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_4,HAL_ENABLE); 
            dma_init.dma_p_addr = ADC_DATA_REG_4;      
            break;
        case ADC_CH5:  
            hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_3,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_5,HAL_ENABLE);   
            dma_init.dma_p_addr = ADC_DATA_REG_5;    
            break;
        case ADC_CH6:  
            hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_4,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_6,HAL_ENABLE);   
            dma_init.dma_p_addr = ADC_DATA_REG_6;    
            break;
        case ADC_CH7:  
            hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_5,GPIO_MODE_ANALOG);  
            hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_7,HAL_ENABLE);  
            dma_init.dma_p_addr = ADC_DATA_REG_7;     
            break;
        default:
            break;
    }

    dma_init.dma_mem_addr = (uint32_t)data_arr;        //设置DMA内存地址
    dma_init.dma_data_num = data_num;                  //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_P;                //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;          //设置DMA内存是否增长
    dma_init.dma_mem_size = DMA_MEM_SIZE_16_BIT;       //配置内存地址位宽
    dma_init.dma_p_size = DMA_P_SIZE_16_BIT;           //配置外设地址位宽
    hal_dma_init(DMA_CH_7,&dma_init);                  //初始化DMA
    
    hal_dma_it_cfg(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);
    NVIC_EnableIRQ(DMA_IRQn);

    if(dma_complete_it_callback != NULL)
        dma_complete_it_handler = dma_complete_it_callback;
}

/**
 * @brief 启动ADC 和 DMA
 * 
 */
void adc_dma_start(void)
{
    hal_adc_en(ADC_BASE,HAL_ENABLE);
    hal_adc_dma_en(ADC_BASE,HAL_ENABLE);
    hal_adc_start_conv(ADC_BASE);
    hal_dma_en(DMA_CH_7,HAL_ENABLE);
}




/**
 * @brief  DMA中断服务函数
 * 
 */
void DMA_IRQHandler()
{
    if(hal_dma_get_it_flag(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)
    {
        hal_dma_clr_it_flag(DMA_CH_7,DMA_IT_FLAG_TRAN_COMP);
        if(dma_complete_it_handler != NULL)
            dma_complete_it_handler();
    }
}
