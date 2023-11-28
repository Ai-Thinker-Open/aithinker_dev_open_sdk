#include "hello_h1.h"
#include "hello_h2.h"

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


static aiio_os_thread_handle_t *aiio_test_ap_wifi_thread = NULL;


static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
static uint8_t sta_num=0;
uint8_t mac[MAC_LEN];

aiio_wifi_sta_basic_info_t sta_info_test[CONFIG_MAX_STA_CONN_LIMIT]={0};

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

            break;
        case AIIO_WIFI_EVENT_AP_START:
            aiio_log_d("<<<<<<<<< start soft ap OK<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STOP:
            aiio_log_d("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");
    
            aiio_wifi_config_t test_wifi_ap_config;
        
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
            for(uint8_t i=0; i<sta_num;i++){
                aiio_log_d("wifi ap_sta_idx =  %u!!",sta_info_test[i].sta_idx);
                aiio_log_d("wifi ap_sta_mac = %02x%02x%02x%02x%02x%02x!!",
                            sta_info_test[i].sta_mac[0],
                            sta_info_test[i].sta_mac[1],
                            sta_info_test[i].sta_mac[2],
                            sta_info_test[i].sta_mac[3],
                            sta_info_test[i].sta_mac[4],
                            sta_info_test[i].sta_mac[5]);
            }

            break;
        case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
            aiio_log_d("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
            
            break;
        default:
            break;
    }
}



static void test_ap_wifi_entry(void *pvParameters)
{
    
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
                },
            };

    aiio_wifi_set_config(AIIO_WIFI_IF_AP,&wifi_ap_config);

    aiio_wifi_start();

    aiio_os_thread_delete(aiio_test_ap_wifi_thread);
}


void aiio_main(void *params){

    int32_t ret=0;

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_d("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_wifi_register_event_cb(cb_wifi_event);
    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
    }

    aiio_os_thread_create(&aiio_test_ap_wifi_thread, "test_ap_wifi_entry", test_ap_wifi_entry, 4096, NULL, 15);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

