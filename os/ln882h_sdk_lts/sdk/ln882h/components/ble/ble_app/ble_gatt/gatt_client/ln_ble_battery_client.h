#ifndef __LN_BLE_BATTERY_CLIENT_H__
#define __LN_BLE_BATTERY_CLIENT_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"

#if (BLE_BATT_CLIENT)

///Parameters of the @ref BASC_BATT_LEVEL_NTF_CFG_REQ message
typedef struct 
{
    ///Notification Configuration
    uint16_t ntf_cfg;
    ///Battery Service Instance - From 0 to BASC_NB_BAS_INSTANCES_MAX-1
    uint8_t bas_nb;
} batt_cli_ntf_cfg_req_t;

///Parameters of the @ref BASC_READ_INFO_REQ message
typedef struct
{
    ///Characteristic info @see enum basc_info
    uint8_t info;
    ///Battery Service Instance - From 0 to BASC_NB_BAS_INSTANCES_MAX-1
    uint8_t bas_nb;
} batt_cli_read_info_req_t;

typedef struct {
    int conid;
    /// Profile task identifier
    uint16_t prf_task_id;
    /// Profile task number allocated
    uint16_t prf_task_nb;
    /// Service start handle
    /// Only applies for services - Ignored by collectors
    uint16_t start_hdl;
} gatt_battery_cli_t;

extern gatt_battery_cli_t batt_client;


void ln_batt_client_add(void);
void ln_batt_cli_discover_server(uint8_t conidx);
void ln_batt_cli_ntf_cfg_req(uint8_t conidx, batt_cli_ntf_cfg_req_t *p_req);
void ln_batt_cli_rd_char_req(uint8_t conidx, batt_cli_read_info_req_t * p_req);
int ln_batt_client_init(void);


#endif
#endif /* __LN_BLE_BATTERY_CLIENT_H__ */

