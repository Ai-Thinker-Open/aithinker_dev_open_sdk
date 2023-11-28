#define struct_func_dcl(name, type, param)                                      \
    type name##_get_##param(void *pa);                                          \
    void name##_set_##param(void *pa, type val)                                 \

#define struct_func_len(name)                                                   \
    int name##_len()                                                            \

struct_func_dcl(rxu_mgt_ind, uint16_t, length);
struct_func_dcl(rxu_mgt_ind, uint8_t, inst_nbr);
struct_func_dcl(rxu_mgt_ind, uint32_t*, payload);
struct_func_dcl(rxu_mgt_ind, uint16_t, center_freq);
struct_func_dcl(rxu_mgt_ind, int8_t, rssi);

struct_func_dcl(scanu_start_cfm, uint8_t, vif_idx);
struct_func_dcl(scanu_start_cfm, uint8_t, status);
struct_func_dcl(scanu_start_cfm, uint8_t, result_cnt);

struct_func_dcl(sm_connect_ind, uint8_t, vif_idx);
struct_func_dcl(sm_connect_ind, uint16_t, status_code);
struct_func_dcl(sm_connect_ind, uint16_t, ieeetypes_code);
void *sm_connect_ind_getp_bssid(void* param);
struct_func_dcl(sm_connect_ind, uint16_t, assoc_req_ie_len);
struct_func_dcl(sm_connect_ind, uint16_t, assoc_rsp_ie_len);
struct_func_dcl(sm_connect_ind, uint8_t, security);
uint16_t sm_connect_ind_get_chan_prim20_freq(void* param);
struct_func_dcl(sm_connect_ind, uint8_t, ap_idx);
struct_func_dcl(sm_connect_ind, uint8_t, ch_idx);
struct_func_dcl(sm_connect_ind, uint32_t*, assoc_ie_buf);
struct_func_dcl(sm_connect_ind, uint8_t, acm);
struct_func_dcl(sm_connect_ind, uint16_t, aid);
struct_func_dcl(sm_connect_ind, struct mac_chan_op, chan);
struct_func_dcl(sm_connect_ind, bool, qos);
struct_func_dcl(sm_disconnect_ind, uint16_t, status_code);
struct_func_dcl(sm_disconnect_ind, uint16_t, reason_code);
struct_func_dcl(sm_disconnect_ind, uint8_t, vif_idx);

struct_func_dcl(me_tkip_mic_failure_ind, uint8_t, vif_idx);
void* me_tkip_mic_failure_ind_getp_addr(void *param);
struct_func_dcl(me_tkip_mic_failure_ind, bool, ga);

struct_func_dcl(me_tx_credits_update_ind, uint8_t, sta_idx);
struct_func_dcl(me_tx_credits_update_ind, uint8_t, tid);
struct_func_dcl(me_tx_credits_update_ind, int8_t, credits);

struct_func_dcl(sm_external_auth_required_ind, uint8_t, vif_idx);
struct_func_dcl(sm_external_auth_required_ind, struct mac_addr, bssid);
struct_func_dcl(sm_external_auth_required_ind, struct mac_ssid, ssid);
struct_func_dcl(sm_external_auth_required_ind, uint32_t, akm);

struct_func_dcl(mm_ps_change_ind, uint8_t, sta_idx);
struct_func_dcl(mm_ps_change_ind, uint8_t, ps_state);

struct_func_dcl(mm_traffic_req_ind, uint8_t, sta_idx);
struct_func_dcl(mm_traffic_req_ind, uint8_t, pkt_cnt);
struct_func_dcl(mm_traffic_req_ind, bool, uapsd);

struct_func_dcl(apm_probe_client_ind, uint8_t, vif_idx);
struct_func_dcl(apm_probe_client_ind, uint8_t, sta_idx);
struct_func_dcl(apm_probe_client_ind, bool, client_present);

struct_func_dcl(mm_remain_on_channel_exp_ind, uint8_t, vif_index);
struct_func_dcl(mm_remain_on_channel_exp_ind, uint16_t, freq);

