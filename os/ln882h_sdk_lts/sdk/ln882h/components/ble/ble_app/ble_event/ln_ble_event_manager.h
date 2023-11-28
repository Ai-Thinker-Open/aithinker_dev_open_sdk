#ifndef __LN_BLE_EVENT_MANAGER_H__
#define __LN_BLE_EVENT_MANAGER_H__

#include <stdint.h>
#include <string.h>

typedef enum 
{
    BLE_EVT_ID_STARTUP             = 0, /**<  BLE startup */
    BLE_EVT_ID_CONNECTED           = 1, /**<  BLE connected */
    BLE_EVT_ID_DISCONNECTED        = 2, /**<  BLE disconnected */

    BLE_EVT_ID_SCAN_REPORT         = 3, /**<  BLE scan report information */

    /*ble slave role event*/
    BLE_EVT_ID_GATT_READ_REQ       = 4, /**<  gatt read request from client */
    BLE_EVT_ID_GATT_WRITE_REQ      = 5, /**<  gatt write request from client */

    /*ble master role event*/
    BLE_EVT_ID_GATT_DISC_SVC       = 6, /**<  gatt respone discover server from server */
    BLE_EVT_ID_GATT_READ_RSP       = 7, /**<  gatt read respond from server */
    BLE_EVT_ID_GATT_NOTIFY         = 8, /**<  gatt notify from server */
    BLE_EVT_ID_GATT_INDICATE       = 9, /**<  gatt indication from server */

    BLE_EVT_ID_MAX,
} ble_event_id_t;

typedef struct
{
    uint8_t  conn_idx;        /* Connection index */
    uint16_t con_interval;    /* Connection interval */ 
    uint16_t con_latency;     /* Connection latency */
    uint16_t sup_to;          /* Link supervision timeout */
    uint8_t  peer_addr_type;  /* Peer address type */
    uint8_t  peer_addr[6];    /* Peer BT address */
    uint8_t  role;            /* Role of device in connection (0 = Master / 1 = Slave) */
} ble_evt_connected_t;

typedef struct
{
    uint8_t conn_idx;         /* Connection index */
    uint8_t reason;           /* Reason of disconnection */
} ble_evt_disconnected_t;

typedef struct
{
    uint8_t  actv_idx;        /* Activity identifier */
    uint8_t  info;            /* Bit field providing information about the received report (@see enum gapm_adv_report_info) */
    uint8_t  trans_addr_type;
    uint8_t  trans_addr[6];   /* Transmitter device address */
    uint8_t  target_addr_type;
    uint8_t  target_addr[6];  /* Target address (in case of a directed advertising report) */
    int8_t   tx_pwr;          /* TX power (in dBm) */
    int8_t   rssi;            /* RSSI (between -127 and +20 dBm) */

    uint16_t length;          /* Report length */
    uint8_t  data[0];         /* Report data */
} ble_evt_scan_report_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t handle;          /* Handle of the attribute that has to be written */
    uint16_t length;          /* read Data length */
    uint8_t  *value;
} ble_evt_gatt_read_req_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t handle;          /* Handle of the attribute that has to be written */
    uint16_t offset;          /* offset at which the data has to be written */
    uint16_t length;          /* Data length to be written */
    uint8_t  value[0];        /* Data to be written in attribute database */
} ble_evt_gatt_write_req_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t start_hdl;       /* service start handle */
    uint16_t end_hdl;         /* service end handle */
    uint8_t  uuid_len;        /* service UUID length */
    uint8_t  uuid[0];         /* service UUID */
} ble_evt_gatt_svr_disc_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t handle;          /* Handle of the attribute that has to be written */
    uint16_t offset;          /* offset at which the data has to be read */
    uint16_t length;          /* Data length */
    uint8_t  value[0];
} ble_evt_gatt_read_rsp_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t handle;          /* Handle of the attribute that has to be written */
    uint16_t length;          /* Data length */
    uint8_t  value[0];
} ble_evt_gatt_notify_t;

typedef struct
{
    uint8_t  conidx;          /* Connection index */
    uint16_t handle;          /* Handle of the attribute that has to be written */
    uint16_t length;          /* Data length */
    uint8_t  value[0];
} ble_evt_gatt_indicate_t;

typedef void (*ble_event_cb_t)(void * arg);


int  ln_ble_evt_mgr_init(void);
int  ln_ble_evt_mgr_deinit(void);
int  ln_ble_evt_mgr_reg_evt(ble_event_id_t evt_id, ble_event_cb_t cb);
int  ln_ble_evt_mgr_process(ble_event_id_t evt_id, void * arg);

#endif

