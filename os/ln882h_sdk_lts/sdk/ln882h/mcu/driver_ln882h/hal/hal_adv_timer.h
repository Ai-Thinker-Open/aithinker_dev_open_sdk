/**
 * @file     hal_adv_timer.h
 * @author   BSP Team 
 * @brief    This file contains all of ADV-TIMER functions prototypes.
 * @version  0.0.0.1
 * @date     2020-12-14
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_ADV_TIMER_H
#define __HAL_ADV_TIMER_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "reg_adv_timer.h"


#define ADV_TIMER_IT_BASE  (PWM_BASE + 0x20 * 0)
#define ADV_TIMER_0_BASE   (PWM_BASE + 0x20 * 1)
#define ADV_TIMER_1_BASE   (PWM_BASE + 0x20 * 2)
#define ADV_TIMER_2_BASE   (PWM_BASE + 0x20 * 3)
#define ADV_TIMER_3_BASE   (PWM_BASE + 0x20 * 4)
#define ADV_TIMER_4_BASE   (PWM_BASE + 0x20 * 5)
#define ADV_TIMER_5_BASE   (PWM_BASE + 0x20 * 6)
#define IS_ADV_TIMER_IT_BASE(BASE)      ((BASE  == ADV_TIMER_IT))

#define IS_ADV_TIMER_ALL_PERIPH(PERIPH) (((PERIPH) == ADV_TIMER_0_BASE)  || ((PERIPH) == ADV_TIMER_1_BASE)   ||  \
                                         ((PERIPH) == ADV_TIMER_2_BASE)  || ((PERIPH) == ADV_TIMER_3_BASE)   ||  \
                                         ((PERIPH) == ADV_TIMER_4_BASE)  || ((PERIPH) == ADV_TIMER_5_BASE))     


typedef enum
{
    ADV_TIMER_CAP_DIS       = 0,     
    ADV_TIMER_CAP_EN        = 1,         /* It will be diabled by itself when "cap_mod=0" */
}adv_tim_cap_en_t;

#define IS_TIMER_CAP_EN(EN)   (((EN) == ADV_TIMER_CAP_DIS) || ((EN) == ADV_TIMER_CAP_EN))


typedef enum
{
    ADV_TIMER_TIMER_DIS     = 0,     
    ADV_TIMER_TIMER_EN      = 1,        
}adv_tim_tim_en_t;

#define IS_TIMER_TIMER_EN(EN) (((EN) == ADV_TIMER_TIMER_DIS) || ((EN) == ADV_TIMER_TIMER_EN))


typedef enum
{
    ADV_TIMER_DEAD_DIS      = 0,     
    ADV_TIMER_DEAD_EN       = 1,        
}adv_tim_dead_en_t;

#define IS_TIMER_DEAD_EN(EN)  (((EN) == ADV_TIMER_DEAD_DIS) || ((EN) == ADV_TIMER_DEAD_EN))


typedef enum
{
    ADV_TIMER_CHA_DIS       = 0,     
    ADV_TIMER_CHA_EN        = 1,        
}adv_tim_cha_en_t;

#define IS_TIMER_CHA_EN(EN)  (((EN) == ADV_TIMER_CHA_DIS) || ((EN) == ADV_TIMER_CHA_EN))


typedef enum
{
    ADV_TIMER_CHB_DIS       = 0,     
    ADV_TIMER_CHB_EN        = 1,        
}adv_tim_chb_en_t;

#define IS_TIMER_CHB_EN(EN)  (((EN) == ADV_TIMER_CHB_DIS) || ((EN) == ADV_TIMER_CHB_EN))

typedef enum
{
    ADV_TIMER_CHB_IT_MODE_DIS       = 0,     
    ADV_TIMER_CHB_IT_MODE_INC       = 1,     /* Equal channel b at increase counter */ 
    ADV_TIMER_CHB_IT_MODE_DEC       = 2,     /* Equal channel b at decrease counter */ 
    ADV_TIMER_CHB_IT_MODE_BOTH      = 3,     /* Equal channel b both at increase and decrease */ 
}adv_tim_chb_it_mode_t;

