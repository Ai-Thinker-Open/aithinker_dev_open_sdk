/** @brief      AT ble command.
 *
 *  @file       aiio_ble.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>lins        <td>First version
 *  </table>
 *
 */

#ifndef __AIIO_AT_BLE_H__
#define __AIIO_AT_BLE_H__

#include "aiio_type.h"
#include "aiio_ble.h"

#define AIIO_BLE_TASK_STACK_SIZE    4096
#define AIIO_BLE_TASK_PRIO          10

#define AT_BLE_LOCAL_NAME_SIZE  29
#define AT_BLE_ADV_DATA_SIZE    16
#define BLE_SEND_MAX_SIZE       256
#define BLE_UUID128_SIZE        16
#define BLE_ADV_MIN             200
#define BLE_ADV_MAX             220
#define BLE_MIN_INTV            200
#define BLE_MAX_INTV            220
#define BLE_LATENCY             0
#define BLE_TIMEOUT             400
#define BLE_SCAN_INTV           100
#define BLE_SCAN_WINDOW         50
#define BLE_SCAN_TIME           5000
#define BLE_CONN_TIME           5000

#define BLE_MASTER_SCAN_MAX     36
#define AT_BLE_PASSTHROUGH_SIZE 247

typedef enum {
    AIIO_AT_BLE_MODE_PERIPHERAL = 0,
    AIIO_AT_BLE_MODE_CENTRAL = 1,
    AIIO_AT_BLE_MODE_IBEACON = 2,
    AIIO_AT_BLE_MODE_STOP = 9,
} aiio_at_ble_mode_t;

typedef enum {
    AIIO_AT_BLE_STATE_DISCONNECTED,
    AIIO_AT_BLE_STATE_CONNECTED,
} aiio_at_ble_state_t;

typedef enum {
    AIIO_AT_BLE_SCAN_LIST,
    AIIO_AT_BLE_SCAN_CONN,
} aiio_at_ble_scan_type_t;

typedef struct
{
    int8_t rssi;
    uint8_t mac[6];
    char name[AT_BLE_LOCAL_NAME_SIZE];
} aiio_at_ble_scan_info_t;

typedef struct {
    int8_t power;
    uint16_t company_id;
    uint16_t major;
    uint16_t minor;
    uint8_t uuid[BLE_UUID128_SIZE];
} aiio_at_ble_ibeacon_t;

typedef struct {
    uint8_t flag;
    uint8_t mac[6];
    uint16_t uuid16;
} aiio_at_ble_autoconn_t;

typedef struct
{
    int8_t power;
    int8_t state;
    int8_t scan_type;
    uint8_t mode;
    uint8_t mac[6];
    aiio_at_ble_ibeacon_t ibeacon;
    aiio_ble_uuid_128_t srv_tt_uuid;
    aiio_ble_uuid_128_t tt_tx_uuid;
    aiio_ble_uuid_128_t tt_rx_uuid;
    aiio_ble_uuid_128_t srv_cmd_uuid;
    aiio_ble_uuid_128_t cmd_tx_uuid;
    aiio_ble_uuid_128_t cmd_rx_uuid;
    struct {
        uint8_t adv_data[AT_BLE_ADV_DATA_SIZE];
        uint16_t adv_length;
    } adv_cfg;
    uint16_t adv_intv_max;
    uint16_t adv_intv_min;
    aiio_ble_conn_interval_t conn_intv;
    aiio_at_ble_autoconn_t autoconn;
    char blename[AT_BLE_LOCAL_NAME_SIZE + 1];
} aiio_at_ble_config_t;

CORE_API int32_t aiio_at_ble_cmd_regist(void);
CORE_API int32_t aiio_ble_auto_connect_onboot(void);
CORE_API int32_t aiio_ble_get_custom_mac(uint8_t *mac);

extern aiio_at_ble_config_t aiio_at_ble_config;

#endif
