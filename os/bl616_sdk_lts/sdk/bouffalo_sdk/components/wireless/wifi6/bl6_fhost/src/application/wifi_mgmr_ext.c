#include <fhost_connect.h>
#include <fhost_wpa.h>
#include "fhost_config.h"
#include "fhost_cntrl.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"
#include "bl_fw_api.h"

#include "platform_al.h"
#include "rtos_al.h"
#include "net_al_ext.h"

#if defined(CFG_BL_WIFI_PS_ENABLE) || defined(CFG_WIFI_PDS_RESUME)
#include <bl_lp.h>
#endif

#ifdef IOT_SDK_ADAPTER
#include "iot_sdk_adapter.h"
#endif

/*just limit packet len with a reasonable value*/
#define LEN_PKT_RAW_80211   (480)

static void str_upper(char *str, char *stop)
{
    char *ptr = str;
    char c;

    if (stop)
    {
        c = *stop;
        *stop = 0;
    }

    while (*ptr)
    {
        if ((*ptr >= 'a') && (*ptr <= 'z'))
            *ptr -= 'a' - 'A';
        ptr++;
    }

    if (stop)
        *stop = c;
}

int fhost_read_akm(char *akm_str, uint32_t *akm, bool show_error)
{
    char *next_akm, *cur_akm = akm_str;
    int ret = 0;

    str_upper(akm_str, NULL);
    next_akm = strchr(cur_akm, ',');

    *akm = 0;
    while (cur_akm)
    {
        if (strncmp(cur_akm, "OPEN", 4) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_NONE);
        }
        else if (strncmp(cur_akm, "WEP", 3) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_PRE_RSN);
        }
        else if ((strncmp(cur_akm, "RSN", 3) == 0) ||
                 (strncmp(cur_akm, "WPA2", 4) == 0))
        {
            *akm |= CO_BIT(MAC_AKM_PSK) | CO_BIT(MAC_AKM_PSK_SHA256);
        }
        else if ((strncmp(cur_akm, "SAE", 3) == 0) ||
                 (strncmp(cur_akm, "WPA3", 4) == 0))
        {
            *akm |= CO_BIT(MAC_AKM_SAE);
        }
        else if (strncmp(cur_akm, "WPA", 4) == 0)
        {
            *akm |= CO_BIT(MAC_AKM_PRE_RSN) | CO_BIT(MAC_AKM_PSK);
        }
        else
        {
            ret++;
        }

        cur_akm = next_akm;
        if (cur_akm)
        {
            cur_akm++; //skip coma
            next_akm = strchr(cur_akm, ',');
        }
    }

    if (ret && show_error)
    {
        fhost_print(RTOS_TASK_NULL, "[%s] Only the following AKM are supported:\n"
                    "OPEN: For open AP\n"
#if 0 // not supported now
                    "WEP: For AP with WEP security\n"
#endif
                    "WPA: For AP with WPA/PSK security (pre WPA2)\n"
                    "RSN (or WPA2): For AP with WPA2/PSK security\n"
                    "SAE (or WPA3): For AP with WPA3/PSK security\n"
                    , akm_str);
    }
    return ret;
}

int fhost_search_itf(char *name)
{
    int fhost_vif_idx = fhost_vif_idx_from_name(name);

    if (fhost_vif_idx >= 0)
        return fhost_vif_idx;

    fhost_print(RTOS_TASK_NULL, "Cannot find interface %s\n", name);
    return -1;
}

int fhost_ipc_parse_mac_addr(char *str, struct mac_addr *addr)
{
    char *ptr = str;
    uint32_t i;

    if (!str || strlen(str) < 17 || !addr)
        return -1;

    for (i = 0 ; i < 6 ; i++)
    {
        char *next;
        long int hex = strtol(ptr, &next, 16);
        if (((unsigned)hex > 255) || ((hex == 0) && (next == ptr)) ||
            ((i < 5) && (*next != ':')) ||
            ((i == 5) && (*next != '\0')))
            return -1;

        ((uint8_t *)addr)[i] = (uint8_t)hex;
        ptr = ++next;
    }

    return 0;
}

/// Power Save mode setting
enum
{
    /// Power-save off
    MGMR_PS_MODE_OFF,
    /// Power-save on - Normal mode
    MGMR_PS_MODE_ON,
    /// Power-save on - Dynamic mode
    MGMR_PS_MODE_ON_DYN,
};

wifi_mgmr_t wifiMgmr;

extern int fhost_ipc_wpa_cli_api(int fhost_vif_idx, char *cmd, char **resp_result);
extern int fhost_ipc_wpa_cli_api_free(char *resp_result);

static int check_wifi_ready(void)
{
    if (0 == wifiMgmr.ready) {
        printf("Run WifiMgmr init first...\r\n");
        return -1;
    }

    return 0;
}

char *wifi_mgmr_mode_to_str(uint32_t mode)
{
    switch (mode)
    {
    case (WIFI_MODE_802_11B):
        return "B";
    case (WIFI_MODE_802_11B | WIFI_MODE_802_11G):
        return "BG";
    case (WIFI_MODE_802_11B | WIFI_MODE_802_11G | WIFI_MODE_802_11N_2_4):
        return "BGN";
    default:
        return "Unknown";
    }
}

static int get_key_value(const char *str, const char *key, char *value, int *len)
{
    int chr, key_len, counter = 0;

    key_len = strlen(key);
#if 0
    puts("************************************************\r\n");
    printf("key_len:%d, for key ==>> %s <<== to parse string:\r\n", key_len, key);
    puts(str);
    puts("================================================\r\n");
#endif
next_line_read:
    //should be the key of key-value pair now
    if (0 == strncmp(str, key, key_len) && '=' == *(str + key_len)) {//XXX memory boundary check
        while ((chr = *(str + key_len + 1)) && '\n' != chr) {
            *(value++) = chr;
            counter ++;
            str++;
        }
        *value = '\n';
        *len = counter + 1;
        return 0;
    }

    while ((chr = *(str++)) && '\r' != chr && '\n' != chr) {
        /*seek to the end of line*/
    }
    if ('\0' == chr) {
        goto end;
    }

    goto next_line_read;

end:
    return -1;
}

/*get value of key string from wpa_cli cmd:status*/
static int _fhost_ipc_get_wpa_status(char *key, char *res_str, int *res_str_buf_size)
{
    char *resp_result;
    int ret = 0;

    resp_result = NULL;
    memset(res_str, 0, *res_str_buf_size);

    ret = fhost_ipc_wpa_cli_api(-1, "STATUS", &resp_result);
    if (NULL == resp_result) {
        /*no result is returned*/
        return -1;
    }

    printf("Ret:%d, Result dump:\r\n%s\r\n", ret, resp_result);
    if (get_key_value(resp_result, key, res_str, res_str_buf_size)) {
        ret = -1;
        goto exit;
    }
    printf("Get %d:%s: %s\r\n", *res_str_buf_size, key, res_str);

exit:
    if (resp_result) {
       fhost_ipc_wpa_cli_api_free(resp_result);
    }
    return -1;
}

