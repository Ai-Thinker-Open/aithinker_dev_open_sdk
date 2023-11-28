#include <assert.h>
#include "bflb_core.h"
// #include <wl_api.h>
#include <bl616_pm.h>
#include <bflb_rtc.h>
#include <bl_lp.h>
#include <bl616_pds.h>
#include "hardware/timer_reg.h"
#include <bl616_glb_gpio.h>
#include <bflb_dma.h>
#include <bl616_aon.h>
#include <bflb_acomp.h>
#include <bflb_flash.h>
#include <bflb_sflash.h>
#include <bflb_sf_ctrl.h>
#include "sf_ctrl_reg.h"
#include <bl616_l1c.h>
#include <bl616_clock.h>
#include <bl616_tzc_sec.h>
// #include <utils_crc.h>
#include "bflb_sec_sha.h"
// #include <bl_gpio.h>

#ifdef CONF_PSRAM_RESTORE
#include <bl616_psram.h>
#endif

#ifndef BL_WIFI_LP_FW
#include "wifi_mgmr_ext.h"
#endif

// #include "time_statics.h"

#define LP_HOOK(x, ...)           \
    if (&lp_hook_##x) {           \
        lp_hook_##x(__VA_ARGS__); \
    }

#define WL_API_RMEM_ADDR      0x20010600
#define LP_FW_START_ADDR      0x63026800
#define LP_FW_PRE_JUMP_ADDR   0x20010000
#define LP_FW_MAX_SZIE        38 * 1024
#define BL616_UART_TX         21
#define BL616_UART_RX         22
#define BL616_UART_ID         0
#define BL616_UART_FREQ       40000000
#define UART_BAUDRATE         (2000000)

#define BL616_ACOMP_VREF_1V65 33

#if 1
#define BL_LP_LOG    printf
#define BL_LP_LOG_EN 0
#else
#define BL_LP_LOG(...)
#define BL_LP_LOG_EN 0
#endif

extern unsigned char __lpfw_start[];
// extern void CPU_Interrupt_Enable(uint32_t irq_num);
// extern void CPU_Interrupt_Disable(uint32_t irq_num);
static void bl_lp_xip_para_save(void);

bl_lp_fw_cfg_t lpfw_cfg = {
    .tim_wakeup_en = 1,
    .rtc_timeout_us = (60 * 1000 * 1000),
    .tpre = 0,
    .dtim_origin = BL_DTIM_NUM,
    .dtim_num = 0,
};

struct lp_env {
    void *sys_enter_arg;
    void *user_enter_arg;
    void *sys_exit_arg;
    void *user_exit_arg;

    bl_lp_cb_t sys_pre_enter_callback;
    bl_lp_cb_t user_pre_enter_callback;
    bl_lp_cb_t sys_after_exit_callback;
    bl_lp_cb_t user_after_exit_callback;

    uint8_t wifi_hw_resume;
    uint8_t wifi_fw_ready;
    uint32_t gpio_stat;
};

static struct lp_env *gp_lp_env = NULL;

static void bl_lp_vtime_before_sleep(void);
static void bl_lp_vtime_after_sleep(void);

static uint64_t g_rtc_timestamp_before_sleep_us = 0;    /* rtc 1 */
static uint64_t g_rtc_timestamp_after_sleep_us = 0;     /* rtc 2 */
static uint64_t g_mtimer_timestamp_before_sleep_us = 0; /* mtime 1 */
static uint64_t g_mtimer_timestamp_after_sleep_us = 0;  /* mtime 2 */
static uint64_t g_virtual_timestamp_base_us = 0;        /* virtual time base */

static void bl_lp_io_wakeup_init(bl_lp_io_cfg_t *io_wakeup_cfg);
static void bl_lp_acomp_wakeup_init(bl_lp_acomp_cfg_t *acomp_wakeup_cfg);

static bl_lp_io_cfg_t *gp_lp_io_cfg = NULL;
static bl_lp_io_cfg_t g_lp_io_cfg_bak;
static bl_lp_acomp_cfg_t *gp_lp_acomp_cfg = NULL;
static bl_lp_acomp_cfg_t g_lp_acomp_cfg_bak;
static void bl_lp_soft_irq(void);
static bl_lp_soft_irq_callback_t lp_soft_callback = { NULL };

// #if (PM_PDS_LDO_LEVEL_DEFAULT != 0)
// const uint32_t hbn_ucode[] = {
//     0x2000f737, 0x06375b1c, 0xe7934000, 0xdb1c0017, 0x200097b7, 0xf6934bd4, 0xcbd4c006, 0xe6934bd4,
//     0xcbd40276, 0x8ed14bd4, 0x4bd4cbd4, 0xc0000637, 0x8ef1167d, 0x5b1ccbd4, 0x00f106b7, 0x8ff516fd,
//     0xaa0a06b7, 0xdb1c8fd5, 0x200057b7, 0x063743b4, 0x8ed10040, 0x43b4c3b4, 0x00800637, 0xc3b48ed1,
//     0x10072223, 0x630277b7, 0x10072023, 0x80078793, 0x00008782, 0x00000000, 0x00000000, 0x00000000
// };
// #else
const uint32_t hbn_ucode[] = {
    0x2000f737, 0x06375b1c, 0xe7934000, 0xdb1c0017, 0x200097b7, 0xf6934bd4, 0xcbd4c006, 0xe6934bd4,
    0xcbd40276, 0x8ed14bd4, 0x4bd4cbd4, 0xc0000637, 0x8ef1167d, 0x57b7cbd4, 0x43b42000, 0x00400637,
    0xc3b48ed1, 0x063743b4, 0x8ed10080, 0x2223c3b4, 0x77b71007, 0x20236302, 0x87931007, 0x87828007
};
// #endif

#define GET_OFFSET(_type, _member) ((unsigned long)(&((_type *)0)->_member))

void lp_fw_save_cpu_para(uint32_t save_addr)
{
    __asm__ __volatile__(
        "sw      x0, 0(a0)\n\t"
        "sw      x1, 4(a0)\n\t"
        "sw      x2, 8(a0)\n\t"
        "sw      x3, 12(a0)\n\t"
        "sw      x4, 16(a0)\n\t"
        "sw      x5, 20(a0)\n\t"
        "sw      x6, 24(a0)\n\t"
        "sw      x7, 28(a0)\n\t"
        "sw      x8, 32(a0)\n\t"
        "sw      x9, 36(a0)\n\t"
        "sw      x10, 40(a0)\n\t"
        "sw      x11, 44(a0)\n\t"
        "sw      x12, 48(a0)\n\t"
        "sw      x13, 52(a0)\n\t"
        "sw      x14, 56(a0)\n\t"
        "sw      x15, 60(a0)\n\t"
        "sw      x16, 64(a0)\n\t"
        "sw      x17, 68(a0)\n\t"
        "sw      x18, 72(a0)\n\t"
        "sw      x19, 76(a0)\n\t"
        "sw      x20, 80(a0)\n\t"
        "sw      x21, 84(a0)\n\t"
        "sw      x22, 88(a0)\n\t"
        "sw      x23, 92(a0)\n\t"
        "sw      x24, 96(a0)\n\t"
        "sw      x25, 100(a0)\n\t"
        "sw      x26, 104(a0)\n\t"
        "sw      x27, 108(a0)\n\t"
        "sw      x28, 112(a0)\n\t"
        "sw      x29, 116(a0)\n\t"
        "sw      x30, 120(a0)\n\t"
        "sw      x31, 124(a0)\n\t");
}
void lp_fw_restore_cpu_para(uint32_t save_addr)
{
    __asm__ __volatile__(
        "lw      x0, 0(a0)\n\t"
        "lw      x1, 4(a0)\n\t"
        "lw      x2, 8(a0)\n\t"
        "lw      x3, 12(a0)\n\t"
        "lw      x4, 16(a0)\n\t"
        "lw      x5, 20(a0)\n\t"
        "lw      x6, 24(a0)\n\t"
        "lw      x7, 28(a0)\n\t"
        "lw      x8, 32(a0)\n\t"
        "lw      x9, 36(a0)\n\t"
        "lw      x10, 40(a0)\n\t"
        "lw      x11, 44(a0)\n\t"
        "lw      x12, 48(a0)\n\t"
        "lw      x13, 52(a0)\n\t"
        "lw      x14, 56(a0)\n\t"
        "lw      x15, 60(a0)\n\t"
        "lw      x16, 64(a0)\n\t"
        "lw      x17, 68(a0)\n\t"
        "lw      x18, 72(a0)\n\t"
        "lw      x19, 76(a0)\n\t"
        "lw      x20, 80(a0)\n\t"
        "lw      x21, 84(a0)\n\t"
        "lw      x22, 88(a0)\n\t"
        "lw      x23, 92(a0)\n\t"
        "lw      x24, 96(a0)\n\t"
        "lw      x25, 100(a0)\n\t"
        "lw      x26, 104(a0)\n\t"
        "lw      x27, 108(a0)\n\t"
        "lw      x28, 112(a0)\n\t"
        "lw      x29, 116(a0)\n\t"
        "lw      x30, 120(a0)\n\t"
        "lw      x31, 124(a0)\n\t"
        "la      a0, freertos_risc_v_trap_handler\n\t" // mcu default_trap_handler
        "ori     a0, a0, 3\n\t"
        "csrw    mtvec, a0\n\t");
}
#if 0
/* dma copy lpfw */
#define DMA_ID_USE        DMA0_ID
#define DMA_CH_USE        DMA_CH3
#define DMA_TRNS_SIZE_MAX 4064

static DMA_LLI_Ctrl_Type dam_lli_buff[4] ATTR_EALIGN(32);

static void dma_memcpy_nonblock(uint32_t *dest, uint32_t *source, uint32_t word_num)
{
    /* no cache ram */
    DMA_LLI_Ctrl_Type *dam_lli = (DMA_LLI_Ctrl_Type *)((uintptr_t)dam_lli_buff & 0xBFFFFFFF);

    DMA_LLI_Cfg_Type lliCfg = {
        DMA_TRNS_M2M,
        DMA_REQ_NONE,
        DMA_REQ_NONE,
    };

    GLB_PER_Clock_UnGate(GLB_AHB_CLOCK_IP_DMA_0);

    dam_lli[0].srcDmaAddr = 0;
    dam_lli[0].destDmaAddr = 0;
    dam_lli[0].nextLLI = 0;
    dam_lli[0].dmaCtrl.TransferSize = 0;
    dam_lli[0].dmaCtrl.SBSize = DMA_BURST_SIZE_4;
    dam_lli[0].dmaCtrl.DBSize = DMA_BURST_SIZE_4;
    dam_lli[0].dmaCtrl.dst_min_mode = DISABLE;
    dam_lli[0].dmaCtrl.dst_add_mode = DISABLE;
    dam_lli[0].dmaCtrl.SWidth = DMA_TRNS_WIDTH_32BITS;
    dam_lli[0].dmaCtrl.DWidth = DMA_TRNS_WIDTH_32BITS;
    dam_lli[0].dmaCtrl.fix_cnt = 0;
    dam_lli[0].dmaCtrl.SI = DMA_MINC_ENABLE;
    dam_lli[0].dmaCtrl.DI = DMA_MINC_ENABLE;
    dam_lli[0].dmaCtrl.I = 0;

    for (uint8_t i = 0; i < 4; i++) {
        uint32_t addr_offset = DMA_TRNS_SIZE_MAX * i * 4;
        dam_lli[i] = dam_lli[0];
        dam_lli[i].srcDmaAddr = (uint32_t)(uintptr_t)source + addr_offset;
        dam_lli[i].destDmaAddr = (uint32_t)(uintptr_t)dest + addr_offset;

        if (word_num > DMA_TRNS_SIZE_MAX) {
            dam_lli[i].dmaCtrl.TransferSize = DMA_TRNS_SIZE_MAX;
            dam_lli[i].nextLLI = (uint32_t)(uintptr_t)&dam_lli[i + 1];
            word_num -= DMA_TRNS_SIZE_MAX;
        } else {
            dam_lli[i].dmaCtrl.TransferSize = DMA_TRNS_SIZE_MAX;
            dam_lli[i].nextLLI = 0;
            break;
        }
    }

    DMA_Disable(DMA_ID_USE);
    DMA_Channel_Disable(DMA_ID_USE, DMA_CH_USE);
    DMA_LLI_Init(DMA_ID_USE, DMA_CH_USE, &lliCfg);
    DMA_LLI_Update(DMA_ID_USE, DMA_CH_USE, (uint32_t)(uintptr_t)dam_lli);
    DMA_Enable(DMA_ID_USE);
    DMA_Channel_Enable(DMA_ID_USE, DMA_CH_USE);
}

