/**
 * @file     hal_common.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-17
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "utils/debug/log.h"
#include "hal/hal_common.h"

void assert_failed(uint8_t* file, uint32_t line)
{
    uint8_t *file_name = file;
    uint32_t line_num  = line;

    while(1)
    {
        LOG(LOG_LVL_ERROR,"ASSERT Failed-> file: %s @ line %d\r\n", file_name, line_num);
        for(volatile uint32_t i = 0; i < 10000000;i++);
    };
}

