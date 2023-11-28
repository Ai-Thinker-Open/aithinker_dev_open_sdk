#include "wifi_port.h"
#include "osal/osal.h"
#include "ln_nvds.h"
#include "ln_compiler.h"
#include "utils/ln_sha1.h"
#include "utils/ln_aes.h"
#include <stdarg.h>


/* hardware for MAC Interrupt */
void wlib_mac_isr_enable(void)
{
    NVIC_EnableIRQ(MAC_IRQn);
}

void wlib_mac_isr_disable(void)
{
    NVIC_DisableIRQ(MAC_IRQn);
}

/* hardware for EVM test, temp cal */
#include "hal/hal_timer.h"
#include "hal/hal_clock.h"
#include "hal/hal_misc.h"

static void default_empty_timer_cb(void);
void default_empty_timer_cb(void)
{
    return;
}

void (*hw_timer_cb)(void) = default_empty_timer_cb;

void wlib_hwtimer_init(void * timer_cb, uint32_t period_us)
{
    tim_init_t_def tim_init;
    memset(&tim_init,0,sizeof(tim_init));

    tim_init.tim_load_value = ((hal_clock_get_apb0_clk() / 1000000) / 40) * period_us - 1;
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;
    tim_init.tim_div = 39;

    hw_timer_cb = ( void (*)(void) )timer_cb;

    NVIC_SetPriority(TIMER3_IRQn,     4);
    NVIC_EnableIRQ(TIMER3_IRQn);

    hal_tim_it_cfg(TIMER3_BASE,TIM_IT_FLAG_ACTIVE,HAL_ENABLE);
    hal_tim_init(TIMER3_BASE,&tim_init);
}

void wlib_hwtimer_init_v2(void * timer_cb)
{
    tim_init_t_def tim_init;
    memset(&tim_init, 0, sizeof(tim_init));

    tim_init.tim_load_value = 0;              
    tim_init.tim_mode = TIM_USER_DEF_CNT_MODE;
    tim_init.tim_div = (uint32_t)(hal_clock_get_apb0_clk() / 1000000) - 1;                             
    hal_tim_init(TIMER3_BASE, &tim_init);                

    hw_timer_cb = ( void (*)(void) )timer_cb;

    NVIC_SetPriority(TIMER3_IRQn, 4);
    NVIC_EnableIRQ(TIMER3_IRQn);
    hal_tim_it_cfg(TIMER3_BASE, TIM_IT_FLAG_ACTIVE, HAL_ENABLE);      
}

void wlib_hwtimer_start(void)
{
    NVIC_EnableIRQ(TIMER3_IRQn);
    hal_tim_en(TIMER3_BASE,HAL_ENABLE);
}

void wlib_hwtimer_stop(void)
{
    NVIC_DisableIRQ(TIMER3_IRQn);
    hal_tim_en(TIMER3_BASE,HAL_DISABLE);
}

void TIMER3_IRQHandler(void)
{
    if(hal_tim_get_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE) == 1)
    {
        if(hw_timer_cb != NULL) {
            (*hw_timer_cb)();
        }
        hal_tim_clr_it_flag(TIMER3_BASE,TIM_IT_FLAG_ACTIVE);
    }
}

void wlib_software_reset_core(void)
{
#include "hal/hal_misc.h"
    hal_misc_reset_all();
}

static wlib_pvtcmd_output_pfn wlib_pvtcmd_output_cb = NULL;
void wlib_pvtcmd_output_cb_set(wlib_pvtcmd_output_pfn cb)
{
    wlib_pvtcmd_output_cb = cb;
}

wlib_pvtcmd_output_pfn wlib_pvtcmd_output_cb_get(void)
{
    return wlib_pvtcmd_output_cb;
}

void wlib_pvtcmd_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    if (wlib_pvtcmd_output_cb) {
        wlib_pvtcmd_output_cb(format, args);
    }
    va_end(args);
}

void wlib_xtal40m_cap_set(uint8_t cap)
{
    hal_misc_awo_set_r_xtal40m_cap_bit(cap); // xtal40m Cap (pf)
}

void wlib_xtal40m_vol_set(uint8_t vol)
{
    hal_misc_awo_set_r_xtal40m_ldo_vbit(vol); // xtal40m LDO (1.2V ~ )
}

void wlib_r_xtal40m_ibsel(uint8_t sel)
{
    hal_misc_awo_set_r_xtal40m_ibsel(sel);
}

void wlib_awo_set_r_vtrim(uint8_t value)
{
    hal_misc_awo_set_r_vtrim(value);
}