int dma_busy_get()
{
    if (DMA_Channel_Is_Busy(DMA_ID_USE, DMA_CH_USE)) {
        return 1;
    }
    return 0;
}
#endif
static void rtc_wakeup_init(uint64_t rtc_wakeup_cmp_cnt, uint64_t sleep_us)
{
    uint32_t tmpVal;
    uint64_t rtc_cnt;

    if (sleep_us == 0) {
        return;
    }

    /* Set RTC compare mode, and enable rtc */
    tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
    tmpVal = tmpVal & 0xfffffff1;
    BL_WR_REG(HBN_BASE, HBN_CTL, tmpVal | 0x00000003);

    if( rtc_wakeup_cmp_cnt ==0 ) {
        /* Tigger RTC val latch  */
        tmpVal = BL_RD_REG(HBN_BASE, HBN_RTC_TIME_H);
        tmpVal = BL_SET_REG_BIT(tmpVal, HBN_RTC_TIME_LATCH);
        BL_WR_REG(HBN_BASE, HBN_RTC_TIME_H, tmpVal);
        tmpVal = BL_CLR_REG_BIT(tmpVal, HBN_RTC_TIME_LATCH);
        BL_WR_REG(HBN_BASE, HBN_RTC_TIME_H, tmpVal);

        /* Read RTC cnt */
        rtc_cnt = (BL_RD_REG(HBN_BASE, HBN_RTC_TIME_H) & 0xff);
        rtc_cnt <<= 32;
        rtc_cnt |= BL_RD_REG(HBN_BASE, HBN_RTC_TIME_L);

        /* calculate RTC Comp cnt */
        rtc_cnt += BL_US_TO_PDS_CNT(sleep_us);
    } else {
         rtc_cnt = rtc_wakeup_cmp_cnt;
    }

    iot2lp_para->rtc_wakeup_cnt = rtc_cnt;
    iot2lp_para->rtc_wakeup_en = 1;

    /* Set RTC Comp cnt */
    BL_WR_REG(HBN_BASE, HBN_TIME_H, (uint32_t)(rtc_cnt >> 32) & 0xff);
    BL_WR_REG(HBN_BASE, HBN_TIME_L, (uint32_t)rtc_cnt);

    /* Set interrupt delay option */
    tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, HBN_RTC_DLY_OPTION, HBN_RTC_INT_DELAY_0T);
    BL_WR_REG(HBN_BASE, HBN_CTL, (uint32_t)tmpVal);
}

ATTR_NOCACHE_NOINIT_RAM_SECTION struct bflb_sha256_ctx_s ctx_sha256;

static void lpfw_sec_sha256(uint32_t addr, uint32_t len, uint8_t *result)
{
    struct bflb_device_s *sha256;
    sha256 = bflb_device_get_by_name("sha");

    bflb_group0_request_sha_access(sha256);

    bflb_sha_init(sha256, SHA_MODE_SHA256);
    bflb_sha256_start(sha256, &ctx_sha256);
    bflb_sha256_update(sha256, &ctx_sha256, (const uint8_t *)addr, len);
    bflb_sha256_finish(sha256, &ctx_sha256, result);
}

void bl_lp_fw_init()
{
#if (BL_LP_LOG_EN)
    static lp_fw_time_debug_t time_debug_buff[TIME_DEBUG_NUM_MAX];
#endif

    uintptr_t dst_addr = LP_FW_START_ADDR;
    uint32_t lpfw_size = *((uint32_t *)__lpfw_start - 7);
    uint32_t chip_version = 0;

    /* clean iot2lp_para */
     memset(iot2lp_para, 0, (uint32_t)&iot2lp_para->reset_keep - (uint32_t)iot2lp_para);

    /* Setting the Default Value */
    iot2lp_para->dtim_num = 10;
    iot2lp_para->dtim_num_correction = iot2lp_para->dtim_num;
    iot2lp_para->beacon_interval_tu = 100;
    iot2lp_para->rc32k_auto_cal_en = 1;
    iot2lp_para->debug_io = 0xFF;

    /* Save rc32k code in HBN_RAM */
    iot2lp_para->rc32k_fr_ext = (*((volatile uint32_t *)0x2000F200)) >> 22;

    /* sliding window init */
    iot2lp_para->bcn_delay_sliding_win_size = sizeof(iot2lp_para->bcn_delay_sliding_win_buff);
    iot2lp_para->rssi_sliding_win_size = sizeof(iot2lp_para->rssi_sliding_win_buff);

    /* First load */
    memcpy((void *)dst_addr, __lpfw_start, lpfw_size);

    /* get chip version*/
    chip_version = BL_RD_WORD(0x90015800);
    if (chip_version == 0x06160001) {
        /* only first version need pre jump */
        memcpy((void *)LP_FW_PRE_JUMP_ADDR, hbn_ucode, sizeof(hbn_ucode));
    } else {
        /* later version use OCRAM for recovery */
        BL_WR_WORD(0x22FC0000, 0x4e42484d);
        BL_WR_WORD(0x22FC0004, 0x4e42484d);
        BL_WR_WORD(0x22FC0008, dst_addr);
        Tzc_Sec_OCRAM_Access_Set_Advance(0, 0x22FC0000, (0x400), 0x0);
    }

    // iot2lp_para->flash_cfg = bl_flash_get_flashCfg();
    uint32_t flash_cfg_len;
    bflb_flash_get_cfg((uint8_t **)&iot2lp_para->flash_cfg, &flash_cfg_len);

    iot2lp_para->flash_jdec_id = GLB_Get_Flash_Id_Value();

    BL_LP_LOG("flash jdec_id 0x%08lX\r\n", (unsigned long)iot2lp_para->flash_jdec_id);

    bl_lp_xip_para_save();

    /* nocache ram */
#if (BL_LP_LOG_EN)
    iot2lp_para->time_debug = (void *)((uint32_t)time_debug_buff & 0x2FFFFFFF);
    memset(iot2lp_para->time_debug, 0, sizeof(lp_fw_time_debug_t) * TIME_DEBUG_NUM_MAX);
#endif
}

int bl_lp_beacon_interval_update(uint16_t beacon_interval_tu)
{
    if(beacon_interval_tu < 20 || beacon_interval_tu > 1000 ){
        return -1;
    }

    if(beacon_interval_tu == iot2lp_para->beacon_interval_tu){
        return 0;
    }

    BL_LP_LOG("beacon interval update %d -> %d", (int)iot2lp_para->beacon_interval_tu, (int)beacon_interval_tu);

    iot2lp_para->beacon_interval_tu = beacon_interval_tu;

    /* TODO: Other actions may be required, such as resetting the state */

    return 0;
}

void bl_lp_fw_disconnection()
{
    /* Reset parameters */
    iot2lp_para->dtim_num = 10;
    iot2lp_para->dtim_num_correction = iot2lp_para->dtim_num;
    iot2lp_para->beacon_interval_tu = 100;
    /* Save rc32k code in HBN_RAM */
    iot2lp_para->rc32k_fr_ext = (*((volatile uint32_t *)0x2000F200)) >> 22;
    /* Clear continuous_loss_cnt */
    iot2lp_para->continuous_loss_cnt = 0;

    /* Clear the data of the sliding window */
    memset(iot2lp_para->bcn_delay_sliding_win_buff, 0, iot2lp_para->bcn_delay_sliding_win_size);
    iot2lp_para->bcn_delay_sliding_win_point = 0;
    iot2lp_para->bcn_delay_sliding_win_status = 0;
    iot2lp_para->bcn_delay_stamp_last_us = 0;

    memset(iot2lp_para->rssi_sliding_win_buff, 0, iot2lp_para->rssi_sliding_win_size);
    iot2lp_para->rssi_sliding_win_point = 0;
    iot2lp_para->rssi_sliding_win_status = 0;

    /* Invalid timestamp */
    iot2lp_para->last_beacon_stamp_rtc_valid = 0;
    iot2lp_para->last_rc32trim_stamp_valid = 0;
    iot2lp_para->last_rc32trim_stamp_rtc_us = 0;
}

void bl_lp_rtc_use_xtal32K()
{
    GLB_GPIO_Cfg_Type gpioCfg = {
        .gpioPin = GLB_GPIO_PIN_0,
        .gpioFun = GPIO_FUN_ANALOG,
        .gpioMode = GPIO_MODE_ANALOG,
        .pullType = GPIO_PULL_NONE,
        .drive = 1,
        .smtCtrl = 1
    };

    gpioCfg.gpioPin = 16;
    GLB_GPIO_Init(&gpioCfg);
    gpioCfg.gpioPin = 17;
    GLB_GPIO_Init(&gpioCfg);

    HBN_Power_On_Xtal_32K();
    HBN_32K_Sel(1);
    /* GPIO17 no pull */
    *((volatile uint32_t *)0x2000F014) &= ~(1 << 16);
}

void bl_lp_rtc_use_rc32k()
{
    HBN_Power_Off_Xtal_32K();
    HBN_32K_Sel(0);
}

uint64_t get_platform_timer(void);

static __inline uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Clk_Sel_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_SF_CFG0);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_SEL);
}

static __inline uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Div_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_SF_CFG0);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_DIV);
}

static __inline uint8_t ATTR_CLOCK_SECTION Clock_Get_SF_Clk_Sel2_Val(void)
{
    uint32_t tmpVal;

    tmpVal = BL_RD_REG(GLB_BASE, GLB_SF_CFG0);

    return BL_GET_REG_BITS_VAL(tmpVal, GLB_SF_CLK_SEL2);
}

static void bl_lp_xip_get_flash_clock(uint8_t *flash_clk, uint8_t *flash_clk_div)
{
    uint8_t sel = Clock_Get_SF_Clk_Sel_Val();
    uint8_t sel2 = Clock_Get_SF_Clk_Sel2_Val();

    if (sel == 0) {
        /* sf sel2 */
        if (sel2 == 0) {
            /* wifi pll 120m */
            *flash_clk = 0;
        } else if (sel2 == 1) {
            /* xtal */
            *flash_clk = 1;
        } else if (sel2 == 2) {
            /* wifi pll 120m */
            *flash_clk = 0;
        } else {
            /* aupll div5 clk */
            *flash_clk = 2;
        }
    } else if (sel == 1) {
        /* mux 80m */
        *flash_clk = 3;
    } else if (sel == 2) {
        /* mcu pbclk */
        *flash_clk = 4;
    } else {
        /* wifi 96m */
        *flash_clk = 5;
    }
    *flash_clk_div = Clock_Get_SF_Div_Val();
}

extern uint32_t __binary_length;
static void bl_lp_xip_para_save(void)
{
#define SF_Ctrl_Get_AES_Region(addr, r) (addr + SF_CTRL_AES_REGION_OFFSET + (r)*0x80)
    uint32_t tmpVal, i = 3;
    uint32_t regionRegBase = SF_Ctrl_Get_AES_Region(SF_CTRL_BASE, 0);
    uint8_t *iv = iot2lp_para->aesiv;
    uint32_t read_data = 0;
    //spi_flash_cfg_type *flash_cfg;
    uint8_t flash_clk, flash_clk_div;

    tmpVal = getreg32(SF_CTRL_BASE + SF_CTRL_SF_AES_OFFSET);

    if (((tmpVal >> SF_CTRL_SF_AES_BLK_MODE_POS) & 0x1) == 1) {
        iot2lp_para->xts_mode = 1;
        memset(iv, 0, 4);
        tmpVal = SF_CTRL_SF_AES_IV_W2_OFFSET;
        while (i--) {
            iv += 4;
            read_data = getreg32(regionRegBase + tmpVal);
            memcpy(iv, &read_data, 4);
            tmpVal -= 4;
        }
    } else {
        iot2lp_para->xts_mode = 0;

        tmpVal = SF_CTRL_SF_AES_IV_W0_OFFSET;
        while (i--) {
            read_data = getreg32(regionRegBase + tmpVal);
            memcpy(iv, &read_data, 4);
            iv += 4;
            tmpVal += 4;
        }
    }

    uint32_t lpfw_size = *((uint32_t *)__lpfw_start - 7);
    iot2lp_para->img_len = (int)(&__binary_length) + lpfw_size;
    printf("app_len=%d\r\n", (int)&__binary_length);
    printf("lp_len=%d\r\n", (int)lpfw_size);
    printf("image_len=%d\r\n", (int)iot2lp_para->img_len);

    tmpVal = getreg32(SF_CTRL_BASE + SF_CTRL_SF_AES_OFFSET);
    // BL_RD_REG(SF_CTRL_BASE, SF_CTRL_SF_AES);
    iot2lp_para->encrypt_type = BL_GET_REG_BITS_VAL(tmpVal, SF_CTRL_SF_AES_MODE);

    bl_lp_xip_get_flash_clock(&flash_clk, &flash_clk_div);
    iot2lp_para->flash_clk = flash_clk;
    iot2lp_para->flash_clk_div = flash_clk_div;
    iot2lp_para->do_xip_recovery = 0;
}

#ifdef CONF_PSRAM_RESTORE
#define LPFW_PSRAM_ID1_WINBOND_4MB  0x5f
#define LPFW_PSRAM_ID2_WINBOND_32MB 0xe86
static void bflb_init_psram_gpio(void)
{
    GLB_GPIO_Cfg_Type cfg;

    cfg.pullType = GPIO_PULL_NONE;
    cfg.drive = 0;
    cfg.smtCtrl = 1;

    for (uint8_t i = 0; i < 12; i++) {
        cfg.gpioPin = 41 + i;
        cfg.gpioMode = GPIO_MODE_INPUT;

        GLB_GPIO_Init(&cfg);
    }
}

