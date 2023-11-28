/**
 * @file     ln_drv_spi_dma.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_spi_dma.h"

#define TIMEOUT_CYCLE 1000

uint32_t spi_cs_base = 0;
gpio_pin_t spi_cs_pin;

#define LN_SPI_CS_LOW               hal_gpio_pin_reset(spi_cs_base,spi_cs_pin)
#define LN_SPI_CS_HIGH              hal_gpio_pin_set(spi_cs_base,spi_cs_pin)

/**
 * @brief SPI主机模式初始化
 * 
 * @param spi_init_cfg 配置SPI引脚
 */
void spi_dma_master_init(spi_pin_cfg_t *spi_init_cfg)
{
    uint32_t gpio_base = 0;
    if(spi_init_cfg->gpio_clk_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(spi_init_cfg->gpio_clk_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base,spi_init_cfg->gpio_clk_pin,SPI0_CLK);
    hal_gpio_pin_afio_en(gpio_base,spi_init_cfg->gpio_clk_pin,HAL_ENABLE);

    if(spi_init_cfg->gpio_mosi_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(spi_init_cfg->gpio_mosi_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base,spi_init_cfg->gpio_mosi_pin,SPI0_MOSI);
    hal_gpio_pin_afio_en(gpio_base,spi_init_cfg->gpio_mosi_pin,HAL_ENABLE);

    if(spi_init_cfg->gpio_miso_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(spi_init_cfg->gpio_miso_port == GPIO_B)
        gpio_base = GPIOB_BASE;

    hal_gpio_pin_afio_select(gpio_base,spi_init_cfg->gpio_miso_pin,SPI0_MISO);
    hal_gpio_pin_afio_en(gpio_base,spi_init_cfg->gpio_miso_pin,HAL_ENABLE);

    if(spi_init_cfg->gpio_cs_port == GPIO_A)
        gpio_base = GPIOA_BASE;
    else if(spi_init_cfg->gpio_cs_port == GPIO_B)
        gpio_base = GPIOB_BASE;
    
    spi_cs_base = gpio_base;
    spi_cs_pin  = spi_init_cfg->gpio_cs_pin;
    
    gpio_init_t_def gpio_init;
    memset(&gpio_init,0,sizeof(gpio_init));
    gpio_init.dir = GPIO_OUTPUT;
    gpio_init.pin = spi_cs_pin;
    gpio_init.speed = GPIO_HIGH_SPEED;
    hal_gpio_init(spi_cs_base,&gpio_init);
    hal_gpio_pin_set(spi_cs_base,spi_cs_pin);

    spi_init_type_def spi_init;
    memset(&spi_init,0,sizeof(spi_init));

    spi_init.spi_baud_rate_prescaler = SPI_BAUDRATEPRESCALER_2;         //设置波特率
    spi_init.spi_mode      = SPI_MODE_MASTER;                           //设置主从模式
    spi_init.spi_data_size = SPI_DATASIZE_8B;                           //设置数据大小
    spi_init.spi_first_bit = SPI_FIRST_BIT_MSB;                         //设置帧格式
    spi_init.spi_cpol      = SPI_CPOL_LOW;                              //设置时钟极性
    spi_init.spi_cpha      = SPI_CPHA_1EDGE;                            //设置时钟相位
    hal_spi_init(SPI0_BASE,&spi_init);                                  //初始化SPI
    
    hal_spi_en(SPI0_BASE,HAL_ENABLE);                                   //SPI使能
    hal_spi_ssoe_en(SPI0_BASE,HAL_DISABLE);                             //关闭CS OUTPUT

    /* init the spi rx dma */
    dma_init_t_def dma_init;
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)0;      //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_P;             //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    hal_dma_init(DMA_CH_3,&dma_init);               //初始化DMA
    
    //hal_dma_it_cfg(DMA_CH_3,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);        //配置DMA中断
 
    hal_dma_en(DMA_CH_3,HAL_DISABLE);                                   //DMA使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_RX_EN,HAL_ENABLE);                 //SPI DMA RX 使能
    
    /* init the spi tx dma */
    memset(&dma_init,0,sizeof(dma_init));
    
    dma_init.dma_mem_addr = (uint32_t)0;      //设置DMA内存地址
    dma_init.dma_data_num = 0;                      //设置DMA传输次数
    dma_init.dma_dir = DMA_READ_FORM_MEM;           //设置DMA方向
    dma_init.dma_mem_inc_en = DMA_MEM_INC_EN;       //设置DMA内存是否增长
    dma_init.dma_p_addr = SPI0_DATA_REG;            //设置DMA外设地址
    hal_dma_init(DMA_CH_4,&dma_init);               //初始化DMA
    
    //hal_dma_it_cfg(DMA_CH_4,DMA_IT_FLAG_TRAN_COMP,HAL_ENABLE);        //配置DMA中断
    
    hal_dma_en(DMA_CH_4,HAL_DISABLE);                                   //DMA使能
    hal_spi_dma_en(SPI0_BASE,SPI_DMA_TX_EN,HAL_ENABLE);                 //SPI DMA TX 使能
}

/**
 * @brief SPI 读写数据
 * 
 * @param send_data 写数据内容
 * @param rec_data  读数据内容
 * @param data_len  数据长度
 */
void spi_dma_master_write_and_read_data(uint8_t *send_data,uint8_t *rec_data,uint32_t data_len)
{
    //开始传输，先拉低CS。
    LN_SPI_CS_LOW;
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)send_data);
    hal_dma_set_data_num(DMA_CH_3,data_len);
    hal_dma_set_data_num(DMA_CH_4,data_len);
    hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
    hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
    
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
   
    //传输结束，拉高CS
    LN_SPI_CS_HIGH;
    
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);
}

