/**
 ****************************************************************************************
 *
 * @file ln_ble_gap_ind_handler.c
 *
 * @brief GAP callback funcion source code
 *
 * Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manallger Task API
#include "gapc_task.h"               // GAP Controller Task API

#include "utils/debug/log.h"

#include "ln_ble_advertising.h"
#include "ln_ble_scan.h"
#include "ln_ble_gap.h"
#include "ln_ble_connection_manager.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_default_cfg.h"


static int ln_gap_get_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_get_dev_info_req_ind *p_param = (struct gapc_get_dev_info_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gap_get_dev_info_req_ind_handler param->req=0x%x,conidx=0x%x\r\n",p_param->req,conidx);

    switch(p_param->req)
    {
        case GAPC_DEV_NAME:
        {
            struct gapc_get_dev_info_cfm * cfm = KE_MSG_ALLOC_DYN(GAPC_GET_DEV_INFO_CFM,
                                                 src_id, dest_id,
                                                 gapc_get_dev_info_cfm, APP_DEVICE_NAME_MAX_LEN);
            cfm->req = p_param->req;
            cfm->info.name.length = BLE_DEVICE_NAME_LEN;
            memcpy(cfm->info.name.value, BLE_DEFAULT_DEVICE_NAME, BLE_DEVICE_NAME_LEN);

            // Send message
            ke_msg_send(cfm);
        }
            break;

        case GAPC_DEV_APPEARANCE:
        {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                                src_id, dest_id,
                                                gapc_get_dev_info_cfm);
            cfm->req = p_param->req;
            // Set the device appearance
            // Send message
            ke_msg_send(cfm);
        }
            break;

        case GAPC_DEV_SLV_PREF_PARAMS:
        {
            // Allocate message
            struct gapc_get_dev_info_cfm *cfm = KE_MSG_ALLOC(GAPC_GET_DEV_INFO_CFM,
                                                src_id, dest_id,
                                                gapc_get_dev_info_cfm);
            cfm->req = p_param->req;
            // Slave preferred Connection interval Min
            cfm->info.slv_pref_params.con_intv_min = 8;
            // Slave preferred Connection interval Max
            cfm->info.slv_pref_params.con_intv_max = 10;
            // Slave preferred Connection latency
            cfm->info.slv_pref_params.slave_latency  = 0;
            // Slave preferred Link supervision timeout
            cfm->info.slv_pref_params.conn_timeout    = 200;  // 2s (500*10ms)

            // Send message
            ke_msg_send(cfm);
        }
            break;

        default: /* Do Nothing */
            break;
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gap_set_dev_info_req_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_set_dev_info_req_ind *p_param = (struct gapc_set_dev_info_req_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gap_set_dev_info_req_ind_handler param->req=0x%x,conidx=0x%x\r\n",p_param->req,conidx);

    // Set Device configuration
    struct gapc_set_dev_info_cfm* cfm = KE_MSG_ALLOC(GAPC_SET_DEV_INFO_CFM, src_id, dest_id,
                                        gapc_set_dev_info_cfm);
    // Reject to change parameters
    cfm->status = 0;
    cfm->req = p_param->req;
    ke_msg_send(cfm);

    return (KE_MSG_CONSUMED);
}

