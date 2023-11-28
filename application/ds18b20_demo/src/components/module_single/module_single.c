#define __MODULE_SINGLE_C_
#include "module_single.h"

static uint8_t module_single_reset(struct module_single_type *this)
{
    uint8_t i = 0;
    this->module_single_config->module_single_config_write(0);
    for (i = 0; i < 140; i++)
    {
        // if(this->module_single_config->module_single_config_delay != null)
        // {
            this->module_single_config->module_single_config_delay();
        // }
    }
    this->module_single_config->module_single_config_write(1);
    for (i = 0; i < 24; i++)
    {
        this->module_single_config->module_single_config_delay();
    }

    if (this->module_single_config->module_single_config_read() == 0)
    {
        while (!this->module_single_config->module_single_config_read())
            ;
        for (i = 0; i < 40; i++)
        {
            this->module_single_config->module_single_config_delay();
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

static void module_single_clrbit(struct module_single_type *this)
{
    uint8_t i = 0;
    this->module_single_config->module_single_config_write(0);
    for (i = 0; i < 12; i++)
    {
        this->module_single_config->module_single_config_delay();
    }
    this->module_single_config->module_single_config_write(1);
}

static void module_single_setbit(struct module_single_type *this)
{
    uint8_t i = 0;
    this->module_single_config->module_single_config_write(0);
    for (i = 0; i < 3; i++)
    {
        this->module_single_config->module_single_config_delay();
    }
    this->module_single_config->module_single_config_write(1);
    for (i = 0; i < 9; i++)
    {
        this->module_single_config->module_single_config_delay();
    }
}

static uint8_t module_single_readbit(struct module_single_type *this)
{
    uint8_t dat = 0, i = 0;
    this->module_single_config->module_single_config_write(0);
    this->module_single_config->module_single_config_delay();
    this->module_single_config->module_single_config_write(1);
    dat = this->module_single_config->module_single_config_read();
    for (i = 0; i < 9; i++)
    {
        this->module_single_config->module_single_config_delay();
    }
    return dat;
}

static void module_single_write_bench(struct module_single_type *this, uint8_t dat)
{
    uint8_t i = 0;
    for (i = 0; i < 8; i++)
    {
        if (dat & 0x01)
        {
            module_single_setbit(this);
        }
        else
        {
            module_single_clrbit(this);
        }
        dat >>= 1;
    }
}

static void module_single_write_byte(struct module_single_type *this, module_single_dat_t *module_single_dat)
{
    module_single_write_bench(this, module_single_dat->buf[0]);
}

static void module_single_write(struct module_single_type *this, module_single_dat_t *module_single_dat)
{
    uint8_t i = 0;
    for (i = 0; i < module_single_dat->buf_len; i++)
    {
        module_single_write_bench(this, module_single_dat->buf[i]);
    }
}

static uint8_t module_single_read_bench(struct module_single_type *this)
{
    uint8_t i = 0, dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat >>= 1;
        if(module_single_readbit(this))
        {
            dat |= 0x80;
        }
        // dat |= module_single_readbit(this);
        // dat <<= 1;
    }

    return dat;
}

static void module_single_read(struct module_single_type *this, module_single_dat_t *module_single_dat)
{
    uint8_t i = 0;
    for (i = 0; i < module_single_dat->buf_len; i++)
    {
        module_single_dat->buf[i] = module_single_read_bench(this);
    }
}

static void module_single_read_byte(struct module_single_type *this, module_single_dat_t *module_single_dat)
{
    module_single_dat->buf[0] = module_single_read_bench(this);
}

uint8_t module_single_create(module_single_type_t *this, module_single_config_t *module_single_config)
{
    if (this == null)
    {
        return -1;
    }
    if (module_single_config == null)
    {
        return -1;
    }

    this->module_single_config = module_single_config;

    this->module_single_reset = module_single_reset;
    this->module_single_write = module_single_write;
    this->module_single_write_byte = module_single_write_byte;
    this->module_single_read = module_single_read;
    this->module_single_read_byte = module_single_read_byte;

    return 0;
}
