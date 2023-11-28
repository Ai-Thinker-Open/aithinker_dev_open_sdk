/**
 * @file     main.c
 * @author   BSP Team 
 * @brief    
 * @version  0.0.0.1
 * @date     2021-08-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "ln_test_common.h"

#include "ln_drv_spi_dma.h"
#include "ln_drv_spi_flash.h"


static unsigned char tx_data[100];
static unsigned char rx_data[100];

static unsigned char status[2];
static unsigned int err_cnt = 0;

/*
 *  注意！！！ 
 *  在驱动例程中，有一些函数的参数是指向字符串（数组）地址的指针，函数中没有检查这些指针的有效性，
 *  没有判断指针是否为空，也没有判断指针指向的数据是否还有剩余空间，这些都需要使用者自行决定！！！
*/
int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. spi flash 测试***********************/
    spi_pin_cfg_t spi_pin_cfg;
    memset(&spi_pin_cfg,0,sizeof(spi_pin_cfg));

    spi_pin_cfg.gpio_clk_port  = GPIO_A;
    spi_pin_cfg.gpio_clk_pin   = GPIO_PIN_5;

    spi_pin_cfg.gpio_mosi_port = GPIO_A;
    spi_pin_cfg.gpio_mosi_pin  = GPIO_PIN_6;

    spi_pin_cfg.gpio_miso_port = GPIO_A;
    spi_pin_cfg.gpio_miso_pin  = GPIO_PIN_7;

    spi_pin_cfg.gpio_cs_port   = GPIO_A;
    spi_pin_cfg.gpio_cs_pin    = GPIO_PIN_10;

    spi_dma_master_init(&spi_pin_cfg);

    for(int i = 0; i < 100 ; i++)
    {
        tx_data[i] = i + 2;
    }

    //读取ID
    spi_flash_read_id(rx_data);
    LOG(LOG_LVL_INFO,"LN882H 25WQ16 ID: %x %x %x \n",rx_data[0],rx_data[1],rx_data[2]);
    
    //读取状态
    spi_flash_read_status(rx_data);

    //写使能
    spi_flash_write_enable();

    //读取状态
    spi_flash_read_status(rx_data);

    //擦除芯片
    spi_flash_erase_flash(0x200);

    //等待擦除完成
    while(1)
    {
        spi_flash_read_status(status);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    //向Flash芯片中写入数据
    spi_flash_write_flash(0x200,tx_data,100);

    //等待写入完成
    while(1)
    {
        spi_flash_read_status(status);
        if((status[0] & 0x01) == 0x00)
            break;
    }

    //从Flash芯片中读出数据
    spi_flash_read_flash(0x200,rx_data,100);
    
    //判断写入的数据是否正确
    err_cnt = 0;
    for(int i = 0 ; i < 100; i++)
    {
        if(rx_data[i] != tx_data[i])
        {
            err_cnt++;
        }
    }

    //打印LOG
    if(err_cnt != 0)
    {
        LOG(LOG_LVL_INFO,"LN882H SPI DMA + 25WQ16 test fail! \n");
    }
    else
    {
        LOG(LOG_LVL_INFO,"LN882H SPI DMA + 25WQ16 test success! \n");
    }
    
    while(1)
    {
        LOG(LOG_LVL_INFO,"LN882H is running! \n");
        ln_delay_ms(5000);
    }
}
