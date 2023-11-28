#include "ota_port.h"
#include "ota_err.h"
#include "hal/hal_flash.h"
#include "ln_nvds.h"
#include "ln882h.h" 

static ota_port_ctx_t ota_port;

static int flash_read(uint32_t offset, void *buf, uint32_t len)
{
    return hal_flash_read(offset, len, buf);
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

static int ota_upg_state_set(upg_state_t state)
{
    if (NVDS_ERR_OK == ln_nvds_set_ota_upg_state(state)) {
        return LN_TRUE;
    }
    return LN_FALSE;
}

static int ota_upg_state_get(upg_state_t *state)
{
    if (NVDS_ERR_OK == ln_nvds_get_ota_upg_state((uint32_t *)state)) {
        return LN_TRUE;
    }
    return LN_FALSE;
}

ota_err_t ota_port_init(void)
{
    ota_port.flash_drv.read  = flash_read;
    ota_port.flash_drv.write = flash_write;
    ota_port.flash_drv.erase = flash_erase;
    
    ota_port.upg_state_set   = ota_upg_state_set;
    ota_port.upg_state_get   = ota_upg_state_get;

    return OTA_ERR_NONE;
}

ota_port_ctx_t * ota_get_port_ctx(void)
{
    return &ota_port;
}









