/**
 * @file     ln_spi_slave_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-19
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
 
/**
        SPI从机调试说明：
        
                    1. 接线说明：
                                LN882H(从机)    STM32(主机)
                                PA10    ->      PA4     -> SPI_CS
                                PA11    ->      PA5     -> SPI_SCK
                                PA12    ->      PA6     -> SPI_MISO
                                PA13    ->      PA7     -> SPI_MOSI
                                GND     ->      GND

                    2. CS引脚由SPI_CR1中的NSS和SSM控制，对应控制函数为：
                    
                                SSOE -> hal_spi_ssoe_en();            
                                SSM  -> hal_spi_set_nss(); 
                                
                       通过这两个函数可以设置CS引脚状态：
                        
                                （1） SSM = 1：软件管理CS引脚，从器件的选择信息由SPI_CR1寄存器中得SSI位的值驱动。外部CS引脚可供他用，这个模式只适合从模式下使用。
                                
                                （2） SSM = 0: 硬件管理CS引脚，根据SSOE的值，可以分为两种情况：（不推荐使用a模式，自行使用标准GPIO驱动SPI会使软件更灵活）
                                        
                                        a.  CS输出使能（SSOE = 1），主模式下才能使用此功能，当主机开始工作时，会把CS拉低，停止工作后恢复CS。
                                          
                                        b.  CS输出失能（SSOE = 0），从模式下使用该模式，会在CS为低电平时选中该从器件，CS为高电平时，取消从器件的片选。     
                       

                    3. LN882H SPI SLAVE + DMA  可能会出现中断进不了的问题，在中断中，使能DMA之前加个读SPI_DR的指令，可能会解决这个问题。（非必现）



*/
#include "ln_spi_slave_test.h"
#include "hal/hal_spi.h"
#include "hal/hal_gpio.h"
#include "hal/hal_dma.h"
#include "utils/debug/log.h"


#define LN_SPI_TEST_CS_PORT_BASE GPIOB_BASE
#define LN_SPI_TEST_CS_PIN       GPIO_PIN_5

#define LN_SPI_CS_LOW       hal_gpio_pin_reset(LN_SPI_TEST_CS_PORT_BASE,LN_SPI_TEST_CS_PIN)
#define LN_SPI_CS_HIGH      hal_gpio_pin_set(LN_SPI_TEST_CS_PORT_BASE,LN_SPI_TEST_CS_PIN)

static unsigned char tx_data[100];
static unsigned char rx_data_1[100];
static unsigned char rx_data_2[100];

static volatile unsigned int         data_num = 100;     //DMA传输次数
static unsigned char        data_sel = 0;       //双缓冲BUFFER选择
static unsigned char        data_comp= 0;       //数据接收完成标志位

void ln_spi_slave_init()
{
    /* 配置GPIO引脚 */
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_5,SPI0_CSN);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_6,SPI0_CLK);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_7,SPI0_MISO);
    hal_gpio_pin_afio_select(GPIOB_BASE,GPIO_PIN_8,SPI0_MOSI);
    
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_5,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_6,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_7,HAL_ENABLE);
    hal_gpio_pin_afio_en(GPIOB_BASE,GPIO_PIN_8,HAL_ENABLE);

    /* 配置SPI */
    spi_init_type_def spi_init;
    memset(&spi_init,0,sizeof(spi_init));
    spi_init.spi_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2; //配置分频，SLAVE模式无效果
    spi_init.spi_mode = SPI_MODE_SLAVE;                         //配置主从模式
    spi_init.spi_data_size = SPI_DATASIZE_8B;                   //配置数据位宽
    spi_init.spi_first_bit = SPI_FIRST_BIT_LSB;                 //配置最先发送的bit位置
    spi_init.spi_cpha = SPI_CPHA_1EDGE;                         //配置时钟相位
    spi_init.spi_cpol = SPI_CPOL_LOW;                           //配置时钟相位
    hal_spi_init(SPI0_BASE,&spi_init);                          //初始化SPI
    
    hal_spi_ssoe_en(SPI0_BASE,HAL_DISABLE);                     //关闭CS引脚输出
    hal_spi_set_nss(SPI0_BASE,SPI_NSS_HARD);                    //配置CS引脚为硬件模式
    
    //hal_spi_it_cfg(SPI0_BASE,SPI_IT_FLAG_OVR,HAL_ENABLE);     //配置SPI中断
}

