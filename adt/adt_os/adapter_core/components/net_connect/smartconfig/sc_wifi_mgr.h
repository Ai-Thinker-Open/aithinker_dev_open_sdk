/** @brief      smartconfig scan api.
 *
 *  @file       sc_wifi_mgr.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/04      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */

#ifndef __SC_WIFI_MGR_H__
#define __SC_WIFI_MGR_H__

#include <stdint.h>
#include "aiio_wifi.h"

/** @brief smartconfig scan init
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         init successful.
 *  @retval         AIIO_ERROR      init error.
 *  @note
 *  @see
 */
int32_t sc_scan_init(void);
/** @brief smartconfig scan deinit
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         deinit successful.
 *  @retval         AIIO_ERROR      deinit error.
 *  @note
 *  @see
 */
void sc_scan_deinit(void);
/** @brief smartconfig scan start
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         start successful.
 *  @retval         AIIO_ERROR      start error.
 *  @note
 *  @see
 */
int32_t sc_scan_start(void);

/** @brief  get next available channel.
 *  @param[in] chan_cur      current channel, start at 1.
 *  @return             channel.
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
int32_t sc_get_next_channel(int32_t chan_cur);

/** @brief  get apinfo by bssid.
 *  @param[in] bssid      bssid.
 *  @return             ap info.
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
aiio_wifi_ap_item_t *sc_scan_get_ap_by_bssid(uint8_t *bssid);


#endif
