/**
 * @file     ln_drv_spi.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-04
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "ln_drv_spi.h"

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
void spi_master_init(spi_pin_cfg_t *spi_init_cfg)
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
}

/**
 * @brief SPI 读写数据
 * 
 * @param send_data 写数据内容
 * @param rec_data  读数据内容
 * @param data_len  数据长度
 */
void spi_master_write_and_read_data(uint8_t *send_data,uint8_t *rec_data,uint32_t data_len)
{
    LN_SPI_CS_LOW;
    for(int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        
        if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
            hal_spi_send_data(SPI0_BASE,send_data[i]);
        if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
            rec_data[i] = hal_spi_recv_data(SPI0_BASE);
        
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    }
    LN_SPI_CS_HIGH;
}

/**
 * @brief SPI主机模式下带地址写入数据
 * 
 * @param addr       地址
 * @param addr_len   地址长度
 * @param send_data  要写入的数据
 * @param data_len   要写入的数据长度
 */
void spi_master_write_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *send_data,uint32_t data_len)
{
    volatile uint8_t rec_data_buffer[255];
    
    LN_SPI_CS_LOW;

    for(int i = 0; i < addr_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        
        if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
            hal_spi_send_data(SPI0_BASE,addr[i]);
        if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
            rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);
        
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    }
    for(int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        
        if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
            hal_spi_send_data(SPI0_BASE,send_data[i]);
        if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
            rec_data_buffer[0] = hal_spi_recv_data(SPI0_BASE);
        
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    }
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
void spi_master_read_data_with_addr(uint8_t *addr,uint32_t addr_len,uint8_t *rec_data,uint32_t data_len)
{
    volatile uint8_t buffer[255];
    LN_SPI_CS_LOW;
    for(int i = 0; i < addr_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        
        if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
            hal_spi_send_data(SPI0_BASE,addr[i]);
        if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
            buffer[0] = hal_spi_recv_data(SPI0_BASE);
        
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    }
    for(int i = 0; i < data_len; i++)
    {
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
        
        if(hal_spi_wait_txe(SPI0_BASE,1000000) == 1)
            hal_spi_send_data(SPI0_BASE,0xFF);
        if(hal_spi_wait_rxne(SPI0_BASE,1000000) == 1)
            rec_data[i] = hal_spi_recv_data(SPI0_BASE);
        
        hal_spi_wait_bus_idle(SPI0_BASE,1000000);
    }
    LN_SPI_CS_HIGH;
}


