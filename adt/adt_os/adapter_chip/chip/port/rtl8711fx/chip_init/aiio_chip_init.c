//中间件
#include "aiio_type.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_chip_init.h"

//SDK相关
#include "ameba_soc.h"
#if (defined CONFIG_WHC_HOST || defined CONFIG_WHC_NONE || defined CONFIG_WHC_WPA_SUPPLICANT_OFFLOAD)
#include "vfs.h"
#endif
#include "os_wrapper.h"
#include "ssl_rom_to_ram_map.h"
#ifdef CONFIG_MBEDTLS_ENABLED
#include "threading_alt.h"
#endif
#if defined(CONFIG_BT_COEXIST)
#include "rtw_coex_ipc.h"
#endif
#include "ameba_diagnose.h"


//u32 use_hw_crypto_func;

#if (defined(CONFIG_BT) && CONFIG_BT) && (defined(CONFIG_BT_INIC) && CONFIG_BT_INIC)
#include "bt_inic.h"
#endif


#ifdef CONFIG_MBEDTLS_ENABLED
void app_mbedtls_rom_init(void)
{
    ssl_function_map.ssl_calloc = (void *(*)(unsigned int, unsigned int))rtos_mem_calloc;
	ssl_function_map.ssl_free = (void (*)(void *))rtos_mem_free;
	ssl_function_map.ssl_printf = (long unsigned int (*)(const char *, ...))DiagPrintf;
	ssl_function_map.ssl_snprintf = (int (*)(char *s, size_t n, const char *format, ...))DiagSnPrintf;
#if defined(CONFIG_MBEDTLS_THREADING)
	mbedtls_threading_init();
#endif
}
#endif
void CPU1_WDG_RST_Handler(void)
{
	/* Let NP run */
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_CFG, HAL_READ32(SYSTEM_CTRL_BASE, REG_LSYS_BOOT_CFG) | LSYS_BIT_BOOT_CPU1_RUN);

	/* clear CPU1_WDG_RST intr*/
	HAL_WRITE32(SYSTEM_CTRL_BASE, REG_AON_BOOT_REASON_HW, AON_BIT_RSTF_WDG0_CPU);
}

#ifdef CONFIG_VFS_ENABLED
extern uint32_t vfs_ftl_init(void);
extern int vfs_kv_init(void);
void app_filesystem_init(void)
{
	int ret = 0;
	vfs_init();

	vfs_user_register(VFS_PREFIX, VFS_LITTLEFS, VFS_INF_FLASH, VFS_REGION_1, VFS_RW);
	ret = vfs_kv_init();
	if (ret == 0) {
		aiio_log_i("File System Init Success \n");
	} else {
		aiio_log_i("File System Init Fail \n");
	}

#ifdef CONFIG_FATFS_WITHIN_APP_IMG
	ret = vfs_user_register(VFS_R3_PREFIX, VFS_FATFS, VFS_INF_FLASH, VFS_REGION_3, VFS_RO);
	if (ret == 0) {
		aiio_log_i("VFS-FAT Init Success \n");
	} else {
		aiio_log_i("VFS-FAT Init Fail \n");
	}
#endif

#if defined(CONFIG_FTL_ENABLED) && CONFIG_FTL_ENABLED
	vfs_ftl_init();
#endif
}
#endif


int32_t aiio_chip_init(void)
{
    ipc_table_init(IPCAP_DEV);
	InterruptRegister(IPC_INTHandler, IPC_CPU0_IRQ, (u32)IPCAP_DEV, INT_PRI5);
	InterruptEn(IPC_CPU0_IRQ, INT_PRI5);

#ifdef CONFIG_MBEDTLS_ENABLED
    app_mbedtls_rom_init();
#endif

#ifdef CONFIG_VFS_ENABLED
	app_filesystem_init();
#endif

#if defined(CONFIG_BT_COEXIST)
	/* init coex ipc */
	coex_ipc_entry();
#endif
	
    /* Register CPU1_WDG_RST_IRQ Callback function */
	InterruptRegister((IRQ_FUN) CPU1_WDG_RST_Handler, CPU1_WDG_RST_IRQ, (u32)NULL, INT_PRI_LOWEST);
	InterruptEn(CPU1_WDG_RST_IRQ, INT_PRI_LOWEST);

	rtk_diag_init(RTK_DIAG_HEAP_SIZE, RTK_DIAG_SEND_BUFFER_SIZE);
    /* Set delay function & critical function for hw ipc sema */
	IPC_patch_function(&rtos_critical_enter, &rtos_critical_exit);
	IPC_SEMDelayStub(rtos_time_delay_ms);
    return AIIO_OK;
}

char *aiio_get_chipname(void)
{
    return "RTL8711F";
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