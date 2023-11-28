/**
 ****************************************************************************************
 *
 * @file fhost.c
 *
 * @brief Implementation of the fully hosted entry point.
 *
 * Copyright (C) RivieraWaves 2017-2021
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup FHOST
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "fhost.h"
#include "fhost_rx.h"
#include "fhost_tx.h"
#include "fhost_cntrl.h"
#include "fhost_config.h"
#include "fhost_wpa.h"
#include "sys/socket.h"
#include "net_al_ext.h"

/*
 * DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
struct fhost_env_tag fhost_env;

/// Semaphore signaled when all fhost tasks are ready
static rtos_semaphore fhost_ready_sem;

/// Bitfield of task that are not yet fully initialized
static uint32_t fhost_pending_task;
static rtos_mutex fhost_pending_task_lock;

#ifndef offsetof
#define offsetof(st, m) \
    ((size_t)&(((st *)0)->m))
#endif

/*
 * FUNCTIONS
 ****************************************************************************************
 */
int fhost_init()
{
    // no need now for Initialize MAC 

    // Initialize FHOST environment
    memset(&fhost_env, 0, sizeof(fhost_env));

    // Initialize the different modules
    if(rtos_semaphore_create(&fhost_ready_sem, 1, 0))
    {
        ASSERT_ERR(0);
        return 1;
    }

    if (rtos_mutex_create(&fhost_pending_task_lock))
    {
       ASSERT_ERR(0);
       return 1;
    }

    fhost_pending_task = (CO_BIT(IP_TASK)      |
                          CO_BIT(CONTROL_TASK) |
                          CO_BIT(RX_TASK)      |
                          CO_BIT(TX_TASK)      |
                          CO_BIT(WIFI_TASK)    |
                          CO_BIT(SUPPLICANT_TASK));

    // TCP/IP stack
    if (net_init())
    {
        ASSERT_ERR(0);
        return 1;
    }

    // Control task
    if (fhost_cntrl_init())
    {
        ASSERT_ERR(0);
        return 1;
    }

    // RX task
    if (fhost_rx_init())
    {
        ASSERT_ERR(0);
        return 1;
    }

    // TX task
    if (fhost_tx_init())
    {
        ASSERT_ERR(0);
        return 1;
    }

    // Wifi task
    if (rtos_wifi_task_create())
    {
        ASSERT_ERR(0);
        return 1;
    }

    // WPA supplicant environment
    if (fhost_wpa_init())
    {
        ASSERT_ERR(0);
        return 1;
    }

    return 0;
}

void fhost_task_ready(enum rtos_task_id task_id)
{
    rtos_mutex_lock(fhost_pending_task_lock);
    fhost_pending_task &= ~(CO_BIT(task_id));
    rtos_mutex_unlock(fhost_pending_task_lock);

    TRACE_FHOST(TASK_INIT, "%rT task is now initialized", task_id);

    if (fhost_pending_task == 0)
    {
        TRACE_FHOST(TASK_INIT, "All FHOST tasks are initialized");
        rtos_semaphore_signal(fhost_ready_sem, false);
    }
}

int fhost_wait_ready(void)
{
    if (fhost_ready_sem == NULL)
        return -1;

    if (rtos_semaphore_wait(fhost_ready_sem, -1))
        return -1;

    // always re-signal the semaphore in case it is called by several tasks.
    rtos_semaphore_signal(fhost_ready_sem, false);
    return 0;
}

int fhost_vif_name(int fhost_vif_idx, char *name, int len)
{
    if (fhost_vif_idx > NX_VIRT_DEV_MAX)
        return -1;

    return net_if_get_name(fhost_env.vif[fhost_vif_idx].net_if, name, len);
}

int fhost_vif_idx_from_name(const char *name)
{
    net_if_t *net_if;
    int i;

    if (name == NULL)
        return -1;

    net_if = net_if_find_from_name(name);
    if (!net_if)
        return -1;

    for (i = 0 ; i < NX_VIRT_DEV_MAX ; i++)
    {
        if (fhost_env.vif[i].net_if == net_if)
            return i;
    }

    return -1;
}

int fhost_vif_set_uapsd_queues(int fhost_vif_idx, uint8_t uapsd_queues)
{
    if (fhost_vif_idx < 0)
    {
        int i;
        for (i = 0 ; i < NX_VIRT_DEV_MAX ; i++)
        {
            fhost_env.vif[i].uapsd_queues = uapsd_queues;
        }
    }
    else
    {
        if (fhost_vif_idx > NX_VIRT_DEV_MAX)
            return -1;
        fhost_env.vif[fhost_vif_idx].uapsd_queues = uapsd_queues;
    }

    return 0;
}

