#include "ln_ble_event_manager.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_err_code.h"
#include "ln882h.h"

static ble_event_cb_t ble_evt_cb[BLE_EVT_ID_MAX] = {NULL};
static void _dummy_func(void * arg){};

int ln_ble_evt_mgr_init(void)
{
    CRITICAL_SECT_START();
    for (int i = 0; i < BLE_EVT_ID_MAX; i++){
        ble_evt_cb[i] = _dummy_func;
    }
    CRITICAL_SECT_END();

    return BLE_ERR_NONE;
}

int ln_ble_evt_mgr_deinit(void)
{
    CRITICAL_SECT_START();
    for (int i = 0; i < BLE_EVT_ID_MAX; i++){
        ble_evt_cb[i] = _dummy_func;
    }
    CRITICAL_SECT_END();

    return BLE_ERR_NONE;
}

int  ln_ble_evt_mgr_reg_evt(ble_event_id_t evt_id, ble_event_cb_t cb)
{
    if (evt_id >= BLE_EVT_ID_MAX) {
        return BLE_ERR_INVALID_PARAM;
    }

    CRITICAL_SECT_START();
    if (cb == NULL) {
        ble_evt_cb[evt_id] = _dummy_func;
    } else {
        ble_evt_cb[evt_id] = cb;
    }
    CRITICAL_SECT_END();

    return BLE_ERR_NONE;
}

int ln_ble_evt_mgr_process(ble_event_id_t evt_id, void * arg)
{
    ble_evt_cb[evt_id](arg);

    return BLE_ERR_NONE;
}

