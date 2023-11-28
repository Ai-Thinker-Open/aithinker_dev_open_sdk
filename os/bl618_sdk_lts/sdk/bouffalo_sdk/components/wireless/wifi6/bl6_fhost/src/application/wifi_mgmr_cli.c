#include "rwnx_config.h"
#include "fhost.h"
#include "fhost_ipc.h"
#include "fhost_wpa.h"
#include "fhost_cntrl.h"
#include "fhost_config.h"
#include "mac_frame.h"
#if NX_PING
#include "fhost_ping.h"
#endif
#if NX_IPERF
#include "fhost_iperf.h"
#endif
#include "fhost_connect.h"
#include "fhost_rx.h"
#include "ipc_emb.h"
#include "sys/socket.h"
#include "cfgrwnx.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"
#ifdef IOT_SDK_ADAPTER
#include "iot_sdk_adapter.h"
#else
#include "shell.h"
#endif
#include "utils_getopt.h"
#include "net_al_ext.h"

#define MAC_ADDR_LIST(m) (m)[0], (m)[1], (m)[2], (m)[3], (m)[4], (m)[5]
#define DEFAULT_CHAN_FOR_RAW_PKT 6

void cmd_hello(int argc, char **argv)
{
    int i;
    printf("cmd_hello\r\n");
#if 1
    fhost_print(RTOS_TASK_NULL, "cmd_hello argc = %d\r\n", argc);
    for (i = 0; i < argc; i++) {
        fhost_print(RTOS_TASK_NULL, "cmd_hello argv[%d] = %s\r\n", i, argv[i]);
    }
#endif
}

void cmd_phy(int argc, char **argv)
{
    int i;

    printf("cmd_phy\r\n");
#if 1
    fhost_print(RTOS_TASK_NULL, "cmd_phy argc = %d\r\n", argc);
    for (i = 0; i < argc; i++) {
        fhost_print(RTOS_TASK_NULL, "cmd_phy argc = %s\r\n", argv[i]);
    }
#endif
    printf("riu_psselect_getf     *0x24C0B080 = 0x%08lx, = %ld, bit0_1 = 0x%01lx\r\n",
            (*(volatile uint32_t *)0x24C0B080),
            (*(volatile uint32_t *)0x24C0B080),
            (*(volatile uint32_t *)0x24C0B080) & 0x3);
    printf("mdm_primaryind_get    *0x24C00850 = 0x%08lx\r\n",
            (*(volatile uint32_t *)0x24C00850));
    printf("*0x24C00824           *0x24C00824 = 0x%08lx, = %ld, bit25_24 = 0x%01lx\r\n",
            (*(volatile uint32_t *)0x24C00824),
            (*(volatile uint32_t *)0x24C00824),
            ((*(volatile uint32_t *)0x24C00824)>>24) & 0x3);
    printf("dot11WEPExcludedCount *0x24C00800 = 0x%08lx, = %ld, bit0_3 = 0x%01lx\r\n",
            (*(volatile uint32_t *)0x24C00800),
            (*(volatile uint32_t *)0x24C00800),
            (*(volatile uint32_t *)0x24C00800) & 0xF);
}


#ifdef CMD_TXL_CNTRL_PUSH_AC_ENABLE
void cmd_ac_set(int argc, char **argv)
{
    int i;
    uint8_t ac;

    if (argc != 2) {
        printf("arg error.\r\n");
        return;
    }

    ac = (uint8_t)atoi(argv[1]);

    printf("set ac = %d\r\n", ac);

    extern void wifi_set_txl_cntrl_push_access_category(uint8_t type);
    wifi_set_txl_cntrl_push_access_category(ac);
}
#endif

//In order to do coexistence test, Made a temporary patch
//#ifdef CFG_FOR_COEXISTENCE_TEST_STOPAP_PATCH
void cmd_ap_stop(int argc, char **argv)
{
    extern int wpa_stop_ap(void);
    printf("--------------- 1 wpa_stop_ap\r\n");
    wpa_stop_ap();

    extern void wpa_rwnx_deinitap(void);
    ///wpa_rwnx_deinitap();
}
//#endif

static unsigned char char_to_hex(char asccode)
{
    unsigned char ret;

    if('0'<=asccode && asccode<='9')
        ret=asccode-'0';
    else if('a'<=asccode && asccode<='f')
        ret=asccode-'a'+10;
    else if('A'<=asccode && asccode<='F')
        ret=asccode-'A'+10;
    else
        ret=0;

    return ret;
}

static void chan_str_to_hex(uint8_t *sta_num, char *sta_str)
{
    int i, str_len, base=1;
    uint16_t val = 0;
    char *q;

    str_len = strlen(sta_str);
    q = sta_str;
    q[str_len] = '\0';
    for (i=0; i< str_len; i++) {
        val = val + char_to_hex(q[str_len-1-i]) * base;
        base = base * 10;
    }
    (*sta_num) = val;
    printf("sta_str: %s, str_len: %d, sta_num: %d, q: %s\r\n", sta_str, str_len, (*sta_num), q);

}

static void wifi_scan_cmd(int argc, char **argv)
{
    int opt, ret;
    getopt_env_t getopt_env;
    uint32_t duration_scan_ms;

    wifi_mgmr_scan_params_t config;
    memset(&config, 0 , sizeof(wifi_mgmr_scan_params_t));

    utils_getopt_init(&getopt_env, 0);

    while ((opt = utils_getopt(&getopt_env, argc, argv, "s:c:b:t:")) != -1) {
         switch (opt) {
             case 's':
             {
                 memcpy(config.ssid_array, getopt_env.optarg, MAC_SSID_LEN);
                 config.ssid_length = strlen(getopt_env.optarg);
                 printf("ssid: %s len: %d\r\n", config.ssid_array, config.ssid_length);
             }
             break;
             case 'c':
             {
                 utils_parse_number_adv(getopt_env.optarg, ',', config.channels, MAX_FIXED_CHANNELS_LIMIT, 10, &config.channels_cnt);
             }
             break;
             case 'b':
             {
                 config.bssid_set_flag = 1;
                 utils_parse_number(getopt_env.optarg, ':', config.bssid, 6, 16);
                 printf("bssid: %s, mac:%02X:%02X:%02X:%02X:%02X:%02X\r\n", getopt_env.optarg,
                          MAC_ADDR_LIST(config.bssid));
             }
             break;
             case 't':
             {
                 config.duration = atoi(getopt_env.optarg);
             }
             break;
             default:
             {
                 printf("unknow option: %c\r\n", getopt_env.optopt);
             }
         }

    }
    ret = wifi_mgmr_sta_scan(&config);
    if (ret < 0) {
        printf("scan failed \r\n");
        return;
    }
}

#define WEP_PASSWORD_BUF_LEN (13 * 2 + 1)

static void cvt_bin_2_hex(const void *in, size_t in_len, char *out)
{
    for (size_t i = 0; i < in_len; ++i) {
        uint8_t b = ((uint8_t *)in)[i];
        uint8_t b_l = b & 0xf;
        uint8_t b_h = b >> 4;
        *out++ = b_h <= 9 ? b_h + '0' : b_h - 10 + 'A';
        *out++ = b_l <= 9 ? b_l + '0' : b_l - 10 + 'A';
    }
    *out = '\0';
}

static bool is_valid_hex_str(const char *str)
{
    size_t len = strlen(str);
    if (len % 2) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        char c = str[i];
        if (!(('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F'))) {
            return false;
        }
    }
    return true;
}

static int cvt_wep_password(const char *in, char *out)
{
    size_t len = strlen(in);

    memset(out, 0, WEP_PASSWORD_BUF_LEN);

    if (len == 5 || len == 13) {
        cvt_bin_2_hex(in, len, out);
    } else if (len == 10 || len == 26) {
        if (!is_valid_hex_str(in)) {
            return -1;
        }
        strcpy(out, in);
    } else {
        return -1;
    }
    return 0;
}

/**
 * CLI options:
 * b: BSSID
 * 2: WPA2-Only
 * 3: WPA3-Only
 * f: PMF cfg, 0/1/2, defaults to 1(PMF Capable)
 * w: WEP. In this mode, the password can be given in ASCII or HEX format(wo prefix 0x)
 * D: Disable DHCP
 * c: channel index
 * q: quick_connect
 */
