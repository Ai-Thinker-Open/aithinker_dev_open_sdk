 /**
 ****************************************************************************************
 *
 * @file fhost_cntrl.c
 *
 * @brief Definition of control task for Fully Hosted firmware.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

#include "fhost.h"
#include "fhost_cntrl.h"
#include "fhost_config.h"
#include "fhost_tx.h"
#include "fhost_rx.h"
#include "cfgrwnx.h"
#include "rtos.h"
#include "ke_msg.h"
#include "sys/socket.h"
#include "mac_ie.h"
#if RW_MESH_EN
#include "mesh_task.h"
#endif
#include "mac_types.h"

#include <bl_fw_api.h>

#include "ieee80211.h"
#include "wpa.h"

#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "platform_al.h"
#include "net_al_ext.h"

#ifdef NO_MSG_PEEK
#include "rtos_al.h"
rtos_mutex cntrl_no_msg_peek_send_lock;
#endif

/**
 ****************************************************************************************
 * @addtogroup FHOST_CNTRL
 * @{
 ****************************************************************************************
 */

/// Master message queue
static rtos_queue queue;
/// KE message queue
static rtos_queue queue_ke_msg;
/// Fhost cntrl task handle
rtos_task_handle * const fhost_cntrl_task_handle;

/// KE message queue size
#define FHOST_CNTRL_QUEUE_KE_MSG_SIZE 5

/// Master message queue size
#define FHOST_CNTRL_QUEUE_SIZE (FHOST_CNTRL_QUEUE_KE_MSG_SIZE + 5)

/// CFGRWNX response queue size
#define FHOST_CNTRL_QUEUE_CFGRWNX_RESP_SIZE 5

/// Number of UDP sockets for CFGRWNX connections
#define FHOST_CNTRL_MAX_LINK 2

#define COMMON_PARAM(name, default_val)    \
    .name = default_val,

struct fhost_mod_params fhost_params = {
    COMMON_PARAM(listen_itv, 0)
};

uint8_t g_sta_idx = 0;

static void fhost_cntrl_cfgrwnx_resp_send(void const *msg, int msg_len,
                                          rtos_queue resp_queue);

static const struct reason_code sm_reason_list[] = {
    {WLAN_FW_SUCCESSFUL, "sm connect ind ok"},
    {WLAN_FW_TX_AUTH_FRAME_ALLOCATE_FAIILURE, "tx auth frame alloc failure"},
    {WLAN_FW_AUTHENTICATION_FAIILURE, "Authentication failure"},
    {WLAN_FW_AUTH_ALGO_FAIILURE, "Auth response but auth algo failure"},
    {WLAN_FW_TX_ASSOC_FRAME_ALLOCATE_FAIILURE, "tx assoc frame alloc failure"},
    {WLAN_FW_ASSOCIATE_FAIILURE, "Association failure"},
    {WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION, "deauth by AP when connecting"},
    {WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION, "deauth by AP when connected"},
    {WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE, "Passwd error, 4-way handshake timeout"},
    {WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE, "Passwd error, tx deauth frame transmit failure"},
    {WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE, "Passwd error, tx deauth frame allocate failure"},
    {WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE, "auth or associate frame response timeout failure"},
    {WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL, "SSID error, scan no bssid and channel"},
    {WLAN_FW_CREATE_CHANNEL_CTX_FAILURE_WHEN_JOIN_NETWORK, "create channel context failure when join network"},
    {WLAN_FW_JOIN_NETWORK_FAILURE, "join network failure"},
    {WLAN_FW_ADD_STA_FAILURE, "add sta failure"},
    {WLAN_FW_BEACON_LOSS, "ap beacon loss"},
    {WLAN_FW_NETWORK_SECURITY_NOMATCH, "network security no match"},
    {WLAN_FW_NETWORK_WEPLEN_ERROR, "wep network psk len error"},
    {WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH, "user disconnect and send deauth"},
    {WLAN_FW_DISCONNECT_BY_USER_NO_DEAUTH, "user disconnect but no send deauth"},
    {WLAN_FW_DISCONNECT_BY_FW_PS_TX_NULLFRAME_FAILURE, "fw disconnect(tx nullframe failures)"},
    {WLAN_FW_TRAFFIC_LOSS, "fw disconnect(traffic loss)"},
    {WLAN_FW_SWITCH_CHANNEL_FAILURE, "fw switch channel failure"},
    {WLAN_FW_AUTH_OR_ASSOC_RESPONSE_CFM_FAILURE, "auth or associate frame response cfm failure"},
};

static const char* _get_status_code_str(const struct reason_code list[], uint16_t len, uint16_t status_code)
{
    int i;

    for (i = 0; i < len; i++) {
        if (list[i].reason_code == status_code) {
            break;
        }
    }
    if (len == i) {
        return "Unknown Code";
    }

    return list[i].action;
}

const char* wifi_mgmr_get_sm_status_code_str(uint16_t status_code)
{
    return _get_status_code_str(sm_reason_list, sizeof(sm_reason_list)/sizeof(sm_reason_list[0]), status_code);
}

static wifi_cipher_type_t cipher_type_map_supp_to_public(unsigned wpa_cipher)
{
    switch (wpa_cipher) {
    case WPA_CIPHER_NONE:
        return WIFI_CIPHER_TYPE_NONE;

    case WPA_CIPHER_WEP40:
        return WIFI_CIPHER_TYPE_WEP40;

    case WPA_CIPHER_WEP104:
        return WIFI_CIPHER_TYPE_WEP104;

    case WPA_CIPHER_TKIP:
        return WIFI_CIPHER_TYPE_TKIP;

    case WPA_CIPHER_CCMP:
        return WIFI_CIPHER_TYPE_CCMP;

    case WPA_CIPHER_CCMP|WPA_CIPHER_TKIP:
        return WIFI_CIPHER_TYPE_TKIP_CCMP;

    case WPA_CIPHER_AES_128_CMAC:
        return WIFI_CIPHER_TYPE_AES_CMAC128;

    case WPA_CIPHER_SMS4:
        return WIFI_CIPHER_TYPE_SMS4;

    default:
        return WIFI_CIPHER_TYPE_UNKNOWN;
    }
}

static int wpa_parse_wpa_ie_wrapper(const uint8_t *wpa_ie, size_t wpa_ie_len, wifi_wpa_ie_t *data)
{
    struct wpa_ie_data ie;
    int ret = 0;

    ret = wpa_parse_wpa_ie(wpa_ie, wpa_ie_len, &ie);
    data->proto = ie.proto;
    data->pairwise_cipher = cipher_type_map_supp_to_public(ie.pairwise_cipher);
    data->group_cipher = cipher_type_map_supp_to_public(ie.group_cipher);
    data->key_mgmt = ie.key_mgmt;
    data->capabilities = ie.capabilities;
    data->pmkid = ie.pmkid;
    data->mgmt_group_cipher = cipher_type_map_supp_to_public(ie.mgmt_group_cipher);

    return ret;
}

static int find_ie_ssid(uint8_t *buffer, int len, uint8_t *result, int *ssid_len)
{
    int i;

    i = 0;
    while (i < len) {
#define IE_ID_SSID 0x00
        if (IE_ID_SSID == buffer[0]) {//FIXME no magic ID for SSID
            if (buffer[1] > 32) {
                return -1;
            }
            *ssid_len = buffer[1];
            memcpy(result, &(buffer[2]), buffer[1]);//NO check for len valid
            result[buffer[1]] = '\0';
            return 0;
        } else {
            /*move to next ie*/
            i += buffer[1] + 2;//FIXME 2 is for id and len
            buffer = buffer + buffer[1] + 2;
        }
    }

    return -1;
}

static int find_ie_ds(uint8_t *buffer, int len, uint8_t *result) //bl_rx.c
{
    int i;

    i = 0;
    while (i < len) {
#define IE_ID_DS_CHANNEL 0x03
        if (IE_ID_DS_CHANNEL == buffer[0]) {//FIXME no magic DS for SSID
            if (buffer[1] > 32) {
                return -1;
            }
            *result = buffer[2];
            return 0;
        } else {
            /*move to next ie*/
            i += buffer[1] + 2;//FIXME 2 is for id and len
            buffer = buffer + buffer[1] + 2;
        }
    }

    return -1;
}

static void _rx_handle_beacon(void *ind, struct ieee80211_mgmt *mgmt)
{
    wifi_mgmr_scan_item_t scan_item;
    uint32_t var_part_addr, var_part_len;
    uint32_t elmt_addr;
    uint16_t rsn_wpa_ie_len;
    wifi_wpa_ie_t wpa_ie, rsn_ie;
    uint16_t ind_length = rxu_mgt_ind_get_length(ind);

    memset(&scan_item, 0, sizeof(scan_item));

    var_part_addr = (uint32_t)(mgmt->u.beacon.variable);
    var_part_len = ind_length - (uint32_t)&(((struct ieee80211_mgmt*)NULL)->u.beacon.variable);

    find_ie_ssid(mgmt->u.beacon.variable, ind_length, (uint8_t *)scan_item.ssid, &scan_item.ssid_len);
    find_ie_ds(mgmt->u.beacon.variable, ind_length, &scan_item.channel);

    uint16_t len;
    elmt_addr = mac_vsie_find(var_part_addr, var_part_len, (const uint8_t *) "\x00\x50\xF2\x04", 4, &len);
    if (elmt_addr != 0)
    {
        /*wps is suoported*/
        scan_item.wps = 1;
    } else {
        /*wps isn't supported*/
        scan_item.wps = 0;
    }

    /* TODO: Only consider 2.4G and bgn mode
     * (NO 5G and a/ac/ax) / (NO g-only and n-only difference)
     */
    #define MAC_ELTID_HT_CAPA                45
    #define MAC_ELTID_EXT_RATES              50
    if (mac_ie_find(var_part_addr, var_part_len, MAC_ELTID_HT_CAPA, &len))
    {
        scan_item.mode = (WIFI_MODE_802_11B | WIFI_MODE_802_11G | WIFI_MODE_802_11N_2_4);
    }
    else if (mac_ie_find(var_part_addr, var_part_len, MAC_ELTID_EXT_RATES, &len))
    {
        scan_item.mode = (WIFI_MODE_802_11B | WIFI_MODE_802_11G);
    }
    else
    {
        scan_item.mode = (WIFI_MODE_802_11B);
    }


    #define le16_to_cpu(v16) (v16)
    if (WLAN_CAPABILITY_PRIVACY & (le16_to_cpu(mgmt->u.beacon.capab_info))) {
        wifi_wpa_ie_t *parsed_wpa_ie[2] = {};
        int parsed_wpa_ie_len = 0;
        int i;
        bool tkip = false, ccmp = false;

        #define MAC_ELTID_RSN_IEEE               48
        elmt_addr = mac_ie_find(var_part_addr, var_part_len, MAC_ELTID_RSN_IEEE, &rsn_wpa_ie_len);
        if (elmt_addr != 0) {
            memset(&rsn_ie, 0, sizeof(rsn_ie));
            wpa_parse_wpa_ie_wrapper((uint8_t *)elmt_addr, rsn_wpa_ie_len, &rsn_ie);
            parsed_wpa_ie[parsed_wpa_ie_len++] = &rsn_ie;
        }

        elmt_addr = mac_vsie_find(var_part_addr, var_part_len, (uint8_t const *)"\x00\x50\xF2\x01", 4, &rsn_wpa_ie_len);
        if (elmt_addr != 0) {
            //rsn_wpa_ie_len = co_read8p(elmt_addr + MAC_INFOELT_LEN_OFT) + MAC_INFOELT_INFO_OFT;
            wpa_parse_wpa_ie_wrapper((uint8_t *)elmt_addr, rsn_wpa_ie_len, &wpa_ie);
            parsed_wpa_ie[parsed_wpa_ie_len++] = &wpa_ie;
        }

        for (i = 0; i < parsed_wpa_ie_len; ++i) {
            wifi_wpa_ie_t *ie = parsed_wpa_ie[i];
            int pc = ie->pairwise_cipher;
            int gc = ie->group_cipher;
            int ciphers[2] = { pc, gc };
            int j;

            if (ie->proto == WPA_PROTO_WPA) {
                scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WPA_PSK;
            } else if (ie->proto == WPA_PROTO_RSN) {
                if (ie->key_mgmt & (WPA_KEY_MGMT_PSK | WPA_KEY_MGMT_PSK_SHA256)) {
                    scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK;
                    if (ie->key_mgmt & WPA_KEY_MGMT_SAE) {
                        scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK_WPA3_SAE;
                    }
                } else if (ie->key_mgmt & WPA_KEY_MGMT_SAE) {
                    scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WPA3_SAE;
                }
            }

            for (j = 0; j < sizeof(ciphers) / sizeof(ciphers[0]); ++j) {
                int cipher = ciphers[j];
                if (cipher == WIFI_CIPHER_TYPE_TKIP) {
                    tkip = true;
                }
                if (cipher == WIFI_CIPHER_TYPE_CCMP) {
                    ccmp = true;
                }
                if (cipher == WIFI_CIPHER_TYPE_TKIP_CCMP) {
                    tkip = ccmp = true;
                }
            }
        }
        if (parsed_wpa_ie_len == sizeof(parsed_wpa_ie) / sizeof(parsed_wpa_ie[0])) {
            scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_PSK;
        } else if (parsed_wpa_ie_len == 0) {
            scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_WEP;
            scan_item.cipher = WIFI_EVENT_BEACON_IND_CIPHER_WEP;
        }

        if (ccmp) {
            scan_item.cipher = WIFI_EVENT_BEACON_IND_CIPHER_AES;
        }
        if (tkip) {
            scan_item.cipher = WIFI_EVENT_BEACON_IND_CIPHER_TKIP;
        }
        if (tkip && ccmp) {
            scan_item.cipher = WIFI_EVENT_BEACON_IND_CIPHER_TKIP_AES;
        }
    } else {
        /*This is an open BSS*/
        scan_item.auth = WIFI_EVENT_BEACON_IND_AUTH_OPEN;
    }

    scan_item.rssi = rxu_mgt_ind_get_rssi(ind);
    //ind_new.ppm_abs = ind_ppm_abs;
    //ind_new.ppm_rel = ind_ppm_rel;
    scan_item.bssid[0]  = mgmt->bssid[0];
    scan_item.bssid[1]  = mgmt->bssid[1];
    scan_item.bssid[2]  = mgmt->bssid[2];
    scan_item.bssid[3]  = mgmt->bssid[3];
    scan_item.bssid[4]  = mgmt->bssid[4];
    scan_item.bssid[5]  = mgmt->bssid[5];

    wifi_mgmr_scan_beacon_save(&scan_item);
}

static void _rx_handle_probersp(void *ind, struct ieee80211_mgmt *mgmt)
{
    _rx_handle_beacon(ind, mgmt);
}

/****************************************************************************************
 * WIFI KE MSG process
 ***************************************************************************************/
