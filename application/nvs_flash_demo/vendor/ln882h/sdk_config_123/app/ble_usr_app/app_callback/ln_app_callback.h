/**
 ****************************************************************************************
 *
 * @file ln_app_callback.h
 *
 * @brief APP callback function.
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef _LN_APP_CALLBACK_H_
#define _LN_APP_CALLBACK_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @group BLE
 *
 * @brief APP callback function.
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "gap.h"                     // GAP Definition
#include "gapm_task.h"               // GAP Manager Task API
#include "gapc_task.h"               // GAP Controller Task API
#include "gattc_task.h"
#include "gattm_task.h"
#include "att.h"
#include "rwip_task.h" // Task definitions
#include "ke_msg.h"
#include "ln_gatt_callback.h"
#include "ln_app_gap.h"
/*
 * DEFINES
 ****************************************************************************************
 */


/// Application environment information structure
struct app_env_info_tag
{
    struct gapm_activity_created_ind actv_creat_info;
    struct gapm_activity_stopped_ind actv_stop_info;
    struct gapm_ext_adv_report_ind  ext_adv_rep_info;
    struct gapm_profile_added_ind  prof_add;
    struct gapm_cmp_evt   cmp_evt_info;
    struct ln_gapc_connection_req_info conn_req_info;
    struct gapc_param_update_req_ind param_upd_info;
    struct gapc_cmp_evt     gapc_cmp_info;
    struct ln_gapc_disconnect_info dis_conn_info;
    struct gapm_gen_rand_nb_ind    gen_rand;
    struct gapm_ral_addr_ind  ral_addr_info;
    struct gapm_dev_version_ind      dev_version_info;
    struct gapm_dev_bdaddr_ind    dev_bdadr_info;
    struct gapm_dev_adv_tx_power_ind    adv_tx_power;
    struct gapm_sugg_dflt_data_len_ind  dflt_data_len;
    struct gapm_max_data_len_ind      max_data_len;
    struct gapm_list_size_ind             list_size;
    struct gapm_nb_adv_sets_ind     nb_adv_sets;
    struct gapm_max_adv_data_len_ind   max_adv_data_len;
    struct gapm_dev_tx_pwr_ind   dev_tx_pwr;
    struct gapm_addr_solved_ind   addr_solv_info;
    struct gapm_use_enc_block_ind   use_enc_block;
    struct gapm_scan_request_ind    scan_req_info;
    struct gapm_sync_established_ind sync_established_info;
    struct gapm_peer_name_ind        peer_name_info;
    struct gapm_le_test_end_ind     le_test_end_info;
    struct gapm_pub_key_ind   pub_key_info;
    struct gapc_peer_att_info_ind   peer_att_info;
    struct gapc_peer_version_ind    peer_version_info;
    struct gapc_peer_features_ind    peer_features_info;
    struct gapc_con_rssi_ind          con_rssi_info;
    struct  gapc_con_channel_map_ind    con_channel_map_info;
    struct gapc_le_ping_to_val_ind     le_ping_to_val_info;
    struct gapc_le_phy_ind    le_phy_info;
    struct gapc_chan_sel_algo_ind    chan_sel_algo_info;
    struct gapc_param_updated_ind    param_updated_info;
    struct gapc_bond_ind        bond_ind_info;
    struct gapc_bond_req_ind   bond_req_info;
    struct  gapc_key_press_notif_ind    key_press_notif_info;
    struct gapc_encrypt_ind    encrypt_ind_info;
    struct gapc_encrypt_req_ind  encrypt_req_info;
    struct  gapc_security_ind  security_ind_info;
    struct gapc_le_pkt_size_ind    le_pkt_size_info;
    struct gapc_sign_counter_ind  sign_counter_info;
    struct gapc_get_dev_info_req_ind  get_dev_info_req_info;
    struct gapc_set_dev_info_req_ind  set_dev_info_req_info;
    struct gattm_add_svc_rsp      add_svc_rsp_info;
    struct gattm_svc_get_permission_rsp  svc_get_permission_rsp_info;
    struct gattm_svc_set_permission_rsp  svc_set_permission_rsp_info;
    struct gattm_att_get_permission_rsp    att_get_permission_rsp_info;
    struct gattm_att_set_permission_rsp    att_set_permission_rsp_info;
    struct gattm_att_get_value_rsp        att_get_value_rsp_info;
    struct gattm_att_set_value_rsp  att_set_value_rsp_info;
    struct gattc_cmp_evt   gattc_cmp_evt_info;
    struct gattc_mtu_changed_ind   mtu_changed_ind_info;
    struct gattc_disc_svc_ind    disc_svc_ind_info;
    struct gattc_disc_svc_incl_ind  disc_svc_incl_info;
    struct gattc_disc_char_ind    disc_char_info;
    struct gattc_disc_char_desc_ind  disc_char_desc_info;
    struct gattc_read_ind  read_ind_info;
    struct gattc_event_ind  event_ind_info;
    struct gattc_read_req_ind  read_req_info;
    struct gattc_write_req_ind  write_req_info;
    struct gattc_att_info_req_ind  att_info_req_info;
    struct gattc_sdp_svc_ind   sdp_svc_info;
    struct gattc_event_ind event_req_info;
};
/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */



#endif // _LN_APP_CALLBACK_H_