static void wifi_connect_cmd(int argc, char **argv)
{
    getopt_env_t getopt_env;
    int opt;
    int bssid_set_flag = 0;
    char bssid[18] = {0};
    char *password = NULL;
    char password_buf[WEP_PASSWORD_BUF_LEN];
    int wpa2_only = 0, wpa3_only = 0;
    const char *akm = NULL;
    int pmf_cfg = 1;
    int use_dhcp = 1;
    int wep = 0;
    uint8_t channel_index = 0;
    uint16_t freq = 0;
    uint8_t quick_connect = 0;

    if (2 > argc) {
        goto _ERROUT;
    }

    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "b:23f:qwDc:")) != -1) {
        switch (opt) {
        case 'b':
            bssid_set_flag = 1;
            memcpy(bssid, getopt_env.optarg, 17);
            printf("bssid: %s \r\n", getopt_env.optarg);
            break;
        case 'f':
            pmf_cfg = atoi(getopt_env.optarg);
            if (!(0 <= pmf_cfg && pmf_cfg <= 2)) {
                goto _ERROUT;
            }
            break;
        case '2':
             wpa2_only = 1;
             break;
        case '3':
             wpa3_only = 1;
             break;
        case 'D':
             use_dhcp = 0;
             break;
        case 'q':
            ++quick_connect;
            break;
        case 'w':
             wep = 1;
             break;
        case 'c':
            channel_index = atoi(getopt_env.optarg);
            printf("channel_index: %d\r\n", channel_index);
            break;
             break;

        case '?':
            printf("unknow option: %c\r\n", getopt_env.optopt);
            goto _ERROUT;
        }
    }

    if (getopt_env.optind == argc) {
        printf("Expected ssid\r\n");
        goto _ERROUT;
    }

    password = argv[getopt_env.optind + 1];

    // Reject conflicting config
    if (wpa2_only + wpa3_only + wep > 1) {
        goto _ERROUT;
    }
    if (!password && (wpa2_only || wpa3_only || wep)) {
        goto _ERROUT;
    }

    if (wpa2_only) {
        akm = "WPA2";
    }
    if (wpa3_only) {
        akm = "WPA3";
        // PMF required for WPA3 only
        pmf_cfg = 2;
    }
    if (wep) {
        akm = "WEP";
        if (cvt_wep_password(password, password_buf)) {
            printf("invalid WEP password\r\n");
            goto _ERROUT;
        }
        password = password_buf;
    }

    printf("connect wifi ssid:%s, psk:%s, bssid:%d, q:%d\r\n", argv[getopt_env.optind], password, bssid_set_flag, quick_connect);
    // XXX
    // akm must be all in uppercase.
    //
    // TODO remove all reference to fhost_ipc_upper() and change related function
    // parameters from T * to const T *.
    freq = phy_channel_to_freq(PHY_BAND_2G4, channel_index);
    if (quick_connect) {
        wifi_mgmr_sta_connect_params_t conn_param;
        memset(&conn_param, 0, sizeof(conn_param));
        strncpy((char *)conn_param.ssid, argv[getopt_env.optind], MGMR_SSID_LEN);
        conn_param.ssid_len = strlen((char *)conn_param.ssid);

        if (password) {
          strncpy((char *)conn_param.key, password, MGMR_KEY_LEN);
          conn_param.key_len = strlen((char *)conn_param.key);
        }
        conn_param.freq1 = freq;
        conn_param.freq2 = 0;
        conn_param.use_dhcp = 1;
        conn_param.pmf_cfg = 1;
        conn_param.quick_connect = 1;
        wifi_mgmr_sta_connect(&conn_param);
    } else
        wifi_sta_connect(argv[getopt_env.optind], password, bssid_set_flag ? bssid : NULL, (char *)akm, pmf_cfg, freq, freq, use_dhcp);

    return;

_ERROUT:
    printf("[USAGE]: %s [-b <bssid>] [-2/3] [-f 0/1/2] [-q] [-w] [-D] [-c <ch_idx>] <ssid> [password]\r\n", argv[0]);
    return;
}

static void wifi_mgmr_ap_start_cmd(int argc, char **argv)
{
    getopt_env_t getopt_env;
    int opt;
    wifi_mgmr_ap_params_t config;

    if (argc < 2) {
        goto _ERROUT;
    }

    memset(&config, 0, sizeof(config));

    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "s:k:c:a:t:I:S:L:")) != -1) {
        switch (opt) {
        case 's':
            config.ssid = getopt_env.optarg;
            break;

        case 'k':
            config.key = getopt_env.optarg;
            break;

        case 'c':
            config.channel = (uint8_t)atoi(getopt_env.optarg);
            break;

        case 'a':
            config.akm = getopt_env.optarg;
            break;

        case 't':
            config.ap_max_inactivity = (uint32_t)atoi(getopt_env.optarg);
            break;

        case 'I':
            config.ap_ipaddr = inet_addr(getopt_env.optarg);
            config.ap_mask = inet_addr("255.255.255.0");
            if((config.ap_ipaddr & 0xff000000)  != 0x01000000)
                printf("ap mode ipaddr is not x.x.x.1 \r\n");
            break;

        case 'S':
            config.start = atoi(getopt_env.optarg);
            break;

        case 'L':
            config.limit = atoi(getopt_env.optarg);
            break;

        case '?':
            printf("unknow option: %c \r\n", getopt_env.optopt);
            goto _ERROUT;

        }
    }

    // TODO add option
    config.use_dhcpd = true;

    if (config.ssid == NULL) {
        goto _ERROUT;
    }

    wifi_mgmr_ap_start(&config);

    return;

 _ERROUT:
    printf("[USAGE]: %s -s <ssid> [-k <key>] [-c <channel>] [-a <akm>] [-I <ipv4_addr>] [-S <dhcpd_start>] [-L <dhcpd_limit>] \r\n", argv[0]);
    return;
}

static void wifi_mgmr_ap_stop_cmd(int argc, char **argv)
{
    wifi_mgmr_ap_stop();
}

static void wifi_disconnect_cmd(int argc, char **argv)
{
    wifi_sta_disconnect();
}

static void lwip_cmd(int argc, char **argv)
{
void stats_display(void);
    stats_display();
}

#if NX_FHOST_MONITOR
static void cb_sniffer(struct bl_frame_info *info, void *arg)
{
    if (info->payload == NULL)
    {
        TRACE_APP(INF, "Unsupported frame: length = %d", info->length)
        printf("Unsupported frame: length = %dr\r\n", info->length);
    }
    else
    {
        struct mac_hdr *hdr __MAYBE_UNUSED = (struct mac_hdr *)info->payload;
        TRACE_APP(INF, "%pM %pM %pM %fc SN:%d length = %d", TR_MAC(hdr->addr1.array),
                  TR_MAC(hdr->addr2.array), TR_MAC(hdr->addr3.array), hdr->fctl, hdr->seq >> 4,
                  info->length);
        printf("SN:%d length = %d\r\n", hdr->seq >> 4, info->length);
    }
}

static void cmd_wifi_sniffer_on(int argc, char **argv)
{
    wifi_mgmr_sniffer_item_t sniffer_item;
    getopt_env_t getopt_env;
    int opt;

    memset(&sniffer_item, 0 , sizeof(wifi_mgmr_sniffer_item_t));
    utils_getopt_init(&getopt_env, 0);

    while ((opt = utils_getopt(&getopt_env, argc, argv, "i:t:f:c:C:")) != -1) {
        switch (opt) {
        case 'i':
            sniffer_item.itf = getopt_env.optarg;
            printf("itf: %s\r\n", sniffer_item.itf);
            break;

        case 't':
            sniffer_item.type = atoi(getopt_env.optarg);
            printf("type: %d\r\n", sniffer_item.type);
            break;

        case 'f':
            sniffer_item.prim20_freq = atoi(getopt_env.optarg);
            printf("prim20_freq: %d\r\n", sniffer_item.prim20_freq);
            break;

        case 'c':
            sniffer_item.center1_freq = atoi(getopt_env.optarg);
            printf("center1_freq: %d\r\n", sniffer_item.center1_freq);
            break;

        case 'C':
            sniffer_item.center2_freq = atoi(getopt_env.optarg);
            printf("center2_freq: %d\r\n", sniffer_item.center2_freq);
            break;

        case '?':
            printf("unknow option: %c\r\n", getopt_env.optopt);
            goto _ERROUT;
        }
    }

    sniffer_item.cb = cb_sniffer;
    sniffer_item.cb_arg = NULL;
    wifi_mgmr_sniffer_enable(sniffer_item);
    return;
_ERROUT:
    printf("[USAGE]: %s -i <itf> -f <prim20_freq> [-t <bw>] [-c <center1_freq>] [-C <center2_freq>]\r\n", argv[0]);
    return;
}

