/**
 * @file     hal_clock.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2022-01-19
 * 
 * @copyright Copyright (c) 2022 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_clock.h"

clk_src_t hal_clock_clk_src;

uint32_t hal_clock_core_clk = 0;
uint32_t hal_clock_ahb_clk = 0;
uint32_t hal_clock_apb0_clk = 0;


void hal_clock_init(clock_init_t *clock_init)
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
    
    /* check the parameters */
    hal_assert(IS_CLK_SRC(clock_init->clk_src));
    hal_assert(IS_CLK_PLL_CLK_MUL(clock_init->clk_pllclk_mul));
    hal_assert(IS_CLK_PCLK0_DIV(clock_init->clk_pclk0_div));
    hal_assert(IS_CLK_HCLK_DIV(clock_init->clk_hclk_div));

    sysc_awo_clk_src_sel_setf(clock_init->clk_src);
    sysc_awo_syspll_div_setf(clock_init->clk_pllclk_mul);
    sysc_cmp_pclk0_div_para_m1_setf(clock_init->clk_pclk0_div - 1);
    //sysc_awo_pclk1_div_para_m1_setf(clock_init->clk_pclk0_div - 1);
    sysc_awo_hclk_div_para_m1_setf(clock_init->clk_hclk_div - 1);

    sysc_cmp_rfintf_div_para_m1_setf(0);//rf interface 40M
    
    sysc_awo_hclk_div_para_up_setf(1);
    sysc_cmp_pclk0_div_para_up_setf(1);
    //sysc_awo_pclk1_div_para_up_setf(1);

    sysc_cmp_rfintf_div_para_up_setf(1);//rf interface 40M
    
    if(clock_init->clk_src == CLK_SRC_XTAL)
    {
        hal_clock_clk_src = CLK_SRC_XTAL;
        hal_clock_ahb_clk = XTAL_CLOCK / clock_init->clk_hclk_div;
        hal_clock_apb0_clk = XTAL_CLOCK / clock_init->clk_hclk_div / clock_init->clk_pclk0_div;
        hal_clock_core_clk = XTAL_CLOCK;
    }
    else if(clock_init->clk_src == CLK_SRC_PLL)
    {
        hal_clock_clk_src = CLK_SRC_PLL;
        hal_clock_ahb_clk = XTAL_CLOCK * clock_init->clk_pllclk_mul / 2 / clock_init->clk_hclk_div;
        hal_clock_apb0_clk = XTAL_CLOCK * clock_init->clk_pllclk_mul / 2 / clock_init->clk_hclk_div / clock_init->clk_pclk0_div;
        hal_clock_core_clk = XTAL_CLOCK * clock_init->clk_pllclk_mul / 2;
    }
    
    
}
void hal_clock_select_clk_src(clk_src_t clk_src)
{
    /* check the parameters */
    hal_assert(IS_CLK_SRC(clk_src));
    sysc_awo_clk_src_sel_setf(clk_src);
    
    if(clk_src == CLK_SRC_XTAL)
    {
        hal_clock_clk_src = CLK_SRC_XTAL;
        hal_clock_ahb_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK;
    }
    else if(clk_src == CLK_SRC_PLL)
    {
        hal_clock_clk_src = CLK_SRC_PLL;
        hal_clock_ahb_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2;
    }
}

void hal_clock_set_pll_clk_mul(clk_pllclk_mul_t clk_pllclk_mul)
{
    /* check the parameters */
    hal_assert(IS_CLK_PLL_CLK_MUL(clk_pllclk_mul));
    sysc_awo_syspll_div_setf(clk_pllclk_mul);
    
    if(hal_clock_clk_src == CLK_SRC_XTAL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK;
    }
    else if(hal_clock_clk_src == CLK_SRC_PLL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2;
    }
}

void hal_clock_set_apb0_clk_div(clk_pclk0_div_t clk_pclk0_div)
{
    /* check the parameters */
    hal_assert(IS_CLK_PCLK0_DIV(clk_pclk0_div));
    sysc_cmp_pclk0_div_para_m1_setf(clk_pclk0_div - 1);
    sysc_cmp_pclk0_div_para_up_setf(1);
    
    if(hal_clock_clk_src == CLK_SRC_XTAL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK;
    }
    else if(hal_clock_clk_src == CLK_SRC_PLL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2;
    }
}

void hal_clock_set_ahb_clk_div(clk_hclk_div_t clk_hclk_div)
{
    /* check the parameters */
    hal_assert(IS_CLK_HCLK_DIV(clk_hclk_div));
    sysc_awo_hclk_div_para_m1_setf(clk_hclk_div - 1);
    sysc_awo_hclk_div_para_up_setf(1);
    
    if(hal_clock_clk_src == CLK_SRC_XTAL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK;
    }
    else if(hal_clock_clk_src == CLK_SRC_PLL)
    {
        hal_clock_ahb_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1);
        hal_clock_apb0_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2 / (sysc_awo_hclk_div_para_m1_getf() + 1) / (sysc_cmp_pclk0_div_para_m1_getf() + 1);
        hal_clock_core_clk = XTAL_CLOCK * (sysc_awo_syspll_div_getf()) / 2;
    }
}

clk_src_t hal_clock_get_clk_src()
{
    return hal_clock_clk_src;
}

uint32_t hal_clock_get_core_clk()
{
    return hal_clock_core_clk;
}

uint32_t hal_clock_get_apb0_clk()
{
    return hal_clock_apb0_clk;
}

uint32_t hal_clock_get_ahb_clk()
{
    return hal_clock_ahb_clk;
}
