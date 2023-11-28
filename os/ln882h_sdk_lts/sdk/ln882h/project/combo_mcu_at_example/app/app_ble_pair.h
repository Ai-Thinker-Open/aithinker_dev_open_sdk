#ifndef _APP_BLE_PAIR_H_
#define _APP_BLE_PAIR_H_

#include "rwip_config.h"
#include "llm_int.h"

#include "ln_ble_gap.h"
#include "ln_ble_gatt.h"
#include "usr_app.h"
#include "usr_ble_app.h"
#include "usr_ctrl.h"
#include "wifi_driver_port.h"
#include "utils/system_parameter.h"
#include "ln_ble_advertising.h"
#include "ln_ble_connection_manager.h"
#include "ln_ble_event_manager.h"
#include "ln_ble_app_kv.h"
#include "usr_ble_app.h"
#include "gap.h"
#include "ln_at.h"
#include "utils/debug/log.h"

#define AT_BLE_SSID_LEN 64
#define AT_BLE_PWD_LEN  32

#define AT_BLE_RESPONSE_DATA "BLE CONFIG NET OK"

#define BLE_PAIR_TASK_STACK_SIZE   6*256 //Byte

typedef struct
{
    void (*start_adv)(uint8_t *adv_name, uint8_t len);
    void (*response_get_link_info)(void);
    void (*stop_adv)(void);
}at_ble_pair_func_t;

typedef struct
{
    OS_Semaphore_t connect_sem;
    OS_Semaphore_t get_pwd_sem;
}at_ble_pair_semaphore_t;

typedef struct
{
    bool ble_pair_start_flag;
    uint8_t pair_status;
    char *ssid;
    char *pwd;
    OS_Mutex_t mutex;
    OS_Thread_t thread;
    at_ble_pair_semaphore_t sem;
    at_ble_pair_func_t func;
}at_ble_pair_manage_t;

#endif