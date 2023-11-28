/**
 ****************************************************************************************
 *
 * @file ln_app_gap.h
 *
 * @brief GAP API.
 *
 * Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef LN_BLE_SCAN_H_
#define LN_BLE_SCAN_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_defines.h"

#define SCAN_INTERVAL_DEF        (0xa0) 
#define SCAN_WINDOW_DEF          (0x50)

#define SCAN_RPT_DATA_MAX_LENGTH    31

/*! States used internally by the LE Scan Manager Module */
typedef enum
{
    /*! Scan Manager module has not yet been initialised */
    LE_SCAN_STATE_UNINITIALIZED = 0,
    /*! Scan Manager module has been initialised */
    LE_SCAN_STATE_INITIALIZED,
    /*! Scan is starting */
    LE_SCAN_STATE_STARTING,
    /*! Scan has been started */
    LE_SCAN_STATE_STARTED,
    /*! Scan is stoping */
    LE_SCAN_STATE_STOPING,
    /*! Scan has been stoped */
    LE_SCAN_STATE_STOPED,
} le_scan_state_t;

typedef enum
{
    SCAN_FILTER_TYPE_RSSI        = (1<<0),
    SCAN_FILTER_TYPE_TX_PWR      = (1<<1),
    SCAN_FILTER_TYPE_PHY         = (1<<2),
    SCAN_FILTER_TYPE_ADDR_TYPE   = (1<<3),
    SCAN_FILTER_TYPE_ADDR        = (1<<4),
    SCAN_SFILTER_TYPE_ADV_DATA   = (1<<5),
} le_scan_filter_type_t;

typedef struct
{
    uint8_t len;
    uint8_t type;
    uint8_t data[1];
} adv_data_item_t;

/*! \brief LE advertising report filter. */
typedef struct
{
    uint16_t filter_type;//@see le_scan_filter_type_t
    int8_t rssi;
    int8_t tx_pwr;
    uint8_t phy;
    uint8_t find_addr_type;
    uint8_t find_addr[LN_BD_ADDR_LEN];
    uint8_t pattern_len;
    uint8_t pattern_data[SCAN_RPT_DATA_MAX_LENGTH];
} le_scan_report_filter_t;

/*! \brief LE scan parameters. */
typedef struct
{
    /// Type of scanning to be started (@see enum gapm_scan_type)
    uint8_t type;
    /// Properties for the scan procedure (@see enum gapm_scan_prop for bit signification)
    uint8_t prop;
    /// Duplicate packet filtering policy
    uint8_t dup_filt_pol;
    /// Scan interval
    uint16_t scan_intv;
    /// Scan window
    uint16_t scan_wd;
} le_scan_parameters_t;

typedef struct {
    uint8_t scan_actv_idx;
    /*! state for Scan Manager Module */
    le_scan_state_t state;
    /*! scan result report filter*/
    le_scan_report_filter_t filter;
    /*! scan parameters */
    le_scan_parameters_t scan_param;
} ln_ble_scan_manager_t;

extern ln_ble_scan_manager_t ble_scan_mgr;
/* get scan manager private data struct */
#define le_scan_mgr_info_get()          (&ble_scan_mgr)
#define le_scan_state_get()             (ble_scan_mgr.state)
#define le_scan_state_set(sta)          (ble_scan_mgr.state = sta)
#define le_scan_actv_idx_get()          (ble_scan_mgr.scan_actv_idx)
#define le_scan_actv_idx_set(id)        (ble_scan_mgr.scan_actv_idx = id)


/**
 ****************************************************************************************
 * @brief find advertising item by type
 *
 * @param[in] type              advertising item type.
 * @param[in] data              advertising data.
 * @param[in] len               advertising data length.
 *
 ****************************************************************************************
 */
adv_data_item_t *ln_ble_scan_data_find_item_by_type(uint8_t type, uint8_t *data, uint8_t len);

/**
 ****************************************************************************************
 * @brief Create a scanning activity
 *
 * @param[in] scan_creat_param              Pointer to activity create structure.
 *
 ****************************************************************************************
 */
void ln_ble_scan_actv_creat(void);


/**
 ****************************************************************************************
 * @brief set scan report filter
 *
 ****************************************************************************************
 */
void ln_ble_scan_set_rpt_filter(le_scan_report_filter_t *filter);

/**
 ****************************************************************************************
 * @brief clear all scan report filter
 *
 ****************************************************************************************
 */
void ln_ble_scan_clear_rpt_filter(void);

/**
 ****************************************************************************************
 * @brief Start a scanning activity
 *
 ****************************************************************************************
 */
void ln_ble_scan_start(le_scan_parameters_t *param);

/**
 ****************************************************************************************
 * @brief Stop a scanning activity
 *
 ****************************************************************************************
 */
void ln_ble_scan_stop(void);

/**
 ****************************************************************************************
 * @brief scan manager Module initialize
 *
 ****************************************************************************************
 */
int ln_ble_scan_mgr_init(void);

#endif
