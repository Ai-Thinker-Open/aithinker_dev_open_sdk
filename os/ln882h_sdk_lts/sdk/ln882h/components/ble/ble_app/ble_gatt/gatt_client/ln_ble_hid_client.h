#ifndef __LN_BLE_HID_CLIENT_H__
#define __LN_BLE_HID_CLIENT_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"


#if (BLE_HID_REPORT_HOST)

typedef struct {
    int conid;
    /// Profile task identifier
    uint16_t prf_task_id;
    /// Profile task number allocated
    uint16_t prf_task_nb;
    /// Service start handle
    /// Only applies for services - Ignored by collectors
    uint16_t start_hdl;
} gatt_hid_cli_t;

extern gatt_hid_cli_t hid_client;


void ln_hid_client_add(void);
void ln_hid_cli_discover_server(uint8_t conidx);
int ln_hid_client_init(void);

#endif
#endif /* __LN_BLE_HID_CLIENT_H__ */

