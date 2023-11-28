/**
 * @file     hal_rtc.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-23
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_rtc.h"

void hal_rtc_init(uint32_t rtc_base,rtc_init_t_def* rtc_init)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));

    /* Check the RTC parameters */
    hal_assert(IS_RTC_WRAP_EN(rtc_init->rtc_warp_en));

    /* Set the RTC swap enable status */
    if (rtc_init->rtc_warp_en == RTC_WRAP_EN) {
        rtc_rtc_wen_setf(rtc_base,1);
    }
    else if (rtc_init->rtc_warp_en == RTC_WRAP_DIS) {
        rtc_rtc_wen_setf(rtc_base,0);
    }
}

void hal_rtc_deinit(void)
{
    sysc_awo_srstn_rtc_setf(0);
    sysc_awo_srstn_rtc_setf(1);
}

void hal_rtc_en(uint32_t rtc_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    if (en == HAL_ENABLE) {
        rtc_rtc_en_setf(rtc_base,1);
    }
    else if (en == HAL_DISABLE) {
        rtc_rtc_en_setf(rtc_base,0);
    }

}

void hal_rtc_set_cnt_match(uint32_t rtc_base,uint32_t match_value)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_RTC_CNT_MATCH_VALUE(match_value));
    rtc_countermatch_setf(rtc_base,match_value);
}

void hal_rtc_set_cnt_load(uint32_t rtc_base,uint32_t load_value)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_RTC_CNT_LOAD_VALUE(load_value));
    rtc_counterload_setf(rtc_base,load_value);
}

uint32_t hal_rtc_get_cnt(uint32_t rtc_base)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    return rtc_currentcountervalue_getf(rtc_base);
}

        
void hal_rtc_it_cfg(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag,hal_en_t en)
{
    uint8_t it_flag = 0;   
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_RTC_IT_FLAG(rtc_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    for(int i = 0; i < RTC_IT_FLAG_BIT_NUM; i++)
    {
        it_flag = (rtc_it_flag & (0x01 << i));
        switch (it_flag)
        {
            case RTC_IT_FLAG_ACTIVE:
                if (en == HAL_ENABLE){
                    rtc_rtc_ien_setf(rtc_base,1);
                    rtc_rtc_mask_setf(rtc_base,0);
                }
                else if (en == HAL_DISABLE) {
                    rtc_rtc_ien_setf(rtc_base,0);
                    rtc_rtc_mask_setf(rtc_base,1);
                }
                break;
            default:
                break;
        }
    }
}

uint8_t hal_rtc_get_it_flag(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_RTC_IT_FLAG(rtc_it_flag));
    switch (rtc_it_flag)
    {
        case RTC_IT_FLAG_ACTIVE:
            it_flag = rtc_rtc_stat_getf(rtc_base);
            break;
        case RTC_IT_FLAG_ACTIVE_RAW:
            it_flag = rtc_rtc_rstat_getf(rtc_base);
        default:
            break;
    }
    return it_flag;
}

void hal_rtc_clr_it_flag(uint32_t rtc_base,rtc_it_flag_t rtc_it_flag)
{
    /* check the parameters */
    hal_assert(IS_RTC_ALL_PERIPH(rtc_base));
    hal_assert(IS_RTC_IT_FLAG(rtc_it_flag));
    switch (rtc_it_flag)
    {
        case RTC_IT_FLAG_ACTIVE:
            rtc_rtc_eoi_getf(rtc_base);      //Read the RTC_EOI reg,can clear the interrupt flag.
            break;
        default:
            break;
    }
}

/************************END OF FILE*******************************/