static int ln_gap_mgr_cmd_evt_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_cmp_evt *p_param = (struct gapm_cmp_evt *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_mgr_cmd_evt_handler: operation=0x%x, status=0x%x\r\n", p_param->operation,p_param->status);

    switch(p_param->operation)
    {
        // Reset completed
        case (GAPM_RESET):
        {
            if(p_param->status == GAP_ERR_NO_ERROR)
            {
                ln_gap_set_dev_config_t cfg_param;
                memset(&cfg_param,0,sizeof(ln_gap_set_dev_config_t));
                // Set Data length parameters
                cfg_param.sugg_max_tx_octets = BLE_MIN_OCTETS;
                cfg_param.sugg_max_tx_time   = BLE_MIN_TIME;
                // Host privacy enabled by default
                cfg_param.privacy_cfg = 0;
                cfg_param.role    = GAP_ROLE_ALL;
                cfg_param.max_mtu = 1024;//2048;
                cfg_param.max_mps = 1024;//2048;
#if (BLE_CONFIG_SECURITY)
                cfg_param.pairing_mode = GAPM_PAIRING_LEGACY | GAPM_PAIRING_SEC_CON;
#endif
                ln_gap_set_dev_config(&cfg_param);
            }
            else
            {
                ASSERT_ERR(0);
            }
        }
        break;

        // Device Configuration updated
        case (GAPM_SET_DEV_CONFIG):
        {
            ASSERT_INFO(p_param->status == GAP_ERR_NO_ERROR, p_param->operation, p_param->status);
        }
        break;

        case (GAPM_START_ACTIVITY):
        {
            if(p_param->status == GAP_ERR_NO_ERROR)
            {
                if(le_adv_actv_idx_get() == p_param->actv_idx)
                    le_adv_state_set(LE_ADV_STATE_STARTED);
                if(le_scan_actv_idx_get() == p_param->actv_idx)
                    le_scan_state_set(LE_SCAN_STATE_STARTED);
            }
        }
        break;

        case (GAPM_STOP_ACTIVITY):
        {
            if(p_param->status == GAP_ERR_NO_ERROR)
            {
                if(le_adv_actv_idx_get() == p_param->actv_idx)
                    le_adv_state_set(LE_ADV_STATE_STOPED);
                if(le_scan_actv_idx_get() == p_param->actv_idx)
                    le_scan_state_set(LE_SCAN_STATE_STOPED);
            }
        }
        break;

        default:
            break;
    }
    return (KE_MSG_CONSUMED);
}

static int ln_gap_ctl_cmd_evt_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_cmp_evt const *p_param = (struct gapc_cmp_evt const *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gap_ctl_cmd_evt_handler operation=0x%x,status=0x%x,conidx=0x%x\r\n",p_param->operation,p_param->status,conidx);

    return (KE_MSG_CONSUMED);

}

static int ln_gap_activity_created_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_activity_created_ind *p_param = (struct gapm_activity_created_ind *)param;

    LOG(LOG_LVL_TRACE, "ln_gap_activity_created_ind_handler: actv_idx=%d,actv_type=%d \r\n",p_param->actv_idx,p_param->actv_type);

    if(p_param->actv_type == GAPM_ACTV_TYPE_ADV)
        le_adv_actv_idx_set(p_param->actv_idx);

    if(p_param->actv_type == GAPM_ACTV_TYPE_INIT)
        le_conn_actv_idx_set(p_param->actv_idx);

    if(p_param->actv_type == GAPM_ACTV_TYPE_SCAN)
        le_scan_actv_idx_set(p_param->actv_idx);

    return (KE_MSG_CONSUMED);
}

