#ifndef LN_BLE_APP_KV_H_
#define LN_BLE_APP_KV_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_defines.h"
#include "ln_ble_device_manager.h"


#define BLE_APP_NVDS_MAGIC_NUM  0xa5a5


typedef enum {
    BT_KV_ERR_NONE          = 0,
    BT_KV_ERR_INVALID_PARAM = 1,
    BT_KV_ERR_STORE         = 2,
    BT_KV_ERR_LOAD          = 3,
    BT_KV_ERR_RESET_ALL     = 4,
} bt_kv_err_t;

typedef struct {
    uint16_t name_len;
    uint8_t  name[BLE_DEV_NAME_MAX_LEN];
} kv_name_info_t;

typedef struct {
    uint8_t vailed;
    uint8_t irk[16];
} kv_irk_info_t;

typedef struct {
    uint16_t magic;
    uint8_t ble_role;
    bt_white_list_t white_list[MAX_DEV_WHITE_LIST_MUN];
    bt_device_info_t pair_list_dev[MAX_DEV_BOND_MUN];
} kv_usr_data_t;


int             ln_kv_ble_name_store(const uint8_t *name, uint16_t name_len);
uint8_t        *ln_kv_ble_dev_name_get(uint16_t *name_len);
int             ln_kv_ble_addr_store(ln_bd_addr_t addr);
ln_bd_addr_t   *ln_kv_ble_pub_addr_get(void);
int             ln_kv_ble_irk_store(uint8_t irk[16]);
uint8_t        *ln_kv_ble_irk_get(void);
int             ln_kv_ble_usr_data_store(void);
kv_usr_data_t  *ln_kv_ble_usr_data_get(void);
int             ln_kv_ble_factory_reset(void);
int             ln_kv_ble_app_init(void);

#endif
