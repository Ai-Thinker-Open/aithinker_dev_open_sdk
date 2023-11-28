/**
 * @file     ln_sdio_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-30
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/*
    SDIO使用说明：
    
                1. 接线说明：
                
                           LN882H(从)       STM32F4阿波罗(主)
                    CMD -> PA8      ->      PD2
                    CLK -> PA9      ->      PC12
                    SD0 -> PA10     ->      PC8
                    SD1 -> PA11     ->      PC9
                    SD2 -> PA6      ->      PC10
                    SD3 -> PA7      ->      PC11
                    
                2. 跳线测试速度推荐4Mhz，主机配置从机一些寄存器参数的时候，属于低速通信，当配置完成后，才会切到高速通信。
                
                3. SDIO 的 CMD、SD0、SD1、SD2、SD3需要接上拉电阻。
                
                4. SDIO数据CRC校验需要手动打开CRC的校验中断，打开CRC校验之后需要手动在数据块的末尾加上CRC校验，例如数据块大小256，那就在第255和第256处放两字节CRC.
                            
                5. 



*/






#include "hal_sdio_device.h"
#include "hal_misc.h"
#include "hal_gpio.h"
#include "ln_test_common.h"
#include "ln_sdio_test.h"

#include "utils/debug/log.h"

static uint8_t sdio_cis_fn0[128];
static uint8_t sdio_cis_fn1[128];

static uint8_t tx_data[512];
static uint8_t rx_data[512];

void ln_sdio_init()
{
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_6,GPIO_PULL_NONE);              //配置引脚上拉
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_7,GPIO_PULL_NONE);              //配置引脚上拉
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_10,GPIO_PULL_NONE);             //配置引脚上拉
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_11,GPIO_PULL_NONE);             //配置引脚上拉
    
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_8,GPIO_PULL_NONE);              //配置引脚上拉
    hal_gpio_pin_pull_set(GPIOA_BASE,GPIO_PIN_9,GPIO_PULL_NONE);              //配置引脚上拉
    


    hal_misc_cmp_set_sdio_io_en(1);                                         //使能SDIO引脚

    sdio_init_t sdio_init;
    memset(&sdio_init,0,sizeof(sdio_init));
    
    sdio_init.recv_buff = rx_data;                                          //设置接收数据的BUFFER
    sdio_init.cis_fn0_base = sdio_cis_fn0;
    sdio_init.cis_fn1_base = sdio_cis_fn1;
    
    
    
    sdio_init.sdio_dev_csa_support = FN1_CSA_SUPPORT | FN2_CSA_SUPPORT | \
                                     FN3_CSA_SUPPORT | FN4_CSA_SUPPORT | \
                                     FN5_CSA_SUPPORT | FN6_CSA_SUPPORT | \
                                     FN7_CSA_SUPPORT; 
    
    sdio_init.sdio_dev_scsi = SDIO_CCCR_CAP_SCSI_1;
    sdio_init.sdio_dev_smb  = SDIO_CCCR_CAP_SMB_1;
    sdio_init.sdio_dev_sdc  = SDIO_CCCR_CAP_SDC_1;
    
    sdio_init.sdio_dev_srw  = SDIO_CCCR_CAP_SRW_1;
    sdio_init.sdio_dev_sbs  = SDIO_CCCR_CAP_SBS_1;
    sdio_init.sdio_dev_4bls = SDIO_CCCR_CAP_4BLS_1;
    
    sdio_init.sdio_dev_func_num = SDIO_FUNC1;
    
    hal_sdio_device_init(&sdio_init);
    
    hal_sdio_device_it_cfg(FN1_ENABLE_INTERRPT     | RESET_FN1_INTERRPT       |  \
                           FN1_WRITE_OVER_INTERRPT | FN1_READ_OVER_INTERRPT   |  \
                           READ_ERROR_FN1_INTERRPT | WRITE_ERROR_FN1_INTERRPT , HAL_ENABLE); //     //crc check 

    NVIC_EnableIRQ(SDIO_1_IRQn);
    
    NVIC_SetPriority(SDIO_1_IRQn,    1);
}

void sdio_boot_sdio_device_send_data(void)
{
    hal_sdio_device_set_send_buf_addr(tx_data);
    hal_sdio_device_set_send_buf_size(256);
    hal_sdio_device_trig_host_data1_int();
}


void ln_sdio_test()
{
    ln_sdio_init();

    for (int i = 0; i < 256; i++)
    {
        tx_data[i] = i;
    }
    
    while(1)
    {
        ln_delay_ms(1000);
    }
}




void SDIO_F1_IRQHandler(void)
{
    LOG(LOG_LVL_INFO,"it %d! \n",sdio_sdio_ahb_int_sts1_get());
    if(hal_sdio_device_it_get_flag(FN1_WRITE_OVER_INTERRPT) == HAL_SET)
    {
        unsigned int len = hal_sdio_device_get_recv_buf_size();
        unsigned char *src = (uint8_t *)hal_sdio_device_get_recv_buf_addr();
        
        if(len == 256)
        {
            // set the next receive buffer address.
            
            LOG(LOG_LVL_INFO,"recv_it! \n");
            hal_sdio_device_set_recv_buf_addr(rx_data);
            
            // send data
 
            tx_data[0] ++;
            sdio_boot_sdio_device_send_data();
        }
   
        
        //clear the receive interrupt flag and clear the sdio device busy flag.
        hal_sdio_device_it_clr_flag(FN1_WRITE_OVER_INTERRPT);
        hal_sdio_device_clear_busy();
    }
    
    if(hal_sdio_device_it_get_flag(FN1_READ_OVER_INTERRPT) == HAL_SET)
    {
        LOG(LOG_LVL_INFO,"read_it! \n");
        hal_sdio_device_it_clr_flag(FN1_READ_OVER_INTERRPT);
    }


    if(hal_sdio_device_it_get_flag(READ_ERROR_FN1_INTERRPT) == HAL_SET)
    {
        LOG(LOG_LVL_INFO,"read error it! \n");
        hal_sdio_device_it_clr_flag(READ_ERROR_FN1_INTERRPT);
    }
    
    //CRC ERROR
    if(hal_sdio_device_it_get_flag(WRITE_ERROR_FN1_INTERRPT) == HAL_SET)
    {
        LOG(LOG_LVL_INFO,"crc error it! \n");
        hal_sdio_device_it_clr_flag(WRITE_ERROR_FN1_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(WRITE_ABORT_FN1_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(WRITE_ABORT_FN1_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(RESET_FN1_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(RESET_FN1_INTERRPT);
    }
    
    //when host enable the FN1 interrupt,will receive the interrupt.
    if(hal_sdio_device_it_get_flag(FN1_ENABLE_INTERRPT) == HAL_SET)
    {
       LOG(LOG_LVL_INFO,"enable it! \n");
       hal_sdio_device_it_clr_flag(FN1_ENABLE_INTERRPT);
    }
    
    if(hal_sdio_device_it_get_flag(FN1_STATUS_PCRRT_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_STATUS_PCRRT_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(FN1_STATUS_PCWRT_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_STATUS_PCWRT_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(FN1_RTC_SET_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_RTC_SET_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(FN1_CLINTRD_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_CLINTRD_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(FN1_INT_EN_UP_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_INT_EN_UP_INTERRPT);
    }
    if(hal_sdio_device_it_get_flag(FN1_M2S_INT_INTERRPT) == HAL_SET)
    {
        hal_sdio_device_it_clr_flag(FN1_M2S_INT_INTERRPT);
    }
    
}

