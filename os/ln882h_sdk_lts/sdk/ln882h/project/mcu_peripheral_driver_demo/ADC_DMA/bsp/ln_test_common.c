/**
 * @file     ln_test_common.c
 * @author   BSP Team 
 * @brief    This file provides peripheral test common functions.
 * @version  0.0.0.1
 * @date     2021-08-06
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_test_common.h"
#include "hal_clock.h"


/*This delay is the delay of the blocking method. When there is an interruption, the delay will be inaccurate!*/
void ln_delay_ms(unsigned int time)
{
    //Only supports 40M and 160M
    unsigned int timetout = 0;
    unsigned int time_1ms = 0;
    //40M
    if(hal_clock_get_core_clk() == 40000000)
    {
        time_1ms = 490;
    }
    else
    //160M
    {
        time_1ms = 19995*2;
    }
    while(time--)
    {
        timetout = time_1ms;
        while(timetout--);
    }
}

