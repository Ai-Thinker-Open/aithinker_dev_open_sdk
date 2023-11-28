/**
 * @file     ln_flash_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-30
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
       Flash测试说明：
       
            1. 这个Flash 是指芯片用来存储程序的Flash，使用QSPI操作这个Flash的


*/



#include "ln_test_common.h"
#include "ln_flash_test.h"
#include "hal_flash.h"


static volatile uint32_t flash_id = 0;


static uint8_t test_data[4096];
static uint8_t read_data[4096];

static uint32_t err_cnt = 0;


void ln_flash_test()
{
    //hal_flash_init();

    /* 1. 读取flash ID */
    flash_id = hal_flash_read_id();

    /* 2. 擦除一部分Flash，要4K对齐的擦除 */
    hal_flash_erase(0x006000,0x1000);

    /* 3. 生成测试用数据 */
    for(int i = 0; i < 4096; i++)
        test_data[i] = i * 7 + 2;

    /* 4. 写入测试数据*/
    hal_flash_program(0x006000,0x1000,test_data);

    /* 5. 读取并比对数据,根据 err_cnt 计算出是否写入出错 */
    hal_flash_read(0x006000,0x1000,read_data);

    for(int i = 0; i < 4096; i++)
    {
        if(read_data[i] != test_data[i])
            err_cnt ++;
    }

    memset(read_data,0,4096);

    /* 5. 通过CACHE读取并比对数据,根据 err_cnt 计算出是否写入出错 */
    hal_flash_read_by_cache(0x006000,0x1000,read_data);

    for(int i = 0; i < 4096; i++)
    {
        if(read_data[i] != test_data[i])
            err_cnt ++;
    }


    while(1)
    {
        //do nothing.
        ln_delay_ms(500);
    }
}

