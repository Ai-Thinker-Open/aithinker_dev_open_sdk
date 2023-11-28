#ifndef __LN_BLE_TRANS_CLIENT_H__
#define __LN_BLE_TRANS_CLIENT_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"


#if (BLE_DATA_TRANS_CLIENT)

typedef struct {
    uint8_t conid;
    uint16_t svr_hdl_start;
    uint16_t svr_hdl_end;
} data_trans_cli_t;

typedef struct
{
    /// connection index
    uint8_t conn_idx;
    /// characteristic handle
    uint16_t hdl;
    /// length of packet to send
    uint16_t len;
    /// point to value buff
    uint8_t* data;
} ln_trans_cli_send_t;


void ln_ble_trans_disc_svc(uint8_t conidx, uint8_t *svc_uuid, uint8_t svc_uuid_len);
void ln_ble_trans_cli_read(uint8_t conidx, uint16_t handle, uint16_t len);
void ln_ble_trans_cli_write_cmd(ln_trans_cli_send_t *param);
void ln_ble_trans_cli_write_req(ln_trans_cli_send_t *param);
int ln_ble_trans_cli_init(void);

#endif
#endif /* __LN_BLE_TRANS_CLIENT_H__ */

