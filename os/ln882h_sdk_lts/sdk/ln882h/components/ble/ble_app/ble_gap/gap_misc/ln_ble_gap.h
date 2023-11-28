/**
 ****************************************************************************************
 *
 * @file ln_ble_gap.h
 *
 * @brief GAP API.
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_BLE_GAP_H_
#define _LN_BLE_GAP_H_

#include <stdint.h>
#include <stdbool.h>

#include "ln_ble_app_defines.h"


/// get device local info command.
enum ln_get_dev_info_cmd
{
    /// Get local device version
    GET_INFO_VERSION,
    /// Get local device BD Address
    GET_INFO_BDADDR,
    /// Get device advertising power level
    GET_INFO_ADV_TX_POWER,
     /// Get minimum and maximum transmit powers supported by the controller
    GET_INFO_TX_POWER,
    /// Get suggested default data length
    GET_INFO_SUGG_DFT_DATA,
    /// Get supported default data length
    GET_INFO_SUPP_DFT_DATA,
    /// Get maximum advertising data length supported by the controller.
    GET_INFO_MAX_ADV_DATA_LEN,
    /// Get White List Size.
    GET_INFO_WLIST_SIZE,
    /// Get resolving address list size.
    GET_INFO_RAL_SIZE,
    /// Get periodic advertiser list size.
    GET_INFO_PAL_SIZE,
    /// Get number of available advertising sets.
    GET_INFO_NB_ADV_SETS,
};

// get peer device info command.
enum ln_get_peer_info
{
    GET_PEER_NAME,
    GET_PEER_VERSION,
    GET_PEER_FEATURES,
    GET_CON_RSSI,
    GET_CON_CHANNEL_MAP,
    GET_PEER_APPEARANCE,
    GET_PEER_SLV_PREF_PARAMS,
    GET_ADDR_RESOL_SUPP,
    GET_LE_PING_TO,
    GET_PHY,
    GET_CHAN_SEL_ALGO,  
};

/// Parameters of the @ref GAPC_SET_LE_PKT_SIZE_CMD message
/*@TRACE*/
typedef struct
{
    ///Preferred maximum number of payload octets that the local Controller should include
    ///in a single Link Layer Data Channel PDU.
    uint16_t tx_octets;
    ///Preferred maximum number of microseconds that the local Controller should use to transmit
    ///a single Link Layer Data Channel PDU
    uint16_t tx_time;
} ln_gap_set_le_pkt_size_t;

/// Set device configuration command
/*@TRACE*/
typedef struct
{
    /// Device Role: Central, Peripheral, Observer, Broadcaster or All roles.
    uint8_t role;

    /// -------------- Privacy Config -----------------------
    /// Duration before regenerate device address when privacy is enabled. - in seconds
    uint16_t renew_dur;
    /// Provided own static private random address
    uint8_t addr[LN_BD_ADDR_LEN];
    /// Device IRK used for resolvable random BD address generation (LSB first)
    uint8_t irk[LN_GAP_KEY_LEN];
    /// Privacy configuration bit field (@see enum gapm_priv_cfg for bit signification)
    uint8_t privacy_cfg;

    /// -------------- Security Config -----------------------

    /// Pairing mode authorized (@see enum gapm_pairing_mode)
    uint8_t pairing_mode;

    /// -------------- ATT Database Config -----------------------

    /// GAP service start handle
    uint16_t gap_start_hdl;
    /// GATT service start handle
    uint16_t gatt_start_hdl;

/// Attribute database configuration
    /// Device Name write permission @see gapm_write_att_perm
    uint8_t att_devname_write_perm;
    /// Device Appearance write permission @see gapm_write_att_perm
    uint8_t att_apperance_write_perm;
    /// Slave Preferred Connection Parameters present ( 1 = yes, 0 = no )
    uint8_t att_slv_pref_conn_param_present;
    /// Service change feature present in GATT attribute database ( 1 = yes, 0 = no )
    uint8_t svc_change_feat_present;

    /// -------------- LE Data Length Extension -----------------------
    ///Suggested value for the Controller's maximum transmitted number of payload octets to be used
    uint16_t sugg_max_tx_octets;
    ///Suggested value for the Controller's maximum packet transmission time to be used
    uint16_t sugg_max_tx_time;

    /// --------------- L2CAP Configuration ---------------------------
    /// Maximal MTU acceptable for device
    uint16_t max_mtu;
    /// Maximal MPS Packet size acceptable for device
    uint16_t max_mps;
    /// Maximum number of LE Credit based connection that can be established
    uint8_t  max_nb_lecb;

    /// --------------- LE Audio Mode Supported -----------------------
    ///
    /// LE Audio Mode Configuration (@see gapm_audio_cfg_flag)
    uint16_t  audio_cfg;

    /// ------------------ LE PHY Management  -------------------------
    /// Preferred LE PHY for data transmission (@see enum gap_phy)
    uint8_t tx_pref_phy;
    /// Preferred LE PHY for data reception (@see enum gap_phy)
    uint8_t rx_pref_phy;

    /// ------------------ Miscellaneous 2 ----------------------------
    /// RF TX Path Compensation value (from -128dB to 128dB, unit is 0.1dB)
    uint16_t tx_path_comp;
    /// RF RX Path Compensation value (from -128dB to 128dB, unit is 0.1dB)
    uint16_t rx_path_comp;
} ln_gap_set_dev_config_t;

/// Set content of white list
/*@TRACE*/
typedef struct
{
    /// Number of entries to be added in the list. 0 means that list content has to be cleared
    uint8_t size;
    /// point to List of entries to be added in the list
    ln_bdaddr_t* wl_info;
} ln_gap_set_wl_t;

/// Set device phy
/*@TRACE*/
typedef struct
{
    /// Supported LE PHY for data transmission (@see enum gap_phy)
    uint8_t tx_phy;
    /// Supported LE PHY for data reception (@see enum gap_phy)
    uint8_t rx_phy;
    /// PHY options (@see enum gapc_phy_option)
    uint8_t phy_opt;
} ln_gap_set_phy_t;

/// Set device channel map
/*@TRACE*/
typedef struct
{
    /// Channel map
    uint8_t chmap[LN_BLE_CHNL_MAP_LEN];
} ln_gap_set_channel_map_t;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void ln_gap_reset(void);
void ln_gap_get_dev_info_by_type(uint8_t type);
void ln_gap_get_peer_info_by_type(int conidx, uint8_t type);
void ln_gap_set_le_pkt_size(int conidx, ln_gap_set_le_pkt_size_t *pkt_size);
void ln_gap_set_dev_config(ln_gap_set_dev_config_t *cfg_param);
void ln_gap_set_white_list(ln_gap_set_wl_t *wlist);
void ln_gap_set_phy(int conidx, ln_gap_set_phy_t *p_phy);
void ln_gap_set_chn_map(ln_gap_set_channel_map_t *p_chn_map);
int ln_gap_app_init(void);

#endif

