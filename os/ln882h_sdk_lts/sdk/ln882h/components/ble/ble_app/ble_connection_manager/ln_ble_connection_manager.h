#ifndef LN_BLE_CONNECTION_MANAGER_H_
#define LN_BLE_CONNECTION_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_device_manager.h"
#include "ln_ble_app_default_cfg.h"

typedef enum
{
    /*! connection Manager module has not yet been initialised */
    LE_CONN_STATE_UNINITIALIZED,
    /*! connection Manager module has been initialised */
    LE_CONN_STATE_INITIALIZED,
    /*! ble state disconnected */
    LE_CONN_STATE_DISCONNECTED,
    /*! ble is connecting */
    LE_CONN_STATE_CONNECTING,
    /*! ble state connected */
    LE_CONN_STATE_CONNECTED,
    /*! ble is disconnecting */
    LE_CONN_STATE_DISCONNECTING,
} ble_conn_state_t;

typedef enum {
    /*slow connect mode for low power*/
    LN_BLE_CONN_MODE_SLOW,
    /*normal connect mode*/
    LN_BLE_CONN_MODE_NORMAL,
    /*fast mode high speed data transfer*/
    LN_BLE_CONN_MODE_FAST,

    LN_BLE_CONN_MODE_MAX
} ln_ble_conn_mode_t;

/*! Connection Parameter used to update connection parameters */
typedef struct
{
    /* Connection interval minimum, range: 0x06~0x0c80, time = N*1.25ms (7.5ms--4s) */
    uint16_t intv_min;
    /* Connection interval maximum, range: 0x06~0x0c80, time = N*1.25ms (7.5ms--4s) */
    uint16_t intv_max;
    /* Latency, range: 0~0x01f3 */
    uint16_t latency;
    /* Supervision timeout, range:0x0a~0x0c80, time = N*10ms (100ms--32s) */
    uint16_t time_out;
} ln_ble_conn_param_t;

/*! connection device environment variablen */
typedef struct
{
    uint16_t conn_handle;
    bool is_bonded;
    bt_device_info_t dev_remote;
    ble_conn_state_t state;
} le_dev_con_env_t;

typedef struct
{
    uint8_t init_actv_idx;
    uint8_t conn_num;
    ln_ble_conn_param_t con_param;
    le_dev_con_env_t con_env[BLE_CONN_DEV_NUM_MAX];
} ble_dev_conn_manager_t;

extern ble_dev_conn_manager_t ble_conn_mgr;
/* get advertising manager private data struct */
#define le_conn_mgr_info_get()          (&ble_conn_mgr)
#define le_conn_actv_idx_get()          (ble_conn_mgr.init_actv_idx)
#define le_conn_actv_idx_set(id)        (ble_conn_mgr.init_actv_idx = id)


/**
****************************************************************************************
* @brief set connect work mode
* @note: Either master or slave can call this function.
*
* @param[in] conidx                Connection index.
* @param[in] connect mode          (@see enum ln_ble_conn_mode_t)
*
****************************************************************************************
*/
void ln_ble_conn_mode_set(int conidx, ln_ble_conn_mode_t mode);

/**
****************************************************************************************
* @brief set connection paramter
* @note: Either master or slave can call this function.
*
* @param[in] conidx                Connection index.
* @param[in] p_conn_param          (@see enum ln_ble_conn_param_t)
*
****************************************************************************************
*/
void ln_ble_conn_param_update(int conidx, ln_ble_conn_param_t *p_conn_param);

/**
 ****************************************************************************************
 * @brief Disconnect a link
 * @note: Either master or slave can call this function.
 *
 * @param[in] conidx                Connection index.
 *
 *
 ****************************************************************************************
 */
void ln_ble_disc_req(int conidx);

/**
 ****************************************************************************************
 * @brief create a link
 * @note: only master can call this function.
 *
 * @param[in] addr                peer deivce addr.
 *
 ****************************************************************************************
 */
void ln_ble_conn_req(ln_bd_addr_t *addr);

/**
 ****************************************************************************************
 * @brief create initer activity
 * @note: Either master or slave can call this function.
 *
 ****************************************************************************************
 */
void ln_ble_init_actv_creat(void);

/**
 ****************************************************************************************
 * @brief provides connection manager private data to other components
 *
 * @param[in]  void
 *
 * @return connection manager private data struct.
 *
 ****************************************************************************************
 */
bool ln_ble_is_connected(void);

/**
 ****************************************************************************************
 * @brief ble connection manager Module initialize
 *
 ****************************************************************************************
 */
int ln_ble_conn_mgr_init(void);

#endif
