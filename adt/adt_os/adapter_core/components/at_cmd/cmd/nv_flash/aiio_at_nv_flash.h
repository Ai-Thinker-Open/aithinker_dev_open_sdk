/** @brief      AT NV_FLASH API.
 *
 *  @file       aiio_at_nv_flash.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/04/25      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_NV_FLASH_H__
#define __AIIO_AT_NV_FLASH_H__

#include "aiio_type.h"
/*
#define UART_DEF_CONFIG {\
    115200,  \
    8,\
    UART_STOP_BITS_1,\
    AT_UART_PARITY_NONE,\
    AT_UART_FLOWCTRL_DISABLE,\
}
*/
#define AIIO_AT_MAGIC 0X5A5AA5A5

/** @brief 	restore all param
 *  @return				return AIIO_OK or AIIO_ERROR                        
 *  @retval         	AIIO_OK: 	restore successful  
 *  @retval         	AIIO_ERROR: restore failed
 *  @note		
 *  @see				
 */
CORE_API int32_t aiio_at_nv_restore_param_all(void);
/** @brief 	load all param
 *  @return				return AIIO_OK or AIIO_ERROR                        
 *  @retval         	AIIO_OK: 	load successful  
 *  @retval         	AIIO_ERROR: load failed
 *  @note		
 *  @see				
 */
CORE_API int32_t aiio_at_nv_load_param_all(void);

int32_t aiio_at_nv_save_magicnum(void);
int32_t aiio_at_nv_load_magicnum(void);
int32_t aiio_at_nv_save_uart_def(void);
int32_t aiio_at_nv_load_uart_def(void);
int32_t aiio_at_nv_load_ota_def(void);
int32_t aiio_at_nv_save_ota_def(void);

int32_t aiio_at_nv_save_wautoconn(void);
int32_t aiio_at_nv_load_wautoconn(void);
int32_t aiio_at_nv_save_wmode(void);
int32_t aiio_at_nv_load_wmode(void);
int32_t aiio_at_nv_save_wjap(void);
int32_t aiio_at_nv_load_wjap(void);
int32_t aiio_at_nv_save_wcountry(void);
int32_t aiio_at_nv_load_wcountry(void);


int32_t aiio_at_nv_save_wapdhcp(void);
int32_t aiio_at_nv_load_wapdhcp(void);
int32_t aiio_at_nv_save_wsdhcp(void);
int32_t aiio_at_nv_load_wsdhcp(void);

int32_t aiio_at_nv_save_socketrecvcfg(void);
int32_t aiio_at_nv_load_socketrecvcfg(void);
int32_t aiio_at_nv_save_socketautott(void);
int32_t aiio_at_nv_load_socketautott(void);
int32_t aiio_at_nv_save_socketsslcret(void);
int32_t aiio_at_nv_load_socketsslcret(void);

int32_t aiio_at_nv_save_bleseruuid(void);
int32_t aiio_at_nv_load_bleseruuid(void);
int32_t aiio_at_nv_save_bletxuuid(void);
int32_t aiio_at_nv_load_bletxuuid(void);
int32_t aiio_at_nv_save_blerxuuid(void);
int32_t aiio_at_nv_load_blerxuuid(void);
int32_t aiio_at_nv_save_bleautocon(void);
int32_t aiio_at_nv_load_bleautocon(void);
int32_t aiio_at_nv_save_blemac(void);
int32_t aiio_at_nv_load_blemac(void);
int32_t aiio_at_nv_save_bleadvdata(void);
int32_t aiio_at_nv_load_bleadvdata(void);

int32_t aiio_at_nv_save_iomap(void);
int32_t aiio_at_nv_load_iomap(void);


#endif