/**
 * @brief SPI主机模式下带地址写入数据
 * 
 * @param addr       地址
 * @param addr_len   地址长度
 * @param send_data  要写入的数据
 * @param data_len   要写入的数据长度
 */
void spi_dma_master_write_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *send_data,uint32_t data_len)
{
    uint32_t rec_data_buf = 0;
    //开始传输，先拉低CS。
    LN_SPI_CS_LOW;
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)&rec_data_buf);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)addr);
    hal_dma_set_data_num(DMA_CH_3,addr_len);
    hal_dma_set_data_num(DMA_CH_4,addr_len);
    hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_DIS);
    hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
    
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
    
            
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)&rec_data_buf);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)send_data);
    hal_dma_set_data_num(DMA_CH_3,data_len);
    hal_dma_set_data_num(DMA_CH_4,data_len);
    hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_DIS);
    hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
    
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
    
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);        
   
    //传输结束，拉高CS
    LN_SPI_CS_HIGH;
}

/**
 * @brief SPI主机模式带地址读取数据
 * 
 * @param addr      地址
 * @param addr_len  地址长度
 * @param rec_data  接收数据指针
 * @param data_len  要接收的数据长度
 */
void spi_dma_master_read_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *rec_data,uint32_t data_len)
{
    //开始传输，先拉低CS。
    LN_SPI_CS_LOW;
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)addr);
    hal_dma_set_data_num(DMA_CH_3,addr_len);
    hal_dma_set_data_num(DMA_CH_4,addr_len);
    hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
    hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
    
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
    
            
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);
    
    //配置DMA传输参数。
    hal_dma_set_mem_addr(DMA_CH_3,(uint32_t)rec_data);
    hal_dma_set_mem_addr(DMA_CH_4,(uint32_t)rec_data);
    hal_dma_set_data_num(DMA_CH_3,data_len);
    hal_dma_set_data_num(DMA_CH_4,data_len);
    hal_dma_set_mem_inc_en(DMA_CH_3,DMA_MEM_INC_EN);
    hal_dma_set_mem_inc_en(DMA_CH_4,DMA_MEM_INC_EN);
    //开始传输。
    hal_dma_en(DMA_CH_3,HAL_ENABLE);
    hal_dma_en(DMA_CH_4,HAL_ENABLE);
    
    //等待传输完成。
    while(hal_dma_get_data_num(DMA_CH_3) != 0 || hal_dma_get_data_num(DMA_CH_4) != 0);
    hal_spi_wait_bus_idle(SPI0_BASE,10000);
    
    //发送完成后及时关闭DMA，为下次配置DMA参数做准备。
    hal_dma_en(DMA_CH_3,HAL_DISABLE);
    hal_dma_en(DMA_CH_4,HAL_DISABLE);        
   
    //传输结束，拉高CS
    LN_SPI_CS_HIGH;
}


