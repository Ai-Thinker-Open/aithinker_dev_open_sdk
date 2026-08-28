// #include "hello_h1.h"
// #include "hello_h2.h"

#include "aiio_adapter_include.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS
#define EXAMPLE_AXK_WIFI_STA_CHANNEL            CONFIG_AXK_WIFI_STA_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_SSID                CONFIG_AXK_WIFI_AP_SSID
#define EXAMPLE_AXK_WIFI_AP_PASS                CONFIG_AXK_WIFI_AP_PASS
#define EXAMPLE_AXK_WIFI_MAX_STA_CONN           CONFIG_AXK_WIFI_MAX_STA_CONN
#define EXAMPLE_AXK_WIFI_AP_CHANNEL             CONFIG_AXK_WIFI_AP_CHANNEL
#define EXAMPLE_AXK_WIFI_AP_IP                  CONFIG_AXK_WIFI_AP_IP
#ifdef CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     1
#else
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     0
#endif

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ap_wifi_thread = NULL;

static aiio_os_thread_handle_t *aiio_test_ble_slave_thread = NULL;

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
static int32_t ap_stop_flag=0;
static uint8_t sta_num=0;
uint8_t mac[MAC_LEN];
uint8_t testmac[MAC_LEN]={0x18,0xB9,0x05,0x00,0x00,0x01};
aiio_wifi_mode_t cur_wifi_mode=AIIO_WIFI_MODE_NULL;
aiio_wifi_ip_params_t sta_ip_pra={0};
aiio_wifi_sta_basic_info_t sta_info_test[CONFIG_MAX_STA_CONN_LIMIT]={0};
aiio_wifi_ip_params_t ap_ip_pra={0};
aiio_wifi_sta_connect_ind_stat_info_t wifi_ind_stat={0};

//ble
uint8_t adv_data[27] = {
    /* Flags */
    0x02,             /* length */
    0x01,             /* type="Flags" */
    0x02 | 0x04,      /* GENERAL | BREDR_NOT_SUPPORTED */
    /* Manufacturer Specific */
    0x2,             /* length */
    0xFF,             /* type="Manufacturer Specific Data" */
    0x55,
    /* Name */
    0x04,             /* length */
    0x09,             /* type="Complete name" */
    'A','X','K',
};
uint8_t rsp_data[31] = {
    /* Name */
    // 0x04,             /* length */
    // 0x09,             /* type="Complete name" */
    // 'a','x','k',
};

aiio_ble_uuid_128_t server_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,0x77, 0x88, 0x99,  0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
aiio_ble_uuid_128_t tx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,0x77, 0x88, 0x99,  0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};
aiio_ble_uuid_128_t rx_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_128, .val = {0x02, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66,0x77, 0x88, 0x99,  0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff}};