static uint16_t winbond_x8_psram_init(int8_t burst_len, uint8_t is_fixLatency, uint8_t latency, uint16_t dqs_delay, uint8_t size)
{
    uint16_t reg_read = 0;
    PSRAM_Ctrl_Cfg_Type default_psram_ctrl_cfg = {
        .vendor = PSRAM_CTRL_VENDOR_WINBOND,
        .ioMode = PSRAM_CTRL_X8_MODE,
        .size = PSRAM_SIZE_4MB,
        .dqs_delay = 0xfff0,
    };

    PSRAM_Winbond_Cfg_Type default_winbond_cfg = {
        .rst = DISABLE,
        .clockType = PSRAM_CLOCK_DIFF,
        .inputPowerDownMode = DISABLE,
        .hybridSleepMode = DISABLE,
        .linear_dis = ENABLE,
        .PASR = PSRAM_PARTIAL_REFRESH_FULL,
        .disDeepPowerDownMode = ENABLE,
        .fixedLatency = DISABLE,
        .brustLen = PSRAM_WINBOND_BURST_LENGTH_32_BYTES,
        .brustType = PSRAM_WRAPPED_BURST,
        .latency = PSRAM_WINBOND_6_CLOCKS_LATENCY,
        .driveStrength = PSRAM_WINBOND_DRIVE_STRENGTH_35_OHMS_FOR_4M_115_OHMS_FOR_8M,
    };

    default_winbond_cfg.brustLen = burst_len;
    default_winbond_cfg.fixedLatency = is_fixLatency;
    default_winbond_cfg.latency = latency;

    default_psram_ctrl_cfg.size = size;
    default_psram_ctrl_cfg.dqs_delay = dqs_delay;

    PSram_Ctrl_Init(PSRAM0_ID, &default_psram_ctrl_cfg);
    // PSram_Ctrl_Winbond_Reset(PSRAM0_ID);
    PSram_Ctrl_Winbond_Write_Reg(PSRAM0_ID, PSRAM_WINBOND_REG_CR0, &default_winbond_cfg);
    /* check psram work or not */
    PSram_Ctrl_Winbond_Read_Reg(PSRAM0_ID, PSRAM_WINBOND_REG_ID0, &reg_read);
    return reg_read;
}
/**
 * @brief
 *
 * @return uint32_t
 */
#define EF_PSRAM_INFO_NONE  0x0
#define EF_PSRAM_INFO_WB_4M 0x1
uint32_t board_psram_x8_init(void)
{
    int16_t psram_id = 0;
    int32_t left_flag = 0, right_flag = 0, c_val = 0;
    uint32_t chip_info = 0, psram_trim = 0;
    uint16_t dqs_val[] = {
        0x8000,
        0xC000,
        0xE000,
        0xF000,
        0xF800,
        0xFC00,
        0xFE00,
        0xFF00,
        0xFF80,
        0xFFC0,
        0xFFE0,
        0xFFF0,
        0xFFF8,
        0xFFFC,
        0xFFFE,
        0xFFFF,
    };

    /* read efuse */
    EF_Ctrl_Load_Efuse_R0();
    chip_info = BL_RD_WORD(0x20056018);

    /* check psram exist */
    if ((chip_info & 0x3000000) != 0x0) {
        uint8_t psram_size_info = 0;
        uint8_t psram_size = 0;
        /* set psramb clk */
        GLB_Set_PSRAMB_CLK_Sel(ENABLE, GLB_PSRAMB_EMI_WIFIPLL_320M, 0);

        /* init psram gpio */
        bflb_init_psram_gpio();

        /* psram init*/
        psram_size_info = ((chip_info & 0x3000000) >> 24);

        /* read psram trim */
        psram_trim = BL_RD_WORD(0x200560E8);

        if ((psram_size_info) == EF_PSRAM_INFO_WB_4M) {
            psram_size = PSRAM_SIZE_4MB;
        } else {
            return -1;
        }

        if ((psram_trim & (0x1000)) && (((psram_trim & 0x800) >> 11) == EF_Ctrl_Get_Trim_Parity(psram_trim, 11))) {
            left_flag = ((psram_trim & (0xf0)) >> 0x4);
            right_flag = (psram_trim & (0xf));
            c_val = ((left_flag + right_flag) >> 0x1);
            psram_id = winbond_x8_psram_init(PSRAM_WINBOND_BURST_LENGTH_64_BYTES, 0, PSRAM_WINBOND_6_CLOCKS_LATENCY, dqs_val[c_val], psram_size);
        } else {
            psram_id = winbond_x8_psram_init(PSRAM_WINBOND_BURST_LENGTH_64_BYTES, 0, PSRAM_WINBOND_6_CLOCKS_LATENCY, dqs_val[11], psram_size);
        }

        if ((psram_id != LPFW_PSRAM_ID1_WINBOND_4MB) && (psram_id != LPFW_PSRAM_ID2_WINBOND_32MB)) {
            return -1;
        }
    }
    return psram_id;
}
#endif

static uint8_t bl_lp_wakeup_check(void)
{
    if ((iot2lp_para->wake_io_bits) || (iot2lp_para->wake_acomp_bits)) {
        return 1;
    } else {
        return 0;
    }
}

__WEAK uint64_t bl_lp_check_gpio_int(void)
{
    uint64_t result = 0;
    return result;
}
__WEAK uint8_t bl_lp_check_acomp_int(void)
{
    uint8_t result = 0;
    return result;
}

uint64_t ulLowPowerTimeEnterFunction;
uint64_t ulLowPowerTimeAfterSleep;

void bl_lp_debug_record_time(iot2lp_para_t *iot_lp_para, char *info_str)
{
#if BL_LP_LOG_EN
    uint64_t rtc_cnt, rtc_now_us;
    lp_fw_time_debug_t *time_debug = iot_lp_para->time_debug;

    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    rtc_now_us = BL_PDS_CNT_TO_US(rtc_cnt);

    for (uint8_t i = 0; i < TIME_DEBUG_NUM_MAX; i++) {
        if (time_debug[i].trig_cnt == 0 || time_debug[i].info_str == info_str) {
            time_debug[i].time_stamp_us = rtc_now_us;
            time_debug[i].info_str = info_str;
            time_debug[i].trig_cnt++;
            break;
        }
    }
#endif
}

void bl_lp_debug_clean_time(iot2lp_para_t *iot_lp_para)
{
#if BL_LP_LOG_EN
    memset(iot_lp_para->time_debug, 0, sizeof(lp_fw_time_debug_t) * TIME_DEBUG_NUM_MAX);
#endif
}

void bl_lp_debug_dump_time(iot2lp_para_t *iot_lp_para)
{
#if BL_LP_LOG_EN
    lp_fw_time_debug_t *time_debug = iot_lp_para->time_debug;

    BL_LP_LOG("time debug dump\r\n");

    for (int i = 0; i < 8; i++) {
        if (time_debug[i].trig_cnt == 0) {
            break;
        }

        BL_LP_LOG("time debug[%d] = %lld, cnt: %ld", i, time_debug[i].time_stamp_us, time_debug[i].trig_cnt);

        if (i > 0) {
            BL_LP_LOG(", diff %lld", time_debug[i].time_stamp_us - time_debug[i - 1].time_stamp_us);
        }

        if (time_debug[i].info_str) {
            BL_LP_LOG(", \tinfo: %s", time_debug[i].info_str);
        }

        BL_LP_LOG(";\r\n");
    }

    BL_LP_LOG("\r\ndump end\r\n");
#endif
}

