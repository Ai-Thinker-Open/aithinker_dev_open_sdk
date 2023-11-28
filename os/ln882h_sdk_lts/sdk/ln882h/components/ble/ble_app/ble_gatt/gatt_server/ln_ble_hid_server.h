#ifndef __LN_BLE_HID_SERVER_H__
#define __LN_BLE_HID_SERVER_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"


#if (BLE_HID_DEVICE)

/// Request sending of a report to the host - notification
typedef struct
{
    /// Connection Index
    uint8_t conidx;
    /// HIDS Instance
    uint8_t  hid_idx;
    /// type of report (@see enum hogpd_report_type)
    uint8_t  type;
    /// Report Length (uint8_t)
    uint16_t length;
    /// Report Instance - 0 for boot reports and report map
    uint8_t  idx;
    /// Report data
    uint8_t value[0];
} hid_svr_report_upd_req_t;

typedef struct
{
    int conid;
    /// Profile task identifier
    uint16_t prf_task_id;
    /// Profile task number allocated
    uint16_t prf_task_nb;
    /// Service start handle
    /// Only applies for services - Ignored by collectors
    uint16_t start_hdl;
    /// Service end handle
    uint16_t end_hdl;
    ///Characteristic client config
    uint16_t value_ccc;
} gatt_hid_svr_t;

extern gatt_hid_svr_t hid_server;


void ln_hid_server_add(void);
void ln_hid_svr_enable(int conidx);
void ln_hid_svr_report_upd_req(hid_svr_report_upd_req_t *p_param);
int ln_hid_server_init(void);

#endif
#endif /* __LN_BLE_HID_SERVER_H__ */



