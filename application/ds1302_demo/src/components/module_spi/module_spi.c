#define __MODULE_SPI_C_
#include "module_spi.h"

static uint8_t module_spi_master_setbit(module_spi_master_type_t *this)
{
    if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else
    {
        return -1;
    }

    return 0;
}

static uint8_t module_spi_master_clrbit(module_spi_master_type_t *this)
{
    if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_mosi(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else
    {
        return -1;
    }

    return 0;
}

static uint8_t module_spi_master_readbit(module_spi_master_type_t *this)
{
    uint8_t value = 0;
    if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        value = this->module_spi_master_config->module_spi_master_config_miso();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 0) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_delay();
        value = this->module_spi_master_config->module_spi_master_config_miso();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 0))
    {
        this->module_spi_master_config->module_spi_master_config_sck(1);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        value = this->module_spi_master_config->module_spi_master_config_miso();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else if ((this->module_spi_master_config->module_spi_cpol == 1) && (this->module_spi_master_config->module_spi_cpha == 1))
    {
        this->module_spi_master_config->module_spi_master_config_delay();
        value = this->module_spi_master_config->module_spi_master_config_miso();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(0);
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_delay();
        this->module_spi_master_config->module_spi_master_config_sck(1);
    }
    else
    {
        return -1;
    }

    return value;
}

static uint8_t module_spi_master_write_bench(module_spi_master_type_t *this, uint8_t dat)
{
    uint8_t i = 0;
    if (this->module_spi_master_config->module_spi_direction == 0)
    {
        for (i = 0; i < 8; i++)
        {
            if ((dat & 0x80) == 0x80)
            {
                module_spi_master_setbit(this);
            }
            else
            {
                module_spi_master_clrbit(this);
            }
            dat <<= 1;
        }
    }
    else if (this->module_spi_master_config->module_spi_direction == 1)
    {
        for (i = 0; i < 8; i++)
        {
            if ((dat & 0x01) == 0x01)
            {
                module_spi_master_setbit(this);
            }
            else
            {
                module_spi_master_clrbit(this);
            }
            dat >>= 1;
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

static uint8_t module_spi_master_read_bench(module_spi_master_type_t *this, uint8_t *dat)
{
    uint8_t i = 0;
    if (this->module_spi_master_config->module_spi_direction == 0)
    {
        for (i = 0; i < 8; i++)
        {
            *dat <<= 1;
            if (module_spi_master_readbit(this))
            {
                *dat |= 0x01;
            }
            else
            {
                *dat &= 0xFE;
            }
        }
    }
    else if (this->module_spi_master_config->module_spi_direction == 1)
    {
        for (i = 0; i < 8; i++)
        {
            *dat >>= 1;
            if (module_spi_master_readbit(this))
            {
                *dat |= 0x80;
            }
            else
            {
                *dat &= 0x7F;
            }
        }
    }
    else
    {
        return -1;
    }

    return 0;
}

static uint8_t module_spi_master_write(module_spi_master_type_t *this, module_spi_master_dat_t *module_spi_master_dat)
{
    uint8_t i = 0;
    this->module_spi_master_config->module_spi_master_config_cs(1);
    this->module_spi_master_config->module_spi_master_config_delay();
    for (i = 0; i < module_spi_master_dat->register_addr_len; i++)
    {
        module_spi_master_write_bench(this, module_spi_master_dat->register_addr[i]);
    }

    for (i = 0; i < module_spi_master_dat->buf_len; i++)
    {
        module_spi_master_write_bench(this, module_spi_master_dat->buf[i]);
    }
    this->module_spi_master_config->module_spi_master_config_delay();
    this->module_spi_master_config->module_spi_master_config_cs(0);
}

static uint8_t module_spi_master_read(module_spi_master_type_t *this, module_spi_master_dat_t *module_spi_master_dat)
{
    uint8_t i = 0;
    this->module_spi_master_config->module_spi_master_config_cs(1);
    this->module_spi_master_config->module_spi_master_config_delay();
    for (i = 0; i < module_spi_master_dat->register_addr_len; i++)
    {
        module_spi_master_write_bench(this, module_spi_master_dat->register_addr[i]);
    }

    for (i = 0; i < module_spi_master_dat->buf_len; i++)
    {
        module_spi_master_read_bench(this, &module_spi_master_dat->buf[i]);
    }
    this->module_spi_master_config->module_spi_master_config_delay();
    this->module_spi_master_config->module_spi_master_config_cs(0);
}

uint8_t module_spi_master_create(module_spi_master_type_t *this, module_spi_master_config_t *module_spi_master_config)
{
    if (this == null)
    {
        return -1;
    }
    if (module_spi_master_config == null)
    {
        return -1;
    }

    this->module_spi_master_config = module_spi_master_config;

    this->module_spi_master_read = module_spi_master_read;
    this->module_spi_master_write = module_spi_master_write;

    return 0;
}
