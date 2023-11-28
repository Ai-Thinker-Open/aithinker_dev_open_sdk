    struct_func_alloc_def(apm_start_req);
    struct_func_alloc_def(apm_start_cfm);
    struct_func_alloc_def(apm_stop_req);
    struct_func_alloc_def(apm_probe_client_req);
    struct_func_alloc_def(apm_probe_client_cfm);
#if NX_FTM_INITIATOR
    struct_func_alloc_def(ftm_start_req);
    struct_func_alloc_def(ftm_start_cfm);
#endif
    struct_func_alloc_def(scanu_start_req);
    struct_func_alloc_def(scanu_get_scan_result_req);
    struct_func_alloc_def(sm_connect_req);
    struct_func_alloc_def(sm_connect_cfm);

    struct_func_alloc_def(sm_disconnect_req);
    struct_func_alloc_def(sm_external_auth_required_rsp);
    struct_func_alloc_def(scan_raw_send_req);

    struct_func_alloc_def(mm_add_if_req);
    struct_func_alloc_def(mm_add_if_cfm);
    struct_func_alloc_def(mm_remove_if_req);
    struct_func_alloc_def(mm_key_add_req);
    struct_func_alloc_def(mm_key_add_cfm);
    struct_func_alloc_def(mm_key_del_req);
    struct_func_alloc_def(me_set_control_port_req);
    struct_func_alloc_def(me_set_control_port_cfm);
    struct_func_alloc_def(mm_version_cfm);
    struct_func_alloc_def(me_config_monitor_req);
    struct_func_alloc_def(me_config_monitor_cfm);
    struct_func_alloc_def(mm_set_edca_req);
    struct_func_alloc_def(mm_bcn_change_req);
    struct_func_alloc_def(me_sta_add_req);
    struct_func_alloc_def(me_sta_add_cfm);
    struct_func_alloc_def(me_sta_del_req);
    struct_func_alloc_def(me_rc_set_rate_req);
#if NX_POWERSAVE
    struct_func_alloc_def(me_set_ps_mode_req);
#endif
    struct_func_alloc_def(mm_remain_on_channel_req);
    struct_func_alloc_def(mm_remain_on_channel_cfm);
    struct_func_alloc_def(mm_set_p2p_noa_req);
    struct_func_alloc_def(mm_set_p2p_noa_cfm);
#if RW_MESH_EN
    struct_func_alloc_def(mesh_start_req);
    struct_func_alloc_def(mesh_start_cfm);
    struct_func_alloc_def(mesh_stop_req);
    struct_func_alloc_def(mesh_stop_cfm);
    struct_func_alloc_def(mesh_peer_update_ntf);
#endif
    struct_func_alloc_def(me_config_req);
    struct_func_alloc_def(mm_start_req);

    struct_func_alloc_def(me_traffic_ind_req);
    struct_func_alloc_def(dbg_get_sys_stat_cfm);
