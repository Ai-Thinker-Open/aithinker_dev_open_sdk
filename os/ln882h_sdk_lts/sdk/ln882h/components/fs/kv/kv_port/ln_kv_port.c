#include "ln_kv_api.h"
#include "ln_kv_port.h"
#include "ln_kv_flash.h"
#include "ln_kv.h"
#include "hal/hal_flash.h"


static kv_flash_drv_t   flash_drv;
static kv_flash_prop_t  flash_prop;

static int flash_read(uint32_t offset, void *buf, uint32_t len)
{
#if (defined(FLASH_XIP) && (FLASH_XIP == 1))
    return hal_flash_read_by_cache(offset, len, buf);
#else
    return hal_flash_read(offset, len, buf);
#endif
}

static int flash_write(uint32_t offset, const void *buf, uint32_t len)
{
    return hal_flash_program(offset, len, (uint8_t*)buf);
}

static int flash_erase(uint32_t offset, uint32_t len)
{
    hal_flash_erase(offset, len);
    return 0;
}

kv_err_t ln_kv_port_init(uint32_t flash_start, uint32_t flash_end)
{
    flash_drv.read  = flash_read;
    flash_drv.write = flash_write;
    flash_drv.erase = flash_erase;

    flash_prop.sector_size_log2 = KV_CONFIG_FLASH_SECT_SIZE_LOG2;

    return ln_kv_init(flash_start, flash_end, &flash_drv, &flash_prop);
}