/* misc */
#include "utils/ln_misc.h"
uint8_t wlib_char2hex(char c)
{
    return ln_char2hex(c);
}

int wlib_str2bytes(uint8_t *bytes, const char *str)
{
    return ln_str2bytes(bytes, str);
}

int wlib_is_valid_mac_str(const char* mac_str)
{
    return ln_is_valid_mac_str(mac_str);
}

int wlib_is_valid_mac(const char *mac)
{
    return ln_is_valid_mac(mac);
}

int wlib_mac_str2hex(const uint8_t *str, uint8_t *hex)
{
    return ln_mac_str2hex(str, hex);
}

/* hmac-sha1 psk */
void wlib_hmac_sha1(uint8_t  * k,    /* secret key */
                    size_t     lk,   /* length of the key in bytes */
                    uint8_t  * d,    /* data */
                    size_t     ld,   /* length of data in bytes */
                    uint8_t  * out   /* output buffer, at least "t" bytes */
                    )
{
    ln_hmac_sha1(k, lk, d, ld, out);
}

/* aes */
void *wlib_aes_init(const uint8_t *key, uint8_t keysize)
{
    ln_aes_context_t *ctx = wlib_malloc(sizeof(ln_aes_context_t));
    if (NULL == ctx || NULL == key)
    {
        return NULL;
    }
    memset(ctx, 0x0, sizeof(ln_aes_context_t));

    switch (keysize)
    {
        case 16:
            ctx->nrounds = 10;
            break;
        case 24:
            ctx->nrounds = 12;
            break;
        case 32:
            ctx->nrounds = 14;
            break;
        default:
            return NULL;
    }

    ln_aes_setup(ctx, keysize, key);
    return ctx;
}

void wlib_aes_deinit(void *ctx)
{
    if (ctx)
    {
        wlib_free(ctx);
    }
}

void wlib_aes_encrypt(void *ctx, const uint8_t *ptext, uint8_t *ctext)
{
    if (ctx && ptext != NULL && ctext != NULL)
    {
        ln_aes_encrypt(ctx, ptext, ctext);
    }
}

void wlib_aes_decrypt(void *ctx, const uint8_t *ctext, uint8_t *ptext)
{
    if (ctx && ptext != NULL && ctext != NULL)
    {
        ln_aes_decrypt(ctx, ctext, ptext);
    }
}

/* tx power external compensation */
void wlib_get_tx_power_ext_comp_val(int8_t *bgn_pwr, int8_t *b_pwr, int8_t *gn_pwr)
{
    uint8_t rd_val1 = 0, rd_val2 = 0, rd_val3 = 0;
    if (!bgn_pwr || !b_pwr || !gn_pwr) {
        return;
    }

    if(ln_get_read_param_from_fotp_flag() != 0x01){
        if (NVDS_ERR_OK != ln_nvds_get_ate_result(&rd_val1)) {
            return;
        } else {
            if (rd_val1 != NV9_ATE_RESULT_OK) {
               return; 
            }
        }
    }
    

    if (NVDS_ERR_OK != ln_nvds_get_tx_power_comp(&rd_val1)) {
        return;
    }
    if (NVDS_ERR_OK != ln_nvds_get_tx_power_b_comp(&rd_val2)) {
        return;
    }
    if (NVDS_ERR_OK != ln_nvds_get_tx_power_gn_comp(&rd_val3)) {
        return;
    }

    if (rd_val1 == 0xFF) {
        rd_val1 = 0;
    }
    if (rd_val2 == 0xFF) {
        rd_val2 = 0;
    }
    if (rd_val3 == 0xFF) {
        rd_val3 = 0;
    }

    *bgn_pwr = (int8_t)rd_val1;
    *b_pwr   = (int8_t)rd_val2;
    *gn_pwr  = (int8_t)rd_val3;
}

/* heap memory manager */
void *wlib_malloc(uint32_t size)
{
    return OS_Malloc(size);
}

void wlib_free(void *ptr)
{
    OS_Free(ptr);
}

/* log print, hexdump */
void wlib_log_printf(uint8_t tag_en, uint8_t level, const char *format, ...)
{
    const char *tag = NULL;
    va_list args;

    va_start(args, format);
    if (tag_en) {
        switch(level) {
            case WLIB_LOG_LVL_E:
                tag = WLIB_LOG_E_TAG;
                break;
            case WLIB_LOG_LVL_I:
                tag = WLIB_LOG_I_TAG;
                break;
            case WLIB_LOG_LVL_W:
                tag = WLIB_LOG_W_TAG;
                break;
            case WLIB_LOG_LVL_D:
                tag = WLIB_LOG_D_TAG;
                break;
            default:
                break;
        }
    }

    if (level <= WLIB_LOG_LEVEL_CFG) {
        __sprintf(tag, (stdio_write_fn)log_stdio_write, format, args);
    }
    va_end(args);
}