int fhost_open_loopback_udp_sock(int port)
{
    struct sockaddr_in cntrl;
    struct sockaddr_in wpa;
    int sock;

    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        TRACE_FHOST(ERR, "Failed to create UDP loopback socket");
        return sock;
    }

    cntrl.sin_family = AF_INET;
    cntrl.sin_addr.s_addr = htonl(INADDR_ANY);
    cntrl.sin_port =  htons(0);
    if (bind(sock, (struct sockaddr *)&cntrl, sizeof(cntrl)) < 0)
        goto err;

    wpa.sin_family = AF_INET;
    wpa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    wpa.sin_port = htons(port);
    if (connect(sock, (struct sockaddr *)&wpa, sizeof(wpa)) < 0)
        goto err;

    return sock;

  err:
    TRACE_FHOST(ERR, "Failed to connect UDP loopback socket (port %d)", port);
    close(sock);
    return -1;
}

uint16_t fhost_ip_chksum(const void *dataptr, int len)
{
#if 0
    uint16_t checksum;

    // Try to compute the checksum using the HSU
    if (hsu_ip_checksum(CPU2HW(dataptr), len, &checksum))
        return checksum;
#endif

    // HSU cannot compute for some reason. Use the TCP/IP stack function instead.
    return net_ip_chksum(dataptr, len);
}

void fhost_get_status(struct fhost_status *status)
{
    int i;

    status->vif_max_cnt = NX_VIRT_DEV_MAX;
    status->vif_active_cnt = 0;
    status->vif_first_active = -1;

    for (i = 0 ; i < NX_VIRT_DEV_MAX ; i++)
    {
        if (fhost_env.vif[i].mac_vif != NULL)
        {
            status->vif_active_cnt++;
            if (status->vif_first_active < 0)
                status->vif_first_active = i;
        }
    }

    status->chan_2g4_cnt = fhost_chan.chan2G4_cnt;
    status->chan_2g4 = fhost_chan.chan2G4;

    status->chan_5g_cnt = fhost_chan.chan5G_cnt;
    status->chan_5g = fhost_chan.chan5G;
}

int fhost_get_vif_status(int fvif_idx, struct fhost_vif_status *status)
{
    struct fhost_vif_tag *fhost_vif;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    memset(status, 0, sizeof(*status));

    fhost_vif = &fhost_env.vif[fvif_idx];
    status->index = fvif_idx;
    if (fhost_vif->mac_vif)
    {
        void *mac_vif = fhost_vif->mac_vif;
        status->type = mac_vif_get_type(mac_vif);

        mac_vif_get_channel(mac_vif, &status->chan);
        mac_vif_get_sta_status(mac_vif, &status->sta.bssid, &status->sta.aid, &status->sta.rssi);
    }
    else
    {
        status->type = VIF_UNKNOWN;
    }

    status->mac_addr = (uint8_t *)&fhost_vif->mac_addr;

    return 0;
}


int fhost_set_vif_type(struct fhost_cntrl_link *link, int fvif_idx, enum mac_vif_type type,
                       bool p2p)
{
    struct fhost_vif_tag *fhost_vif;
    enum mac_vif_type prev_type = VIF_UNKNOWN;
    struct net_al_ext_ip_addr_cfg ip_cfg;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    fhost_vif = &fhost_env.vif[fvif_idx];

    if (fhost_vif->mac_vif)
        prev_type = mac_vif_get_type(fhost_vif->mac_vif);

    // Do nothing if interface type is already the requested one
    if (prev_type == type)
        return 0;

    // Close current connection (if any)
    ip_cfg.mode = IP_ADDR_NONE;
    net_al_ext_set_vif_ip(fvif_idx, &ip_cfg);
    fhost_wpa_remove_vif(fvif_idx);
    if (prev_type == VIF_MONITOR)
        fhost_rx_set_monitor_cb(NULL, NULL);

    // Change type of the associated MAC vif
    return fhost_cntrl_set_mac_vif_type(link, fvif_idx, type, p2p);
}

int fhost_configure_vif(struct fhost_cntrl_link *link, int fvif_idx,
                        struct fhost_vif_cfg *cfg)
{
    struct fhost_vif_tag *fhost_vif;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    fhost_vif = &fhost_env.vif[fvif_idx];

    if ((!fhost_vif->mac_vif) || (mac_vif_get_type(fhost_vif->mac_vif) != cfg->cfg_type))
        return -1;

