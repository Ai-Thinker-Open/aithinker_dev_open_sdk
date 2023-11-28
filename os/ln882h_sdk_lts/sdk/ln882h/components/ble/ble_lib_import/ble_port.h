/**
 * @file ble_port.h
 * @brief 
 *        Not allowed to add header files other than the ble component in 
 *        this file, except for standard library header files!!!
*/

#ifndef  __BLE_LIB_PORT_H__
#define  __BLE_LIB_PORT_H__

#include <stdint.h>

/* hardware for MAC Interrupt */
void   blib_ble_mac_isr_enable(void);
void   blib_ble_mac_isr_disable(void);
void   blib_ble_err_isr_enable(void);
void   blib_ble_err_isr_disable(void);


/* heap memory manager */
void  *blib_malloc(uint32_t size);
void   blib_free(void *ptr);


/* flash api adapter */
void   blib_flash_read(uint32_t addr, uint32_t len, uint8_t* buf);
void   blib_flash_write(uint32_t addr, uint32_t len, uint8_t* buf);
void   blib_flash_erase(uint32_t addr, uint32_t len);


/* host mem map, reg base, nop */

/* blib log output */
void   blib_log_printf(uint8_t tag_en, uint8_t level, const char *format, ...);
void   blib_hexdump(const char *info, void *buf, uint32_t buf_len);
void blib_assert(int param0, int param1, int type, const char *file, int line);

#define BLIB_LOG_E_TAG            "[BLIB_E]"
#define BLIB_LOG_I_TAG            "[BLIB_I]"
#define BLIB_LOG_W_TAG            "[BLIB_W]"
#define BLIB_LOG_D_TAG            "[BLIB_D]"
#define BLIB_LOG_LVL_E            (1) //ble lib log error output.
#define BLIB_LOG_LVL_I            (2) //ble lib log (necessary) infor output.
#define BLIB_LOG_LVL_W            (3) //ble lib log warning output.
#define BLIB_LOG_LVL_D            (4) //ble lib log debug output.
#define BLIB_LOG_LEVEL_CFG        BLIB_LOG_LVL_I

#define BLIB_LOG_E(...)           blib_log_printf(1, BLIB_LOG_LVL_E, __VA_ARGS__)
#define BLIB_LOG_I(...)           blib_log_printf(1, BLIB_LOG_LVL_I, __VA_ARGS__)
#define BLIB_LOG_W(...)           blib_log_printf(1, BLIB_LOG_LVL_W, __VA_ARGS__)
#define BLIB_LOG_D(...)           blib_log_printf(1, BLIB_LOG_LVL_D, __VA_ARGS__)
#define BLIB_LOG(...)             blib_log_printf(1, __VA_ARGS__)
#define BLIB_LOG_RAW(...)         blib_log_printf(0, __VA_ARGS__)

/* cpu sleep voter register */
void   blib_cpu_sleep_voter_reg(void *vote_check, void *pre_sleep, void *post_sleep);


/*  ble RF prepare init. TODO:  */
void   blib_modem_init(void);
void   blib_rf_reg_prepare_init(void);
void   blib_rf_cal(void);


#endif /* __BLE_LIB_PORT_H__ */