static int _fhost_ipc_check_wpa_state(const char *state)
{
    char res_str[20];
    int res_str_buf_size = 20;
    memset(res_str, 0 ,res_str_buf_size);

    if (!_fhost_ipc_get_wpa_status("wpa_state", res_str, &res_str_buf_size)) {
        return -1;
    }

    printf("WPA_State:%s\r\n", res_str);

    return strncmp(state, res_str, strlen(state));
}


void show_auth_cipher(struct mac_scan_result *result)
{
    bool sep = false;
    uint16_t mfp;
    int i;

    if (result->akm & CO_BIT(MAC_AKM_NONE)) {
        printf(" [OPEN]\n");
         return;
    }

    if (result->akm == CO_BIT(MAC_AKM_PRE_RSN)) {
        printf(" [WEP]\n");
        return;
    }

    if (result->akm & CO_BIT(MAC_AKM_WAPI_CERT) ||
        result->akm & CO_BIT(MAC_AKM_WAPI_PSK)) {
        bool cert = result->akm & CO_BIT(MAC_AKM_WAPI_CERT);
        bool psk = result->akm & CO_BIT(MAC_AKM_WAPI_PSK);
        printf("[WAPI:%s%s%s %s/%s]\n",
                    cert ? "CERT" : "",
                    cert && psk ? "-" : "",
                    psk ? "PSK" : "",
                    result->group_cipher == CO_BIT(MAC_CIPHER_WPI_SMS4) ? "SMS4" : "?",
                    result->pairwise_cipher == CO_BIT(MAC_CIPHER_WPI_SMS4) ? "SMS4" : "?");
        return;
    }

    if (result->akm & CO_BIT(MAC_AKM_PRE_RSN))
        printf(" [WPA:");
    else
        printf(" [RSN:");

    for (i = MAC_AKM_8021X ; i <= MAC_AKM_OWE ; i++)
    {
        if (result->akm & CO_BIT(i))
        {
            fhost_ipc_print_akm(i, sep);
            sep = true;
        }
    }

    printf(" ");
    sep = false;
    for (i = MAC_CIPHER_WEP40 ; i <= MAC_CIPHER_BIP_CMAC_256 ; i++)
    {
        if (result->pairwise_cipher & CO_BIT(i))
        {
            fhost_ipc_print_cipher(i, sep);
            sep = true;
        }
    }
    printf("/");

    // remove MFP cipher from group
    mfp = (CO_BIT(MAC_CIPHER_BIP_CMAC_128) | CO_BIT(MAC_CIPHER_BIP_GMAC_128) |
           CO_BIT(MAC_CIPHER_BIP_GMAC_256) | CO_BIT(MAC_CIPHER_BIP_CMAC_256));

    mfp = result->group_cipher & mfp;
    result->group_cipher &= ~(mfp);

    if (result->group_cipher)
        fhost_ipc_print_cipher(31 - co_clz(result->group_cipher), false);
    else
        printf("?");

    if (mfp)
    {
        printf("][MFP:");
        sep = false;
        for (i = MAC_CIPHER_BIP_CMAC_128 ; i <= MAC_CIPHER_BIP_CMAC_256 ; i++)
        {
              if (mfp & CO_BIT(i))
              {
                  fhost_ipc_print_cipher(i, sep);
                  sep = true;
              }
        }
    }

    printf("]\n");

}

int wifi_sta_connect(char *ssid, char *key, char *bssid, char *akm_str, uint8_t pmf_cfg, uint16_t freq1, uint16_t freq2, uint8_t use_dhcp)
{
    struct fhost_ipc_connect_cfg cfg;
    size_t ssid_len = ssid ? strlen(ssid) : 0;
    size_t key_len = key ? strlen(key) : 0;

    if (check_wifi_ready())
        return -1;

    memset(&cfg, 0, sizeof(cfg));

    cfg.fhost_vif_idx = MGMR_VIF_STA;

    // timeout for connnect, default 200s (this var will impact auto connect when scan an unexist ssid)
    cfg.cfg.timeout_ms = 200000;

    if (ssid_len > sizeof(cfg.cfg.ssid.array))
    {
        printf("Invalid SSID\r\n");
        return -1;
    }
    wifi_mgmr_sta_info_reset();
    memcpy(cfg.cfg.ssid.array, ssid, ssid_len);
    wifi_mgmr_sta_ssid_set(ssid);
    cfg.cfg.ssid.length = ssid_len;

    fhost_ipc_parse_mac_addr(bssid, &cfg.cfg.bssid);

    if ((key_len + 1) > sizeof(cfg.cfg.key))
    {
        printf("Invalid Key\r\n");
        return -1;
    }
    if (key != NULL) {
        strcpy(cfg.cfg.key, key);
        wifi_mgmr_sta_passphr_set(key);
    }

    cfg.cfg.pmf = pmf_cfg;
    cfg.cfg.freq[0] = freq1;
    cfg.cfg.freq[1] = freq2;
    wifiMgmr.sta_connect_param.use_dhcp = use_dhcp;

    if (akm_str != NULL) {
        fhost_read_akm(akm_str, &cfg.cfg.akm, true);
    }

    if (cfg.cfg.ssid.length == 0) {
        printf("cfg.cfg.ssid.length == 0 \r\n");
        return -1;
    }

    cfg.link = cntrl_link;

    printf("start connecting ... \r\n");

    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 1);

    fhost_ipc_connect_task_start(&cfg);

    return 0;
}

int wifi_mgmr_sta_quickconnect(char *ssid, char *key, uint16_t freq1, uint16_t freq2)
{
    wifiMgmr.sta_connect_param.quick_connect = 1;
    wifi_sta_connect(ssid, key, NULL, NULL, 1, freq1, freq2, 1);
    return 0;
}

int wifi_mgmr_sta_connect(const wifi_mgmr_sta_connect_params_t *config)
{
    printf("wifi_mgmr_sta_connect: ssid:<%s>/<%d> key:<%s>/<%d> akm:<%s>/<%d> %d-%d-%d-%d\r\n",
            config->ssid,config->ssid_len,config->key,config->key_len,config->akm_str,config->akm_len,config->pmf_cfg,config->freq1,config->freq2,config->use_dhcp);
    struct fhost_ipc_connect_cfg cfg;
    size_t ssid_len = config->ssid_len;
    size_t key_len = config->key_len;

    if (check_wifi_ready())
        return -1;

    memset(&cfg, 0, sizeof(cfg));

    cfg.fhost_vif_idx = MGMR_VIF_STA;

    cfg.cfg.timeout_ms = 20000;

    if (ssid_len > sizeof(cfg.cfg.ssid.array))
    {
        printf("Invalid SSID\r\n");
        return -1;
    }
    wifi_mgmr_sta_info_reset();
    memcpy(cfg.cfg.ssid.array, config->ssid, ssid_len);
    wifi_mgmr_sta_ssid_set(config->ssid);
    cfg.cfg.ssid.length = ssid_len;

    fhost_ipc_parse_mac_addr(config->bssid, &cfg.cfg.bssid);

    if ((key_len + 1) > sizeof(cfg.cfg.key))
    {
        printf("Invalid Key\r\n");
        return -1;
    }
    if (config->key_len != 0) {
        memcpy(cfg.cfg.key, config->key, config->key_len);
        wifi_mgmr_sta_passphr_set(config->key);
    }

    cfg.cfg.pmf = config->pmf_cfg;
    cfg.cfg.freq[0] = config->freq1;
    cfg.cfg.freq[1] = config->freq2;

    if (config->akm_len != 0) {
        fhost_read_akm(config->akm_str, &cfg.cfg.akm, true);
    }

    if (cfg.cfg.ssid.length == 0) {
        printf("cfg.cfg.ssid.length == 0 \r\n");
        return -1;
    }

    cfg.link = cntrl_link;

    printf("start connecting ... \r\n");

    PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 1);

    memcpy(&wifiMgmr.sta_connect_param, config, sizeof(*config));

    fhost_ipc_connect_task_start(&cfg);

    return 0;
}