#define IS_TIMER_CHB_IT_MODE(MODE)  (((MODE) == ADV_TIMER_CHB_IT_MODE_DIS) || ((MODE) == ADV_TIMER_CHB_IT_MODE_INC)  ||  \
                                     ((MODE) == ADV_TIMER_CHB_IT_MODE_DEC) || ((MODE) == ADV_TIMER_CHB_IT_MODE_BOTH)  )

typedef enum
{
    ADV_TIMER_CHA_IT_MODE_DIS       = 0,     
    ADV_TIMER_CHA_IT_MODE_INC       = 1,     /* Equal channel a at increase counter */ 
    ADV_TIMER_CHA_IT_MODE_DEC       = 2,     /* Equal channel a at decrease counter */ 
    ADV_TIMER_CHA_IT_MODE_BOTH      = 3,     /* Equal channel a both at increase and decrease */ 
}adv_tim_cha_it_mode_t;

#define IS_TIMER_CHA_IT_MODE(MODE)  (((MODE) == ADV_TIMER_CHA_IT_MODE_DIS) || ((MODE) == ADV_TIMER_CHA_IT_MODE_INC)  ||  \
                                     ((MODE) == ADV_TIMER_CHA_IT_MODE_DEC) || ((MODE) == ADV_TIMER_CHA_IT_MODE_BOTH) )

typedef enum
{
    ADV_TIMER_ONCE_DIS       = 0,     
    ADV_TIMER_ONCE_EN        = 1,    /* The timer runs only once */
}adv_tim_once_en_t;

#define IS_TIMER_ONCE_EN(EN)  (((EN) == ADV_TIMER_ONCE_DIS) || ((EN) == ADV_TIMER_ONCE_EN))

typedef enum
{
    ADV_TIMER_CAP_MODE_1      = 0,   /* Counter input edge from pin untill to load, then output interrupt.                */  
    ADV_TIMER_CAP_MODE_2      = 1,   /* Save count value to "pwm_trig" when input edge from pin,and generation interrupt. */    
}adv_tim_cap_mode_t;

#define IS_TIMER_CAP_MODE(MODE)  (((MODE) == ADV_TIMER_CAP_MODE_1) || ((MODE) == ADV_TIMER_CAP_MODE_2))

typedef enum
{
    ADV_TIMER_EDG_DIS         = 0,     
    ADV_TIMER_EDG_RISE        = 1,        
    ADV_TIMER_EDG_FALL        = 2,        
    ADV_TIMER_EDG_BOTH        = 3,        
}adv_tim_cap_edg_t;

#define IS_TIMER_CAP_EDG(MODE)      (((MODE) == ADV_TIMER_EDG_DIS)  || ((MODE) == ADV_TIMER_EDG_RISE)  ||  \
                                     ((MODE) == ADV_TIMER_EDG_FALL) || ((MODE) == ADV_TIMER_EDG_BOTH)  )

typedef enum
{
    ADV_TIMER_TRG_CLR_DIS     = 0,     
    ADV_TIMER_TRG_CLR_EN      = 1,        
}adv_tim_trg_clr_en_t;

#define IS_TIMER_TRG_CLR_EN(EN)    (((EN) == ADV_TIMER_TRG_CLR_DIS) || ((EN) == ADV_TIMER_TRG_CLR_EN))


typedef enum
{
    ADV_TIMER_TRG_SEL_TIM_0         = 0,     
    ADV_TIMER_TRG_SEL_TIM_1         = 1,        
    ADV_TIMER_TRG_SEL_TIM_2         = 2,        
    ADV_TIMER_TRG_SEL_TIM_3         = 3,        
    ADV_TIMER_TRG_SEL_TIM_4         = 4,        
    ADV_TIMER_TRG_SEL_TIM_5         = 5,               
}adv_tim_trg_sel_t;                          /* When trig_clr_en=1, trig_sel active           */

