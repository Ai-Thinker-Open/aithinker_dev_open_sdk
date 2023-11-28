#ifndef __BL616_LP_H__
#define __BL616_LP_H__
#include <stdint.h>

#define IOT2LP_PARA_ADDR 0x20010400

#define iot2lp_para             ((iot2lp_para_t *)IOT2LP_PARA_ADDR)
#define LPFW_STATIC_VAL_ADDR    (iot2lp_para->lpfw_static_val)
#define LPFW_CONSTANTS          (LPFW_STATIC_VAL_ADDR->lpfw_constants)
#define LPFW_VARIABLES          (LPFW_STATIC_VAL_ADDR->lpfw_variables)

#define RAM_WIFI_START_ADD          (LPFW_CONSTANTS.wifi_buff_start_addr)
#define ADDR_BEACON_START           (LPFW_CONSTANTS.wifi_beacon_start_addr)
#define ADDR_BEACON_BSSID           (LPFW_CONSTANTS.wifi_beacon_bssid_addr)
#define ADDR_BEACON_FIXED_PART      (LPFW_CONSTANTS.wifi_beacon_fixed_part_addr)
#define ADDR_BEACON_INTERVAL_PART   (LPFW_CONSTANTS.wifi_beacon_interval_part_addr)
#define ADDR_BEACON_VARIABLE_PART   (LPFW_CONSTANTS.wifi_beacon_variable_part_addr)

#define WIFI_RX_READY_MTIMER        (LPFW_VARIABLES.wifi_rx_ready_mtimer)
#define WIFI_RX_START_MTIMER        (LPFW_VARIABLES.wifi_rx_start_mtimer)
#define INTERVAL_START_MTIMER       (LPFW_VARIABLES.interval_start_mtimer)
#define LPFW_SLEEP_MTIMER           (LPFW_VARIABLES.lpfw_sleep_mtimer)
#define MTIMER_TIMEOUT              (LPFW_VARIABLES.mtimer_timeout)
#define BCN_DELAY_AVE               (LPFW_VARIABLES.bcn_delay_ave)

#define ADDR_BEACON_SUBTYPE         ADDR_BEACON_START

#define HBN_SYS_RESET_REASON        (iot2lp_para->reset_keep.reset_reason)
#define HBN_SYS_RESET_REASON_CHK    (iot2lp_para->reset_keep.reset_reason_chk)

#define BEACON_DATA_RATE            (iot2lp_para->beacon_leg_rate)

#define PDS_WAKEUP_MINI_LIMIT_US  (1220)
#define PDS_WAKEUP_MINI_LIMIT_CNT (40)

#define PDS_WAKEUP_DELAY_US  (1000)
#define PDS_WAKEUP_DELAY_CNT (33)

enum PSM_EVENT {
    PSM_EVENT_SCAN = 0,
    PSM_EVENT_CONNECT,
    PSM_EVENT_DISCONNECT,
    PSM_EVENT_PS,
    PSM_EVENT_APP,
    PSM_EVENT_LP_BUF_REUSED,
};

#define TIME_DEBUG_NUM_MAX 10

typedef struct {
    uint64_t time_stamp_us;
    char *info_str;
    uint32_t trig_cnt;
}lp_fw_time_debug_t;

typedef struct {
    /* lpfw timer */
    volatile uint32_t wifi_rx_ready_mtimer;
    volatile uint32_t wifi_rx_start_mtimer;
    volatile uint32_t interval_start_mtimer;
    volatile uint32_t lpfw_sleep_mtimer;
    volatile uint32_t mtimer_timeout;
    volatile int32_t bcn_delay_ave;
} lp_fw_variables_t;

typedef struct {
    /* wifi parse addr */
    volatile uint32_t wifi_buff_start_addr;
    volatile uint32_t wifi_beacon_start_addr;
    volatile uint32_t wifi_beacon_bssid_addr;
    volatile uint32_t wifi_beacon_fixed_part_addr;
    volatile uint32_t wifi_beacon_interval_part_addr;
    volatile uint32_t wifi_beacon_variable_part_addr;
} lp_fw_constants_t;

