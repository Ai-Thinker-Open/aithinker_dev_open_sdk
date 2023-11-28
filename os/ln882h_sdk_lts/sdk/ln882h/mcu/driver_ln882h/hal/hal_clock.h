/**
 * @file     hal_clock.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2022-01-19
 * 
 * @copyright Copyright (c) 2022 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_CLOCK_H
#define __HAL_CLOCK_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "hal/hal_common.h"

typedef enum
{
    CLK_SRC_XTAL      = 0,
    CLK_SRC_PLL       = 1,
}clk_src_t;


#define IS_CLK_SRC(SRC) (((SRC) == CLK_SRC_XTAL) || ((SRC) == CLK_SRC_PLL))

typedef enum
{
    CLK_PCLK0_NO_DIV      = 1,
    CLK_PCLK0_2_DIV       = 2,
    CLK_PCLK0_3_DIV       = 3,
    CLK_PCLK0_4_DIV       = 4,
    CLK_PCLK0_5_DIV       = 5,
    CLK_PCLK0_6_DIV       = 6,
    CLK_PCLK0_7_DIV       = 7,
    CLK_PCLK0_8_DIV       = 8,
    CLK_PCLK0_9_DIV       = 9,
    CLK_PCLK0_10_DIV      = 10,
    CLK_PCLK0_11_DIV      = 11,
    CLK_PCLK0_12_DIV      = 12,
    CLK_PCLK0_13_DIV      = 13,
    CLK_PCLK0_14_DIV      = 14,
    CLK_PCLK0_15_DIV      = 15,
    CLK_PCLK0_16_DIV      = 16,

}clk_pclk0_div_t;

#define IS_CLK_PCLK0_DIV(DIV) (((DIV) >= CLK_PCLK0_NO_DIV) && ((DIV) <= CLK_PCLK0_16_DIV))
 

typedef enum
{
    CLK_HCLK_NO_DIV      = 1,
    CLK_HCLK_2_DIV       = 2,
    CLK_HCLK_3_DIV       = 3,
    CLK_HCLK_4_DIV       = 4,
    CLK_HCLK_5_DIV       = 5,
    CLK_HCLK_6_DIV       = 6,
    CLK_HCLK_7_DIV       = 7,
    CLK_HCLK_8_DIV       = 8,
    CLK_HCLK_9_DIV       = 9,
    CLK_HCLK_10_DIV      = 10,
    CLK_HCLK_11_DIV      = 11,
    CLK_HCLK_12_DIV      = 12,
    CLK_HCLK_13_DIV      = 13,
    CLK_HCLK_14_DIV      = 14,
    CLK_HCLK_15_DIV      = 15,
    CLK_HCLK_16_DIV      = 16,
    
}clk_hclk_div_t;

#define IS_CLK_HCLK_DIV(DIV) (((DIV) >= CLK_HCLK_NO_DIV) && ((DIV) <= CLK_HCLK_16_DIV))

typedef enum
{ 
    CLK_PLL_CLK_NO_MUL         = 2,       //1.0
    CLK_PLL_CLK_1_MUL          = 2,       //1.0
    CLK_PLL_CLK_1_5_MUL        = 3,       //1.5 
    CLK_PLL_CLK_2_MUL          = 4,       //2.0
    CLK_PLL_CLK_2_5_MUL        = 5,       //2.5
    CLK_PLL_CLK_3_MUL          = 6,       //3.0
    CLK_PLL_CLK_3_5_MUL        = 7,       //3.5
    CLK_PLL_CLK_4_MUL          = 8,       //4.0
    CLK_PLL_CLK_4_5_MUL        = 9,       //Not recommended
    CLK_PLL_CLK_5_MUL          = 10,      //Not recommended
    CLK_PLL_CLK_5_5_MUL        = 11,      //Not recommended
    CLK_PLL_CLK_6_MUL          = 12,      //Not recommended
}clk_pllclk_mul_t;


#define IS_CLK_PLL_CLK_MUL(MUL) (((MUL) >= CLK_PLL_CLK_NO_MUL) && ((MUL) <= CLK_PLL_CLK_6_MUL))

typedef struct
{
    /*
        if use pll clock:
                            APB  Clock = XTAL_CLOCK * PLL_MUL / AHB_DIV / APB_DIV
                            AHB  Clock = XTAL_CLOCK * PLL_MUL / AHB_DIV
                            Core Clock = XTAL_CLOCK * PLL_MUL
        if not use pll clock:

                            APB  Clock = XTAL_CLOCK / AHB_DIV / APB_DIV
                            AHB  Clock = XTAL_CLOCK / AHB_DIV
                            Core Clock = XTAL_CLOCK 
    */


    clk_src_t                   clk_src;                    /*!< Specifies the clock source.
                                                            This parameter can be a value of @ref clk_src_t */

    clk_pllclk_mul_t            clk_pllclk_mul;             /*!< Specifies the pll clock mul.
                                                            This parameter can be a value of @ref clk_pllclk_mul_t */

    clk_pclk0_div_t             clk_pclk0_div;               /*!< Specifies the APB clock div.
                                                            The range of parameters can be referred to @ref clk_pclk_div_t */

    clk_hclk_div_t              clk_hclk_div;               /*!< Specifies the AHB clock div.
                                                            This parameter can be a value of @ref clk_hclk_div_t */

}clock_init_t;


void        hal_clock_init(clock_init_t *clock_init);
void        hal_clock_select_clk_src(clk_src_t clk_src);
void        hal_clock_set_pll_clk_mul(clk_pllclk_mul_t clk_pllclk_mul);
void        hal_clock_set_apb0_clk_div(clk_pclk0_div_t clk_pclk0_div);
void        hal_clock_set_ahb_clk_div(clk_hclk_div_t clk_hclk_div);
clk_src_t   hal_clock_get_clk_src(void);
uint32_t    hal_clock_get_core_clk(void);
uint32_t    hal_clock_get_apb0_clk(void);
uint32_t    hal_clock_get_ahb_clk(void);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_CLOCK_H */
