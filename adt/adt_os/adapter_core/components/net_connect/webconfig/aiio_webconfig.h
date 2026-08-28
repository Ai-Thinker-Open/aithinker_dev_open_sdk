#ifndef _AIIO_WEBCONFIG_H_
#define _AIIO_WEBCONFIG_H_

#include <stdint.h>
#include "aiio_type.h"

typedef int32_t (*aiio_webconfig_callback_t)(const char *ssid, const char *pwd,int32_t channel);

/** @brief  Webconfig start.
 *  @param[in]
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_webconfig_start(void);

/** @brief  Webconfig stop.
 *  @param[in]
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_webconfig_stop(void);

/** @brief  Webconfig WIFI event.
 *  @param[in] evt_code       WIFI event.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API void aiio_webconfig_wifi_event(int32_t evt_code, void *param);

/** @brief  Register the WiFi webconfig callback.
 *  @param[in] cb       Callback function.
 *  @return             Return the operation status. When the return value is AIIO_OK, the register is successful.
 *  @retval             AIIO_OK         register successful.
 *  @retval             AIIO_ERROR      register error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_webconfig_register_cb(aiio_webconfig_callback_t cb);

#endif //_AIIO_WEBCONFIG_H_