int bl_lp_fw_enter(bl_lp_fw_cfg_t *bl_lp_fw_cfg)
{
    uintptr_t dst_addr = LP_FW_START_ADDR;
    uint32_t lpfw_size = *((uint32_t *)__lpfw_start - 7);

    uint64_t rtc_cnt, rtc_now_us, last_beacon_rtc_us;
    uint32_t dtim_period_us, pds_sleep_us, beacon_interval_us, total_error;
    uint64_t rtc_sleep_us, rtc_wakeup_cmp_cnt;

    if (bl_lp_fw_cfg == NULL) {
        return -1;
    }

    rtc_wakeup_cmp_cnt = bl_lp_fw_cfg->rtc_wakeup_cmp_cnt;
    rtc_sleep_us = bl_lp_fw_cfg->rtc_timeout_us;

    if ((bl_lp_fw_cfg->tim_wakeup_en == 0) && (rtc_wakeup_cmp_cnt == 0) && (bl_lp_fw_cfg->rtc_timeout_us == 0)) {
        /* no wake-up source */
        return -2;
    }

    bl_lp_debug_record_time(iot2lp_para, "bl_lp_fw_enter");

    /* clean wake bits */
    iot2lp_para->wake_io_bits = 0;
    iot2lp_para->wake_acomp_bits = 0;
    iot2lp_para->wake_io_edge_bits = 0;
    iot2lp_para->wake_acomp_edge_bits = 0;

    LP_HOOK(pre_user, bl_lp_fw_cfg);

    __disable_irq();

    LP_HOOK(pre_sys, bl_lp_fw_cfg);

    if (bl_lp_fw_cfg->lpfw_copy) {
        /* ensure integrity of lpfw  */
        /* Copy move to idle task */
        // memcpy((void *)dst_addr, __lpfw_start, lpfw_size);
        L1C_DCache_Clean_All();

        uint8_t result[32];
        uint8_t *lpfw_sha256 = (uint8_t *)(__lpfw_start - 64);
        lpfw_sec_sha256(dst_addr, lpfw_size, result);
        if (memcmp(result, lpfw_sha256, 32) != 0) {
            /* Unreoverable Error!!!! */
            assert(0);
        }
    }

    iot2lp_para->wakeup_reason = LPFW_WAKEUP_UNKOWN;
    iot2lp_para->rtc_wakeup_en = 0;
    iot2lp_para->rtc_wakeup_cnt = 0;


    if((rtc_wakeup_cmp_cnt == 0) && rtc_sleep_us > ((uint64_t)24 * 60 * 60 * 1000 * 1000)){
        rtc_sleep_us = ((uint64_t)24 * 60 * 60 * 1000 * 1000);
    }

    /* tim interval */
    if (bl_lp_fw_cfg->dtim_num != 0) {
        iot2lp_para->dtim_num = bl_lp_fw_cfg->dtim_num;
    } else {
        iot2lp_para->dtim_num = 10;
    }

    if( iot2lp_para->beacon_leg_rate == 0){
        iot2lp_para->beacon_leg_rate = 2; //1M
    }

    /* beacon interval us, TU * 1024 */
    beacon_interval_us = iot2lp_para->beacon_interval_tu * 1024;

    /* get period of dtim */
    if (iot2lp_para->continuous_loss_cnt && iot2lp_para->dtim_num_correction < iot2lp_para->dtim_num) {
        dtim_period_us = iot2lp_para->dtim_num_correction * beacon_interval_us;
    } else {
        dtim_period_us = iot2lp_para->dtim_num * beacon_interval_us;
    }

    /* last beacon timestamp */
    last_beacon_rtc_us = iot2lp_para->last_beacon_stamp_rtc_us;
    /* beacon delay */
    last_beacon_rtc_us -= iot2lp_para->last_beacon_delay_us;

    /* Gets the current rtc time */
    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    rtc_now_us = BL_PDS_CNT_TO_US(rtc_cnt);

    /* calculate pds and rtc sleep time */
    if (iot2lp_para->last_beacon_stamp_rtc_valid) {

        /* Time to the next beacon */
        if (last_beacon_rtc_us + dtim_period_us > rtc_now_us) {
            /* Before the expected time */
            pds_sleep_us = last_beacon_rtc_us + dtim_period_us - rtc_now_us;
        } else {
            /* After the expected time */
            pds_sleep_us = beacon_interval_us - ((rtc_now_us - last_beacon_rtc_us) % beacon_interval_us);
        }

        if (pds_sleep_us <= PROTECT_AF_MS * 1000) {
            /* Time is too short, delay to a beacon */
            pds_sleep_us += beacon_interval_us;
        }

        /* Prevent rtc from colliding with pds */
        if (rtc_wakeup_cmp_cnt == 0 && rtc_sleep_us) {
            /*  */
            if (((pds_sleep_us >= rtc_sleep_us) && (pds_sleep_us - rtc_sleep_us) < PROTECT_BF_MS * 1000) ||
                ((pds_sleep_us <= rtc_sleep_us) && (rtc_sleep_us - pds_sleep_us) < PROTECT_AF_MS * 1000)) {
                /* Advance the RTC time */
                if(rtc_sleep_us > pds_sleep_us - PROTECT_AF_MS * 1000){
                    rtc_sleep_us = pds_sleep_us - PROTECT_AF_MS * 1000;
                } else {
                    rtc_sleep_us = 0;
                }
                BL_LP_LOG("---- Advance of rtc ----\r\n");
            }
        }

    } else {
        /* It shouldn't be here */
        pds_sleep_us = dtim_period_us;
    }

    /* Compensates for pds goto sleep error */
    if (iot2lp_para->last_beacon_stamp_rtc_valid) {
        /* last beacon form */
        pds_sleep_us -= 1000;
    }

    /* error compensation */
    pds_sleep_us += (int64_t)pds_sleep_us * iot2lp_para->rtc32k_error_ppm / 1000000;
    // rtc_sleep_us += (int64_t)rtc_sleep_us * iot2lp_para->rtc32k_error_ppm / 1000000;

#if 0
    BL_LP_LOG("last_beacon_stamp_rtc_us: %lld\r\n", iot2lp_para->last_beacon_stamp_rtc_us);
    BL_LP_LOG("stamp_rtc_valid: %ld\r\n", iot2lp_para->last_beacon_stamp_rtc_valid);
    BL_LP_LOG("pds_sleep_ms: %ld\r\n", pds_sleep_us / 1000);
    BL_LP_LOG("rtc_timeout_ms: %lld\r\n", rtc_sleep_us / 1000);
    BL_LP_LOG("rc32k code %ld\r\n", iot2lp_para->rc32k_fr_ext);
    BL_LP_LOG("rtc ppm %ld\r\n", iot2lp_para->rtc32k_error_ppm);
    pds_sleep_us -= 500;
#endif

    /* get glb gpio inturrpt status */
    uint64_t glb_io_int_stat = bl_lp_check_gpio_int();
    iot2lp_para->wake_io_bits = glb_io_int_stat;
    uint8_t acomp_int_stat = bl_lp_check_acomp_int();
    iot2lp_para->wake_acomp_bits = acomp_int_stat;

    /* Set lp_fw as the wake-up entry  */
    pm_set_wakeup_callback((void (*)(void))LP_FW_PRE_JUMP_ADDR);
    *((volatile uint32_t *)0x2000e504) &= ~(1 << 6);
    pm_pds_mask_all_wakeup_src();
    AON_Output_Float_LDO15_RF();
    AON_Output_Float_DCDC18();
    HBN_Pin_WakeUp_Mask(0xF);
    PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_1_GPIO0_GPIO7);
    PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_2_GPIO8_GPIO15);
    PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_3_GPIO20_GPIO27);
    PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_4_GPIO28_GPIO34);

    if (gp_lp_io_cfg) {
        g_lp_io_cfg_bak = *gp_lp_io_cfg;
        BL_LP_LOG("io_wakeup_unmask: 0x%llX\r\n", (unsigned long long)g_lp_io_cfg_bak.io_wakeup_unmask);
        bl_lp_io_wakeup_init(&g_lp_io_cfg_bak);
    }

    if (gp_lp_acomp_cfg) {
        g_lp_acomp_cfg_bak = *gp_lp_acomp_cfg;
        BL_LP_LOG("acomp0: %d, acomp1: %d\r\n", g_lp_acomp_cfg_bak.acomp0_trig_mode, g_lp_acomp_cfg_bak.acomp1_trig_mode);
        bl_lp_acomp_wakeup_init(&g_lp_acomp_cfg_bak);
    }

    /* lpfw cfg: system para */
    iot2lp_para->mcu_sts = bl_lp_fw_cfg->mcu_sts;
    // iot2lp_para->lpfw_loss_cnt = 0;
    // iot2lp_para->lpfw_recv_cnt = 0;
    iot2lp_para->lpfw_wakeup_cnt = 0;
    iot2lp_para->pattern = 0xAA5555AA;
    iot2lp_para->wakeup_flag = 0;
    iot2lp_para->flash_offset = bflb_sf_ctrl_get_flash_image_offset(0, SF_CTRL_FLASH_BANK0);
    iot2lp_para->app_entry = (uintptr_t)lp_fw_restore_cpu_para;
    iot2lp_para->args[0] = GET_OFFSET(iot2lp_para_t, cpu_regs) + IOT2LP_PARA_ADDR;

    /* rc32k auto calibration */
    iot2lp_para->rc32k_auto_cal_en = 1;

    if (bl_lp_fw_cfg->tim_wakeup_en) {
        iot2lp_para->tim_wake_enable = 1;
        /* lpfw cfg: wifi para */
        memcpy(iot2lp_para->bssid, bl_lp_fw_cfg->bssid, 6);
        memcpy(iot2lp_para->local_mac, bl_lp_fw_cfg->mac, 6);
        iot2lp_para->ap_channel = bl_lp_fw_cfg->channel;
        iot2lp_para->aid = bl_lp_fw_cfg->aid;
        /* lpfw rx timeout */
        iot2lp_para->mtimer_timeout_en = 1;
        iot2lp_para->mtimer_timeout_mini_us = bl_lp_fw_cfg->mtimer_timeout_mini_us;
        iot2lp_para->mtimer_timeout_max_us = bl_lp_fw_cfg->mtimer_timeout_max_us;

        /* lpfw cfg: beacon attracking para */
        iot2lp_para->tpre = bl_lp_fw_cfg->tpre;

        /* take rc32k error rate as default */
        if (!(*((volatile uint32_t *)0x2000f030) & (1 << 3))) {
            /* rc32k 1500-ppm */
            iot2lp_para->rtc32k_jitter_error_ppm = 1000;
        } else {
            /* xtal 50-ppm */
            iot2lp_para->rtc32k_jitter_error_ppm = 200;
        }

        if (bl_lp_fw_cfg->timeout_cnt_th) {
            iot2lp_para->timeout_cnt_th = bl_lp_fw_cfg->timeout_cnt_th;
        } else {
            iot2lp_para->timeout_cnt_th = 4; /* The default value is 3 */
        }

        if (bl_lp_fw_cfg->ap_loss_cnt_th) {
            iot2lp_para->ap_loss_cnt_th = bl_lp_fw_cfg->ap_loss_cnt_th;
        } else {
            iot2lp_para->ap_loss_cnt_th = iot2lp_para->timeout_cnt_th + 3;
        }

    } else {
        iot2lp_para->tim_wake_enable = 0;
    }

    if (iot2lp_para->last_beacon_stamp_rtc_valid) {
        /* total error form (lasst bcn -> next bcn), jitter error compensation (ppm) */
        total_error = ((uint32_t)(rtc_now_us - last_beacon_rtc_us) + pds_sleep_us) / 1000 * iot2lp_para->rtc32k_jitter_error_ppm / 1000;
    } else {
        /* total error form (now -> next bcn), jitter error compensation (ppm) */
        total_error = pds_sleep_us / 1000 * iot2lp_para->rtc32k_jitter_error_ppm / 1000;
    }
    if(pds_sleep_us > total_error){
        pds_sleep_us -= total_error;
    }else{
        pds_sleep_us = 0;
    }

    /* Recorded the error value, for mtimer timeout */
    iot2lp_para->last_sleep_error_us = total_error;

    /* Minimum limit */
    if (pds_sleep_us < (PDS_WAKEUP_MINI_LIMIT_US + PDS_WAKEUP_DELAY_US)) {
        pds_sleep_us = PDS_WAKEUP_MINI_LIMIT_US;
    }else{
        pds_sleep_us -= PDS_WAKEUP_DELAY_US;
    }

    if(rtc_wakeup_cmp_cnt == 0){
        if (rtc_sleep_us < (PDS_WAKEUP_MINI_LIMIT_US + PDS_WAKEUP_DELAY_US)) {
            rtc_sleep_us = PDS_WAKEUP_MINI_LIMIT_US;
        }else{
            rtc_sleep_us -= PDS_WAKEUP_DELAY_US;
        }
    }else{
        if (rtc_wakeup_cmp_cnt > PDS_WAKEUP_DELAY_CNT) {
            rtc_wakeup_cmp_cnt -= PDS_WAKEUP_DELAY_CNT;
        }
    }

    LP_HOOK(pre_sleep, iot2lp_para);

    bl_lp_vtime_before_sleep();

    L1C_DCache_Clean_All();

    bl_lp_debug_record_time(iot2lp_para, "lp_fw_save_cpu_para");

    lp_fw_save_cpu_para(GET_OFFSET(iot2lp_para_t, cpu_regs) + IOT2LP_PARA_ADDR);

    if (iot2lp_para->wakeup_flag == 0) {
        /* Check io_stat , judge whether to enter PDS mode */
        /* if io_stat isn't 0x0, will sleep 2ms */
        if (1 == bl_lp_wakeup_check()) {
            rtc_sleep_us = 2000;
            pds_sleep_us = 2000;
        }

        if ((bl_lp_fw_cfg->tim_wakeup_en) && (rtc_wakeup_cmp_cnt || rtc_sleep_us)) {
            /* rtc cfg */
            rtc_wakeup_init(rtc_wakeup_cmp_cnt, rtc_sleep_us);
            /* clear rtc IRQ */
            BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0x00000001 << 16);
            /* enable rtc wakeup source */
            BL_WR_REG(PDS_BASE, PDS_INT, BL_RD_REG(PDS_BASE, PDS_INT) | (0x00000001 << 11));
            /* pds15 enter */
            pm_pds_mode_enter(PM_PDS_LEVEL_15, BL_US_TO_PDS_CNT(pds_sleep_us));

        } else if (bl_lp_fw_cfg->tim_wakeup_en) {
            /* disable rtc comp */
            uint32_t tmpVal;
            tmpVal = BL_RD_REG(HBN_BASE, HBN_CTL);
            tmpVal = tmpVal & 0xfffffff1;
            BL_WR_REG(HBN_BASE, HBN_CTL, tmpVal);
            /* pds15 enter */
            pm_pds_mode_enter(PM_PDS_LEVEL_15, BL_US_TO_PDS_CNT(pds_sleep_us));

        } else if (rtc_wakeup_cmp_cnt || rtc_sleep_us) {
            /* rtc cfg */
            rtc_wakeup_init(rtc_wakeup_cmp_cnt, rtc_sleep_us);

            /* clear rtc IRQ */
            BL_WR_REG(HBN_BASE, HBN_IRQ_CLR, 0x00000001 << 16);

            /* enable rtc wakeup source */
            BL_WR_REG(PDS_BASE, PDS_INT, BL_RD_REG(PDS_BASE, PDS_INT) | (0x00000001 << 11));

            /* pds15 enter */
            pm_pds_mode_enter(PM_PDS_LEVEL_15, 0);
        }

        iot2lp_para->wakeup_flag = 1;
        lp_fw_restore_cpu_para(GET_OFFSET(iot2lp_para_t, cpu_regs) + IOT2LP_PARA_ADDR);
    }

#if defined(__riscv_flen)
    __asm__ __volatile__(
        "lw      x0, 0(a0)\n\t"
        /* FP: initial state */
        "csrr    t0, mstatus\n\t"
        "li      t1, ~0x6000\n\t"
        "and     t0, t0, t1\n\t"
        "li      t1, 0x2000\n\t"
        "or      t0, t0, t1\n\t"
        "csrw    mstatus, t0\n\t");
    /* csrwi   fcsr, 0 */
#endif
    /* enable mstatus FS */
    uint32_t mstatus = __get_MSTATUS();
    mstatus |= (1 << 13);
    __set_MSTATUS(mstatus);

    /* enable mxstatus THEADISAEE */
    uint32_t mxstatus = __get_MXSTATUS();
    mxstatus |= (1 << 22);
    /* enable mxstatus MM */
    mxstatus |= (1 << 15);
    __set_MXSTATUS(mxstatus);

    /* get interrupt level from info */
    CLIC->CLICCFG = (((CLIC->CLICINFO & CLIC_INFO_CLICINTCTLBITS_Msk) >> CLIC_INFO_CLICINTCTLBITS_Pos) << CLIC_CLICCFG_NLBIT_Pos);

    /* tspend interrupt will be clear auto*/
    /* tspend use positive interrupt */
#ifdef CONFIG_IRQ_USE_VECTOR
    CLIC->CLICINT[MSOFT_IRQn].ATTR = 0x3;
#else
    CLIC->CLICINT[MSOFT_IRQn].ATTR = 0x2;
#endif

    /* disable mexstatus SPUSHEN and SPSWAPEN for ipush/ipop*/
    uint32_t mexstatus = __get_MEXSTATUS();
    mexstatus &= ~(0x3 << 16);
    __set_MEXSTATUS(mexstatus);

    /* Restore CPU:320M and Mtimer:1M */
    // GLB_Power_On_XTAL_And_PLL_CLK(GLB_XTAL_40M, GLB_PLL_WIFIPLL | GLB_PLL_AUPLL);
    GLB_Set_MCU_System_CLK_Div(0, 3);
    CPU_Set_MTimer_CLK(ENABLE, BL_MTIMER_SOURCE_CLOCK_MCU_CLK, Clock_System_Clock_Get(BL_SYSTEM_CLOCK_MCU_CLK) / 1000000 - 1);

    /* disable wdt */
    // bl_wdt_disable();
    uint32_t regval = 0;
    putreg16(0xBABA, 0x2000A500 + TIMER_WFAR_OFFSET);
    putreg16(0xEB10, 0x2000A500 + TIMER_WSAR_OFFSET);

    regval = getreg32(0x2000A500 + TIMER_WMER_OFFSET);
    regval &= ~TIMER_WE;
    putreg32(regval, 0x2000A500 + TIMER_WMER_OFFSET);

    /* The rc32k code is restored  */
    uint32_t reg = *((volatile uint32_t *)0x2000F200) & ~(0x3FF << 22);
    reg = reg | iot2lp_para->rc32k_fr_ext << 22;
    *((volatile uint32_t *)0x2000F200) = reg;

    bl_lp_vtime_after_sleep();

    bl_lp_debug_record_time(iot2lp_para, "return APP");

