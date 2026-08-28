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
/*
 * ┌───────────────────────────┐
 * │    Frame Control Field    │  ── 2 Bytes
 * ├───────────────────────────┤
 * │          Duration         │  ── 2 Bytes
 * ├───────────────────────────┤
 * │          Address 1        │  ── 6 Bytes
 * ├───────────────────────────┤
 * │          Address 2        │  ── 6 Bytes
 * ├───────────────────────────┤
 * │          Address 3        │  ── 6 Bytes
 * ├───────────────────────────┤
 * │      Sequence Control     │  ── 2 Bytes
 * └───────────────────────────┘
 */
struct libwifi_frame_ctrl_flags {
    uint32_t to_ds : 1;
    uint32_t from_ds : 1;
    uint32_t more_frags : 1;
    uint32_t retry : 1;
    uint32_t power_mgmt : 1;
    uint32_t more_data : 1;
    uint32_t protect : 1;
    uint32_t ordered : 1;
} __attribute__((packed));
struct libwifi_frame_ctrl {
    uint32_t version : 2;
    uint32_t type : 2;
    uint32_t subtype : 4;
    struct libwifi_frame_ctrl_flags flags;
} __attribute__((packed));
struct libwifi_seq_control {
    uint32_t fragment_number : 4;
    uint32_t sequence_number : 12;
} __attribute__((packed));
struct libwifi_frame {
    struct libwifi_frame_ctrl frame_control;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    struct libwifi_seq_control seq_control;
} __attribute__((packed));

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