/**
 ****************************************************************************************
 * @brief Process SCANU_RESULT_IND message
 *
 * Sent by firmware when a new bssid is found.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_scan_result_ind(ke_msg_id_t const msgid,
                                              void *param,
                                              ke_task_id_t const dest_id,
                                              ke_task_id_t const src_id)
{
    struct cfgrwnx_scan_result result;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(rxu_mgt_ind_get_inst_nbr(param));

    struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)rxu_mgt_ind_get_payload(param);

    if(!fhost_vif->scanning)
        return 0;

    if (ieee80211_is_beacon(mgmt->frame_control)) {
        _rx_handle_beacon(param, mgmt);
    } else if (ieee80211_is_probe_resp(mgmt->frame_control)) {
        _rx_handle_probersp(param, mgmt);
    } else {
        printf("Bug Scan IND?\r\n");
    }

    // printf("    --> fhost cntrl rssi %d\r\n", param->rssi);
    result.hdr.id = CFGRWNX_SCAN_RESULT_EVENT;
    result.hdr.len = sizeof(result);

    result.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    result.freq = rxu_mgt_ind_get_center_freq(param);
    result.rssi = rxu_mgt_ind_get_rssi(param);
    result.length = rxu_mgt_ind_get_length(param);
    result.payload = rtos_malloc(rxu_mgt_ind_get_length(param));
    if (result.payload == NULL)
        return 0;
    memcpy(result.payload, rxu_mgt_ind_get_payload(param), rxu_mgt_ind_get_length(param));

    if (!fhost_vif->is_cntrl_link) {
        if (fhost_cntrl_cfgrwnx_event_send(&result.hdr, fhost_vif->scan_sock))
            rtos_free(result.payload);
    } else {
        rtos_free(result.payload);
    }


    return 0;
}

/**
 ****************************************************************************************
 * @brief Process SCANU_START_CFM message
 *
 * Sent by firmware when the scan is done.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_scan_cfm(ke_msg_id_t const msgid,
                                       void *param,
                                       ke_task_id_t const dest_id,
                                       ke_task_id_t const src_id)
{
    struct cfgrwnx_scan_completed resp;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(scanu_start_cfm_get_vif_idx(param));

    resp.hdr.id = CFGRWNX_SCAN_DONE_EVENT;
    resp.hdr.len = sizeof(resp);
    resp.result_cnt = scanu_start_cfm_get_result_cnt(param);
    resp.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);

    if (scanu_start_cfm_get_status(param) == CO_OK)
        resp.status = CFGRWNX_SUCCESS;
    else
        resp.status = CFGRWNX_ERROR;

    if (!fhost_vif->is_cntrl_link) {
        fhost_cntrl_cfgrwnx_event_send(&resp.hdr, fhost_vif->scan_sock);
    }
    fhost_vif->scan_sock = -1;
    fhost_vif->scanning = false;

    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_SCAN, 0);

    if (fhost_vif->is_cntrl_link) {
        platform_post_event(EV_WIFI, CODE_WIFI_ON_SCAN_DONE);
    }

    return 0;
}

#if NX_FHOST_RX_STATS
/**
 ****************************************************************************************
 * @brief Allocates an entry in the RX stats table
 *
 * @param[in] sta_idx   Station index and index of the RX stats table to allocate
 ****************************************************************************************
 */
static void fhost_cntrl_alloc_rx_rates(int sta_idx)
{
    if (!rx_stats[sta_idx])
    {
        rx_stats[sta_idx] = rtos_calloc(sizeof(struct fhost_rx_stats_sta) +
                            fhost_conf_rx.n_rates * sizeof(uint32_t), sizeof(uint8_t));
        if (!rx_stats[sta_idx])
            return;
    }
}

/**
 ****************************************************************************************
 * @brief Free entry in the RX stats table
 *
 * @param[in] sta_idx   Station index and index of the RX stats table to free
 ****************************************************************************************
 */
static void fhost_cntrl_free_rx_rates(int sta_idx)
{
    if (rx_stats[sta_idx])
    {
        rtos_free(rx_stats[sta_idx]);
        rx_stats[sta_idx] = NULL;
    }
}
#endif

static void fhost_cntrl_connect_ind_dump(uint16_t status_code, uint16_t ieeetypes_code)
{
    wifi_mgmr_connect_ind_stat_info_t *connection_info = &wifiMgmr.wifi_mgmr_stat_info;

    wifi_mgmr_sta_info_upatestatus(status_code, ieeetypes_code);
    printf("=================================================================\r\n");
    printf("[AT][RX] Connection Status\r\n");
    printf("[AT][RX]   status_code %u\r\n", status_code);
    printf("[AT][RX]   reason_code %u\r\n", ieeetypes_code);
    printf("[AT][RX]   status detail: %s\r\n", wifi_mgmr_get_sm_status_code_str(status_code));
    printf("[AT][RX]   MAC %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                                connection_info->bssid[0],
                                connection_info->bssid[1],
                                connection_info->bssid[2],
                                connection_info->bssid[3],
                                connection_info->bssid[4],
                                connection_info->bssid[5]
    );
    printf("[AT][RX]   vif_idx %u\r\n", connection_info->vif_idx);
    printf("[AT][RX]   ap_idx %u\r\n", connection_info->ap_idx);
    printf("[AT][RX]   ch_idx %u\r\n", connection_info->ch_idx);
    printf("[AT][RX]   qos %u\r\n", connection_info->qos);
    printf("[AT][RX]   aid %u\r\n", connection_info->aid);
    printf("[AT][RX]   band %s\r\n", (connection_info->chan_band) ? "5G" : "2.4G");
    printf("[AT][RX]   channel %d\r\n", connection_info->channel);
    printf("=================================================================\r\n");

    if (status_code == 0) {
        platform_post_event(EV_WIFI, CODE_WIFI_ON_CONNECTED);
        net_al_ext_dhcp_connect();
    } else {
        platform_post_event(EV_WIFI, CODE_WIFI_ON_DISCONNECT);
    }
}


/**
 ****************************************************************************************
 * @brief Process SM_CONNECT_IND message
 *
 * Sent by firmware when the connection is done
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_connect_ind(ke_msg_id_t const msgid,
                                          void *param,
                                          ke_task_id_t const dest_id,
                                          ke_task_id_t const src_id)
{
    struct cfgrwnx_connect_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(sm_connect_ind_get_vif_idx(param));
    int req_resp_ies_len = sm_connect_ind_get_assoc_req_ie_len(param) + sm_connect_ind_get_assoc_rsp_ie_len(param);

    if (sm_connect_ind_get_status_code(param)) {
        //connect fail
        fhost_cntrl_connect_ind_dump(sm_connect_ind_get_status_code(param), sm_connect_ind_get_ieeetypes_code(param));
    } else {
        //printf("%s: save ind data\r\n", __func__);
    }

    wifi_mgmr_sta_info_save(param);

    event.hdr.id = CFGRWNX_CONNECT_EVENT;
    event.hdr.len = sizeof(event);

    MAC_ADDR_CPY(&event.bssid, sm_connect_ind_getp_bssid(param));
    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    //XXX should use 802.11 reason_code from Packet IE
    event.status_code = sm_connect_ind_get_ieeetypes_code(param);
    event.freq = sm_connect_ind_get_chan_prim20_freq(param);
    event.assoc_req_ie_len = sm_connect_ind_get_assoc_req_ie_len(param);
    event.assoc_resp_ie_len = sm_connect_ind_get_assoc_rsp_ie_len(param);
    event.sta_idx = sm_connect_ind_get_ap_idx(param);

    #if NX_FHOST_RX_STATS
    fhost_cntrl_alloc_rx_rates(event.sta_idx);
    #endif

    if (req_resp_ies_len)
    {
        event.req_resp_ies = rtos_malloc(req_resp_ies_len);
        if (event.req_resp_ies == NULL)
        {
            TRACE_FHOST(ERR, "Failed to allocate assoc IE");
            return 0;
        }

        memcpy(event.req_resp_ies, sm_connect_ind_get_assoc_ie_buf(param), req_resp_ies_len);
    }
    else
    {
        event.req_resp_ies = NULL;
    }

    if (fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock) &&
        event.req_resp_ies)
    {
        rtos_free(event.req_resp_ies);
        return 0;
    }

    if (sm_connect_ind_get_status_code(param) == MAC_ST_SUCCESSFUL)
    {
        fhost_vif->ap_id = sm_connect_ind_get_ap_idx(param);
        fhost_vif->acm = sm_connect_ind_get_acm(param);

        if (fhost_vif->acm)
        {
            printf("admission control!!!\n");
        }

        fhost_tx_sta_add(fhost_vif->ap_id, 0);

        net_if_up_cb(fhost_vif->net_if);
    }
    return 0;
}

/**
 ****************************************************************************************
 * @brief Process SM_DISCONNECT_IND message
 *
 * Sent by firmware when the connection is lost. (After a disconnection request, a
 * disconnection from the AP or a lost link)
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_disconnection_ind(ke_msg_id_t const msgid,
                                                void *param,
                                                ke_task_id_t const dest_id,
                                                ke_task_id_t const src_id)
{
    struct cfgrwnx_disconnect_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(sm_disconnect_ind_get_vif_idx(param));

    printf("=================================================================\r\n");
    printf("[AT][RX] Connection Status\r\n");
    printf("[AT][RX]   status_code %u\r\n", sm_disconnect_ind_get_status_code(param));
    printf("[AT][RX]   reason_code %u\r\n", sm_disconnect_ind_get_reason_code(param));
    printf("[AT][RX]   status detail: %s\r\n", wifi_mgmr_get_sm_status_code_str(sm_disconnect_ind_get_status_code(param)));
    printf("[AT][RX]   vif_idx %u\r\n", sm_disconnect_ind_get_vif_idx(param));
    printf("=================================================================\r\n");

    wifi_mgmr_sta_info_upatestatus(sm_disconnect_ind_get_status_code(param), sm_disconnect_ind_get_reason_code(param));

    event.hdr.id = CFGRWNX_DISCONNECT_EVENT;
    event.hdr.len = sizeof(event);

    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    event.reason_code = sm_disconnect_ind_get_reason_code(param);

    net_if_down_cb(fhost_vif->net_if);

    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);
    fhost_vif->conn_sock = -1;

    fhost_vif = &fhost_env.vif[sm_disconnect_ind_get_vif_idx(param)];

    #if NX_FHOST_RX_STATS
    fhost_cntrl_free_rx_rates(fhost_vif->ap_id);
    #endif
    fhost_tx_sta_del(fhost_vif->ap_id);

    #ifdef LP_APP
    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_DISCONNECT, 0);
    #endif

    net_al_ext_dhcp_disconnect();

#ifdef CONFIG_RA_EN
    extern uint8_t conn_flag;

    if (!conn_flag)
    {
#endif

        platform_post_event(EV_WIFI, CODE_WIFI_ON_DISCONNECT);
#ifdef CONFIG_RA_EN
    }
    else {
        // Do not post CODE_WIFI_ON_DISCONNECT event during connecting
        conn_flag = 0;
    }
#endif
    return 0;
}

/**
 ****************************************************************************************
 * @brief Process ME_TKIP_MIC_FAILURE_IND message
 *
 * Sent by firmware when a TKIP MIC failure is detected on a received packet.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_mic_failure_ind(ke_msg_id_t const msgid,
                                              void *param,
                                              ke_task_id_t const dest_id,
                                              ke_task_id_t const src_id)
{
    struct cfgrwnx_mic_failure_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(me_tkip_mic_failure_ind_get_vif_idx(param));

    event.hdr.id = CFGRWNX_MIC_FAILURE_EVENT;
    event.hdr.len = sizeof(event);

    MAC_ADDR_CPY(&event.addr, me_tkip_mic_failure_ind_getp_addr(param));
    event.ga = me_tkip_mic_failure_ind_get_ga(param);
    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);

    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);

    return 0;
}

/**
 ****************************************************************************************
 * @brief Process ME_TX_CREDITS_UPDATE_IND message
 *
 * Sent by firmware when the a BlockAck agreement is established/deleted
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_credits_update_ind(ke_msg_id_t const msgid,
                                                 void *param,
                                                 ke_task_id_t const dest_id,
                                                 ke_task_id_t const src_id)
{
    fhost_tx_credits_update(me_tx_credits_update_ind_get_sta_idx(param), me_tx_credits_update_ind_get_tid(param), me_tx_credits_update_ind_get_credits(param));

    return 0;
}

/**
 ****************************************************************************************
 * @brief Process SM_EXTERNAL_AUTH_REQUIRED_IND message
 *
 * Sent by firmware when external authentication is required (i.e. firmware offload
 * authentication procedure to the supplicant)
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_external_auth_required_ind(ke_msg_id_t const msgid,
                                                         void *param,
                                                         ke_task_id_t const dest_id,
                                                         ke_task_id_t const src_id)
{
    struct cfgrwnx_external_auth_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(sm_external_auth_required_ind_get_vif_idx(param));

    event.hdr.id = CFGRWNX_EXTERNAL_AUTH_EVENT;
    event.hdr.len = sizeof(event);

    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    event.bssid = sm_external_auth_required_ind_get_bssid(param);
    event.ssid = sm_external_auth_required_ind_get_ssid(param);
    event.akm = sm_external_auth_required_ind_get_akm(param);

    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);

    return 0;
}

#if NX_BEACONING
/**
 ****************************************************************************************
 * @brief Process MM_PS_CHANGE_IND message
 *
 * Sent by firmware when a station changed its power save status (only in AP mode).
 * Forward message to TX task to start/stop TXQ of the station.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_ps_change_ind(ke_msg_id_t const msgid,
                                            void *param,
                                            ke_task_id_t const dest_id,
                                            ke_task_id_t const src_id)
{
    return fhost_tx_sta_ps_enable(mm_ps_change_ind_get_sta_idx(param), mm_ps_change_ind_get_ps_state(param));
}

/**
 ****************************************************************************************
 * @brief Process MM_TRAFFIC_REQ_IND message
 *
 * Sent by firmware when a station changed its power save status (only in AP mode).
 * Forward message to TX task to start/stop TXQ of the station.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_traffic_req_ind(ke_msg_id_t const msgid,
                                               void *param,
                                               ke_task_id_t const dest_id,
                                               ke_task_id_t const src_id)
{
    return fhost_tx_sta_ps_traffic_req(mm_traffic_req_ind_get_sta_idx(param), mm_traffic_req_ind_get_pkt_cnt(param), mm_traffic_req_ind_get_uapsd(param));
}

/**
 ****************************************************************************************
 * @brief Process APM_PROBE_CLIENT_IND message
 *
 * Sent by firmware to indicates result of APM_PROBE_CLIENT_REQ (only in AP mode).
 * Forward event (as CFGRWNX_PROBE_CLIENT_EVENT) to wpa_supplicant using 'conn_sock'.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_apm_probe_client_ind(ke_msg_id_t const msgid,
                                                   void *param,
                                                   ke_task_id_t const dest_id,
                                                   ke_task_id_t const src_id)
{
    struct cfgrwnx_probe_client_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(apm_probe_client_ind_get_vif_idx(param));

    event.hdr.id = CFGRWNX_PROBE_CLIENT_EVENT;
    event.hdr.len = sizeof(event);

    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);
    event.addr = sta_idx_getp_mac_addr(apm_probe_client_ind_get_sta_idx(param));
    event.client_present = apm_probe_client_ind_get_client_present(param);

    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);
    return 0;
}

#endif // NX_BEACONING

/**
 ****************************************************************************************
 * @brief Process MM_REMAIN_ON_CHANNEL_EXP_IND message
 *
 * Sent by firmware to indicates that the remain on channel has expired.
 * Forward event (as CFGRWNX_REMAIN_ON_CHANNEL_EXP_EVENT) to wpa_supplicant using 'conn_sock'.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_remain_on_channel_exp_ind(ke_msg_id_t const msgid,
                                                        void *param,
                                                        ke_task_id_t const dest_id,
                                                        ke_task_id_t const src_id)
{
    struct cfgrwnx_remain_on_channel_event event;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(mm_remain_on_channel_exp_ind_get_vif_index(param));

    event.hdr.id = CFGRWNX_REMAIN_ON_CHANNEL_EXP_EVENT;
    event.hdr.len = sizeof(event);

    event.freq =  mm_remain_on_channel_exp_ind_get_freq(param);
    event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);

    fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);
    return 0;
}

/**
 ****************************************************************************************
 * @brief Process MM_P2P_VIF_PS_CHANGE_IND message
 *
 * Sent by firmware when a GO changed its power save status in P2P mode.
 * Forward message to TX task to start/stop TXQ of the station associated to the GO in case
 * P2P is configured in client mode or the GO when P2P is configured in GO mode.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_p2p_vif_ps_change_ind(ke_msg_id_t const msgid,
                                                    void *param,
                                                    ke_task_id_t const dest_id,
                                                    ke_task_id_t const src_id)
{
    return fhost_tx_vif_ps_enable(mm_p2p_vif_ps_change_ind_get_vif_index(param), mm_p2p_vif_ps_change_ind_get_ps_state(param));
}

#if RW_MESH_EN
/**
 ****************************************************************************************
 * @brief Process MM_MESH_PATH_UPDATE_IND message
 *
 * Sent by firmware when a MESH path bas been updated. Allocate, delete  or update a MESH
 * path depending on the message.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_mesh_path_update_ind(ke_msg_id_t const msgid,
                                                   void *param,
                                                   ke_task_id_t const dest_id,
                                                   ke_task_id_t const src_id)
{
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(mesh_path_update_ind_get_vif_idx(param));
    bool found = false;

    // Path to be used
    struct fhost_mesh_path *mesh_path;

    mesh_path = (struct fhost_mesh_path *) co_list_pick(&fhost_vif->mpath_list);

    while (mesh_path != NULL)
    {
        if (mesh_path->path_idx == mesh_path_update_ind_get_path_idx(param))
        {
            found = true;
            break;
        }
        mesh_path = (struct fhost_mesh_path *) co_list_next((struct co_list_hdr *)mesh_path);
    }

    // Check if element has been deleted
    if (mesh_path_update_ind_get_delete(param))
    {
        if (found)
        {
            /* Remove element from list */
            co_list_extract(&fhost_vif->mpath_list, (struct co_list_hdr *)mesh_path);
            /* Add to the available list*/
            co_list_push_back(&fhost_vif->free_mpath_list, (struct co_list_hdr*)mesh_path);
        }
    }
    else
    {
        if (found)
        {
            // Update the Next Hop STA
            mesh_path->nhop_sta = mesh_path_update_ind_get_nhop_sta_idx(param);
        }
        else
        {
            // Allocate a Mesh Path structure
            mesh_path = (struct fhost_mesh_path *) co_list_pop_front(&fhost_vif->free_mpath_list);
            if (mesh_path != NULL)
            {
                mesh_path->path_idx = mesh_path_update_ind_get_path_idx(param);
                mesh_path->nhop_sta = mesh_path_update_ind_get_nhop_sta_idx(param);
                memcpy(&mesh_path->tgt_mac_addr, mesh_path_update_ind_get_tgt_mac_addr_p(param), MAC_ADDR_LEN);

                // Insert the path in the list of path
                co_list_push_back(&fhost_vif->mpath_list, (struct co_list_hdr*)mesh_path);
            }
        }
    }
    return 0;
}
#endif

