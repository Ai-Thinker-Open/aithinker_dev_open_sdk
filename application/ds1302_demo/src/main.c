#include "module_spi.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

module_spi_master_type_t module_spi_master;
module_spi_master_config_t module_spi_master_config;
module_spi_master_dat_t module_spi_master_dat;

void module_spi_master_config_cs(uint8_t status) // 1    使能   0  表示关闭
{
}

void module_spi_master_config_sck(uint8_t status)
{
}

void module_spi_master_config_mosi(uint8_t status)
{
}

uint8_t module_spi_master_config_miso(void)
{

    return 0;
}

void module_spi_master_config_delay(void)
{
}

void aiio_main(void *params)
{
    module_spi_master_config.module_spi_cpol = 0;
    module_spi_master_config.module_spi_cpha = 1;
    module_spi_master_config.module_spi_direction = 1;
    module_spi_master_config.module_spi_master_config_cs = module_spi_master_config_cs;
    module_spi_master_config.module_spi_master_config_sck = module_spi_master_config_sck;
    module_spi_master_config.module_spi_master_config_mosi = module_spi_master_config_mosi;
    module_spi_master_config.module_spi_master_config_miso = module_spi_master_config_miso;
    module_spi_master_config.module_spi_master_config_delay = module_spi_master_config_delay;

    module_spi_master_create(&module_spi_master, &module_spi_master_config);
    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
