/**
 * @file     hal_adc.h
 * @author   BSP Team 
 * @brief    This file contains all of AES functions prototypes.
 * @version  0.0.0.1
 * @date     2021-06-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */



#ifndef __HAL_ADC_H
#define __HAL_ADC_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_adc.h"

#define IS_ADC_ALL_PERIPH(PERIPH) (((PERIPH) == ADC_BASE))


typedef enum
{
    /* The ADC_CH0 will be used by WiFi CAL !!!*/
    ADC_AWD_CH0     = 0,     
    
    ADC_AWD_CH1     = 1,     
    ADC_AWD_CH2     = 2,     
    ADC_AWD_CH3     = 3,     
    ADC_AWD_CH4     = 4,     
    ADC_AWD_CH5     = 5,     
    ADC_AWD_CH6     = 6,     
    ADC_AWD_CH7     = 7,     
      
}adc_awd_ch_t;

#define IS_AWD_ADC_CH(CH)  (((CH) == ADC_AWD_CH0)  || ((CH) == ADC_AWD_CH1)  || \
                            ((CH) == ADC_AWD_CH2)  || ((CH) == ADC_AWD_CH3)  || \
                            ((CH) == ADC_AWD_CH4)  || ((CH) == ADC_AWD_CH5)  || \
                            ((CH) == ADC_AWD_CH6)  || ((CH) == ADC_AWD_CH7) ) 


typedef enum
{
    ADC_AWD_ALL_CH      = 0,     // 0: Analog watchdog enabled on all channels; 
    ADC_AWD_SGL_CH      = 1,     // 1: Analog watchdog enabled on a single channel   
}adc_awd_sgl_t;                  // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

#define IS_ADC_AWD_CH_SEL(CH)  (((CH) == ADC_AWD_ALL_CH)  || ((CH) == ADC_AWD_SGL_CH)) 


typedef enum
{
    ADC_AUTO_OFF_MODE_DIS      = 0,   
    ADC_AUTO_OFF_MODE_EN       = 1,     
}adc_auto_off_mode_t;

#define IS_ADC_AUTO_OFF_MODE(MODE)  (((MODE) == ADC_AUTO_OFF_MODE_DIS)  || ((MODE) == ADC_AUTO_OFF_MODE_EN)) 


typedef enum
{
    ADC_WAIT_CONV_MODE_DIS      = 0,        // 0: Wait conversion mode off;
    ADC_WAIT_CONV_MODE_EN       = 1,        // 1: Wait conversion mode on.
}adc_wait_conv_mode_en_t;                   // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

#define IS_ADC_WAIT_CONV_MODE_EN_STATUS(STATUS)  (((STATUS) == ADC_WAIT_CONV_MODE_DIS)  || ((STATUS) == ADC_WAIT_CONV_MODE_EN)) 


typedef enum
{
    ADC_CONV_MODE_SINGLE         = 0,   
    ADC_CONV_MODE_CONTINUE       = 1,     
}adc_conv_mode_t;

#define IS_ADC_CONV_MODE(MODE)  (((MODE) == ADC_CONV_MODE_SINGLE)  || ((MODE) == ADC_CONV_MODE_CONTINUE)) 


typedef enum
{
    ADC_DATA_ALIGN_RIGHT        = 0,   
    ADC_DATA_ALIGN_LEFT         = 1,     
}adc_data_align_mode_t;         // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

#define IS_ADC_DATA_ALIGN_MODE(MODE)  (((MODE) == ADC_DATA_ALIGN_RIGHT)  || ((MODE) == ADC_DATA_ALIGN_LEFT)) 


typedef enum
{
    ADC_SPEC_EXT_TRIG_DIS      = 0,         //TODO
    ADC_SPEC_EXT_TRIG_EN       = 1,         //TODO
}adc_spec_ext_trig_en_t;

#define IS_ADC_SPEC_EXT_TRIG_EN_STATUS(STATUS)  (((STATUS) == ADC_SPEC_EXT_TRIG_DIS)  || ((STATUS) == ADC_SPEC_EXT_TRIG_EN)) 


typedef enum
{
    ADC_SPEC_EXT_TRIG_0      = 0,        
    ADC_SPEC_EXT_TRIG_1      = 1,        
    ADC_SPEC_EXT_TRIG_2      = 2,        
    ADC_SPEC_EXT_TRIG_3      = 3,          
}adc_spec_ext_trig_event_sel_t;