#if NX_FTM_INITIATOR
/**
 ****************************************************************************************
 * @brief Process FTM_DONE_IND message
 *
 * Sent by firmware when a FTM measurement is available.
 *
 * @param[in] msgid   KE Message ID
 * @param[in] param   KE Message content
 * @param[in] dest_id KE task id that received this message
 * @param[in] src_id  KE task id that sent this message
 *
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_cntrl_ke_msg_ftm_done_ind(ke_msg_id_t const msgid,
                                           void *param,
                                           ke_task_id_t const dest_id,
                                           ke_task_id_t const src_id)
{
    struct cfgrwnx_ftm_done_event resp;
    struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(ftm_done_ind_get_vif_idx(param));

    resp.hdr.id = CFGRWNX_FTM_DONE_EVENT;
    resp.hdr.len = sizeof(resp);
    resp.vif_idx = ftm_done_ind_get_vif_idx(param);
    resp.results = ftm_done_ind_get_results(param);

    fhost_cntrl_cfgrwnx_event_send(&resp.hdr, fhost_vif->ftm_sock);
    fhost_vif->ftm_sock = -1;

    return 0;
}
#endif // NX_FTM_INITIATOR

/// Handlers function for KE indication message
static const struct ke_msg_handler fhost_cntrl_ke_msg_handlers[] =
{
    {SCANU_RESULT_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_scan_result_ind},
    {SCANU_START_CFM, (ke_msg_func_t)fhost_cntrl_ke_msg_scan_cfm},
    {SM_CONNECT_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_connect_ind},
    {SM_DISCONNECT_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_disconnection_ind},
    {ME_TX_CREDITS_UPDATE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_credits_update_ind},
    {ME_TKIP_MIC_FAILURE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_mic_failure_ind},
    {SM_EXTERNAL_AUTH_REQUIRED_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_external_auth_required_ind},
    #if NX_BEACONING
    {MM_PS_CHANGE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_ps_change_ind},
    {MM_TRAFFIC_REQ_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_traffic_req_ind},
    {APM_PROBE_CLIENT_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_apm_probe_client_ind},
    #endif
    {MM_REMAIN_ON_CHANNEL_EXP_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_remain_on_channel_exp_ind},
    {MM_P2P_VIF_PS_CHANGE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_p2p_vif_ps_change_ind},
    #if RW_MESH_EN
    {MESH_PATH_UPDATE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_mesh_path_update_ind},
    #endif
    #if NX_FTM_INITIATOR
    {FTM_DONE_IND, (ke_msg_func_t)fhost_cntrl_ke_msg_ftm_done_ind},
    #endif
    {0,NULL}
};

/**
 ****************************************************************************************
 * @brief Process one KE message
 *
 * looks for an adequate handler in @ref fhost_cntrl_ke_msg_handlers, and call the
 * corresponding function if found.
 * Message is ignored if no handler is found.
 * It is assumed that only indication message will be processed via this function.
 *
 * @param[in] ke_msg Message to process
 ****************************************************************************************
 */
static void fhost_cntrl_ke_msg_process(struct ke_msg *ke_msg)
{
    const struct ke_msg_handler *handler = fhost_cntrl_ke_msg_handlers;

    while (handler->id)
    {
        if (handler->id == ke_msg->id)
        {
            handler->func(ke_msg->id, (void *)ke_msg->param, ke_msg->dest_id,
                          ke_msg->src_id);
            break;
        }
        handler++;
    }
}

/**
 ****************************************************************************************
 * @brief Read message for KE message queue and process it
 *
 * All pending message in the KE message queue and read and processed.
 * The message is freed once processed.
 * Read is done without blocking, so that function returned after processing the last
 * message.
 *
 ****************************************************************************************
 */
static void fhost_cntrl_ke_msg_read_n_process(void)
{
    struct fhost_msg msg;
    struct ke_msg *ke_msg;

    while (!rtos_queue_read(queue_ke_msg, &msg, 0, false))
    {
        ke_msg = msg.data;
        fhost_cntrl_ke_msg_process(ke_msg);
        ke_msg_free(ke_msg);
    }
}

/**
 ****************************************************************************************
 * @brief Wait for specific response from wifi task
 *
 * Calling this function will block the control task until it receives the expected msg.
 * The task waits on th KE message queue and then ignore messages received on the master
 * queue. Other KE messages, if any, are processed.
 * Once the message is received its parameters are copied into @p resp buffer and the
 * message is freed.
 *
 * @param[in]  msg_id Id of the expected message
 * @param[out] resp   Buffer to copy message parameters into (may be NULL)
 * @param[in]  len    Size of the buffer pointed by @p resp
 ****************************************************************************************
 */
static void fhost_cntrl_ke_msg_wait(int msg_id, void *resp, int len)
{
    struct fhost_msg msg;
    struct ke_msg *ke_msg;
    bool found = false;

    while (!found)
    {
        rtos_queue_read(queue_ke_msg, &msg, -1, false);
        ke_msg = msg.data;

        if (ke_msg->id == msg_id)
        {
            found = true;
            if (resp && len)
            {
                int copy = ke_msg->param_len;
                if (copy > len)
                {
                    /* must not happen */
                    ASSERT_WARN(0);
                    copy = len;
                }
                memcpy(resp, ke_msg->param, copy);
            }
        }
        else
        {
            fhost_cntrl_ke_msg_process(ke_msg);
        }
        ke_msg_free(ke_msg);
    }
}

/****************************************************************************************
 * CFGRWNX MSG process
 ***************************************************************************************/
/// CFGRWNX handler description
struct fhost_cntrl_cfgrwnx_handler {
    /// message index
    int index;
    /// handler function
    void (*func) (void *msg);
};

/// CFGRWNX link parameters
struct fhost_cntrl_link cfgrwnx_link[FHOST_CNTRL_MAX_LINK];

/**
 ****************************************************************************************
 * @brief Send a cfgrwnx response message to task that sent a command
 *
 * @param[in] msg     Pointer on buffer to send
 * @param[in] msg_len Size, in bytes, on the buffer
 * @param[in] resp_queue RTOS queue on which the response to the command shall be written
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_resp_send(void const *msg, int msg_len,
                                          rtos_queue resp_queue)
{
    void *msg_buf;

    // Allocate memory to copy the data, because the message memory used by the caller
    // is supposed to be re-usable immediately after the function returns
    msg_buf = rtos_malloc(msg_len);
    if (msg_buf == NULL)
    {
        TRACE_FHOST(ERR, "[CFGRWNX] Failed to send response message of length %d", msg_len);
        return;
    }

    memcpy(msg_buf, msg, msg_len);
    rtos_queue_write(resp_queue, &msg_buf, -1, false);
}

/**
 ****************************************************************************************
 * @brief Add a new virtual interface to the MAC
 * This interface will be attached to the FHOST VIF element passed as parameter.
 *
 * @param[in,out] fhost_vif Pointer to the FHOST VIF element
 * @param[in]     type      Type of the new interface (@ref mac_vif_type)
 * @param[in]     p2p       Flag indicating whether the new interface is a P2P one
 *
 * @return 0 if successful, !=0 otherwise
 ****************************************************************************************
 */
static int fhost_cntrl_mac_vif_add(struct fhost_vif_tag *fhost_vif, uint8_t type, bool p2p)
{
    void *req = mm_add_if_req_alloc();
    void *cfm = mm_add_if_cfm_alloc();

    // Sanity check - We should not add a MAC VIF to a FHOST VIF that already has one
    ASSERT_ERR(fhost_vif->mac_vif == NULL);

    // Set message parameters
    mm_add_if_req_set_type(req, type);
    mm_add_if_req_set_p2p(req, p2p);
    mm_add_if_req_set_addr(req, fhost_vif->mac_addr);

    // Send message
    if (macif_kmsg_push(MM_ADD_IF_REQ, TASK_MM, req, mm_add_if_req_len()))
        goto err;

    // Wait for response
    fhost_cntrl_ke_msg_wait(MM_ADD_IF_CFM, cfm, mm_add_if_cfm_len());
    if (mm_add_if_cfm_get_status(cfm))
        goto err;

    fhost_vif->mac_vif = vif_info_get_vif(mm_add_if_cfm_get_inst_nbr(cfm));
    fhost_env.mac2fhost_vif[mm_add_if_cfm_get_inst_nbr(cfm)] = fhost_vif;
    #if RW_MESH_EN
    co_list_init(&fhost_vif->mpath_list);
    co_list_init(&fhost_vif->free_mpath_list);
    fhost_vif->is_resending = false;
    #endif
    fhost_tx_vif_txq_enable(fhost_vif);

    rtos_free(req);
    rtos_free(cfm);
    return 0;
 err:
    rtos_free(req);
    rtos_free(cfm);
    return -1;
}

/**
 ****************************************************************************************
 * @brief Delete a virtual interface of the MAC
 * This interface will be unlinked from the FHOST VIF element passed as parameter.
 *
 * @param[in] fhost_vif Pointer to the FHOST VIF element
 *
 * @return 0 if successful, !=0 otherwise
 ****************************************************************************************
 */
