/**
 * @file     ln_adc_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-09-10
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
/*
            ADC使用说明：

                1. 本测试使用PB3作为了测试引脚

*/



#include "hal_common.h"
#include "hal_adc.h"
#include "hal_gpio.h"
#include "hal_dma.h"
#include "utils/debug/log.h"

#include "ln_adc_test.h"
#include "ln_test_common.h"

static volatile int16_t adc_data[4096];
static volatile int16_t adc_data_buffer;
static volatile uint32_t cnt = 0;

void ln_adc_test(void)
{
    
    adc_init_t_def adc_init;
    
    memset(&adc_init,0,sizeof(adc_init));
    
    adc_init.adc_ch = ADC_CH5;                              //配置通道
    adc_init.adc_conv_mode = ADC_CONV_MODE_CONTINUE;        //配置ADC为连续转换模式
    adc_init.adc_presc = 40;                                //设置采样周期为1M
    hal_adc_init(ADC_BASE,&adc_init);                       //初始化ADC
    
    hal_gpio_pin_mode_set(GPIOB_BASE,GPIO_PIN_3,GPIO_MODE_ANALOG);      //配置PB3为模拟引脚
//    hal_adc_it_cfg(ADC_BASE,ADC_IT_FLAG_EOC_5,HAL_ENABLE);            
    
//    
//    /* init the adc rx dma */
//    dma_init_t_def dma_init;
//    memset(&dma_init,0,sizeof(dma_init));
//    
//    dma_init.dma_mem_addr = (uint32_t)adc_data;        //设置DMA内存地址
//    dma_init.dma_data_num = 4096;                      //设置DMA传输次数
//    dma_init.dma_dir = DMA_READ_FORM_P;                //设置DMA方向
//    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;          //设置DMA内存是否增长
//    dma_init.dma_p_addr = ADC_DATA_REG_5;              //设置DMA外设地址
//    dma_init.dma_mem_size = DMA_MEM_SIZE_16_BIT;
//    dma_init.dma_p_size = DMA_P_SIZE_16_BIT;
//    hal_dma_init(DMA_CH_7,&dma_init);                  //初始化DMA
//   
//    hal_adc_dma_en(ADC_BASE,HAL_ENABLE);
    hal_adc_en(ADC_BASE,HAL_ENABLE);
    hal_adc_start_conv(ADC_BASE);
//                                 
//    
//    hal_dma_en(DMA_CH_7,HAL_ENABLE);     
    while(1)
    {
        if(hal_adc_get_conv_status(ADC_BASE,ADC_CH5) == HAL_SET)
        {
            adc_data_buffer = hal_adc_get_data(ADC_BASE,ADC_CH5);  
            LOG(LOG_LVL_INFO,"adc = %d \r\n",adc_data_buffer);
        }
//        while(hal_dma_get_data_num(DMA_CH_7) != 0){};
//        if(hal_adc_get_it_flag(ADC_BASE,ADC_IT_FLAG_EOC_5))
//        {
//            adc_data_buffer = hal_adc_get_data(ADC_BASE,ADC_CH5);     
//////            hal_adc_clr_it_flag(ADC_BASE,ADC_IT_FLAG_EOC_5);
//            LOG(LOG_LVL_INFO,"%d \r\n",adc_data_buffer);

////            adc_data[cnt++] = hal_adc_get_data(ADC_BASE,ADC_CH5); 
////            hal_adc_clr_it_flag(ADC_BASE,ADC_IT_FLAG_EOC_5);
//        }
//        if(cnt >= 4096)
//        {
//            for(int i = 0; i < 4096; i++)
//            {
//                LOG(LOG_LVL_INFO,"%d \r\n",adc_data[i]);
//            }
//            while(1);
//        }

        
        ln_delay_ms(100);
    }
    
}