    switch (cfg->cfg_type)
    {
        case VIF_STA:
            return fhost_sta_cfg(fvif_idx, &cfg->sta);
        #if NX_FHOST_MONITOR
        case VIF_MONITOR:
            return fhost_cntrl_monitor_cfg(link, fvif_idx, &cfg->monitor);
        #endif
        #if NX_BEACONING
        case VIF_AP:
            return fhost_ap_cfg(fvif_idx, &cfg->ap, false);
        #endif
        #if RW_MESH_EN
        case VIF_MESH_POINT:
            return fhost_ap_cfg(fvif_idx, &cfg->mesh_point, true);
        #endif

        default:
            break;
    }

    return -1;
}

int fhost_scan(struct fhost_cntrl_link *link, int fvif_idx)
{
    struct cfgrwnx_scan cmd;
    struct cfgrwnx_resp resp;
    struct cfgrwnx_scan_ssid ssid;
#if 0
    struct cfgrwnx_msg_hdr msg_hdr;
#endif
    int nb_result = -1;

    ssid.len = 0;
    ssid.ssid = NULL;
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SCAN_CMD;
    cmd.fhost_vif_idx = fvif_idx;
    cmd.freqs = NULL;
    cmd.chans = NULL;
    cmd.chans_cnt = 0;
    cmd.extra_ies = NULL;
    cmd.bssid = NULL;
    cmd.ssids = &ssid;
    cmd.extra_ies_len = 0;
    cmd.no_cck = 0;
    cmd.ssid_cnt = 1;
    cmd.duration = 0;
    cmd.sock = link->sock_send;
    cmd.hdr.resp_queue = link->queue;
    cmd.passive = false;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SCAN_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
        return -1;

#if 0
    // Wait scan to complete
    while (fhost_cntrl_cfgrwnx_event_peek_header(link, &msg_hdr) == 0)
    {
        if (msg_hdr.id == CFGRWNX_SCAN_DONE_EVENT)
        {
            struct cfgrwnx_scan_completed res;
            if ((fhost_cntrl_cfgrwnx_event_get(link, &res, sizeof(res)) == sizeof(res)) &&
                (res.status == CFGRWNX_SUCCESS))
                nb_result = res.result_cnt;
            break;
        }
        else
        {
            fhost_cntrl_cfgrwnx_event_discard(link, &msg_hdr);
        }
    }

#endif
    return nb_result;
}

int fhost_get_scan_results(struct fhost_cntrl_link *link, int result_idx,
                           int max_nb_result, struct mac_scan_result *results)
{
    struct cfgrwnx_scan_results cmd;
    struct cfgrwnx_scan_results_resp resp;
    struct mac_scan_result *results_ptr = results;
    int nb_res = 0;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SCAN_RESULTS_CMD;
    cmd.hdr.resp_queue = link->queue;
    cmd.idx = result_idx;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SCAN_RESULTS_RESP;

    while ((nb_res < max_nb_result) &&
           !fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) &&
           (resp.status == CFGRWNX_SUCCESS) &&
           resp.scan_result.valid_flag)
    {
        *results_ptr++ = resp.scan_result;
        nb_res++;
        cmd.idx++;
    }

    return nb_res;
}

