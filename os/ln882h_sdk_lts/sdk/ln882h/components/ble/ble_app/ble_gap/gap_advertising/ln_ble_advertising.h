#ifndef LN_BLE_ADVERTISING_H_
#define LN_BLE_ADVERTISING_H_

#include <stdint.h>
#include <stdbool.h>

#define BLE_USE_LEGACY_ADV           (1)

#define APP_ADV_CHMAP                (0x07)  // Advertising channel map - 37, 38, 39
#define APP_ADV_INT_MIN              (160)   // Advertising minimum interval - 40ms (64*0.625ms)
#define APP_ADV_INT_MAX              (160)   // Advertising maximum interval - 40ms (64*0.625ms)

#define ADV_DATA_LEGACY_MAX          (28)
#define ADV_DATA_EXTEND_MAX          (247)

/* Data type for the state of LE Advertising Manager */
typedef enum
{
    /*! Advertising Manager module has not yet been initialised */
    LE_ADV_STATE_UNINITIALIZED,
    /*! Advertising Manager module has been initialised */
    LE_ADV_STATE_INITIALIZED,
    /*! Advertising is starting */
    LE_ADV_STATE_STARTING,
    /*! Advertising has been started */
    LE_ADV_STATE_STARTED,
    /*! Advertising is stoping */
    LE_ADV_STATE_STOPING,
    /*! Advertising has been stoped */
    LE_ADV_STATE_STOPED,
} ble_adv_state_t;

/// Set advertising, scan response or periodic advertising data command
/*@TRACE*/
typedef struct
{
    /// Data length
    uint16_t length;
    //point to adv data
    uint8_t* data;
} ln_adv_data_t;

/* Data type for the Advertising paramter */
typedef struct
{
    /// Own address type (@see enum gapm_own_addr)
    uint8_t own_addr_type;
    /// adv type (@see enum gapm_adv_type)
    uint8_t adv_type;
    /// adv prop (@see enum gapm_leg_adv_prop or enum gapm_ext_adv_prop)
    uint8_t adv_prop;
    /// Minimum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_min;
    /// Maximum advertising interval (in unit of 625us). Must be greater than 20ms
    uint32_t adv_intv_max;
} adv_param_t;

typedef struct
{
    uint8_t adv_actv_idx;
    ble_adv_state_t state;
    adv_param_t adv_param;
} ln_ble_adv_manager_t;

extern ln_ble_adv_manager_t ble_adv_mgr;
/* get advertising manager private data struct */
#define le_adv_mgr_info_get()          (&ble_adv_mgr)
#define le_adv_state_get()             (ble_adv_mgr.state)
#define le_adv_state_set(sta)          (ble_adv_mgr.state = sta)
#define le_adv_actv_idx_get()          (ble_adv_mgr.adv_actv_idx)
#define le_adv_actv_idx_set(id)        (ble_adv_mgr.adv_actv_idx = id)


/**
 ****************************************************************************************
 * @brief Set  scan response data.
 *
 * @param[in] scan_rsp_data     Pointer to scan response data structure.
 *
 ****************************************************************************************
 */
int ln_ble_adv_scan_rsp_data_set(ln_adv_data_t *scan_rsp_data);

/**
 ****************************************************************************************
 * @brief Set  advertising data.
 *
 * 
 * @param[in] adv_data      Pointer to adv data structure.
 *
 ****************************************************************************************
 */
int ln_ble_adv_data_set(ln_adv_data_t *adv_data);

/**
 ****************************************************************************************
 * @brief Create a advertising activity
 *
 * @param[in] void.
 *
 ****************************************************************************************
 */
int ln_ble_adv_actv_creat(adv_param_t *param);

/**
 ****************************************************************************************
 * @brief start ble advertising
 *
 ****************************************************************************************
 */
void ln_ble_adv_start(void);

/**
 ****************************************************************************************
 * @brief stop ble advertising
 *
 ****************************************************************************************
 */
void ln_ble_adv_stop(void);

int ln_ble_adv_mgr_init(void);
    
#endif