#define IS_ADC_SPEC_EXT_TRIG_EVENT(EVENT)  (((EVENT) == ADC_SPEC_EXT_TRIG_0)  || ((EVENT) == ADC_SPEC_EXT_TRIG_1) || \
                                            ((EVENT) == ADC_SPEC_EXT_TRIG_2)  || ((EVENT) == ADC_SPEC_EXT_TRIG_3) ) 



typedef enum
{
    ADC_EXT_TRIG_DIS                        = 0,        
    ADC_EXT_TRIG_RISING                     = 1,     
    ADC_EXT_TRIG_FALLING                    = 2,     
    ADC_EXT_TRIG_RISING_AND_FALLING         = 3,     
}adc_ext_trig_en_t;

#define IS_ADC_EXT_TRIG_EN_STATUS(STATUS)  (((STATUS) == ADC_EXT_TRIG_DIS)      || ((STATUS) == ADC_EXT_TRIG_RISING)             || \
                                            ((STATUS) == ADC_EXT_TRIG_FALLING)  || ((STATUS) == ADC_EXT_TRIG_RISING_AND_FALLING)) 


typedef enum
{
    ADC_EXT_TRIG_0      = 0,        
    ADC_EXT_TRIG_1      = 1,        
    ADC_EXT_TRIG_2      = 2,        
    ADC_EXT_TRIG_3      = 3,          
}adc_ext_trig_sel_t;

#define IS_ADC_EXT_TRIG_EVENT(EVENT)  (((EVENT) == ADC_EXT_TRIG_0)  || ((EVENT) == ADC_EXT_TRIG_1) || \
                                       ((EVENT) == ADC_EXT_TRIG_2)  || ((EVENT) == ADC_EXT_TRIG_3) ) 

#define IS_ADC_STAB_TIME(TIME)        ((TIME) < 0xFF)


typedef enum
{
    ADC_OVER_SAMPLING_RATIO_X2       = 0,        
    ADC_OVER_SAMPLING_RATIO_X4       = 1,        
    ADC_OVER_SAMPLING_RATIO_X5       = 2,        
    ADC_OVER_SAMPLING_RATIO_X16      = 3,        
    ADC_OVER_SAMPLING_RATIO_X32      = 4,        
    ADC_OVER_SAMPLING_RATIO_X64      = 5,        
}adc_ov_smp_ratio_t;                 // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

#define IS_ADC_OVER_SAMPLING_RATIO(RATIO)   ((RATIO) < ADC_OVER_SAMPLING_RATIO_X64) 


typedef enum
{
    ADC_OVER_SAMPLING_EN_STATUS_BIT0        = 1 << 0,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT1        = 1 << 1,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT2        = 1 << 2,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT3        = 1 << 3,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT4        = 1 << 4,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT5        = 1 << 5,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT6        = 1 << 6,       
    ADC_OVER_SAMPLING_EN_STATUS_BIT7        = 1 << 7,               
}adc_ov_smp_ratio_en_t;              // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

#define ADC_OVER_SAMPLING_EN_BIT_NUM            8
#define IS_ADC_OVER_SAMPLING_EN_STATUS(STATUS)  (((((uint32_t)STATUS) & ~0xFF) == 0x00U))


#define IS_ADC_HIGH_THOD(THOD)        ((THOD) <= 0xFFF)       // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).
#define IS_ADC_LOW_THOD(THOD)         ((THOD) <= 0xFFF)       // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).


typedef enum
{
    ADC_SPE_CH0     = 0,     
    ADC_SPE_CH1     = 1,     
    ADC_SPE_CH2     = 2,     
    ADC_SPE_CH3     = 3,     
    ADC_SPE_CH4     = 4,     
    ADC_SPE_CH5     = 5,     
    ADC_SPE_CH6     = 6,     
    ADC_SPE_CH7     = 7,     
      
}adc_spe_ch_t;

#define IS_ADC_SPE_CH(CH)  (((CH) == ADC_SPE_CH0)  || ((CH) == ADC_SPE_CH1)  || \
                            ((CH) == ADC_SPE_CH2)  || ((CH) == ADC_SPE_CH3)  || \
                            ((CH) == ADC_SPE_CH4)  || ((CH) == ADC_SPE_CH5)  || \
                            ((CH) == ADC_SPE_CH6)  || ((CH) == ADC_SPE_CH7)) 


typedef enum
{
    ADC_CH0     = 1 << 0,     //internal temperature resistance
    ADC_CH1     = 1 << 1,     
    ADC_CH2     = 1 << 2,     
    ADC_CH3     = 1 << 3,     
    ADC_CH4     = 1 << 4,     
    ADC_CH5     = 1 << 5,     
    ADC_CH6     = 1 << 6,     
    ADC_CH7     = 1 << 7,     
      
}adc_ch_t;      // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).