#define IS_TIMER_TRG_SEL(SEL)    (((SEL) == ADV_TIMER_TRG_SEL_TIM_0) || ((SEL) == ADV_TIMER_TRG_SEL_TIM_1)  || \
                                    ((SEL) == ADV_TIMER_TRG_SEL_TIM_2) || ((SEL) == ADV_TIMER_TRG_SEL_TIM_3)  || \
                                    ((SEL) == ADV_TIMER_TRG_SEL_TIM_4) || ((SEL) == ADV_TIMER_TRG_SEL_TIM_5))


typedef enum
{
    ADV_TIMER_TRG_MODE_DIS          = 0,     /* 0: disable                                    */ 
    ADV_TIMER_TRG_MODE_INC          = 1,     /* 1: count eq "pwm_trig" when count up          */     
    ADV_TIMER_TRG_MODE_DEC          = 2,     /* 2: count eq "pwm_trig" when count down        */   
    ADV_TIMER_TRG_MODE_BOTH         = 3,     /* 3: count eq "pwm_trig" both count up or down. */
}adv_tim_trg_mode_t; 

#define IS_TIMER_TRG_MODE(MODE)      (((MODE) == ADV_TIMER_TRG_MODE_DIS)  || ((MODE) == ADV_TIMER_TRG_MODE_INC)   ||  \
                                      ((MODE) == ADV_TIMER_TRG_MODE_DEC)  || ((MODE) == ADV_TIMER_TRG_MODE_BOTH) )

typedef enum
{
    ADV_TIMER_CNT_MODE_INC          = 0,
    ADV_TIMER_CNT_MODE_DEC          = 1,
    ADV_TIMER_CNT_MODE_BOTH         = 2,
}adv_tim_cnt_mode_t;

#define IS_TIMER_CNT_MODE(MODE)       (((MODE) == ADV_TIMER_CNT_MODE_INC) || ((MODE) == ADV_TIMER_CNT_MODE_DEC) || ((MODE) == ADV_TIMER_CNT_MODE_BOTH))


typedef enum
{
    ADV_TIMER_CHB_INV_DIS     = 0,     
    ADV_TIMER_CHB_INV_EN      = 1,        
}adv_tim_chb_inv_en_t;

#define IS_TIMER_CHB_INV_EN(EN)    (((EN) == ADV_TIMER_CHB_INV_DIS) || ((EN) == ADV_TIMER_CHB_INV_EN))

typedef enum
{
    ADV_TIMER_CHA_INV_DIS     = 0,     
    ADV_TIMER_CHA_INV_EN      = 1,        
}adv_tim_cha_inv_en_t;

#define IS_TIMER_CHA_INV_EN(EN)    (((EN) == ADV_TIMER_CHA_INV_DIS) || ((EN) == ADV_TIMER_CHA_INV_EN))



typedef enum
{
    //isrr
    ADV_TIMER_STATUS_FLAG_LOAD_RAW    = 0,        // [0] load flag
    ADV_TIMER_STATUS_FLAG_TRIG_RAW    = 1,        // [1] trigger flag
    ADV_TIMER_STATUS_FLAG_CMPA_RAW    = 2,        // [2] compare a flag
    ADV_TIMER_STATUS_FLAG_CMPB_RAW    = 3,        // [3] compare b flag 
}adv_tim_status_flag_t;

#define IS_ADV_TIM_STATUS_FLAG(FLAG)        (( FLAG >= 0) && ( FLAG >= 7 ))


typedef enum
{
    ADV_TIMER_IT_FLAG_LOAD        = 0,          // [0] load interrupt flag
    ADV_TIMER_IT_FLAG_TRIG        = 1,          // [1] trigger interrupt flag
    ADV_TIMER_IT_FLAG_CMPA        = 2,          // [2] compare interrupt a flag
    ADV_TIMER_IT_FLAG_CMPB        = 3,          // [3] compare interrupt b flag 

}adv_tim_it_flag_t;

