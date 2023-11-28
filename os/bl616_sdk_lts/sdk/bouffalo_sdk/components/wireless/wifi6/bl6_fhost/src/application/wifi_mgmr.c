#include "wifi_mgmr.h"
#include "wpa.h"
#include "rtos_al.h"
#include "export_struct.h"
#include "fhost_rtos.h"

#if defined(CFG_BL_WIFI_PS_ENABLE) || defined(CFG_WIFI_PDS_RESUME)
#include <bl_lp.h>
#endif

static const struct ieee80211_dot_d country_list[] =
{
    /*First Country is the default country*/
    {
        .code   = "CN",
        .channel_num = 13,
    },
    {
        .code   = "JP",
        .channel_num = 14,
    },
    {
        .code   = "US",
        .channel_num = 11,
    },
    {
        .code   = "EU",
        .channel_num = 13,
    },
};


static int _features_is_set(uint32_t bit)
{
    return (wifiMgmr.features & bit) ? 1 : 0;
}

char *wifi_mgmr_auth_to_str(uint8_t auth)
{
    switch (auth) {
        case WIFI_EVENT_BEACON_IND_AUTH_WPA3_SAE:
        {
            return "WPA3-SAE";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK_WPA3_SAE:
        {
            return "WPA2-PSK/WPA3-SAE";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_OPEN:
        {
            return "Open";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WEP:
        {
            return "WEP";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WPA_PSK:
        {
            return "WPA-PSK";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK:
        {
            return "WPA2-PSK";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_PSK:
        {
            return "WPA2-PSK/WPA-PSK";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_WPA_ENT:
        {
            return "WPA/WPA2-Enterprise";
        }
        break;
        case WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN:
        {
            return "Unknown";
        }
        break;
        default:
        {
            return "Unknown";
        }
    }
}

char *wifi_mgmr_cipher_to_str(uint8_t cipher)
{
    if (cipher == WIFI_EVENT_BEACON_IND_CIPHER_NONE) {
        return "NONE";
    } else if (cipher == WIFI_EVENT_BEACON_IND_CIPHER_WEP) {
        return "WEP";
    } else if (cipher == WIFI_EVENT_BEACON_IND_CIPHER_AES) {
        return "AES";
    } else if (cipher == WIFI_EVENT_BEACON_IND_CIPHER_TKIP) {
        return "TKIP";
    } else if (cipher == WIFI_EVENT_BEACON_IND_CIPHER_TKIP_AES) {
        return "TKIP/AES";
    } else{
        return "Unknown";
    }
}

#if 1
int wifi_mgmr_scan_beacon_save( wifi_mgmr_scan_item_t *scan )
{
#define SCAN_UPDATE_LIMIT_TIME_MS (3000)

    int i, j, pos_empty = -1, ret = 0, flag = 0;
    uint32_t oldest = 0xFFFFFFFF;
    uint32_t counter = 0;
    int8_t lowest_rssi = 0, pos_rssi = -1;
    int8_t pos_oldest = -1;
    int8_t pos_same_ssid = -1;
    wifi_mgmr_scan_item_t tmp_item;

// #ifdef DEBUG_SCAN_BEACON
#if 0
    printf("channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, ppm %d:%d, auth %s, cipher:%s \t, SSID %s\r\n",
            scan->channel,
            MAC_ADDR_LIST(scan->bssid),
            scan->rssi,
            scan->ppm_abs,
            scan->ppm_rel,
            wifi_mgmr_auth_to_str(scan->auth),
            wifi_mgmr_cipher_to_str(scan->cipher),
            scan->ssid
    );
#endif

    rtos_mutex_lock(wifiMgmr.scan_items_lock);
    if (scan->channel > wifiMgmr.channel_nums || !scan->channel){
        ret = -1;
        goto __exit;
    }
    if (0 == scan->ssid[0] && (!_features_is_set(WIFI_MGMR_FEATURES_SCAN_SAVE_HIDDEN_SSID))) {
        ret = -1;
        goto __exit;
    }
    /*update scan_items, we just store the newly found item, or update exsiting one*/
    counter = rtos_now(0);
    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
        if(wifiMgmr.scan_items[i].channel > wifiMgmr.channel_nums){
            memset(&wifiMgmr.scan_items[i], 0, sizeof(wifi_mgmr_scan_item_t));
        }

        if(0 == wifiMgmr.scan_items[i].is_used) {
            pos_empty = i;
            continue;
        }

        /* delete old item */
        if(counter - wifiMgmr.scan_items[i].timestamp_lastseen > SCAN_UPDATE_LIMIT_TIME_MS) {
            // printf("ssid:%s rssi:%d is old\r\n",wifiMgmr.scan_items[i].ssid,wifiMgmr.scan_items[i].rssi);
            memset(&wifiMgmr.scan_items[i], 0, sizeof(wifi_mgmr_scan_item_t));
            pos_empty = i;
            continue;
        }

        if(0 == flag) {
            lowest_rssi = wifiMgmr.scan_items[i].rssi;
            pos_rssi = i;
            oldest = wifiMgmr.scan_items[i].timestamp_lastseen;
            pos_oldest = i;
            flag = 1;
        }

        /* find the weakest rssi. If rssi is samed, update to the older position */
        if(lowest_rssi > wifiMgmr.scan_items[i].rssi) {
            lowest_rssi = wifiMgmr.scan_items[i].rssi;
            pos_rssi = i; 
        }else if(lowest_rssi == wifiMgmr.scan_items[i].rssi && wifiMgmr.scan_items[i].timestamp_lastseen < wifiMgmr.scan_items[pos_rssi].timestamp_lastseen) {
            pos_rssi = i;
        }

        if (0 == memcmp(wifiMgmr.scan_items[i].bssid, scan->bssid, sizeof(scan->bssid)) && 0 == strcmp(scan->ssid, wifiMgmr.scan_items[i].ssid)) {
            pos_same_ssid = i;
        }
    }

    if (i == sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0])) {
        /* The oldest item is deleted if exist. Just need update the same ssid */
        if(-1 != pos_same_ssid) {
            if(scan->rssi >= wifiMgmr.scan_items[pos_same_ssid].rssi){
                wifiMgmr.scan_items[pos_same_ssid].channel = scan->channel;
                wifiMgmr.scan_items[pos_same_ssid].rssi = scan->rssi;
                wifiMgmr.scan_items[pos_same_ssid].ppm_abs = scan->ppm_abs;
                wifiMgmr.scan_items[pos_same_ssid].ppm_rel = scan->ppm_rel;
                wifiMgmr.scan_items[pos_same_ssid].timestamp_lastseen = counter;
                wifiMgmr.scan_items[pos_same_ssid].auth = scan->auth;
                wifiMgmr.scan_items[pos_same_ssid].cipher = scan->cipher;
                wifiMgmr.scan_items[pos_same_ssid].wps = scan->wps;
                wifiMgmr.scan_items[pos_same_ssid].mode = scan->mode;
            }
        }else {
            /* find empty item */
            if(-1 != pos_empty) {
                memset(&wifiMgmr.scan_items[pos_empty], 0, sizeof(wifiMgmr.scan_items[0]));
                strncpy(wifiMgmr.scan_items[pos_empty].ssid, scan->ssid, sizeof(wifiMgmr.scan_items[0].ssid));
                wifiMgmr.scan_items[pos_empty].ssid_tail[0] = '\0';
                wifiMgmr.scan_items[pos_empty].ssid_len = strlen(wifiMgmr.scan_items[pos_empty].ssid);
                memcpy(wifiMgmr.scan_items[pos_empty].bssid, scan->bssid, sizeof(wifiMgmr.scan_items[pos_empty].bssid));
                wifiMgmr.scan_items[pos_empty].channel = scan->channel;
                wifiMgmr.scan_items[pos_empty].rssi = scan->rssi;
                wifiMgmr.scan_items[pos_empty].timestamp_lastseen = counter;
                wifiMgmr.scan_items[pos_empty].auth = scan->auth;
                wifiMgmr.scan_items[pos_empty].cipher = scan->cipher;
                wifiMgmr.scan_items[pos_empty].wps = scan->wps;
                wifiMgmr.scan_items[pos_empty].mode = scan->mode;
                wifiMgmr.scan_items[pos_empty].is_used = 1;
            }else {
                /* if no empty item, copy to low rssi item */
                if(scan->rssi > lowest_rssi) {
                    memset(&wifiMgmr.scan_items[pos_rssi], 0, sizeof(wifiMgmr.scan_items[0]));
                    strncpy(wifiMgmr.scan_items[pos_rssi].ssid, scan->ssid, sizeof(wifiMgmr.scan_items[0].ssid));
                    wifiMgmr.scan_items[pos_rssi].ssid_tail[0] = '\0';
                    wifiMgmr.scan_items[pos_rssi].ssid_len = strlen(wifiMgmr.scan_items[pos_rssi].ssid);
                    memcpy(wifiMgmr.scan_items[pos_rssi].bssid, scan->bssid, sizeof(wifiMgmr.scan_items[pos_rssi].bssid));
                    wifiMgmr.scan_items[pos_rssi].channel = scan->channel;
                    wifiMgmr.scan_items[pos_rssi].rssi = scan->rssi;
                    wifiMgmr.scan_items[pos_rssi].timestamp_lastseen = counter;
                    wifiMgmr.scan_items[pos_rssi].auth = scan->auth;
                    wifiMgmr.scan_items[pos_rssi].cipher = scan->cipher;
                    wifiMgmr.scan_items[pos_rssi].wps = scan->wps;
                    wifiMgmr.scan_items[pos_rssi].mode = scan->mode;
                    wifiMgmr.scan_items[pos_rssi].is_used = 1;
                }
            }
        }
    }

    /* sort scan results according to rssi. The empty item placed in the end of the list*/
    for(i = 0; i < WIFI_MGMR_SCAN_ITEMS_MAX - 1; ++i) {
        for(j = 0; j < WIFI_MGMR_SCAN_ITEMS_MAX-i-1; ++j) {
            if((wifiMgmr.scan_items[j].rssi < wifiMgmr.scan_items[j+1].rssi && wifiMgmr.scan_items[j+1].is_used)
                    || ((!wifiMgmr.scan_items[j].is_used) && (wifiMgmr.scan_items[j+1].is_used))) {
                memcpy(&tmp_item, &wifiMgmr.scan_items[j], sizeof(wifi_mgmr_scan_item_t));
                memcpy(&wifiMgmr.scan_items[j], &wifiMgmr.scan_items[j+1], sizeof(wifi_mgmr_scan_item_t));
                memcpy(&wifiMgmr.scan_items[j+1], &tmp_item, sizeof(wifi_mgmr_scan_item_t));
            }
        }
    }
__exit:
    rtos_mutex_unlock(wifiMgmr.scan_items_lock);

    return ret;
}
#else
int wifi_mgmr_scan_beacon_save( wifi_mgmr_scan_item_t *scan )
{
#define SCAN_UPDATE_LIMIT_TIME_MS (3000)

    int i, empty = -1, oldest = -1, ret = 0;
    uint32_t lastseen = 0xFFFFFFFF;
    uint32_t counter = 0;
    uint32_t lastseen_found = 0;

#ifdef DEBUG_SCAN_BEACON
    printf("channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, ppm %d:%d, auth %s, cipher:%s \t, SSID %s\r\n",
            scan->channel,
            MAC_ADDR_LIST(scan->bssid),
            scan->rssi,
            scan->ppm_abs,
            scan->ppm_rel,
            wifi_mgmr_auth_to_str(scan->auth),
            wifi_mgmr_cipher_to_str(scan->cipher),
            scan->ssid
    );
#endif

    rtos_mutex_lock(wifiMgmr.scan_items_lock);
    if (scan->channel > wifiMgmr.channel_nums || !scan->channel){
        ret = -1;
        goto __exit;
    }
    if (0 == scan->ssid[0] && (!_features_is_set(WIFI_MGMR_FEATURES_SCAN_SAVE_HIDDEN_SSID))) {
        ret = -1;
        goto __exit;
    }
    /*update scan_items, we just store the newly found item, or update exsiting one*/
    counter = rtos_now(0);
    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
        if(wifiMgmr.scan_items[i].channel > wifiMgmr.channel_nums){
            memset(&wifiMgmr.scan_items[i], 0, sizeof(wifi_mgmr_scan_item_t));
            wifiMgmr.scan_items[i].is_used = 0;
        }

        if (wifiMgmr.scan_items[i].is_used) {
            /*track the oldest scan_item*/
            if ((0 == lastseen_found) ||
                ((int32_t)wifiMgmr.scan_items[i].timestamp_lastseen - (int32_t)lastseen < 0)) {
                lastseen_found = 1;
                lastseen = wifiMgmr.scan_items[i].timestamp_lastseen;
                oldest = i;
            }

            /*bssid and ssid must be the same at the same time*/
            if (0 == memcmp(wifiMgmr.scan_items[i].bssid, scan->bssid, sizeof(scan->bssid)) &&
                    0 == strcmp(scan->ssid, wifiMgmr.scan_items[i].ssid)) {

                /*exactly the same scan item found*/
                if ((scan->rssi < wifiMgmr.scan_items[i].rssi) &&
                    ((int32_t)rtos_now(0) - (int32_t)wifiMgmr.scan_items[i].timestamp_lastseen < SCAN_UPDATE_LIMIT_TIME_MS)) {

                    //bl_os_log_debug("skip update %s with rssi %d\r\n", scan->ssid, scan->rssi);

                } else {

                    wifiMgmr.scan_items[i].channel = scan->channel;
                    wifiMgmr.scan_items[i].rssi = scan->rssi;
                    wifiMgmr.scan_items[i].ppm_abs = scan->ppm_abs;
                    wifiMgmr.scan_items[i].ppm_rel = scan->ppm_rel;
                    wifiMgmr.scan_items[i].timestamp_lastseen = counter;
                    wifiMgmr.scan_items[i].auth = scan->auth;
                    wifiMgmr.scan_items[i].cipher = scan->cipher;
                    wifiMgmr.scan_items[i].wps = scan->wps;
                    wifiMgmr.scan_items[i].mode = scan->mode;
                }
                break;
            }
        } else {
            empty  = i;
        }
    }
    if (i == sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0])) {
        /*no valid item found in database, so try to store this newly found*/
        i = (-1 != empty) ? empty : oldest;
        if (-1 != i) {
            memset(&wifiMgmr.scan_items[i], 0, sizeof(wifiMgmr.scan_items[0]));
            strncpy(wifiMgmr.scan_items[i].ssid, scan->ssid, sizeof(wifiMgmr.scan_items[0].ssid));
            wifiMgmr.scan_items[i].ssid_tail[0] = '\0';
            wifiMgmr.scan_items[i].ssid_len = strlen(wifiMgmr.scan_items[i].ssid);
            memcpy(wifiMgmr.scan_items[i].bssid, scan->bssid, sizeof(wifiMgmr.scan_items[i].bssid));
            wifiMgmr.scan_items[i].channel = scan->channel;
            wifiMgmr.scan_items[i].rssi = scan->rssi;
            wifiMgmr.scan_items[i].timestamp_lastseen = counter;
            wifiMgmr.scan_items[i].auth = scan->auth;
            wifiMgmr.scan_items[i].cipher = scan->cipher;
            wifiMgmr.scan_items[i].wps = scan->wps;
            wifiMgmr.scan_items[i].mode = scan->mode;
            wifiMgmr.scan_items[i].is_used = 1;
        }
    }

__exit:
    rtos_mutex_unlock(wifiMgmr.scan_items_lock);

    return ret;
}
#endif

int wifi_mgmr_get_channel_nums(const char *country_code)
{
    int i;
    int channel_num = country_list[0].channel_num; // default value

    for (i = 0; i < sizeof(country_list)/sizeof(country_list[0]); i++) {
        if (0 == strcmp(country_list[i].code, country_code)) {
            channel_num = country_list[i].channel_num;
        }
    }
    return channel_num;
}

int wifi_mgmr_sta_connect_params_get()
{
    uint32_t bl_flags = 0;
    if (wifiMgmr.sta_connect_param.quick_connect)
        bl_flags |= SCAN_STOP_ALL_CHANNEL_IF_TARGET_AP_FOUND|SCAN_STOP_CURRENT_CHANNEL_IF_TARGET_AP_FOUND;
    if (!wifiMgmr.sta_connect_param.scan_mode)
        bl_flags |= SCAN_STOP_ALL_CHANNEL_IF_TARGET_AP_FOUND;
    #if BL_RA_EN
    bl_flags |= LOW_RATE_CONNECT;
    #endif
    return bl_flags;
}

int wifi_mgmr_sta_connect_params_setdefault()
{
    wifiMgmr.sta_connect_param.scan_mode = 1;
    wifiMgmr.sta_connect_param.pmf_cfg = 1;
    return 0;
}

void wifi_mgmr_sta_info_reset()
{
    memset(&wifiMgmr.wifi_mgmr_stat_info, 0xFF, sizeof(wifi_mgmr_connect_ind_stat_info_t));
}

void wifi_mgmr_sta_info_upatestatus(uint16_t status_code, uint16_t reason_code)
{
    wifi_mgmr_connect_ind_stat_info_t *connection_info = &wifiMgmr.wifi_mgmr_stat_info;
    connection_info->status_code = status_code;
    connection_info->reason_code = reason_code;
    //clear connect params
    wifi_mgmr_sta_connect_params_setdefault();
    return;
}

void wifi_mgmr_sta_info_save(void *param)
{
    wifi_mgmr_connect_ind_stat_info_t *connection_info = &wifiMgmr.wifi_mgmr_stat_info;
    struct mac_chan_op chan_op = sm_connect_ind_get_chan(param);
    memcpy(connection_info->bssid, sm_connect_ind_getp_bssid(param), sizeof(connection_info->bssid));
    connection_info->aid = sm_connect_ind_get_aid(param);
    connection_info->chan_band = chan_op.band;
    connection_info->channel = phy_freq_to_channel(chan_op.band, chan_op.prim20_freq);
    connection_info->vif_idx = sm_connect_ind_get_vif_idx(param);
    connection_info->ap_idx = sm_connect_ind_get_ap_idx(param);
    connection_info->ch_idx = sm_connect_ind_get_ch_idx(param);
    connection_info->qos = sm_connect_ind_get_qos(param);
    connection_info->security = sm_connect_ind_get_security(param);
    return;
}

int fhost_application_init()
{
    extern int bl_wifi6_cli_init(void);
    bl_wifi6_cli_init();
    return 0;
}

struct fhost_cntrl_link *cntrl_link;
static RTOS_TASK_FCT(wifi_mgmr_task)
{
    extern struct fhost_cntrl_link *fhost_cntrl_cfgrwnx_link_open_without_sock();
    extern void fhost_wait_ready();
    extern void platform_post_event(int catalogue, int code, ...);

    fhost_wait_ready();
    cntrl_link = fhost_cntrl_cfgrwnx_link_open_without_sock();
    if (cntrl_link == NULL)
    {
        TRACE_APP(ERR, "Failed to init link with control task");
        ASSERT_ERR(0);
    } else {
        platform_post_event(EV_WIFI, CODE_WIFI_ON_MGMR_DONE);
    }
    rtos_task_delete(NULL);
}

rtos_task_handle * const wifi_mgmr_task_handle;
int wifi_mgmr_init(wifi_conf_t *conf)
{
    if (!wifiMgmr.ready) {
        rtos_mutex_create(&wifiMgmr.scan_items_lock);
    }
    wifiMgmr.scan_item_timeout = WIFI_MGMR_CONFIG_SCAN_ITEM_TIMEOUT;
    wifiMgmr.wlan_sta.dhcp_started = 0;
    wifiMgmr.disable_autoreconnect = 1;
    wifiMgmr.max_sta_supported = NX_REMOTE_STA_MAX;
    wifiMgmr.num_sta = 0;
    wifiMgmr.ready = 1;
    wifi_mgmr_set_country_code(conf->country_code);
    wifi_mgmr_sta_connect_params_setdefault();

    if (rtos_task_create(wifi_mgmr_task, "wifi_mgmr", APPLICATION_TASK,
                         FHOST_IPC_STACK_SIZE, NULL, FHOST_IPC_PRIORITY, &wifi_mgmr_task_handle))
        return 0;

    return 1;
}