typedef struct {
    lp_fw_variables_t lpfw_variables;
    lp_fw_constants_t lpfw_constants;
} lp_fw_static_variable_t;

typedef struct {
    uint32_t pattern; /*0xAA5555AA*/
    lp_fw_static_variable_t* lpfw_static_val;
    /* flash recovery */
    uint32_t mcu_sts;
    void *flash_cfg;
    uint32_t flash_jdec_id;
    uint32_t encrypt_type;
    uint32_t xts_mode;
    uint32_t img_len;
    uint8_t aesiv[16];
    uint8_t flash_clk;
    uint8_t flash_clk_div;
    uint8_t do_xip_recovery;
    uint8_t xip_rsvd;

    /* wifi para */
    uint8_t ap_channel;
    uint8_t rsvd_1;
    uint16_t aid;
    uint8_t bssid[6];
    uint8_t local_mac[6];
    uint32_t beacon_interval_tu;   /* beacon interval tu */
    /* system para */
    uint32_t wakeup_flag;
    uint32_t flash_offset;
    uint32_t app_entry;
    uint32_t args[4];
    uint32_t cpu_regs[32];
    uint32_t tim_wake_enable;
    uint32_t rtc_wakeup_en;  /* rtc wakeup enable */
    uint64_t rtc_wakeup_cnt; /* The value of the next rtc wake up */
    int32_t wakeup_reason;   /* reason of wake up */
    int32_t lpfw_wakeup_cnt;
    uint64_t wake_io_bits;   /* wake io bits */
    uint64_t wake_io_edge_bits;   /* wake edge bits */
    uint8_t wake_acomp_bits;   /* wake acomp bits */
    uint8_t wake_acomp_edge_bits;   /* wake acomp bits */
    uint8_t transport_mode_enable;
    uint8_t feed_wdt_gpio;
    /* beacon  */
    int32_t tpre;

    int32_t last_sleep_error_us;
    uint32_t last_beacon_stamp_rtc_valid;
    uint64_t last_beacon_stamp_rtc_us;    /*  */
    uint64_t last_beacon_stamp_beacon_us; /* beacon delay */
    int32_t last_beacon_delay_us;

    uint32_t mtimer_timeout_mini_us;
    uint32_t mtimer_timeout_max_us;
    uint8_t mtimer_timeout_en;

    uint8_t continuous_loss_cnt;
    uint8_t timeout_cnt_th; /*  */
    uint8_t ap_loss_cnt_th; /* Must be greater than timeout_cnt_th */


    uint8_t dtim_num;
    uint8_t dtim_num_correction;     /* dtim_num when a beacon is lost */
    uint8_t beacon_leg_rate;
    uint8_t debug_io;   /* io toggle after wifi init*/

    int32_t rtc32k_jitter_error_ppm; /* Jitter error compensation, ppm (10^6) */

    /* Statistical analysis */
    uint8_t bcn_delay_sliding_win_buff[16];
    uint8_t bcn_delay_sliding_win_size;
    uint8_t bcn_delay_sliding_win_point;
    uint8_t bcn_delay_sliding_win_status;
    uint64_t bcn_delay_stamp_last_us;

    uint8_t rssi_sliding_win_buff[10];
    uint8_t rssi_sliding_win_size;
    uint8_t rssi_sliding_win_point;
    uint8_t rssi_sliding_win_status;

    /* rtc32k_trim */
    uint32_t rc32k_auto_cal_en;
    uint32_t last_rc32trim_stamp_valid;
    uint64_t last_rc32trim_stamp_rtc_us;
    uint64_t last_rc32trim_stamp_beacon_us;
    int32_t rc32k_fr_ext;
    int32_t rtc32k_error_ppm; /*  */
    /* other */
    uint32_t lpfw_recv_cnt;
    uint32_t lpfw_loss_cnt;
    uint32_t app_recv_cnt;
    uint32_t app_loss_cnt;

    /* RTC-UTC Timestamp */
    uint32_t last_ntp_sync_timestamp;
    uint64_t last_ntp_sync_rtc;

    lp_fw_time_debug_t *time_debug;

    /* The parameter that remains after the reset, it has to be at the back */
    struct {
        /* reset reason */
        uint32_t reset_reason;
        uint32_t reset_reason_chk;
    } reset_keep;

} iot2lp_para_t;

