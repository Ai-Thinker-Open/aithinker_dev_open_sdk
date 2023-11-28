/**
 ****************************************************************************************
 *
 * @file ln_app_gap.c
 *
 * @brief GAP callback function  source code
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "rwip_config.h"
#include "ke_task.h"                 // Kernel Task
#include "gap.h"                     // GAP Definition
#include "gapm.h"
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "co_utils.h"

#include "utils/debug/log.h"

#include "ln_ble_gap.h"
#include "ln_ble_gap_ind_handler.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_kv.h"


void ln_gap_reset(void)
{
    // Reset the stack
    struct gapm_reset_cmd *p_cmd = KE_MSG_ALLOC(GAPM_RESET_CMD,
                                   TASK_GAPM, TASK_APP,
                                   gapm_reset_cmd);
    p_cmd->operation = GAPM_RESET;
    ln_ke_msg_send(p_cmd);
}

void ln_gap_set_dev_config(ln_gap_set_dev_config_t *cfg_param)
{
    bd_addr_t *loc_addr = gapm_get_bdaddr();

    struct gapm_set_dev_config_cmd *p_cmd = KE_MSG_ALLOC(GAPM_SET_DEV_CONFIG_CMD,
                                            TASK_GAPM, TASK_APP,
                                            gapm_set_dev_config_cmd);
    p_cmd->operation = GAPM_SET_DEV_CONFIG;
    p_cmd->role = cfg_param->role;
    p_cmd->renew_dur = cfg_param->renew_dur;
    memcpy(p_cmd->addr.addr, cfg_param->addr, GAP_BD_ADDR_LEN);
    memcpy(p_cmd->irk.key, cfg_param->irk, GAP_KEY_LEN);
    p_cmd->privacy_cfg = cfg_param->privacy_cfg;
    if(cfg_param->privacy_cfg & GAPM_PRIV_CFG_PRIV_EN_BIT) {
        bd_addr_t *loc_addr = gapm_get_bdaddr();
        uint8_t *irk = ln_kv_ble_irk_get();
        if(loc_addr->addr[5] & 0xc0)
            p_cmd->privacy_cfg |= GAPM_PRIV_CFG_PRIV_ADDR_BIT;
        if(irk) {
            memcpy(p_cmd->irk.key, irk, KEY_LEN);
        } else {
            extern uint8_t debug_irk_key[];
            memcpy(p_cmd->irk.key, debug_irk_key, KEY_LEN);
        }
        memcpy(p_cmd->addr.addr, loc_addr->addr, LN_BD_ADDR_LEN);
    }

    p_cmd->pairing_mode = cfg_param->pairing_mode;
    p_cmd->gap_start_hdl = cfg_param->gap_start_hdl;
    p_cmd->gatt_start_hdl = cfg_param->gatt_start_hdl;
    p_cmd->att_cfg = (cfg_param->att_devname_write_perm & 0x7) |
                     ((cfg_param->att_apperance_write_perm & 0x7) << 3) |
                     ((cfg_param->att_slv_pref_conn_param_present & 0x1) << 6) |
                     ((cfg_param->svc_change_feat_present & 0x1) << 7);
    p_cmd->sugg_max_tx_octets = cfg_param->sugg_max_tx_octets;
    p_cmd->sugg_max_tx_time = cfg_param->sugg_max_tx_time;
    p_cmd->max_mtu = cfg_param->max_mtu;
    p_cmd->max_mps = cfg_param->max_mps;
    p_cmd->max_nb_lecb = cfg_param->max_nb_lecb;
    p_cmd->tx_pref_phy = cfg_param->tx_pref_phy;
    p_cmd->rx_pref_phy = cfg_param->rx_pref_phy;
    p_cmd->tx_path_comp = cfg_param->tx_path_comp;
    p_cmd->rx_path_comp = cfg_param->rx_path_comp;
    ln_ke_msg_send(p_cmd);
}

void ln_gap_get_dev_info_by_type(uint8_t type)
{
    int operation;
    switch (type)
    {
        case GET_INFO_VERSION:
        operation = GAPM_GET_DEV_VERSION;
        break;
        case GET_INFO_BDADDR:
        operation = GAPM_GET_DEV_BDADDR;
        break; 
        case GET_INFO_ADV_TX_POWER:
        operation = GAPM_GET_DEV_ADV_TX_POWER;
        break;
        case GET_INFO_TX_POWER:
        operation = GAPM_GET_DEV_TX_PWR;
        break;
        case GET_INFO_SUGG_DFT_DATA:
        operation = GAPM_GET_SUGGESTED_DFLT_LE_DATA_LEN;
        break;
        case GET_INFO_SUPP_DFT_DATA:
        operation = GAPM_GET_MAX_LE_DATA_LEN;
        break;
        case GET_INFO_MAX_ADV_DATA_LEN:
        operation = GAPM_GET_MAX_LE_ADV_DATA_LEN;
        break;
        case GET_INFO_WLIST_SIZE:
        operation = GAPM_GET_WLIST_SIZE;
        break;
        case GET_INFO_RAL_SIZE:
        operation = GAPM_GET_RAL_SIZE;
        break;
        case GET_INFO_PAL_SIZE:
        operation = GAPM_GET_PAL_SIZE;
        break;
        case GET_INFO_NB_ADV_SETS:
        operation = GAPM_GET_NB_ADV_SETS;
        break;
        default:
            break;
    }
    struct gapm_get_dev_info_cmd *p_cmd = KE_MSG_ALLOC(GAPM_GET_DEV_INFO_CMD,
                                          TASK_GAPM, TASK_APP,
                                          gapm_get_dev_info_cmd);
    p_cmd->operation = operation;
    ln_ke_msg_send(p_cmd);
}

void ln_gap_get_peer_info_by_type(int conidx, uint8_t type)
{
    struct gapc_get_info_cmd *p_cmd = KE_MSG_ALLOC( GAPC_GET_INFO_CMD,
                                      KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                      gapc_get_info_cmd);
    switch(type)
    {
        case GET_PEER_NAME:
            p_cmd->operation = GAPC_GET_PEER_NAME;
            break;
        case GET_PEER_VERSION:
            p_cmd->operation = GAPC_GET_PEER_VERSION;
            break;
        case GET_PEER_FEATURES:
            p_cmd->operation = GAPC_GET_PEER_FEATURES;
            break;
        case GET_CON_RSSI:
            p_cmd->operation = GAPC_GET_CON_RSSI;
            break;
        case GET_CON_CHANNEL_MAP:
            p_cmd->operation = GAPC_GET_CON_CHANNEL_MAP;
            break;
        case GET_PEER_APPEARANCE:
            p_cmd->operation = GAPC_GET_PEER_APPEARANCE;
            break;
        case GET_PEER_SLV_PREF_PARAMS:
            p_cmd->operation = GAPC_GET_PEER_SLV_PREF_PARAMS;
            break;
        case GET_ADDR_RESOL_SUPP:
            p_cmd->operation = GAPC_GET_ADDR_RESOL_SUPP;
            break;
        case GET_LE_PING_TO:
            p_cmd->operation = GAPC_GET_LE_PING_TO;
            break;
        case GET_PHY:
            p_cmd->operation = GAPC_GET_PHY;
            break;
        case GET_CHAN_SEL_ALGO:
            p_cmd->operation = GAPC_GET_CHAN_SEL_ALGO;
            break;
        default:
            break;
    }
    ln_ke_msg_send(p_cmd);
}

void ln_gap_set_le_pkt_size(int conidx, ln_gap_set_le_pkt_size_t *pkt_size)
{
    struct gapc_set_le_pkt_size_cmd *p_cmd = KE_MSG_ALLOC(GAPC_SET_LE_PKT_SIZE_CMD,
                                          KE_BUILD_ID(TASK_GAPC, conidx), TASK_APP,
                                          gapc_set_le_pkt_size_cmd);
    p_cmd->operation = GAPC_SET_LE_PKT_SIZE;
    p_cmd->tx_octets = pkt_size->tx_octets;
    p_cmd->tx_time   = pkt_size->tx_time;
    ln_ke_msg_send(p_cmd);
}

void ln_gap_set_white_list(ln_gap_set_wl_t *wlist)
{
    struct gapm_list_set_wl_cmd *p_cmd = KE_MSG_ALLOC_DYN( GAPM_LIST_SET_CMD,
                                         TASK_GAPM, TASK_APP,
                                         gapm_list_set_wl_cmd, (wlist->size * sizeof(struct gap_bdaddr)));
    p_cmd->operation = GAPM_SET_WL;
    p_cmd->size = wlist->size;
    for (uint32_t i = 0; i < wlist->size; i++)
    {
        p_cmd->wl_info[i].addr_type = wlist->wl_info[i].addr_type;
        memcpy(p_cmd->wl_info[i].addr.addr, wlist->wl_info[i].addr.addr, BD_ADDR_LEN);
    }
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gap_set_phy(int conidx, ln_gap_set_phy_t *p_phy)
{
    struct gapc_set_phy_cmd *p_cmd = KE_MSG_ALLOC(GAPC_SET_PHY_CMD,KE_BUILD_ID(TASK_GAPC,conidx),
                                     TASK_APP, gapc_set_phy_cmd);
    p_cmd->operation = GAPC_SET_PHY;
    p_cmd->tx_phy = p_phy->tx_phy;
    p_cmd->rx_phy = p_phy->rx_phy;
    p_cmd->phy_opt= p_phy->phy_opt;
    // Send the message
    ln_ke_msg_send(p_cmd);
}

void ln_gap_set_chn_map(ln_gap_set_channel_map_t *p_chn_map)
{
    struct gapm_set_channel_map_cmd *p_cmd = KE_MSG_ALLOC( GAPM_SET_CHANNEL_MAP_CMD,
                                                TASK_GAPM, TASK_APP,
                                                gapm_set_channel_map_cmd);
    p_cmd->operation = GAPM_SET_CHANNEL_MAP;
    memcpy(p_cmd->chmap.map, p_chn_map->chmap, GAP_LE_CHNL_MAP_LEN);
    // Send the message
    ln_ke_msg_send(p_cmd);
}

int ln_gap_app_init(void)
{
    int ret = 0;

    ret = ln_app_msg_regester(app_gap_general_msg_tab, app_gap_msg_size());

    return ret;
}
