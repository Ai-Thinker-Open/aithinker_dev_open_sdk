/**
 * @file     ln_ds18b20_test.h
 * @author   BSP Team 
 * @brief    This file contains all of ds18b20 test functions
 * @version  0.0.0.1
 * @date     2021-03-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef LN_DS18B20_TEST_H
#define LN_DS18B20_TEST_H

#include "hal/hal_common.h"
#include "hal/hal_gpio.h"



#define DATA_OUT_MODE       (hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_7,GPIO_OUTPUT))

#define DATA_IN_MODE        (hal_gpio_pin_direction_set(GPIOB_BASE,GPIO_PIN_7,GPIO_INPUT))

#define DATA_OUT(STATUS)    (((STATUS == 1)? hal_gpio_pin_set(GPIOB_BASE, GPIO_PIN_7) : hal_gpio_pin_reset(GPIOB_BASE, GPIO_PIN_7)))

#define DATA_READ           hal_gpio_pin_input_read(GPIOB_BASE, GPIO_PIN_7)



#define DS18B220_CMD_READ_ROM       0x33
#define DS18B220_CMD_MATCH_ROM      0x55
#define DS18B220_CMD_SKIP_ROM       0xCC
#define DS18B220_CMD_SEARCH_ROM     0xF0
#define DS18B220_CMD_ALARM_ROM      0xEC

#define DS18B220_CMD_CONVERT_T      0x44
#define DS18B220_CMD_WR_SRATCH      0x4E
#define DS18B220_CMD_RD_SRATCH      0xBE
#define DS18B220_CMD_CP_SRATCH      0x48
#define DS18B220_CMD_RECALL_EE      0xE8
#define DS18B220_CMD_RD_POWERS      0xB4




typedef union 
{
    struct
    {
        unsigned char family_code : 8 ;
        long long     serial_code : 48;
        unsigned char crc_value   : 8 ;
    }bit_field;
    unsigned char data[8];
}ds18b20_rom_code;

typedef union 
{
    struct
    {
        unsigned char temp_l        : 8 ;
        unsigned char temp_h        : 8 ;
        unsigned char th_reg        : 8 ;
        unsigned char tl_reg        : 8 ;
        unsigned char cfg_reg       : 8 ;
        unsigned char reserved_0    : 8 ;        //FFH
        unsigned char reserved_1    : 8 ;        //
        unsigned char reserved_2    : 8 ;        //10H
        unsigned char crc_value     : 8 ;
    }bit_field;
    unsigned char data[8];
}ds18b20_mem_data;

enum ds18b20_data_resolu
{   
    TEMP_9_BIT  = 0x1F,
    TEMP_10_BIT = 0x3F,
    TEMP_11_BIT = 0x5F,
    TEMP_12_BIT = 0x7F,

};

typedef struct 
{
    ds18b20_rom_code    rom_code;
    ds18b20_mem_data    mem_data;
}ds18b20_data_str;

extern ds18b20_data_str    ds18b20_data[1];
void ln_ds18b20_test(void);

#endif
