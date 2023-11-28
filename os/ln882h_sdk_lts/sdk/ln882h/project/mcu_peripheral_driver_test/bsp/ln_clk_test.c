/**
 * @file     ln_clk_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-09-10
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
/**
        时钟测试说明：
        
                            1. 接线说明：
                                        LN8210          
                                        PA10        ->      RC振荡器 32K时钟
                                        PA11        ->      外部晶振振荡器 40Mhz时钟
*/

#include "ln_clk_test.h"
#include "ln882h.h"
#include "hal/hal_gpio.h"
#include "reg_sysc_cmp.h"

void ln_clk_test()
{
    /* 1. set fullmux funtion 52&53 (clk_test0 & clk_test1) */
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_10,CLK_TEST0);
    hal_gpio_pin_afio_select(GPIOA_BASE,GPIO_PIN_11,CLK_TEST1);

    /* 2. fullmux enable*/   
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_10,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOA_BASE,GPIO_PIN_11,HAL_ENABLE);

    /* 3. clock test enable*/   
    sysc_cmp_clk_test_en0_setf(1);
    sysc_cmp_clk_test_en1_setf(1);

    /* 4. select clock source for clk_test0 & clk_test1 (source clock should be enable first)*/   
    /*
    selection        1            2            3            4          5            6            7
    source           32k          40m          20m          10m        5m           2.5m         12m
    */
    sysc_cmp_clk_test_sel0_setf(1);        // Internal RC oscillator
    sysc_cmp_clk_test_sel1_setf(2);        // External crystal oscillator

/************************************************************************************************************************/
    while(1)
    {
        unsigned int count = 10000;
        while(count--);
    }
}

