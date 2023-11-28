/**
 * @file     ln_show_reg.c
 * @author   BSP Team 
 * @brief    This file provides show the reg in the watch window function.
 * @version  0.0.0.1
 * @date     2021-03-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "ln_show_reg.h"
#include "ln882h.h"
#include "hal/hal_adv_timer.h"
#include "hal/hal_timer.h"

#include "reg_dma.h"
#include "reg_cache.h"
#include "reg_sysc_awo.h"
#include "reg_sysc_cmp.h"
#include "reg_cache.h"
#include "reg_ln_uart.h"
#include "reg_i2c.h"
#include "reg_i2s.h"
#include "reg_spi.h"
#include "reg_qspi.h"
#include "reg_timer.h"
#include "reg_efuse.h"
#include "reg_adc.h"
#include "reg_wdt.h"
#include "reg_adv_timer.h"
#include "reg_rtc.h"
#include "reg_trng.h"
#include "reg_gpio.h"
#include "reg_ws2811.h"
#include "reg_sdio.h"
#include "reg_aes.h"

volatile t_hwp_dma_t        *DMA            = (t_hwp_dma_t *)       DMA_BASE;
volatile T_HWP_CACHE_T      *CACHE          = (T_HWP_CACHE_T *)     CACHE_FLASH_BASE;
volatile T_HWP_QSPI_T       *QSPI           = (T_HWP_QSPI_T *)      QSPI_BASE;
volatile t_hwp_sysc_cmp_t   *CMP            = (t_hwp_sysc_cmp_t *)  SYSC_CMP_BASE;
volatile t_hwp_ln_uart_t    *UART0          = (t_hwp_ln_uart_t *)   UART0_BASE;
volatile t_hwp_ln_uart_t    *UART1          = (t_hwp_ln_uart_t *)   UART1_BASE;
volatile t_hwp_ln_uart_t    *UART2          = (t_hwp_ln_uart_t *)   UART2_BASE;

volatile t_hwp_i2c_t        *I2C            = (t_hwp_i2c_t *)       I2C_BASE;
volatile T_HWP_I2S_T        *I2S            = (T_HWP_I2S_T *)       I2S_BASE;
volatile t_hwp_spi_t        *SPI0           = (t_hwp_spi_t *)       SPI0_BASE;
volatile t_hwp_spi_t        *SPI1           = (t_hwp_spi_t *)       SPI1_BASE;
volatile t_hwp_efuse_t      *EFUSE          = (t_hwp_efuse_t *)     EFUSE_BASE;

volatile t_hwp_timer_tmp_t  *TIMER0         = (t_hwp_timer_tmp_t *) TIMER0_BASE;
volatile t_hwp_timer_tmp_t  *TIMER1         = (t_hwp_timer_tmp_t *) TIMER1_BASE;
volatile t_hwp_timer_tmp_t  *TIMER2         = (t_hwp_timer_tmp_t *) TIMER2_BASE;
volatile t_hwp_timer_tmp_t  *TIMER3         = (t_hwp_timer_tmp_t *) TIMER3_BASE;

volatile t_hwp_adc_t        *ADC            = (t_hwp_adc_t *)       ADC_BASE;



volatile T_HWP_WDT_T        *WDT            = (T_HWP_WDT_T *)       WDT_BASE;

volatile t_hwp_pwm_t0       *ADV_TIMER_ISR  = (t_hwp_pwm_t0 *)      ADV_TIMER_IT_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_0    = (t_hwp_pwm_t *)       ADV_TIMER_0_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_1    = (t_hwp_pwm_t *)       ADV_TIMER_1_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_2    = (t_hwp_pwm_t *)       ADV_TIMER_2_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_3    = (t_hwp_pwm_t *)       ADV_TIMER_3_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_4    = (t_hwp_pwm_t *)       ADV_TIMER_4_BASE;
volatile t_hwp_pwm_t        *ADV_TIMER_5    = (t_hwp_pwm_t *)       ADV_TIMER_5_BASE;

volatile t_hwp_sysc_awo_t   *AWO            = (t_hwp_sysc_awo_t *)  SYSC_AWO_BASE;
volatile t_hwp_rtc_t        *RTC            = (t_hwp_rtc_t *)       RTC_BASE;
volatile t_hwp_trng_t       *TRNG           = (t_hwp_trng_t *)      TRNG_BASE;
volatile t_hwp_ws2811_t     *WS2811         = (t_hwp_ws2811_t*)     WS2811_BASE;

volatile t_hwp_gpio_t       *GPIOA          = (t_hwp_gpio_t *)      GPIOA_BASE;
volatile t_hwp_gpio_t       *GPIOB          = (t_hwp_gpio_t *)      GPIOB_BASE;

volatile T_HWP_SDIO_T       *SDIO           =  (T_HWP_SDIO_T*)      SDIO_BASE;

volatile t_hwp_aes_t        *AES            =  (t_hwp_aes_t*)       AES_BASE;



void ln_show_reg_init()
{
    //dummy read register
    DMA->dma_ccr1.val;
    CACHE->reg_cache_en.val;
    QSPI->baudr;
    CMP->clk_test;
    UART0->uart_cr1;    
    UART1->uart_cr1;    
    UART2->uart_cr1;  
    I2C->i2c_ccr.val;
    I2S->ch0_imr;
    SPI0->cr1.val;
    SPI1->cr1.val;
    EFUSE->efuse_key1;
    TIMER0->timercontrolreg.val;
    TIMER1->timercontrolreg.val;
    TIMER2->timercontrolreg.val;
    TIMER3->timercontrolreg.val;
    ADC->adc_ccr;
    WDT->wdt_ccvr.val;
    ADV_TIMER_ISR->ise.val;
    ADV_TIMER_0->cr;
    ADV_TIMER_1->cr;
    ADV_TIMER_2->cr;
    ADV_TIMER_3->cr;
    ADV_TIMER_4->cr;
    ADV_TIMER_5->cr;
    AWO->clk_sel;
    RTC->rtc_ccr.val;
    TRNG->trng_cr.val;
    GPIOA->gpio_bsrr.val;
    GPIOB->gpio_ddr.val;
    SDIO->sdio_cis_fn0;
    AES->ctr0;
}

