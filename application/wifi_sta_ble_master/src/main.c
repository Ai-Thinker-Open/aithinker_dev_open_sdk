#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS

#ifdef CONFIG_AXK_WIFI_SSID_HIDDEN_ENABLE
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     1
#else
    #define EXAMPLE_AXK_WIFI_SSID_HIDDEN_ENABLE     0
#endif

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ble_master_thread = NULL;
static uint8_t sta_num=0;

#define BLE_CONN_TIMEOUT  10000
#define BLE_TARGET_UUID16 0x01A0

#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

#define BLE_SCAN_INTV   100
#define BLE_SCAN_WINDOW 50

#define AT_BLE_LOCAL_NAME_SIZE  29

static uint8_t target_mac[6];

static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};

struct ble_adv_field {
    uint8_t length;
    uint8_t type;
    uint8_t value[];
};
typedef int32_t (* ble_adv_parse_func_t) (const struct ble_adv_field *, void *);
static int32_t ble_adv_parse(const uint8_t *data, uint8_t length,
                 ble_adv_parse_func_t func, void *user_data)
{
    const struct ble_adv_field *field;

    while (length > 1) {
        field = (const void *) data;

        if (field->length >= length) {
            return AIIO_ERROR;
        }

        if (func(field, user_data) == AIIO_OK) {
            return AIIO_OK;
        }

        length -= 1 + field->length;
        data += 1 + field->length;
    }

    return AIIO_OK;
}

static int32_t ble_name_parse(const struct ble_adv_field *field, void *user_data)
{
    char *name = (char *)user_data;

#define BT_DATA_NAME_SHORTENED          0x08 /* Shortened name */
#define BT_DATA_NAME_COMPLETE           0x09 /* Complete name */
    switch (field->type) {
        case BT_DATA_NAME_SHORTENED:
        case BT_DATA_NAME_COMPLETE:
            memcpy(name, field->value, field->length - 1);
            return AIIO_OK;
        default:
            return AIIO_ERROR;
    }
}

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");
            break;
        case AIIO_BLE_EVT_MTU:
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            break;
        case AIIO_BLE_EVT_DATA:
            aiio_log_i("data down handle:%d len:%d,DATA:", event->data.handle, event->data.length);
            for(int i=0; i<event->data.length; i++)
            {
                printf("%c", event->data.data[i]);
            }
            printf("\r\n");
            break;
        case AIIO_BLE_EVT_SCAN: {
            char name[AT_BLE_LOCAL_NAME_SIZE] = {0};

            if (event->scan.ad_len > 0) {
                ble_adv_parse(event->scan.ad_data, event->scan.ad_len, ble_name_parse, name);
            }
            if (name[0] == '\0' && event->scan.sd_len > 0) {
                ble_adv_parse(event->scan.sd_data, event->scan.sd_len, ble_name_parse, name);
            }

            if (name[0] != '\0' && strcmp(name, "AXK") == 0) {
                /* stop scan */
                aiio_ble_scan_stop();
                memcpy(target_mac, event->scan.mac, 6);
                aiio_log_i("scan AXK, rssi:%d", event->scan.rssi);
            }
            break;
        }
        default:
            aiio_log_w("undef event!");
    }
}

void ble_print_mac(void)
{
    uint8_t mac[6];

    aiio_ble_get_mac(mac);
    aiio_log_i("ble mac:%02x:%02x:%02x:%02x:%02x:%02x",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}


aiio_ble_scan_param_t scan_cfg = {
    .type = AIIO_BLE_SCAN_TYPE_ACTIVE,
    .interval = BLE_SCAN_INTV,
    .window = BLE_SCAN_WINDOW,
};
void ble_master_init(void)
{
    aiio_ble_init(AIIO_BLE_MODE_CENTRAL, &ble_config);

    if (aiio_ble_scan(&scan_cfg) != AIIO_OK) {
        aiio_log_i("scan fail");
        return ;
    }
}


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


static void test_sta_wifi_entry(void *pvParameters)
{
    
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = EXAMPLE_AXK_WIFI_STA_SSID,
            .password = EXAMPLE_AXK_WIFI_STA_PASS,
            .use_dhcp=1,
        },
    };

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();

    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}

static void test_ble_master_thread(void *pvParameters)
{
    uint8_t empty_mac[6] = {0};

    aiio_ble_register_event_cb(aiio_ble_cb);

    ble_master_init();

    ble_print_mac();

    while(1) {
        aiio_os_tick_dealy(aiio_os_ms2tick(100));

        if (memcmp(target_mac, empty_mac, 6) == 0) {
            continue;
        }

        aiio_log_i("start connect mac:%02x:%02x:%02x:%02x:%02x:%02x",
               target_mac[0], target_mac[1], target_mac[2], target_mac[3], target_mac[4], target_mac[5]);
        if (aiio_ble_connect(AIIO_BLE_ADDR_PUBLIC, target_mac, BLE_CONN_TIMEOUT) != AIIO_OK) {
            aiio_log_e("ble connect fail!");
            continue;
        }
        break;
    }

    aiio_os_thread_delete(aiio_test_ble_master_thread);
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

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    aiio_os_thread_create(&aiio_test_ble_master_thread, "test_ble_slave_thread", test_ble_master_thread, 4096, NULL, 1);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

