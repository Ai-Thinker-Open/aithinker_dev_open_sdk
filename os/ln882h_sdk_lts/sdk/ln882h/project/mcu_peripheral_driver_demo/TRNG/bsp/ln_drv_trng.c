/**
 * @file     ln_drv_trng.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_trng.h"

/**
 * @brief 随机数初始化
 * 
 * @param range 设置随机数的范围
 * @param data  设置存储随机数的指针
 * @param count 设置要想生成随机数的数量
 */
void trng_init(uint32_t range,uint32_t *data,uint32_t count)
{
    uint32_t data_pos = 0;

    trng_init_t_def trng_init;
    memset(&trng_init,0,sizeof(trng_init));

    trng_init.trng_fast_mode_en_status = TRNG_FAST_MODE_DIS;
    trng_init.trng_gap = 10;
    
    hal_trng_init(TRNG_BASE,&trng_init);

    while(count > 4)
    {
        hal_trng_en(TRNG_BASE,HAL_ENABLE);
        while(hal_trng_get_it_flag(TRNG_BASE,TRNG_IT_FLAG_TRNG_DONE) == 0);   
        data[data_pos++] = hal_trng_get_data_0_31(TRNG_BASE)   % range;
        data[data_pos++] = hal_trng_get_data_32_63(TRNG_BASE)  % range;
        data[data_pos++] = hal_trng_get_data_64_95(TRNG_BASE)  % range;
        data[data_pos++] = hal_trng_get_data_96_127(TRNG_BASE) % range;     

        count -= 4;
    }

    if(count > 0)
    {
        hal_trng_en(TRNG_BASE,HAL_ENABLE);
        while(hal_trng_get_it_flag(TRNG_BASE,TRNG_IT_FLAG_TRNG_DONE) == 0);  
        switch(count)
        {
            case 1:
            {
                data[data_pos++] = hal_trng_get_data_0_31(TRNG_BASE) % range;
                break;
            }
            case 2:
            {
                data[data_pos++] = hal_trng_get_data_0_31(TRNG_BASE)  % range;
                data[data_pos++] = hal_trng_get_data_32_63(TRNG_BASE) % range;
                break;
            }
            case 3:
            {
                data[data_pos++] = hal_trng_get_data_0_31(TRNG_BASE)  % range;
                data[data_pos++] = hal_trng_get_data_32_63(TRNG_BASE) % range;
                data[data_pos++] = hal_trng_get_data_64_95(TRNG_BASE) % range;
                break;
            }
            case 4:
            {
                data[data_pos++] = hal_trng_get_data_0_31(TRNG_BASE)   % range;
                data[data_pos++] = hal_trng_get_data_32_63(TRNG_BASE)  % range;
                data[data_pos++] = hal_trng_get_data_64_95(TRNG_BASE)  % range;
                data[data_pos++] = hal_trng_get_data_96_127(TRNG_BASE) % range;
                break;
            }

        }
    }         
}