static void cmd_wifi_sniffer_off(int argc, char **argv)
{
    wifi_mgmr_sniffer_item_t sniffer_item;
    getopt_env_t getopt_env;
    int opt;

    memset(&sniffer_item, 0 , sizeof(wifi_mgmr_sniffer_item_t));
    utils_getopt_init(&getopt_env, 0);

    while ((opt = utils_getopt(&getopt_env, argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            sniffer_item.itf = getopt_env.optarg;
            printf("itf: %s\r\n", sniffer_item.itf);
            break;

        case '?':
            printf("unknow option: %c\r\n", getopt_env.optopt);
            goto _ERROUT;
        }
    }

    wifi_mgmr_sniffer_disable(sniffer_item);
    return;
_ERROUT:
    printf("[USAGE]: %s -i <itf>\r\n", argv[0]);
    return;
}
#endif

static void cmd_wifi_state_get(int argc, char **argv)
{
    wifi_mgmr_state_get();
    return;
}

static void cmd_wifi_sta_rssi_get(int argc, char **argv)
{
    int rssi = 0;

    wifi_mgmr_sta_rssi_get(&rssi);
    fhost_print(RTOS_TASK_NULL, "rssi: %ddB\r\n", rssi);
    return;
}

static void cmd_wifi_sta_channel_get(int argc, char **argv)
{
    int channel = 0;

    wifi_mgmr_sta_channel_get(&channel);
    printf("STA Channel: %d\r\n", channel);
    return;
}

static void cmd_wifi_sta_ssid_passphr_get(int argc, char **argv)
{
    wifi_mgmr_connect_ind_stat_info_t wifi_mgmr_ind_stat;

    wifi_mgmr_sta_connect_ind_stat_get(&wifi_mgmr_ind_stat);
    return;
}

static void cmd_wifi_sta_mac_get(int argc, char **argv)
{
    uint8_t mac_addr[6];

    wifi_mgmr_sta_mac_get(mac_addr);
    printf("STA MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        mac_addr[0], mac_addr[1],
                        mac_addr[2], mac_addr[3],
                        mac_addr[4], mac_addr[5]);
    return;
}

static void cmd_wifi_ap_mac_get(int argc, char **argv)
{
    uint8_t mac_addr[6];

    wifi_mgmr_ap_mac_get(mac_addr);
    printf("AP MAC: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        mac_addr[0], mac_addr[1],
                        mac_addr[2], mac_addr[3],
                        mac_addr[4], mac_addr[5]);
    return;
}

static void wifi_enable_autoreconnect_cmd(int argc, char **argv)
{
    wifi_mgmr_sta_autoconnect_enable();
}

static void wifi_disable_autoreconnect_cmd(int argc, char **argv)
{
    wifi_mgmr_sta_autoconnect_disable();
}

static void wifi_sta_ps_on_cmd(int argc, char **argv)
{
    wifi_mgmr_sta_ps_enter();
}

static void wifi_sta_ps_off_cmd(int argc, char **argv)
{
    wifi_mgmr_sta_ps_exit();
}

static void _print_sta_pwr(uint8_t numb, int8_t *power_table)
{
    uint8_t i;
    for (i = 0; i<numb; i++) {
        printf("%3u ", power_table[i]);
    }
    printf("\r\n");
}

static void wifi_sta_info_cmd(int argc, char **argv)
{
    ip4_addr_t addr, mask, gw, dns;
    int rssi = 0;
    rf_pwr_table_t power_table;

    memset(&addr, 0, sizeof(ip4_addr_t));
    memset(&mask, 0, sizeof(ip4_addr_t));
    memset(&gw, 0, sizeof(ip4_addr_t));
    memset(&dns, 0, sizeof(ip4_addr_t));
    memset(power_table.pwr_11b, 0, sizeof(rf_pwr_table_t));

    wifi_sta_ip4_addr_get(&addr, &mask, &gw, &dns);
    wifi_mgmr_sta_rssi_get(&rssi);
    wifi_mgmr_tpc_pwr_get(&power_table);
    fhost_print(RTOS_TASK_NULL, "================================================================\r\n");
    fhost_print(RTOS_TASK_NULL, "RSSI:    %ddbm\r\n", rssi);
    fhost_print(RTOS_TASK_NULL, "IP  :    %s \r\n", ip4addr_ntoa(&addr));
    fhost_print(RTOS_TASK_NULL, "MASK:    %s \r\n", ip4addr_ntoa(&mask));
    fhost_print(RTOS_TASK_NULL, "GW  :    %s \r\n", ip4addr_ntoa(&gw));
    fhost_print(RTOS_TASK_NULL, "DNS :    %s \r\n", ip4addr_ntoa(&dns));
    fhost_print(RTOS_TASK_NULL,   "Power Table (dbm):\r\n");
    fhost_print(RTOS_TASK_NULL,   "-----------------------------------------------------------\r\n");
    fhost_print(RTOS_TASK_NULL, "  11b:         ");
    _print_sta_pwr(4,  power_table.pwr_11b);
    fhost_print(RTOS_TASK_NULL, "  11g:         ");
    _print_sta_pwr(8,  power_table.pwr_11g);
    fhost_print(RTOS_TASK_NULL, "  11n(ht20):   ");
    _print_sta_pwr(8,  power_table.pwr_11n_ht20);
    fhost_print(RTOS_TASK_NULL, "  11n(ht40):   ");
    _print_sta_pwr(8,  power_table.pwr_11n_ht40);
    fhost_print(RTOS_TASK_NULL, "  11ac(vht20): ");
    _print_sta_pwr(10, power_table.pwr_11ac_vht20);
    fhost_print(RTOS_TASK_NULL, "  11ac(vht40): ");
    _print_sta_pwr(10, power_table.pwr_11ac_vht40);
#if 0
    fhost_print(RTOS_TASK_NULL, "  11ac(vht80): ");
    _print_sta_pwr(10, power_table.pwr_11ac_vht80);
#endif
    fhost_print(RTOS_TASK_NULL, "  11ax(he20):  ");
    _print_sta_pwr(12, power_table.pwr_11ax_he20);
    fhost_print(RTOS_TASK_NULL, "  11ax(he40):  ");
    _print_sta_pwr(12, power_table.pwr_11ax_he40);
#if 0
    fhost_print(RTOS_TASK_NULL, "  11ax(he80):  ");
    _print_sta_pwr(12, power_table.pwr_11ax_he80);
    fhost_print(RTOS_TASK_NULL, "  11ax(he160): ");
    _print_sta_pwr(12, power_table.pwr_11ax_he160);
#endif
    fhost_print(RTOS_TASK_NULL, "================================================================\r\n");
}

static void wifi_ap_sta_list_get_cmd(int argc, char **argv)
{
    int i, j;
    struct wifi_sta_basic_info sta_info;
    long long sta_time;
    uint8_t index = 0;

    memset(&sta_info, 0, sizeof(struct wifi_sta_basic_info));
    printf("sta list:\r\n");
    printf("--------------------------------------------\r\n");
    printf("No.      StaIndex      Mac-Address       Aid\r\n");
    printf("--------------------------------------------\r\n");
    for(i = 0, j = 0;i < NX_REMOTE_STA_MAX;i++){
        wifi_mgmr_ap_sta_info_get(&sta_info, i);
        if (!sta_info.is_used) {
            continue;
        }

        printf(" %u       "
            "   %u        "
            "%02X:%02X:%02X:%02X:%02X:%02X    "
            "%d      "
            "\r\n",
            j++,
            sta_info.sta_idx,
            sta_info.sta_mac[0],
            sta_info.sta_mac[1],
            sta_info.sta_mac[2],
            sta_info.sta_mac[3],
            sta_info.sta_mac[4],
            sta_info.sta_mac[5],
            sta_info.aid
        );
    }
}

static void wifi_ap_sta_delete_cmd(int argc, char **argv)
{
    struct wifi_sta_basic_info sta_info;
    uint8_t sta_idx = 0;

    if (2 != argc) {
        printf("[USAGE]: %s sta_idx\r\n", argv[0]);
        return;
    }

    printf("Delete Sta Idx.%s \r\n", argv[1]);
    chan_str_to_hex(&sta_idx, argv[1]);
    printf("sta idx = %d \r\n", sta_idx);

    memset(&sta_info, 0, sizeof(struct wifi_sta_basic_info));
    wifi_mgmr_ap_sta_info_get(&sta_info, sta_idx);
    if (!sta_info.is_used || (sta_info.sta_idx == 0xef)){
        printf("Idx.%d sta is invalid\r\n", sta_idx);
        return;
    }

    printf("sta info: Idx = %u,"
        "mac = %02X:%02X:%02X:%02X:%02X:%02X,"
        "aid = %d"
        "\r\n",
        sta_info.sta_idx,
        sta_info.sta_mac[0],
        sta_info.sta_mac[1],
        sta_info.sta_mac[2],
        sta_info.sta_mac[3],
        sta_info.sta_mac[4],
        sta_info.sta_mac[5],
        sta_info.aid
    );
    wifi_mgmr_ap_sta_delete(sta_info.sta_idx);
}

static void cmd_wifi_ap_conf_max_sta(int argc, char **argv)
{
    int max_sta_supported;

    if (2 != argc) {
        printf("Usage: wifi_ap_max_sta [num]\r\n");
        return;
    }

    max_sta_supported = atoi(argv[1]);
    wifi_mgmr_conf_max_sta(max_sta_supported);
}

static uint8_t packet_raw[] = {
    0x48, 0x02,
    0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x00
};

static void cmd_wifi_raw_send(int argc, char **argv)
{
    static uint32_t seq = 0;
    getopt_env_t getopt_env;
    int opt;
    int channel = DEFAULT_CHAN_FOR_RAW_PKT;
    wifi_mgmr_raw_send_params_t config;

    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "c:")) != -1) {
        switch (opt) {
        case 'c':
            channel = atoi(getopt_env.optarg);
            printf("set chan: %d\r\n", channel);
            break;

        case '?':
            printf("unknow option: %c\r\n", getopt_env.optopt);
            goto _ERROUT;
        }
    }

    memset(&config, 0, sizeof(config));
    config.channel = channel;
    config.pkt = packet_raw;;
    config.len = sizeof(packet_raw);

    for (int i = 0; i < 5; i++) {
        packet_raw[sizeof(packet_raw) - 2] = ((seq << 4) & 0xFF);
        packet_raw[sizeof(packet_raw) - 1] = (((seq << 4) & 0xFF00) >> 8);
        seq++;

        if (wifi_mgmr_raw_80211_send(&config)) {
            printf("Raw send failed\r\n");
        } else {
            printf("Raw send succeed\r\n");
        }

        //delay 200ms
        rtos_task_suspend(200);
    }
    return;

_ERROUT:
    printf("[USAGE]: %s -c <channel num>\r\n", argv[0]);
}

#ifdef CONFIG_CLI_WIFI_DUBUG
static void cmd_wifi(int argc, char **argv)
{
void mm_sec_keydump(void);
    mm_sec_keydump();
}
#endif

#define SET_IPV4_USAGE                    \
    "set_ipv4 [ip] [dns] [gw] [mask]\r\n"   \
    "\t ip: set static ip\r\n" \
    "\t dns: set local host dns\r\n" \
    "\t gw: set local host gate way. default is 255.255.255.0\r\n" \
    "\t mask: set local host mask. default is 255.255.255.0\r\n"

static int wifi_sta_static_ipv4(int argc, char **argv)
{
    struct net_al_ext_ip_addr_cfg ip_cfg;
    ip_cfg.mode = IP_ADDR_STATIC_IPV4;
    ip_cfg.default_output = true;
    ip_cfg.dhcp.to_ms = 15000;

    char *addr, *mask, *gw, *dns;
    if (argc < 3) {
        printf("%s", SET_IPV4_USAGE);
        return 0;
    }

    /* ip addr */
    if (argc > 1) {
        addr = argv[1];    
    } else {
        addr = "127.0.0.1";
    }

    /* ip dns */
    if (argc > 2) {
        dns = argv[2];    
    } else {
        dns = "0.0.0.0";
    }

    /* ip gw */
    if (argc > 3) {
        gw = argv[3];
    } else {
        gw = "255.255.255.0";
    }
    /* ip mask */
    if (argc > 4) {
        mask = argv[4];
    } else {
        mask = "255.255.255.0";
    }

    ip_cfg.ipv4.addr = inet_addr(addr);
    ip_cfg.ipv4.mask = inet_addr(mask);
    ip_cfg.ipv4.gw = inet_addr(gw);
    ip_cfg.ipv4.dns = inet_addr(dns);

    printf("addr:%s,mask:%s,gw:%s,dns:%s\r\n", addr, mask, gw, dns);

    if (net_al_ext_set_vif_ip(0, &ip_cfg)) {
        printf("set static ipv4 error!\r\n");
        return -1;
    }
    return 0;
}