struct_func_dcl(mm_p2p_vif_ps_change_ind, uint8_t, vif_index);
struct_func_dcl(mm_p2p_vif_ps_change_ind, uint8_t, ps_state);

struct_func_dcl(mesh_path_update_ind, uint8_t, vif_idx);
struct_func_dcl(mesh_path_update_ind, uint8_t, path_idx);
struct_func_dcl(mesh_path_update_ind, bool, delete);
struct_func_dcl(mesh_path_update_ind, uint8_t, nhop_sta_idx);

struct_func_dcl(ftm_done_ind, uint8_t, vif_idx);
struct_func_dcl(ftm_done_ind, struct mac_ftm_results, results);

struct_func_dcl(apm_start_req, struct mac_rateset, basic_rates);
struct_func_dcl(apm_start_req, struct mac_chan_op, chan);
void* apm_start_req_getp_chan(void *param);
struct_func_dcl(apm_start_req, uint32_t, bcn_addr);
struct_func_dcl(apm_start_req, uint16_t, bcn_len);
struct_func_dcl(apm_start_req, uint16_t, bcn_int);
struct_func_dcl(apm_start_req, uint16_t, tim_oft);
struct_func_dcl(apm_start_req, uint8_t, tim_len);
struct_func_dcl(apm_start_req, uint32_t, flags);
struct_func_dcl(apm_start_req, uint16_t, ctrl_port_ethertype);
struct_func_dcl(apm_start_req, uint8_t, vif_idx);
int apm_start_req_len();

struct_func_dcl(apm_start_cfm, uint8_t, bcmc_idx);
int apm_start_cfm_len();
struct_func_dcl(apm_stop_req, uint8_t, vif_idx);
int apm_stop_req_len();

struct_func_dcl(apm_probe_client_req, uint8_t, vif_idx);
struct_func_dcl(apm_probe_client_req, uint8_t, sta_idx);
int apm_probe_client_req_len();
struct_func_dcl(apm_probe_client_cfm, uint8_t, status);
int apm_probe_client_cfm_len();

struct_func_dcl(ftm_start_req, uint8_t, vif_idx);
struct_func_dcl(ftm_start_req, uint8_t, ftm_per_burst);
struct_func_dcl(ftm_start_req, uint8_t, nb_ftm_rsp);
int ftm_start_req_len();
struct_func_dcl(ftm_start_cfm, uint8_t, status);
int ftm_start_cfm_len();

void *scanu_start_req_chan(void *req, int nb);
struct_func_dcl(scanu_start_req, uint8_t, chan_cnt);
void scanu_start_req_set_chan_flags(void *req, int nb, enum mac_chan_flags flag);
void *scanu_start_req_ssid_array(void *req, int nb);
void scanu_start_req_set_ssid_length(void *req, int nb, uint8_t len);
struct_func_dcl(scanu_start_req, uint8_t, ssid_cnt);
void* scanu_start_req_getp_bssid(void *param);
struct_func_dcl(scanu_start_req, uint32_t, add_ies);
struct_func_dcl(scanu_start_req, uint16_t, add_ie_len);
struct_func_dcl(scanu_start_req, uint8_t, vif_idx);
struct_func_dcl(scanu_start_req, bool, no_cck);
struct_func_dcl(scanu_start_req, uint32_t, duration);
struct_func_dcl(scanu_start_req, bool, scan_only);
struct_func_dcl(scanu_start_req, uint32_t, flags);
int scanu_start_req_len();

struct_func_dcl(scanu_get_scan_result_req, uint8_t, idx);
int scanu_get_scan_result_req_len();