#define IS_ADV_TIM_IT_FLAG(FLAG)        (( FLAG <= 3 ))



#define IS_TIMER_DEAD_GAP(GAP)         (((GAP)   <= 0x0FFF))
#define IS_TIMER_LOAD_VALUE(VALUE)     (((VALUE) <= 0xFFFF))
#define IS_TIMER_CMPA_VALUE(VALUE)     (((VALUE) <= 0xFFFF))
#define IS_TIMER_CMPB_VALUE(VALUE)     (((VALUE) <= 0xFFFF))
#define IS_TIMER_TRIG_VALUE(VALUE)     (((VALUE) <= 0xFFFF))
#define IS_TIMER_CLK_DIV(DIV)          (((DIV)   <= 0x003F))


typedef struct
{

    adv_tim_cap_en_t            adv_tim_cap_en;             /*!< Specifies the adv timer capture enable status.
                                                            This parameter can be a value of adv_tim_cap_en_t */

    adv_tim_tim_en_t            adv_tim_tim_en;             /*!< Specifies the adv timer timer enable status.
                                                            This parameter can be a value of adv_tim_tim_en_t */

    adv_tim_dead_en_t           adv_tim_dead_en;            /*!< Specifies the adv timer dead gap enable status.
                                                            This parameter can be a value of adv_tim_dead_en_t */

    adv_tim_cha_en_t            adv_tim_cha_en;             /*!< Specifies the adv timer channel A output enable status.
                                                            This parameter can be a value of adv_tim_cha_en_t */

    adv_tim_chb_en_t            adv_tim_chb_en;             /*!< Specifies the adv timer channel B output enable status.
                                                            This parameter can be a value of adv_tim_chb_en_t */

    adv_tim_chb_it_mode_t       adv_tim_chb_it_mode;        /*!< Specifies the adv timer channel B interrupt mode.
                                                            This parameter can be a value of adv_tim_chb_it_mode_t */

    adv_tim_cha_it_mode_t       adv_tim_cha_it_mode;        /*!< Specifies the adv timer channel A interrupt mode.
                                                            This parameter can be a value of adv_tim_cha_it_mode_t */

    adv_tim_once_en_t           adv_tim_once_en;            /*!< Specifies the adv timer run once enable status.
                                                            This parameter can be a value of adv_tim_once_en_t */ 

    adv_tim_cap_mode_t          adv_tim_cap_mode;           /*!< Specifies the adv timer capture mode.
                                                            This parameter can be a value of adv_tim_cap_mode_t */      

    adv_tim_cap_edg_t           adv_tim_cap_edg;            /*!< Specifies the adv timer capture edge.
                                                            This parameter can be a value of adv_tim_cap_edg_t */           

    adv_tim_trg_clr_en_t        adv_tim_trg_clr_en;         /*!< Specifies the adv timer selected trigger source and clear cnt.
                                                            This parameter can be a value of adv_tim_trg_clr_en_t */             

    adv_tim_trg_sel_t           adv_tim_trg_sel;            /*!< Specifies the adv timer trigger source select,when trig_clr_en=1, trig_sel active.
                                                            This parameter can be a value of adv_tim_trg_sel_t */  

    adv_tim_trg_mode_t          adv_tim_trg_mode;           /*!< Specifies the adv timer trigger mode.
                                                            This parameter can be a value of adv_tim_trg_mode_t */  

    adv_tim_cnt_mode_t          adv_tim_cnt_mode;           /*!< Specifies the adv timer counter mode.
                                                            This parameter can be a value of adv_tim_cnt_mode_t */  

    adv_tim_chb_inv_en_t        adv_tim_chb_inv_en;         /*!< Specifies the adv timer channel b output invert enable status.
                                                            This parameter can be a value of adv_tim_chb_inv_en_t */  

    adv_tim_cha_inv_en_t        adv_tim_cha_inv_en;         /*!< Specifies the adv timer channel a output invert enable status.
                                                            This parameter can be a value of adv_tim_cha_inv_en_t */

    uint16_t                    adv_tim_load_value;         /*!< Specifies the adv timer load value.
                                                            The parameter range can be referred to IS_TIMER_LOAD_VALUE*/

    uint16_t                    adv_tim_cmp_a_value;        /*!< Specifies the adv timer cmp a value.
                                                            The parameter range can be referred to IS_TIMER_CMPA_VALUE*/

    uint16_t                    adv_tim_cmp_b_value;        /*!< Specifies the adv timer cmp b value.
                                                            The parameter range can be referred to IS_TIMER_CMPB_VALUE*/

    uint8_t                     adv_tim_clk_div;            /*!< Specifies the adv timer clock division.
                                                            The parameter range can be referred to IS_TIMER_CLK_DIV*/

    uint16_t                    adv_tim_dead_gap_value;     /*!< Specifies the adv timer dead gap value.
                                                            The parameter range can be referred to IS_TIMER_DEAD_GAP*/

    uint16_t                    adv_tim_trig_value;         /*!< Specifies the adv timer trig value.
                                                            The parameter range can be referred to IS_TIMER_TRIG_VALUE*/
}adv_tim_init_t_def;


            //Advanced Timer init and config