#ifdef CONF_PSRAM_RESTORE
    board_psram_x8_init();
#endif

    LP_HOOK(post_sys, iot2lp_para);

    /* try to use app mtimer to cal rc32,so make beacon_stamp_mtimer_valid=0 and init rc32k auto cal
       if xtal32 used, following code is also ok */
    pm_rc32k_auto_cal_init();

    PDS_IntClear();

    bl_lp_fw_cfg->wakeup_reason = iot2lp_para->wakeup_reason;
    bl_lp_fw_cfg->lpfw_recv_cnt = iot2lp_para->lpfw_recv_cnt;
    bl_lp_fw_cfg->lpfw_loss_cnt = iot2lp_para->lpfw_loss_cnt;

    if (iot2lp_para->wake_io_bits || iot2lp_para->wake_acomp_bits) {
        /* register */
        bflb_irq_attach(MSOFT_IRQn, (irq_callback)bl_lp_soft_irq,NULL);
        /* trig soft int */
        BL_LP_SOFT_INT_TRIG;
    }

#if 1
    BL_LP_LOG("--app wakeup reason %ld\r\n", bl_lp_fw_cfg->wakeup_reason);
    BL_LP_LOG("wake_io_bits 0x%016llx\r\n", iot2lp_para->wake_io_bits);
    BL_LP_LOG("wake_acomp_bits 0x%x\r\n", iot2lp_para->wake_acomp_bits);
    BL_LP_LOG("wake_io_edge_bits 0x%016llx\r\n", iot2lp_para->wake_io_edge_bits);
    BL_LP_LOG("wake_acomp_edge_bits 0x%x\r\n", iot2lp_para->wake_acomp_edge_bits);

    BL_LP_LOG("want receive %ld, loss %ld\r\n", bl_lp_fw_cfg->lpfw_recv_cnt, bl_lp_fw_cfg->lpfw_loss_cnt);
    if (bl_lp_fw_cfg->lpfw_recv_cnt) {
        BL_LP_LOG("loss rate %ld.%ld%%\r\n", bl_lp_fw_cfg->lpfw_loss_cnt * 100 / bl_lp_fw_cfg->lpfw_recv_cnt, (bl_lp_fw_cfg->lpfw_loss_cnt * 10000 / bl_lp_fw_cfg->lpfw_recv_cnt) % 100);
    }
    BL_LP_LOG("next tim : %d\r\n", bl_lp_get_next_beacon_time(0));
    BL_LP_LOG("rc32k code %ld, rc32k ppm %ld \r\n", iot2lp_para->rc32k_fr_ext, iot2lp_para->rtc32k_error_ppm);
#endif

    bl_lp_debug_record_time(iot2lp_para, "bl_lp_fw_enter end");

    BL_LP_LOG("rtc_sleep: %lld\r\n", rtc_sleep_us);
    BL_LP_LOG("rtc_wakeup_cnt: %ld\r\n", (uint32_t)(BL_PDS_CNT_TO_US(iot2lp_para->rtc_wakeup_cnt)));

#if (BL_LP_LOG_EN)
    bl_lp_debug_dump_time(iot2lp_para);
#endif

    bl_lp_debug_clean_time(iot2lp_para);

    return iot2lp_para->wakeup_reason;
}

uint8_t bl_lp_get_hal_boot2_lp_ret(void)
{
    if (iot2boot2_para->hbn_pattern == 0x55AAAA55) {
        return iot2boot2_para->hal_boot2_lp_ret;
    } else {
        return -1;
    }
}

uint32_t bl_lp_get_hbn_wakeup_reason(void)
{
    return iot2boot2_para->wakeup_reason;
}
void bl_lp_get_hbn_wakeup_io_bits(uint8_t *io_bits, uint8_t *io_edge_bits)
{
    *io_bits = iot2boot2_para->wkup_io_bits;
    *io_edge_bits = iot2boot2_para->wkup_io_edge_bits;
}

void bl_lp_get_hbn_wakeup_acomp_bits(uint8_t *acomp_bits, uint8_t *acomp_edge_bits)
{
    *acomp_bits = iot2boot2_para->wkup_acomp_bits;
    *acomp_edge_bits = iot2boot2_para->wkup_acomp_edge_bits;
}

void bl_lp_hbn_init(uint8_t wdt_en, uint8_t feed_wdt_pin, uint8_t feed_wdt_type, uint32_t feed_wdt_max_continue_times)
{
    memset((void *)IOT2BOOT2_PARA_ADDR, 0, sizeof(iot2boot2_para_t));

    if (wdt_en != 0) {
        iot2boot2_para->wdt_pattern = 0xAAAA5555;
        iot2boot2_para->feed_wdt_io = feed_wdt_pin;
        iot2boot2_para->feed_wdt_type = feed_wdt_type;
        iot2boot2_para->feed_wdt_max_continue_times = feed_wdt_max_continue_times;
        iot2boot2_para->feed_wdt_continue_times = 0;
    } else {
        iot2boot2_para->wdt_pattern = 0x0;
        iot2boot2_para->feed_wdt_io = 0xFF;
        iot2boot2_para->feed_wdt_type = 0;
        iot2boot2_para->feed_wdt_max_continue_times = 0;
        iot2boot2_para->feed_wdt_continue_times = 0;
    }
}

int bl_lp_hbn_enter(bl_lp_hbn_fw_cfg_t *bl_lp_hbn_fw_cfg)
{
    // uint32_t time_s=10;

    /* clean wake bits */
    iot2boot2_para->wkup_io_bits = 0;
    iot2boot2_para->wkup_acomp_bits = 0;
    iot2boot2_para->wkup_io_edge_bits = 0;
    iot2boot2_para->wkup_acomp_edge_bits = 0;
    iot2boot2_para->wakeup_reason = 0;

    if (gp_lp_io_cfg) {
        g_lp_io_cfg_bak = *gp_lp_io_cfg;
        BL_LP_LOG("io_wakeup_unmask: 0x%llX\r\n", g_lp_io_cfg_bak.io_wakeup_unmask);
        bl_lp_io_wakeup_init(&g_lp_io_cfg_bak);
    }

    if (gp_lp_acomp_cfg) {
        g_lp_acomp_cfg_bak = *gp_lp_acomp_cfg;
        BL_LP_LOG("acomp0: %d, acomp1: %d\r\n", g_lp_acomp_cfg_bak.acomp0_trig_mode, g_lp_acomp_cfg_bak.acomp1_trig_mode);
        bl_lp_acomp_wakeup_init(&g_lp_acomp_cfg_bak);
    }

    iot2boot2_para->feed_wdt_continue_times = 0;
    iot2boot2_para->hbn_level = bl_lp_hbn_fw_cfg->hbn_level;
    iot2boot2_para->hbn_sleep_period = bl_lp_hbn_fw_cfg->hbn_sleep_cnt;
    iot2boot2_para->hbn_pattern = 0x55AAAA55;
    iot2boot2_para->wkup_rtc_cnt = (bflb_rtc_get_time(NULL) + bl_lp_hbn_fw_cfg->hbn_sleep_cnt) & 0xFFFFFFFFFF;
    HBN_Set_Ldo11_Rt_Vout(0xA);
    HBN_Set_Ldo11_Soc_Vout(0xA);
    AON_Output_Pulldown_DCDC18();
    pm_hbn_mode_enter(iot2boot2_para->hbn_level, iot2boot2_para->hbn_sleep_period);
    return 0;
}

int bl_lp_fw_enter_check_allow(void)
{
    uint64_t last_beacon_rtc_us, rtc_now_us, rtc_cnt;
    uint32_t dtim_period_us, beacon_interval_us;

    if (iot2lp_para->last_beacon_stamp_rtc_valid == 0) {
        /* It shouldn't be here */
        return 1;
    }

    /* beacon interval us, TU * 1024 */
    beacon_interval_us = iot2lp_para->beacon_interval_tu * 1024;

    /* Time stamp of the last beacon */
    last_beacon_rtc_us = iot2lp_para->last_beacon_stamp_rtc_us;
    last_beacon_rtc_us -= iot2lp_para->last_beacon_delay_us;

    /* Gets the current rtc time */
    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    rtc_now_us = BL_PDS_CNT_TO_US(rtc_cnt);

    /* get period of dtim */
    if (iot2lp_para->continuous_loss_cnt) {
        dtim_period_us = iot2lp_para->dtim_num_correction * beacon_interval_us;
    } else {
        dtim_period_us = iot2lp_para->dtim_num * beacon_interval_us;
    }

    /* next beacon */
    if (last_beacon_rtc_us + dtim_period_us >= rtc_now_us) {
        /* */
        if ((last_beacon_rtc_us + dtim_period_us - rtc_now_us) < PROTECT_BF_MS * 1000) {
            /* beacon will be received soon */
            BL_LP_LOG("---- Not allowed sleep BF1 ----");
            return 0;
        }
    } else {
        /* Get the nearest beacon */
        int32_t next_time = ((rtc_now_us - last_beacon_rtc_us) % beacon_interval_us);

        if(next_time < beacon_interval_us / 2){
            if(next_time < PROTECT_AF_MS * 1000){
                BL_LP_LOG("---- Not allowed sleep AF ----");
                return 0;
            }
        }else{
            next_time = beacon_interval_us - next_time;
            if(next_time < PROTECT_BF_MS * 1000){
                BL_LP_LOG("---- Not allowed sleep BF2 ----");
                return 0;
            }
        }
    }

    return 1;
}

/* get the remaining time of the next beacon or dtim, mode 0:dtim, 1:beacon */
int bl_lp_get_next_beacon_time(uint8_t mode)
{
    uint64_t last_beacon_rtc_us, rtc_now_us, rtc_cnt;
    uint32_t dtim_period_us, beacon_interval_us;
    int32_t next_time;

    if (iot2lp_para->last_beacon_stamp_rtc_valid == 0) {
        /* It shouldn't be here */
        return -1;
    }

    /* beacon interval us, TU * 1024 */
    beacon_interval_us = iot2lp_para->beacon_interval_tu * 1024;

    /* Time stamp of the last beacon */
    last_beacon_rtc_us = iot2lp_para->last_beacon_stamp_rtc_us;
    last_beacon_rtc_us -= iot2lp_para->last_beacon_delay_us;

    /* get the current rtc time */
    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    rtc_now_us = BL_PDS_CNT_TO_US(rtc_cnt);

    if (mode) {
        /* get period of beacon */
        dtim_period_us = beacon_interval_us;
    } else {
        /* get period of dtim */
        if (iot2lp_para->continuous_loss_cnt) {
            dtim_period_us = iot2lp_para->dtim_num_correction * beacon_interval_us;
        } else {
            dtim_period_us = iot2lp_para->dtim_num * beacon_interval_us;
        }
    }

    if (last_beacon_rtc_us + dtim_period_us >= rtc_now_us) {
        /* Before the expected time */
        next_time = (last_beacon_rtc_us + dtim_period_us - rtc_now_us);
    } else {
        /* After the expected time */
        next_time = beacon_interval_us - ((rtc_now_us - last_beacon_rtc_us) % beacon_interval_us);
    }

    /* us to ms */
    next_time = next_time / 1000;

    return next_time;
}

int bl_lp_init(void)
{
    assert(!gp_lp_env);

    gp_lp_env = malloc(sizeof(struct lp_env));
    assert(gp_lp_env);

    memset(gp_lp_env, 0, sizeof(struct lp_env));

#if defined(CFG_BL_WIFI_PS_ENABLE)
    bl_lp_fw_init();
#endif

    return 0;
}

int bl_lp_sys_callback_register(bl_lp_cb_t enter_callback, void *enter_arg, bl_lp_cb_t exit_callback, void *exit_arg)
{
    assert(gp_lp_env);

    if (enter_callback == NULL && exit_callback == NULL) {
        return -1;
    }

    gp_lp_env->sys_pre_enter_callback = enter_callback;
    gp_lp_env->sys_after_exit_callback = exit_callback;
    gp_lp_env->sys_enter_arg = enter_arg;
    gp_lp_env->sys_exit_arg = exit_arg;

    return 0;
}

int bl_lp_user_callback_register(bl_lp_cb_t enter_callback, void *enter_arg, bl_lp_cb_t exit_callback, void *exit_arg)
{
    assert(gp_lp_env);

    if (enter_callback == NULL && exit_callback == NULL) {
        return -1;
    }

    gp_lp_env->user_pre_enter_callback = enter_callback;
    gp_lp_env->user_after_exit_callback = exit_callback;
    gp_lp_env->user_enter_arg = enter_arg;
    gp_lp_env->user_exit_arg = exit_arg;

    return 0;
}

void bl_lp_call_user_pre_enter(void)
{
    assert(gp_lp_env);

    if (gp_lp_env->user_pre_enter_callback) {
        gp_lp_env->user_pre_enter_callback(gp_lp_env->user_enter_arg);
    }
}

void bl_lp_call_user_after_exit(void)
{
    assert(gp_lp_env);

    if (gp_lp_env->user_after_exit_callback) {
        gp_lp_env->user_after_exit_callback(gp_lp_env->user_exit_arg);
    }
}