static aiio_ble_default_handle_t srv_handle;


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
            aiio_log_d("wifi cur_rssi = %d!!",rssi);
            aiio_wifi_sta_mac_get(mac);
            aiio_log_d("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            aiio_wifi_sta_connect_ind_stat_get(&wifi_ind_stat);
            aiio_log_d("wifi sta_bssid = %02x%02x%02x%02x%02x%02x!!",wifi_ind_stat.bssid[0],wifi_ind_stat.bssid[1],wifi_ind_stat.bssid[2],wifi_ind_stat.bssid[3],wifi_ind_stat.bssid[4],wifi_ind_stat.bssid[5]);
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
            aiio_wifi_ap_sta_list_get(&sta_num,&sta_info_test);
            aiio_log_d("wifi ap_sta_num =  %d!!",sta_num);
            for(uint8_t i=0; i<CONFIG_MAX_STA_CONN_LIMIT;i++){
                if(sta_info_test[i].rssi != 0){
                    aiio_log_d("wifi ap_sta_idx =  %u!!",sta_info_test[i].sta_idx);
                    aiio_log_d("wifi ap_sta_mac = %02x%02x%02x%02x%02x%02x!!",sta_info_test[i].sta_mac[0],sta_info_test[i].sta_mac[1],sta_info_test[i].sta_mac[2],sta_info_test[i].sta_mac[3],sta_info_test[i].sta_mac[4],sta_info_test[i].sta_mac[5]);
                    aiio_log_d("wifi ap_sta_rssi = %d !!",sta_info_test[i].rssi);
                }
            }

            break;
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
            aiio_log_d("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
            ap_stop_flag=1;
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


static void test_ap_wifi_entry(void *pvParameters)
{
    uint8_t apmac[MAC_LEN]={0};
    ret=aiio_wifi_ap_ip_set(EXAMPLE_AXK_WIFI_AP_IP);
    if(ret!=0){
        aiio_log_e("wifi ap_ip_set error!!");
    }

    aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);

    aiio_wifi_config_t wifi_ap_config = {
                .ap = {
                    .ssid = EXAMPLE_AXK_WIFI_AP_SSID,
                    .password = EXAMPLE_AXK_WIFI_AP_PASS,
                    .channel=EXAMPLE_AXK_WIFI_AP_CHANNEL,
                    .max_connection=EXAMPLE_AXK_WIFI_MAX_STA_CONN,
                    .ssid_hidden=EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE,
                    .ssid_len=strlen(EXAMPLE_AXK_WIFI_AP_SSID),
                    .use_dhcp=1,
                    .dhcp_params.lease_time=120,
                    .dhcp_params.start=2,
                    .dhcp_params.end=100,
                },
            };

    aiio_wifi_set_config(AIIO_WIFI_IF_AP,&wifi_ap_config);

    aiio_wifi_start();

    aiio_wifi_ap_ip_get(&ap_ip_pra.ip,&ap_ip_pra.gateway,&ap_ip_pra.netmask);
    aiio_wifi_ap_mac_get(apmac);
    aiio_log_d("wifi ap_mac = %02x%02x%02x%02x%02x%02x!!",apmac[0],apmac[1],apmac[2],apmac[3],apmac[4],apmac[5]);
    // aiio_log_d("GOT %s:\"%s\"", "ap_ip", ip4addr_ntoa(&ap_ip_pra.ip));
    // aiio_log_d("GOT %s:\"%s\"", "ap_gateway", ip4addr_ntoa(&ap_ip_pra.gateway));
    // aiio_log_d("GOT %s:\"%s\"", "ap_netmask", ip4addr_ntoa(&ap_ip_pra.netmask));

    aiio_wifi_get_mode(&cur_wifi_mode);

    aiio_log_d("wifi cur_mode = %d!!",cur_wifi_mode);

    while(1){
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
        if(ap_stop_flag==1){
            ap_stop_flag=0;
            ret=aiio_wifi_stop();
            if(ret!=0){
                aiio_log_e("wifi ap_stop error!!");
                break;
            }
            aiio_log_d("wifi ap_stop success!!");
            break;
        }
    }

    aiio_os_thread_delete(aiio_test_ap_wifi_thread);
}

static void test_sta_wifi_entry(void *pvParameters)
{
    uint8_t bssid[6] = {0, 0, 0, 0, 0, 0};
    uint16_t channels[6] = {1,2,3,4,5,6};
    aiio_wifi_scan_params_t scan_config={
        .channel_num=6,
        .duration_scan=(142 * 1000),
        .scan_mode=AIIO_SCAN_ACTIVE,
    };

    memcpy(scan_config.bssid,bssid,sizeof(bssid));
    memcpy(scan_config.channels,channels,sizeof(channels));
    memset(scan_config.ssid,0,sizeof(scan_config.ssid));
    ret=aiio_wifi_scan_adv(NULL, scan_complete_cb, 0, &scan_config);

    ret=aiio_wifi_scan(NULL,scan_complete_cb,0);
    if(ret!=0){
        aiio_log_e("wifi scan error!!");
    }
    aiio_os_tick_dealy(aiio_os_ms2tick(3000));

    while(1){
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
        aiio_wifi_get_state(&state);
        aiio_log_d("wifi cur_state = 0x%x!!",state);
        if(state==AIIO_WIFI_STATE_CONNECTED_IP_GOT){
            aiio_log_d("wifi got ip !!");
            aiio_os_tick_dealy(aiio_os_ms2tick(5000));
            ret=aiio_wifi_stop();
            if(ret!=0){
                aiio_log_e("wifi sta stop error!!");
                break;
            }
            aiio_log_d("wifi sta stop success!!");


            aiio_os_thread_create(&aiio_test_ap_wifi_thread, "test_ap_wifi_entry", test_ap_wifi_entry, 4096, NULL, 15);

            break;
        }
    }
    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");

            aiio_log_i("\r\n+EVENT:BLE_DISCONNECT\r\n");

            aiio_ble_restart_advertising();
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");

            aiio_log_i("\r\n+EVENT:BLE_CONNECT\r\n");
            break;
        case AIIO_BLE_EVT_MTU:
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            break;
        case AIIO_BLE_EVT_DATA:
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);

            printf("\r\n+DATA:");
            for(int i=0; i<event->data.length; i++)
            {
                printf("%c,", event->data.data[i]);
            }
            printf("\r\n");
            break;
        default:
            aiio_log_w("undef event:%d", event->type);
    }
}
static aiio_ble_default_server_t cfg = { 
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x01),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x03),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
};
static void test_ble_slave_thread(void *pvParameters)
{
    int ret = 0;
    aiio_ble_config_t ble_config = {0};
    aiio_ble_adv_param_t adv_param = {0};
    // aiio_ble_default_server_t cfg = {
    //     .server_uuid[0] = (aiio_ble_uuid_t *)&server_uuid,
    //     .tx_char_uuid[0] = (aiio_ble_uuid_t *)&tx_uuid,
    //     .rx_char_uuid[0] = (aiio_ble_uuid_t *)&rx_uuid,
    // };


    ret = aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
    if(ret != AIIO_OK){
        aiio_log_e("ble init fail!!");
    }

    if (ble_gatts_add_default_svcs(&cfg, &srv_handle) != AIIO_OK) {
        aiio_log_e("ble gatts add fail!!");
    }
    adv_param.conn_mode = AIIO_BLE_CONN_MODE_NON;
    adv_param.disc_mode = AIIO_BLE_DISC_MODE_GEN;
    adv_param.interval_min = 20;
    adv_param.interval_max = 50;
    ret = aiio_ble_adv_start(&adv_param, adv_data, 11, rsp_data, 2);
    if(ret != AIIO_OK){
        aiio_log_e("ble start adv fail!!");
    }


    aiio_ble_register_event_cb(&aiio_ble_cb);
    while(1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    }
    aiio_os_thread_delete(aiio_test_ble_slave_thread);
}


void aiio_main(void *params){

    int32_t ret=0;

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
    }

    aiio_wifi_register_event_cb(cb_wifi_event);

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    aiio_os_thread_create(&aiio_test_ble_slave_thread, "test_ble_slave_thread", test_ble_slave_thread, 4096, NULL, 15);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