#define ADC_CH_NUM               8
#define IS_ADC_CH(CH)          (((CH) == ADC_CH0)  || ((CH) == ADC_CH1)  || \
                                ((CH) == ADC_CH2)  || ((CH) == ADC_CH3)  || \
                                ((CH) == ADC_CH4)  || ((CH) == ADC_CH5)  || \
                                ((CH) == ADC_CH6)  || ((CH) == ADC_CH7)) 


#define IS_ADC_PRESCALE_VALUE(VALUE)     (((VALUE) >= 1 && (VALUE) <= 0xFF))  


typedef enum
{
    ADC_VREF_0_8_V   = 0,       
    ADC_VREF_0_8_5_V = 1,       
    ADC_VREF_0_9_5_V = 2,       
    ADC_VREF_1_0_5_V = 3,       
}adc_vref_set_t;

#define IS_ADC_VREF_VALUE(VALUE)     ((VALUE) == ADC_VREF_0_8_V   || ADC_VREF_0_8_5_V   ||  \
                                      (VALUE) == ADC_VREF_0_9_5_V || ADC_VREF_1_0_5_V   )  


#define IS_ADC_CH_INGAIN_VALUE(VALUE)   ((VALUE) == ADC_ISOLATE_SIGNAL   || ADC_BYPASS_THE_INPUT_DRIVER   ||  \
                                         (VALUE) == ADC_SIGNAL_DIV_4     || ADC_SIGNAL_DIV_2                     )

typedef enum
{
    ADC_EOC_FLAG_7       = 0x0080,     // [7] End of conversion flag. 
    ADC_EOC_FLAG_6       = 0x0040,     // [6] End of conversion flag. 
    ADC_EOC_FLAG_5       = 0x0020,     // [5] End of conversion flag. 
    ADC_EOC_FLAG_4       = 0x0010,     // [4] End of conversion flag. 
    ADC_EOC_FLAG_3       = 0x0008,     // [3] End of conversion flag. 
    ADC_EOC_FLAG_2       = 0x0004,     // [2] End of conversion flag. 
    ADC_EOC_FLAG_1       = 0x0002,     // [1] End of conversion flag. 
    ADC_EOC_FLAG_0       = 0x0001,     // [0] End of conversion flag. 
}adc_eoc_flag_t;

#define IS_ADC_EOC_FLAG_VALUE(VALUE)  (((VALUE) <= ADC_EOC_FLAG_7)) 


typedef enum
{
    ADC_IT_FLAG_AWD         = 0x0400,     // [10] Analog watchdog flag. 
    ADC_IT_FLAG_OVR         = 0x0200,     // [9] ADC overrun.     
    ADC_IT_FLAG_EOS         = 0x0100,     // [8] End of sequence flag.   
    ADC_IT_FLAG_EOC_7       = 0x0080,     // [7] End of conversion flag. 
    ADC_IT_FLAG_EOC_6       = 0x0040,     // [6] End of conversion flag. 
    ADC_IT_FLAG_EOC_5       = 0x0020,     // [5] End of conversion flag. 
    ADC_IT_FLAG_EOC_4       = 0x0010,     // [4] End of conversion flag. 
    ADC_IT_FLAG_EOC_3       = 0x0008,     // [3] End of conversion flag. 
    ADC_IT_FLAG_EOC_2       = 0x0004,     // [2] End of conversion flag. 
    ADC_IT_FLAG_EOC_1       = 0x0002,     // [1] End of conversion flag. 
    ADC_IT_FLAG_EOC_0       = 0x0001,     // [0] End of conversion flag. 
}adc_it_flag_t;

#define IS_ADC_IT_FLAG_VALUE(VALUE)  (((VALUE) <= ADC_IT_FLAG_AWD)) 