void bl_lp_call_sys_pre_enter(void)
{
    assert(gp_lp_env);

    if (gp_lp_env->sys_pre_enter_callback) {
        gp_lp_env->sys_pre_enter_callback(gp_lp_env->sys_enter_arg);
    }
}

void bl_lp_call_sys_after_exit(void)
{
    assert(gp_lp_env);

    if (gp_lp_env->sys_after_exit_callback) {
        gp_lp_env->sys_after_exit_callback(gp_lp_env->sys_exit_arg);
    }
}

static void bl_lp_soft_irq(void)
{
    uint64_t wakeup_io_bits = iot2lp_para->wake_io_bits;
    uint32_t wakeup_acmp_bits = iot2lp_para->wake_acomp_bits;

    /* disable soft int */
    bflb_irq_disable(MSOFT_IRQn);
    /* clear soft int */
    BL_LP_SOFT_INT_CLEAR;

    if (wakeup_io_bits && lp_soft_callback.wakeup_io_callback) {
        lp_soft_callback.wakeup_io_callback(wakeup_io_bits);
    }

    if (wakeup_acmp_bits && lp_soft_callback.wakeup_acomp_callback) {
        lp_soft_callback.wakeup_acomp_callback(wakeup_acmp_bits);
    }

    /* clear */
    iot2lp_para->wake_io_bits = 0;
}

void bl_lp_wakeup_io_int_register(void (*wakeup_io_callback)(uint64_t wake_up_io_bits))
{
    lp_soft_callback.wakeup_io_callback = wakeup_io_callback;
}

void bl_lp_wakeup_acomp_int_register(void (*wakeup_acomp_callback)(uint32_t wake_up_acomp))
{
    lp_soft_callback.wakeup_acomp_callback = wakeup_acomp_callback;
}

int bl_lp_wakeup_io_get_mode(uint8_t io_num)
{
    uint64_t wakeup_io_bits = iot2lp_para->wake_io_bits;
    uint64_t wakeup_io_edge_bits = iot2lp_para->wake_io_edge_bits;
    uint8_t trig_mode;

    if (io_num >= BL_LP_WAKEUP_IO_MAX_NUM) {
        return -1;
    }

    if ((wakeup_io_bits & ((uint64_t)0x0001 << io_num)) == 0) {
        return 0;
    }

    if (io_num >= 16 && io_num <= 19) {
        trig_mode = g_lp_io_cfg_bak.io_16_19_aon_trig_mode;

        if (trig_mode == BL_LP_AON_IO_TRIG_SYNC_RISING_FALLING_EDGE) {
            if ((wakeup_io_edge_bits & ((uint64_t)0x0001 << io_num)) == 0) {
                return BL_LP_IO_WAKEUP_MODE_FALLING;
            } else {
                return BL_LP_IO_WAKEUP_MODE_RISING;
            }
        } else if (trig_mode == BL_LP_AON_IO_TRIG_SYNC_FALLING_EDGE || trig_mode == BL_LP_AON_IO_TRIG_ASYNC_FALLING_EDGE) {
            return BL_LP_IO_WAKEUP_MODE_FALLING;
        } else if (trig_mode == BL_LP_AON_IO_TRIG_SYNC_RISING_EDGE || trig_mode == BL_LP_AON_IO_TRIG_ASYNC_RISING_EDGE) {
            return BL_LP_IO_WAKEUP_MODE_RISING;
        } else if (trig_mode == BL_LP_AON_IO_TRIG_SYNC_LOW_LEVEL || trig_mode == BL_LP_AON_IO_TRIG_ASYNC_LOW_LEVEL) {
            return BL_LP_IO_WAKEUP_MODE_LOW;
        } else if (trig_mode == BL_LP_AON_IO_TRIG_SYNC_HIGH_LEVEL || trig_mode == BL_LP_AON_IO_TRIG_ASYNC_HIGH_LEVEL) {
            return BL_LP_IO_WAKEUP_MODE_HIGH;
        } else {
            return -1;
        }
    }

    if (io_num <= 7) {
        trig_mode = g_lp_io_cfg_bak.io_0_7_pds_trig_mode;
    } else if (io_num >= 8 && io_num <= 15) {
        trig_mode = g_lp_io_cfg_bak.io_8_15_pds_trig_mode;
    } else if (io_num >= 20 && io_num <= 27) {
        trig_mode = g_lp_io_cfg_bak.io_20_27_pds_trig_mode;
    } else if (io_num >= 28 && io_num <= 34) {
        trig_mode = g_lp_io_cfg_bak.io_28_34_pds_trig_mode;
    } else {
        return -1;
    }

    if (trig_mode == BL_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE || trig_mode == BL_LP_PDS_IO_TRIG_ASYNC_FALLING_EDGE) {
        return BL_LP_IO_WAKEUP_MODE_FALLING;
    } else if (trig_mode == BL_LP_PDS_IO_TRIG_SYNC_HIGH_LEVEL || trig_mode == BL_LP_PDS_IO_TRIG_ASYNC_HIGH_LEVEL) {
        return BL_LP_IO_WAKEUP_MODE_HIGH;
    } else {
        return -1;
    }
}

bl_lp_statistics_t bl_lp_get_statics(void)
{
    bl_lp_statistics_t s;
    s.lpfw_wakeup_cnt = iot2lp_para->lpfw_wakeup_cnt;
    s.lpfw_loss_cnt = iot2lp_para->lpfw_loss_cnt;
    s.lpfw_recv_cnt = iot2lp_para->lpfw_recv_cnt;
    return s;
}

static void bl_lp_vtime_before_sleep(void)
{
    uint64_t rtc_cnt, mtimer_cnt;

    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    mtimer_cnt = CPU_Get_MTimer_Counter();

    /* update timestamp */
    g_rtc_timestamp_before_sleep_us = BL_PDS_CNT_TO_US(rtc_cnt);
    g_mtimer_timestamp_before_sleep_us = mtimer_cnt;

    /* update virtual time base */
    g_virtual_timestamp_base_us += g_mtimer_timestamp_before_sleep_us - g_mtimer_timestamp_after_sleep_us;
}

static void bl_lp_vtime_after_sleep(void)
{
    uint64_t rtc_cnt, mtimer_cnt;

    HBN_Get_RTC_Timer_Val((uint32_t *)&rtc_cnt, (uint32_t *)&rtc_cnt + 1);
    mtimer_cnt = CPU_Get_MTimer_Counter();

    /* update timestamp */
    g_rtc_timestamp_after_sleep_us = BL_PDS_CNT_TO_US(rtc_cnt);
    g_mtimer_timestamp_after_sleep_us = mtimer_cnt;

    /* update virtual time base */
    g_virtual_timestamp_base_us += g_rtc_timestamp_after_sleep_us - g_rtc_timestamp_before_sleep_us;
}

uint64_t bl_lp_get_virtual_us(void)
{
    uint64_t mtimer_cnt;

    mtimer_cnt = CPU_Get_MTimer_Counter();

    return (g_virtual_timestamp_base_us + mtimer_cnt - g_mtimer_timestamp_after_sleep_us);
}

int bl_lp_get_wake_reason(void)
{
    return (int)iot2lp_para->wakeup_reason;
}

int bl_lp_get_wake_io_state(void)
{
    return (int)gp_lp_env->gpio_stat;
}

void bl_lp_set_resume_wifi(void)
{
    assert(gp_lp_env);

    gp_lp_env->wifi_hw_resume = 1;
}

void bl_lp_clear_resume_wifi(void)
{
    assert(gp_lp_env);

    gp_lp_env->wifi_hw_resume = 0;
}

int bl_lp_get_resume_wifi(void)
{
    assert(gp_lp_env);

    return gp_lp_env->wifi_hw_resume;
}

void bl_set_fw_ready(void)
{
    assert(gp_lp_env);

    gp_lp_env->wifi_fw_ready = 1;
}

void bl_clear_fw_ready(void)
{
    assert(gp_lp_env);

    gp_lp_env->wifi_fw_ready = 0;
}

int bl_check_fw_ready(void)
{
    assert(gp_lp_env);

    return gp_lp_env->wifi_fw_ready;
}

static void bl_lp_set_aon_io(int pin, int trigMode, int res_mode)
{
    /* set pin's GLB_GPIO_FUNC_Type as GPIO_FUN_GPIO */
    GLB_GPIO_Cfg_Type gpio_cfg;
    uint8_t pu, pd;

    gpio_cfg.drive = 0;
    gpio_cfg.smtCtrl = 0;
    gpio_cfg.outputMode = 0;
    gpio_cfg.gpioMode = GPIO_MODE_INPUT;
    gpio_cfg.pullType = GPIO_PULL_NONE;
    gpio_cfg.gpioPin = (GLB_GPIO_Type)pin;
    gpio_cfg.gpioFun = GPIO_FUN_GPIO;
    GLB_GPIO_Init(&gpio_cfg);

    if (res_mode == BL_LP_IO_RES_PULL_UP) {
        pu = 1;
        pd = 0;
    } else if (res_mode == BL_LP_IO_RES_PULL_DOWN) {
        pu = 0;
        pd = 1;
    } else {
        pu = 0;
        pd = 0;
    }

    /* set pin's aonPadCfg */
    HBN_AON_PAD_CFG_Type aonPadCfg;
    uint32_t mask = 0;

    aonPadCfg.ctrlEn = 1;
    aonPadCfg.ie = 1;
    aonPadCfg.oe = 0;
    aonPadCfg.pullUp = pu;
    aonPadCfg.pullDown = pd;
    HBN_Aon_Pad_Cfg(ENABLE, (pin - 16), &aonPadCfg);

    mask = BL_RD_REG(HBN_BASE, HBN_IRQ_MODE);
    mask = BL_GET_REG_BITS_VAL(mask, HBN_PIN_WAKEUP_MASK);
    mask = mask & ~(1 << (pin - 16));

    /* set trigMode */
    HBN_Aon_Pad_WakeUpCfg(DISABLE, (HBN_GPIO_INT_Trigger_Type)trigMode, mask, 0, 7);

    /* UnMask Hbn_Irq Wakeup PDS*/
    pm_pds_wakeup_src_en(PDS_WAKEUP_BY_HBN_IRQ_OUT_EN_POS);
}

static void bl_lp_io_wakeup_init(bl_lp_io_cfg_t *io_wakeup_cfg)
{
    uint32_t tmpVal;
    uint64_t io_unmask = io_wakeup_cfg->io_wakeup_unmask;
    uint8_t res_mode, pu, pd;

    if (io_wakeup_cfg == NULL) {
        return;
    }

    /* pds io 0~15 */
    if ((io_wakeup_cfg->io_0_15_ie)) {
        res_mode = io_wakeup_cfg->io_0_15_res;
        if (res_mode == BL_LP_IO_RES_PULL_UP) {
            pu = 1;
            pd = 0;
        } else if (res_mode == BL_LP_IO_RES_PULL_DOWN) {
            pu = 0;
            pd = 1;
        } else {
            pu = 0;
            pd = 0;
        }
        PDS_Set_GPIO_Pad_Pn_Pu_Pd_Ie(PDS_GPIO_GROUP_SET_GPIO0_GPIO15, pu, pd, 1);

        if (io_unmask & 0xFFFF) {
            if (io_unmask & 0xFF) {
                PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_1_GPIO0_GPIO7);
                PDS_Set_GPIO_Pad_IntMode(PDS_GPIO_INT_SET_1_GPIO0_GPIO7, io_wakeup_cfg->io_0_7_pds_trig_mode);
            }

            if (io_unmask & 0xFF00) {
                PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_2_GPIO8_GPIO15);
                PDS_Set_GPIO_Pad_IntMode(PDS_GPIO_INT_SET_2_GPIO8_GPIO15, io_wakeup_cfg->io_8_15_pds_trig_mode);
            }

            tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PD_SET);
            tmpVal &= ~0xFFFF;
            tmpVal |= ((~io_unmask) & 0xFFFF);
            BL_WR_REG(PDS_BASE, PDS_GPIO_PD_SET, tmpVal);

            pm_pds_wakeup_src_en(PDS_WAKEUP_BY_PDS_GPIO_IRQ_EN_POS);
        }
    }

    /* aon io 16 */
    if (io_wakeup_cfg->io_16_ie && ((io_unmask >> 16) & 0x01)) {
        bl_lp_set_aon_io(16, io_wakeup_cfg->io_16_19_aon_trig_mode, io_wakeup_cfg->io_16_res);
    }

    /* aon io 17 */
    if (io_wakeup_cfg->io_17_ie && ((io_unmask >> 17) & 0x01)) {
        bl_lp_set_aon_io(17, io_wakeup_cfg->io_16_19_aon_trig_mode, io_wakeup_cfg->io_17_res);
    }

    /* aon io 18 */
    if (io_wakeup_cfg->io_18_ie && ((io_unmask >> 18) & 0x01)) {
        bl_lp_set_aon_io(18, io_wakeup_cfg->io_16_19_aon_trig_mode, io_wakeup_cfg->io_18_res);
    }

    /* aon io 19 */
    if (io_wakeup_cfg->io_19_ie && ((io_unmask >> 19) & 0x01)) {
        bl_lp_set_aon_io(19, io_wakeup_cfg->io_16_19_aon_trig_mode, io_wakeup_cfg->io_19_res);
    }

    /* pds io 20~34 */
    if ((io_wakeup_cfg->io_20_34_ie)) {
        res_mode = io_wakeup_cfg->io_20_34_res;
        if (res_mode == BL_LP_IO_RES_PULL_UP) {
            pu = 1;
            pd = 0;
        } else if (res_mode == BL_LP_IO_RES_PULL_DOWN) {
            pu = 0;
            pd = 1;
        } else {
            pu = 0;
            pd = 0;
        }
        PDS_Set_GPIO_Pad_Pn_Pu_Pd_Ie(PDS_GPIO_GROUP_SET_GPIO20_GPIO36, pu, pd, 1);

        if ((io_unmask >> 20) & 0x7FFF) {
            if ((io_unmask >> 20) & 0xFF) {
                PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_3_GPIO20_GPIO27);
                PDS_Set_GPIO_Pad_IntMode(PDS_GPIO_INT_SET_3_GPIO20_GPIO27, io_wakeup_cfg->io_20_27_pds_trig_mode);
            }

            if ((io_unmask >> 20) & 0x7F00) {
                PDS_Set_GPIO_Pad_IntClr(PDS_GPIO_INT_SET_4_GPIO28_GPIO34);
                PDS_Set_GPIO_Pad_IntMode(PDS_GPIO_INT_SET_4_GPIO28_GPIO34, io_wakeup_cfg->io_28_34_pds_trig_mode);
            }

            tmpVal = BL_RD_REG(PDS_BASE, PDS_GPIO_PD_SET);
            tmpVal &= ~0xFFFF0000;
            tmpVal |= ((((~io_unmask) >> 20) & 0x7FFF) << 16);
            BL_WR_REG(PDS_BASE, PDS_GPIO_PD_SET, tmpVal);

            pm_pds_wakeup_src_en(PDS_WAKEUP_BY_PDS_GPIO_IRQ_EN_POS);
        }
    }
}