#if NX_IPERF
/**
 ****************************************************************************************
 * @brief Process function for 'iperf' command
 *
 * Start an iperf server on the specified port.
 *
 * @param[in] params parameters passed to iperf command
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int parse_ip4(char *str, uint32_t *ip, uint32_t *mask)
{
    char *token;
    uint32_t a, i, j;

    #define check_is_num(_str)  for (j = 0 ; j < strlen(_str); j++) {  \
            if (_str[j] < '0' || _str[j] > '9')                        \
                return -1;                                             \
        }

    // Check if mask is present
    token = strchr(str, '/');
    if (token && mask) {
        *token++ = '\0';
        check_is_num(token);
        a = atoi(token);
        if (a == 0 || a > 32)
            return -1;
        *mask = (1<<a) - 1;
    }
    else if (mask)
    {
        *mask = 0xffffffff;
    }

    // parse the ip part
    *ip = 0;
    for (i = 0; i < 4; i ++)
    {
        if (i < 3)
        {
            token = strchr(str, '.');
            if (!token)
                return -1;
            *token++ = '\0';
        }
        check_is_num(str);
        a = atoi(str);
        if (a > 255)
            return -1;
        str = token;
        *ip += (a << (i * 8));
    }

    return 0;
}
static void cmd_iperf(int argc, char **argv)
{
    char conv, *token, *substr;
    struct fhost_iperf_settings iperf_settings;
    bool client_server_set = 0;
    int ret, opt;
    getopt_env_t getopt_env;
    rtos_task_handle iperf_handle;

    if(argc >= 2 && !strcmp("stop", argv[1])) {
        if((iperf_handle = fhost_iperf_msg_handle_get()) != NULL) {
            fhost_iperf_sigkill_handler(iperf_handle);
            return;
        }
        printf("havn't start iperf \r\n");
        return;
    }

    fhost_iperf_settings_init(&iperf_settings);
    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "b:w:c:f:i:l:n:p:sut:S:T:XI:h")) != -1) {
        switch (opt) {
        case 'b': // UDP bandwidth
        case 'w': // TCP window size
        {
            char *decimal_str;
            int decimal = 0;
            uint64_t value;

            token = getopt_env.optarg;
            decimal_str = strchr(token, '.');
            if (decimal_str) {
                int fact = 100;
                decimal_str++;
                while (*decimal_str >= '0' && *decimal_str <= '9') {
                    decimal += (*decimal_str - '0') * fact;
                    if (fact == 1)
                        break;
                    fact = fact / 10;
                    decimal_str++;
                }
            }

            value = atoi(token);
            conv = token[strlen(token) - 1];

            // convert according to [Gg Mm Kk]
            switch (conv) {
            case 'G':
            case 'g':
                value *= 1000000000;
                value += decimal * 1000000;
                break;
            case 'M':
            case 'm':
                value *= 1000000;
                value += decimal * 1000;
                break;
            case 'K':
            case 'k':
                value *= 1000;
                value += decimal;
                break;
            default:
                break;
            }

            if (opt == 'b') {
                iperf_settings.udprate = value;
                iperf_settings.flags.is_udp = true;
                iperf_settings.flags.is_bw_set = true;
                // if -l has already been processed, is_buf_len_set is true so don't overwrite that value.
                if (!iperf_settings.flags.is_buf_len_set)
                    iperf_settings.buf_len = FHOST_IPERF_DEFAULT_UDPBUFLEN;
            }
            else {
                // TCP window is ignored for now
            }
            break;
        }
        case 'c': // Client mode with server host to connect to
        {
            if (client_server_set)
                goto help;

            iperf_settings.flags.is_server = 0;
            client_server_set = true;

            if (parse_ip4(getopt_env.optarg, &iperf_settings.host_ip, NULL)) {
                fhost_print(RTOS_TASK_NULL, "invalid IP address %s\n", getopt_env.optarg);
                return;  //FHOST_IPC_ERROR;
            }
            break;
        }
        case 'f': // format to print in
        {
            iperf_settings.format = getopt_env.optarg[0];
            break;
        }
        case 'i': // Interval between periodic reports
        {
            uint32_t interval = 0;
            substr = strchr(getopt_env.optarg, '.');

            if (substr) {
                *substr++ = '\0';
                interval += atoi(substr);
            }

            interval += atoi(getopt_env.optarg) * 10;
            if (interval < 5) {
                fhost_print(RTOS_TASK_NULL, "interval must be greater than or "
                            "equal to 0.5. Interval set to 0.5\n");
                interval = 5;
            }

            iperf_settings.interval.sec = interval / 10;
            iperf_settings.interval.usec = 100000 * (interval - (iperf_settings.interval.sec * 10));
            iperf_settings.flags.show_int_stats = true;
            break;
        }
        case 'l': //Length of each buffer
        {
            uint32_t udp_min_size = sizeof(struct iperf_UDP_datagram);

            iperf_settings.buf_len = atoi( getopt_env.optarg );
            iperf_settings.flags.is_buf_len_set = true;
            if (iperf_settings.flags.is_udp && iperf_settings.buf_len < udp_min_size) {
                iperf_settings.buf_len = udp_min_size;
                fhost_print(RTOS_TASK_NULL, "buffer length must be greater than or "
                            "equal to %d in UDP\n", udp_min_size);
            }
            break;
        }
        case 'n': // amount mode (instead of time mode)
        {
            iperf_settings.flags.is_time_mode = false;
            iperf_settings.amount = atoi( getopt_env.optarg );
            break;
        }
        case 'p': //server port
        {
            iperf_settings.port = atoi( getopt_env.optarg );
            break;
        }
        case 's': // server mode
        {
            if (client_server_set)
                goto help;
            iperf_settings.flags.is_server = 1;
            client_server_set = true;
            break;
        }
        case 't': // time mode (instead of amount mode)
        {
            iperf_settings.flags.is_time_mode = true;
            iperf_settings.amount = 0;
            substr = strchr(getopt_env.optarg, '.');
            if (substr) {
                *substr++ = '\0';
                iperf_settings.amount += atoi(substr);
            }

            iperf_settings.amount += atoi(getopt_env.optarg) * 10;
            break;
        }
        case 'u': // UDP instead of TCP
        {
            // if -b has already been processed, UDP rate will be non-zero, so don't overwrite that value
            if (!iperf_settings.flags.is_udp) {
                iperf_settings.flags.is_udp = true;
                iperf_settings.udprate = FHOST_IPERF_DEFAULT_UDPRATE;
            }

            // if -l has already been processed, is_buf_len_set is true, so don't overwrite that value.
            if (!iperf_settings.flags.is_buf_len_set) {
                iperf_settings.buf_len = FHOST_IPERF_DEFAULT_UDPBUFLEN;
            }
            break;
        }
        case 'S': // IP type-of-service
        {
            // the zero base allows the user to specify
            // hexadecimals: "0x#"
            // octals: "0#"
            // decimal numbers: "#"
            iperf_settings.tos = strtol( getopt_env.optarg, NULL, 0 );
            break;
        }
        case 'T': // TTL
        {
            iperf_settings.ttl = atoi(getopt_env.optarg);
            break;
        }
        case 'X': // Peer version detect
        {
            iperf_settings.flags.is_peer_ver = true;
            break;
        }
        case 'I': // vif num
        {
            uint16_t vif_num;
            vif_num = atoi(getopt_env.optarg);
            if (vif_num == MGMR_VIF_STA) {
                iperf_settings.vif_num = MGMR_VIF_STA;
            } else if (vif_num == MGMR_VIF_AP) {
                iperf_settings.vif_num = MGMR_VIF_AP;
            } else {
                fhost_print(RTOS_TASK_NULL, iperf_long_help);
                return; //FHOST_IPC_ERROR;
            }
            break;
        }
        case 'h': // Long Help
        {
        help:
            fhost_print(RTOS_TASK_NULL, iperf_long_help);
            return;
        }
        default:
        {
            goto help;
        }
        }
    }

    if (!client_server_set)
        goto help;

    fhost_iperf_start(&iperf_settings);
}
#endif // NX_IPERF

#define LINE_MAX_SZ 150
/// Rate Control configuration
struct fhost_rc_conf {
    /// String describing the RC configuration
    char line[LINE_MAX_SZ + 1];
    /// Index of the RC configuration
    int r_idx;
};

/**
 ****************************************************************************************
 * @brief Compare indexes of rate control configurations
 *
 * @param[in] a         First rate control configuration
 * @param[in] b         Second rate control configuration
 * @return 1 if the index of a is higher than the index of b
 ****************************************************************************************
 */
static int fhost_rc_compare_idx(const void *a, const void *b)
{
    struct fhost_rc_conf *idx_a = *(struct fhost_rc_conf **) a;
    struct fhost_rc_conf *idx_b = *(struct fhost_rc_conf **) b;

    if (idx_a->r_idx < idx_b->r_idx)
        return -1;
    else
        return 1;
}

/**
 ****************************************************************************************
 * @brief Convert RC table index to Rate configuration
 *
 * @param[in] idx      Index of the RC table
 * @param[in] is_rx    Indicates whether the conversion is for RX stats
 * @param[out] r_cfg   Rate configuration to fill
 * @param[out] ru_size RU size for the HE TB
 *
 * @return 0 if successful, != 0 otherwise
 ****************************************************************************************
 */
