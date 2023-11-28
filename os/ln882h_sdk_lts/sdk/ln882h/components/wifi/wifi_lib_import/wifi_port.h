#ifndef  __MACLIB_PORT_H__
#define  __MACLIB_PORT_H__

#include "ln_utils.h"
#include "ln_compiler.h"
#include "ln_types.h"
#include "ln_wifi_err.h"
#include <stdarg.h>

typedef size_t (*wlib_pvtcmd_output_pfn)(const char *format, va_list args);

#define WLIB_DR_11B_MAX_NUM (4)
#define WLIB_DR_11G_MAX_NUM (8)
#define WLIB_DR_11N_MAX_NUM (8)
#define WLIB_DR_BR_MAX_NUM  (8)

/**
 * wlib_wifi_data_rate_t
 * Configure the unused rate to 0
*/
typedef struct
{
    uint8_t dr_11b[WLIB_DR_11B_MAX_NUM];  /* 1,  2,  5, 11                 */ /* Use 5 for 5.5 */
    uint8_t dr_11g[WLIB_DR_11G_MAX_NUM];  /* 6,  9, 12, 18, 24, 36, 48, 54 */
    uint8_t dr_11n[WLIB_DR_11N_MAX_NUM];  /* 0,  1,  2,  3,  4,  5,  6,  7 */ /* MCS0 - MCS7 */
                                          /* 6, 13, 19, 26, 39, 52, 58, 65 */ /* GI = 800ns */
                                          /* 7, 14, 21, 28, 43, 57, 65, 72 */ /* GI = 400ns */
    uint8_t dr_11b_only_basic_rate[WLIB_DR_11B_MAX_NUM];
    uint8_t dr_11g_only_basic_rate[WLIB_DR_11G_MAX_NUM];
    uint8_t dr_11bg_mixed1_basic_rate[WLIB_DR_BR_MAX_NUM];
    uint8_t dr_11bg_mixed2_basic_rate[WLIB_DR_BR_MAX_NUM];
} wlib_wifi_data_rate_t; /* unit: Mbps */

#define USER_CFG_DR_11B {1, 2, 5, 11}
#define USER_CFG_DR_11G {6, 9, 12, 18, 24, 36, 48, 54}

#define USER_CFG_DR_11B_ONLY_BR    {1, 2, 0, 0}
#define USER_CFG_DR_11G_ONLY_BR    {6, 12, 24, 0, 0, 0, 0, 0}
#define USER_CFG_DR_11BG_MIXED1_BR {1, 2, 5, 11, 0, 0, 0, 0}
#define USER_CFG_DR_11BG_MIXED2_BR {1, 2, 5, 11, 6, 12, 24, 0}

const wlib_wifi_data_rate_t *wlib_data_rate_info_get(void);

/* hardware for MAC Interrupt */
void    wlib_mac_isr_enable(void);
void    wlib_mac_isr_disable(void);

/* hardware for EVM test */
void    wlib_hwtimer_init(void * timer_cb, uint32_t period_us);
void    wlib_hwtimer_start(void);
void    wlib_hwtimer_stop(void);
void    wlib_software_reset_core(void);
void    wlib_pvtcmd_printf(const char *format, ...);
void    wlib_pvtcmd_output_cb_set(wlib_pvtcmd_output_pfn cb);
wlib_pvtcmd_output_pfn wlib_pvtcmd_output_cb_get(void);
void    wlib_xtal40m_cap_set(uint8_t cap);
void    wlib_xtal40m_vol_set(uint8_t vol);
void    wlib_r_xtal40m_ibsel(uint8_t sel);
void wlib_awo_set_r_vtrim(uint8_t value);

/* misc */
uint8_t wlib_char2hex(char c);
int     wlib_str2bytes(uint8_t *bytes, const char *str);
int     wlib_is_valid_mac_str(const char* mac_str);
int     wlib_is_valid_mac(const char *mac);
int     wlib_mac_str2hex(const uint8_t *str, uint8_t *hex);

/* hmac-sha1 psk */
void wlib_hmac_sha1(uint8_t  * k,    /* secret key */
                    size_t     lk,   /* length of the key in bytes */
                    uint8_t  * d,    /* data */
                    size_t     ld,   /* length of data in bytes */
                    uint8_t  * out   /* output buffer, at least "t" bytes */
                    );

/**
 * @brief wlib_aes_init
 * 
 * @param key     in param
 * @param keysize in param. Optional 16/24/32, uint byte.
 * @return void*  return NULL: error; others: success.
 */
void *wlib_aes_init(const uint8_t *key, uint8_t keysize);

/**
 * @brief wlib_aes_deinit
 * 
 * @param ctx in param, from wlib_aes_init()
 */
void  wlib_aes_deinit(void *ctx);

/**
 * @brief wlib_aes_encrypt
 * 
 * @param ctx in param, from wlib_aes_init()
 * @param ptext in  param, plaintext
 * @param ctext out param, ciphertext
 */
void wlib_aes_encrypt(void *ctx, const uint8_t *ptext, uint8_t *ctext);