static int ln_gap_activity_stopped_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_activity_stopped_ind *p_param = (struct gapm_activity_stopped_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_activity_stopped_ind_handler actv_id=%d   actv_type=%d, reason=0x%x\r\n", p_param->actv_idx, p_param->actv_type,p_param->reason);

    if(p_param->actv_type == GAPM_ACTV_TYPE_ADV)
    {
        le_adv_state_set(LE_ADV_STATE_STOPED);
        if(BLE_CONFIG_AUTO_ADV && le_conn_mgr_info_get()->conn_num < BLE_CONN_DEV_NUM_MAX)
            ln_ble_adv_start();
    }
    if(p_param->actv_type == GAPM_ACTV_TYPE_SCAN)
    {
        le_scan_state_set(LE_SCAN_STATE_STOPED);
        if(BLE_CONFIG_AUTO_SCAN){
            ln_ble_scan_start(&le_scan_mgr_info_get()->scan_param);
        }
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gap_pkt_size_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapc_le_pkt_size_ind *p_param = (struct gapc_le_pkt_size_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE, "ln_gap_pkt_size_ind_handler conidx=0x%x,max_rx_octets:%d,max_rx_time:%d,max_tx_octets:%d, max_tx_time :%d\r\n",conidx,p_param->max_rx_octets,p_param->max_rx_time,p_param->max_tx_octets,p_param->max_tx_time);

    return (KE_MSG_CONSUMED);
}

int ln_gap_profile_added_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
    struct gapm_profile_added_ind *p_param = (struct gapm_profile_added_ind *)param;
    // Current State
    ke_state_t state = ke_state_get(dest_id);

    LOG(LOG_LVL_TRACE,"gapm_profile_added_ind_handler profile_task_id:0x%x,state=0x%x\r\n",p_param->prf_task_id,state);

    switch (p_param->prf_task_id)
    {
        case TASK_ID_BASS:
        {
#if (BLE_BATT_SERVER)
            batt_server.prf_task_id = p_param->prf_task_id;
            batt_server.prf_task_nb = p_param->prf_task_nb;
            batt_server.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        case TASK_ID_BASC:
        {
#if (BLE_BATT_CLIENT)
            batt_client.prf_task_id = p_param->prf_task_id;
            batt_client.prf_task_nb = p_param->prf_task_nb;
            batt_client.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        case TASK_ID_DISS:
        {
#if (BLE_DIS_SERVER)
            disc_server.prf_task_id = p_param->prf_task_id;
            disc_server.prf_task_nb = p_param->prf_task_nb;
            disc_server.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        case TASK_ID_DISC:
        {
#if (BLE_DIS_CLIENT)
            disc_client.prf_task_id = p_param->prf_task_id;
            disc_client.prf_task_nb = p_param->prf_task_nb;
            disc_client.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        case TASK_ID_HOGPD:
        {
#if (BLE_HID_DEVICE)
            hid_server.prf_task_id = p_param->prf_task_id;
            hid_server.prf_task_nb = p_param->prf_task_nb;
            hid_server.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        case TASK_ID_HOGPRH:
        {
#if (BLE_HID_REPORT_HOST)
            hid_client.prf_task_id = p_param->prf_task_id;
            hid_client.prf_task_nb = p_param->prf_task_nb;
            hid_client.start_hdl = p_param->start_hdl;
#endif
        }
            break;

        default:
            break;
    }

    return (KE_MSG_CONSUMED);
}

static int ln_gap_peer_name_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapm_peer_name_ind *p_param = (struct gapm_peer_name_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_peer_name_ind_handler\r\n");

    return (KE_MSG_CONSUMED);
}

static int ln_gap_local_version_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapm_dev_version_ind *p_param = (struct gapm_dev_version_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_local_version_ind_handler\r\n");

    return (KE_MSG_CONSUMED);
}

static int ln_gap_peer_version_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapc_peer_version_ind *p_param = (struct gapc_peer_version_ind *)param;
    uint8_t conidx = KE_IDX_GET(src_id);

    LOG(LOG_LVL_TRACE,"ln_gap_peer_version_ind_handler conidx=0x%x\r\n",conidx);

    return (KE_MSG_CONSUMED);
}

static int ln_gap_local_bdaddr_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapm_dev_bdaddr_ind *p_param = (struct gapm_dev_bdaddr_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_local_bdaddr_ind_handler \r\n");

    return (KE_MSG_CONSUMED);
}

static int ln_gap_dev_tx_pwr_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapm_dev_tx_pwr_ind *p_param = (struct gapm_dev_tx_pwr_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_dev_tx_pwr_ind_handler \r\n");

    return (KE_MSG_CONSUMED);
}

static int ln_gap_dev_adv_tx_power_ind_handler(ke_msg_id_t const msgid,
        void const *param,
        ke_task_id_t const dest_id,
        ke_task_id_t const src_id)
{
//    struct gapm_dev_adv_tx_power_ind *p_param = (struct gapm_dev_adv_tx_power_ind *)param;

    LOG(LOG_LVL_TRACE,"ln_gap_dev_adv_tx_power_ind_handler \r\n");

    return (KE_MSG_CONSUMED);
}

struct ke_msg_handler app_gap_general_msg_tab[] =
{
    //GAPM
    {GAPM_CMP_EVT,                   ln_gap_mgr_cmd_evt_handler},
    {GAPM_DEV_VERSION_IND,           ln_gap_local_version_ind_handler},
    {GAPM_DEV_BDADDR_IND,            ln_gap_local_bdaddr_ind_handler},
    {GAPC_LE_PKT_SIZE_IND,           ln_gap_pkt_size_ind_handler},
    {GAPM_DEV_ADV_TX_POWER_IND,      ln_gap_dev_adv_tx_power_ind_handler},
    //{GAPM_ADDR_SOLVED_IND,         gapm_addr_solved_ind_handler},
    //{GAPM_USE_ENC_BLOCK_IND,       gapm_use_enc_block_handler},
    //{GAPM_GEN_RAND_NB_IND,         gapm_gen_rand_nb_ind_handler},
    //{GAPM_SUGG_DFLT_DATA_LEN_IND,  gapm_sugg_dflt_data_len_ind_handler},
    //{GAPM_MAX_DATA_LEN_IND,        gapm_max_data_len_ind_handler},
    //{GAPM_LIST_SIZE_IND,           gapm_list_size_ind_handler},
    //{GAPM_NB_ADV_SETS_IND,         gapm_nb_adv_sets_ind_handler},
    //{GAPM_MAX_ADV_DATA_LEN_IND,    gapm_max_adv_data_len_ind_handler},
    {GAPM_DEV_TX_PWR_IND,            ln_gap_dev_tx_pwr_ind_handler},
    //{GAPM_RAL_ADDR_IND,            gapm_ral_addr_ind_handler},
    //{GAPM_SYNC_ESTABLISHED_IND,    gapm_sync_established_ind_handler},
    {GAPM_PEER_NAME_IND,             ln_gap_peer_name_ind_handler},
    {GAPM_PROFILE_ADDED_IND,         ln_gap_profile_added_ind_handler},
    //{GAPM_LE_TEST_END_IND,         gapm_le_test_end_ind_handler},
    //{GAPM_PUB_KEY_IND,             gapm_pub_key_ind_handler},
    //{GAPM_GEN_DH_KEY_IND,          gapm_gen_dh_key_ind_handler},
    {GAPM_ACTIVITY_CREATED_IND,      ln_gap_activity_created_ind_handler},
    {GAPM_ACTIVITY_STOPPED_IND,      ln_gap_activity_stopped_ind_handler},
    {GAPC_CMP_EVT,                   ln_gap_ctl_cmd_evt_handler },

    //{GAPM_SCAN_REQUEST_IND,        gapm_scan_request_ind_handler},

    {GAPC_GET_DEV_INFO_REQ_IND,      ln_gap_get_dev_info_req_ind_handler},
    {GAPC_SET_DEV_INFO_REQ_IND,      ln_gap_set_dev_info_req_ind_handler},
    //{GAPC_PEER_ATT_INFO_IND,       gapc_peer_att_info_ind_handler},
    {GAPC_PEER_VERSION_IND,          ln_gap_peer_version_ind_handler},
    //{GAPC_PEER_FEATURES_IND,       gapc_peer_features_ind_handler},
    //{GAPC_CON_RSSI_IND,            gapc_con_rssi_ind_handler},
    //{GAPC_CON_CHANNEL_MAP_IND,     gapc_con_channel_map_ind_handler},
    //{GAPC_LE_PHY_IND,              gapc_le_phy_ind_handler},
    //{GAPC_CHAN_SEL_ALGO_IND,       gapc_chan_sel_algo_ind_handler},
};

int app_gap_msg_size(void)
{
    return sizeof((app_gap_general_msg_tab))/sizeof((app_gap_general_msg_tab)[0]);
}

