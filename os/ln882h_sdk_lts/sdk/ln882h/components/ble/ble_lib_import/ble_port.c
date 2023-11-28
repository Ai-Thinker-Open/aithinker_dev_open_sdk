#include "ble_port.h"
#include "osal/osal.h"      // For mem

#include "rwip.h"           // For rwip_assert
#include "ble_arch/arch.h"  // For platform_reset

#include "ln_utils.h"       // For LN_UNUSED
#include "core_cm4.h"       // For NVIC


/* hardware for BLE MAC Interrupt */
void blib_ble_mac_isr_enable(void)
{
    NVIC_EnableIRQ(BLE_MAC_IRQn);
}

void blib_ble_mac_isr_disable(void)
{
    NVIC_DisableIRQ(BLE_MAC_IRQn);
}

void blib_ble_err_isr_enable(void)
{
    NVIC_EnableIRQ(BLE_ERR_IRQn);
}

void blib_ble_err_isr_disable(void)
{
    NVIC_DisableIRQ(BLE_ERR_IRQn);
}


/* heap memory manager */
void *blib_malloc(uint32_t size)
{
    return OS_Malloc(size);
}

void blib_free(void *ptr)
{
    OS_Free(ptr);
}

/* flash api adapter */
#include "hal/hal_flash.h"
void blib_flash_read(uint32_t addr, uint32_t len, uint8_t* buf)
{
//    hal_flash_read(addr, buf, len);

    addr += CACHE_FLASH_BASE;
    for(uint32_t i = 0; i < len; i++) {
        *buf++ = *((uint8_t *)(addr++));
    }
}

void blib_flash_write(uint32_t addr, uint32_t len, uint8_t* buf)
{
    hal_flash_program(addr, len, buf);
}

void blib_flash_erase(uint32_t addr, uint32_t len)
{
    hal_flash_erase(addr, len);
}


/* lib log print, hexdump */
#include "utils/debug/log.h"
void blib_log_printf(uint8_t tag_en, uint8_t level, const char *format, ...)
{
    const char *tag = NULL;
    va_list args;

    va_start(args, format);
    if (tag_en) {
        switch(level) {
            case BLIB_LOG_LVL_E:
                tag = BLIB_LOG_E_TAG;
                break;
            case BLIB_LOG_LVL_I:
                tag = BLIB_LOG_I_TAG;
                break;
            case BLIB_LOG_LVL_W:
                tag = BLIB_LOG_W_TAG;
                break;
            case BLIB_LOG_LVL_D:
                tag = BLIB_LOG_D_TAG;
                break;
            default:
                break;
        }
    }

    if (level <= BLIB_LOG_LEVEL_CFG) {
       __sprintf(tag, (stdio_write_fn)log_stdio_write, format, args);
    }
    va_end(args);
}

void blib_hexdump(const char *info, void *buf, uint32_t buf_len)
{
    hexdump(LOG_LVL_EMERG, info, buf, buf_len);
}

/**
 * @brief 
 * @param[in] type    blib_assert_type_t defined in arch.h
*/
void blib_assert(int param0, int param1, int type, const char *file, int line)
{    
    switch (type)
    {
        case ASSERT_TYPE_ERROR:
            __disable_irq();
            blib_log_printf(0, BLIB_LOG_LVL_E, "[BLIB_ASSERT]:ERROR;[%s:%d]\r\n", file, line);
            __BKPT(0);
            while(1);
        
        case ASSERT_TYPE_WARNING:
            blib_log_printf(0, BLIB_LOG_LVL_E, "[BLIB_ASSERT]:WARNING;[%s:%d]\r\n", file, line);
            break;
        case ASSERT_TYPE_INFO:        
            blib_log_printf(0, BLIB_LOG_LVL_E, "[BLIB_ASSERT]:INFO;[%s:%d]\r\n", file, line);
            break;
        default:
            blib_log_printf(0, BLIB_LOG_LVL_E, "[BLIB_ASSERT]type:%d;[%s:%d]\r\n", (int)type, file, line);
            break;
    }
}

/**
 * @brief platform_reset
 * @param[in] error defined in arch.h
 *            RESET_NO_ERROR
 *            RESET_MEM_ALLOC_FAIL
 *            RESET_TO_ROM
 *            RESET_AND_LOAD_FW
*/
void platform_reset(uint32_t error)
{
    LOG(LOG_LVL_ERROR, "[%s] error: 0x%08X !!!\r\n", __func__, error);

    // Disable interrupts
    // TODO: maybe we can't disable irq, it will affects wifi stack.
    GLOBAL_INT_STOP();
}


/**
 * For hardware lowpower
*/
void blib_cpu_sleep_voter_reg(void *vote_check, void *pre_sleep, void *post_sleep)
{
    LN_UNUSED(vote_check);
    LN_UNUSED(pre_sleep);
    LN_UNUSED(post_sleep);
}


/**
 * For ble RF modem
*/
#include "base_addr_ln882h.h"
void blib_modem_init(void)
{
    *(uint32_t*)0x4000D108 = 0x3FE7FFCF;
    *(uint32_t*)0x4000D20c = 0x10001;     //rx startup delay
    *(uint32_t*)0x4000D210 = 0x10004;
    *(uint32_t*)0x4000D368 = 0x80000300;  //sync threshold enable and set
    *(uint32_t*)0x4000D214 = 0x0;         //disable fm tx
    *(uint32_t*)0x4000D3bc = 0x41003FF;   //lr_search_08_30
    *(uint32_t*)0x4000D3c0 = 0xF7356831;
    *(uint32_t*)0x4000D3c4 = 0x1e;
    *(uint32_t*)0x4000D3cc = 0x300;

//    *(uint32_t*)0x4000D394 = 0xFDEF7FFF;  //2m_12_18
//    *(uint32_t*)0x4000D398 = 0xF6AD040F;
//    *(uint32_t*)0x4000D39c = 0x1e;
//    *(uint32_t*)0x4000D3a4 = 0x780;

    *(uint32_t*)0x4000D394 = 0x81ff7bdc;  //2m_12_18
    *(uint32_t*)0x4000D398 = 0xF7314720;
    *(uint32_t*)0x4000D39c = 0x1e;
    *(uint32_t*)0x4000D3a4 = 0x780;


    *(uint32_t*)0x4000D380 = 0x81ff7bdc;  //1m_10_18
    *(uint32_t*)0x4000D384 = 0xF7314720;
    *(uint32_t*)0x4000D388 = 0x1e;
    *(uint32_t*)0x4000D390 = 0x780;
}

void blib_rf_reg_prepare_init(void)
{
}

void blib_rf_cal(void)
{
}