static int fhost_cntrl_mac_vif_del(struct fhost_vif_tag *fhost_vif)
{
    void *req = mm_remove_if_req_alloc();

    // Sanity check - We should not delete a MAC VIF for a FHOST VIF that doesn't have one
    ASSERT_ERR(fhost_vif->mac_vif != NULL);

    // Set parameters for the MM_REMOVE_IF_REQ message
    uint8_t inst_nbr = mac_vif_get_index(fhost_vif->mac_vif);
    mm_remove_if_req_set_inst_nbr(req, inst_nbr);

    fhost_tx_vif_txq_disable(fhost_vif);

    // Send message
    if (macif_kmsg_push(MM_REMOVE_IF_REQ, TASK_MM, req, mm_remove_if_req_len()))
        goto err;

    // Wait for response
    fhost_cntrl_ke_msg_wait(MM_REMOVE_IF_CFM, NULL, 0);

    fhost_vif->mac_vif = NULL;
    fhost_env.mac2fhost_vif[inst_nbr] = NULL;

    rtos_free(req);
    return 0;
 err:
    rtos_free(req);
    return -1;
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_HW_FEATURE_CMD message
 *
 * @param[in] msg Command header
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_hw_feature(void *msg)
{
    struct cfgrwnx_msg *cmd = msg;
    struct cfgrwnx_hw_feature feat;
    struct fhost_mm_start_req fhost_start;

    feat.hdr.id = CFGRWNX_HW_FEATURE_RESP;
    feat.hdr.len = sizeof(feat);
    fhost_config_prepare(&feat.me_config, &fhost_start, false);
    feat.chan = &fhost_chan;
    fhost_cntrl_cfgrwnx_resp_send(&feat, feat.hdr.len, cmd->hdr.resp_queue);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_GET_CAPA_CMD message
 *
 * @param[in] msg Command header
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_get_capa(void *msg)
{
    struct cfgrwnx_msg *cmd = msg;
    struct cfgrwnx_msg resp;

    resp.hdr.id = CFGRWNX_GET_CAPA_RESP;
    resp.hdr.len = sizeof(resp);

    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SET_KEY_CMD message
 *
 * @param[in] msg Key parameters (@ref cfgrwnx_set_key)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_set_key(void *msg)
{
    struct cfgrwnx_set_key *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    int sta_idx = INVALID_STA_IDX;

    resp.hdr.id = CFGRWNX_SET_KEY_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    if (mac_vif_get_type(mac_vif) == VIF_UNKNOWN)
    {
        resp.status = CFGRWNX_INVALID_VIF;
        goto send_resp;
    }

    if (cmd->addr && !MAC_ADDR_GROUP(cmd->addr))
    {
        sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
        if (sta_idx == INVALID_STA_IDX)
        {
            resp.status = CFGRWNX_INVALID_STA;
            goto send_resp;
        }
    }

    if (cmd->cipher_suite != MAC_CIPHER_INVALID)
    {
        struct mac_sec_key sec_key;
        void *req = mm_key_add_req_alloc();
        void *cfm =  mm_key_add_cfm_alloc();
        memset(req, 0, mm_key_add_req_len());
        mm_key_add_req_set_key_idx(req, cmd->key_idx);
        mm_key_add_req_set_sta_idx(req, sta_idx);
        ASSERT_ERR(cmd->key_len <= MAC_SEC_KEY_LEN);
        sec_key.length = cmd->key_len;
        memcpy(sec_key.array, cmd->key, cmd->key_len);
        mm_key_add_req_set_key(req, sec_key);
        mm_key_add_req_set_cipher_suite(req, cmd->cipher_suite);
        mm_key_add_req_set_inst_nbr(req, mac_vif_get_index(mac_vif));
        mm_key_add_req_set_spp(req, 0);
        mm_key_add_req_set_pairwise(req, cmd->pairwise);

        if (macif_kmsg_push(MM_KEY_ADD_REQ, TASK_MM, req, mm_key_add_req_len()))
        {
            resp.status = CFGRWNX_ERROR;
            rtos_free(req);
            rtos_free(cfm);
            goto send_resp;
        }

        fhost_cntrl_ke_msg_wait(MM_KEY_ADD_CFM, cfm, mm_key_add_cfm_len());
        if (mm_key_add_cfm_get_status(cfm))
            resp.status = CFGRWNX_ERROR;
        rtos_free(req);
        rtos_free(cfm);
    }
    else
    {
        void *req = mm_key_del_req_alloc();
        if (cmd->pairwise)
        {
            mm_key_del_req_set_hw_key_idx(req, export_MM_STA_TO_KEY(sta_idx));
        }
        #if RW_MESH_EN
        else if ((sta_idx != INVALID_STA_IDX) && (mac_vif_get_type(mac_vif) == VIF_MESH_POINT))
        {
            uint8_t mlink_idx = sta_idx_get_mlink_idx(sta_idx);
            if (cmd->key_idx > 3)
                mm_key_del_req_set_hw_key_idx(req, export_MM_MLINK_TO_MFP_KEY(cmd->key_idx, mlink_idx));
            else
                mm_key_del_req_set_hw_key_idx(req, export_MM_MLINK_TO_KEY(cmd->key_idx, mlink_idx));
        }
        #endif
        else
        {
            #if NX_MFP
            if (cmd->key_idx > 3)
                mm_key_del_req_set_hw_key_idx(req, export_MM_VIF_TO_MFP_KEY(cmd->key_idx, mac_vif_get_index(mac_vif)));
            else
            #endif
                mm_key_del_req_set_hw_key_idx(req, export_MM_VIF_TO_KEY(cmd->key_idx, mac_vif_get_index(mac_vif)));
        }

        if (macif_kmsg_push(MM_KEY_DEL_REQ, TASK_MM, req, mm_key_del_req_len()))
        {
            resp.status = CFGRWNX_ERROR;
            rtos_free(req);
            goto send_resp;
        }

        rtos_free(req);
        fhost_cntrl_ke_msg_wait(MM_KEY_DEL_CFM, NULL, 0);
    }

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
}

static void fhost_cntrl_cfgrwnx_raw_send(void *msg)
{
    struct cfgrwnx_raw_send *cmd = msg;
    struct cfgrwnx_resp resp;
    void *req = scan_raw_send_req_alloc();

    memset(req, 0, scan_raw_send_req_len());

    resp.hdr.id = CFGRWNX_RAW_SEND_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    scan_raw_send_req_set_pkt(req, cmd->pkt);
    scan_raw_send_req_set_len(req, cmd->len);
    scan_raw_send_req_set_vif_idx(req, cmd->fhost_vif_idx);
    scan_raw_send_req_set_index(req, 0);
    struct mac_chan_op chan_op;
    chan_op.band = PHY_BAND_2G4;
    chan_op.type = PHY_CHNL_BW_20;
    chan_op.prim20_freq = phy_channel_to_freq(chan_op.band, cmd->channel);
    chan_op.center1_freq = phy_channel_to_freq(chan_op.band, cmd->channel);//useless when bandwidth bigger than 20MHZ?
    chan_op.center2_freq = phy_channel_to_freq(chan_op.band, cmd->channel);//useless when bandwidth bigger than 20MHZ?
    chan_op.tx_power = 15;//FIXME which value should be tx_power set?
    chan_op.flags = CHAN_RADAR;
    scan_raw_send_req_set_chan(req, chan_op);

    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_SCAN, 1);

    if (macif_kmsg_push(SCAN_RAW_SEND_REQ, TASK_SCAN, req, scan_raw_send_req_len()))
        resp.status = CFGRWNX_ERROR;

    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SCAN_CMD message
 *
 * @param[in] msg Scan parameters (@ref cfgrwnx_scan)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_scan(void *msg)
{
    struct cfgrwnx_scan *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = scanu_start_req_alloc();
    int i, nb_ssid = 0;

    resp.hdr.id = CFGRWNX_SCAN_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    // if last scanning not done
    if (fhost_vif->scanning) {
        resp.status = CFGRWNX_ERROR;
        goto send_resp;
    }

    if (cmd->freqs && NULL == cmd->chans) {
        int *freq = cmd->freqs;
        struct mac_chan_def *chan;
        int nb_scan = 0;

        while (*freq) {
            chan = fhost_chan_get(*freq);

            if (chan) {
                memcpy(scanu_start_req_chan(req, nb_scan), chan, sizeof(*chan));
                nb_scan++;
            }

            freq++;

            if (nb_scan == SCAN_CHANNEL_MAX)
                break;
        }

        if (!nb_scan)
        {
            resp.status = CFGRWNX_INVALID_PARAM;
            goto send_resp;
        }
        scanu_start_req_set_chan_cnt(req, nb_scan);

    } else if (cmd->chans) {
        uint8_t *chans = cmd->chans;
        struct mac_chan_def *chan;
        int nb_scan = 0;
        int num = 0;

        while (num++ < cmd->chans_cnt) {
            if (*chans - 1 < fhost_chan.chan2G4_cnt && *chans > 0) {
                memcpy(scanu_start_req_chan(req, nb_scan), &fhost_chan.chan2G4[*chans - 1], sizeof(*chan));
                nb_scan++;
            }
            chans++;
            if (nb_scan == SCAN_CHANNEL_MAX)
                break;
        }

        if (!nb_scan)
        {
            resp.status = CFGRWNX_INVALID_PARAM;
            printf("Not support channel\r\n");
            goto send_resp;
        }
        scanu_start_req_set_chan_cnt(req, nb_scan);
    } else {
        ASSERT_ERR(SCAN_CHANNEL_MAX >= (fhost_chan.chan5G_cnt + fhost_chan.chan2G4_cnt));
        memcpy(scanu_start_req_chan(req, 0), fhost_chan.chan2G4, fhost_chan.chan2G4_cnt * sizeof(struct mac_chan_def));
        //memcpy(scanu_start_req_chan(req, fhost_chan.chan2G4_cnt), fhost_chan.chan5G, fhost_chan.chan5G_cnt * sizeof(struct mac_chan_def));
        scanu_start_req_set_chan_cnt(req, fhost_chan.chan2G4_cnt);
    }

    if (cmd->passive)
    {
        for (i = 0; i < scanu_start_req_get_chan_cnt(req); i++)
            scanu_start_req_set_chan_flags(req, i, CHAN_NO_IR);
    }

    for (i = 0; i < cmd->ssid_cnt ; i++)
    {
        if (cmd->ssids[i].len <= MAC_SSID_LEN)
        {
            memcpy(scanu_start_req_ssid_array(req, nb_ssid), cmd->ssids[i].ssid, cmd->ssids[i].len);
            scanu_start_req_set_ssid_length(req, nb_ssid++, cmd->ssids[i].len);
        }

        if (nb_ssid == SCAN_SSID_MAX)
            break;
    }
    if (! nb_ssid)
    {
        resp.status = CFGRWNX_INVALID_PARAM;
        goto send_resp;
    }
    scanu_start_req_set_ssid_cnt(req, nb_ssid);

    struct mac_addr tmp_bssid = {{0xffff, 0xffff, 0xffff}};
    if (cmd->bssid)
    {
        MAC_ADDR_CPY(scanu_start_req_getp_bssid(req), cmd->bssid);
    }
    else
    {
        MAC_ADDR_CPY(scanu_start_req_getp_bssid(req), &tmp_bssid);
    }

    scanu_start_req_set_add_ies(req, (uint32_t)cmd->extra_ies);
    scanu_start_req_set_add_ie_len(req, cmd->extra_ies_len);
    scanu_start_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    scanu_start_req_set_no_cck(req, cmd->no_cck);
    scanu_start_req_set_duration(req, cmd->duration * TU_DURATION);
    scanu_start_req_set_scan_only(req, cmd->is_cntrl_link);
    scanu_start_req_set_flags(req, cmd->flags);
    if (!cmd->is_cntrl_link) {
        scanu_start_req_set_flags(req, cmd->flags | wifi_mgmr_sta_connect_params_get());
    }
    fhost_vif->scan_sock = cmd->sock;
    fhost_vif->scanning = true;
    fhost_vif->is_cntrl_link = cmd->is_cntrl_link;

    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_SCAN, 1);

    if (macif_kmsg_push(SCANU_START_REQ, TASK_SCANU, req, scanu_start_req_len()))
        resp.status = CFGRWNX_ERROR;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_CONNECT_CMD message
 *
 * @param[in] msg Connection parameters (@ref cfgrwnx_connect)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_connect(void *msg)
{
    struct cfgrwnx_connect *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = sm_connect_req_alloc();
    void *cfm = sm_connect_cfm_alloc();

    resp.hdr.id = CFGRWNX_CONNECT_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    if (mac_vif_get_type(mac_vif) != VIF_STA)
    {
        resp.status = CFGRWNX_INVALID_VIF;
        goto send_resp;
    }

    if (cmd->ssid.len > MAC_SSID_LEN)
    {
        resp.status = CFGRWNX_INVALID_PARAM;
        goto send_resp;
    }

    struct mac_ssid ssid;
    ssid.length = cmd->ssid.len;
    memcpy(ssid.array, cmd->ssid.ssid, cmd->ssid.len);
    sm_connect_req_set_ssid(req, ssid);
    MAC_ADDR_CPY(sm_connect_req_getp_bssid(req), cmd->bssid);
    sm_connect_req_set_chan(req, cmd->chan);
    sm_connect_req_set_flags(req, cmd->flags|wifi_mgmr_sta_connect_params_get());
    sm_connect_req_set_ctrl_port_ethertype(req, cmd->ctrl_port_ethertype);
    sm_connect_req_set_ie_len(req, cmd->ie_len);
    sm_connect_req_set_listen_interval(req, fhost_params.listen_itv);
    sm_connect_req_set_dont_wait_bcmc(req, 0);  // TODO ?
    sm_connect_req_set_auth_type(req, cmd->auth_alg);
    sm_connect_req_set_wpa(req, cmd->wpa);
    if ((int16_t)cmd->uapsd == -1)
        sm_connect_req_set_uapsd_queues(req, fhost_vif->uapsd_queues);
    else
        sm_connect_req_set_uapsd_queues(req, cmd->uapsd);
    if (mac_vif_get_type(mac_vif) == VIF_UNKNOWN)
    {
        resp.status = CFGRWNX_INVALID_VIF;
        goto send_resp;
    }

    sm_connect_req_set_vif_idx(req, mac_vif_get_index(mac_vif));

    if (macif_kmsg_push_t(SM_CONNECT_REQ, TASK_SM, 2, req, sm_connect_req_len(), cmd->ie, cmd->ie_len))
        resp.status = CFGRWNX_ERROR;
    else
        fhost_cntrl_ke_msg_wait(SM_CONNECT_CFM, cfm, sm_connect_cfm_len());

    if (sm_connect_cfm_get_status(cfm) != CO_OK)
        resp.status = CFGRWNX_ERROR;
    else
        fhost_vif->conn_sock = cmd->sock;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_DISCONNECT_CMD message
 *
 * @param[in] msg Disconnection parameters (@ref cfgrwnx_disconnect)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_disconnect(void *msg)
{
    struct cfgrwnx_disconnect *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = sm_disconnect_req_alloc();

    resp.hdr.id = CFGRWNX_DISCONNECT_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    if (mac_vif_get_type(mac_vif) != VIF_STA)
    {
        resp.status = CFGRWNX_INVALID_VIF;
        goto send_resp;
    }
    sm_disconnect_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    sm_disconnect_req_set_reason_code(req, cmd->reason_code);
    sm_disconnect_req_set_status_code(req, cmd->status_code);

    if (macif_kmsg_push(SM_DISCONNECT_REQ, TASK_SM, req, sm_disconnect_req_len()))
        resp.status = CFGRWNX_ERROR;
    else
        fhost_cntrl_ke_msg_wait(SM_DISCONNECT_CFM, NULL, 0);

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_CTRL_PORT_CMD message
 *
 * @param[in] msg Control port parameters (@ref cfgrwnx_ctrl_port)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_ctrl_port(void *msg)
{
    struct cfgrwnx_ctrl_port *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = me_set_control_port_req_alloc();
    void *cfm = me_set_control_port_cfm_alloc();
    uint8_t sta_idx;

    resp.hdr.id = CFGRWNX_CTRL_PORT_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || ((mac_vif_get_type(mac_vif) != VIF_STA) && (mac_vif_get_type(mac_vif) != VIF_AP) && (mac_vif_get_type(mac_vif) != VIF_MESH_POINT)))
        goto send_resp;

    if (mac_vif_get_type(mac_vif) == VIF_STA)
        sta_idx = mac_vif_get_sta_ap_id(mac_vif);
    else
        sta_idx = vif_mgmt_get_staid(mac_vif, &cmd->addr);
    me_set_control_port_req_set_sta_idx(req, sta_idx);

    if (sta_idx == INVALID_STA_IDX)
        goto send_resp;

    me_set_control_port_req_set_control_port_open(req, cmd->authorized);

    if (macif_kmsg_push(ME_SET_CONTROL_PORT_REQ, TASK_ME, req, me_set_control_port_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_SET_CONTROL_PORT_CFM, cfm, me_set_control_port_cfm_len());
    if(mac_vif && mac_vif_get_type(mac_vif) == VIF_STA && me_set_control_port_cfm_get_control_port_open(cfm))
        fhost_cntrl_connect_ind_dump(WLAN_FW_SUCCESSFUL, 0);
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

#if NX_SYS_STAT
/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SYS_STATS_CMD message
 *
 * @param[in] msg Parameters (@ref cfgrwnx_msg)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_sys_stats(void *msg)
{
    struct cfgrwnx_msg *cmd = msg;
    void *cfm = dbg_get_sys_stat_cfm_alloc();
    struct cfgrwnx_sys_stats_resp resp;

    resp.hdr.id = CFGRWNX_SYS_STATS_RESP;
    resp.hdr.len = sizeof(resp);

    if (macif_kmsg_push(DBG_GET_SYS_STAT_REQ, TASK_DBG, NULL, 0))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(DBG_GET_SYS_STAT_CFM, cfm, dbg_get_sys_stat_cfm_len());
    resp.stats.cpu_sleep_time = dbg_get_sys_stat_cfm_get_cpu_sleep_time(cfm);
    resp.stats.doze_time = dbg_get_sys_stat_cfm_get_doze_time(cfm);
    resp.stats.stats_time = dbg_get_sys_stat_cfm_get_stats_time(cfm);

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(cfm);
}
#endif //NX_SYS_STAT

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SCAN_RESULTS_CMD message
 *
 * @param[in] msg Parameters (@ref cfgrwnx_scan_results)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_scan_results(void *msg)
{
    struct cfgrwnx_scan_results *cmd = msg;
    struct cfgrwnx_scan_results_resp resp;
    void *req = scanu_get_scan_result_req_alloc();

    resp.hdr.id = CFGRWNX_SCAN_RESULTS_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_SUCCESS;

    scanu_get_scan_result_req_set_idx(req, cmd->idx);

    if (macif_kmsg_push(SCANU_GET_SCAN_RESULT_REQ, TASK_SCANU, req, scanu_get_scan_result_req_len()))
    {
        resp.status = CFGRWNX_ERROR;
        goto send_resp;
    }

    fhost_cntrl_ke_msg_wait(SCANU_GET_SCAN_RESULT_CFM, &resp.scan_result,
                            sizeof(resp.scan_result));

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_LIST_FEATURES_CMD message
 *
 * @param[in] msg Parameters (@ref  cfgrwnx_msg)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_list_features(void *msg)
{
    struct cfgrwnx_msg *cmd = msg;
    struct cfgrwnx_list_features_resp resp;
    void *cfm = mm_version_cfm_alloc();

    resp.hdr.id = CFGRWNX_LIST_FEATURES_RESP;
    resp.hdr.len = sizeof(resp);

    if (macif_kmsg_push(MM_VERSION_REQ, TASK_MM, NULL, 0))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MM_VERSION_CFM, cfm, mm_version_cfm_len());
    resp.version.version_lmac = mm_version_cfm_get_version_lmac(cfm);
    resp.version.version_machw_1 = mm_version_cfm_get_version_machw_1(cfm);
    resp.version.version_machw_2 = mm_version_cfm_get_version_machw_2(cfm);
    resp.version.version_phy_1 = mm_version_cfm_get_version_phy_1(cfm);
    resp.version.version_phy_2 = mm_version_cfm_get_version_phy_2(cfm);
    resp.version.features = mm_version_cfm_get_features(cfm);
    resp.version.max_sta_nb = mm_version_cfm_get_max_sta_nb(cfm);
    resp.version.max_vif_nb = mm_version_cfm_get_max_vif_nb(cfm);

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SET_VIF_TYPE_CMD message
 *
 * Change the type of the MAC vif associated to the FHOST vif:
 * - Delete the current MAC vif (if any)
 * - Create a new MAC vif (unless requested type ifs VIF_UNKNOWN)
 *
 * @param[in] msg Parameters (@ref cfgrwnx_set_vif_type)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_set_vif_type(void *msg)
{
    struct cfgrwnx_set_vif_type *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif;

    resp.hdr.id = CFGRWNX_SET_VIF_TYPE_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (cmd->fhost_vif_idx >= NX_VIRT_DEV_MAX)
    {
        resp.status = CFGRWNX_INVALID_VIF;
        goto send_resp;
    }
    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];

    // Always delete even if it is already of the requested type
    if (fhost_vif->mac_vif && fhost_cntrl_mac_vif_del(fhost_vif))
        goto send_resp;

    if ((cmd->type != VIF_UNKNOWN) &&
        fhost_cntrl_mac_vif_add(fhost_vif, cmd->type, cmd->p2p))
        goto send_resp;

    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_MONITOR_CFG_CMD message
 *
 * @param[in] msg Key parameters (@ref cfgrwnx_monitor_cfg)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_monitor_cfg(void *msg)
{
    struct cfgrwnx_monitor_cfg *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = me_config_monitor_req_alloc();
    void *cfm = me_config_monitor_cfm_alloc();

    resp.hdr.id = CFGRWNX_MONITOR_CFG_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_MONITOR))
        goto send_resp;

    // Set parameters for the ME_CONFIG_MONITOR_REQ message
    me_config_monitor_req_set_chan(req, cmd->cfg.chan);
    me_config_monitor_req_set_uf(req, cmd->cfg.uf);
    me_config_monitor_req_set_chan_set(req, true);

    if (macif_kmsg_push(ME_CONFIG_MONITOR_REQ, TASK_ME, req, me_config_monitor_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_CONFIG_MONITOR_CFM, cfm, me_config_monitor_cfm_len());

    // Initialize the callback function
    fhost_rx_set_monitor_cb(cmd->cfg.cb_arg, cmd->cfg.cb);

    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_EXTERNAL_AUTH_STATUS_RESP message
 *
 * @param[in] msg Key parameters (@ref cfgrwnx_external_auth_status)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_external_auth_status(void *msg)
{
    struct cfgrwnx_external_auth_status *status = msg;
    void *mac_vif = fhost_to_mac_vif(status->fhost_vif_idx);
    void *rsp = sm_external_auth_required_rsp_alloc();

    if (!mac_vif)
        goto fail;

    sm_external_auth_required_rsp_set_vif_idx(rsp, mac_vif_get_index(mac_vif));
    sm_external_auth_required_rsp_set_status(rsp, status->status);

    macif_kmsg_push(SM_EXTERNAL_AUTH_REQUIRED_RSP, TASK_SM, rsp, sm_external_auth_required_rsp_len());

  fail:
    rtos_free(rsp);
}

#if NX_BEACONING
/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_START_AP_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_start_ap)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_start_ap(void *msg)
{
    struct cfgrwnx_start_ap *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif;
    void *req = apm_start_req_alloc();
    void *cfm = apm_start_cfm_alloc();

    resp.hdr.id = CFGRWNX_START_AP_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (cmd->fhost_vif_idx >= NX_VIRT_DEV_MAX)
        goto send_resp;
    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];

    if (!fhost_vif->mac_vif || (mac_vif_get_type(fhost_vif->mac_vif) != VIF_AP))
        goto send_resp;

    if (cmd->bcn_len > NX_BCNFRAME_LEN)
    {
        TRACE_FHOST(ERR, "Beacon is too long (%d bytes)", cmd->bcn_len);
        goto send_resp;
    }

    apm_start_req_set_basic_rates(req, cmd->basic_rates);
    apm_start_req_set_chan(req, cmd->chan);
    me_init_chan(apm_start_req_getp_chan(req));
    apm_start_req_set_bcn_addr(req, (uint32_t)cmd->bcn);
    apm_start_req_set_bcn_len(req, cmd->bcn_len);
    apm_start_req_set_bcn_int(req, cmd->bcn_int);
    apm_start_req_set_tim_oft(req, cmd->tim_oft);
    apm_start_req_set_tim_len(req, cmd->tim_len);
    apm_start_req_set_flags(req, cmd->flags);
    apm_start_req_set_ctrl_port_ethertype(req, cmd->ctrl_ethertype);
    apm_start_req_set_vif_idx(req, mac_vif_get_index(fhost_vif->mac_vif));

    if (macif_kmsg_push(APM_START_REQ, TASK_APM, req, apm_start_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(APM_START_CFM, cfm, apm_start_cfm_len());
    fhost_vif->conn_sock = cmd->sock;
    net_if_up_cb(fhost_vif->net_if);
    fhost_tx_sta_add(apm_start_cfm_get_bcmc_idx(cfm), cmd->bcn[cmd->tim_oft + MAC_TIM_PERIOD_OFT]
                     * 5 * cmd->bcn_int * TU_DURATION);
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_STOP_AP_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_stop_ap)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_stop_ap(void *msg)
{
    struct cfgrwnx_stop_ap *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif;
    void *req = apm_stop_req_alloc();

    resp.hdr.id = CFGRWNX_STOP_AP_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (cmd->fhost_vif_idx >= NX_VIRT_DEV_MAX)
        goto send_resp;
    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];

    if (!fhost_vif->mac_vif || (mac_vif_get_type(fhost_vif->mac_vif) != VIF_AP))
        goto send_resp;

    fhost_tx_sta_del(VIF_TO_BCMC_IDX(mac_vif_get_index(fhost_vif->mac_vif)));

    apm_stop_req_set_vif_idx(req, mac_vif_get_index(fhost_vif->mac_vif));
    if (macif_kmsg_push(APM_STOP_REQ, TASK_APM, req, apm_stop_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(APM_STOP_CFM, NULL, 0);
    fhost_vif->conn_sock = -1;
    net_if_down_cb(fhost_vif->net_if);
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SET_EDCA_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_set_edca)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_set_edca(void *msg)
{
    struct cfgrwnx_set_edca *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mm_set_edca_req_alloc();

    resp.hdr.id = CFGRWNX_SET_EDCA_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_AP))
        goto send_resp;

    if (cmd->aifsn > 15 || cmd->cwmin > 0x7FFF || cmd->cwmax > 0x7FFF || cmd->hw_queue > AC_VO)
        goto send_resp;

    mm_set_edca_req_set_inst_nbr(req, mac_vif_get_index(mac_vif));
    mm_set_edca_req_set_hw_queue(req, cmd->hw_queue);
    mm_set_edca_req_set_uapsd(req, false);
    mm_set_edca_req_set_ac_param(req, (cmd->aifsn |
                                       (32 - co_clz(cmd->cwmin)) << 4 |
                                       (32 - co_clz(cmd->cwmax)) << 8 |
                                       cmd->txop << 12));

    if (macif_kmsg_push(MM_SET_EDCA_REQ, TASK_MM, req, mm_set_edca_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MM_SET_EDCA_CFM, NULL, 0);
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_BCN_UPDATE_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_bcn_update)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_bcn_update(void *msg)
{
    struct cfgrwnx_bcn_update *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mm_bcn_change_req_alloc();

    resp.hdr.id = CFGRWNX_BCN_UPDATE_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_AP))
        goto send_resp;

    if (cmd->bcn_len > NX_BCNFRAME_LEN)
    {
        TRACE_FHOST(ERR, "Beacon update is too long (%d bytes)", cmd->bcn_len);
        goto send_resp;
    }

    mm_bcn_change_req_set_bcn_ptr(req, (uint32_t)cmd->bcn);
    mm_bcn_change_req_set_bcn_len(req, cmd->bcn_len);
    mm_bcn_change_req_set_tim_oft(req, cmd->tim_oft);
    mm_bcn_change_req_set_tim_len(req, cmd->tim_len);
    mm_bcn_change_req_set_inst_nbr(req, mac_vif_get_index(mac_vif));

    for (int i = 0; i < BCN_MAX_CSA_CPT; i++)
    {
        mm_bcn_change_req_set_csa_oft(req, cmd->csa_oft[i], i);
    }

    if (macif_kmsg_push(MM_BCN_CHANGE_REQ, TASK_MM, req, mm_bcn_change_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MM_BCN_CHANGE_CFM, NULL, 0);
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_STA_ADD_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_sta_add)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_sta_add(void *msg)
{
    struct cfgrwnx_sta_add *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = me_sta_add_req_alloc();
    void *cfm =  me_sta_add_cfm_alloc();

    resp.hdr.id = CFGRWNX_STA_ADD_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || ((mac_vif_get_type(mac_vif) != VIF_AP) && (mac_vif_get_type(mac_vif) != VIF_MESH_POINT)))
        goto send_resp;
    if (wifiMgmr.num_sta >= (wifiMgmr.max_sta_supported > NX_REMOTE_STA_MAX ? NX_REMOTE_STA_MAX : wifiMgmr.max_sta_supported)) {
        printf("Number of sta is %u now\r\n", wifiMgmr.num_sta);
        goto send_resp;
    }

    me_sta_add_req_set_mac_addr(req, *cmd->addr);
    me_sta_add_req_set_rate_set(req, cmd->rate_set);
    me_sta_add_req_set_ht_cap(req, cmd->ht_cap);
    me_sta_add_req_set_vht_cap(req, cmd->vht_cap);
    me_sta_add_req_set_he_cap(req, cmd->he_cap);
    me_sta_add_req_set_flags(req, cmd->flags);
    me_sta_add_req_set_aid(req, cmd->aid);
    me_sta_add_req_set_uapsd_queues(req, cmd->uapsd_queues);
    me_sta_add_req_set_max_sp_len(req, cmd->max_sp_len);
    me_sta_add_req_set_opmode(req, cmd->opmode);
    me_sta_add_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    me_sta_add_req_set_tdls_sta(req, false);
    me_sta_add_req_set_tdls_initiator(req, false);
    me_sta_add_req_set_tdls_chsw_allowed(req, false);

    if (macif_kmsg_push(ME_STA_ADD_REQ, TASK_ME, req, me_sta_add_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_STA_ADD_CFM, cfm, me_sta_add_cfm_len());
    uint8_t sta_idx = me_sta_add_cfm_get_sta_idx(cfm);
    fhost_tx_sta_add(sta_idx,
                     3 * cmd->listen_interval * mac_vif_get_bcn_int(mac_vif) * TU_DURATION);
    #if NX_FHOST_RX_STATS
    fhost_cntrl_alloc_rx_rates(sta_idx);
    #endif

    wifiMgmr.ap_sta_info[sta_idx].sta_idx = sta_idx;
    wifiMgmr.ap_sta_info[sta_idx].is_used = 1;
    wifiMgmr.ap_sta_info[sta_idx].aid = cmd->aid;
    memcpy(wifiMgmr.ap_sta_info[sta_idx].sta_mac, cmd->addr, 6);
    wifiMgmr.num_sta++;
    g_sta_idx=sta_idx;
    platform_post_event(EV_WIFI, CODE_WIFI_ON_AP_STA_ADD);

    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_STA_REMOVE_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_sta_remove)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_sta_remove(void *msg)
{
    struct cfgrwnx_sta_remove *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = me_sta_del_req_alloc();

    resp.hdr.id = CFGRWNX_STA_REMOVE_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || ((mac_vif_get_type(mac_vif) != VIF_AP) && (mac_vif_get_type(mac_vif) != VIF_MESH_POINT)))
        goto send_resp;

    uint8_t sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
    me_sta_del_req_set_tdls_sta(req, false);
    me_sta_del_req_set_sta_idx(req, sta_idx);
    if (sta_idx == INVALID_STA_IDX)
        goto send_resp;

    fhost_tx_sta_del(sta_idx);
    if (macif_kmsg_push(ME_STA_DEL_REQ, TASK_ME, req, me_sta_del_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_STA_DEL_CFM, NULL, 0);
    resp.status = CFGRWNX_SUCCESS;

    #if NX_FHOST_RX_STATS
    fhost_cntrl_free_rx_rates(sta_idx);
    #endif

    wifiMgmr.ap_sta_info[sta_idx].is_used = 0;
    wifiMgmr.num_sta--;
    g_sta_idx=sta_idx;
    platform_post_event(EV_WIFI, CODE_WIFI_ON_AP_STA_DEL);
  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_KEY_SEQNUM_CMD message
 *
 * @param[in] msg AP parameters (@ref cfgrwnx_key_seqnum)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_key_seqnum(void *msg)
{
    struct cfgrwnx_key_seqnum *cmd = msg;
    struct cfgrwnx_key_seqnum_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    struct key_info_tag *key = NULL;

    resp.hdr.id = CFGRWNX_KEY_SEQNUM_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_AP))
        goto send_resp;

    if (cmd->addr)
    {
        struct sta_info_tag *sta;
        int sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
        if (sta_idx != INVALID_STA_IDX)
        {
            if (sta_idx_get_key_idx(sta_idx) == cmd->key_idx)
                key = (struct key_info_tag *)sta_idx_getp_key_info(sta_idx);
        }
    }
    else if (cmd->key_idx < mac_vif_get_key_info_len(mac_vif))
    {
        key = mac_vif_get_key(mac_vif, cmd->key_idx);
    }

    if (key == NULL || !key->valid)
        goto send_resp;

    resp.seqnum = key->tx_pn;
    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_GET_STA_INFO_CMD message
 *
 * For now the only information is its inactivity time.
 *
 * @param[in] msg STA parameters (@ref cfgrwnx_get_sta_info)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_get_sta_info(void *msg)
{
    struct cfgrwnx_get_sta_info *cmd = msg;
    struct cfgrwnx_get_sta_info_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    int sta_idx;

    resp.hdr.id = CFGRWNX_GET_STA_INFO_RESP;
    resp.hdr.len = sizeof(resp);
    resp.inactive_msec = 0;

    if (!mac_vif || ((mac_vif_get_type(mac_vif) != VIF_AP) && (mac_vif_get_type(mac_vif) != VIF_MESH_POINT)))
        goto send_resp;

    sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
    if (sta_idx != INVALID_STA_IDX)
        resp.inactive_msec = (inline_hal_machw_time() - sta_idx_get_last_active(sta_idx)) / 1000;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_PROBE_CLIENT_EVENT message
 *
 * Forward the request to UMAC firmware (via APM_PROBE_CLIENT_REQ).
 *
 * @param[in] msg STA parameters (@ref cfgrwnx_get_sta_info)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_probe_client(void *msg)
{
    struct cfgrwnx_probe_client *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = apm_probe_client_req_alloc();
    void *cfm = apm_probe_client_cfm_alloc();
    int sta_idx;

    resp.hdr.id = CFGRWNX_PROBE_CLIENT_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_AP))
        goto send_resp;

    sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
    if (sta_idx == INVALID_STA_IDX)
        goto send_resp;

    apm_probe_client_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    apm_probe_client_req_set_sta_idx(req, sta_idx);

    if (macif_kmsg_push(APM_PROBE_CLIENT_REQ, TASK_APM, req, apm_probe_client_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(APM_PROBE_CLIENT_CFM, cfm, apm_probe_client_cfm_len());
    if (apm_probe_client_cfm_get_status(cfm) == CO_OK)
        resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

#endif // NX_BEACONING

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_RC_CMD message
 *
 * @param[in] msg RC parameters (@ref cfgrwnx_set_rc)
 ****************************************************************************************
 */
static void fhost_cnrtl_cfgrwnx_rc(void *msg)
{
#if 0
    // todo remove
    struct cfgrwnx_rc *cmd = msg;
    struct cfgrwnx_rc_result resp;
    struct me_rc_stats_req req;

    resp.hdr.id = CFGRWNX_RC_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;
    req.sta_idx = cmd->sta_idx;

    if (macif_kmsg_push(ME_RC_STATS_REQ, TASK_ME, &req, sizeof(req)))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_RC_STATS_CFM, &resp.cfm, sizeof(resp.cfm));
    if (resp.cfm.no_samples == 0)
        goto send_resp;

    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
#endif
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_RC_SET_RATE_CMD message
 *
 * @param[in] msg RC parameters (@ref cfgrwnx_set_rc)
 ****************************************************************************************
 */
static void fhost_cnrtl_cfgrwnx_rc_set_rate(void *msg)
{
    struct cfgrwnx_rc_set_rate *cmd = msg;
    void *req = me_rc_set_rate_req_alloc();
    struct cfgrwnx_resp resp;

    resp.hdr.id = CFGRWNX_RC_SET_RATE_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    me_rc_set_rate_req_set_sta_idx(req, cmd->sta_idx);
    me_rc_set_rate_req_set_fixed_rate_cfg(req, cmd->fixed_rate_cfg);

    if (macif_kmsg_push(ME_RC_SET_RATE_REQ, TASK_ME, req, me_rc_set_rate_req_len()))
        goto send_resp;

    resp.status = CFGRWNX_SUCCESS;

    send_resp:
      fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
      rtos_free(req);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_SET_PS_MODE_CMD message
 *
 * @param[in] msg PS parameters (@ref cfgrwnx_set_ps_mode)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_set_ps_mode(void *msg)
{
    struct cfgrwnx_set_ps_mode *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    #if NX_POWERSAVE
    void *req = me_set_ps_mode_req_alloc();
    #endif

    resp.hdr.id = CFGRWNX_SET_PS_MODE_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_STA))
        goto send_resp;

    #if NX_POWERSAVE
    me_set_ps_mode_req_set_ps_state(req, cmd->enabled);
    me_set_ps_mode_req_set_ps_mode(req, cmd->ps_mode);

    if (macif_kmsg_push(ME_SET_PS_MODE_REQ, TASK_ME, req, me_set_ps_mode_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(ME_SET_PS_MODE_CFM, NULL, 0);
    resp.status = CFGRWNX_SUCCESS;
    #endif

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    #if NX_POWERSAVE
    rtos_free(req);
    #endif
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_REMAIN_ON_CHANNEL_CMD message
 *
 * @param[in] msg remain on channel parameters (@ref cfgrwnx_remain_on_channel)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_remain_on_channel(void *msg)
{
    struct cfgrwnx_remain_on_channel *cmd = msg;
    struct cfgrwnx_resp resp;
    struct mac_chan_def *chan_def;
    struct mac_chan_op chan;
    struct fhost_vif_tag *fhost_vif;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    struct cfgrwnx_remain_on_channel_event event;
    void *req = mm_remain_on_channel_req_alloc();
    void *cfm = mm_remain_on_channel_cfm_alloc();

    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];
    fhost_vif->conn_sock = cmd->sock;

    resp.hdr.id = CFGRWNX_REMAIN_ON_CHANNEL_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    chan_def = fhost_chan_get(cmd->freq);
    chan.band = chan_def->band;
    chan.flags = chan_def->flags;
    chan.prim20_freq= chan_def->freq;
    chan.tx_power = chan_def->tx_power;
    chan.center1_freq = chan_def->freq;
    chan.center2_freq = 0;

    mm_remain_on_channel_req_set_chan(req, chan);
    mm_remain_on_channel_req_set_duration_ms(req, (uint32_t)cmd->duration);
    mm_remain_on_channel_req_set_op_code(req, MM_ROC_OP_START);
    mm_remain_on_channel_req_set_vif_index(req, mac_vif_get_index(mac_vif));

    // Send message
    if (macif_kmsg_push(MM_REMAIN_ON_CHANNEL_REQ, TASK_MM, req, mm_remain_on_channel_req_len()))
    {
        goto send_resp;
    }

    fhost_cntrl_ke_msg_wait(MM_REMAIN_ON_CHANNEL_CFM, cfm, mm_remain_on_channel_cfm_len());

    if (mm_remain_on_channel_cfm_get_status(cfm) == CO_OK)
    {
        event.hdr.id = CFGRWNX_REMAIN_ON_CHANNEL_EVENT;
        event.hdr.len = sizeof(event);

        event.duration = cmd->duration;
        event.freq = chan_def->freq;
        event.fhost_vif_idx = cmd->fhost_vif_idx;

        fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock);
        resp.status = CFGRWNX_SUCCESS;
    }

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_CANCEL_REMAIN_ON_CHANNEL_CMD message
 *
 * @param[in] msg remain on channel parameters (@ref cfgrwnx_remain_on_channel)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_cancel_remain_on_channel(void *msg)
{
    struct cfgrwnx_cancel_remain_on_channel *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mm_remain_on_channel_req_alloc();
    void *cfm = mm_remain_on_channel_cfm_alloc();

    resp.hdr.id = CFGRWNX_CANCEL_REMAIN_ON_CHANNEL_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    mm_remain_on_channel_req_set_op_code(req, MM_ROC_OP_CANCEL);
    mm_remain_on_channel_req_set_vif_index(req, mac_vif_get_index(mac_vif));

    // Send message
    if (macif_kmsg_push(MM_REMAIN_ON_CHANNEL_REQ, TASK_MM, req, mm_remain_on_channel_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MM_REMAIN_ON_CHANNEL_CFM, cfm, mm_remain_on_channel_cfm_len());

    if (mm_remain_on_channel_cfm_get_status(cfm) == CO_OK)
        resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_P2P_NOA_CMD message
 *
 * @param[in] msg NOA protocol parameters (@ref cfgrwnx_p2p_noa)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_p2p_noa(void *msg)
{
    struct cfgrwnx_p2p_noa_cmd *cmd = msg;
    struct cfgrwnx_resp resp;

    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mm_set_p2p_noa_req_alloc();
    void *cfm = mm_set_p2p_noa_cfm_alloc();

    resp.hdr.id = CFGRWNX_P2P_NOA_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    /* Fill the message parameters */
    mm_set_p2p_noa_req_set_vif_index(req, mac_vif_get_index(mac_vif));
    mm_set_p2p_noa_req_set_noa_inst_nb(req, 0);
    mm_set_p2p_noa_req_set_count(req, cmd->count);

    if (cmd->count) {
        mm_set_p2p_noa_req_set_duration_us(req, cmd->duration_us);
        mm_set_p2p_noa_req_set_interval_us(req, cmd->interval_us);
        mm_set_p2p_noa_req_set_start_offset(req, cmd->interval_us - cmd->duration_us - 10);
        mm_set_p2p_noa_req_set_dyn_noa(req, cmd->dyn_noa);
    }

    // Send message
    if (macif_kmsg_push(MM_SET_P2P_NOA_REQ, TASK_MM, req, mm_set_p2p_noa_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MM_SET_P2P_NOA_CFM, cfm, mm_set_p2p_noa_cfm_len());

    if (mm_set_p2p_noa_cfm_get_status(cfm) == CO_OK)
        resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
  }

#if RW_MESH_EN
/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_JOIN_MESH message
 *
 * @param[in] msg MESH parameters (@ref cfgrwnx_join_mesh)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_join_mesh(void *msg)
{
    struct cfgrwnx_join_mesh_cmd *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mesh_start_req_alloc();
    void *cfm = mesh_start_cfm_alloc();
    struct fhost_vif_tag *fhost_vif;

    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];

    resp.hdr.id = CFGRWNX_JOIN_MESH_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    mesh_start_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    mesh_start_req_set_bcn_int(req, cmd->bcn_int);
    mesh_start_req_set_dtim_period(req, cmd->dtim_period);
    mesh_start_req_set_mesh_id_len(req, cmd->mesh_id_len);

    for (int i = 0; i < cmd->mesh_id_len; i++)
    {
        mesh_start_req_set_mesh_id(req, *(cmd->mesh_id + i), i);
    }

    mesh_start_req_set_user_mpm(req, cmd->user_mpm);
    mesh_start_req_set_is_auth(req, cmd->is_auth);
    mesh_start_req_set_auth_id(req, cmd->auth_id);
    mesh_start_req_set_ie_len(req, cmd->ie_len);
    mesh_start_req_set_ie_addr(req, (uint32_t)cmd->ie);
    mesh_start_req_set_chan(req, cmd->chan);
    mesh_start_req_set_basic_rates(req, cmd->rates);

    if (macif_kmsg_push(MESH_START_REQ, TASK_MESH, req, mesh_start_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MESH_START_CFM, cfm, mesh_start_cfm_len());

    fhost_vif->conn_sock = cmd->sock;
    net_if_up_cb(fhost_vif->net_if);
    fhost_tx_sta_add(mesh_start_cfm_get_bcmc_idx(cfm), cmd->dtim_period * 5 * cmd->bcn_int * TU_DURATION);

    for (int i = 0; i < RW_MESH_PATH_NB; i++)
        co_list_push_front(&fhost_vif->free_mpath_list,&fhost_vif->fhost_mesh_path_pool[i].hdr);

    if (mesh_start_cfm_get_status(cfm) == CO_OK)
    {
        resp.status = CFGRWNX_SUCCESS;
    }

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_LEAVE_MESH message
 *
 * @param[in] msg MESH parameters (@ref cfgrwnx_leave_mesh)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_leave_mesh(void *msg)
{
    struct cfgrwnx_leave_mesh_cmd *cmd = msg;
    struct cfgrwnx_resp resp;
    void *req = mesh_stop_req_alloc();
    void *cfm = mesh_stop_cfm_alloc();
    struct fhost_vif_tag *fhost_vif;

    resp.hdr.id = CFGRWNX_LEAVE_MESH_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (cmd->fhost_vif_idx >= NX_VIRT_DEV_MAX)
        goto send_resp;

    fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];

    if (!fhost_vif->mac_vif || (mac_vif_get_type(fhost_vif->mac_vif) != VIF_MESH_POINT))
        goto send_resp;

    fhost_tx_sta_del(VIF_TO_BCMC_IDX(mac_vif_get_index(fhost_vif->mac_vif)));

    mesh_stop_req_set_vif_idx(req, mac_vif_get_index(fhost_vif->mac_vif));

    if (macif_kmsg_push(MESH_STOP_REQ, TASK_MESH, req, mesh_stop_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(MESH_STOP_CFM, cfm, mesh_stop_cfm_len());
    fhost_vif->conn_sock = -1;
    net_if_down_cb(fhost_vif->net_if);

    co_list_init(&fhost_vif->mpath_list);

    if (mesh_stop_cfm_get_status(cfm) == CO_OK)
        resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
}

/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_MESH_PEER_UPDATE_NTF message
 *
 * @param[in] msg MESH parameters (@ref cfgrwnx_mesh_peer_update_ntf)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_mesh_peer_update_ntf(void *msg)
{
    struct cfgrwnx_mesh_peer_update_ntf *cmd = msg;
    struct cfgrwnx_resp resp;
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *req = mesh_peer_update_ntf_alloc();
    int sta_idx;

    resp.hdr.id = CFGRWNX_MESH_PEER_UPDATE_NTF_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    if (!mac_vif || (mac_vif_get_type(mac_vif) != VIF_MESH_POINT))
        goto send_resp;

    sta_idx = vif_mgmt_get_staid(mac_vif, cmd->addr);
    if (sta_idx == INVALID_STA_IDX)
        goto send_resp;

    mesh_peer_update_ntf_set_vif_idx(req, mac_vif_get_index(mac_vif));
    mesh_peer_update_ntf_set_sta_idx(req, sta_idx);
    mesh_peer_update_ntf_set_state(req, cmd->state);

    if (macif_kmsg_push(MESH_PEER_UPDATE_NTF, TASK_MESH, req, mesh_peer_update_ntf_len()))
        goto send_resp;

    resp.status = CFGRWNX_SUCCESS;

  send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
}
#endif

static void fhost_cntrl_cfgrwnx_status_code(void *msg)
{
    struct cfgrwnx_status_code_print *cmd = msg;
#if 0
    if (!strncmp(cmd->msg, "CTRL-EVENT-CONNECTED", strlen("CTRL-EVENT-CONNECTED"))) {
        fhost_cntrl_connect_ind_dump(WLAN_FW_SUCCESSFUL, 0);
    } else
#endif
    if (!strncmp(cmd->msg, "CTRL-EVENT-NETWORK-NOT-FOUND", strlen("CTRL-EVENT-NETWORK-NOT-FOUND"))) {
        fhost_cntrl_connect_ind_dump(WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL,0);
    }
}

#if NX_FTM_INITIATOR
/**
 ****************************************************************************************
 * @brief Process @ref CFGRWNX_FTM_START_CMD message
 *
 * @param[in] msg FTM parameters (@ref cfgrwnx_ftm_start)
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_ftm_start(void *msg)
{
    void *req = ftm_start_req_alloc();
    struct cfgrwnx_ftm_start_cmd *cmd = msg;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[cmd->fhost_vif_idx];
    void *mac_vif = fhost_to_mac_vif(cmd->fhost_vif_idx);
    void *cfm = ftm_start_cfm_alloc();

    resp.hdr.id = CFGRWNX_FTM_START_RESP;
    resp.hdr.len = sizeof(resp);
    resp.status = CFGRWNX_ERROR;

    /* Fill the message parameters */
    ftm_start_req_set_vif_idx(req, mac_vif_get_index(mac_vif));
    ftm_start_req_set_ftm_per_burst(req, 4);
    ftm_start_req_set_nb_ftm_rsp(req, 3);

    // Send message
    if (macif_kmsg_push(FTM_START_REQ, TASK_FTM, req, ftm_start_req_len()))
        goto send_resp;

    fhost_cntrl_ke_msg_wait(FTM_START_CFM, cfm, ftm_start_cfm_len());

    if (ftm_start_cfm_get_status(cfm) == CO_OK)
    {
        resp.status = CFGRWNX_SUCCESS;
        fhost_vif->ftm_sock = cmd->sock;
    }

    send_resp:
    fhost_cntrl_cfgrwnx_resp_send(&resp, resp.hdr.len, cmd->hdr.resp_queue);
    rtos_free(req);
    rtos_free(cfm);
  }
#endif // NX_FTM_INITIATOR

/// Handlers function for cfgrwnx messages
static const struct fhost_cntrl_cfgrwnx_handler fhost_cntrl_cfgrwnx_msg_handlers[] =
{
    {CFGRWNX_HW_FEATURE_CMD, fhost_cntrl_cfgrwnx_hw_feature},
    {CFGRWNX_GET_CAPA_CMD, fhost_cntrl_cfgrwnx_get_capa},
    {CFGRWNX_SET_KEY_CMD, fhost_cntrl_cfgrwnx_set_key},
    {CFGRWNX_SCAN_CMD, fhost_cntrl_cfgrwnx_scan},
    {CFGRWNX_RAW_SEND_CMD, fhost_cntrl_cfgrwnx_raw_send},
    {CFGRWNX_CONNECT_CMD, fhost_cntrl_cfgrwnx_connect},
    {CFGRWNX_DISCONNECT_CMD, fhost_cntrl_cfgrwnx_disconnect},
    {CFGRWNX_CTRL_PORT_CMD, fhost_cntrl_cfgrwnx_ctrl_port},
    #if NX_SYS_STAT
    {CFGRWNX_SYS_STATS_CMD, fhost_cntrl_cfgrwnx_sys_stats},
    #endif //NX_SYS_STAT
    {CFGRWNX_SCAN_RESULTS_CMD, fhost_cntrl_cfgrwnx_scan_results},
    {CFGRWNX_LIST_FEATURES_CMD, fhost_cntrl_cfgrwnx_list_features},
    {CFGRWNX_SET_VIF_TYPE_CMD, fhost_cntrl_cfgrwnx_set_vif_type},
    {CFGRWNX_MONITOR_CFG_CMD, fhost_cntrl_cfgrwnx_monitor_cfg},
    {CFGRWNX_EXTERNAL_AUTH_STATUS_RESP, fhost_cntrl_cfgrwnx_external_auth_status},
    #if NX_BEACONING
    {CFGRWNX_START_AP_CMD, fhost_cntrl_cfgrwnx_start_ap},
    {CFGRWNX_STOP_AP_CMD, fhost_cntrl_cfgrwnx_stop_ap},
    {CFGRWNX_SET_EDCA_CMD, fhost_cntrl_cfgrwnx_set_edca},
    {CFGRWNX_BCN_UPDATE_CMD, fhost_cntrl_cfgrwnx_bcn_update},
    {CFGRWNX_STA_ADD_CMD, fhost_cntrl_cfgrwnx_sta_add},
    {CFGRWNX_STA_REMOVE_CMD, fhost_cntrl_cfgrwnx_sta_remove},
    {CFGRWNX_KEY_SEQNUM_CMD, fhost_cntrl_cfgrwnx_key_seqnum},
    {CFGRWNX_GET_STA_INFO_CMD, fhost_cntrl_cfgrwnx_get_sta_info},
    {CFGRWNX_PROBE_CLIENT_CMD, fhost_cntrl_cfgrwnx_probe_client},
    #endif // NX_BEACONING
    {CFGRWNX_SET_PS_MODE_CMD, fhost_cntrl_cfgrwnx_set_ps_mode},
    {CFGRWNX_REMAIN_ON_CHANNEL_CMD, fhost_cntrl_cfgrwnx_remain_on_channel},
    {CFGRWNX_CANCEL_REMAIN_ON_CHANNEL_CMD, fhost_cntrl_cfgrwnx_cancel_remain_on_channel},
    {CFGRWNX_RC_CMD, fhost_cnrtl_cfgrwnx_rc},
    {CFGRWNX_P2P_NOA_CMD, fhost_cntrl_cfgrwnx_p2p_noa},
    {CFGRWNX_RC_SET_RATE_CMD, fhost_cnrtl_cfgrwnx_rc_set_rate},
    #if RW_MESH_EN
    {CFGRWNX_JOIN_MESH_CMD, fhost_cntrl_cfgrwnx_join_mesh},
    {CFGRWNX_LEAVE_MESH_CMD, fhost_cntrl_cfgrwnx_leave_mesh},
    {CFGRWNX_MESH_PEER_UPDATE_NTF_CMD, fhost_cntrl_cfgrwnx_mesh_peer_update_ntf},
    #endif
    #if NX_FTM_INITIATOR
    {CFGRWNX_FTM_START_CMD, fhost_cntrl_cfgrwnx_ftm_start},
    #endif
    {CFGRWNX_STATUS_CODE_PRINT_CMD, fhost_cntrl_cfgrwnx_status_code},
    {0,NULL}
};

/**
 ****************************************************************************************
 * @brief Process a CFGRWNX message
 *
 * Find and call callback associated to CFGRWNX message.
 *
 * @param[in] msg Message to process
 ****************************************************************************************
 */
static void fhost_cntrl_cfgrwnx_msg_process(struct fhost_msg *msg)
{
    const struct fhost_cntrl_cfgrwnx_handler *handler = fhost_cntrl_cfgrwnx_msg_handlers;
    int index = FHOST_MSG_INDEX(msg->id);

    while (handler->index)
    {
        if (handler->index == index)
        {
            handler->func(msg->data);
            break;
        }
        handler++;
    }
}

/****************************************************************************************
 * Task loop and helper
 ***************************************************************************************/
/**
 ****************************************************************************************
 * @brief Initialize FHOST VIF
 *
 * This include initialization of the fhost VIF structure (mac address, ...) and
 * registration of the interface in the IP stack.
 *
 * @param[in] fhost_vif_idx  Index of the FHOST VIF to initialize.
 *                           No check on the value, assume it is valid.
 * @param[in] base_mac_addr  Base MAC address
 ****************************************************************************************
 */
static void fhost_cntrl_vif_init(int fhost_vif_idx, struct mac_addr *base_mac_addr)
{
    struct fhost_vif_tag *fhost_vif =  &fhost_env.vif[fhost_vif_idx];

    fhost_vif->mac_addr = *base_mac_addr;
    fhost_vif->scan_sock = -1;
    fhost_vif->scanning = false;
    fhost_vif->conn_sock = -1;
    fhost_vif->ftm_sock = -1;
    fhost_vif->mac_vif = NULL;
    fhost_vif->ap_id = INVALID_STA_IDX;
    fhost_vif->acm = 0;

    net_if_add(&fhost_vif->net_if, (uint8_t *)fhost_vif->mac_addr.array,
               NULL, NULL, NULL, fhost_vif);
}

/**
 ****************************************************************************************
 * @brief Start WiFi stack.
 *
 * Send messages to configure and start the WiFi STack and then add a first STA interface.
 * Configuration is read form IPC buffer so that it can be modified via driver for test
 * purpose.
 *
 * @return 0 on success and != 0 otherwise
 ****************************************************************************************
 */
static int fhost_cntrl_start(void)
{
    struct fhost_mm_version_cfm version;
    void *cfm = mm_version_cfm_alloc();
    struct fhost_me_config_req me_config;
    void *me_req = me_config_req_alloc();
    struct fhost_mm_start_req start;
    void *mm_req = mm_start_req_alloc();
    struct mac_addr mac_addr;

    fhost_config_prepare(&me_config, &start, true);
    memset(me_req, 0, me_config_req_len());
    me_config_req_set_ht_cap(me_req, me_config.ht_cap);
    me_config_req_set_vht_cap(me_req, me_config.vht_cap);
    me_config_req_set_he_cap(me_req, me_config.he_cap);
    me_config_req_set_tx_lft(me_req, me_config.tx_lft);
    me_config_req_set_phy_bw_max(me_req, me_config.phy_bw_max);
    me_config_req_set_ht_supp(me_req, me_config.ht_supp);
    me_config_req_set_vht_supp(me_req, me_config.vht_supp);
    me_config_req_set_he_supp(me_req, me_config.he_supp);
    me_config_req_set_he_ul_on(me_req, me_config.he_ul_on);
    me_config_req_set_ps_on(me_req, me_config.ps_on);
    me_config_req_set_ant_div_on(me_req, me_config.ant_div_on);
    me_config_req_set_dpsm(me_req, me_config.dpsm);
    me_config_req_set_amsdu_tx(me_req, me_config.amsdu_tx);

    memset(mm_req, 0, mm_start_req_len());
    mm_start_req_set_phy_cfg(mm_req, start.phy_cfg);
    mm_start_req_set_uapsd_timeout(mm_req, start.uapsd_timeout);
    mm_start_req_set_lp_clk_accuracy(mm_req, start.lp_clk_accuracy);
    mm_start_req_set_tx_timeout(mm_req, start.tx_timeout, AC_MAX);

    wifi_mgmr_sta_mac_get((uint8_t *)mac_addr.array);
    fhost_cntrl_vif_init(0, &mac_addr);
    wifi_mgmr_ap_mac_get((uint8_t *)mac_addr.array);
    fhost_cntrl_vif_init(1, &mac_addr);

    if (macif_kmsg_push(MM_RESET_REQ, TASK_MM, NULL, 0))
        goto err;
    fhost_cntrl_ke_msg_wait(MM_RESET_CFM, NULL, 0);

    if (macif_kmsg_push(MM_VERSION_REQ, TASK_MM, NULL, 0))
        goto err;
    fhost_cntrl_ke_msg_wait(MM_VERSION_CFM, cfm, mm_version_cfm_len());
    version.version_lmac = mm_version_cfm_get_version_lmac(cfm);
    version.version_machw_1 = mm_version_cfm_get_version_machw_1(cfm);
    version.version_machw_2 = mm_version_cfm_get_version_machw_2(cfm);
    version.version_phy_1 = mm_version_cfm_get_version_phy_1(cfm);
    version.version_phy_2 = mm_version_cfm_get_version_phy_2(cfm);
    version.features = mm_version_cfm_get_features(cfm);
    version.max_sta_nb = mm_version_cfm_get_max_sta_nb(cfm);
    version.max_vif_nb = mm_version_cfm_get_max_vif_nb(cfm);

    if (macif_kmsg_push(ME_CONFIG_REQ, TASK_ME, me_req, me_config_req_len()))
        goto err;
    fhost_cntrl_ke_msg_wait(ME_CONFIG_CFM, NULL, 0);

    if (macif_kmsg_push(MM_START_REQ, TASK_MM, mm_req, mm_start_req_len()))
        goto err;

    // Create interface 0
    if (fhost_cntrl_mac_vif_add(&fhost_env.vif[0], VIF_STA, false))
        goto err;

    platform_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE);

    rtos_free(cfm);
    rtos_free(me_req);
    rtos_free(mm_req);
    return 0;
 err:
    rtos_free(cfm);
    rtos_free(me_req);
    rtos_free(mm_req);
    return 1;
}

#if 0
int fhost_cntrl_restart(void)
{
    struct fhost_mm_version_cfm version;
    void *cfm = mm_version_cfm_alloc();
    struct fhost_me_config_req me_config;
    void *me_req = me_config_req_alloc();
    struct fhost_mm_start_req start;
    void *mm_req = mm_start_req_alloc();
    struct mac_addr mac_addr;

    vTaskSuspend(fhost_cntrl_task_handle);

    //rwnxl_init();

    fhost_config_prepare(&me_config, &start, true);
    memset(me_req, 0, me_config_req_len());
    me_config_req_set_ht_cap(me_req, me_config.ht_cap);
    me_config_req_set_vht_cap(me_req, me_config.vht_cap);
    me_config_req_set_he_cap(me_req, me_config.he_cap);
    me_config_req_set_tx_lft(me_req, me_config.tx_lft);
    me_config_req_set_phy_bw_max(me_req, me_config.phy_bw_max);
    me_config_req_set_ht_supp(me_req, me_config.ht_supp);
    me_config_req_set_vht_supp(me_req, me_config.vht_supp);
    me_config_req_set_he_supp(me_req, me_config.he_supp);
    me_config_req_set_he_ul_on(me_req, me_config.he_ul_on);
    me_config_req_set_ps_on(me_req, me_config.ps_on);
    me_config_req_set_ant_div_on(me_req, me_config.ant_div_on);
    me_config_req_set_dpsm(me_req, me_config.dpsm);
    me_config_req_set_amsdu_tx(me_req, me_config.amsdu_tx);

    memset(mm_req, 0, mm_start_req_len());
    mm_start_req_set_phy_cfg(mm_req, start.phy_cfg);
    mm_start_req_set_uapsd_timeout(mm_req, start.uapsd_timeout);
    mm_start_req_set_lp_clk_accuracy(mm_req, start.lp_clk_accuracy);
    mm_start_req_set_tx_timeout(mm_req, start.tx_timeout, AC_MAX);

    if (macif_kmsg_push(MM_RESET_REQ, TASK_MM, NULL, 0))
        goto err;
    fhost_cntrl_ke_msg_wait(MM_RESET_CFM, NULL, 0);

    if (macif_kmsg_push(MM_VERSION_REQ, TASK_MM, NULL, 0))
        goto err;
    fhost_cntrl_ke_msg_wait(MM_VERSION_CFM, cfm, mm_version_cfm_len());
    version.version_lmac = mm_version_cfm_get_version_lmac(cfm);
    version.version_machw_1 = mm_version_cfm_get_version_machw_1(cfm);
    version.version_machw_2 = mm_version_cfm_get_version_machw_2(cfm);
    version.version_phy_1 = mm_version_cfm_get_version_phy_1(cfm);
    version.version_phy_2 = mm_version_cfm_get_version_phy_2(cfm);
    version.features = mm_version_cfm_get_features(cfm);
    version.max_sta_nb = mm_version_cfm_get_max_sta_nb(cfm);
    version.max_vif_nb = mm_version_cfm_get_max_vif_nb(cfm);

#if 0
    if (macif_kmsg_push(ME_CONFIG_REQ, TASK_ME, me_req, me_config_req_len()))
        goto err;
    fhost_cntrl_ke_msg_wait(ME_CONFIG_CFM, NULL, 0);
#endif

    if (macif_kmsg_push(MM_START_REQ, TASK_MM, mm_req, mm_start_req_len()))
        goto err;

    fhost_cntrl_mac_vif_del(&fhost_env.vif[0]);

    // Create interface 0
    if (fhost_cntrl_mac_vif_add(&fhost_env.vif[0], VIF_STA, false))
        goto err;

    platform_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE);

    vTaskResume(fhost_cntrl_task_handle);

    rtos_free(cfm);
    rtos_free(me_req);
    rtos_free(mm_req);
    return 0;
 err:
    rtos_free(cfm);
    rtos_free(me_req);
    rtos_free(mm_req);
    return 1;
}
#endif

int fhost_cntrl_chan_config_update(int channel_num)
{
    fhost_chan.chan2G4_cnt = channel_num;
    if (macif_kmsg_push(ME_CHAN_CONFIG_REQ, TASK_ME, &fhost_chan, sizeof(fhost_chan))) {
        return -1;
    }

    return 0;
}

/**
 ****************************************************************************************
 * @brief Contrl task main loop
 *
 ****************************************************************************************
 */
static RTOS_TASK_FCT(fhost_cntrl_task)
{
    struct fhost_msg msg;

    if (fhost_cntrl_start())
    {
        ASSERT_ERR(0);
    }
    fhost_task_ready(CONTROL_TASK);

    for( ;; )
    {
        rtos_queue_read(queue, &msg, -1, false);

        switch (FHOST_MSG_TYPE(msg.id))
        {
            case FHOST_MSG_KE_WIFI:
                fhost_cntrl_ke_msg_read_n_process();
                break;
            case FHOST_MSG_CFGRWNX:
                fhost_cntrl_cfgrwnx_msg_process(&msg);
                break;
            default:
                TRACE_FHOST(ERR, "Task CNTRL: unsupported message received (%d/%d)",
                            FHOST_MSG_TYPE(msg.id),
                            FHOST_MSG_INDEX(msg.id));
                break;
        }
    }
}

/****************************************************************************************
 * Task interface
 ***************************************************************************************/
int fhost_cntrl_init(void)
{
    for (int i = 0; i < FHOST_CNTRL_MAX_LINK; i++)
    {
        cfgrwnx_link[i].queue = NULL;
        cfgrwnx_link[i].sock_recv = -1;
        cfgrwnx_link[i].sock_send = -1;
    }

    if (rtos_queue_create(sizeof(struct fhost_msg), FHOST_CNTRL_QUEUE_SIZE, &queue))
        return 2;

    if (rtos_queue_create(sizeof(struct fhost_msg), FHOST_CNTRL_QUEUE_KE_MSG_SIZE,
                          &queue_ke_msg))
        return 3;

    if (rtos_task_create(fhost_cntrl_task, "Control", CONTROL_TASK,
                         FHOST_CNTRL_STACK_SIZE, NULL, FHOST_CNTRL_PRIORITY, &fhost_cntrl_task_handle))
        return 1;

#ifdef NO_MSG_PEEK
    rtos_mutex_create(&cntrl_no_msg_peek_send_lock);
#endif
    return 0;
}

int fhost_cntrl_write_msg(int id, void *msg_data, int len, int timeout, bool isr)
{
    struct fhost_msg msg;

    msg.id         = id;
    msg.len        = len;
    msg.data       = msg_data;

    if (FHOST_MSG_TYPE(id) == FHOST_MSG_KE_WIFI)
    {
        if (rtos_queue_write(queue_ke_msg, &msg, timeout, isr))
            return -1;

        /* TODO: add status to check if it is needed to also push ke_msg to common queue
           i.e. if task is not currently waiting for confirmation on ke_msg queue.
           For now always push ke_msg on both queue which is not optimal but easier.
        */
    }

    return rtos_queue_write(queue, &msg, timeout, isr);
}

void fhost_cntrl_cfgrwnx_link_close(struct fhost_cntrl_link *close_link)
{
    if (close_link->sock_recv >= 0)
    {
        close(close_link->sock_recv);
        close_link->sock_recv = -1;
    }
    if (close_link->sock_send >= 0)
    {
        close(close_link->sock_send);
        close_link->sock_send = -1;
    }

    rtos_queue_delete(close_link->queue);
    close_link->queue = NULL;
}

struct fhost_cntrl_link *fhost_cntrl_cfgrwnx_link_open()
{
    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr;
    unsigned int port = 0;
    unsigned int i = 0;

    for (i = 0; i < FHOST_CNTRL_MAX_LINK; i++)
    {
        if (cfgrwnx_link[i].queue == NULL)
            break;
    }

    if (i == FHOST_CNTRL_MAX_LINK)
    {
        TRACE_FHOST(ERR, "Maximum number of cfgrwnx link reached");
        return NULL;
    }

    if (rtos_queue_create(sizeof(void *), FHOST_CNTRL_QUEUE_CFGRWNX_RESP_SIZE,
                          &cfgrwnx_link[i].queue))
        return NULL;

    port = CFGRWNX_PORT;
    cfgrwnx_link[i].sock_recv = socket(PF_INET, SOCK_DGRAM, 0);
    if (cfgrwnx_link[i].sock_recv < 0)
        goto err;

    memset(&recv_addr, 0, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    recv_addr.sin_port = htons(port);
    if (bind(cfgrwnx_link[i].sock_recv, (struct sockaddr *) &recv_addr, sizeof(recv_addr)) < 0)
        goto err;

#ifndef NO_GLOBAL_SOCKET
    cfgrwnx_link[i].sock_send = socket(PF_INET, SOCK_DGRAM, 0);
    if (cfgrwnx_link[i].sock_send < 0)
        goto err;

    memset(&send_addr, 0, sizeof(send_addr));
    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    send_addr.sin_port = htons(port);
    if (connect(cfgrwnx_link[i].sock_send, (struct sockaddr *)&send_addr, sizeof(send_addr)) < 0)
        goto err;
#endif

    return &cfgrwnx_link[i];

  err:
    fhost_cntrl_cfgrwnx_link_close(&cfgrwnx_link[i]);
    return NULL;
}

struct fhost_cntrl_link *fhost_cntrl_cfgrwnx_link_open_without_sock()
{
    unsigned int i = 0;

    for (i = 0; i < FHOST_CNTRL_MAX_LINK; i++)
    {
        if (cfgrwnx_link[i].queue == NULL)
            break;
    }

    if (i == FHOST_CNTRL_MAX_LINK)
    {
        TRACE_FHOST(ERR, "Maximum number of cfgrwnx link reached");
        return NULL;
    }

    if (rtos_queue_create(sizeof(void *), FHOST_CNTRL_QUEUE_CFGRWNX_RESP_SIZE,
                          &cfgrwnx_link[i].queue))
        return NULL;

    return &cfgrwnx_link[i];
}

int fhost_cntrl_set_mac_vif_type(struct fhost_cntrl_link *link, int fhost_vif_idx,
                                 enum mac_vif_type type, bool p2p)
{
    struct cfgrwnx_set_vif_type cmd;
    struct cfgrwnx_resp resp;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SET_VIF_TYPE_CMD;
    cmd.hdr.resp_queue = link->queue;
    cmd.fhost_vif_idx = fhost_vif_idx;
    cmd.type = type;
    cmd.p2p = p2p;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SET_VIF_TYPE_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        return -1;
    }

    return 0;
}

#if NX_FHOST_MONITOR
int fhost_cntrl_monitor_cfg(struct fhost_cntrl_link *link, int fhost_vif_idx,
                            struct fhost_vif_monitor_cfg *cfg)
{
    struct cfgrwnx_monitor_cfg cmd;
    struct cfgrwnx_resp resp;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[fhost_vif_idx];
    struct mac_chan_def *chan;

    if (!fhost_vif->mac_vif || (mac_vif_get_type(fhost_vif->mac_vif) != VIF_MONITOR))
        return -1;

    // Test channel config
    chan = fhost_chan_get(cfg->chan.prim20_freq);
    if (!chan)
        return -1;
    if (cfg->chan.tx_power > chan->tx_power)
        cfg->chan.tx_power = chan->tx_power;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_MONITOR_CFG_CMD;
    cmd.hdr.resp_queue = link->queue;
    cmd.fhost_vif_idx = fhost_vif_idx;
    cmd.cfg = *cfg;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_MONITOR_CFG_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
        return -1;

    return 0;
}
#endif // NX_FHOST_MONITOR

int fhost_cntrl_cfgrwnx_cmd_send(struct cfgrwnx_msg_hdr *cmd,
                                 struct cfgrwnx_msg_hdr *resp)
{
    struct cfgrwnx_msg_hdr *msg_hdr;
    int err;

    // Send the command to the FHOST control thread
    err = fhost_cntrl_write_msg(FHOST_MSG_ID(FHOST_MSG_CFGRWNX, cmd->id),
                                cmd, cmd->len, -1, false);
    if (err)
    {
        TRACE_FHOST(ERR, "[CFGRWNX] Command write error");
        return err;
    }

    // return immediately if no response is expected
    if (!resp)
        return 0;

    // Wait for the response from the FHOST control thread
    rtos_queue_read(cmd->resp_queue, &msg_hdr, -1, false);

    if (msg_hdr->id != resp->id)
    {
        TRACE_FHOST(ERR, "[CFGRWNX] Unexpected response ID");
        return -1;
    }

    if (msg_hdr->len > resp->len)
    {
        TRACE_FHOST(ERR, "[CFGRWNX] Response buffer too small for received response");
        return -1;
    }
    memcpy(resp, msg_hdr, msg_hdr->len);

    rtos_free(msg_hdr);

    return 0;
}

int fhost_cntrl_cfgrwnx_event_send(struct cfgrwnx_msg_hdr *msg_hdr, int sock)
{
    ASSERT_ERR(sock >= 0);

    int ret = -1;
#ifdef NO_GLOBAL_SOCKET
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if(sock < 0) {
        TRACE_FHOST(ERR, "[CFGRWNX] fhost_cntrl -> wpa supplicant socket create fail");
        return ret;
    }
    struct sockaddr_in cntrl;
    struct sockaddr_in driver;

    cntrl.sin_family = AF_INET;
    cntrl.sin_addr.s_addr = htonl(INADDR_ANY);
    cntrl.sin_port =  htons(0);
    if (bind(sock, (struct sockaddr *)&cntrl, sizeof(cntrl)) < 0)
        goto err;

    driver.sin_family = AF_INET;
    driver.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    driver.sin_port = htons(CFGRWNX_PORT);
    if (connect(sock, (struct sockaddr *)&driver, sizeof(driver)) < 0)
        goto err;
#endif

#ifndef NO_MSG_PEEK
    if (send(sock, msg_hdr, msg_hdr->len, 0) < 0)
    {
        TRACE_FHOST(ERR, "[CFGRWNX] Failed to send event %d (length %d)",
                    msg_hdr->id, msg_hdr->len);
        goto err;
    }
#else
    rtos_mutex_lock(cntrl_no_msg_peek_send_lock);
    if (send(sock, msg_hdr, sizeof(*msg_hdr), 0) < 0 || send(sock, msg_hdr, msg_hdr->len, 0) < 0)
    {
            rtos_mutex_lock(cntrl_no_msg_peek_send_lock);
            TRACE_FHOST(ERR, "[CFGRWNX] Failed to send event %d (length %d)",
                        msg_hdr->id, msg_hdr->len);
            goto err;
    }
    rtos_mutex_unlock(cntrl_no_msg_peek_send_lock);
#endif

    ret = 0;
 err:
#ifdef NO_GLOBAL_SOCKET
    close(sock);
#endif
    return ret;
}

int fhost_cntrl_cfgrwnx_event_peek_header(struct fhost_cntrl_link *link,
                                          struct cfgrwnx_msg_hdr *msg_hdr)
{
#ifndef NO_MSG_PEEK
    if (recv(link->sock_recv, msg_hdr, (sizeof *msg_hdr), MSG_PEEK) < 0)
#else
    if (recv(link->sock_recv, msg_hdr, (sizeof *msg_hdr), 0) < 0)
#endif
        return -1;

    return 0;
}

int fhost_cntrl_cfgrwnx_event_get(struct fhost_cntrl_link *link, void *event, int len)
{
    int read;

    read = recv(link->sock_recv, event, len, MSG_DONTWAIT);
    if (read < 0)
        return -1;

    return read;
}

int fhost_cntrl_cfgrwnx_event_discard(struct fhost_cntrl_link *link,
                                      struct cfgrwnx_msg_hdr *msg_hdr)
{
    if (msg_hdr == NULL)
        return -1;

    if (msg_hdr->id == CFGRWNX_SCAN_RESULT_EVENT)
    {
        struct cfgrwnx_scan_result event;
        if (recv(link->sock_recv, &event, sizeof(event), MSG_DONTWAIT) < 0)
            return -1;
        if (event.payload)
            rtos_free(event.payload);
    }
    else if (msg_hdr->id == CFGRWNX_CONNECT_EVENT)
    {
        struct cfgrwnx_connect_event event;
        if (recv(link->sock_recv, &event, sizeof(event), MSG_DONTWAIT) < 0)
            return -1;
        if (event.req_resp_ies)
            rtos_free(event.req_resp_ies);
    }
    else if (msg_hdr->id == CFGRWNX_RX_MGMT_EVENT)
    {
        struct cfgrwnx_rx_mgmt_event event;
        if (recv(link->sock_recv, &event, sizeof(event), MSG_DONTWAIT) < 0)
            return -1;
        if (event.payload)
            rtos_free(event.payload);
    }
    else if (recv(link->sock_recv, msg_hdr, sizeof(*msg_hdr), MSG_DONTWAIT) < 0)
    {
        return -1;
    }

    return 0;
}

int fhost_cntrl_new_peer_candidate(int vif_idx, struct fhost_rx_buf_tag *buf)
{
        struct cfgrwnx_new_peer_candidate_event event;
        struct fhost_vif_tag *fhost_vif = fhost_from_mac_vif(vif_idx);

        if (fhost_vif->conn_sock < 0)
            return -1;

        event.hdr.id = CFGRWNX_NEW_PEER_CANDIDATE_EVENT;
        event.hdr.len = sizeof(event);

        event.fhost_vif_idx = CO_GET_INDEX(fhost_vif, fhost_env.vif);

        event.peer = rtos_malloc(MAC_ADDR_LEN);
        if (event.peer == NULL)
            return -1;
        memcpy(event.peer, (uint8_t *)buf->payload + MAC_HEAD_ADDR2_OFT, MAC_ADDR_LEN);

        event.ie_len = buf->info.vect.frmlen - MAC_BEACON_HDR_LEN;

        event.ies = rtos_malloc(event.ie_len);
        if (event.ies == NULL)
        {
            rtos_free(event.peer);
            return -1;
        }

        memcpy(event.ies, (uint8_t *)buf->payload + MAC_BEACON_HDR_LEN, event.ie_len);

        if (fhost_cntrl_cfgrwnx_event_send(&event.hdr, fhost_vif->conn_sock))
        {
            rtos_free(event.ies);
            rtos_free(event.peer);
        }
        return 0;
}

int fhost_set_listen_itv(uint8_t itv)
{
    if (itv > 20) {
        return -1;
    }

    fhost_params.listen_itv = itv;

    return 0;
}

uint8_t fhost_get_listen_itv(void)
{
    return fhost_params.listen_itv;
}

/**
 * @}
 */