void wlib_hexdump(const char *info, void *buf, uint32_t buf_len)
{
    hexdump(LOG_LVL_ERROR, (const char *)info, buf, buf_len);
}

void wlib_assert(int expr, const char *fun, int line)
{
    if (!expr){
        __disable_irq();
        wlib_log_printf(0, WLIB_LOG_LVL_E, "[WLIB_ASSERT]fun:%s, line:%d\r\n", fun, line);
        __BKPT(0);
        while(1);
    }
}

/* sniffer pool buf */
#include "utils/ln_mem_pool.h"
#define SNIFFER_MEM_POOL_USE_DYNAMIC_MEM   (1)
#define SNIFFER_MEM_POOL_CHUNK_CNT         (50) // Office environment maximum use 32
#define SNIFFER_MEM_POOL_CHUNK_BUF_SIZE    (30)
#define SNIFFER_MEM_POOL_CHUNK_SIZE        (MEM_POOL_CHUNK_INFO_SIZE + SNIFFER_MEM_POOL_CHUNK_BUF_SIZE)
static ln_mem_pool_t sniffer_mem_pool = {0};
static uint8_t s_wlib_snp_inited = 0;

int wlib_sniffer_mem_pool_init(void)
{
    if (s_wlib_snp_inited != 0) {
        return LN_TRUE;
    }
    s_wlib_snp_inited = 1;

#if (defined(SNIFFER_MEM_POOL_USE_DYNAMIC_MEM) && SNIFFER_MEM_POOL_USE_DYNAMIC_MEM)
    sniffer_mem_pool.mem_base       = (uint8_t  *)OS_Malloc(SNIFFER_MEM_POOL_CHUNK_CNT * SNIFFER_MEM_POOL_CHUNK_SIZE);
    sniffer_mem_pool.free_chunk_ptr = (uint8_t **)OS_Malloc(SNIFFER_MEM_POOL_CHUNK_CNT * sizeof(void *));
    if (!sniffer_mem_pool.mem_base || !sniffer_mem_pool.free_chunk_ptr) {
//        WLIB_LOG(WLIB_LOG_E, "sniffer mem pool use dynamic mem(OS_Malloc) failed.\r\n");
        return LN_FALSE;
    }
#else
    static uint8_t  g_sniffer_mem_block[SNIFFER_MEM_POOL_CHUNK_CNT * SNIFFER_MEM_POOL_CHUNK_SIZE] = {0};
    static uint8_t *sniffer_mem_pool_chunk_ptr[SNIFFER_MEM_POOL_CHUNK_CNT] = {0};
    sniffer_mem_pool.mem_base         = g_sniffer_mem_block;
    sniffer_mem_pool.free_chunk_ptr   = (uint8_t **)sniffer_mem_pool_chunk_ptr;
#endif

    sniffer_mem_pool.total_chunk_cnt  = SNIFFER_MEM_POOL_CHUNK_CNT;
    sniffer_mem_pool.chunk_size       = SNIFFER_MEM_POOL_CHUNK_SIZE;
    ln_mem_pool_init(&sniffer_mem_pool);

    return LN_TRUE;
}

void wlib_sniffer_mem_pool_deinit(void)
{
    if (s_wlib_snp_inited == 0) {
        return;
    }
    s_wlib_snp_inited = 0;

#if (defined(SNIFFER_MEM_POOL_USE_DYNAMIC_MEM) && SNIFFER_MEM_POOL_USE_DYNAMIC_MEM)
    OS_Free(sniffer_mem_pool.mem_base);
    OS_Free(sniffer_mem_pool.free_chunk_ptr);
#endif
}

void *wlib_sniffer_mem_pool_alloc(void)
{
    return ln_mem_pool_alloc(&sniffer_mem_pool);
}

int wlib_sniffer_mem_pool_free(void *addr)
{
    return ln_mem_pool_free(&sniffer_mem_pool, addr);
}

uint16_t wlib_sniffer_mem_chunk_count_get(void)
{
    return SNIFFER_MEM_POOL_CHUNK_CNT;
}

uint16_t wlib_sniffer_mem_chunk_buf_size_get(void)
{
    return SNIFFER_MEM_POOL_CHUNK_BUF_SIZE;
}


/* cpu sleep voter register */
#include "utils/power_mgmt/ln_pm.h"
void wlib_cpu_sleep_voter_reg(void *vote_check, void *pre_sleep, void *post_sleep)
{
    ln_pm_obj_register("wifi_stack", (int(*)(void))vote_check, (int(*)(void))pre_sleep, (int(*)(void))post_sleep);
}


