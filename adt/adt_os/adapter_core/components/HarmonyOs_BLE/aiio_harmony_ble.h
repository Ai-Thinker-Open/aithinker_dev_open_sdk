/** @brief      Harmony ble interface.
 *
 *  @file       aiio_harmony_ble.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Harmony ble interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/01/09      <td>1.0.0       <td>hongjz      <td>Define Harmony ble API
 *  </table>
 *
 */

#ifndef _AIIO_HARMONY_BLE_H_
#define _AIIO_HARMONY_BLE_H_

#include "aiio_type.h"

#include <stdint.h>

typedef int32_t(*aiio_recv_remote_data_cb_t)(uint8_t *data, uint16_t len);

typedef void(*aiio_harmony_conn_cb_t)(void);

typedef struct
{
    char pid[5];            /*产品ID，与平台定义保持一致*/
    char custom_name[11];   /*自定义，由产品品牌名与设备名称组成, ≤10 */
    char mac[6];            /*mac地址*/
    char ver[8];            /*版本号*/
    char menu[12];          /*品牌名*/
} aiio_hamony_dev_info_t;

/** @brief  Harmony ble init
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_init(void);

/** @brief  Harmony ble device info setting
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         successful.
 *  @retval         AIIO_ERROR      error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_dev_info_set(const aiio_hamony_dev_info_t *dev);

/** @brief  Harmony ble restore
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         successful.
 *  @retval         AIIO_ERROR      error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_restore(void);

/** @brief  Register ble receive data from master callback function
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         successful.
 *  @retval         AIIO_ERROR      error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_recv_remote_data_cb_reg(aiio_recv_remote_data_cb_t cb);

/** @brief  Register ble create session callback function
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         successful.
 *  @retval         AIIO_ERROR      error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_conn_cb_reg(aiio_harmony_conn_cb_t cb);

/** @brief  Harmony ble report data
 *
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         successful.
 *  @retval         AIIO_ERROR      error.
 *  @note
 *  @see
 */
CORE_API int32_t aiio_harmony_ble_data_report(uint8_t *data, uint16_t len);

#endif //_AIIO_HARMONY_H_
