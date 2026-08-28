#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#include "mbedtls/platform.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/md5.h"
#include "mbedtls/debug.h"
#include "ota_hal.h"
#include "ota_parse.h"
#include "ota_config.h"
#include "ai_ota.h"
#include "http_parser.h"

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

#define STA_USE_STATIC_IP       0

#define AI_CONFIG_OTA_THREAD_SIZE	(1024*8*4)	//OTA升级任务堆栈大小
#define AI_CONFIG_OTA_THREAD_PRIORITY	(3)	//OTA升级任务优先级(0最低)

#define url CONFIG_OTA_URL

void axk_hal_user_ota_update(void);

static int32_t ret=0;
static int32_t rssi=0;
static int32_t state=0;
uint8_t mac[MAC_LEN];

typedef enum
{
    AT_OTA_MODE_HTTP,
    AT_OTA_MODE_HTTPS,
    AT_OTA_MODE_ANY,
} at_ota_update_mode_t;

static ota_parame ota_param;
static int ota_type;
static char ota_host[256];
static char ota_resource[256];
static aiio_os_thread_handle_t *aiio_at_OTAThreadHandle = NULL;

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ota_thread = NULL;

aiio_wifi_mode_t cur_wifi_mode=AIIO_WIFI_MODE_NULL;

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
            
            aiio_wifi_rssi_get(&rssi);
            aiio_log_d("wifi cur_rssi = %d!!",rssi);
            aiio_wifi_sta_mac_get(mac);
            aiio_log_d("wifi mac = %02x%02x%02x%02x%02x%02x!!",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
            aiio_wifi_sta_connect_ind_stat_get(&wifi_ind_stat);
            aiio_log_d("wifi sta_bssid = %02x%02x%02x%02x%02x%02x!!",wifi_ind_stat.bssid[0],wifi_ind_stat.bssid[1],wifi_ind_stat.bssid[2],wifi_ind_stat.bssid[3],wifi_ind_stat.bssid[4],wifi_ind_stat.bssid[5]);

            aiio_os_thread_create(&aiio_test_ota_thread, "aiio_test_ota_thread", axk_hal_user_ota_update, 4096, NULL, 15);
            break;
        case AIIO_WIFI_EVENT_AP_START:
            aiio_log_d("<<<<<<<<< start soft ap OK<<<<<<<<<<<");

            break;
        case AIIO_WIFI_EVENT_AP_STOP:
            aiio_log_d("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");

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



void axk_hal_user_ota_update(void)
{
    int port;
    char schema[8] = {0};
    struct http_parser_url purl;
    uint16_t length;

    aiio_os_tick_dealy(aiio_os_ms2tick(2000));

    http_parser_url_init(&purl);
    length=strlen((char*)url);

    int parser_status = http_parser_parse_url(url, length, 0, &purl);

    if (parser_status != 0) {
        printf("[OTA] Error parse url:%s\r\n", url);
        goto exit;
    }

    memset(ota_host, 0, sizeof(ota_host));
    memset(ota_resource, 0, sizeof(ota_resource));

    if (purl.field_data[UF_SCHEMA].len > 8) {
        printf("[OTA] schema ovfl \r\n");
        goto exit;
    }
    memcpy(schema, url + purl.field_data[UF_SCHEMA].off, purl.field_data[UF_SCHEMA].len);
    if (strcasecmp(schema, "http") == 0) {
        ota_type = AT_OTA_MODE_HTTP;
        port = 80;
    } else if (strcasecmp(schema, "https") == 0) {
        ota_type = AT_OTA_MODE_HTTPS;
        port = 443;
    } else {
        printf("[OTA] undef schema\r\n");
        goto exit;
    }
    port = purl.port ? purl.port : port;
    memcpy(ota_host, url + purl.field_data[UF_HOST].off, purl.field_data[UF_HOST].len);
    memcpy(ota_resource, url + purl.field_data[UF_PATH].off, purl.field_data[UF_PATH].len);

    aiio_log_i("[OTA] port:%d host:%s path:%s\r\n", port, ota_host, ota_resource);

    ota_param = ai_ota_parame_init(ota_host, port, ota_resource);

    if (ota_type == AT_OTA_MODE_HTTP) {
        aiio_os_thread_create(&aiio_at_OTAThreadHandle, "http OTA", ai_http_update_ota, AI_CONFIG_OTA_THREAD_SIZE, &ota_param, AI_CONFIG_OTA_THREAD_PRIORITY);
    }
    else if (ota_type == AT_OTA_MODE_HTTPS){
        aiio_os_thread_create(&aiio_at_OTAThreadHandle, "https OTA", ai_https_update_ota, AI_CONFIG_OTA_THREAD_SIZE, &ota_param, AI_CONFIG_OTA_THREAD_PRIORITY);
    }
    
exit:
    aiio_os_thread_delete(aiio_test_ota_thread);

}

static void test_sta_wifi_entry(void *pvParameters)
{
    
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = EXAMPLE_AXK_WIFI_STA_SSID,
            .password = EXAMPLE_AXK_WIFI_STA_PASS,
            .channel = EXAMPLE_AXK_WIFI_STA_CHANNEL,
            .use_dhcp=1,
        },
    };
    #if STA_USE_STATIC_IP
        aiio_wifi_sta_ip_set("192.168.31.240");
        wifi_sta_config.sta.use_dhcp=0;
    #endif
    // memcpy(wifi_sta_config.sta.bssid,bssid,sizeof(bssid));

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();

    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
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

    aiio_nvs_init();

    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
    }

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