static int fhost_idx_to_rate_cfg(int idx, bool is_rx, union fhost_rc_rate_ctrl_info *r_cfg,
                                 int *ru_size)
{
    union fhost_rc_mcs_index {
        struct {
            uint32_t mcs : 3;
            uint32_t nss : 2;
        } ht;
        struct {
            uint32_t mcs : 4;
            uint32_t nss : 3;
        } vht;
        struct {
            uint32_t mcs : 4;
            uint32_t nss : 3;
        } he;
    };
    uint16_t first_ht, first_vht, first_he_su , first_he_er __MAYBE_UNUSED, first_he_mu __MAYBE_UNUSED;
    uint8_t he_mu_rates_per_mcs __MAYBE_UNUSED, he_su_rates_per_mcs __MAYBE_UNUSED,
            he_er_rates_per_mcs __MAYBE_UNUSED, vht_rates_per_mcs, ht_rates_per_mcs;
    uint8_t max_vht_mcs, max_he_mcs __MAYBE_UNUSED;

    #if NX_FHOST_RX_STATS
    if (is_rx)
    {
        first_ht =  fhost_conf_rx.first_ht;
        first_vht = fhost_conf_rx.first_vht;
        #if NX_HE
        first_he_su = fhost_conf_rx.first_he_su;
        first_he_er = fhost_conf_rx.first_he_er;
        #else
        first_he_su = fhost_conf_rx.n_rates;
        #endif
        first_he_mu = fhost_conf_rx.first_he_mu;
        ht_rates_per_mcs = fhost_conf_rx.ht_rates_per_mcs;
        vht_rates_per_mcs = fhost_conf_rx.vht_rates_per_mcs;
        he_su_rates_per_mcs = fhost_conf_rx.he_su_rates_per_mcs;
        he_er_rates_per_mcs = fhost_conf_rx.he_er_rates_per_mcs;
        he_mu_rates_per_mcs = fhost_conf_rx.he_mu_rates_per_mcs;
        max_vht_mcs = fhost_conf_rx.max_vht_mcs;
        max_he_mcs = fhost_conf_rx.max_he_mcs;
    }
    else
    #endif
    {
        first_ht = FIRST_HT;
        first_vht = FIRST_VHT;
        first_he_su = FIRST_HE_SU;
        first_he_er = FIRST_HE_ER;
        first_he_mu = FIRST_HE_MU;
        he_er_rates_per_mcs = 3;
        he_su_rates_per_mcs = 4 * 3;
        vht_rates_per_mcs = 4 * 2;
        ht_rates_per_mcs = 2 * 2;
        he_mu_rates_per_mcs = 6 * 3;
        max_vht_mcs = 10;
        max_he_mcs = 12;
    }

    r_cfg->value = 0;
    if (idx < FIRST_OFDM)
    {
        r_cfg->format_mod_tx = FORMATMOD_NON_HT;
        r_cfg->gi_and_pre_type_tx = (idx & 1) << 1;
        r_cfg->mcs_idx = idx / 2;
    }
    else if (idx < first_ht)
    {

        r_cfg->format_mod_tx = FORMATMOD_NON_HT;
        r_cfg->mcs_idx =  idx - N_CCK + 4;
    }
    else if (idx < first_vht)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_ht;
        r_cfg->format_mod_tx = FORMATMOD_HT_MF;
        r->ht.nss = idx / (8*ht_rates_per_mcs);
        r->ht.mcs = (idx % (8*ht_rates_per_mcs)) / ht_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (8*ht_rates_per_mcs)) % ht_rates_per_mcs) / 2;
        r_cfg->gi_and_pre_type_tx = idx & 1;
    }
    else if (idx < first_he_su)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_vht;
        r_cfg->format_mod_tx = FORMATMOD_VHT;
        r->vht.nss = idx / (max_vht_mcs*vht_rates_per_mcs);
        r->vht.mcs = (idx % (max_vht_mcs*vht_rates_per_mcs)) / vht_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (max_vht_mcs*vht_rates_per_mcs)) % vht_rates_per_mcs) / 2;
        r_cfg->gi_and_pre_type_tx = idx & 1;
    }
    #if NX_HE
    else if (idx < first_he_mu)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_he_su;
        r_cfg->format_mod_tx = FORMATMOD_HE_SU;
        // Jochen 2023/3/6: use msb of nss[2:0] to indicate dcm
        // r->vht.nss = idx / (max_he_mcs*he_su_rates_per_mcs);
        r_cfg->dcm_tx = idx / (4*max_he_mcs*he_su_rates_per_mcs);
        r->vht.nss = (idx % (4*max_he_mcs*he_su_rates_per_mcs)) / (max_he_mcs*he_su_rates_per_mcs);
        r->vht.mcs = (idx % (max_he_mcs*he_su_rates_per_mcs)) / he_su_rates_per_mcs;
        r_cfg->bw_tx = ((idx % (max_he_mcs*he_su_rates_per_mcs)) % he_su_rates_per_mcs) / 3;
        r_cfg->gi_and_pre_type_tx = idx % 3;
    }
    else if (idx < first_he_er)
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        if (ru_size == NULL)
            return -1;

        idx -= first_he_mu;
        r_cfg->format_mod_tx = FORMATMOD_HE_MU;
        r->vht.nss = idx / (max_he_mcs*he_mu_rates_per_mcs);
        r->vht.mcs = (idx % (max_he_mcs*he_mu_rates_per_mcs)) / he_mu_rates_per_mcs;
        *ru_size = ((idx % (max_he_mcs*he_mu_rates_per_mcs)) % he_mu_rates_per_mcs)/ 3;
        r_cfg->gi_and_pre_type_tx = idx % 3;
        r_cfg->bw_tx = 0;
    }
    else
    {
        union fhost_rc_mcs_index *r = (union fhost_rc_mcs_index *)r_cfg;

        idx -= first_he_er;
        r_cfg->format_mod_tx = FORMATMOD_HE_ER;
        // Jochen 2023/3/6: add dcm tx option
        r_cfg->dcm_tx = idx / (2 * 3 * he_er_rates_per_mcs);
        r_cfg->bw_tx = (idx % (2 * 3 *he_er_rates_per_mcs)) / (3 * he_er_rates_per_mcs);
        if (ru_size != NULL)
            *ru_size = idx / (3 * he_er_rates_per_mcs);
        r_cfg->gi_and_pre_type_tx = idx % 3;
        r->vht.mcs = (idx % (3 * he_er_rates_per_mcs)) / he_er_rates_per_mcs;
        r->vht.nss = 0;
    }
    #endif // NX_HE
    return 0;
}

/**
 ****************************************************************************************
 * @brief Write one rate configuration on a string buffer passed as parameter
 *
 * @param[in] buf       String buffer where to write the Rate Control result
 * @param[in] size      Size of the string buffer
 * @param[in] format    Format of the rate configuration
 * @param[in] nss       Number of spatial streams of the rate configuration
 * @param[in] mcs       MCS index of the rate configuration
 * @param[in] bw        Bandwidth of the rate configuration
 * @param[in] sgi       Short Guard interval setting of the rate configuration
 * @param[in] pre       Preamble type of the rate configuration
 * @param[in] is_rx     Indicates whether we are printing RX stats
 * @param[out] r_idx    Index of the rate configuration
 * @return Length of the written string
 ****************************************************************************************
 */
