#ifndef __LN_BLE_BATTERY_SERVER_H__
#define __LN_BLE_BATTERY_SERVER_H__

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"

#if (BLE_BATT_SERVER)


#define LN_NB_BAS_INSTANCES_MAX         (2)

/// Parameters of the @ref BASS_ENABLE_REQ message
typedef struct
{
    /// connection index
    uint8_t conidx;
    /// Notification Configuration
    uint8_t ntf_cfg;
    /// Old Battery Level used to decide if notification should be triggered
    uint8_t old_batt_lvl[LN_NB_BAS_INSTANCES_MAX];
} batt_svr_enable_req_t;

///Parameters of the @ref BASS_BATT_LEVEL_UPD_REQ message
typedef struct
{
    /// BAS instance
    uint8_t bas_instance;
    /// Battery Level
    uint8_t batt_level;
} batt_svr_level_upd_req_t;


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
} gatt_batt_svr_t;

extern gatt_batt_svr_t batt_server;

enum batt_svc_att_idx
{
    BATT_DECL_SVC = 0,
    BATT_DECL_LVL_CHAR,
    BATT_DECL_LVL_CHAR_VAL,
    BATT_DECL_LVL_CHAR_CCC,
    BATT_DECL_LVL_PRES_FMT,

    BATT_IDX_MAX,
};

#define HANDLE_BATT_SERVER_START          0x0000
#define HANDLE_BATT_DECL_LVL_CHAR         0x0001
#define HANDLE_BATT_DECL_LVL_CHAR_VAL     0x0002
#define HANDLE_BATT_DECL_LVL_CHAR_CCC     0x0003
#define HANDLE_BATT_DECL_LVL_PRES_FMT     0x0004
#define HANDLE_BATT_SERVER_END            0xFFFF

/*
 * Configuration Flag, each bit matches with an attribute of
 * att_db (Max: 32 attributes); if the bit is set to 1, the
 * attribute will be added in the service
*/
#define BATT_CONFIG_MASK         (0x1FFF)


void ln_batt_server_add(void);
void ln_batt_svr_enable_prf(batt_svr_enable_req_t * p_req);
void ln_batt_svr_send_lvl(uint8_t conidx, batt_svr_level_upd_req_t *p_req);
int ln_batt_server_init(void);

#endif
#endif /* __LN_BLE_BATTERY_SERVER_H__ */