void ln_spi_slave_dma_init(void)
{
    /* init the spi rx dma */
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));
    
    hal_dma_en(DMA_CH_3,HAL_DISABLE);                               //失能DMA（为了配置参数）
    dma_init.dma_mem_addr = (uint32_t)rx_data_1;                    //配置DMA内存地址
    dma_init.dma_data_num = 100;                                     //配置DMA传输数量
    dma_init.dma_dir = DMA_READ_FORM_P;                             //配置DMA传输方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;                       //配置DMA内存是否自增
    dma_init.dma_p_addr = SPI0_DATA_REG;                             //配置DMA外设地址
    hal_dma_init(DMA_CH_3,&dma_init);                               //初始化DMA
    hal_dma_it_cfg(DMA_CH_3,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);      //配置DMA传输完成中断
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    
    
    /* init the spi tx dma */
    memset(&dma_init,0,sizeof(dma_init));
    
    hal_dma_en(DMA_CH_4,HAL_DISABLE);                               //失能DMA（为了配置参数）
    dma_init.dma_mem_addr = (uint32_t)tx_data;                      //配置DMA内存地址
    dma_init.dma_data_num = 100;                                     //配置DMA传输数量
    dma_init.dma_dir = DMA_READ_FORM_MEM;                           //配置DMA传输方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;                       //配置DMA内存是否自增
    dma_init.dma_p_addr = SPI0_DATA_REG;                            //配置DMA外设地址
    dma_init.dma_circ_mode_en = DMA_CIRC_MODE_EN;                   //使能DMA循环发送
    hal_dma_init(DMA_CH_4,&dma_init);                               //初始化DMA
    //hal_dma_it_cfg(DMA_CH_4,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);      //配置DMA传输完成中断
    
    
    //hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);             //使能SPI TX DMA传输
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_RX_EN,HAL_ENABLE);             //使能SPI RX DMA传输
    
    //hal_dma_en(DMA_CH_4,HAL_ENABLE);                                //使能DMA
    hal_dma_en(DMA_CH_3,HAL_ENABLE);                                //使能DMA
    
    hal_spi_en(SPI0_BASE,HAL_ENABLE);                               //使能SPI
}

volatile unsigned int err_cnt_1 = 0;

void ln_spi_slave_test()
{
    ln_spi_slave_init();
    ln_spi_slave_dma_init();
    NVIC_SetPriority(DMA_IRQn,     4);                         //配置DMA中断优先级
    NVIC_EnableIRQ(DMA_IRQn);                                  //使能DMA中断   
    //NVIC_SetPriority(SPI0_IRQn,     4);                         //配置SPI中断优先级
    //NVIC_EnableIRQ(SPI0_IRQn);                                  //使能SPI中断

    data_sel = 0;

    while(1)
    {
        if(data_comp == 1)
        {
            if(data_sel == 1)
            {
                data_comp = 0;
                for(int i = 0; i < 99; i ++)
                {
                    if((unsigned char)(rx_data_1[i] + 2) != rx_data_1[i+1])
                    {
                        err_cnt_1++;
                    }
                }
            }
            else if(data_sel == 2)
            {
                data_comp = 0;
                for(int i = 0; i < 99; i ++)
                {
                    if((unsigned char)(rx_data_2[i] + 2) != rx_data_2[i+1])
                    {
                        err_cnt_1++;
                    }
                }
            }
        }
    }

}
void SPI0_IRQHandler()
{
    if(hal_spi_get_it_flag(SPI0_BASE,SPI_IT_FLAG_OVR) == HAL_SET)
    {
        hal_spi_recv_data(SPI0_BASE);
    }
}
static volatile unsigned int int_cnt_1 = 0;      //中断计数器，调试使用
static volatile unsigned int int_cnt_2 = 0;      //中断计数器，调试使用


//void DMA_IRQHandler()
//{
//    if(hal_dma_get_it_flag(DMA_CH_3,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)   
//      /* 数据接收 */
//    {
//        //ln_delay_ms(10);
//        if(data_sel == 1)
//        {
//            int_cnt_1++;
//            data_sel = 2;
//            hal_dma_en(DMA_CH_3,HAL_DISABLE);                                 //失能DMA（为了配置参数）
//            hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)(rx_data_1));             //重新设置DMA地址
//            hal_dma_set_data_num(DMA_CH_3,100);                               //设置DMA数据长度
//            hal_dma_en(DMA_CH_3,HAL_ENABLE);                                  //设置DMA使能
//        }
//        else
//        {
//            int_cnt_2++;
//            data_sel = 1;
//            hal_dma_en(DMA_CH_3,HAL_DISABLE);                                 //失能DMA（为了配置参数）
//            hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)(rx_data_2));             //重新设置DMA地址
//            hal_dma_set_data_num(DMA_CH_3,100);                               //设置DMA数据长度
//            hal_dma_en(DMA_CH_3,HAL_ENABLE);                                  //设置DMA使能
//        }
//        //hal_dma_clr_it_flag(DMA_CH_3,DMA_IT_FLAG_TRAN_COMP);
//        data_comp = 1; 
//    }
//    
////    if(hal_dma_get_it_flag(DMA_CH_4,DMA_IT_FLAG_TRAN_COMP) == HAL_SET)
////      /* 数据发送 */
////    {
////        hal_dma_en(DMA_CH_4,HAL_DISABLE);                                     //失能DMA（为了配置参数）
////        hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)(tx_data));                   //重新设置DMA地址
////        hal_dma_set_data_num(DMA_CH_4,100);                                   //设置DMA数据长度
////        hal_dma_en(DMA_CH_4,HAL_ENABLE);                                      //设置DMA使能
////        //hal_dma_clr_it_flag(DMA_CH_4,DMA_IT_FLAG_TRAN_COMP);        
////    }
//}
