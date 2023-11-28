#ifndef _LN_BLE_APP_DEFAULT_CFG_H_
#define _LN_BLE_APP_DEFAULT_CFG_H_

#include "ble_app_user_cfg.h" // for user defined
#include "rwip_config.h"      // for CFG_CON
#include "rwprf_config.h"      // for CFG_CON

#if (BLE_DATA_TRANS_SERVER)
#include "ln_ble_trans_server.h"
#endif
#if (BLE_DATA_TRANS_CLIENT)
#include "ln_ble_trans_client.h"
#endif
#if (BLE_BATT_SERVER)
#include "ln_ble_battery_server.h"
#endif
#if (BLE_BATT_CLIENT)
#include "ln_ble_battery_client.h"
#endif
#if (BLE_HID_DEVICE)
#include "ln_ble_hid_server.h"
#endif
#if (BLE_HID_REPORT_HOST)
#include "ln_ble_hid_client.h"
#endif

typedef enum
{
    BLE_ROLE_UNKNOWN     = 0,
    BLE_ROLE_PERIPHERAL  = (1<<0),
    BLE_ROLE_CENTRAL     = (1<<1),
    BLE_ROLE_ALL         = (BLE_ROLE_PERIPHERAL | BLE_ROLE_CENTRAL),
} ble_role_type_t;


#if !defined BLE_DEFAULT_ROLE
#define BLE_DEFAULT_ROLE                BLE_ROLE_PERIPHERAL
#endif

#if !defined BLE_DEFAULT_DEVICE_NAME
#define BLE_DEFAULT_DEVICE_NAME         ("ln882h")
#endif

#if !defined BLE_DEVICE_NAME_LEN
#define BLE_DEVICE_NAME_LEN             (sizeof(BLE_DEFAULT_DEVICE_NAME))
#endif

#if !defined BLE_DEV_NAME_MAX_LEN
#define BLE_DEV_NAME_MAX_LEN            (40)
#endif

#if !defined BLE_DEFAULT_PUBLIC_ADDR
#define BLE_DEFAULT_PUBLIC_ADDR         ({0x56, 0x34, 0x12, 0x03, 0xFF, 0x00})
#endif

#if !defined APP_ACTV_INVALID_IDX
#define APP_ACTV_INVALID_IDX            (0xFF)
#endif

#if !defined APP_CONN_INVALID_IDX
#define APP_CONN_INVALID_IDX            (0xFF)
#endif

#if !defined APP_CONN_INVALID_HANDLE
#define APP_CONN_INVALID_HANDLE         (0xFFFF)
#endif


#if !defined BLE_CONFIG_AUTO_ADV
#define BLE_CONFIG_AUTO_ADV             (1)
#endif

#if !defined BLE_CONFIG_AUTO_SCAN
#define BLE_CONFIG_AUTO_SCAN            (0)
#endif

/* enable ble data encrypt */
#if !defined BLE_CONFIG_SECURITY
#define BLE_CONFIG_SECURITY            (0)
#endif

/* support multiple connection */
#if !defined BLE_CONFIG_MULTI_CONNECT
#define BLE_CONFIG_MULTI_CONNECT       (0)
#endif

#if (BLE_CONFIG_MULTI_CONNECT == 0)
  #undef BLE_CONN_DEV_NUM_MAX
  #define BLE_CONN_DEV_NUM_MAX         (1)
#else
  #if !defined BLE_CONN_DEV_NUM_MAX
    #define BLE_CONN_DEV_NUM_MAX       (CFG_CON)
  #endif
#endif

/* user data trans Server  */
#if !defined BLE_DATA_TRANS_SERVER
#define BLE_DATA_TRANS_SERVER          (1)
#endif

/* user data trans Client  */
#if !defined BLE_DATA_TRANS_CLIENT
#define BLE_DATA_TRANS_CLIENT          (0)
#endif


#endif /* _LN_BLE_APP_DEFAULT_CFG_H_ */