/**
 * @brief wlib_aes_decrypt
 * 
 * @param ctx in param, from wlib_aes_init()
 * @param ctext in  param, ciphertext
 * @param ptext out param, plaintext
 */
void wlib_aes_decrypt(void *ctx, const uint8_t *ctext, uint8_t *ptext);

/* tx power external compensation */
void   wlib_get_tx_power_ext_comp_val(int8_t *bgn_pwr, int8_t *b_pwr, int8_t *gn_pwr);

/* heap memory manager */
void  *wlib_malloc(uint32_t size);
void   wlib_free(void *ptr);

/* host mem map, reg base, nop */
#include "ln882h.h"
#define IMG_CAL_TMP_MEM_BASE   SRAM3_0_BASE
#define IMG_CAL_TMP_MEM_SIZE   SIZE_32KB

/* wlib log output */
void   wlib_log_printf(uint8_t tag_en, uint8_t level, const char *format, ...);
void   wlib_hexdump(const char *info, void *buf, uint32_t buf_len);
void   wlib_assert(int expr, const char *fun, int line);

#define WLIB_LOG_E_TAG            "[WLIB_E]"
#define WLIB_LOG_I_TAG            "[WLIB_I]"
#define WLIB_LOG_W_TAG            "[WLIB_W]"
#define WLIB_LOG_D_TAG            "[WLIB_D]"
#define WLIB_LOG_LVL_E            (1) //maclib log error output.
#define WLIB_LOG_LVL_I            (2) //maclib log (necessary) infor output.
#define WLIB_LOG_LVL_W            (3) //maclib log warning output.
#define WLIB_LOG_LVL_D            (4) //maclib log debug output.
#define WLIB_LOG_LEVEL_CFG        WLIB_LOG_LVL_W

#define WLIB_LOG_E(...)           wlib_log_printf(1, WLIB_LOG_LVL_E, __VA_ARGS__)
#define WLIB_LOG_I(...)           wlib_log_printf(1, WLIB_LOG_LVL_I, __VA_ARGS__)
#define WLIB_LOG_W(...)           wlib_log_printf(1, WLIB_LOG_LVL_W, __VA_ARGS__)
#define WLIB_LOG_D(...)           wlib_log_printf(1, WLIB_LOG_LVL_D, __VA_ARGS__)
#define WLIB_LOG(...)             wlib_log_printf(1, __VA_ARGS__)
#define WLIB_LOG_RAW(...)         wlib_log_printf(0, __VA_ARGS__)
#define WLIB_ASSERT(expr)         wlib_assert((int)expr, __func__, __LINE__)

/* sniffer pool buf */
int      wlib_sniffer_mem_pool_init(void);
void     wlib_sniffer_mem_pool_deinit(void);
void    *wlib_sniffer_mem_pool_alloc(void);
int      wlib_sniffer_mem_pool_free(void *addr);
uint16_t wlib_sniffer_mem_chunk_count_get(void);
uint16_t wlib_sniffer_mem_chunk_buf_size_get(void);

/* cpu sleep voter register */
void   wlib_cpu_sleep_voter_reg(void *vote_check, void *pre_sleep, void *post_sleep);

/* sdio host interface */
void   wlib_sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc);
void   wlib_sdio_if_reset(void);


/* os service */
#define WLIB_OS_WAIT_FOREVER          (0xffffffffU)

typedef void *wlib_thread_t;
typedef void *wlib_mutex_t;
typedef void *wlib_sem_t;
typedef void *wlib_queue_t;
typedef void *wlib_timer_t;

wlib_thread_t wlib_os_thread_creat(const char *name, void *entry, void *arg);
void          wlib_os_thread_delete(wlib_thread_t *pthread);

wlib_queue_t  wlib_os_queue_creat(uint32_t item_count, uint32_t item_size);
void          wlib_os_queue_delete(wlib_queue_t *pqueue);
int           wlib_os_queue_send(wlib_queue_t *pqueue, const void *item, uint32_t timeout_ms);
int           wlib_os_queue_recv(wlib_queue_t *pqueue, void *item, uint32_t timeout_ms);

wlib_mutex_t  wlib_os_mutex_creat(void);
void          wlib_os_mutex_delete(wlib_mutex_t *pmutex);
int           wlib_os_mutex_lock(wlib_mutex_t *pmutex, uint32_t timeout_ms);
int           wlib_os_mutex_unlock(wlib_mutex_t *pmutex);

wlib_sem_t    wlib_os_sem_create(uint32_t max_count, uint32_t init_count);
wlib_sem_t    wlib_os_sem_binary_create(void);
void          wlib_os_sem_delete(wlib_sem_t *psem);
int           wlib_os_sem_wait(wlib_sem_t *psem, uint32_t timeout_ms);
int           wlib_os_sem_release(wlib_sem_t *psem);

wlib_timer_t  wlib_os_timer_creat(void *timer_cb, uint32_t data);
void          wlib_os_timer_delete(wlib_timer_t *ptimer);
void          wlib_os_timer_start(wlib_timer_t timer, uint32_t ms);
void          wlib_os_timer_stop(wlib_timer_t timer);
void          wlib_os_delay_ms(uint32_t ms);

#endif /* __MACLIB_PORT_H__ */

