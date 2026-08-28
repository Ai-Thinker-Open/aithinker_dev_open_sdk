//中间件
#include "aiio_type.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_chip_init.h"

//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"

#ifdef CONFIG_MBEDTLS_ENABLED
#include "ssl_rom_to_ram_map.h"
#endif
#if defined(CONFIG_BT_COEXIST)
#include "rtw_coex_ipc.h"
#endif

#if defined(CONFIG_FTL_ENABLED) && CONFIG_FTL_ENABLED
#include "ftl_int.h"
void app_ftl_init(void)
{
    u32 ftl_start_addr, ftl_end_addr;

    flash_get_layout_info(FTL, &ftl_start_addr, &ftl_end_addr);
    ftl_phy_page_start_addr = ftl_start_addr - SPI_FLASH_BASE;
    ftl_phy_page_num = (ftl_end_addr - ftl_start_addr + 1) / PAGE_SIZE_4K;
    ftl_init(ftl_phy_page_start_addr, ftl_phy_page_num);
}
#endif

#ifdef CONFIG_MBEDTLS_ENABLED
void app_mbedtls_rom_init(void)
{
    CRYPTO_Init(NULL);
    CRYPTO_SHA_Init(NULL);
    ssl_function_map.ssl_calloc = (void *(*)(unsigned int, unsigned int))rtos_mem_calloc;
    ssl_function_map.ssl_free = (void (*)(void *))rtos_mem_free;
    ssl_function_map.ssl_printf = (long unsigned int (*)(const char *, ...))DiagPrintf;
    ssl_function_map.ssl_snprintf = (int (*)(char *s, size_t n, const char *format, ...))DiagSnPrintf;
}
#endif

int32_t aiio_chip_init(void)
{
    InterruptRegister(IPC_INTHandler, IPC_KM4_IRQ, (u32)IPCKM4_DEV, INT_PRI5);
    InterruptEn(IPC_KM4_IRQ, INT_PRI5);

    /*IPC table initialization*/
    ipc_table_init(IPCKM4_DEV);

#ifdef CONFIG_MBEDTLS_ENABLED
    app_mbedtls_rom_init();
#endif

#if defined(CONFIG_FTL_ENABLED) && CONFIG_FTL_ENABLED
    app_ftl_init();
#endif

#if defined(CONFIG_BT_COEXIST)
    /* init coex ipc */
    coex_ipc_entry();
#endif

    return AIIO_OK;
}

char *aiio_get_chipname(void)
{
    return "RTL8711D";
}

//------------------------------------- 内部调用 ---------------------------------
/* V1.1 SDK BLE功率配置(1.1 功率小于1.0) */
const u32 array_mp_8721da_radioa_diff[] = {
    0xeb00001,
    0x3300038,
    0x3f33050,
    0x3300037,
    0x3f33050,
    0x3300036,
    0x3f33050,
    0x3300035,
    0x3f33050,
    0x3300034,
    0x3f33050,
    0x3300033,
    0x3f33050,
    0x3300032,
    0x3f33050,
    0x3300031,
    0x3f33050,
    0x3300030,
    0x3f33050,
    0x330002f,
    0x3f33050,
    0x330002e,
    0x3f33050,
    0x330002d,
    0x3f33050,
    0x330002c,
    0x3f33050,
    0x330002b,
    0x3f33051,
    0x330002a,
    0x3f33053,
    0x3300029,
    0x3f33040,
    0x3300028,
    0x3f33041,
    0x3300027,
    0x3f23050,
    0x3300026,
    0x3f23051,
    0x3300025,
    0x3f23053,
    0x3300024,
    0x3f23040,
    0x3300023,
    0x3f23041,
    0x3300022,
    0x3f23043,
    0x3300021,
    0x3f22050,
    0x3300020,
    0x3f22051,
    0x330001f,
    0x3f22053,
    0x330001e,
    0x3f22040,
    0x330001d,
    0x3f22041,
    0x330001c,
    0x3f22043,
    0x330001b,
    0x3f22030,
    0x330001a,
    0x3f22031,
    0x3300019,
    0x3f22033,
    0x3300018,
    0x3f22020,
    0x3300017,
    0x3f22021,
    0x3300016,
    0x3f22023,
    0x3300015,
    0x3f20050,
    0x3300014,
    0x3f20051,
    0x3300013,
    0x3f20053,
    0x3300012,
    0x3f20040,
    0x3300011,
    0x3f20041,
    0x3300010,
    0x3f20043,
    0x330000f,
    0x3f20030,
    0x330000e,
    0x3f20031,
    0x330000d,
    0x3f20033,
    0x330000c,
    0x3f20020,
    0x330000b,
    0x3f20021,
    0x330000a,
    0x3f20023,
    0x3300009,
    0x3f20010,
    0x3300008,
    0x3f20011,
    0x3300007,
    0x3f20013,
    0x3300006,
    0x3f20000,
    0x3300005,
    0x3f20001,
    0x3300004,
    0x3f20003,
    0x3300003,
    0x3f10010,
    0x3300002,
    0x3f10011,
    0x3300001,
    0x3f10013,
    0x3300000,
    0x3f10000,
    0xeb00000,
    0xeb00002,
    0x3300000,
    0x3e00007,
    0x3f085e4,
    0x3300001,
    0x3e00007,
    0x3f0a524,
    0x3300002,
    0x3e00007,
    0x3f085e4,
    0xeb00000,
};
u16 array_mp_8721da_radioa_diff_len = sizeof array_mp_8721da_radioa_diff / sizeof(u32);