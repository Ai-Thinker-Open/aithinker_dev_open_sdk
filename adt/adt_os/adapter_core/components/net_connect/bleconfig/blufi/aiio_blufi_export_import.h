/** @brief      blufi_export_import.
 *
 *  @file       aiio_blufi_export_import.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/05/18      <td>1.0.0       <td>wusen       <td>First version
 *  </table>
 *
 */


#ifndef __AIIO_BLUFI_EXPORT_IMPORT_H__
#define __AIIO_BLUFI_EXPORT_IMPORT_H__
#include "aiio_ble.h"

#define BLE_ADV_DATA_MAX_LEN 31

typedef struct
{
    uint8_t data[BLE_ADV_DATA_MAX_LEN];
    uint16_t data_len;
    uint8_t rsp_data[BLE_ADV_DATA_MAX_LEN];
    uint16_t rsp_data_len;
} at_ble_adv_data_t;

/**
 * @brief 
 * 
 * @return int32_t 
 */
int32_t aiio_hal_blufi_init(void);

/**
 * @brief 
 * 
 */
void aiio_hal_blufi_deinit(void);

/**
 * @brief service uuid 0xFFFF
 * char uuid 0xFF01 Properties WRITE
 * char uuid 0xFF02 Properties READ、NOTYFY
 * 
 * @return int32_t 0 success,or fail
 */
int32_t aiio_hal_blufi_server_create_start(void);

int32_t aiio_hal_ble_adv_start(at_ble_adv_data_t *data);

int32_t aiio_hal_ble_adv_stop(void);



/**
 * @brief 
 * 
 * @param buf 
 * @param len 
 * @return int32_t 
 */
int32_t aiio_hal_ble_gatt_blufi_notify_send(uint8_t *buf, uint16_t len);

/**
 * @brief 
 * 
 * @return int32_t 
 */
int32_t aiio_hal_blufi_gap_disconnect(void);

/*****************************************************************************************************************************/

/**
 * @brief 
 * 
 * @param handle 
 * @param peer_addr 
 * @return int32_t 
 */
int32_t aiio_ble_blufi_gap_connect_export(void *handle, uint8_t *peer_addr);

/**
 * @brief 
 * 
 * @param peer_addr 
 * @return int32_t 
 */
int32_t aiio_ble_blufi_gap_disconnect_export(uint8_t *peer_addr);

/**
 * @brief 
 * 
 * @param mtu 
 * @return int32_t 
 */
int32_t aiio_ble_blufi_gap_mtu_change_export(uint16_t mtu);

/**
 * @brief 
 * 
 * @param data 
 * @param len 
 * @return int32_t 
 */
int32_t aiio_ble_blufi_gatt_write_export(uint8_t *data, int32_t len);

#endif