typedef int (*bl_lp_cb_t)(void *arg);

/* Cause of wakeup */
#define LPFW_WAKEUP_UNKOWN   0
#define LPFW_WAKEUP_TIME_OUT (1 << 0)
#define LPFW_WAKEUP_WIFI     (1 << 1)
#define LPFW_WAKEUP_AP_LOSS  (1 << 2)
#define LPFW_WAKEUP_IO       (1 << 3)
#define LPFW_WAKEUP_ACOMP    (1 << 4)
#define LPFW_WAKEUP_BLE      (1 << 5)

/* beacon stamp valid type */
#define BEACON_STAMP_LPFW 1
#define BEACON_STAMP_APP  2

/* Conversion of units */
#define BL_US_TO_PDS_CNT(us)  ((us)*512 / 15625)  /* us * 32768 / 1000 /1000 */
#define BL_MS_TO_PDS_CNT(ms)  ((ms)*4096 / 125)   /* ms * 32768 / 1000 */
#define BL_PDS_CNT_TO_US(cnt) ((cnt)*15625 / 512) /* cnt / 32768 * 1000 /1000 */
#define BL_PDS_CNT_TO_MS(cnt) ((cnt)*125 / 4096)  /* cnt / 32768 * 1000 */

/*  */
#define PROTECT_AF_MS (7)
#define PROTECT_BF_MS (4)

#define BL_DTIM_NUM   (10)

typedef struct {
    uint8_t tim_wakeup_en : 1; /* 1: enable tim wakeup */
    uint8_t lpfw_copy : 1;  /* copy lpfw or not */
    uint8_t channel;
    uint8_t bssid[6];
    uint8_t mac[6];
    uint8_t dtim_num;
    uint8_t dtim_origin;
    uint32_t mtimer_timeout_mini_us;
    uint32_t mtimer_timeout_max_us;

    uint32_t mcu_sts;
    uint32_t aid;
    uint32_t tpre;

    uint8_t timeout_cnt_th;
    uint8_t ap_loss_cnt_th;

    uint64_t rtc_wakeup_cmp_cnt; /* 0: disable, use rtc_timeout_us */
    uint64_t rtc_timeout_us; /* 0: disable rtc wakeup */

    int32_t wakeup_reason;  /* Cause of wakeup */
    uint32_t lpfw_recv_cnt; /* count of loss packet during rtc_timeout_ms */
    uint32_t lpfw_loss_cnt; /* count of wakeup during rtc_timeout_ms */
} bl_lp_fw_cfg_t;

extern bl_lp_fw_cfg_t lpfw_cfg;

#define IOT2BOOT2_PARA_ADDR     0x20010300
#define iot2boot2_para          ((iot2boot2_para_t *)IOT2BOOT2_PARA_ADDR)

#define HAL_BOOT2_LP_RETURN_RESET 0              // RESET
#define HAL_BOOT2_LP_RETURN_HBN_WAKEUP 1         // IO\ACOMP wakeup hbn
#define HAL_BOOT2_LP_RETURN_FEED_WDT_OVER 2      // feed_wdt_max_continue_time <= feed_wdt_continue_time
#define HAL_BOOT2_LP_RETURN_WDT_DISABLE 3        // Disable WDT