int bl_lp_io_wakeup_cfg(bl_lp_io_cfg_t *io_wakeup_cfg)
{
    gp_lp_io_cfg = io_wakeup_cfg;
    return 0;
}

static void bl_lp_set_gpio_as_analog(uint8_t pin)
{
    GLB_GPIO_Cfg_Type cfg;
    cfg.gpioMode = GPIO_MODE_AF;
    cfg.pullType = GPIO_PULL_NONE;
    cfg.drive = 0;
    cfg.smtCtrl = 0;
    cfg.gpioPin = pin;
    cfg.gpioFun = GPIO_FUN_ANALOG;
    GLB_GPIO_Init(&cfg);
}

uint32_t bl_lp_set_acomp(uint8_t chan, uint8_t pin, uint8_t pos_edge_en, uint8_t neg_edge_en)
{
    struct bflb_acomp_config_s acompCfg = {
        .mux_en = ENABLE,                                      /*!< ACOMP mux enable */
        .pos_chan_sel = AON_ACOMP_CHAN_ADC0,                   /*!< ACOMP negtive channel select */
        .neg_chan_sel = AON_ACOMP_CHAN_VIO_X_SCALING_FACTOR_1, /*!< ACOMP positive channel select */
        .vio_sel = BL616_ACOMP_VREF_1V65,                      /*!< ACOMP vref select */
        .scaling_factor =
            AON_ACOMP_SCALING_FACTOR_1,          /*!< ACOMP level select factor */
        .bias_prog = AON_ACOMP_BIAS_POWER_MODE1, /*!< ACOMP bias current control */
        .hysteresis_pos_volt =
            AON_ACOMP_HYSTERESIS_VOLT_NONE, /*!< ACOMP hysteresis voltage for positive */
        .hysteresis_neg_volt =
            AON_ACOMP_HYSTERESIS_VOLT_NONE, /*!< ACOMP hysteresis voltage for negtive */
    };

    if (pin == GLB_GPIO_PIN_20) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC0;
    } else if (pin == GLB_GPIO_PIN_19) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC1;
    } else if (pin == GLB_GPIO_PIN_2) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC2;
    } else if (pin == GLB_GPIO_PIN_3) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC3;
    } else if (pin == GLB_GPIO_PIN_14) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC4;
    } else if (pin == GLB_GPIO_PIN_13) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC5;
    } else if (pin == GLB_GPIO_PIN_12) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC6;
    } else if (pin == GLB_GPIO_PIN_10) {
        acompCfg.pos_chan_sel = AON_ACOMP_CHAN_ADC7;
    } else {
        return -1;
    }

    /* Config Gpio as Analog */
    bl_lp_set_gpio_as_analog(pin);

    /* Config Comp0/1 */
    bflb_acomp_init(chan, &acompCfg);
    bflb_acomp_enable(chan);

    HBN_Clear_IRQ(HBN_INT_ACOMP0 + chan * 2);

    /* enable/disable POSEDGE */
    if (pos_edge_en) {
        HBN_Enable_AComp_IRQ(chan, HBN_ACOMP_INT_EDGE_POSEDGE);
    } else {
        HBN_Disable_AComp_IRQ(chan, HBN_ACOMP_INT_EDGE_POSEDGE);
    }

    /* enable/disable NEGEDGE */
    if (neg_edge_en) {
        HBN_Enable_AComp_IRQ(chan, HBN_ACOMP_INT_EDGE_NEGEDGE);
    } else {
        HBN_Disable_AComp_IRQ(chan, HBN_ACOMP_INT_EDGE_NEGEDGE);
    }

    /* UnMask Hbn_Irq Wakeup PDS*/
    pm_pds_wakeup_src_en(PDS_WAKEUP_BY_HBN_IRQ_OUT_EN_POS);

    return 0;
}

uint32_t bl_lp_set_acomp_pin(uint32_t acomp_chan)
{
    uint32_t tmpPin = 0;
    if (acomp_chan == AON_ACOMP_CHAN_ADC0) {
        tmpPin = GLB_GPIO_PIN_20;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC1) {
        tmpPin = GLB_GPIO_PIN_19;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC2) {
        tmpPin = GLB_GPIO_PIN_2;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC3) {
        tmpPin = GLB_GPIO_PIN_3;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC4) {
        tmpPin = GLB_GPIO_PIN_14;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC5) {
        tmpPin = GLB_GPIO_PIN_13;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC6) {
        tmpPin = GLB_GPIO_PIN_12;
    } else if (acomp_chan == AON_ACOMP_CHAN_ADC7) {
        tmpPin = GLB_GPIO_PIN_10;
    } else {
        return -1;
    }

    bl_lp_set_gpio_as_analog(tmpPin);
    return 0;
}

static void bl_lp_acomp_wakeup_init(bl_lp_acomp_cfg_t *acomp_wakeup_cfg)
{
    if (acomp_wakeup_cfg->acomp0_en && acomp_wakeup_cfg->acomp0_trig_mode) {
        bl_lp_set_acomp(0, acomp_wakeup_cfg->acomp0_io_num, (acomp_wakeup_cfg->acomp0_trig_mode & 0x02), (acomp_wakeup_cfg->acomp0_trig_mode & 0x01));
    } else {
        bflb_acomp_disable(0);
    }

    if (acomp_wakeup_cfg->acomp1_en && acomp_wakeup_cfg->acomp1_trig_mode) {
        bl_lp_set_acomp(1, acomp_wakeup_cfg->acomp1_io_num, (acomp_wakeup_cfg->acomp1_trig_mode & 0x02), (acomp_wakeup_cfg->acomp1_trig_mode & 0x01));
    } else {
        bflb_acomp_disable(1);
    }

    arch_delay_us(15);
    HBN_Clear_IRQ(HBN_INT_ACOMP0);
    HBN_Clear_IRQ(HBN_INT_ACOMP1);
}

int bl_lp_wakeup_acomp_get_mode(uint8_t acomp_num)
{
    uint8_t wakeup_acomp_bits = iot2lp_para->wake_acomp_bits;
    uint8_t wakeup_acomp_edge_bits = iot2lp_para->wake_acomp_edge_bits;

    if (acomp_num == 0) {
        if (g_lp_acomp_cfg_bak.acomp0_en == BL_LP_ACOMP_DISABLE || (wakeup_acomp_bits & 0x01) == 0) {
            return 0;
        }
        if (g_lp_acomp_cfg_bak.acomp0_trig_mode == BL_LP_ACOMP_TRIG_EDGE_FALLING) {
            return BL_LP_ACOMP_WAKEUP_MODE_FALLING;
        } else if (g_lp_acomp_cfg_bak.acomp0_trig_mode == BL_LP_ACOMP_TRIG_EDGE_RISING) {
            return BL_LP_ACOMP_WAKEUP_MODE_RISING;
        } else if (g_lp_acomp_cfg_bak.acomp0_trig_mode == BL_LP_ACOMP_TRIG_EDGE_FALLING_RISING) {
            if (wakeup_acomp_edge_bits & 0x01) {
                return BL_LP_ACOMP_WAKEUP_MODE_RISING;
            } else {
                return BL_LP_ACOMP_WAKEUP_MODE_FALLING;
            }
        } else {
            return -1;
        }
    }

    if (acomp_num == 1) {
        if (g_lp_acomp_cfg_bak.acomp1_en == BL_LP_ACOMP_DISABLE || (wakeup_acomp_bits & 0x02) == 0) {
            return 0;
        }
        if (g_lp_acomp_cfg_bak.acomp1_trig_mode == BL_LP_ACOMP_TRIG_EDGE_FALLING) {
            return BL_LP_ACOMP_WAKEUP_MODE_FALLING;
        } else if (g_lp_acomp_cfg_bak.acomp1_trig_mode == BL_LP_ACOMP_TRIG_EDGE_RISING) {
            return BL_LP_ACOMP_WAKEUP_MODE_RISING;
        } else if (g_lp_acomp_cfg_bak.acomp1_trig_mode == BL_LP_ACOMP_TRIG_EDGE_FALLING_RISING) {
            if (wakeup_acomp_edge_bits & 0x02) {
                return BL_LP_ACOMP_WAKEUP_MODE_RISING;
            } else {
                return BL_LP_ACOMP_WAKEUP_MODE_FALLING;
            }
        } else {
            return -1;
        }
    }

    return -2;
}

int bl_lp_acomp_wakeup_cfg(bl_lp_acomp_cfg_t *acomp_wakeup_cfg)
{
    gp_lp_acomp_cfg = acomp_wakeup_cfg;
    return 0;
}

static void bl_lp_xip_set_sf_ctrl(spi_flash_cfg_type *pFlashCfg)
{
    struct sf_ctrl_cfg_type sfCtrlCfg;

    sfCtrlCfg.owner = SF_CTRL_OWNER_SAHB;

#ifdef BFLB_SF_CTRL_32BITS_ADDR_ENABLE
    /* address 32bits */
    sfCtrlCfg.en32b_addr = 0;
#endif
    /* bit0-3 for clk delay */
    sfCtrlCfg.clk_delay = (pFlashCfg->clk_delay & 0x0f);
    /* bit0 for clk invert */
    sfCtrlCfg.clk_invert = pFlashCfg->clk_invert & 0x01;
    /* bit1 for rx clk invert */
    sfCtrlCfg.rx_clk_invert = (pFlashCfg->clk_invert >> 1) & 0x01;
    /* bit4-6 for do delay */
    sfCtrlCfg.do_delay = 0;
    /* bit2-4 for di delay */
    sfCtrlCfg.di_delay = 0;
    /* bit5-7 for oe delay */
    sfCtrlCfg.oe_delay = 0;

    bflb_sflash_init(&sfCtrlCfg, NULL);
}

static int bl_lp_xip_read_enable(spi_flash_cfg_type *pFlashCfg, uint8_t cont_read, uint32_t image_offset)
{
    uint32_t tmp[1];

    if ((pFlashCfg->c_read_support & 0x01) == 0) {
        cont_read = 0;
    }

    if (cont_read == 1) {
        bflb_sf_ctrl_set_owner(SF_CTRL_OWNER_SAHB);
        bflb_sflash_read(pFlashCfg, pFlashCfg->io_mode & 0xf, 1, 0x00000000, (uint8_t *)tmp, sizeof(tmp));
    }

    bflb_sf_ctrl_set_flash_image_offset(image_offset, 0, SF_CTRL_FLASH_BANK0);
    bflb_sflash_xip_read_enable(pFlashCfg, pFlashCfg->io_mode & 0xf, cont_read, SF_CTRL_FLASH_BANK0);

    return 0;
}

