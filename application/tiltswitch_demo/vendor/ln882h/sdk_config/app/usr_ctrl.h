#ifndef __USR_CTRL_H__
#define __USR_CTRL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "utils/debug/log.h"
#include "osal/osal.h"

#define COLINK_DEBUG

uint8_t get_log_lvl(void);

#define LOG_CTRL(level, ...)                  	\
        do{                              		\
            if(level <= LOG_LVL_INFO){   		\
                log_printf(__VA_ARGS__); 		\
            }                            		\
        }while(0)
	
#define Log_iot(lvl, flag, ...)                    				         	\
	do                                                      				\
	{                                                       				\
		LOG_CTRL(lvl, "%s||%s|%s(%d): ", flag, __FILE__, __func__, __LINE__); 	\
		LOG_CTRL(lvl, __VA_ARGS__);                              				\
		LOG_CTRL(lvl, "\r\n");                                   				\
	} while (0)

#define Log_d(...) Log_iot(LOG_LVL_DEBUG,	"[LN_Dbug]", __VA_ARGS__)
#define Log_i(...) Log_iot(LOG_LVL_INFO,	"[LN_Info]", __VA_ARGS__)
#define Log_w(...) Log_iot(LOG_LVL_WARN,	"[LN_Warn]", __VA_ARGS__)
#define Log_e(...) Log_iot(LOG_LVL_ERROR,	"[LN_Error]", __VA_ARGS__)
	
#define LN_LOG_CTRL_LVL  			("kv_ln_log_ctrl_lvl_parameter")
	
//ESP wifi mode
enum{
	ESP_WIFI_TURNOFF = 0,
	ESP_WIFI_STA,
	ESP_WIFI_AP,
	ESP_WIFI_STA_AP
};
#define LN_WIFI_MODE_TURNOFF		(WIFI_MODE_OFF)
#define ESP2LN_WIFI_MODE(x)			((x) == ESP_WIFI_TURNOFF ? LN_WIFI_MODE_TURNOFF : ((x) - 1))
#define LN2ESP_WIFI_MODE(x)			((x) == LN_WIFI_MODE_TURNOFF ? ESP_WIFI_TURNOFF : ((x) + 1))
	
typedef int ln_err_t;

//ota status
struct ota_status_sem
{
    OS_Semaphore_t ota_find_server_sem;
    OS_Semaphore_t ota_connect_server_sem;
    OS_Semaphore_t ota_get_version_sem;
    OS_Semaphore_t ota_update_done_sem;
};


/* Definitions for error constants. */
#define LN_OK          0       /*!< ln_err_t value indicating success (no error) */
#define LN_FAIL        -1      /*!< Generic ln_err_t code indicating failure */

#define LN_ERR_NO_MEM              0x101   /*!< Out of memory */
#define LN_ERR_INVALID_ARG         0x102   /*!< Invalid argument */
#define LN_ERR_INVALID_STATE       0x103   /*!< Invalid state */
#define LN_ERR_INVALID_SIZE        0x104   /*!< Invalid size */
#define LN_ERR_NOT_FOUND           0x105   /*!< Requested resource not found */
#define LN_ERR_NOT_SUPPORTED       0x106   /*!< Operation or feature not supported */
#define LN_ERR_TIMEOUT             0x107   /*!< Operation timed out */
#define LN_ERR_INVALID_RESPONSE    0x108   /*!< Received response was invalid */
#define LN_ERR_INVALID_CRC         0x109   /*!< CRC or checksum was invalid */
#define LN_ERR_INVALID_VERSION     0x10A   /*!< Version was invalid */
#define LN_ERR_INVALID_MAC         0x10B   /*!< MAC address was invalid */

#define LN_ERR_WIFI_BASE           0x3000  /*!< Starting number of WiFi error codes */
#define LN_ERR_MESH_BASE           0x4000  /*!< Starting number of MESH error codes */
	
extern void create_statistics_list_task(void);
extern void create_at_cmd_plus_task(void);

extern void ln_start_tmr(OS_Timer_t *_tmr, OS_TimerType type, OS_TimerCallback_t cb, void *arg, uint32_t periodMS);

#ifdef __cplusplus
}
#endif

#endif /* __USR_CTRL_H__ */