/* sdio host interface */
//#include "hal/hal_sdio.h"
// __WEAK__ void sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc){}
// __WEAK__ void sdio_if_reset(void){}

void wlib_sdio_if_init(void *wlib_recv_from_sdio, void **wlib_send_to_sdio, void *pkt_buf_malloc)
{
    LN_UNUSED(wlib_recv_from_sdio);
    LN_UNUSED(wlib_send_to_sdio);
    LN_UNUSED(pkt_buf_malloc);

    // sdio_if_init(wlib_recv_from_sdio, wlib_send_to_sdio, pkt_buf_malloc);
}

void wlib_sdio_if_reset(void)
{
    // sdio_if_reset();
}

/* os service */
wlib_thread_t wlib_os_thread_creat(const char *name, void *entry, void *arg)
{
    #define WIFI_MAC_TASK_STACK_SIZE    (4*512)
    #define WIFI_CFG_TASK_STACK_SIZE    (8*256)//5*256
    #define WIFI_MAC_TASK_PRIORITY      (OS_PRIORITY_REAL_TIME)
    #define WIFI_CFG_TASK_PRIORITY      (OS_PRIORITY_NORMAL)

    OS_Priority priority;
    uint32_t stksize;

    if (!name || !entry) {
        return NULL;
    }

    if (strstr(name, "mac")) {
        priority = OS_PRIORITY_REAL_TIME;
        stksize  = WIFI_MAC_TASK_STACK_SIZE;
    } else if (strstr(name, "cfg")) {
        priority = OS_PRIORITY_NORMAL;
        stksize  = WIFI_CFG_TASK_STACK_SIZE;
    } else {
        return NULL;
    }

    OS_Thread_t *thread = OS_Malloc(sizeof(OS_Thread_t));
    WLIB_ASSERT(thread);
    memset(thread, 0, sizeof(OS_Thread_t));

    if (OS_OK != OS_ThreadCreate(thread, name, (OS_ThreadEntry_t)entry, arg, priority, stksize)) {
        WLIB_LOG_E("Create <%s> thread failed.\r\n", name);
        return NULL;
    }
    return (wlib_thread_t)thread;
}

void wlib_os_thread_delete(wlib_thread_t *pthread)
{
    WLIB_ASSERT(pthread);
    if(*pthread != NULL) {
        OS_ThreadDelete((OS_Thread_t *)*pthread);
        OS_Free(*pthread);
        *pthread = NULL;
    }
}

wlib_queue_t wlib_os_queue_creat(uint32_t item_count, uint32_t item_size)
{
    OS_Queue_t *queue = OS_Malloc(sizeof(OS_Queue_t));
    WLIB_ASSERT(queue);
    memset(queue, 0, sizeof(OS_Queue_t));

    if (OS_OK != OS_QueueCreate(queue, item_count, item_size)) {
        WLIB_LOG_E("Create queue failed.\r\n");
        return NULL;
    }
    return (wlib_queue_t)queue;
}

void wlib_os_queue_delete(wlib_queue_t *pqueue)
{
    WLIB_ASSERT(pqueue);
    if(*pqueue != NULL) {
        OS_QueueDelete((OS_Queue_t *)*pqueue);
        OS_Free(*pqueue);
        *pqueue = NULL;
    }
}