int wifi_sta_disconnect(void)
{
    // Disable network connection
    if (fhost_wpa_disable_network(MGMR_VIF_STA)) {
       return -1;
    }
    if (fhost_wpa_remove_vif(MGMR_VIF_STA)) {
       return -1;
    }

    return 0;
}

int wifi_sta_ip4_addr_get(uint32_t *addr, uint32_t *mask, uint32_t *gw, uint32_t *dns)
{
    uint8_t vif_idx = MGMR_VIF_STA;
    struct net_al_ext_ip_addr_cfg ip_cfg;

    if (fhost_env.vif[vif_idx].mac_vif &&
        (mac_vif_get_type(fhost_env.vif[vif_idx].mac_vif) == VIF_STA) &&
        mac_vif_get_active(fhost_env.vif[vif_idx].mac_vif)) {
        if (net_al_ext_get_vif_ip(vif_idx, &ip_cfg)) {
            return -1;
        }
        if (addr) {
            *addr = ip_cfg.ipv4.addr;
        }
        if (mask) {
            *mask = ip_cfg.ipv4.mask;
        }
        if (gw) {
            *gw = ip_cfg.ipv4.gw;
        }
        if (dns) {
            *dns = ip_cfg.ipv4.dns;
        }
        return 0;
    }

    return -1;
}

#if NX_FHOST_MONITOR
int wifi_mgmr_sniffer_enable(wifi_mgmr_sniffer_item_t sniffer_item)
{
    int fhost_vif_idx = 0;
    const struct {
        uint8_t name;
        uint8_t val;
    } bwmap[] = {
        { .name = 20, .val = PHY_CHNL_BW_20, },
        { .name = 40, .val = PHY_CHNL_BW_40, },
        { .name = 80, .val = PHY_CHNL_BW_80, },
        { .name = 80+80, .val = PHY_CHNL_BW_80P80, },
        { .name = 160, .val = PHY_CHNL_BW_160, },
    };

    struct fhost_vif_monitor_cfg cfg;
    struct mac_chan_def *chan;
    unsigned int freq_offset;

    if (check_wifi_ready())
        return -1;

    // get the interface index
    fhost_vif_idx = fhost_search_itf(sniffer_item.itf);
    if (fhost_vif_idx < 0) {
        fhost_print(RTOS_TASK_NULL, "Invalid itf %s\r\n", sniffer_item.itf);
        return -1;
    }

    // frequency
    cfg.chan.prim20_freq = sniffer_item.prim20_freq;
    chan = fhost_chan_get(cfg.chan.prim20_freq);
    if (chan == NULL) {
        fhost_print(RTOS_TASK_NULL, "Invalid freq %d\r\n", cfg.chan.prim20_freq);
        return -1;
    }
    cfg.chan.band = chan->band;
    cfg.chan.tx_power = chan->tx_power;

    // by default 20Mhz bandwidth
    cfg.chan.type = PHY_CHNL_BW_20;
    cfg.chan.center1_freq = cfg.chan.prim20_freq;
    cfg.chan.center2_freq = 0;

    // bw
    if (sniffer_item.type != 0) {
        for (int i = 0; i < CO_ARRAY_SIZE(bwmap); i++) {
            if (bwmap[i].name == sniffer_item.type)
            {
                cfg.chan.type = bwmap[i].val;
                break;
            }
        }
    }

    // center1_freq
    if (sniffer_item.center1_freq != 0) {
        cfg.chan.center1_freq = sniffer_item.center1_freq;
    }

    if (cfg.chan.center1_freq > cfg.chan.prim20_freq)
        freq_offset = cfg.chan.center1_freq - cfg.chan.prim20_freq;
    else
        freq_offset = cfg.chan.prim20_freq - cfg.chan.center1_freq;

    switch(cfg.chan.type) {
        case PHY_CHNL_BW_20:
            if (freq_offset != 0) {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Center frequency of primary channel different from "
                            "frequency of primary channel in 20MHz (%d != %d)\r\n",
                            cfg.chan.center1_freq, cfg.chan.prim20_freq);
                return -1;
            }
            break;
        case PHY_CHNL_BW_40:
            if (freq_offset != 10) {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Center frequency of primary channel different from "
                            "frequency of primary channel +/- 10 in 40MHz (%d != %d)\r\n",
                            cfg.chan.center1_freq, cfg.chan.prim20_freq);
                return -1;
            }
            break;
        case PHY_CHNL_BW_80P80:
            //center2_freq
            if (sniffer_item.center2_freq != 0) {
                cfg.chan.center2_freq = sniffer_item.center2_freq;
            } else {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Center frequency of secondary channel must be set\r\n");
                return -1;
            }

            //adjacent channel rejection
            if ((cfg.chan.center1_freq - cfg.chan.center2_freq == 80) ||
                (cfg.chan.center2_freq - cfg.chan.center1_freq == 80)) {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Adjacent channel is not allowed, use 160MHz bandwidth\r\n");
                return -1;
            }

            // no break
        case PHY_CHNL_BW_80:
            if ((freq_offset != 10) && (freq_offset != 30)) {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                            "Center frequency of primary channel different from "
                            "frequency of primary channel +/- 10 and frequency of"
                            "primary channel +/- 30 (%d != %d)\r\n",
                            cfg.chan.center1_freq, cfg.chan.prim20_freq);
                return -1;
            }
            break;
        case PHY_CHNL_BW_160:
            if ((freq_offset != 10) && (freq_offset != 30) &&
                (freq_offset != 50) && (freq_offset != 70)) {
                fhost_print(RTOS_TASK_NULL, "monitor_start :"
                        "Center frequency of primary channel must belong to the range:"
                        "frequency of primary channel +/- [10, 30, 50, 70]\r\n");
                return -1;
            }
            break;
        default:
            fhost_print(RTOS_TASK_NULL, "monitor_start :"
                        "Invalid bandwidth %d\r\n", cfg.chan.type);
            return -1;
    }

    if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_MONITOR, false)) {
        fhost_print(RTOS_TASK_NULL, "Error while enabling monitor mode\r\n");
        return -1;
    }

    cfg.uf = true;
    cfg.cb = sniffer_item.cb;
    cfg.cb_arg = sniffer_item.cb_arg;

    if (fhost_cntrl_monitor_cfg(cntrl_link, fhost_vif_idx, &cfg)) {
        fhost_print(RTOS_TASK_NULL, "Error while configuring monitor mode\r\n");
        return -1;
    }

    return 0;
}

