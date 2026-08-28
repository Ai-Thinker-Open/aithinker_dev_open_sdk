/** @brief      SNTP interface implementation.
 *
 *  @file       aiio_sntp.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Chip initialization interface implementation.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/01/30      <td>1.0.0       <td>specter      <td>Define file
 *  </table>
 *
 */
#ifndef __AIIO_SNTP_H__
#define __AIIO_SNTP_H__


#include "aiio_type.h"

/**
 * @brief sntp operating mode
 */
typedef enum
{
    AIIO_SNTP_OPT_MODE_POLL = 0x00,
    AIIO_SNTP_OPT_MODE_LISTENONLY = 0x01,
} aiio_sntp_operating_mode_t;

/** @brief get last sntp sync time.
 *
 *  @param[out]      seconds   get second
 *  @param[out]      frags   get millisecond-(second*1000)
 *  @return             Return the operation status. When the return value is AIIO_OK, the initialization is successful.
 *  @retval             AIIO_OK         get successful.
 *  @retval             AIIO_ERROR      get error.
 *  @note               This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *  @see
 */
CHIP_API int32_t aiio_hal_sntp_get_last_time(uint32_t *seconds, uint32_t *frags);

/** @brief set sntp server.
 *
 *  @param[in]      idx     current set sntp server index
 *  @param[in]      server  current set sntp server address,eg:"cn.ntp.org.cn"
 *  @return             Return the operation status. When the return value is AIIO_OK, the initialization is successful.
 *  @retval             AIIO_OK         set successful.
 *  @retval             AIIO_ERROR      set error.
 *  @note               This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *  @see
 */
CHIP_API int32_t aiio_hal_sntp_set_server_name(uint8_t idx, const char *server);

/** @brief set sntp operating mode
 *
 *  @param[in]      operating_mode     current set sntp operating mode
 *  @return             Return the operation status. When the return value is AIIO_OK, the initialization is successful.
 *  @retval             AIIO_OK         set successful.
 *  @retval             AIIO_ERROR      set error.
 *  @note               This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *  @see
 */
CHIP_API int32_t aiio_hal_sntp_set_operating_mode(aiio_sntp_operating_mode_t operating_mode);

#endif