static int fhost_print_rate(char *buf, int size, int format, int nss, int mcs, int bw,
                            int sgi, int pre, int dcm, bool is_rx, int *r_idx)
{
    static const int ru_size_he_er[] = { 242, 106 };
    static const int ru_size_he_mu[] = { 26, 52, 106, 242, 484, 996 };

    int res = 0;
    int bitrates_cck[4] = { 10, 20, 55, 110 };
    int bitrates_ofdm[8] = { 6, 9, 12, 18, 24, 36, 48, 54 };
    char he_gi[3][4] = { "0.8", "1.6", "3.2" };
    uint16_t first_ht, first_vht, first_he_su, first_he_er, first_he_mu;
    uint8_t max_vht_mcs, max_he_mcs, ht_rates_per_mcs, vht_rates_per_mcs;
    uint8_t he_su_rates_per_mcs, he_er_rates_per_mcs, he_mu_rates_per_mcs;

    #if NX_FHOST_RX_STATS
    if (is_rx)
    {
        first_ht = fhost_conf_rx.first_ht;
        first_vht = fhost_conf_rx.first_vht;
        first_he_su = fhost_conf_rx.first_he_su;
        first_he_mu = fhost_conf_rx.first_he_mu;
        first_he_er = fhost_conf_rx.first_he_er;

        max_vht_mcs = fhost_conf_rx.max_vht_mcs;
        max_he_mcs = fhost_conf_rx.max_he_mcs;
        ht_rates_per_mcs = fhost_conf_rx.ht_rates_per_mcs;
        vht_rates_per_mcs = fhost_conf_rx.vht_rates_per_mcs;
        he_su_rates_per_mcs = fhost_conf_rx.he_su_rates_per_mcs;
        he_mu_rates_per_mcs = fhost_conf_rx.he_mu_rates_per_mcs;
        he_er_rates_per_mcs = fhost_conf_rx.he_er_rates_per_mcs;
    }
    else
    #endif
    {
        first_ht = FIRST_HT;
        first_vht = FIRST_VHT;
        first_he_su = FIRST_HE_SU;
        first_he_mu = FIRST_HE_MU;
        first_he_er = FIRST_HE_ER;
        he_su_rates_per_mcs = 4 * 3;
        vht_rates_per_mcs = 4 * 2;
        ht_rates_per_mcs  = 2 * 2;
        he_mu_rates_per_mcs = 6 * 3;
        he_er_rates_per_mcs = 3;
        max_vht_mcs = 10;
        max_he_mcs = 12;
    }

    if (format < FORMATMOD_HT_MF)
    {
        if (mcs < 4)
        {
            if (r_idx)
            {
                *r_idx = (mcs * 2) + pre;
                res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
            }
            res += dbg_snprintf(&buf[res], size - res, "L-CCK/%cP%11c%2u.%1uM   ",
                                pre > 0 ? 'L' : 'S', ' ', bitrates_cck[mcs] / 10,
                                bitrates_cck[mcs] % 10);
        }
        else
        {
            mcs -= 4;
            if (r_idx)
            {
                *r_idx = FIRST_OFDM + mcs;
                res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
            }
            res += dbg_snprintf(&buf[res], size - res, "L-OFDM%13c%2u.0M   ",
                                ' ', bitrates_ofdm[mcs]);
        }
    }
    else if (format < FORMATMOD_VHT)
    {
        if (r_idx)
        {
            *r_idx = first_ht + nss * 8 * ht_rates_per_mcs + mcs * ht_rates_per_mcs +
                      bw * 2 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        mcs += nss * 8;
        res += dbg_snprintf(&buf[res], size - res, "HT%d/%cGI%11cMCS%-2d   ",
                            20 * (1 << bw), sgi ? 'S' : 'L', ' ', mcs);
    }
    else if (format == FORMATMOD_VHT)
    {
        if (r_idx)
        {
            *r_idx = first_vht + nss * max_vht_mcs * vht_rates_per_mcs +
                     mcs * vht_rates_per_mcs + bw * 2 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "VHT%d/%cGI%*cMCS%d/%1d  ",
                            20 * (1 << bw), sgi ? 'S' : 'L', bw > 2 ? 9 : 10, ' ', mcs,
                            nss + 1);
    }
    else if (format == FORMATMOD_HE_SU)
    {
        if (r_idx)
        {
            *r_idx = first_he_su + nss * max_he_mcs * he_su_rates_per_mcs +
                     mcs * he_su_rates_per_mcs+ bw * 3 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HE%d/GI%s%4s%*cMCS%d/%1d%*c",
                            20 * (1 << bw), he_gi[sgi], dcm ? "/DCM" : "",
                            bw > 2 ? 4 : 5, ' ', mcs, nss + 1, mcs > 9 ? 1 : 2, ' ');
    }
    else if (format == FORMATMOD_HE_MU)
    {
        if (r_idx)
        {
            *r_idx = first_he_mu + nss * max_he_mcs * he_mu_rates_per_mcs +
                     mcs * he_mu_rates_per_mcs + bw * 3 + sgi;
            res = dbg_snprintf(buf, size - res, "%4d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HEMU-%d/GI%s%*cMCS%d/%1d%*c",
                            ru_size_he_mu[bw], he_gi[sgi], bw > 1 ? 5 : 6, ' ', mcs, nss + 1,
                            mcs > 9 ? 1 : 2, ' ');
    }
    else // HE ER
    {
        if (r_idx)
        {
            *r_idx = first_he_er + bw * 3 * he_er_rates_per_mcs +
                     mcs * he_er_rates_per_mcs + sgi;
            res = dbg_snprintf(buf, size - res, "%3d ", *r_idx);
        }
        res += dbg_snprintf(&buf[res], size - res, "HEER-%d/GI%s%4s%1cMCS%d/NSS%1d%2c",
                            ru_size_he_er[bw], he_gi[sgi], dcm ? "/DCM" : "",
                            ' ', mcs, nss + 1, ' ');
    }

    return res;
}

/**
 ****************************************************************************************
 * @brief Extract parameters from rate configuration and print it on a buffer
 *
 * @param[in] buf           String buffer where to write the Rate Control result
 * @param[in] size          Size of the string buffer
 * @param[in] rate_config   Rate configuration of the sample
 * @param[in] ru_size       RU size for the HE TB
 * @param[in] is_rx         Indicates whether we are printing RX stats
 * @param[out] r_idx        Index of the rate configuration
 * @return Length of the written string
 ****************************************************************************************
 */
static int fhost_print_rate_from_cfg(char *buf, int size, uint32_t rate_config,
                                     uint8_t ru_size, bool is_rx, int *r_idx)
{
    uint8_t ft, pre, gi, bw, nss, mcs, dcm;

    ft = export_rc_get_format_mod(rate_config);
    mcs = export_rc_get_mcs_index(rate_config);
    nss = export_rc_get_nss(rate_config);
    bw = export_rc_get_bw(rate_config);
    pre = export_rc_get_pre_type(rate_config);
    gi = export_rc_get_sgi(rate_config) | (pre << 1);
    dcm = 0;
    if (ft >= FORMATMOD_HE_SU)
    {
        dcm = (rate_config & RC_DCM_MOD_MASK) >> RC_DCM_MOD_OFT;
        if ((ft == FORMATMOD_HE_MU) || (ft == FORMATMOD_HE_ER))
            bw = ru_size;
    }
    return fhost_print_rate(buf, size, ft, nss, mcs, bw, gi, pre, dcm, is_rx, r_idx);
}

#if NX_FHOST_RX_STATS & 0
// move to mac
/**
 ****************************************************************************************
 * @brief Print RX statistics about a sta passed as parameter
 *
 * @param[in] sta_idx       Index of the STA
 *
 * return 0 if successful, != 0 otherwise
 ****************************************************************************************
 */
static int fhost_ipc_print_rx_stats(int sta_idx)
{
    char hist[] = "##################################################";
    int hist_len = sizeof(hist) - 1, len = 0;
    int8_t rssi[2];
    uint8_t mac_addr[MAC_ADDR_LEN];
    struct rx_vector_1 *last_rx;
    unsigned int mcs, gi, nss, pre, bw, dcm = 0;
    uint32_t i;
    char buf[LINE_MAX_SZ];
    // Extract Mac address
    MAC_ADDR_EXTRACT(mac_addr, sta_mgmt_get_peer_addr(sta_idx));
    fhost_print(RTOS_TASK_NULL, "\nRX rate info for %02x:%02x:%02x:%02x:%02x:%02x:\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                mac_addr[5]);

    if (!rx_stats[sta_idx])
    {
        fhost_print(RTOS_TASK_NULL, "\nThis station has not been initialized\n");
        return -1;
    }

    // Display Statistics
    for (i = 0 ; i < fhost_conf_rx.n_rates ; i++ )
    {
        if (rx_stats[sta_idx]->table[i]) {
            union fhost_rc_rate_ctrl_info r_cfg;
            int percent = (((uint64_t)rx_stats[sta_idx]->table[i]) * 1000) / rx_stats[sta_idx]->cpt;
            int p;
            int ru_size = 0;

            if(fhost_idx_to_rate_cfg(i, true, &r_cfg, &ru_size))
                return -1;

            fhost_print_rate_from_cfg(buf, LINE_MAX_SZ, r_cfg.value, ru_size, true, NULL);
            p = (percent * hist_len) / 1000;
            fhost_print(RTOS_TASK_NULL,"%s: %9d(%2d.%1d%%)%.*s\n",
                        buf, rx_stats[sta_idx]->table[i], percent / 10, percent % 10, p, hist);
        }
    }

    // Display detailed info of the last received rate
    last_rx = &rx_stats[sta_idx]->last_rx;

    fhost_print(RTOS_TASK_NULL,
                "\nLast received rate\n"
                "  type         rate    LDPC STBC BEAMFM DCM DOPPLER %s\n",
                (phy_get_nrx() > 0) ? "rssi1(dBm) rssi2(dBm)" : "rssi(dBm)");

    bw = last_rx->ch_bw;
    pre = last_rx->pre_type;
    #if NX_MAC_VER >= 20
    if (last_rx->format_mod >= FORMATMOD_HE_SU) {
        mcs = last_rx->he.mcs;
        nss = last_rx->he.nss;
        gi = last_rx->he.gi_type;
        if ((last_rx->format_mod == FORMATMOD_HE_MU) ||
            (last_rx->format_mod == FORMATMOD_HE_ER))
            bw = last_rx->he.ru_size;
        dcm = last_rx->he.dcm;
    } else
    #endif
    if (last_rx->format_mod == FORMATMOD_VHT) {
        #if NX_MAC_VER >= 20
        mcs = last_rx->vht.mcs;
        nss = last_rx->vht.nss;
        gi = last_rx->vht.short_gi;
        #else
        mcs = last_rx->mcs;
        nss = last_rx->stbc ? last_rx->n_sts/2 : last_rx->n_sts;
        gi = last_rx->short_gi;
        #endif
    } else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
        #if NX_MAC_VER >= 20
        mcs = last_rx->ht.mcs % 8;
        nss = last_rx->ht.mcs / 8;;
        gi = last_rx->ht.short_gi;
        #else
        mcs = last_rx->mcs % 8;
        nss = last_rx->mcs / 8;;
        gi = last_rx->short_gi;
        #endif
    } else {
        mcs = rxv2macrate[last_rx->leg_rate];
        nss = 0;
        gi = 0;
    }

    len = fhost_print_rate(buf, LINE_MAX_SZ - len, last_rx->format_mod, nss, mcs, bw, gi, pre, dcm, true, NULL);

    // flags for HT/VHT/HE
    #if NX_MAC_VER >= 20
    if (last_rx->format_mod >= FORMATMOD_HE_SU) {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c    %c     %c",
                            last_rx->he.fec ? 'L' : ' ',
                            last_rx->he.stbc ? 'S' : ' ',
                            last_rx->he.beamformed ? 'B' : ' ',
                            last_rx->he.dcm ? 'D' : ' ',
                            last_rx->he.doppler ? 'D' : ' ');
    } else
    #endif
    if (last_rx->format_mod == FORMATMOD_VHT) {
        #if NX_MAC_VER >= 20
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c           ",
                            last_rx->vht.fec ? 'L' : ' ',
                            last_rx->vht.stbc ? 'S' : ' ',
                            last_rx->vht.beamformed ? 'B' : ' ');
        #else
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c     %c           ",
                            last_rx->fec_coding ? 'L' : ' ',
                            last_rx->stbc ? 'S' : ' ',
                            !last_rx->smoothing ? 'B' : ' ');
        #endif
    } else if (last_rx->format_mod >= FORMATMOD_HT_MF) {
        #if NX_MAC_VER >= 20
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c                  ",
                            last_rx->ht.fec ? 'L' : ' ',
                            last_rx->ht.stbc ? 'S' : ' ');
        #else
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "  %c    %c                  ",
                            last_rx->fec_coding ? 'L' : ' ',
                            last_rx->stbc ? 'S' : ' ');
        #endif
    } else {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "                         ");
    }

    hal_desc_get_rssi(last_rx, rssi);
    if (phy_get_nrx() > 0) {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "       %-4d       %d\n",
                            rssi[0], rssi[1]);
    } else {
        len += dbg_snprintf(&buf[len], LINE_MAX_SZ - len, "      %d\n", rssi[0]);
    }

    fhost_print(RTOS_TASK_NULL, "%s\n", buf);
    return 0;
}
#endif // NX_FHOST_RX_STATS

