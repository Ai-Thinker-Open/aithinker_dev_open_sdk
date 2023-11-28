/** @brief      blufi api.
 *
 *  @file       aiio_blufi.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/17      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */
#ifndef __AIIO_BLUFI_H__
#define __AIIO_BLUFI_H__

#include <stdint.h>
#include "aiio_wifi.h"
#include "aiio_blufi_api.h"

typedef int32_t (*aiio_blufi_wifi_callback_t)(const char *ssid, const char *pwd);

typedef int32_t (*aiio_blufi_ble_callback_t)(aiio_blufi_cb_event_t event, aiio_blufi_cb_param_t *param);

typedef struct
{
    aiio_blufi_wifi_callback_t wifi_event_cb;
    aiio_blufi_ble_callback_t ble_event_cb;
} aiio_blufi_cb_t;

/* Initialise blufi profile */
uint8_t aiio_blufi_init(void);

/* start advertising */
void bleprph_advertise(void);

/* send notifications */
void aiio_blufi_send_notify(void *arg);

/* Deinitialise blufi */
void aiio_blufi_deinit(void);
/* disconnect */
void aiio_blufi_disconnect(void);

/* Stop advertisement */
void aiio_blufi_adv_stop(void);

/* Start advertisement */
void aiio_blufi_adv_start(void);

void aiio_blufi_send_encap(void *arg);


/** @brief  Register the  blufi callback.
 *  @param[in] cb       Callback function.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_blufi_register_cb(aiio_blufi_cb_t cb);

/** @brief blufi start
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         start successful.
 *  @retval         AIIO_ERROR      start error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_blufi_start(void);

/** @brief blufi stop
 *
 *  @return             
 *  @retval             
 *  @retval             
 *  @note
 *  @see
 */
CORE_API void aiio_blufi_stop(void);

/** @brief  blufi WIFI event.
 *  @param[in] evt_code       WIFI event.
 *  @return             
 *  @retval             
 *  @retval             
 *  @note
 *  @see
 */
CORE_API void aiio_blufi_wifi_event(int32_t evt_code, void *param);


/** @brief  set blufi ble name.
 *  @param[in] bleName       ble name.
 *  @return             
 *  @retval             
 *  @retval             
 *  @note
 *  @see
 */
CORE_API void aiio_blufi_set_ble_name(char *bleName);


#endif