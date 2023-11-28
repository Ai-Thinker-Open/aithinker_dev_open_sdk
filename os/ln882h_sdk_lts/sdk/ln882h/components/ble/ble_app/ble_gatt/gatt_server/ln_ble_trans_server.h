#ifndef __LN_BLE_TRANS_SERVER_H__
#define __LN_BLE_TRANS_SERVER_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_defines.h"

#if (BLE_DATA_TRANS_SERVER)

#define ATT_UUID_MAX_LEN            16

typedef struct
{
    /// attribute UUID, LSB First
    uint8_t uuid[ATT_UUID_MAX_LEN];
    /// Attribute Permissions (@see enum attm_perm_mask)
    uint16_t perm;
    /// Attribute Extended Permissions (@see enum attm_value_perm_mask)
    uint16_t ext_perm;
    /// Attribute Max Size
    uint16_t max_size;
} ln_attm_desc_t;

typedef struct
{
    uint16_t start_handle;
    uint8_t svr_uuid[ATT_UUID_MAX_LEN];
    uint8_t svr_uuid_len;
    uint8_t att_count;
    ln_attm_desc_t *att_desc;
} ln_trans_svr_desc_t;

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
} ln_trans_svr_send_t;


typedef struct
{
    uint8_t svr_count;
} data_trans_svr_t;

int ln_ble_trans_svr_add(ln_trans_svr_desc_t *param);
void ln_ble_trans_svr_ntf(ln_trans_svr_send_t *param);
void ln_ble_trans_svr_ind(ln_trans_svr_send_t *param);
int ln_ble_trans_svr_init(void);

#endif
#endif /* __LN_BLE_TRANS_SERVER_H__ */