typedef struct {
    uint32_t hbn_pattern; /*0x55AAAA55*/
    uint32_t wakeup_reason;
    uint64_t hbn_sleep_period;
    uint8_t hbn_level;
    uint8_t hal_boot2_lp_ret;
    uint8_t iot2boot2_para_rsv[2];
    uint8_t wkup_io_bits;
    uint8_t wkup_io_edge_bits;
    uint8_t wkup_acomp_bits;
    uint8_t wkup_acomp_edge_bits;
    uint8_t hbn_io_trigger_type;
    uint64_t wkup_rtc_cnt;
    uint32_t wdt_pattern; /*0xAAAA5555*/
    uint8_t feed_wdt_io;
    uint8_t feed_wdt_type;
    uint32_t feed_wdt_continue_times;
    uint32_t feed_wdt_max_continue_times;
} iot2boot2_para_t;

typedef struct {
  uint64_t hbn_sleep_cnt;
  uint8_t hbn_level;
} bl_lp_hbn_fw_cfg_t;

/******************** io wakeup cfg ********************/

#define BL_LP_WAKEUP_IO_MAX_NUM     35

/** @defgroup BL_LP_IO_INPUT_EN input enable
  * @{
  */
#define BL_LP_IO_INPUT_DISABLE  0
#define BL_LP_IO_INPUT_ENABLE   1
/**
  * @}
  */

/** @defgroup BL_LP_AON_IO_TRIG aon io wakeup trigger mode
  * @{
  */
#define BL_LP_AON_IO_TRIG_SYNC_FALLING_EDGE        (0x0) /*!< AON_IO INT trigger type: sync falling edge trigger */
#define BL_LP_AON_IO_TRIG_SYNC_RISING_EDGE         (0x1) /*!< AON_IO INT trigger type: sync rising edge trigger */
#define BL_LP_AON_IO_TRIG_SYNC_LOW_LEVEL           (0x2) /*!< AON_IO INT trigger type: sync low level trigger */
#define BL_LP_AON_IO_TRIG_SYNC_HIGH_LEVEL          (0x3) /*!< AON_IO INT trigger type: sync high level trigger */
#define BL_LP_AON_IO_TRIG_SYNC_RISING_FALLING_EDGE (0x4) /*!< AON_IO INT trigger type: sync rising falling edge trigger */
#define BL_LP_AON_IO_TRIG_ASYNC_FALLING_EDGE       (0x8) /*!< AON_IO INT trigger type: async falling edge trigger */
#define BL_LP_AON_IO_TRIG_ASYNC_RISING_EDGE        (0x9) /*!< AON_IO INT trigger type: async rising edge trigger */
#define BL_LP_AON_IO_TRIG_ASYNC_LOW_LEVEL          (0xA) /*!< AON_IO INT trigger type: async low level trigger */
#define BL_LP_AON_IO_TRIG_ASYNC_HIGH_LEVEL         (0xB) /*!< AON_IO INT trigger type: async high level trigger */
#define BL_LP_AON_IO_TRIG_NONE                     (0xF) /*!< AON_IO INT trigger type: none */
/**
  * @}
  */

/** @defgroup BL_LP_PDS_IO_TRIG aon io wakeup trigger mode
  * @{
  */
#define BL_LP_PDS_IO_TRIG_SYNC_FALLING_EDGE  (0x0) /*!< PDS_IO INT trigger type: sync falling edge trigger */
#define BL_LP_PDS_IO_TRIG_SYNC_RISING_EDGE   (0x1) /*!< PDS_IO INT trigger type: sync rising edge trigger */
#define BL_LP_PDS_IO_TRIG_SYNC_HIGH_LEVEL    (0x3) /*!< PDS_IO INT trigger type: sync high level trigger */
#define BL_LP_PDS_IO_TRIG_ASYNC_FALLING_EDGE (0x8) /*!< PDS_IO INT trigger type: async falling edge trigger */
#define BL_LP_PDS_IO_TRIG_ASYNC_RISING_EDGE  (0x9) /*!< PDS_IO INT trigger type: async rising edge trigger */
#define BL_LP_PDS_IO_TRIG_ASYNC_HIGH_LEVEL   (0xB) /*!< PDS_IO INT trigger type: async high level trigger */
#define BL_LP_PDS_IO_TRIG_NONE               (0xF) /*!< PDS_IO INT trigger type: none */
/**
  * @}
  */