struct_func_dcl(sm_connect_req, struct mac_ssid, ssid);
void* sm_connect_req_getp_bssid(void *param);
struct_func_dcl(sm_connect_req, struct mac_chan_def, chan);
struct_func_dcl(sm_connect_req, uint32_t, flags);
struct_func_dcl(sm_connect_req, uint16_t, ctrl_port_ethertype);
struct_func_dcl(sm_connect_req, uint16_t, ie_len);
struct_func_dcl(sm_connect_req, uint16_t, listen_interval);
struct_func_dcl(sm_connect_req, bool, dont_wait_bcmc);
struct_func_dcl(sm_connect_req, uint8_t, auth_type);
struct_func_dcl(sm_connect_req, uint8_t, uapsd_queues);
struct_func_dcl(sm_connect_req, uint8_t, vif_idx);
struct_func_dcl(sm_connect_req, uint8_t, wpa);
int sm_connect_req_len();

struct_func_dcl(sm_connect_cfm, uint8_t, status);
int sm_connect_cfm_len();

struct_func_dcl(sm_disconnect_req, uint8_t, vif_idx);
struct_func_dcl(sm_disconnect_req, uint16_t, reason_code);
struct_func_dcl(sm_disconnect_req, uint16_t, status_code);
int sm_disconnect_req_len();

struct_func_dcl(sm_external_auth_required_rsp, uint8_t, vif_idx);
struct_func_dcl(sm_external_auth_required_rsp, uint16_t, status);
int sm_external_auth_required_rsp_len();

struct_func_dcl(scan_raw_send_req, void *, pkt);
struct_func_dcl(scan_raw_send_req, uint32_t, len);
struct_func_dcl(scan_raw_send_req, uint8_t, index);
struct_func_dcl(scan_raw_send_req, int, vif_idx);
struct_func_dcl(scan_raw_send_req, struct mac_chan_op, chan);
int scan_raw_send_req_len();

struct_func_dcl(mm_add_if_req, uint8_t, type);
struct_func_dcl(mm_add_if_req, struct mac_addr, addr);
struct_func_dcl(mm_add_if_req, bool, p2p);
int mm_add_if_req_len();

struct_func_dcl(mm_add_if_cfm, uint8_t, status);
struct_func_dcl(mm_add_if_cfm, uint8_t, inst_nbr);
int mm_add_if_cfm_len();

struct_func_dcl(mm_remove_if_req, uint8_t, inst_nbr);
struct_func_len(mm_remove_if_req);

struct_func_dcl(mm_key_add_req, uint8_t, key_idx);
struct_func_dcl(mm_key_add_req, uint8_t, sta_idx);
struct_func_dcl(mm_key_add_req, struct mac_sec_key, key);
struct_func_dcl(mm_key_add_req, uint8_t, cipher_suite);
struct_func_dcl(mm_key_add_req, uint8_t, inst_nbr);
struct_func_dcl(mm_key_add_req, uint8_t, spp);
struct_func_dcl(mm_key_add_req, bool, pairwise);
struct_func_len(mm_key_add_req);
struct_func_dcl(mm_key_add_cfm, uint8_t, status);
struct_func_len(mm_key_add_cfm);

struct_func_dcl(mm_key_del_req, uint8_t, hw_key_idx);
struct_func_len(mm_key_del_req);

struct_func_dcl(me_set_control_port_req, uint8_t, sta_idx);
struct_func_dcl(me_set_control_port_req, bool, control_port_open);
struct_func_len(me_set_control_port_req);
struct_func_dcl(me_set_control_port_cfm, bool, control_port_open);
struct_func_len(me_set_control_port_cfm);

struct_func_dcl(mm_version_cfm, uint32_t, version_lmac);
struct_func_dcl(mm_version_cfm, uint32_t, version_machw_1);
struct_func_dcl(mm_version_cfm, uint32_t, version_machw_2);
struct_func_dcl(mm_version_cfm, uint32_t, version_phy_1);
struct_func_dcl(mm_version_cfm, uint32_t, version_phy_2);
struct_func_dcl(mm_version_cfm, uint32_t, features);
struct_func_dcl(mm_version_cfm, uint16_t, max_sta_nb);
struct_func_dcl(mm_version_cfm, uint8_t, max_vif_nb);
struct_func_len(mm_version_cfm);