/**
 ****************************************************************************************
 * @brief Print Rate Control statistics for a station passed as parameter
 *
 * @param[in] sta_idx    Station index
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_rc_print_stats_sta(int sta_idx, bool is_rx)
{
#if 0
    // todo move to mac
    struct me_rc_stats_cfm res;
    struct fhost_rc_conf *st, *st_sorted[RC_MAX_N_SAMPLE];
    struct cfgrwnx_rc cmd;
    struct cfgrwnx_rc_result resp;
    uint8_t i, mac_addr[MAC_ADDR_LEN];

    // If RX stats
    if (is_rx)
    {
        #if NX_FHOST_RX_STATS
        if (fhost_ipc_print_rx_stats(sta_idx))
            return FHOST_IPC_ERROR;
        #else
        fhost_print(RTOS_TASK_NULL, "RX Statistics not enabled in FW\n");
        return FHOST_IPC_ERROR;
        #endif
        return FHOST_IPC_SUCCESS;
    }

    // prepare CFGRWNX_RC_CMD to send
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RC_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.sta_idx = sta_idx;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RC_RESP;

    // Send CFGRWNX_RC_CMD
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        resp.cfm.no_samples = 0;
        return FHOST_IPC_SUCCESS;
    }

    res = resp.cfm;

    // Allocate RC configurations table
    st = rtos_malloc(sizeof(struct fhost_rc_conf) * res.no_samples);
    if (!st)
        return FHOST_IPC_ERROR;

    // Print mac address
    MAC_ADDR_EXTRACT(mac_addr, sta_mgmt_get_peer_addr(sta_idx));
    fhost_print(RTOS_TASK_NULL, "####### STA: %02x:%02x:%02x:%02x:%02x:%02x\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4],
                mac_addr[5]);

    // Parse and print every RC configuration
    for (i = 0; i < res.no_samples; i++)
    {
        unsigned int tp, eprob;
        uint16_t len;

        len = fhost_print_rate_from_cfg(st[i].line, LINE_MAX_SZ,
                                           res.rate_stats[i].rate_config, 0, false,
                                           &(st[i].r_idx));

        if (res.sw_retry_step != 0)
        {
            len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                                res.retry_step_idx[res.sw_retry_step] == i ? '*' : ' ');
        }
        else
        {
            len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, " ");
        }
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                            res.retry_step_idx[0] == i ? 'T' : ' ');
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c",
                            res.retry_step_idx[1] == i ? 't' : ' ');
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, "%c ",
                            res.retry_step_idx[2] == i ? 'P' : ' ');

        tp = res.tp[i] / 10;
        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len, " %4u.%1u", tp / 10,
                            tp % 10);

        eprob = ((res.rate_stats[i].probability * 1000) >> 16) + 1;

        len += dbg_snprintf(&st[i].line[len], LINE_MAX_SZ - len,
                            "  %4u.%1u %5u(%6u)  %6u", eprob / 10, eprob % 10,
                            res.rate_stats[i].success, res.rate_stats[i].attempts,
                            res.rate_stats[i].sample_skipped);

        st_sorted[i] = &st[i];
    }
    fhost_print(RTOS_TASK_NULL, "\nTX rate info for %02X:%02X:%02X:%02X:%02X:%02X:\n",
                mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    fhost_print(RTOS_TASK_NULL,
                " #  type           rate             tpt   eprob    ok(   tot)   skipped\n");

    // Sort RC statistics
    // qsort(st_sorted, res.no_samples, sizeof(struct fhost_rc_conf *), fhost_rc_compare_idx);

    for (i = 0; i < res.no_samples; i++)
        fhost_print(RTOS_TASK_NULL, "%s\n", st_sorted[i]->line);

    // display HE TB statistics if any
    if (res.rate_stats[RC_HE_STATS_IDX].rate_config != 0)
    {
        unsigned int tp, eprob;
        struct rc_rate_stats *rate_stats = &res.rate_stats[RC_HE_STATS_IDX];
        int ru_index = rate_stats->ru_and_length & 0x07;
        int ul_length = rate_stats->ru_and_length >> 3;

        fhost_print(RTOS_TASK_NULL, "\nHE TB rate info:\n");
        fhost_print(RTOS_TASK_NULL,
                    "    type           rate             tpt   eprob    ok(   tot)   ul_length\n    ");

        fhost_print_rate_from_cfg(st[0].line, LINE_MAX_SZ, rate_stats->rate_config, ru_index, false, NULL);
        fhost_print(RTOS_TASK_NULL, "%s\n", st[0].line);

        tp = res.tp[RC_HE_STATS_IDX] / 10;
        fhost_print(RTOS_TASK_NULL, "      %4u.%1u", tp / 10, tp % 10);

        eprob = ((rate_stats->probability * 1000) >> 16) + 1;
        fhost_print(RTOS_TASK_NULL, "  %4u.%1u %5u(%6u)  %6u\n",
                   eprob / 10, eprob % 10, rate_stats->success,
                   rate_stats->attempts, ul_length);
    }

    fhost_print(RTOS_TASK_NULL, "\n MPDUs AMPDUs AvLen trialP");
    fhost_print(RTOS_TASK_NULL, "\n%6u %6u %3d.%1d %6u\n", res.ampdu_len,
                res.ampdu_packets, res.avg_ampdu_len >> 16,
                ((res.avg_ampdu_len * 10) >> 16) % 10, res.sample_wait);

    // free buffers
    rtos_free(st);

#endif
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Print Rate Control statistics for every station connected to
 * a VIF passed as parameter
 *
 * @param[in] fvif_idx      Index of virtual interface in FHOST
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static int fhost_rc_print_stats(int fvif_idx, bool is_rx)
{
    uint8_t i, idx_table_size;
    uint8_t idx_table[NX_REMOTE_STA_MAX];
    struct fhost_vif_tag *fhost_vif;

    fhost_vif = &fhost_env.vif[fvif_idx];

    // Get VIF status
    idx_table_size = fhost_get_sta_idx(fhost_vif, NX_REMOTE_STA_MAX, idx_table);

    for (i = 0; i < idx_table_size; i++)
    {
        if (fhost_rc_print_stats_sta(idx_table[i], is_rx) == FHOST_IPC_ERROR)
            return FHOST_IPC_ERROR;
    }
    if (!idx_table_size)
        fhost_print(RTOS_TASK_NULL, "No results\n");

    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Set TX Rate Control for a certain station
 *
 * @param[in] sta               Pointer to the STA entry
 * @param[in] fixed_rate_idx    Rate control index to use
 * @return FHOST_IPC_SUCCESS if successful, FHOST_IPC_ERROR otherwise
 ****************************************************************************************
 */