int wifi_mgmr_sniffer_disable(wifi_mgmr_sniffer_item_t sniffer_item)
{
    int fhost_vif_idx = 0;

    if (!sniffer_item.itf) {
        fhost_print(RTOS_TASK_NULL, "Set itf\r\n");
        return -1;
    }

    // get the interface index
    fhost_vif_idx = fhost_search_itf(sniffer_item.itf);
    if (fhost_vif_idx < 0) {
        fhost_print(RTOS_TASK_NULL, "Invalid itf %s\r\n", sniffer_item.itf);
        return -1;
    }

    if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_STA, false)) {
        fhost_print(RTOS_TASK_NULL, "Error while disabling monitor mode\r\n");
        return -1;
    }

    return 0;
}
#endif

void wifi_mgmr_sta_info_dump(wifi_mgmr_connect_ind_stat_info_t *connection_info)
{
    int passphr_len = strlen(connection_info->passphr);
    fhost_print(RTOS_TASK_NULL, "Connected\r\n");
    fhost_print(RTOS_TASK_NULL, "ssid:            %s\r\n", connection_info->ssid);
    fhost_print(RTOS_TASK_NULL, "passphr:         %s\r\n", passphr_len ? connection_info->passphr : "Open");
    fhost_print(RTOS_TASK_NULL, "bssid:           %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                                connection_info->bssid[0],
                                connection_info->bssid[1],
                                connection_info->bssid[2],
                                connection_info->bssid[3],
                                connection_info->bssid[4],
                                connection_info->bssid[5]);
    fhost_print(RTOS_TASK_NULL, "aid:             %d\r\n", connection_info->aid);
    fhost_print(RTOS_TASK_NULL, "channel:         %d\r\n", connection_info->channel);
    fhost_print(RTOS_TASK_NULL, "security:        %d\r\n", connection_info->security);
}

void wifi_mgmr_ap_info_dump(wifi_mgmr_ap_info_t *ap)
{

    wifi_mgmr_ap_info_t *ap_info= &wifiMgmr.ap_info;
    if (NULL != ap) {
        ap_info = ap;
    }
    fhost_print(RTOS_TASK_NULL, "ssid:            %s\r\n", ap_info->ucSSID);
    fhost_print(RTOS_TASK_NULL, "passphr:         %s\r\n", ap_info->ucPWD);
    fhost_print(RTOS_TASK_NULL, "bssid:           %02X:%02X:%02X:%02X:%02X:%02X\r\n",
                                ap_info->ucBSSID[0],
                                ap_info->ucBSSID[1],
                                ap_info->ucBSSID[2],
                                ap_info->ucBSSID[3],
                                ap_info->ucBSSID[4],
                                ap_info->ucBSSID[5]);
    fhost_print(RTOS_TASK_NULL, "security:        %d\r\n", ap_info->ucSecurity);
    fhost_print(RTOS_TASK_NULL, "channel:         %d\r\n", ap_info->ucChannel);
}

uint16_t wifi_mgmr_sta_info_status_code_get()
{
    return (wifiMgmr.wifi_mgmr_stat_info.status_code);
}

int wifi_mgmr_sta_state_get(void)
{
    struct fhost_vif_status vif_status;
    fhost_get_vif_status(MGMR_VIF_STA, &vif_status);
    return (vif_status.chan.prim20_freq != 0) ? (1) :(0);
}

int wifi_mgmr_ap_state_get(void)
{
    struct fhost_vif_status vif_status;
    fhost_get_vif_status(MGMR_VIF_AP, &vif_status);
    return (vif_status.chan.prim20_freq != 0) ? (1) :(0);
}

int wifi_mgmr_state_get(void)
{
    int fhost_vif_idx;
    struct fhost_status status;
    struct fhost_vif_status vif_status;
    char vif_name[4];

    fhost_print(RTOS_TASK_NULL, "-------------------------------------\r\n");
    for (int i=0; i<2; i++) {
        fhost_vif_idx = i;
        fhost_get_vif_status(fhost_vif_idx, &vif_status);
        fhost_vif_name(fhost_vif_idx, vif_name, sizeof(vif_name) - 1);
        vif_name[3] = '\0';
        fhost_print(RTOS_TASK_NULL, "vif_name:        %s\r\n", vif_name);
        fhost_print(RTOS_TASK_NULL, "vif_idx:         %d\r\n", fhost_vif_idx);
        fhost_print(RTOS_TASK_NULL, "vif_state:       ");
        switch (vif_status.type)
        {
            case VIF_STA:
                fhost_print(RTOS_TASK_NULL, "STA\r\n");
                fhost_print(RTOS_TASK_NULL, "state:           ");
                if (vif_status.chan.prim20_freq != 0) {
                    wifi_mgmr_sta_info_dump(&wifiMgmr.wifi_mgmr_stat_info);
                } else {
                    fhost_print(RTOS_TASK_NULL, "Not Connect\r\n");
                }
                break;
            case VIF_AP:
                fhost_print(RTOS_TASK_NULL, "AP\r\n");
                if (vif_status.chan.prim20_freq != 0) {
                    wifi_mgmr_ap_info_dump(NULL);
                } else {
                }
                break;
            case VIF_MONITOR:
                fhost_print(RTOS_TASK_NULL, "MONITOR\r\n");
                break;
            case VIF_MESH_POINT:
                fhost_print(RTOS_TASK_NULL, "MESH\r\n");
                break;
            default:
                fhost_print(RTOS_TASK_NULL, "INACTIVE\r\n");
                break;
        }
        fhost_print(RTOS_TASK_NULL, "-------------------------------------\r\n");
    }

    return 0;
}

int wifi_mgmr_sta_rssi_get(int *rssi)
{
    int fhost_vif_idx = MGMR_VIF_STA;
    struct fhost_vif_status vif_status;
    char vif_name[4];

    fhost_get_vif_status(fhost_vif_idx, &vif_status);
    fhost_vif_name(fhost_vif_idx, vif_name, sizeof(vif_name) - 1);
    *rssi = vif_status.sta.rssi;
    return 0;
}

int wifi_mgmr_sta_channel_get(int *channel)
{
    int fhost_vif_idx = MGMR_VIF_STA;
    struct fhost_vif_status vif_status;
    char vif_name[4];
    uint16_t freq;

    fhost_get_vif_status(fhost_vif_idx, &vif_status);
    fhost_vif_name(fhost_vif_idx, vif_name, sizeof(vif_name) - 1);
    if (vif_status.chan.prim20_freq != 0) {
#if 0
        fhost_print(RTOS_TASK_NULL, "%s: VIF[%d] : Operating Channel: %dMHz@%dMHz\r\n",
                    vif_name,
                    fhost_vif_idx,
                    vif_status.chan.prim20_freq,
                    (1 << vif_status.chan.type) * 20);
#endif
        freq = vif_status.chan.prim20_freq;
        *channel = phy_freq_to_channel(vif_status.chan.band, freq);

        return 0;
    } else {
        fhost_print(RTOS_TASK_NULL, "%s: VIF[%d] : No Operating Channel\r\n", vif_name, fhost_vif_idx);
        return -1;
    }
}

