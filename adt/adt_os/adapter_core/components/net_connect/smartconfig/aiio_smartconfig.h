/** @brief      smartconfig api.
 *
 *  @file       aiio_smartconfig.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/04      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_SMARTCONFIG_H__
#define __AIIO_SMARTCONFIG_H__

#include <stdint.h>
#include "aiio_wifi.h"

typedef int32_t (*aiio_smartconfig_callback_t)(const char *ssid, const char *pwd,int32_t channel);

/** @brief smartconfig start
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         start successful.
 *  @retval         AIIO_ERROR      start error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_wifi_smartconfig_v1_start(void);

/** @brief smartconfig stop
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         stop successful.
 *  @retval         AIIO_ERROR      stop error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_wifi_smartconfig_v1_stop(void);

/** @brief  Register the WiFi smartconfig callback.
 *  @param[in] cb       Callback function.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_smartconfig_register_cb(aiio_smartconfig_callback_t cb);

/** @brief  Smartconfig WIFI event.
 *  @param[in] evt_code       WIFI event.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API void aiio_smartconfig_wifi_event(int32_t evt_code, void *param);


#endif