struct_func_dcl(me_config_monitor_req, struct mac_chan_op, chan);
struct_func_dcl(me_config_monitor_req, bool, chan_set);
struct_func_dcl(me_config_monitor_req, bool, uf);
struct_func_len(me_config_monitor_req);
struct_func_len(me_config_monitor_cfm);

struct_func_dcl(mm_set_edca_req, uint32_t, ac_param);
struct_func_dcl(mm_set_edca_req, bool, uapsd);
struct_func_dcl(mm_set_edca_req, uint8_t, hw_queue);
struct_func_dcl(mm_set_edca_req, uint8_t, inst_nbr);
struct_func_len(mm_set_edca_req);

struct_func_dcl(mm_bcn_change_req, uint32_t, bcn_ptr);
struct_func_dcl(mm_bcn_change_req, uint16_t, bcn_len);
struct_func_dcl(mm_bcn_change_req, uint16_t, tim_oft);
struct_func_dcl(mm_bcn_change_req, uint8_t, tim_len);
struct_func_dcl(mm_bcn_change_req, uint8_t, inst_nbr);
void mm_bcn_change_req_set_csa_oft(void *pa, uint8_t val, int i);;
struct_func_len(mm_bcn_change_req);;

struct_func_dcl(me_sta_add_req, struct mac_addr, mac_addr);
struct_func_dcl(me_sta_add_req, struct mac_rateset, rate_set);
struct_func_dcl(me_sta_add_req, struct mac_htcapability, ht_cap);
struct_func_dcl(me_sta_add_req, struct mac_vhtcapability, vht_cap);
struct_func_dcl(me_sta_add_req, struct mac_hecapability, he_cap);
struct_func_dcl(me_sta_add_req, uint32_t, flags);
struct_func_dcl(me_sta_add_req, uint16_t, aid);
struct_func_dcl(me_sta_add_req, uint8_t, uapsd_queues);
struct_func_dcl(me_sta_add_req, uint8_t, max_sp_len);
struct_func_dcl(me_sta_add_req, uint8_t, opmode);
struct_func_dcl(me_sta_add_req, uint8_t, vif_idx);
struct_func_dcl(me_sta_add_req, bool, tdls_sta);
struct_func_dcl(me_sta_add_req, bool, tdls_initiator);
struct_func_dcl(me_sta_add_req, bool, tdls_chsw_allowed);
struct_func_len(me_sta_add_req);
struct_func_dcl(me_sta_add_cfm, uint8_t, sta_idx);
struct_func_len(me_sta_add_cfm);

struct_func_dcl(me_sta_del_req, uint8_t, sta_idx);
struct_func_dcl(me_sta_del_req, bool, tdls_sta);
struct_func_len(me_sta_del_req);

struct_func_dcl(me_rc_set_rate_req, uint16_t, fixed_rate_cfg);
struct_func_dcl(me_rc_set_rate_req, uint8_t, sta_idx);
struct_func_len(me_rc_set_rate_req);

struct_func_dcl(me_set_ps_mode_req, uint8_t, ps_mode);
struct_func_dcl(me_set_ps_mode_req, uint8_t, ps_state);
struct_func_len(me_set_ps_mode_req);

struct_func_dcl(mm_remain_on_channel_req, uint8_t, op_code);
struct_func_dcl(mm_remain_on_channel_req, uint8_t, vif_index);
struct_func_dcl(mm_remain_on_channel_req, struct mac_chan_op, chan);
struct_func_dcl(mm_remain_on_channel_req, uint32_t, duration_ms);
struct_func_len(mm_remain_on_channel_req);
struct_func_dcl(mm_remain_on_channel_cfm, uint8_t, status);
struct_func_len(mm_remain_on_channel_cfm);

struct_func_dcl(mm_set_p2p_noa_req, uint8_t, vif_index);
struct_func_dcl(mm_set_p2p_noa_req, uint8_t, noa_inst_nb);
struct_func_dcl(mm_set_p2p_noa_req, uint8_t, count);
struct_func_dcl(mm_set_p2p_noa_req, bool, dyn_noa);
struct_func_dcl(mm_set_p2p_noa_req, uint32_t, duration_us);
struct_func_dcl(mm_set_p2p_noa_req, uint32_t, interval_us);
struct_func_dcl(mm_set_p2p_noa_req, uint32_t, start_offset);
struct_func_len(mm_set_p2p_noa_req);
struct_func_dcl(mm_set_p2p_noa_cfm, uint8_t, status);
struct_func_len(mm_set_p2p_noa_cfm);