int wifi_mgmr_sta_ssid_set(char *ssid)
{
    int len = strlen(ssid);

    if (len > 0) {
        len = len > (sizeof(wifiMgmr.wifi_mgmr_stat_info.ssid) - 1) ? sizeof(wifiMgmr.wifi_mgmr_stat_info.ssid) - 1 : len;
        memcpy(wifiMgmr.wifi_mgmr_stat_info.ssid, ssid, len);
        wifiMgmr.wifi_mgmr_stat_info.ssid[len] = 0;
        return 0;
    }

    return -1;
}

int wifi_mgmr_sta_passphr_set(char *passphr)
{
    int len = 0;
    if (passphr) {
        len = strlen(passphr);
    }
    wifiMgmr.wifi_mgmr_stat_info.passphr[len] = 0;

    if (len > 0) {
        len = len > (sizeof(wifiMgmr.wifi_mgmr_stat_info.passphr) - 1) ? sizeof(wifiMgmr.wifi_mgmr_stat_info.passphr) - 1 : len;
        memcpy(wifiMgmr.wifi_mgmr_stat_info.passphr, passphr, len);
        return 0;
    }

    return -1;
}

int wifi_mgmr_sta_connect_ind_stat_get(wifi_mgmr_connect_ind_stat_info_t *wifi_mgmr_ind_stat)
{
    int fhost_vif_idx = MGMR_VIF_STA;
    struct fhost_vif_status vif_status;
    char vif_name[4];

    fhost_get_vif_status(fhost_vif_idx, &vif_status);
    fhost_vif_name(fhost_vif_idx, vif_name, sizeof(vif_name) - 1);
    if (vif_status.chan.prim20_freq != 0) {
        int ssid_len = strlen(wifiMgmr.wifi_mgmr_stat_info.ssid);
        if (ssid_len > 0) {
            memcpy(wifi_mgmr_ind_stat->ssid, wifiMgmr.wifi_mgmr_stat_info.ssid, ssid_len);
            wifi_mgmr_ind_stat->ssid[ssid_len] = '\0';
        }

        int passphr_len = strlen(wifiMgmr.wifi_mgmr_stat_info.passphr);
        if (passphr_len > 0) {
            memcpy(wifi_mgmr_ind_stat->passphr, wifiMgmr.wifi_mgmr_stat_info.passphr, passphr_len);
            wifi_mgmr_ind_stat->passphr[passphr_len] = '\0';
        }
        memcpy(wifi_mgmr_ind_stat->bssid, wifiMgmr.wifi_mgmr_stat_info.bssid, sizeof(wifiMgmr.wifi_mgmr_stat_info.bssid));
        wifi_mgmr_ind_stat->aid = wifiMgmr.wifi_mgmr_stat_info.aid;
        wifi_mgmr_ind_stat->channel = wifiMgmr.wifi_mgmr_stat_info.channel;
        wifi_mgmr_ind_stat->security = wifiMgmr.wifi_mgmr_stat_info.security;

        /*
        fhost_print(RTOS_TASK_NULL, "ssid: %s, passphr: %s\r\n",
            wifi_mgmr_ind_stat->ssid,
            passphr_len ? wifi_mgmr_ind_stat->passphr : "Open");
        */
        wifi_mgmr_sta_info_dump(wifi_mgmr_ind_stat);

        return 0;
    } else {
        fhost_print(RTOS_TASK_NULL, "No Connect\r\n");
        return -1;
    }
}

int wifi_mgmr_rate_config(uint16_t fixed_rate_cfg)
{
    struct cfgrwnx_rc_set_rate cmd;
    struct cfgrwnx_resp resp;

    if (check_wifi_ready())
        return -1;

    if (!wifi_mgmr_sta_state_get()) {
        return -1;
    }
    // prepare CFGRWNX_RC_SET_RATE_CMD to send
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RC_SET_RATE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.sta_idx = wifiMgmr.wifi_mgmr_stat_info.ap_idx;
    cmd.fixed_rate_cfg = (uint16_t) fixed_rate_cfg;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RC_SET_RATE_RESP;

    // Send CFGRWNX_RC_SET_RATE_CMD
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        return -1;
    }
    return 0;

}

int wifi_mgmr_sta_scan(const wifi_mgmr_scan_params_t *config)
{
    struct cfgrwnx_scan cmd;
    struct cfgrwnx_resp resp;
    struct cfgrwnx_scan_ssid ssid;
    struct cfgrwnx_msg_hdr msg_hdr;

    if (check_wifi_ready())
        return -1;

    ssid.len = config->ssid_length;
    ssid.ssid = ssid.len ? config->ssid_array : NULL;
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SCAN_CMD;
    cmd.fhost_vif_idx = MGMR_VIF_STA;
    cmd.freqs = NULL;
    cmd.chans_cnt = config->channels_cnt;
    cmd.chans = cmd.chans_cnt ? config->channels : NULL;
    cmd.extra_ies = NULL;
    cmd.ssids = &ssid;
    cmd.ssid_cnt = 1;
    cmd.bssid = config->bssid_set_flag ? config->bssid : NULL;
    cmd.extra_ies_len = 0;
    cmd.no_cck = 0;
    cmd.duration = config->duration;
    //if is_cntrl_link is true, donot need scan sock
    //cmd.sock = cntrl_link->sock_send;
    cmd.is_cntrl_link = true;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.passive = false;
    cmd.flags = 0;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SCAN_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
        return -1;

    return 0;
}

static inline int wifi_mgmr_scan_item_is_timeout(wifi_mgmr_t *mgmr, wifi_mgmr_scan_item_t *item)
{
    return ((unsigned int)rtos_now(0) - (unsigned int)item->timestamp_lastseen) >= mgmr->scan_item_timeout ? 1 : 0;
}


int wifi_mgmr_sta_scanlist(void)
{
    int i;

    printf("cached scan list\r\n");
    printf("****************************************************************************************************\r\n");
    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i]))) {
            printf("index[%02d]: channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, ppm abs:rel %3d : %3d, wps %2d, mode %6s, auth %20s, cipher:%12s, SSID %s\r\n",
                    i,
                    wifiMgmr.scan_items[i].channel,
                    wifiMgmr.scan_items[i].bssid[0],
                    wifiMgmr.scan_items[i].bssid[1],
                    wifiMgmr.scan_items[i].bssid[2],
                    wifiMgmr.scan_items[i].bssid[3],
                    wifiMgmr.scan_items[i].bssid[4],
                    wifiMgmr.scan_items[i].bssid[5],
                    wifiMgmr.scan_items[i].rssi,
                    wifiMgmr.scan_items[i].ppm_abs,
                    wifiMgmr.scan_items[i].ppm_rel,
                    wifiMgmr.scan_items[i].wps,
                    wifi_mgmr_mode_to_str(wifiMgmr.scan_items[i].mode),
                    wifi_mgmr_auth_to_str(wifiMgmr.scan_items[i].auth),
                    wifi_mgmr_cipher_to_str(wifiMgmr.scan_items[i].cipher),
                    wifiMgmr.scan_items[i].ssid
            );
        } else {
            printf("index[%02d]: empty\r\n", i);
        }
    }
    printf("----------------------------------------------------------------------------------------------------\r\n");
    return 0;
}