int wlib_os_queue_send(wlib_queue_t *pqueue, const void *item, uint32_t timeout_ms)
{
    if (OS_OK != OS_QueueSend((OS_Queue_t *)*pqueue, item, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_queue_recv(wlib_queue_t *pqueue, void *item, uint32_t timeout_ms)
{
    if (OS_OK != OS_QueueReceive((OS_Queue_t *)*pqueue, item, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_mutex_t wlib_os_mutex_creat(void)
{
    OS_Mutex_t *mutex = OS_Malloc(sizeof(OS_Mutex_t));
    WLIB_ASSERT(mutex);
    memset(mutex, 0, sizeof(OS_Mutex_t));

    if (OS_OK != OS_MutexCreate(mutex)) {
        WLIB_LOG_E("Create mutex failed.\r\n");
        return NULL;
    }
    return (wlib_mutex_t)mutex;
}

void wlib_os_mutex_delete(wlib_mutex_t *pmutex)
{
    WLIB_ASSERT(pmutex);
    if(*pmutex != NULL) {
        OS_MutexDelete((OS_Mutex_t *)*pmutex);
        OS_Free(*pmutex);
        *pmutex = NULL;
    }
}

int wlib_os_mutex_lock(wlib_mutex_t *pmutex, uint32_t timeout_ms)
{
    if (OS_OK != OS_MutexLock((OS_Mutex_t *)*pmutex, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_mutex_unlock(wlib_mutex_t *pmutex)
{
    if (OS_OK != OS_MutexUnlock((OS_Mutex_t *)*pmutex)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_sem_t wlib_os_sem_create(uint32_t max_count, uint32_t init_count)
{
    OS_Semaphore_t *sem = OS_Malloc(sizeof(OS_Semaphore_t));
    WLIB_ASSERT(sem);
    memset(sem, 0, sizeof(OS_Semaphore_t));

    if (OS_OK != OS_SemaphoreCreate(sem, init_count, max_count)) {
        WLIB_LOG_E("Create semaphore failed.\r\n");
        return NULL;
    }
    return (wlib_sem_t)sem;
}

wlib_sem_t wlib_os_sem_binary_create(void)
{
    OS_Semaphore_t *sem = OS_Malloc(sizeof(OS_Semaphore_t));
    WLIB_ASSERT(sem);
    memset(sem, 0, sizeof(OS_Semaphore_t));

    if (OS_OK != OS_SemaphoreCreateBinary(sem)) {
        WLIB_LOG_E("Create binary semaphore  failed.\r\n");
        return NULL;
    }
    return (wlib_sem_t)sem;
}

void wlib_os_sem_delete(wlib_sem_t *psem)
{
    WLIB_ASSERT(psem);
    if(*psem != NULL) {
        OS_SemaphoreDelete((OS_Semaphore_t *)*psem);
        OS_Free(*psem);
        *psem = NULL;
    }
}

int wlib_os_sem_wait(wlib_sem_t *psem, uint32_t timeout_ms)
{
    if (OS_OK != OS_SemaphoreWait((OS_Semaphore_t *)*psem, timeout_ms)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

int wlib_os_sem_release(wlib_sem_t *psem)
{
    if (OS_OK != OS_SemaphoreRelease((OS_Semaphore_t *)*psem)) {
        return WIFI_ERR_OS_SERVICE;
    }
    return WIFI_ERR_NONE;
}

wlib_timer_t wlib_os_timer_creat(void *timer_cb, uint32_t data)
{
    WLIB_ASSERT(timer_cb);

    OS_Timer_t *timer = OS_Malloc(sizeof(OS_Timer_t));
    WLIB_ASSERT(timer);
    memset(timer, 0, sizeof(OS_Timer_t));

    if (OS_OK != OS_TimerCreate(timer, OS_TIMER_ONCE, (OS_TimerCallback_t)timer_cb, (void *)data, 200)) {
        WLIB_LOG_E("Create timer failed.\r\n");
        return NULL;
    }
    return (wlib_timer_t)timer;
}

void wlib_os_timer_delete(wlib_timer_t *ptimer)
{
    WLIB_ASSERT(ptimer);
    if(*ptimer != NULL) {
        OS_TimerDelete((OS_Timer_t *)*ptimer);
        OS_Free(*ptimer);
        *ptimer = NULL;
    }
}

void wlib_os_timer_start(wlib_timer_t timer, uint32_t ms)
{
    WLIB_ASSERT(timer);
    WLIB_ASSERT(ms > 0);
    OS_TimerChangePeriod((OS_Timer_t *)timer, ms);
}

void wlib_os_timer_stop(wlib_timer_t timer)
{
    WLIB_ASSERT((OS_Timer_t *)timer);
    OS_TimerStop((OS_Timer_t *)timer);
}

void wlib_os_delay_ms(uint32_t ms)
{
    OS_MsDelay(ms);
}

static const wlib_wifi_data_rate_t g_wlib_wifi_dr = {
    .dr_11b = USER_CFG_DR_11B,
    .dr_11g = USER_CFG_DR_11G,
    .dr_11n = {10, 11, 12, 13, 14, 15, 16, 17}, /* offset is 10, not currently in use */
    .dr_11b_only_basic_rate = USER_CFG_DR_11B_ONLY_BR,
    .dr_11g_only_basic_rate = USER_CFG_DR_11G_ONLY_BR,
    .dr_11bg_mixed1_basic_rate = USER_CFG_DR_11BG_MIXED1_BR,
    .dr_11bg_mixed2_basic_rate = USER_CFG_DR_11BG_MIXED2_BR,
};

const wlib_wifi_data_rate_t *wlib_data_rate_info_get(void)
{
    return (const wlib_wifi_data_rate_t *)(&g_wlib_wifi_dr);
}
