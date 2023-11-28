#ifndef LN_BLE_DEVICE_MANAGER_H_
#define LN_BLE_DEVICE_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_defines.h"


#define MAX_DEV_NAME_LEN                32
#define MAX_DEV_BOND_MUN                8
#define MAX_DEV_WHITE_LIST_MUN          4

#define DEV_LIST_INVAILD_INDEX          0xff
/*! \brief Types of device link modes. */
typedef enum
{
    DEVICE_LINK_MODE_UNKNOWN = 0,           /*!< The Bluetooth link mode is unknown */
    DEVICE_LINK_MODE_NO_SECURE_CONNECTION,  /*!< The Bluetooth link is not using secure connections */
    DEVICE_LINK_MODE_SECURE_CONNECTION,     /*!< The Bluetooth link is using secure connections */
} deviceLinkMode;

/*! \brief Types of device link modes. */
typedef enum
{
    BLE_DEVICE_ADD = 0,           /*!< The Bluetooth link mode is unknown */
    BLE_DEVICE_DELETE,  /*!< The Bluetooth link is not using secure connections */
    BLE_DEVICE_UPDATE,     /*!< The Bluetooth link is using secure connections */
} dev_mgr_operate_t;

typedef struct
{
    uint8_t role;
    uint8_t addr_type;
    ln_bd_addr_t addr;
    ln_gap_irk_t irk;
    ln_gap_ltk_t ltk;
    uint8_t mru;
} bt_device_info_t;

typedef struct
{
    uint8_t addr_type;
    ln_bd_addr_t addr;
} bt_white_list_t;


bool ln_ble_dev_is_exist(bt_device_info_t *dev);
uint8_t ln_ble_get_index_by_addr(ln_bd_addr_t *addr);
bt_device_info_t *ln_ble_get_dev_by_addr(ln_bd_addr_t *addr);
int ln_ble_dev_insert(bt_device_info_t *new_dev);
int ln_ble_dev_delete(bt_device_info_t *dev);

#endif

