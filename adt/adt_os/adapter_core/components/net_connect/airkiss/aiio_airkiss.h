/** @brief      airkiss application interface.
 *
 *  @file       aiio_airkiss.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       airkiss application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>airkiss Init
 *  <tr><td>2023/05/18      <td>1.0.1       <td>wusen       <td>Fixed the WIFI distribution event conflict bug
 *  </table>
 *
 */
#ifndef __AIIO_AIRKISS_H_
#define __AIIO_AIRKISS_H_
#include "aiio_wifi.h"
#include "aiio_stdio.h"

typedef int32_t (*aiio_airkiss_callback_t)(const char *ssid, const char *pwd);

/** @brief airkiss start
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_airkiss_start(void);

/** @brief airkiss stop
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_airkiss_stop(void);

/** @brief  Register the WiFi airkiss callback.
 *  @param[in] cb       Callback function.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_airkiss_register_cb(aiio_airkiss_callback_t cb);

/** @brief  airkiss WIFI event.
 *  @param[in] evt_code       WIFI event.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API void aiio_airkiss_wifi_event(int32_t evt_code, void *param);
#endif