int fhost_sta_cfg(int fhost_vif_idx, struct fhost_vif_sta_cfg *cfg)
{
    struct fhost_vif_tag *fhost_vif;
    char *cfg_str, *cfg_ssid_str, *cfg_key_str, *ptr, *ssid_ptr, *key_ptr;
    int res, cfg_str_len = 400, cfg_ssid_len = 40, cfg_key_len = 70;
    int key_len;

    if ((fhost_vif_idx >= NX_VIRT_DEV_MAX) || (cfg == NULL))
        return -1;

    fhost_vif = &fhost_env.vif[fhost_vif_idx];

    if ((!fhost_vif->mac_vif) || (mac_vif_get_type(fhost_vif->mac_vif) != VIF_STA))
        return -1;

    cfg_str = rtos_malloc(cfg_str_len + 1);
    cfg_ssid_str = rtos_malloc(cfg_ssid_len + 1);
    cfg_key_str = rtos_malloc(cfg_key_len + 1);
    if (!cfg_str || !cfg_ssid_str || !cfg_key_str)
        return -1;
    memset(cfg_key_str, 0, cfg_key_len + 1);
    ptr = cfg_str;
    ssid_ptr = cfg_ssid_str;
    key_ptr = cfg_key_str;

    // SSID
    res = cfg->ssid.length + 8; // 8 = 'ssid "";'
    if (cfg_ssid_len < res)
        goto end;

    memcpy(cfg_ssid_str, "ssid \"", 6);
    ssid_ptr += 6;
    memcpy(ssid_ptr, cfg->ssid.array, cfg->ssid.length);
    ssid_ptr += cfg->ssid.length;
    *ssid_ptr++ = '"';
    *ssid_ptr++ = 0;

    // AKM
    key_len = strlen(cfg->key);
    if (!cfg->akm)
    {
        if (key_len < 8)
            // If key is less than 8, assume WEP key
            cfg->akm = CO_BIT(MAC_AKM_NONE);
        else
            cfg->akm = CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PSK_SHA256)
                #if NX_CRYPTOLIB
                | CO_BIT(MAC_AKM_SAE);
                #endif
                ;
    }
    else
    {
        // remove unsupported AKM
        uint32_t akm_supported = CO_BIT(MAC_AKM_NONE) | CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PSK_SHA256);
        #if NX_CRYPTOLIB
        akm_supported |= CO_BIT(MAC_AKM_SAE);
        #endif

        // For WEP the user select only MAC_AKM_PRE_RSN
        if (cfg->akm == CO_BIT(MAC_AKM_PRE_RSN))
            cfg->akm = CO_BIT(MAC_AKM_NONE);
        else if (!(cfg->akm & CO_BIT(MAC_AKM_PRE_RSN)))
        {
            // User doesn't allow WPA1 AP
             res = dbg_snprintf(ptr, cfg_str_len, "proto RSN;");
             if (res >= cfg_str_len)
                 goto end;
             ptr += res;
             cfg_str_len -= res;
        }

        cfg->akm &= akm_supported;
        if (cfg->akm == 0)
        {
            res = -1;
            goto end;
        }
    }
    res = 8;
    if (cfg_str_len < res)
        goto end;
    memcpy(ptr, "key_mgmt", res);
    ptr += res;
    cfg_str_len -= res;
    res = fhost_wpa_akm_name(cfg->akm, ptr, cfg_str_len);
    if (res < 0)
        goto end;
    ptr += res;
    cfg_str_len -= res;

    // Cipher suites for WPA
    if (cfg->akm & (CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_SAE)))
    {
        uint32_t cipher_supported = 0;
        uint32_t cipher_pairwise, cipher_group;

        if (inline_nxmac_tkip_getf())
            cipher_supported |= CO_BIT(MAC_CIPHER_TKIP);
        if (inline_nxmac_ccmp_getf())
        {
            cipher_supported |= CO_BIT(MAC_CIPHER_CCMP);
            #if NX_MAC_HE
            cipher_supported |= CO_BIT(MAC_CIPHER_CCMP_256);
            #endif
        }
        #if NX_MAC_HE
        if (inline_nxmac_gcmp_getf())
            cipher_supported |= CO_BIT(MAC_CIPHER_GCMP_128) | CO_BIT(MAC_CIPHER_GCMP_256);
        #endif

        if (cfg->unicast_cipher)
            cipher_pairwise = cfg->unicast_cipher & cipher_supported;
        else
            cipher_pairwise = cipher_supported;

        if (cfg->group_cipher)
            cipher_group = cfg->group_cipher & cipher_supported;
        else
            cipher_group = cipher_supported;


        if (!cipher_pairwise || !cipher_group)
        {
            res = -1;
            goto end;
        }

        // By default wpa_supplicant enable TKIP and CCMP. If we support something else need to
        // configure wpa_supplicant accordingly
        if (cipher_pairwise != (CO_BIT(MAC_CIPHER_TKIP) | CO_BIT(MAC_CIPHER_CCMP)))
        {
            res = 8;
            if (cfg_str_len < res)
                goto end;
            memcpy(ptr, "pairwise", res);
            ptr += res;
            cfg_str_len -= res;
            res = fhost_wpa_cipher_name(cipher_pairwise, ptr, cfg_str_len);
            if (res < 0)
                goto end;
            ptr += res;
            cfg_str_len -= res;
        }

        if (cipher_group != (CO_BIT(MAC_CIPHER_TKIP) | CO_BIT(MAC_CIPHER_CCMP)))
        {
            res = 5;
            if (cfg_str_len < res)
                goto end;
            memcpy(ptr, "group", res);
            ptr += res;
            cfg_str_len -= res;
            res = fhost_wpa_cipher_name(cipher_group, ptr, cfg_str_len);
            if (res < 0)
                goto end;
            ptr += res;
            cfg_str_len -= res;
        }
    }

    // Keys
    if (key_len > 0)
    {
        if (cfg->akm & CO_BIT(MAC_AKM_NONE))
        {
            // WEP keys
            res = dbg_snprintf(ptr, cfg_str_len, "wep_key0 %s;auth_alg OPEN SHARED;", cfg->key);
        }
        else
        {
            // PSK (works also for SAE)
            if (key_len == 64) {
                //support for psk setting
                res = dbg_snprintf(key_ptr, cfg_key_len, "psk %s", cfg->key);
            } else {
                res = dbg_snprintf(key_ptr, cfg_key_len, "psk \"%s\"", cfg->key);
            }
            if (res > cfg_key_len)
                goto end;
            key_ptr += res;
            *key_ptr++ = 0;;
            res = 0;
        }

        if (res >= cfg_str_len)
            goto end;

        ptr += res;
        cfg_str_len -= res;

        #if NX_MFP
        if (!(cfg->akm & CO_BIT(MAC_AKM_NONE))) {
            res = dbg_snprintf(ptr, cfg_str_len, "ieee80211w %d;", cfg->pmf);
            if (res >= cfg_str_len)
                goto end;

            ptr += res;
            cfg_str_len -= res;
        }
        #endif
    }

    res = dbg_snprintf(ptr, cfg_str_len, "scan_ssid 1;");
    if (res >= cfg_str_len)
        goto end;

    ptr += res;
    cfg_str_len -= res;


    // BSSID (optional)
    if (cfg->bssid.array[0] || cfg->bssid.array[1] || cfg->bssid.array[2])
    {
        res = dbg_snprintf(ptr, cfg_str_len, "bssid %02x:%02x:%02x:%02x:%02x:%02x;",
                           ((uint8_t *)cfg->bssid.array)[0], ((uint8_t *)cfg->bssid.array)[1],
                           ((uint8_t *)cfg->bssid.array)[2], ((uint8_t *)cfg->bssid.array)[3],
                           ((uint8_t *)cfg->bssid.array)[4], ((uint8_t *)cfg->bssid.array)[5]);
        if (res >= cfg_str_len)
            goto end;

        ptr += res;
        cfg_str_len -= res;
    }

    // Frequencies (optional)
    if (cfg->freq[0])
    {
        unsigned int i, j;

        // silently remove invalid frequencies
        for (i = 0, j = 0; i < CO_ARRAY_SIZE(cfg->freq); i++)
        {
            if (cfg->freq[i] == 0)
                break;
            if (fhost_chan_get(cfg->freq[i]))
            {
                if (j != i)
                    cfg->freq[j] = cfg->freq[i];
                j++;
            }
        }

        if (j > 0)
        {
            res = dbg_snprintf(ptr, cfg_str_len, "scan_freq ");
            if (res >= cfg_str_len)
                goto end;
            ptr += res;
            cfg_str_len -= res;

            while (j > 0)
            {
                j--;
                res = dbg_snprintf(ptr, cfg_str_len, "%d ", cfg->freq[j]);
                if (res >= cfg_str_len)
                    goto end;
                ptr += res;
                cfg_str_len -= res;
            }

            res = dbg_snprintf(ptr, cfg_str_len, ";");
            if (res >= cfg_str_len)
                goto end;
            ptr += res;
            cfg_str_len -= res;
        }
    }

    res = fhost_wpa_create_network(fhost_vif_idx, cfg_str, cfg_ssid_str, cfg_key_str, true, cfg->timeout_ms);

  end:
    if (res > 0)
    {
        TRACE_FHOST(ERR, "Missing at least %d character for wpa_supplicant config",
                    res - cfg_str_len);
        res = -1;
    }
    rtos_free(cfg_str);
    rtos_free(cfg_ssid_str);
    rtos_free(cfg_key_str);
    return res;
}