struct_func_dcl(mesh_start_req, struct mac_rateset, basic_rates);
struct_func_dcl(mesh_start_req, struct mac_chan_op, chan);
struct_func_dcl(mesh_start_req, uint8_t, dtim_period);
struct_func_dcl(mesh_start_req, uint8_t, bcn_int);
struct_func_dcl(mesh_start_req, uint8_t, vif_idx);
struct_func_dcl(mesh_start_req, uint8_t, mesh_id_len);
void mesh_start_req_set_mesh_id(void *pa, uint8_t val, int i);;

struct_func_dcl(mesh_start_req, uint32_t, ie_addr);
struct_func_dcl(mesh_start_req, uint8_t, ie_len);
struct_func_dcl(mesh_start_req, bool, user_mpm);
struct_func_dcl(mesh_start_req, bool, is_auth);
struct_func_dcl(mesh_start_req, uint8_t, auth_id);
struct_func_len(mesh_start_req);
struct_func_dcl(mesh_start_cfm, uint8_t, status);
struct_func_dcl(mesh_start_cfm, uint8_t, bcmc_idx);
struct_func_len(mesh_start_cfm);

struct_func_dcl(mesh_stop_req, uint8_t, vif_idx);
struct_func_len(mesh_stop_req);
struct_func_dcl(mesh_stop_cfm, uint8_t, status);
struct_func_len(mesh_stop_cfm);

struct_func_dcl(mesh_peer_update_ntf, uint8_t, vif_idx);
struct_func_dcl(mesh_peer_update_ntf, uint8_t, sta_idx);
struct_func_dcl(mesh_peer_update_ntf, uint8_t, state);
struct_func_len(mesh_peer_update_ntf);

struct_func_dcl(me_config_req, struct mac_htcapability, ht_cap);
struct_func_dcl(me_config_req, struct mac_vhtcapability, vht_cap);
struct_func_dcl(me_config_req, struct mac_hecapability, he_cap);
struct_func_dcl(me_config_req, uint16_t, tx_lft);
struct_func_dcl(me_config_req, uint8_t, phy_bw_max);
struct_func_dcl(me_config_req, bool, ht_supp);
struct_func_dcl(me_config_req, bool, vht_supp);
struct_func_dcl(me_config_req, bool, he_supp);
struct_func_dcl(me_config_req, bool, he_ul_on);
struct_func_dcl(me_config_req, bool, ps_on);
struct_func_dcl(me_config_req, bool, ant_div_on);
struct_func_dcl(me_config_req, bool, dpsm);
void me_config_req_set_amsdu_tx(void *pa, short val);
struct_func_len(me_config_req);

struct_func_dcl(mm_start_req, struct phy_cfg_tag, phy_cfg);
struct_func_dcl(mm_start_req, uint32_t, uapsd_timeout);
struct_func_dcl(mm_start_req, uint16_t, lp_clk_accuracy);
void mm_start_req_set_tx_timeout(void *pa, uint16_t array[], int len);
struct_func_len(mm_start_req);

struct_func_dcl(me_traffic_ind_req, uint8_t, sta_idx);
struct_func_dcl(me_traffic_ind_req, uint8_t, tx_avail);
struct_func_dcl(me_traffic_ind_req, bool, uapsd);
struct_func_len(me_traffic_ind_req);

struct_func_dcl(dbg_get_sys_stat_cfm, uint32_t, cpu_sleep_time);
struct_func_dcl(dbg_get_sys_stat_cfm, uint32_t, doze_time);
struct_func_dcl(dbg_get_sys_stat_cfm, uint32_t, stats_time);
struct_func_len(dbg_get_sys_stat_cfm);