/** @defgroup BL_LP_RES resistors select
  * @{
  */
#define BL_LP_IO_RES_PULL_UP    1
#define BL_LP_IO_RES_PULL_DOWN  2
#define BL_LP_IO_RES_NONE       3
/**
  * @}
  */

typedef struct{
    /* input enable, use @ref BL_LP_IO_INPUT_EN */
    uint8_t io_0_15_ie;
    uint8_t io_16_ie;
    uint8_t io_17_ie;
    uint8_t io_18_ie;
    uint8_t io_19_ie;
    uint8_t io_20_34_ie;

    /* wake up mask */
    uint64_t io_wakeup_unmask; /* bit[0:34] --> GPIO0 ~ GPIO34 */

    /* trigger mode */
    uint8_t io_0_7_pds_trig_mode;   /* use @ref BL_LP_PDS_IO_TRIG */
    uint8_t io_8_15_pds_trig_mode;  /* use @ref BL_LP_PDS_IO_TRIG */
    uint8_t io_16_19_aon_trig_mode; /* aon io, use @ref BL_LP_AON_IO_TRIG, full mode support */
    uint8_t io_20_27_pds_trig_mode; /* use @ref BL_LP_PDS_IO_TRIG */
    uint8_t io_28_34_pds_trig_mode; /* use @ref BL_LP_PDS_IO_TRIG */

    /* resistors */
    uint8_t io_0_15_res;
    uint8_t io_16_res;
    uint8_t io_17_res;
    uint8_t io_18_res;
    uint8_t io_19_res;
    uint8_t io_20_34_res;

}bl_lp_io_cfg_t;

#define BL_LP_IO_WAKEUP_MODE_LOW        1
#define BL_LP_IO_WAKEUP_MODE_HIGH       2
#define BL_LP_IO_WAKEUP_MODE_FALLING    3
#define BL_LP_IO_WAKEUP_MODE_RISING     4

/******************** acomp wakeup cfg ********************/

#define BL_LP_WAKEUP_ACOMP_MAX_NUM  2

/** @defgroup BL_LP_ACOMP_EN acomp input enable
  * @{
  */
#define BL_LP_ACOMP_DISABLE  0
#define BL_LP_ACOMP_ENABLE   1
/**
  * @}
  */

/** @defgroup BL_LP_ACOMP_TRIG acomp wakeup trigger mode
  * @{
  */
#define BL_LP_ACOMP_TRIG_EDGE_FALLING           1
#define BL_LP_ACOMP_TRIG_EDGE_RISING            2
#define BL_LP_ACOMP_TRIG_EDGE_FALLING_RISING    3
/**
  * @}
  */

typedef struct{
    /* input enable, use @ref BL_LP_ACOMP_EN */
    uint8_t acomp0_en;
    uint8_t acomp1_en;

    /* Map to pins num, range: 2, 3, 10, 12, 13, 14, 19 */
    uint8_t acomp0_io_num;
    uint8_t acomp1_io_num;

    /* trigger mode, use @ref BL_LP_ACOMP_TRIG  */
    uint8_t acomp0_trig_mode;
    uint8_t acomp1_trig_mode;

}bl_lp_acomp_cfg_t;

#define BL_LP_ACOMP_WAKEUP_MODE_FALLING    1
#define BL_LP_ACOMP_WAKEUP_MODE_RISING     2

/******************** acomp end ********************/

#define BL_LP_SOFT_INT_TRIG (*(volatile uint32_t *)(0xE0000000) = 1)
#define BL_LP_SOFT_INT_CLEAR (*(volatile uint32_t *)(0xE0000000) = 0)

typedef struct {
    void (*wakeup_io_callback)(uint64_t wake_up_io_bits);
    void (*wakeup_acomp_callback)(uint32_t wake_up_acomp);
} bl_lp_soft_irq_callback_t;