uint32_t wifi_mgmr_sta_scanlist_nums_get()
{
    uint32_t i, cnt = 0;

    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i]))) {
            cnt++;
        }
    }

    return cnt;
}

uint32_t wifi_mgmr_sta_scanlist_dump(void * results, uint32_t resultNums)
{
    int i, j = 0;
    wifi_mgmr_scan_item_t *scanResults = (wifi_mgmr_scan_item_t *)results;

    for (i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]) && j < resultNums; i++) {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i]))) {
            memcpy(&scanResults[j++], &wifiMgmr.scan_items[i], sizeof(wifi_mgmr_scan_item_t));
        }
    }

    return j;
}

int wifi_mgmr_scan_ap_all(void *env, void *arg, scan_item_cb_t cb)
{
    int i = 0;

    for (i = 0; i < sizeof(wifiMgmr.scan_items) / sizeof(wifiMgmr.scan_items[0]); i++) {
        if (wifiMgmr.scan_items[i].is_used && (!wifi_mgmr_scan_item_is_timeout(&wifiMgmr, &wifiMgmr.scan_items[i]))) {
            cb(env, arg, &wifiMgmr.scan_items[i]);
        }
    }

    return 0;
}

static void wifi_mgmr_basic_ap_info_save(struct fhost_vif_ap_cfg *cfg)
{
    wifi_mgmr_ap_info_t *ap_info= &wifiMgmr.ap_info;
    if(cfg == NULL){
        return;
    }

    /* save ap cfg info */
    memset(ap_info, 0, sizeof(wifi_mgmr_ap_info_t));
    memcpy(ap_info->ucSSID, cfg->ssid.array, cfg->ssid.length);
    ap_info->ucSSIDLength = cfg->ssid.length;
    memcpy(ap_info->ucPWD, cfg->key, strlen(cfg->key));
    ap_info->ucChannel = phy_freq_to_channel(cfg->chan.band, cfg->chan.prim20_freq);
    /**
     * Bit field of Authentication and Key Management (@ref mac_akm_suite)
     * Supported combination are:\n
     * - MAC_AKM_NONE : for open security
     * - MAC_AKM_PRE_RSN : for WEP security (deprecated)
     * - MAC_AKM_PRE_RSN | MAC_AKM_PSK : For WPA1 security (deprecated)
     * - MAC_AKM_PSK : For WPA2 security
     * - MAC_AKM_SAE : For WPA3 security
     * - MAC_AKM_PSK | MAC_AKM_SAE : For WPA2/WPA3 security
     */
    if(cfg->akm & CO_BIT(MAC_AKM_NONE)){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_OPEN;
    }else if(cfg->akm & CO_BIT(MAC_AKM_PRE_RSN)){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_WEP;
    }else if((cfg->akm & CO_BIT(MAC_AKM_PRE_RSN)) && (cfg->akm & CO_BIT(MAC_AKM_PSK))){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_WPA_PSK;
    }else if(cfg->akm & CO_BIT(MAC_AKM_PSK)){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK;
    }else if(cfg->akm & CO_BIT(MAC_AKM_SAE)){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_WPA3_SAE;
    }else if((cfg->akm & CO_BIT(MAC_AKM_PSK)) && (cfg->akm & CO_BIT(MAC_AKM_SAE))){
        ap_info->ucSecurity = WIFI_EVENT_BEACON_IND_AUTH_WPA2_PSK_WPA3_SAE;
    }else{

    }
    wifi_mgmr_ap_mac_get(ap_info->ucBSSID);

    return ;
}

static void wifi_mgmr_basic_ap_info_release()
{
    wifi_mgmr_ap_info_t *ap_info= &wifiMgmr.ap_info;
    memset(ap_info, 0, sizeof(wifi_mgmr_ap_info_t));
    return;
}

int wifi_mgmr_ap_start(const wifi_mgmr_ap_params_t *config)
{
    struct fhost_vif_ap_cfg cfg;
    struct net_al_ext_ip_addr_cfg ip_cfg;
    int res = -1;
    int fhost_vif_idx = MGMR_VIF_AP;

    if (check_wifi_ready())
        return -1;

    memset(&cfg, 0, sizeof(cfg));

    size_t ssid_len = strlen(config->ssid);
    if (ssid_len == 0 || ssid_len > sizeof(cfg.ssid.array)) {
        printf("Invalid SSID");
        res = -1;
        goto end;
    }
    memcpy(cfg.ssid.array, config->ssid, ssid_len);
    cfg.ssid.length = ssid_len;

    if (config->key != NULL) {
        size_t key_len = strlen(config->key);
        if ((key_len + 1) > sizeof(cfg.key)) {
            printf("Invalid Key");
            res = -1;
            goto end;
        }
        strcpy(cfg.key, config->key);
    }

    // default
    cfg.chan.type = PHY_CHNL_BW_20;
    cfg.chan.band = PHY_BAND_2G4;
    if (config->type != 0) {
        cfg.chan.type = config->type;
    }
    if (config->channel != 0) {
        cfg.chan.prim20_freq = phy_channel_to_freq(cfg.chan.band, config->channel);
    } else {
        // default channel is 6
        cfg.chan.prim20_freq = phy_channel_to_freq(cfg.chan.band, 6);
    }

    if (PHY_CHNL_BW_20 == cfg.chan.type) {
        cfg.chan.center1_freq = cfg.chan.prim20_freq;
    }
    if (PHY_CHNL_BW_40 == cfg.chan.type) {
        //for example, ht40-
        cfg.chan.center1_freq = cfg.chan.prim20_freq - 10;
    }

    struct mac_chan_def *chan = NULL;
    chan = fhost_chan_get(cfg.chan.prim20_freq);
    if (!chan) {
        printf("Invalid channel\n");
        goto end;
    }

    if (NULL != config->akm && fhost_read_akm(config->akm, &cfg.akm, true))
        goto end;

    if ((cfg.ssid.length == 0) || (cfg.chan.prim20_freq == 0)) {
         res = -1;
         goto end;
    }
    #define OPT_LEN 50
    if (config->ap_max_inactivity) {
        cfg.extra_cfg = rtos_malloc(OPT_LEN);
        if (!cfg.extra_cfg) {
            printf("%s: no more memory\r\n", __func__);
            goto end;
        }

        dbg_snprintf(cfg.extra_cfg, OPT_LEN, "%s%u;", "ap_max_inactivity ", config->ap_max_inactivity);

    }

    // try to select the best AKM if not set
    if (cfg.akm == 0) {
        if (strlen(cfg.key) == 0)
            cfg.akm = CO_BIT(MAC_AKM_NONE);
#if 0 // not support WEP now
        else if (strlen(cfg.key) == 5)
            cfg.akm = CO_BIT(MAC_AKM_PRE_RSN);
#endif
        else
            cfg.akm = CO_BIT(MAC_AKM_PSK);
    } else if (config->key == NULL) {
        // default key
        strcpy(cfg.key, "12345678");
    }

    // Save IP configuration (if any)
    net_al_ext_get_vif_ip(fhost_vif_idx, &ip_cfg);

    // (Re)Set interface type to AP
    if (fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_UNKNOWN, false) ||
        fhost_set_vif_type(cntrl_link, fhost_vif_idx, VIF_AP, false))
        goto end;

    if (fhost_ap_cfg(fhost_vif_idx, &cfg, false)) {
        printf("Failed to start AP, check your configuration");
        goto end;
    }

    /* save ap cfg info */
    wifi_mgmr_basic_ap_info_save(&cfg);

    // Reconfigure IP only if it was static
    if (ip_cfg.mode == IP_ADDR_STATIC_IPV4) {
        ip_cfg.default_output = true;
        // set ap addr and mask
        ip_cfg.ipv4.addr = config->ap_ipaddr;
        ip_cfg.ipv4.mask = config->ap_mask;
        net_al_ext_set_vif_ip(fhost_vif_idx, &ip_cfg);
    }

    if (config->use_dhcpd) {
	net_dhcpd_start(fhost_to_net_if(fhost_vif_idx), config->start, config->limit);
    }

    res = 0;

  end:
    if (cfg.extra_cfg)
        rtos_free(cfg.extra_cfg);

    return res;
}