typedef struct
{
    adc_ch_t                        adc_awd_ch;                     /*!< Specifies the adc watchdog channels. 
                                                                    This parameter can be a value of @adc_ch_t */              

    adc_awd_sgl_t                   adc_awd_sgl;                    /*!< Specifies the adc watchdog enable on a single channel or on all channels. 
                                                                    This parameter can be a value of @adc_awd_sgl_t */              

    adc_auto_off_mode_t             adc_auto_off_mode;              /*!< Specifies the adc auto-off mode.
                                                                    This parameter can be a value of @adc_auto_off_mode_t */     

    adc_wait_conv_mode_en_t         adc_wait_conv_mode_en;          /*!< Specifies the adc wait conversion mode. 
                                                                    This parameter can be a value of @adc_wait_conv_mode_en_t */    

    adc_conv_mode_t                 adc_conv_mode;                  /*!< Specifies the adc single / continuous conversion mode. 
                                                                    This parameter can be a value of @adc_conv_mode_t */ 

    adc_data_align_mode_t           adc_data_align_mode;            /*!< Specifies the adc data alignment. 
                                                                    This parameter can be a value of @adc_data_align_mode_t */ 
                                                                    // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

    adc_spec_ext_trig_en_t          adc_spec_ext_trig_en;           /*!< Specifies the adc external trigger enable status and polarity for special channel.
                                                                    This parameter can be a value of @adc_spec_ext_trig_en_t */ 

    adc_spec_ext_trig_event_sel_t   adc_spec_ext_trig_event_sel;    /*!< Specifies the adc external trigger for special channel.
                                                                    This parameter can be a value of @adc_spec_ext_trig_event_sel_t */ 

    adc_ext_trig_en_t               adc_ext_trig_en;                /*!< Specifies the adc external trigger enable and polarity. 
                                                                    This parameter can be a value of @adc_ext_trig_en_t */ 

    adc_ext_trig_sel_t              adc_ext_trig_sel;               /*!< Specifies the adc external trigger.
                                                                    This parameter can be a value of @adc_ext_trig_sel_t */ 

    uint8_t                         adc_stab_time;                  /*!< Specifies the adc stable time.
                                                                    The range of parameters can be referred to @IS_ADC_STAB_TIME */ 

    adc_ov_smp_ratio_t              adc_ov_smp_ratio;               /*!< Specifies the adc oversampling ratio. 
                                                                    This parameter can be a value of @adc_ov_smp_ratio_t */ 

    adc_ov_smp_ratio_en_t           adc_ov_smp_ratio_en;            /*!< Specifies the adc oversampling enable status.
                                                                    This parameter can be a value of @adc_ov_smp_ratio_en_t */ 

    uint16_t                        adc_ht;                         /*!< Specifies the adc analog watchdog higher threshold.
                                                                    The range of parameters can be referred to @IS_ADC_HIGH_THOD */ 
                                                                    // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

    uint16_t                        adc_lt;                         /*!< Specifies the adc analog watchdog lower threshold. 
                                                                    The range of parameters can be referred to @IS_ADC_LOW_THOD */ 
                                                                    // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

    adc_spe_ch_t                    adc_spe_ch;                     /*!< Specifies the adc special channel. 
                                                                    This parameter can be a value of @adc_spe_ch_t */ 
                                                                    // Note: Software is allowed to write these bits only when ADSTART=0 (which ensures that no conversion is ongoing).

    adc_ch_t                        adc_ch;                         /*!< Specifies the adc conversion channel.
                                                                    This parameter can be a value of @adc_ch_t */
                                                                   
    uint8_t                         adc_presc;                      /*!< Specifies the adc  prescaler.pre = (n+1)*2, there is a bug,when you need 2
                                                                    The range of parameters can be referred to @IS_ADC_PRESCALE_VALUE */ 

    adc_vref_set_t                  adc_vref_set;                   /*!< Specifies the adc  Vref.
                                                                    The range of parameters can be referred to @IS_ADC_VREF_VALUE */ 
   
}adc_init_t_def;


            //adc init and config
void        hal_adc_init(uint32_t adc_base,adc_init_t_def* adc_init);
void        hal_adc_deinit(void);
void        hal_adc_dma_en(uint32_t adc_base,hal_en_t en);
void        hal_adc_awd_en(uint32_t adc_base,hal_en_t en);
void        hal_adc_en(uint32_t adc_base,hal_en_t en);

void        hal_adc_start_conv(uint32_t adc_base);
void        hal_adc_stop_conv(uint32_t adc_base);
uint8_t     hal_adc_get_conv_status(uint32_t adc_base, adc_ch_t ch);
void        hal_adc_clr_conv_status(uint32_t adc_base, adc_ch_t ch);
void        hal_adc_spe_sw_start(uint32_t adc_base);
uint16_t    hal_adc_get_data(uint32_t adc_base,adc_ch_t ch);

            //adc it config
void        hal_adc_it_cfg(uint32_t adc_base,adc_it_flag_t adc_it_flag ,hal_en_t en);
uint8_t     hal_adc_get_it_flag(uint32_t adc_base,adc_it_flag_t adc_it_flag);
void        hal_adc_clr_it_flag(uint32_t adc_base,adc_it_flag_t adc_it_flag);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // __HAL_ADC_H
