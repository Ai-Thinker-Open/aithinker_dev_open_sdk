#ifndef __MODULE_SPI_H_
#define __MODULE_SPI_H_
#include "aiio_adapter_include.h"
#ifdef __MODULE_SPI_C_
#define MODULE_SPI_EXT
#else
#define MODULE_SPI_EXT extern
#endif

/*
模式0：CPOL=0，CPHA =0  SCK空闲为低电平，数据在SCK的上升沿被采样(提取数据)

模式1：CPOL=0，CPHA =1  SCK空闲为低电平，数据在SCK的下降沿被采样(提取数据)

模式2：CPOL=1，CPHA =0  SCK空闲为高电平，数据在SCK的下降沿被采样(提取数据)

模式3：CPOL=1，CPHA =1  SCK空闲为高电平，数据在SCK的上升沿被采样(提取数据)
*/
#ifndef null
#define null (void*)0
#endif
// 配置
typedef struct
{
    uint8_t module_spi_cpol;
    uint8_t module_spi_cpha;
    uint8_t module_spi_direction;   //0 从低位开始  1 从高位开始写
    void (*module_spi_master_config_cs)(uint8_t status);        // 1    使能   0  表示关闭
    void (*module_spi_master_config_sck)(uint8_t status);
    void (*module_spi_master_config_mosi)(uint8_t status);
    uint8_t (*module_spi_master_config_miso)(void);
    void (*module_spi_master_config_delay)(void);
} module_spi_master_config_t;

//数据类型
typedef struct
{
    uint8_t *register_addr;
    uint8_t register_addr_len;
    uint8_t *buf;
    uint16_t buf_len;
} module_spi_master_dat_t;

// 功能
typedef struct module_spi_master_type
{
    module_spi_master_config_t *module_spi_master_config;

    uint8_t (*module_spi_master_write)(struct module_spi_master_type *this, module_spi_master_dat_t  *module_spi_master_dat);
    uint8_t (*module_spi_master_read)(struct module_spi_master_type *this, module_spi_master_dat_t  *module_spi_master_dat);
} module_spi_master_type_t;

MODULE_SPI_EXT uint8_t module_spi_master_create(module_spi_master_type_t *this, module_spi_master_config_t *module_spi_master_config);

#endif