/* statistics info */
typedef struct {
    int32_t lpfw_wakeup_cnt;
    uint32_t lpfw_recv_cnt;
    uint32_t lpfw_loss_cnt;
} bl_lp_statistics_t;

/* LP_HOOK */
void lp_hook_pre_sys(void *) __attribute__((weak));
void lp_hook_pre_user(void *) __attribute__((weak));
void lp_hook_pre_sleep(iot2lp_para_t *) __attribute__((weak));
void lp_hook_post_sys(iot2lp_para_t *) __attribute__((weak));

int bl_lp_init(void);
int bl_lp_sys_callback_register(bl_lp_cb_t enter_callback, void *enter_arg, bl_lp_cb_t exit_callback, void *exit_arg);
int bl_lp_user_callback_register(bl_lp_cb_t enter_callback, void *enter_arg, bl_lp_cb_t exit_callback, void *exit_arg);

void bl_lp_call_user_pre_enter(void);
void bl_lp_call_sys_pre_enter(void);

void bl_lp_call_user_after_exit(void);
void bl_lp_call_sys_after_exit(void);

int bl_lp_beacon_interval_update(uint16_t beacon_interval_tu);

int bl_lp_fw_enter(bl_lp_fw_cfg_t *bl_lp_fw_cfg);
uint8_t bl_lp_get_hal_boot2_lp_ret(void);
uint32_t bl_lp_get_hbn_wakeup_reason(void);
void bl_lp_get_hbn_wakeup_io_bits(uint8_t* io_bits,uint8_t* io_edge_bits);
void bl_lp_get_hbn_wakeup_acomp_bits(uint8_t* acomp_bits,uint8_t* acomp_edge_bits);
void bl_lp_hbn_init(uint8_t wdt_en,uint8_t feed_wdt_pin,uint8_t feed_wdt_type,uint32_t feed_wdt_max_continue_times);
int bl_lp_hbn_enter(bl_lp_hbn_fw_cfg_t *bl_lp_hbn_fw_cfg);
int bl_lp_fw_enter_check_allow(void);
int bl_lp_get_next_beacon_time(uint8_t mode);
int bl_lp_get_wake_reason(void);
int bl_lp_get_wake_io_state(void);
int bl_lp_update_beacon_stamp(uint64_t beacon_timestamp);
void bl_lp_rtc_use_xtal32K(void);
void bl_lp_rtc_use_rc32k(void);
uint64_t bl_lp_get_virtual_us(void);

int bl_lp_io_wakeup_cfg(bl_lp_io_cfg_t *io_wakeup_cfg);
void bl_lp_wakeup_io_int_register(void (*wakeup_io_callback)(uint64_t wake_up_io_bits));
int bl_lp_wakeup_io_get_mode(uint8_t io_num);

int bl_lp_acomp_wakeup_cfg(bl_lp_acomp_cfg_t *acomp_wakeup_cfg);
void bl_lp_wakeup_acomp_int_register(void (*wakeup_acomp_callback)(uint32_t wake_up_acomp));
int bl_lp_wakeup_acomp_get_mode(uint8_t acomp_num);

void bl_lp_debug_record_time(iot2lp_para_t* iot_lp_para, char *info_str);
void bl_lp_debug_clean_time(iot2lp_para_t* iot_lp_para);
void bl_lp_debug_dump_time(iot2lp_para_t* iot_lp_para);

void bl_lp_set_resume_wifi(void);
void bl_lp_clear_resume_wifi(void);
int bl_lp_get_resume_wifi(void);
void bl_set_fw_ready(void);
void bl_clear_fw_ready(void);
int bl_check_fw_ready(void);
void bl_pm_event_bit_set(enum PSM_EVENT event_bit);
void bl_pm_event_bit_clear(enum PSM_EVENT event_bit);
uint32_t bl_pm_event_get(void);
void bl_pm_enter_ps(void);
void bl_pm_exit_ps(void);
void bl_lp_xip_recovery(void);
void bl_lp_turnoff_rf(void);
void bl_lp_turnon_rf(void);
#endif
