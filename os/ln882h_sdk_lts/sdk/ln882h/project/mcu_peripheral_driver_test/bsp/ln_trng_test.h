/**
 * @file     ln_trng_test.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-09-10
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef LN_TRNG_TEST_H
#define LN_TRNG_TEST_H

#include "hal/hal_trng.h"
#include "hal/hal_gpio.h"
#include "ln_trng_test.h"
#include "utils/debug/log.h"

volatile unsigned int random_num[4]; 
void ln_trng_test(void)
{
    

    trng_init_t_def trng_init;
    memset(&trng_init,0,sizeof(trng_init));
    trng_init.trng_fast_mode_en_status = TRNG_FAST_MODE_DIS;
    trng_init.trng_gap = 10;
    
    hal_trng_init(TRNG_BASE,&trng_init);
    
    hal_trng_en(TRNG_BASE,HAL_ENABLE);
    
    while(hal_trng_get_en_status(TRNG_BASE) == 1);
    while(hal_trng_get_it_flag(TRNG_BASE,TRNG_IT_FLAG_TRNG_DONE) == 0)
       
    random_num[0] = hal_trng_get_data_0_31(TRNG_BASE);
    random_num[1] = hal_trng_get_data_32_63(TRNG_BASE);
    random_num[2] = hal_trng_get_data_64_95(TRNG_BASE);
    random_num[3] = hal_trng_get_data_96_127(TRNG_BASE);
      
    while(1)
    {
        hal_trng_en(TRNG_BASE,HAL_ENABLE);
    
        while(hal_trng_get_en_status(TRNG_BASE) == 1);
        while(hal_trng_get_it_flag(TRNG_BASE,TRNG_IT_FLAG_TRNG_DONE) == 0);
        
        random_num[0] = hal_trng_get_data_0_31(TRNG_BASE);
        random_num[1] = hal_trng_get_data_32_63(TRNG_BASE);
        random_num[2] = hal_trng_get_data_64_95(TRNG_BASE);
        random_num[3] = hal_trng_get_data_96_127(TRNG_BASE);
        
        LOG(LOG_LVL_I,"random_num[0]  = 0x%X! \n",random_num[0]);
        LOG(LOG_LVL_I,"random_num[1]  = 0x%X! \n",random_num[1]);
        LOG(LOG_LVL_I,"random_num[2]  = 0x%X! \n",random_num[2]);
        LOG(LOG_LVL_I,"random_num[3]  = 0x%X! \n\n\n\n\n",random_num[3]);
        
        ln_delay_ms(2000);
    }
}


#endif