#ifndef __LN_POWER_MGMT_H__
#define __LN_POWER_MGMT_H__

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "ln_types.h"

typedef enum
{
    ACTIVE          = 0,
    LIGHT_SLEEP     = 1,
    DEEP_SLEEP      = 2,
    FROZEN_SLEEP    = 3,
} sleep_mode_t;

typedef enum
{
    CLK_G_BLE       = (1U << 0U ),
    CLK_G_QSPI      = (1U << 1U ),
    CLK_G_ADC       = (1U << 2U ),
    CLK_G_I2S       = (1U << 3U ),
    CLK_G_GPIOA     = (1U << 4U ),
    CLK_G_GPIOB     = (1U << 5U ),
    CLK_G_SPI0      = (1U << 6U ),
    CLK_G_SPI1      = (1U << 7U ),
    CLK_G_WS2811    = (1U << 8U ),
    CLK_G_I2C0      = (1U << 9U ),
    CLK_G_UART0     = (1U << 10U),
    CLK_G_UART1     = (1U << 11U),
    CLK_G_UART2     = (1U << 12U),
    CLK_G_WDT       = (1U << 13U),
    CLK_G_TIM_REG   = (1U << 14U),
    CLK_G_TIM1      = (1U << 15U),
    CLK_G_TIM2      = (1U << 16U),
    CLK_G_TIM3      = (1U << 17U),
    CLK_G_TIM4      = (1U << 18U),
    CLK_G_DBGH      = (1U << 19U),
    CLK_G_SDIO      = (1U << 20U),
    CLK_G_MAC       = (1U << 21U),
    CLK_G_CACHE     = (1U << 22U),
    CLK_G_DMA       = (1U << 23U),
    CLK_G_RF        = (1U << 24U),
    CLK_G_ADV_TIMER = (1U << 25U),
    CLK_G_EFUSE     = (1U << 26U),
    CLK_G_TRNG      = (1U << 27U),
    CLK_G_AES       = (1U << 28U),
} soc_clk_gate_t;


void          ln_pm_sleep_mode_set(sleep_mode_t mode);
sleep_mode_t  ln_pm_sleep_mode_get(void);

void          ln_pm_always_clk_disable_select(uint32_t gate_cfg);
void          ln_pm_lightsleep_clk_disable_select(uint32_t gate_cfg);
int           ln_pm_obj_register(const char * name, int(*veto)(void), int(*pre_proc)(void), int(*post_proc)(void));

void          ln_pm_rtos_pre_sleep_proc(uint32_t *expect_ms);
int           ln_pm_rtos_post_sleep_proc(uint32_t *expect_ms);

void          ln_pm_sleep_frozen(void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __LN_POWER_MGMT_H__