static int fhost_rc_set_rate_sta(void *sta, int fixed_rate_idx)
{
    struct cfgrwnx_rc_set_rate cmd;
    struct cfgrwnx_resp resp;
    union fhost_rc_rate_ctrl_info rate_config;
    char buf[LINE_MAX_SZ];

    // Convert rate index into rate configuration
    if ((fixed_rate_idx < 0) || (fixed_rate_idx >= TOT_RATES))
    {
        // disable fixed rate
        rate_config.value = (uint32_t)-1;
    }
    else if (fhost_idx_to_rate_cfg(fixed_rate_idx, false, &rate_config, NULL))
        return FHOST_IPC_ERROR;

    printf("ridx %d => rate (format %d,dcm %d,bw %d,mcs %d,gi %d)\n",
        fixed_rate_idx, rate_config.format_mod_tx,
        rate_config.dcm_tx, rate_config.bw_tx, rate_config.mcs_idx, rate_config.gi_and_pre_type_tx
        );

    fhost_print_rate_from_cfg(buf, LINE_MAX_SZ, rate_config.value,
                              0, false, NULL);

    fhost_print(RTOS_TASK_NULL, "Fix rate to %s\n", buf);

    // prepare CFGRWNX_RC_SET_RATE_CMD to send
    cmd.hdr.len = sizeof(cmd);
    cmd.hdr.id = CFGRWNX_RC_SET_RATE_CMD;
    cmd.hdr.resp_queue = cntrl_link->queue;
    cmd.sta_idx = sta_get_staid(sta);
    cmd.fixed_rate_cfg = (uint16_t) rate_config.value;

    resp.hdr.len = sizeof(resp);
    resp.hdr.id = CFGRWNX_RC_SET_RATE_RESP;

    // Send CFGRWNX_RC_SET_RATE_CMD
    if (fhost_cntrl_cfgrwnx_cmd_send(&cmd.hdr, &resp.hdr) ||
        (resp.status != CFGRWNX_SUCCESS))
    {
        return FHOST_IPC_ERROR;
    }
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Set TX Rate Control for all the stations associated to a VIF
 *
 * @param[in] vif_idx           Index of the virtual interface
 * @param[in] fixed_rate_idx    Rate control index to use
 * @return FHOST_IPC_SUCCESS if successful, FHOST_IPC_ERROR otherwise
 ****************************************************************************************
 */

static int fhost_rc_set_rate(int fvif_idx, int fixed_rate_idx)
{
    struct sta_info_tag *sta;
    struct fhost_vif_tag *fhost_vif = &fhost_env.vif[fvif_idx];

    sta = (struct sta_info_tag *)co_list_pick(mac_vif_get_sta_list(fhost_vif->mac_vif));

    while (sta != NULL)
    {
        if (fhost_rc_set_rate_sta(sta, fixed_rate_idx) == FHOST_IPC_ERROR)
            return FHOST_IPC_ERROR;

        sta = sta_get_list_next(sta);
    }
    return FHOST_IPC_SUCCESS;
}

/**
 ****************************************************************************************
 * @brief Search one connected station based on the mac address
 *
 * @param[in] addr      MAC Address of the connected STA
 * @return pointer to sta_info_tag on success, NULL if error.
 ****************************************************************************************
 */
static void *fhost_search_connected_sta_by_mac(struct mac_addr *addr)
{
    int i;
    struct fhost_vif_tag *fhost_vif;

    for (i = 0; i < NX_VIRT_DEV_MAX; i++)
    {
        void *sta;
        if (fhost_env.vif[i].mac_vif == NULL)
            continue;

        fhost_vif = &fhost_env.vif[i];
        sta = co_list_pick(mac_vif_get_sta_list(fhost_vif->mac_vif));

        while (sta != NULL)
        {
            if (MAC_ADDR_CMP(sta_getp_mac_addr(sta), addr))
                return sta;

            sta = sta_get_list_next(sta);
        }
    }
    return NULL;
}

/**
 ****************************************************************************************
 * @brief Process function for 'rc' command
 *
 * Print the Rate Control Table
 * @verbatim
 rc [-m <mac_addr> | -i <itf>] [-f <idx>]
 @endverbatim
 *
 * @param[in] params  Vif interface
 * @return 0 on success and !=0 if error occurred
 ****************************************************************************************
 */
static void cmd_rc(int argc, char **argv)
{
    struct sta_info_tag *sta = NULL;
    int fhost_vif_idx = INVALID_VIF_IDX;
    int fixed_rate_idx = -1;
    bool is_print = true;
    bool is_rx = false;
    uint8_t i;
    int ret, opt;
    getopt_env_t getopt_env;
    struct mac_addr mac_addr;

    utils_getopt_init(&getopt_env, 0);
    while ((opt = utils_getopt(&getopt_env, argc, argv, "i:m:f:r")) != -1) {
        switch(opt) {
        case 'i':
            fhost_vif_idx = fhost_search_itf(getopt_env.optarg);
            if (fhost_vif_idx < 0)
                return;
            break;
        case 'm':
            if (fhost_ipc_parse_mac_addr(getopt_env.optarg, &mac_addr)) {
                fhost_print(RTOS_TASK_NULL, "Invalid Mac address");
                return;
            }

            sta = fhost_search_connected_sta_by_mac(&mac_addr);
            if (!sta) {
                fhost_print(RTOS_TASK_NULL, "Mac address not found");
                return;
            }
            break;
        case 'f':
            fixed_rate_idx = atoi(getopt_env.optarg);
            is_print = false;
            break;
        case 'r':
            is_rx = true;
            break;
        default:
            fhost_print(RTOS_TASK_NULL, "valid option");
            return;
        }
    }

    // Action is printing
    if (is_print)
    {
        // A sta has been provided (-m option)
        if (sta)
        {
            fhost_rc_print_stats_sta(sta_get_staid(sta), is_rx);
            return;
        }
        // A vif has been provided (-i option)
        else if (fhost_vif_idx != INVALID_VIF_IDX)
        {
            fhost_rc_print_stats(fhost_vif_idx, is_rx);
            return;
        }
        // No vif or sta has been provided (no options)
        else
        {
            // Print rates for every VIF
            for (i = 0; i < NX_VIRT_DEV_MAX; i++)
            {
                if ((fhost_env.vif[i].mac_vif != NULL) &&
                    (fhost_rc_print_stats(i, is_rx) != FHOST_IPC_SUCCESS))
                    return;
            }
            return;
        }
    }
    // Action is setting rate (-f option)
    else
    {
        // A sta has been provided (-m option)
        if (sta)
        {
            fhost_rc_set_rate_sta(sta, fixed_rate_idx);
            return;
        }
        // A vif has been provided (-i option)
        else if (fhost_vif_idx != INVALID_VIF_IDX)
        {
            fhost_rc_set_rate(fhost_vif_idx, fixed_rate_idx);
            return;
        }
        // No vif or sta has been provided (no options)
        else
        {
            for (i = 0; i < NX_VIRT_DEV_MAX; i++)
            {
                if ((fhost_env.vif[i].mac_vif != NULL) &&
                    (fhost_rc_set_rate(i, fixed_rate_idx) == FHOST_IPC_ERROR))
                    return;
            }
            return;
        }
    }

    return;
}

static void cmd_rate(int argc, char **argv)
{
    int msg_id;

    if(argc != 2) {
	printf("rate [number] \r\n");
        return;
    }

    msg_id = atoi(argv[1]);

    printf("rate is %x:%d\r\n", msg_id, msg_id);
extern uint32_t g_fw_rate;
    g_fw_rate = msg_id;

    return;
}

#ifdef CONFIG_CLI_CMD_ENABLE
SHELL_CMD_EXPORT_ALIAS(cmd_hello, hello, test blcli hello);
SHELL_CMD_EXPORT_ALIAS(cmd_phy, phy, test blcli hello);
#ifdef CMD_TXL_CNTRL_PUSH_AC_ENABLE
SHELL_CMD_EXPORT_ALIAS(cmd_ac_set, ac_set, access_category config);
#endif
#ifdef CFG_FOR_COEXISTENCE_TEST_STOPAP_PATCH
SHELL_CMD_EXPORT_ALIAS(cmd_ap_stop, ap_stop, ap stop);
#endif
SHELL_CMD_EXPORT_ALIAS(wifi_scan_cmd, wifi_scan, wifi scan);
SHELL_CMD_EXPORT_ALIAS(wifi_connect_cmd, wifi_sta_connect, wifi station connect);
SHELL_CMD_EXPORT_ALIAS(wifi_disconnect_cmd, wifi_sta_disconnect, wifi station disconnect);
SHELL_CMD_EXPORT_ALIAS(lwip_cmd, lwip, show stats);
#if NX_FHOST_MONITOR
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sniffer_on, wifi_sniffer_on, wifi sniffer on);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sniffer_off, wifi_sniffer_off, wifi sniffer off);
#endif
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_state_get, wifi_state, get wifi state);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sta_rssi_get, wifi_sta_rssi, get wifi sya rssi);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sta_channel_get, wifi_sta_channel, get wifi channel);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sta_ssid_passphr_get, wifi_sta_ssid_passphr_get, get wifi ssid password);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_sta_mac_get, wifi_sta_mac_get, get wifi sta mac);
SHELL_CMD_EXPORT_ALIAS(wifi_enable_autoreconnect_cmd, wifi_sta_autoconnect_enable, wifi station enable auto reconnect);
SHELL_CMD_EXPORT_ALIAS(wifi_disable_autoreconnect_cmd, wifi_sta_autoconnect_disable, wifi station disable auto reconnect);
SHELL_CMD_EXPORT_ALIAS(wifi_sta_ps_on_cmd, wifi_sta_ps_on, wifi sta powersave mode on);
SHELL_CMD_EXPORT_ALIAS(wifi_sta_ps_off_cmd, wifi_sta_ps_off, wifi sta powersave mode off);
SHELL_CMD_EXPORT_ALIAS(wifi_sta_info_cmd, wifi_sta_info, wifi sta info);
SHELL_CMD_EXPORT_ALIAS(wifi_ap_sta_list_get_cmd, wifi_sta_list, get sta list in AP mode);
SHELL_CMD_EXPORT_ALIAS(wifi_ap_sta_delete_cmd, wifi_sta_del, delete one sta in AP mode);
SHELL_CMD_EXPORT_ALIAS(wifi_mgmr_ap_start_cmd, wifi_ap_start, start AP mode);
SHELL_CMD_EXPORT_ALIAS(wifi_mgmr_ap_stop_cmd, wifi_ap_stop, stop AP mode);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_mac_get, wifi_ap_mac_get, get wifi ap mac);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_ap_conf_max_sta, wifi_ap_conf_max_sta, config AP mac sta);
SHELL_CMD_EXPORT_ALIAS(cmd_wifi_raw_send, wifi_raw_send, wifi raw send test);
#ifdef CONFIG_CLI_WIFI_DUBUG
SHELL_CMD_EXPORT_ALIAS(cmd_wifi, wifi, wifi);
#endif
SHELL_CMD_EXPORT_ALIAS(wifi_sta_static_ipv4, set_ipv4, ipc task set);
#if NX_IPERF
SHELL_CMD_EXPORT_ALIAS(cmd_iperf, iperf, iperf test throughput);
#endif
SHELL_CMD_EXPORT_ALIAS(cmd_rc, rc, Print the Rate Control Table);
SHELL_CMD_EXPORT_ALIAS(cmd_rate, rate, set g_fw_rate);
#endif

int bl_wifi6_cli_init(void)
{
    // static command(s) do NOT need to call aos_cli_register_command(s) to register.
    // However, calling aos_cli_register_command(s) here is OK but is of no effect as cmds_user are included in cmds list.
    // XXX NOTE: Calling this *empty* function is necessary to make cmds_user in this file to be kept in the final link.
    //return aos_cli_register_commands(cmds_user, sizeof(cmds_user)/sizeof(cmds_user[0]));
    return 0;
}
