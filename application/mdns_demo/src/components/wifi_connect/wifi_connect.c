#include "aiio_adapter_include.h"
#include "mdns_test.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS
#define EXAMPLE_AXK_WIFI_STA_CHANNEL            CONFIG_AXK_WIFI_STA_CHANNEL

#define ERROR_COED_SUCCESS  (0)
#define ERROR_COED_WIFI_INIT_ERROR  (-1)
#define ERROR_COED_WIFI_TASK_CREATE_ERROR  (-2)

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;

static aiio_wifi_conf_t aiio_conf_test =
{
    .country_code = "CN",
};

typedef struct test_wifi_item {
    char ssid[32];
    uint32_t ssid_len;
    uint8_t bssid[6];
    uint8_t channel;
    uint8_t auth;
    int8_t rssi;
} test_wifi_item_t;

static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
uint8_t mac[MAC_LEN];
aiio_wifi_mode_t cur_wifi_mode=AIIO_WIFI_MODE_NULL;
aiio_wifi_ip_params_t sta_ip_pra={0};
aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};

static void cb_wifi_event(aiio_input_event_t *event, void* data)
{
    int32_t ret=0;
    // uint8_t bssid[6] = {0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};
    switch (event->code) {
        case AIIO_WIFI_EVENT_WIFI_READY:
            aiio_log_d("<<<<<<<<<  WIFI INIT OK <<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_SCAN_DONE:
            aiio_log_d("<<<<<<<<<  SCAN DONE OK <<<<<<<<<<");

            aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

            // aiio_wifi_sta_mac_set(testmac);

            aiio_wifi_set_country_code(aiio_conf_test.country_code);

            aiio_wifi_config_t wifi_sta_config = {
                .sta = {
                    .ssid = EXAMPLE_AXK_WIFI_STA_SSID,
                    .password = EXAMPLE_AXK_WIFI_STA_PASS,
                    .channel = EXAMPLE_AXK_WIFI_STA_CHANNEL,
                    .use_dhcp=1,
                    .flags=0,
                },
            };
            // memcpy(wifi_sta_config.sta.bssid,bssid,sizeof(bssid));

            aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

            aiio_wifi_start();

            aiio_wifi_get_country_code(aiio_conf_test.country_code);

            aiio_wifi_get_mode(&cur_wifi_mode);

            aiio_log_d("wifi cur_mode = %d!!",cur_wifi_mode);

            aiio_log_d("wifi cur_countrycode = %s!!",aiio_conf_test.country_code);

            break;
        case AIIO_WIFI_EVENT_STA_CONNECTING:
            aiio_log_d("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
            break;
        case AIIO_WIFI_EVENT_STA_CONNECTED:
            aiio_log_d("<<<<<<<<< CONNECTED<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_STA_DISCONNECTED:
            aiio_log_d("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
            if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE ||
                    event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE){
                //connect timeout
                aiio_log_d("connect timeout");
            }else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                    event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                    event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION){
                //password error
                aiio_log_d("password error");
            }else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL){
                //not found AP
                aiio_log_d("not found AP");
            }else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH)){
                //wifi disconnect
                aiio_log_d("wifi disconnect");
            }else{
                //connect error
                aiio_log_d("connect error");
            }
            break;
        case AIIO_WIFI_EVENT_STA_GOT_IP:
            aiio_log_d("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");
            aiio_wifi_sta_ip_get(&sta_ip_pra.ip,&sta_ip_pra.gateway,&sta_ip_pra.netmask);
            // aiio_log_d("GOT %s:\"%s\"", "sta_ip", ip4addr_ntoa(&sta_ip_pra.ip));
            // aiio_log_d("GOT %s:\"%s\"", "sta_gateway", ip4addr_ntoa(&sta_ip_pra.gateway));
            // aiio_log_d("GOT %s:\"%s\"", "sta_netmask", ip4addr_ntoa(&sta_ip_pra.netmask));
            aiio_wifi_rssi_get(&rssi);
            aiio_log_a("wifi cur_rssi = %d!!",rssi);
            aiio_wifi_sta_mac_get(mac);
            aiio_log_a("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            aiio_wifi_sta_connect_ind_stat_get(&wifi_ind_stat);
            aiio_log_a("wifi sta_bssid = %02x%02x%02x%02x%02x%02x!!",wifi_ind_stat.bssid[0],wifi_ind_stat.bssid[1],wifi_ind_stat.bssid[2],wifi_ind_stat.bssid[3],wifi_ind_stat.bssid[4],wifi_ind_stat.bssid[5]);
            ret=start_mdns_test();
            if(ret<0)
            {
                aiio_log_a("start_mdns_test error %d!",ret);
            }
            else
            {
                aiio_log_a("start_mdns_test success %d!",ret);
            }
            break;
        case AIIO_WIFI_EVENT_AP_START:
            aiio_log_d("<<<<<<<<< start soft ap OK<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STOP:
            aiio_log_d("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");
            aiio_wifi_config_t test_wifi_sta_config;
            aiio_wifi_config_t test_wifi_ap_config;
            aiio_wifi_get_config(AIIO_WIFI_IF_STA,&test_wifi_sta_config);
            aiio_log_d("sta.ssid=%s",test_wifi_sta_config.sta.ssid);
            aiio_log_d("sta.password=%s",test_wifi_sta_config.sta.password);
            aiio_log_d("sta.bssid=%02x:%02x:%02x:%02x:%02x:%02x",test_wifi_sta_config.sta.bssid[0],test_wifi_sta_config.sta.bssid[1],test_wifi_sta_config.sta.bssid[2],test_wifi_sta_config.sta.bssid[3],test_wifi_sta_config.sta.bssid[4],test_wifi_sta_config.sta.bssid[5]);
            aiio_log_d("sta.band=%d",test_wifi_sta_config.sta.band);
            aiio_log_d("sta.channel=%d",test_wifi_sta_config.sta.channel);
            aiio_log_d("sta.use_dhcp=%d",test_wifi_sta_config.sta.use_dhcp);
            aiio_log_d("sta.flags=%d",test_wifi_sta_config.sta.flags);

            aiio_wifi_get_config(AIIO_WIFI_IF_AP,&test_wifi_ap_config);

            aiio_log_d("ap.ssid=%s",test_wifi_ap_config.ap.ssid);
            aiio_log_d("ap.password=%s",test_wifi_ap_config.ap.password);
            aiio_log_d("ap.max_connection=%d",test_wifi_ap_config.ap.max_connection);
            aiio_log_d("ap.channel=%d",test_wifi_ap_config.ap.channel);
            aiio_log_d("ap.ssid_hidden=%d",test_wifi_ap_config.ap.ssid_hidden);
            aiio_log_d("ap.ssid_len=%d",test_wifi_ap_config.ap.ssid_len);
            aiio_log_d("ap.use_dhcp=%d",test_wifi_ap_config.ap.use_dhcp);
            aiio_log_d("ap.dhcp_params.lease_time=%d",test_wifi_ap_config.ap.dhcp_params.lease_time);
            aiio_log_d("ap.dhcp_params.start=%d",test_wifi_ap_config.ap.dhcp_params.start);
            aiio_log_d("ap.dhcp_params.end=%d",test_wifi_ap_config.ap.dhcp_params.end);

            break;
        case AIIO_WIFI_EVENT_AP_STACONNECTED:
            aiio_log_d("<<<<<<<<< station connent ap <<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
            aiio_log_d("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
            break;
        default:
            break;
    }
}

static void scan_item_cb(aiio_wifi_ap_item_t *env, uint32_t *param1, aiio_wifi_ap_item_t *item)
{
    test_wifi_item_t wifi_item;
    void (*complete)(void *) = (void (*)(void *))param1;
    static uint32_t counter=0;

    wifi_item.auth = item->auth;
    wifi_item.rssi = item->rssi;
    wifi_item.channel = item->channel;
    wifi_item.ssid_len = item->ssid_len;
    memcpy(wifi_item.ssid, item->ssid, sizeof(wifi_item.ssid));
    memcpy(wifi_item.bssid, item->bssid, sizeof(wifi_item.bssid));
    aiio_log_i("index[%02d]: channel %02u, bssid %02X:%02X:%02X:%02X:%02X:%02X, rssi %3d, auth %d, SSID %s",
                    counter,
                    wifi_item.channel,
                    wifi_item.bssid[0],
                    wifi_item.bssid[1],
                    wifi_item.bssid[2],
                    wifi_item.bssid[3],
                    wifi_item.bssid[4],
                    wifi_item.bssid[5],
                    wifi_item.rssi,
                    wifi_item.auth,
                    wifi_item.ssid
            );
    counter++;
    if(counter>=50){
        counter=0;
    }

    if (complete) {
        complete(&wifi_item);
    }
}

static void scan_complete_cb(void *p_arg, void *param)
{
    aiio_wifi_scan_ap_all(NULL, p_arg, scan_item_cb);
}

static void test_sta_wifi_entry(void *pvParameters)
{
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);
    ret=aiio_wifi_scan(NULL,scan_complete_cb,0);
    if(ret!=0){
        aiio_log_e("wifi scan error!!");
    }
    aiio_os_tick_dealy(aiio_os_ms2tick(3000));

    while(1){
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
        aiio_wifi_get_state(&state);
        aiio_log_i("wifi cur_state = 0x%x!!",state);
        if(state==AIIO_WIFI_STATE_CONNECTED_IP_GOT){
            aiio_log_d("wifi got ip !!");
            break;
        }
    }
    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

int32_t start_connect_wifi()
{
    int32_t ret=0;

	aiio_wifi_register_event_cb(cb_wifi_event);
    aiio_log_d("start wifi init");
    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
        return ERROR_COED_WIFI_INIT_ERROR;
    }
    aiio_log_d("wifi ok");

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    if(NULL==aiio_test_sta_wifi_thread){
        return ERROR_COED_WIFI_TASK_CREATE_ERROR;
    }else{
        return ERROR_COED_SUCCESS;
    }
}