int fhost_get_sta_idx(struct fhost_vif_tag *fhost_vif, uint8_t table_max_size,
                      uint8_t *idx_table)
{
    uint8_t size = 0, i = 0;
    struct vif_info_tag *vif = fhost_vif->mac_vif;
    void *sta = (void *)co_list_pick(mac_vif_get_sta_list(vif));

    while (sta != NULL)
    {
        if (i >= table_max_size)
            break;

        idx_table[i++] = sta_get_staid(sta);
        sta = sta_get_list_next(sta);
    }
    size = i;
    return size;
}

#if NX_BEACONING
int fhost_ap_cfg(int fhost_vif_idx, struct fhost_vif_ap_cfg *cfg, bool mesh_mode)
{
    struct fhost_vif_tag *fhost_vif;
    struct mac_chan_def *chan = NULL;
    char *cfg_str, *cfg_ssid_str, *cfg_key_str, *ptr, *ssid_ptr, *key_ptr;
    int res, cfg_str_len = 300, cfg_ssid_len = 40, cfg_key_len = 70;
    int key_len;
    uint32_t akm;
    int timeout;

    if (mesh_mode)
        timeout = 3000;
    else
        timeout = 500;

    if ((fhost_vif_idx >= NX_VIRT_DEV_MAX) || (cfg == NULL))
        return -1;

    fhost_vif = &fhost_env.vif[fhost_vif_idx];

    if ((!fhost_vif->mac_vif) || ((mac_vif_get_type(fhost_vif->mac_vif) != VIF_AP) && !mesh_mode))
        return -1;

    cfg_str = rtos_malloc(cfg_str_len + 1);
    cfg_ssid_str = rtos_malloc(cfg_ssid_len + 1);
    cfg_key_str = rtos_malloc(cfg_key_len + 1);
    if (!cfg_str || !cfg_ssid_str || !cfg_key_str)
        return -1;
    memset(cfg_key_str, 0, cfg_key_len + 1);
    ptr = cfg_str;
    ssid_ptr = cfg_ssid_str;
    key_ptr = cfg_key_str;

    if (mesh_mode)
        // Enable MESH mode
        res = dbg_snprintf(ptr, cfg_str_len, "mode 5;");
    else
        // Enable AP mode
        res = dbg_snprintf(ptr, cfg_str_len, "mode 2;");
    ptr += res;
    cfg_str_len -= res;

    // SSID
    res = cfg->ssid.length + 8; // 8 = 'ssid "";'
    if (cfg_ssid_len < res)
        goto end;

    memcpy(cfg_ssid_str, "ssid \"", 6);
    ssid_ptr += 6;
    memcpy(ssid_ptr, cfg->ssid.array, cfg->ssid.length);
    ssid_ptr += cfg->ssid.length;
    *ssid_ptr++ = '"';
    *ssid_ptr++ = 0;

    // Operating Channel and Mode
    chan = fhost_chan_get(cfg->chan.prim20_freq);
    if (!chan || (chan->flags & (CHAN_NO_IR | CHAN_DISABLED | CHAN_RADAR)))
        goto end;

    res = dbg_snprintf(ptr, cfg_str_len, "frequency %d;", cfg->chan.prim20_freq);
    if (res >= cfg_str_len)
        goto end;
    ptr += res;
    cfg_str_len -= res;

    if (cfg->mode == FHOST_AP_NON_HT)
    {
        res = dbg_snprintf(ptr, cfg_str_len, "ht 0;");
        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
    }
    else
    {
        int freq_offset = cfg->chan.center1_freq - cfg->chan.prim20_freq;
        int vht = 0, he = 0, ht40 = 0, chwidth = 0;

        #if NX_VHT
        if (phy_vht_supported() &&
            ((cfg->mode == FHOST_AP_AUTO) || (cfg->mode >= FHOST_AP_VHT)) &&
            (cfg->chan.band == PHY_BAND_5G))
            vht = 1;
        #endif

        #if NX_HE
        if (phy_he_supported() &&
            ((cfg->mode == FHOST_AP_AUTO) || (cfg->mode == FHOST_AP_HE)))
            he = 1;
        #endif

        if (cfg->chan.type == PHY_CHNL_BW_20)
        {
            if (freq_offset != 0)
                goto end;
        }
        else if (cfg->chan.type == PHY_CHNL_BW_40)
        {
            if (phy_get_bw() < PHY_CHNL_BW_40)
                goto end;

            if (freq_offset == -10)
                ht40 = -1;
            else if (freq_offset == 10)
                ht40 = 1;
            else
                goto end;
        }
        #if NX_VHT
        else if (cfg->chan.type == PHY_CHNL_BW_80)
        {
            int freq_offset = cfg->chan.center1_freq - cfg->chan.prim20_freq;

            if (phy_get_bw() < PHY_CHNL_BW_80)
                goto end;

            if (cfg->mode == FHOST_AP_HT)
                goto end;

            if ((freq_offset == 10) || (freq_offset == -30))
                ht40 = -1;
            else if ((freq_offset == -10) || (freq_offset == 30))
                ht40 = 1;
            else
                goto end;

            chwidth = 1;
        }
        #endif //NX_VHT
        else
        {
            // not supported
            goto end;
        }

        if (ht40)
            // time for ht40 overlap scan, fix wap timeout so increase 1000
            timeout += 2500;

        res = dbg_snprintf(ptr, cfg_str_len, "vht %d;he %d;ht40 %d;max_oper_chwidth %d;vht_center_freq1 %d;vht_center_freq2 %d;",
                           vht, he, ht40, chwidth, cfg->chan.center1_freq, cfg->chan.center2_freq);
        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
    }

    // Beacon
    if (cfg->bcn_interval == 0)
        cfg->bcn_interval = 100;
    else if (cfg->bcn_interval < 15)
        cfg->bcn_interval = 15;
    if (cfg->dtim_period < 1)
        cfg->dtim_period = 1;
    res = dbg_snprintf(ptr, cfg_str_len, "beacon_int %d;dtim_period %d;",
                       cfg->bcn_interval, cfg->dtim_period);
    if (res >= cfg_str_len)
        goto end;
    ptr += res;
    cfg_str_len -= res;

    // AKM (remove unsupported ones)
    akm = cfg->akm & (CO_BIT(MAC_AKM_PSK) |
                      CO_BIT(MAC_AKM_PRE_RSN) |
                      #if NX_CRYPTOLIB
                      CO_BIT(MAC_AKM_SAE) |
                      #endif
                      CO_BIT(MAC_AKM_NONE));
    if (!akm)
        goto end;

    key_len = strlen(cfg->key);
    if (akm & CO_BIT(MAC_AKM_NONE))
    {
        if (cfg->akm & ~CO_BIT(MAC_AKM_NONE))
            goto end;
    }
    else if (akm & CO_BIT(MAC_AKM_PRE_RSN))
    {
        if (akm & CO_BIT(MAC_AKM_PSK))
        {
            if (key_len < 8)
                goto end;

            akm = CO_BIT(MAC_AKM_PSK);
            cfg->unicast_cipher = CO_BIT(MAC_CIPHER_TKIP);
            // WEP is no longer allowed for group cipher so always use TKIP
            cfg->group_cipher =  CO_BIT(MAC_CIPHER_TKIP);
            res = dbg_snprintf(ptr, cfg_str_len, "proto WPA;");
        }
        else if ((key_len == 5) || (key_len == 13))
        {
            akm = CO_BIT(MAC_AKM_NONE);
            res = dbg_snprintf(ptr, cfg_str_len, "wep_key0 \"%s\";wep_tx_keyidx 0;",
                               cfg->key);
        }
        else
            goto end;

        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
    }
    else
    {
        if (key_len < 8)
            goto end;
        res = dbg_snprintf(ptr, cfg_str_len, "proto RSN;");
        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
    }
    res = 8;
    if (cfg_str_len < res)
        goto end;
    memcpy(ptr, "key_mgmt", res);
    ptr += res;
    cfg_str_len -= res;
    res = fhost_wpa_akm_name(akm, ptr, cfg_str_len);
    if (res < 0)
        goto end;
    ptr += res;
    cfg_str_len -= res;

    // WPAx config (cipher, PSK, MFP)
    if (akm & (CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_SAE)))
    {
        uint32_t group, pairwise;
        uint32_t cipher_supported = 0;

        if (inline_nxmac_tkip_getf())
            cipher_supported |= CO_BIT(MAC_CIPHER_TKIP);
        if (inline_nxmac_ccmp_getf())
        {
            cipher_supported |= CO_BIT(MAC_CIPHER_CCMP);
            #if NX_MAC_HE
            cipher_supported |= CO_BIT(MAC_CIPHER_CCMP_256);
            #endif
        }
        #if NX_MAC_HE
        if (inline_nxmac_gcmp_getf())
            cipher_supported |= CO_BIT(MAC_CIPHER_GCMP_128) | CO_BIT(MAC_CIPHER_GCMP_256);
        #endif

        // Remove unsupported cipher or set default value if not set
        if (cfg->unicast_cipher)
        {
            pairwise = cfg->unicast_cipher & cipher_supported;
            if (!pairwise)
                goto end;
        }
        else
            pairwise = CO_BIT(MAC_CIPHER_CCMP);

        res = 8;
        if (cfg_str_len < res)
            goto end;
        memcpy(ptr, "pairwise", res);
        ptr += res;
        cfg_str_len -= res;
        res = fhost_wpa_cipher_name(pairwise, ptr, cfg_str_len);
        if (res < 0)
            goto end;
        ptr += res;
        cfg_str_len -= res;

        if (cfg->group_cipher)
        {
            group = cfg->group_cipher & cipher_supported;
            if (!group)
                goto end;
        }
        else
            group = CO_BIT(MAC_CIPHER_CCMP);

        res = 5;
        if (cfg_str_len < res)
            goto end;
        memcpy(ptr, "group", res);
        ptr += res;
        cfg_str_len -= res;
        res = fhost_wpa_cipher_name(group, ptr, cfg_str_len);
        if (res < 0)
            goto end;
        ptr += res;
        cfg_str_len -= res;

        res = dbg_snprintf(key_ptr, cfg_key_len, "psk \"%s\";", cfg->key);
        if (res > cfg_key_len)
            goto end;
        key_ptr += res;
        *key_ptr++ = 0;

        #if NX_MFP
        if (cfg->mfp < 0 || cfg->mfp > 2)
            goto end;

        res = dbg_snprintf(ptr, cfg_str_len, "ieee80211w %d;", cfg->mfp);
        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
        #endif
    }

    if (mesh_mode)
    {
        int i = 0;

        if(cfg->basic_rates[i] != 0)
        {
           res = 16;
           if (cfg_str_len < res)
               goto end;
           memcpy(ptr, "mesh_basic_rates", res);
           ptr += res;
           cfg_str_len -= res;

           res = dbg_snprintf(ptr, cfg_str_len," %d", cfg->basic_rates[i]);
           if (res >= cfg_str_len)
               goto end;
           ptr += res;
           cfg_str_len -= res;

           while(cfg->basic_rates[++i] != 0 && (i < MAC_RATESET_LEN))
           {
               res = dbg_snprintf(ptr, cfg_str_len," %d", cfg->basic_rates[i]);
               if (res >= cfg_str_len)
                   goto end;
               ptr += res;
               cfg_str_len -= res;
           }
           res = 1;
           if (cfg_str_len < res)
               goto end;
           memcpy(ptr, ";", res);
           ptr++;
           cfg_str_len--;
        }
    }

    // Extra user config
    if (cfg->extra_cfg)
    {
        res = dbg_snprintf(ptr, cfg_str_len, "%s", cfg->extra_cfg);
        if (res >= cfg_str_len)
            goto end;
        ptr += res;
        cfg_str_len -= res;
    }

    if (mesh_mode)
        res = fhost_wpa_create_network(fhost_vif_idx, cfg_str, cfg_ssid_str, cfg_key_str, true, timeout);
    else
        res = (fhost_wpa_create_network(fhost_vif_idx, cfg_str, cfg_ssid_str, cfg_key_str, false, 0) |
               fhost_wpa_execute_cmd(fhost_vif_idx, NULL, 0, 300, "AP_SCAN 2") |
               fhost_wpa_enable_network(fhost_vif_idx, timeout));

    if (res)
        fhost_wpa_remove_vif(fhost_vif_idx);

    // Isolation Mode
    fhost_vif->isolation_mode = cfg->isolation_mode;

  end:
    if (res > 0)
    {
        if (res >= cfg_str_len)
        {
            TRACE_FHOST(ERR, "Missing at least %d character for wpa_supplicant config (AP)",
                        res - cfg_str_len);
        }
        else
        {
            TRACE_FHOST(ERR, "Invalid AP config: chan_freq=%d chan_flags=%x "
                        "akm=%08lx unicast=%08lx group=%08lx key_len=%d",
                        cfg->chan.prim20_freq, (chan) ? chan->flags : 0xffff,
                        TR_32(cfg->akm), TR_32(cfg->unicast_cipher),
                        TR_32(cfg->group_cipher), strlen(cfg->key));
            TRACE_BUF_FHOST(ERR, "Config before error: %pBs", ptr - cfg_str, cfg_str);
        }
        res = -1;
    }
    rtos_free(cfg_str);
    rtos_free(cfg_ssid_str);
    rtos_free(cfg_key_str);
    return res;
}
#endif

#if NX_FTM_INITIATOR
int fhost_ftm(struct fhost_cntrl_link *link, int fvif_idx,
              struct mac_ftm_results *res)
{
    struct cfgrwnx_ftm_start_cmd cmd;
    struct cfgrwnx_resp resp;
    struct cfgrwnx_msg_hdr msg_hdr;
    struct cfgrwnx_ftm_done_event event;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_FTM_START_CMD;
    cmd.hdr.resp_queue = link->queue;
    cmd.sock = link->sock_send;
    cmd.fhost_vif_idx = fvif_idx;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_FTM_START_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
        return -1;

    // Wait FTM measurements to complete
    while (fhost_cntrl_cfgrwnx_event_peek_header(link, &msg_hdr) == 0)
    {
        if (msg_hdr.id == CFGRWNX_FTM_DONE_EVENT)
        {
            if (fhost_cntrl_cfgrwnx_event_get(link, &event, sizeof(event)) != sizeof(event))
                return -1;
            break;
        }
        else
        {
            fhost_cntrl_cfgrwnx_event_discard(link, &msg_hdr);
        }
    }

    // Copy results
    *res = event.results;

    return 0;
}
#endif // NX_FTM_INITIATOR

/// @}