void        hal_adv_tim_init(uint32_t adv_tim_x_base,adv_tim_init_t_def* adv_tim_init);             
void        hal_adv_tim_deinit(void);

void        hal_adv_tim_cap_en(uint32_t adv_tim_x_base,hal_en_t en);
void        hal_adv_tim_tim_en(uint32_t adv_tim_x_base,hal_en_t en);
void        hal_adv_tim_dead_gap_en(uint32_t adv_tim_x_base,hal_en_t en);
void        hal_adv_tim_a_en(uint32_t adv_tim_x_base,hal_en_t en);
void        hal_adv_tim_b_en(uint32_t adv_tim_x_base,hal_en_t en);

void        hal_adv_tim_set_dead_gap(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_load_value(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_inv_a(uint32_t adv_tim_x_base,uint8_t value);
void        hal_adv_tim_set_inv_b(uint32_t adv_tim_x_base,uint8_t value);
void        hal_adv_tim_set_comp_b(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_comp_a(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_trig_value(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_clock_div(uint32_t adv_tim_x_base,uint16_t value);
void        hal_adv_tim_set_cap_edge(uint32_t adv_tim_x_base,adv_tim_cap_edg_t adv_tim_cap_edg);

            //get value
uint16_t    hal_adv_tim_get_trig_value(uint32_t adv_tim_x_base);
uint16_t    hal_adv_tim_get_load_value(uint32_t adv_tim_x_base);
uint16_t    hal_adv_tim_get_count(uint32_t adv_tim_x_base);
uint16_t    hal_adv_tim_get_comp_b(uint32_t adv_tim_x_base);
uint16_t    hal_adv_tim_get_comp_a(uint32_t adv_tim_x_base);
uint8_t     hal_adv_tim_get_cap_dege(uint32_t adv_tim_x_base);
uint8_t     hal_adv_tim_get_clock_div(uint32_t adv_tim_x_base);

            //interrupt
void        hal_adv_tim_it_cfg(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag,hal_en_t en);
uint8_t     hal_adv_tim_get_it_flag(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag);
void        hal_adv_tim_clr_it_flag(uint32_t adv_tim_x_base,adv_tim_it_flag_t adv_tim_it_flag);

uint8_t     hal_adv_tim_get_status_flag(uint32_t adv_tim_x_base,adv_tim_status_flag_t adv_tim_status_flag);
void        hal_adv_tim_clr_status_flag(uint32_t adv_tim_x_base,adv_tim_status_flag_t adv_tim_status_flag);




#ifdef __cplusplus
}
#endif

#endif /* __HAL_ADV_TIMER_H */


/**************************************END OF FILE********************************************/
