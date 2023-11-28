#include "hal/hal_common.h"
#include "hal/hal_misc.h"
#include "hal/hal_clock.h"
#include "string.h"
/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
extern uint32_t __Vectors[];
#define vector_table	__Vectors
#endif

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = XTAL_CLOCK;  /* System Core Clock Frequency */
/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
    SystemCoreClock = hal_clock_get_core_clk();
}
uint32_t get_SystemCoreClock(void)
{
    return SystemCoreClock;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{

#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
    __DMB();
    SCB->VTOR = (uint32_t) &vector_table;
    __DSB();
#endif

#if defined (__FPU_USED) && (__FPU_USED == 1U)
    SCB->CPACR |= ((3U << 10U*2U) | /* enable CP10 Full Access */
                  (3U << 11U*2U) ); /* enable CP11 Full Access */
#endif

#if defined(SCB_SHCSR_MEMFAULTENA_Msk)
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
#endif
#if defined(SCB_SHCSR_BUSFAULTENA_Msk)
    SCB->SHCSR |= SCB_SHCSR_BUSFAULTENA_Msk;
#endif
#if defined(SCB_SHCSR_USGFAULTENA_Msk)
    SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
#endif
    
#if defined(SCB_CCR_DIV_0_TRP_Msk)
    SCB->CCR |= SCB_CCR_DIV_0_TRP_Msk;
#endif

#ifdef UNALIGNED_SUPPORT_DISABLE
  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif
}

/* 
* Configure the System clock source, 
* PLL Multiplier and Divider factors, 
* AHB/APBx prescalers and 32K settings.
* This function must be called in __main, because RW section must be initialized.
*/
void SetSysClock(void)
{
/*
    if use pll clock:
                        APB0 Clock = XTAL_CLOCK * PLL_MUL / AHB_DIV / APB0_DIV
                        AHB  Clock = XTAL_CLOCK * PLL_MUL / AHB_DIV
                        Core Clock = XTAL_CLOCK * PLL_MUL
    if not use pll clock:

                        APB0 Clock = XTAL_CLOCK / AHB_DIV / APB0_DIV
                        AHB  Clock = XTAL_CLOCK / AHB_DIV
                        Core Clock = XTAL_CLOCK 
*/
    clock_init_t clock_init;
    memset((void *)(&clock_init),0,sizeof(clock_init));
    
    
    /*
        APB0 Clock = 40M
        AHB  Clock = 160M
        Core Clock = 160M
    */
    clock_init.clk_src          = CLK_SRC_PLL;
    clock_init.clk_pclk0_div    = CLK_PCLK0_4_DIV;
    clock_init.clk_hclk_div     = CLK_HCLK_NO_DIV;
    clock_init.clk_pllclk_mul   = CLK_PLL_CLK_4_MUL;

    //1. xtal40m config
    hal_misc_awo_set_r_vtrim(1);              //(1: comply with FT_Test) set Vbandgap 1.21V, Fix chip work abnormal at -30C. -- DongBao
    hal_misc_awo_set_r_xtal40m_ldo_vbit(3);   //5 xtal40m LDO (1.2V ~ )
    hal_misc_awo_set_r_xtal40m_cap_bit(0);    // xtal40m Cap (pf)
    hal_misc_awo_set_r_xtal40m_ibsel(0);      //1
    hal_misc_awo_set_ldo18_4_vset(7);
    hal_misc_awo_set_ldo18_2_vset(7);         // tx_rx_filter LDO18_2 vol set
    hal_misc_awo_set_ldo15_1_vset(7);         //(7: optimize pll @vtrim=1) Fix OFDM package losses

    hal_misc_awo_set_o_cpu_sleep_counter_bp(0); //32K RCO cal
    hal_misc_awo_set_bod_vth(0x03);             //0x03=2.2V BOD, fix reboot problem

    //2. clock
    hal_clock_init(&clock_init);
    SystemCoreClock = hal_clock_get_core_clk();
}