int wifi_mgmr_ap_stop(void)
{
    // Disable network connection
    if (fhost_wpa_disable_network(MGMR_VIF_AP)) {
       return -1;
    }
    if (fhost_wpa_remove_vif(MGMR_VIF_AP)) {
       return -1;
    }

    /* release ap cfg info */
    wifi_mgmr_basic_ap_info_release();

    return 0;
}

int wifi_mgmr_mac_set(uint8_t mac[6])
{
    memcpy(wifiMgmr.wlan_sta.mac, mac, 6);
    // mac[5] ^= 0x01;
    memcpy(wifiMgmr.wlan_ap.mac, mac, 6);
    wifiMgmr.wlan_sta.set = 1;
    wifiMgmr.wlan_ap.set = 1;
    return 0;
}

int wifi_mgmr_sta_mac_get(uint8_t mac[6])
{
    if (wifiMgmr.wlan_sta.set == 0) {
        if (platform_get_mac(wifiMgmr.wlan_sta.mac)) {
            printf("get sta mac error\r\n");
            return -1;
        }
        printf("get sta mac: %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                wifiMgmr.wlan_sta.mac[0], wifiMgmr.wlan_sta.mac[1], wifiMgmr.wlan_sta.mac[2],
                wifiMgmr.wlan_sta.mac[3], wifiMgmr.wlan_sta.mac[4], wifiMgmr.wlan_sta.mac[5]);
        wifiMgmr.wlan_sta.set = 1;
    }

    memcpy(mac, wifiMgmr.wlan_sta.mac, 6);
    return 0;
}

int wifi_mgmr_ap_mac_get(uint8_t mac[6])
{
    if (wifiMgmr.wlan_ap.set == 0) {
        if (platform_get_mac(wifiMgmr.wlan_ap.mac)) {
            printf("get ap mac error\r\n");
            return -1;
        }
        // wifiMgmr.wlan_ap.mac[5] ^= 0x01;
        printf("get ap mac: %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                wifiMgmr.wlan_ap.mac[0], wifiMgmr.wlan_ap.mac[1], wifiMgmr.wlan_ap.mac[2],
                wifiMgmr.wlan_ap.mac[3], wifiMgmr.wlan_ap.mac[4], wifiMgmr.wlan_ap.mac[5]);
        wifiMgmr.wlan_ap.set = 1;
    }

    memcpy(mac, wifiMgmr.wlan_ap.mac, 6);
    return 0;
}

int wifi_mgmr_get_country_code(char *country_code)
{
    if (country_code == NULL){
        return -1;
    }

    memcpy(country_code, wifiMgmr.country_code, sizeof(wifiMgmr.country_code));
    return 0;
}

int wifi_mgmr_set_country_code(char *country_code)
{
    int ret;
    strncpy(wifiMgmr.country_code, country_code, sizeof(wifiMgmr.country_code));
    wifiMgmr.country_code[2] = '\0';
    wifiMgmr.channel_nums = wifi_mgmr_get_channel_nums(country_code);
    ret = fhost_cntrl_chan_config_update(wifiMgmr.channel_nums);
    if (ret != 0) {
        printf("%s channel config failed\r\n", __func__);
        return -1;
    }

    printf("%s:code = %s\r\n", __func__, country_code);
    return 0;
}

int wifi_mgmr_sta_autoconnect_enable(void)
{
    wifiMgmr.disable_autoreconnect = 0;

    if (check_wifi_ready())
        return -1;

    if (FHOST_WPA_STATE_STOPPED == fhost_wpa_get_state(MGMR_VIF_STA)) {
        goto end;
    }

    if (fhost_wpa_execute_cmd(MGMR_VIF_STA, NULL, 0, 300, "STA_AUTOCONNECT %u", !wifiMgmr.disable_autoreconnect)) {
        return -1;
    }
end:
    printf("Enable Auto Reconnect\r\n");
    return 0;
}

int wifi_mgmr_sta_autoconnect_disable(void)
{
    wifiMgmr.disable_autoreconnect = 1;

    if (check_wifi_ready())
        return -1;

    if (FHOST_WPA_STATE_STOPPED == fhost_wpa_get_state(MGMR_VIF_STA)) {
        goto end;
    }

    if (fhost_wpa_execute_cmd(MGMR_VIF_STA, NULL, 0, 300, "STA_AUTOCONNECT %u", !wifiMgmr.disable_autoreconnect)) {
        return -1;
    }
end:
    printf("Disable Auto Reconnect\r\n");
    return 0;
}

int wifi_mgmr_sta_ps_enter(void)
{
    struct cfgrwnx_set_ps_mode cmd;
    struct cfgrwnx_resp resp;
    int fhost_vif_idx = MGMR_VIF_STA;
    int uapsd_queue = -1;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SET_PS_MODE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SET_PS_MODE_RESP;

    cmd.ps_mode = MGMR_PS_MODE_ON_DYN;
    cmd.enabled = true;

    if (check_wifi_ready())
        return -1;

    cmd.fhost_vif_idx = fhost_vif_idx;
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return -1;

    return 0;
}

int wifi_mgmr_sta_ps_exit(void)
{
    struct cfgrwnx_set_ps_mode cmd;
    struct cfgrwnx_resp resp;
    int fhost_vif_idx = MGMR_VIF_STA;
    int uapsd_queue = -1;

    if (check_wifi_ready())
        return -1;

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_SET_PS_MODE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_SET_PS_MODE_RESP;

    cmd.ps_mode = MGMR_PS_MODE_OFF;
    cmd.enabled = false;

    cmd.fhost_vif_idx = fhost_vif_idx;
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr))
        return -1;

    return 0;
}