static void bflb_bootrom_media_boot_set_encrypt(void)
{
    uint32_t encrypt_type = iot2lp_para->encrypt_type; /* the origin val */
    uint32_t xts_mode = iot2lp_para->xts_mode;
    uint32_t img_offset = iot2lp_para->flash_offset;
    uint32_t img_len = iot2lp_para->img_len;

    if (xts_mode) {
        bflb_sf_ctrl_disable_wrap_access(0);
        bflb_sf_ctrl_aes_set_mode(SF_CTRL_AES_XTS_MODE);
        bflb_sf_ctrl_aes_xts_set_key_be(0, NULL, (uint8_t)(encrypt_type));
        bflb_sf_ctrl_aes_xts_set_iv_be(0, iot2lp_para->aesiv, img_offset);

        bflb_sf_ctrl_aes_set_region(0, 1 /*enable this region*/, 1 /*hardware key*/,
                                    img_offset,
                                    img_offset + img_len - 1,
                                    1 /*lock*/);
    } else {
        bflb_sf_ctrl_disable_wrap_access(1);
        bflb_sf_ctrl_aes_set_mode(SF_CTRL_AES_CTR_MODE);
        bflb_sf_ctrl_aes_xts_set_key_be(0, NULL, (uint8_t)(encrypt_type));
        bflb_sf_ctrl_aes_xts_set_iv_be(0, iot2lp_para->aesiv, img_offset);

        bflb_sf_ctrl_aes_set_region(0, 1 /*enable this region*/, 1 /*hardware key*/,
                                    img_offset,
                                    img_offset + img_len - 1,
                                    1 /*lock*/);
    }

    bflb_sf_ctrl_aes_enable_be();
    bflb_sf_ctrl_aes_enable();
}

static void bl_lp_set_power(void)
{
    AON_Trim_DcdcDis();
    HBN_Trim_Ldo33VoutTrim();
    AON_Trim_DcdcVoutSel();
    AON_Trim_DcdcVoutTrim();
    AON_Trim_Ldo11socVoutTrim();
}

void bl_lp_flash_set_cmds(spi_flash_cfg_type *p_flash_cfg)
{
    struct sf_ctrl_cmds_cfg cmds_cfg;

    cmds_cfg.ack_latency = 1;
    cmds_cfg.cmds_core_en = 1;
    cmds_cfg.cmds_en = 1;
    cmds_cfg.cmds_wrap_mode = 1;
    cmds_cfg.cmds_wrap_len = 9;

    if ((p_flash_cfg->io_mode & 0x1f) == SF_CTRL_QIO_MODE) {
        cmds_cfg.cmds_wrap_mode = 2;
        cmds_cfg.cmds_wrap_len = 2;
    }
    bflb_sf_ctrl_cmds_set(&cmds_cfg, 0);
}
static void bflb_bootrom_sboot_set(uint8_t val)
{
    uint32_t tmpVal;
    tmpVal = BL_RD_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL);
    tmpVal = BL_SET_REG_BITS_VAL(tmpVal, TZC_SEC_TZC_SBOOT_DONE, val);
    BL_WR_REG(TZC_SEC_BASE, TZC_SEC_TZC_ROM_TZSRG_CTRL, tmpVal);
}

int format_printf(const char *format, ...);
void bl_lp_xip_recovery()
{
    uint32_t flash_pin = 0;
    uint32_t encrypted = 0;
    uint32_t jdec_id = 0;
    uint32_t timeout = 0;
    uint32_t tmp_val;
    uint32_t ret = 0;
    uint8_t power_trim_disable = 0;
    uint8_t ldo18io_cfg_dis = 0;

    bl_lp_debug_record_time(iot2lp_para, "xip flash start");

    spi_flash_cfg_type *pFlashCfg = (spi_flash_cfg_type *)iot2lp_para->flash_cfg;

    /* called from lowpower, open clock */
    *((volatile uint32_t *)0x20000584) = 0xffffffff;

    //EF_Ctrl_Load_Efuse_R0();

    /* system power set */
    power_trim_disable = BL_RD_WORD(0x2005605C);
    power_trim_disable = (power_trim_disable >> 22) & 0x1;
    if (!power_trim_disable) {
        bl_lp_set_power();
    }
    ldo18io_cfg_dis = BL_RD_WORD(0x20056060);
    ldo18io_cfg_dis = (ldo18io_cfg_dis >> 27) & 0x1;
    if (!ldo18io_cfg_dis) {
        GLB_Trim_Ldo18ioVoutSel();
        GLB_Trim_Ldo18ioVoutTrim();
        GLB_Trim_Ldo18ioBypass();
    }

    /* power on flash power */
    /* pu_ldo18flash=1 */
    ret = BL_RD_REG(GLB_BASE, GLB_LDO18IO);
    ret = BL_SET_REG_BIT(ret, GLB_PU_LDO18IO);
    BL_WR_REG(GLB_BASE, GLB_LDO18IO, ret);

    arch_delay_us(200);

    /* get flash gpio info */
    flash_pin = BL_RD_WORD(0x2005605C);
    flash_pin = (flash_pin >> 14) & 0x3f;
    /* init flash gpio */
    bflb_sf_cfg_init_flash_gpio((uint8_t)flash_pin, 0);

    uint32_t REG_PLL_BASE_ADDRESS = 0;
    uint32_t tmpVal = 0;

    REG_PLL_BASE_ADDRESS = GLB_BASE + GLB_WIFI_PLL_CFG0_OFFSET;
    if(iot2lp_para->flash_clk==0){
        tmpVal = BL_RD_WORD(REG_PLL_BASE_ADDRESS + 4 * 8);
        tmpVal = BL_SET_REG_BIT(tmpVal, GLB_WIFIPLL_EN_DIV8); //120M
        BL_WR_WORD(REG_PLL_BASE_ADDRESS + 4 * 8, tmpVal);
    } else if(iot2lp_para->flash_clk==2){
        GLB_Config_AUDIO_PLL_To_491P52M();
    } else if(iot2lp_para->flash_clk==3){
        tmpVal = BL_RD_WORD(REG_PLL_BASE_ADDRESS + 4 * 8);
        tmpVal = BL_SET_REG_BIT(tmpVal, GLB_WIFIPLL_EN_DIV12); //80M
        BL_WR_WORD(REG_PLL_BASE_ADDRESS + 4 * 8, tmpVal);
    } else if(iot2lp_para->flash_clk==5){
        tmpVal = BL_RD_WORD(REG_PLL_BASE_ADDRESS + 4 * 8);
        tmpVal = BL_SET_REG_BIT(tmpVal, GLB_WIFIPLL_EN_DIV10); //96M
        BL_WR_WORD(REG_PLL_BASE_ADDRESS + 4 * 8, tmpVal);
    }

    /* set flash clock */
    GLB_Set_SF_CLK(1, iot2lp_para->flash_clk, iot2lp_para->flash_clk_div);

    /* update flash controller */
    bl_lp_xip_set_sf_ctrl(pFlashCfg);

    /* set flash cmds */
    bl_lp_flash_set_cmds(pFlashCfg);

    /* do flash recovery */
    bflb_sflash_release_powerdown(pFlashCfg);
    arch_delay_us(120);
    timeout = 0;
    do {
        timeout++;
        if (timeout > 2000) {
            break;
        } else if (timeout > 0) {
            arch_delay_us(10);
        }
        bflb_sflash_reset_continue_read(pFlashCfg);
        /* Send software reset command(80bv has no this command)to deburst wrap for ISSI like */
        bflb_sflash_software_reset(pFlashCfg);
        /* Disable burst may be removed(except for 80BV) and only work with winbond,but just for make sure */
        bflb_sflash_write_enable(pFlashCfg);
        /* For disable command that is setting register instaed of send command, we need write enable */
        /* Delete this line before the other flash_2_wires glitch */
        //SFlash_DisableBurstWrap(pFlashCfg);

        bflb_sflash_set_spi_mode(SF_CTRL_SPI_MODE);
        bflb_sflash_get_jedecid(pFlashCfg, (uint8_t *)&jdec_id);
        /* Dummy disable burstwrap for make sure */
        bflb_sflash_write_enable(pFlashCfg);
    } while (jdec_id != iot2lp_para->flash_jdec_id);

    GLB_Set_Flash_Id_Value(jdec_id);

    timeout = 0;
    do {
        timeout++;
        if (timeout > 50000) {
            break;
        } else if (timeout > 0) {
            arch_delay_us(10);
        }
        ret = bflb_sflash_busy(pFlashCfg);
    } while (1 == ret);

    /* set flash into 32bits address mode,maybe flash is already in 32bits mode,but it's OK*/
    if (pFlashCfg->io_mode & 0x20) {
        bflb_sflash_set_32bits_addr_mode(pFlashCfg, ENABLE);
    }

#if 0
    /* Enable QE again */
    if ((pFlashCfg->io_mode & 0x0f) == SF_CTRL_QO_MODE || (pFlashCfg->io_mode & 0x0f) == SF_CTRL_QIO_MODE) {
        SFlash_Qspi_Enable(pFlashCfg);
    }
#endif
    /* set burst wrap */
    if (((pFlashCfg->io_mode >> 4) & 0x01) == 1) {
        L1C_Set_Wrap(DISABLE); //useless, reserved for bootrom architecture
    } else {
        L1C_Set_Wrap(ENABLE); //useless, reserved for bootrom architecture
        /* For command that is setting register instead of send command, we need write enable */
        bflb_sflash_write_enable(pFlashCfg);
        if ((pFlashCfg->io_mode & 0x0f) == SF_CTRL_QO_MODE || (pFlashCfg->io_mode & 0x0f) == SF_CTRL_QIO_MODE) {
            bflb_sflash_set_burst_wrap(pFlashCfg);
        }
    }

    timeout = 0;
    do {
        timeout++;
        bflb_sflash_read(pFlashCfg, pFlashCfg->io_mode & 0xf, 0, 0x00000000, (uint8_t *)&tmp_val, 4);
        arch_delay_us(10);
    } while (timeout < 1000 && tmp_val != 0x504e4642);
    bflb_bootrom_sboot_set(0xf);

#if LPFW_LOG_EN
    format_printf("encrypt_type %d\r\n", iot2lp_para->encrypt_type);
    format_printf("xts_mode %d\r\n", iot2lp_para->xts_mode);
    format_printf("img_offset %08x\r\n", iot2lp_para->flash_offset);
    format_printf("iv[0]= %02x\r\n", iot2lp_para->aesiv[0]);
    format_printf("iv[1]= %02x\r\n", iot2lp_para->aesiv[1]);
    format_printf("iv[2]= %02x\r\n", iot2lp_para->aesiv[2]);
    format_printf("iv[3]= %02x\r\n", iot2lp_para->aesiv[3]);
    format_printf("iv[4]= %02x\r\n", iot2lp_para->aesiv[4]);
    format_printf("iv[5]= %02x\r\n", iot2lp_para->aesiv[5]);
    format_printf("iv[6]= %02x\r\n", iot2lp_para->aesiv[6]);
    format_printf("iv[7]= %02x\r\n", iot2lp_para->aesiv[7]);
    format_printf("iv[8]= %02x\r\n", iot2lp_para->aesiv[8]);
    format_printf("iv[9]= %02x\r\n", iot2lp_para->aesiv[9]);
    format_printf("iv[10]= %02x\r\n", iot2lp_para->aesiv[10]);
    format_printf("iv[11]= %02x\r\n", iot2lp_para->aesiv[11]);
    format_printf("iv[12]= %02x\r\n", iot2lp_para->aesiv[12]);
    format_printf("iv[13]= %02x\r\n", iot2lp_para->aesiv[13]);
    format_printf("iv[14]= %02x\r\n", iot2lp_para->aesiv[14]);
    format_printf("iv[15]= %02x\r\n", iot2lp_para->aesiv[15]);
#endif

    encrypted = BL_RD_WORD(0x20056000);
    encrypted = encrypted & 0x03;
    if (encrypted == 0) {
        bl_lp_xip_read_enable(pFlashCfg, 1 /* cont read*/, iot2lp_para->flash_offset);
    } else {
        bflb_bootrom_media_boot_set_encrypt();
        bl_lp_xip_read_enable(pFlashCfg, 0 /* not cont read*/, iot2lp_para->flash_offset);
    }

    BL_WR_REG(GLB_BASE, GLB_UART_CFG1, 0xffffffff);
    BL_WR_REG(GLB_BASE, GLB_UART_CFG2, 0x0000ffff);

    iot2lp_para->wakeup_flag = 1;

    //csi_icache_enable();
    //csi_dcache_enable();

    csi_icache_invalid();
    csi_dcache_invalid();

    bl_lp_debug_record_time(iot2lp_para, "xip flash end");

    void (*pFunc)(uint32_t);
    pFunc = (void (*)(uint32_t))iot2lp_para->app_entry;
    pFunc(iot2lp_para->args[0]);
}

#ifndef BL_WIFI_LP_FW
static uint32_t ps_mamager_event = 0;

void bl_pm_event_bit_set(enum PSM_EVENT event_bit)
{
    ps_mamager_event |= 1 << event_bit;
}

void bl_pm_event_bit_clear(enum PSM_EVENT event_bit)
{
    ps_mamager_event &= ~(1 << event_bit);
}

uint32_t bl_pm_event_get(void)
{
    return ps_mamager_event;
}

void bl_pm_enter_ps(void)
{
    wifi_mgmr_sta_ps_enter();
    bl_pm_event_bit_clear(PSM_EVENT_PS);
}

void bl_pm_exit_ps(void)
{
    wifi_mgmr_sta_ps_exit();
    bl_pm_event_bit_set(PSM_EVENT_PS);
}

void bl_pm_enter_pds(void *arg)
{
    return;
}

void bl_lp_turnoff_rf(void)
{
    PDS_Power_Off_WB();
}

void bl_lp_turnon_rf(void)
{
    PDS_Power_On_WB();
}

#endif