int wifi_mgmr_sta_set_listen_itv(uint8_t itv)
{
    if (check_wifi_ready())
        return -1;
    return fhost_set_listen_itv(itv);
}

uint8_t wifi_mgmr_sta_get_listen_itv(void)
{
    if (check_wifi_ready())
        return -1;
    return fhost_get_listen_itv();
}

int wifi_mgmr_sta_aid_get(void)
{
    uint8_t vif_idx = MGMR_VIF_STA;
    struct fhost_status status;
    struct fhost_vif_status vif_status;
    char vif_name[4];
    int16_t aid = -1;

    if (fhost_env.vif[vif_idx].mac_vif &&
        (mac_vif_get_type(fhost_env.vif[vif_idx].mac_vif) == VIF_STA) &&
        mac_vif_get_active(fhost_env.vif[vif_idx].mac_vif)) {
        fhost_get_vif_status(vif_idx, &vif_status);
        fhost_vif_name(vif_idx, vif_name, sizeof(vif_name) - 1);
        aid = vif_status.sta.aid;
        //fhost_print(RTOS_TASK_NULL, "%s: VIF[%d] : aid: %d\r\n", vif_name, vif_idx, vif_status.sta.aid);
    }

    return aid;
}

int wifi_mgmr_sta_get_bssid(uint8_t bssid[6])
{
    struct fhost_vif_status vif_status;

    fhost_get_vif_status(MGMR_VIF_STA, &vif_status);

    if (vif_status.chan.prim20_freq != 0) {
        memcpy(bssid, wifiMgmr.wifi_mgmr_stat_info.bssid, sizeof(wifiMgmr.wifi_mgmr_stat_info.bssid));

        return 0;
    } else {
        return -1;
    }
}

int wifi_mgmr_ap_sta_info_get(struct wifi_sta_basic_info *sta_info, uint8_t idx)
{
    sta_info->is_used = wifiMgmr.ap_sta_info[idx].is_used;
    if (!sta_info->is_used)
        return 0;
    sta_info->sta_idx = wifiMgmr.ap_sta_info[idx].sta_idx;
    sta_info->aid = wifiMgmr.ap_sta_info[idx].aid;
    memcpy(sta_info->sta_mac, wifiMgmr.ap_sta_info[idx].sta_mac, 6);
    return 0;
}

int wifi_mgmr_tpc_pwr_set(rf_pwr_table_t *power_table)
{
    bl_tpc_update_power_table((int8_t *)power_table->pwr_11b);

    return 0;
}

int wifi_mgmr_tpc_pwr_get(rf_pwr_table_t *power_table)
{
    bl_tpc_power_table_get((int8_t *)power_table->pwr_11b);

    return 0;
}

int wifi_mgmr_sta_keepalive_time_set(uint8_t time_seconds)
{
    bl_sta_set_keepalive_period(time_seconds);
    return 0;
}

int wifi_mgmr_get_ap_info(wifi_mgmr_ap_info_t *ap)
{
    wifi_mgmr_ap_info_t *ap_info= &wifiMgmr.ap_info;
    if(ap == NULL){
        return -1;
    }

    memcpy(ap, ap_info, sizeof(wifi_mgmr_ap_info_t));
    wifi_mgmr_ap_info_dump(ap);
    return 0;
}

int wifi_mgmr_conf_max_sta(uint8_t max_sta_supported)
{
    if (max_sta_supported <= NX_REMOTE_STA_MAX) {
        wifiMgmr.max_sta_supported = max_sta_supported;
        printf("Conf Max Sta to %u\r\n", max_sta_supported);
    } else {
        printf("You should set max_sta_supported <= %d\r\n", NX_REMOTE_STA_MAX);
    }
    return 0;
}

int wifi_mgmr_ap_sta_delete(uint8_t sta_idx)
{
    struct wifi_sta_basic_info sta_info;

    if (!wifiMgmr.ap_sta_info[sta_idx].is_used) {
        printf("STA %d already remove\r\n", sta_idx);
        return 0;
    }

    fhost_wpa_ap_sta_del(MGMR_VIF_AP, wifiMgmr.ap_sta_info[sta_idx].sta_mac);

    return 0;
}

int wifi_mgmr_raw_80211_send(const wifi_mgmr_raw_send_params_t *config)
{
    struct cfgrwnx_raw_send cmd;
    struct cfgrwnx_resp resp;

    if (check_wifi_ready())
        return -1;

    if (config->len > LEN_PKT_RAW_80211) {
        /*raw packet len is too long*/
        printf("Raw packet len is too long");
        return -1;
    }

    if (!config->pkt || !config->len) {
        return -1;
    }

    if (config->channel > fhost_chan.chan2G4_cnt) {
        printf("Not supported channel\r\n");
        return -1;
    }

    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RAW_SEND_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.fhost_vif_idx = MGMR_VIF_STA;
    cmd.pkt = config->pkt;
    cmd.len = config->len;
    cmd.channel = config->channel;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RAW_SEND_RESP;

    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS)) {
        return -1;
    }

    return 0;
}

int wifi_mgmr_psk_cal(char *password, const uint8_t *ssid, int ssid_len, char *output)
{
    int ret = 1;
    char psk[32];

    extern int pbkdf2_sha1(const char *passphrase, const uint8_t *ssid, size_t ssid_len, int iterations, uint8_t *buf, size_t buflen);
    ret = pbkdf2_sha1(password, ssid, ssid_len, 4096, (uint8_t *)psk, sizeof(psk));
    if (0 == ret) {
        utils_bin2hex(output, psk, 32);
    }

    return ret;
}

#if defined(CFG_BL_WIFI_PS_ENABLE) || defined(CFG_WIFI_PDS_RESUME)
int wifi_mgmr_wifi_pwr_off(void)
{
    if (wifi_mgmr_sta_state_get() || wifi_mgmr_ap_state_get()) {
        printf("Wifi is connected or ap mode is on, should not turn off wifi.");
        return -1;
    }

    bl_lp_turnoff_rf();

    return 0;
}

int wifi_mgmr_wifi_pwr_on(void)
{
    bl_lp_turnon_rf();

    return 0;
}
#endif

#ifdef CFG_BL_WIFI_PS_ENABLE
platform_event_handler_t keep_alive_handler = NULL;

int wifi_mgmr_sta_start_keep_alive(uint16_t seconds)
{
    //todo: add send nullframe msg to firmware
    //keep_alive_handler = platform_add_schedule_event((platform_event_func_t)bl_sta_send_nullframe, NULL, seconds * 1000, 1);
    if (!keep_alive_handler) {
        printf("Keep alive fail.\r\n");

        return -1;
    }

    return 0;
}

int wifi_mgmr_sta_stop_keep_alive(void)
{
    if(!keep_alive_handler) {
        return -1;
    }

    if (platform_delete_schedule_event(keep_alive_handler)) {
        printf("stop keep alive fail.\r\n");
        return -1;
    }

    keep_alive_handler = NULL;

    return 0;
}

uint8_t wifi_mgmr_sta_extra_timcnt_get(void)
{
    if (bl_nonstandard_ap_detect()) {
        return bl_tim_cnt_get();
    } else {
        return 0;
    }
}

#endif
