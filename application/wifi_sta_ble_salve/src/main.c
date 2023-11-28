#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define EXAMPLE_AXK_WIFI_STA_SSID               CONFIG_AXK_WIFI_STA_SSID
#define EXAMPLE_AXK_WIFI_STA_PASS               CONFIG_AXK_WIFI_STA_PASS

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ble_slave_thread = NULL;

#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

#define BLE_ADV_MIN    100
#define BLE_ADV_MAX    120

static aiio_ble_default_handle_t srv_handle;
static aiio_ble_conn_t cur_conn=-1;
static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};
uint8_t adv_data[] = {
    /* Flags */
    0x02,             /* length */
    0x01,             /* type="Flags" */
    0x02 | 0x04,      /* GENERAL | BREDR_NOT_SUPPORTED */
    /* UUID */
    0x05,             /* length */
    0x03,             /* type="16-bit UUID, all listed" */
    0xa0,             /* type="16-bit UUID, all listed" */
    0x01,             /* type="16-bit UUID, all listed" */
    0xb0,             /* type="16-bit UUID, all listed" */
    0x01,             /* type="16-bit UUID, all listed" */
    /* Name */
    0x04,             /* length */
    0x09,             /* type="Complete name" */
    'A','X','K'
};

static aiio_ble_default_server_t ble_server = {
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x01),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x03),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
};

aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = BLE_ADV_MIN,
    .interval_max = BLE_ADV_MAX,
};

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");
            cur_conn = -1;
            aiio_ble_adv_start(&param, adv_data, sizeof adv_data, NULL, 0);
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");
            cur_conn = event->connect.conn;
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

static void ble_slave_task(void *pvParameters)
{
    aiio_ble_send_t send_param;
    char send_data[] = "test";

    while(1) {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));

        if (cur_conn!=-1) {
            /* ble send data */
            send_param.conn = cur_conn;
            send_param.handle = srv_handle.tx_char_handle[0];
            send_param.type = AIIO_BLE_NOTIFY;
            send_param.data = (uint8_t *)send_data;
            send_param.length = (uint16_t)sizeof send_data;
            if (aiio_ble_send(&send_param) != AIIO_OK) {
                aiio_log_w("send data fail");
            }
        }
    }

    aiio_os_thread_delete(NULL);
}


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

static void test_ble_slave_thread(void *pvParameters)
{
    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);

    ble_gatts_add_default_svcs(&ble_server, &srv_handle);

    aiio_ble_adv_start(&param, adv_data, sizeof adv_data, NULL, 0);

    aiio_os_thread_create(NULL, "ble_task", ble_slave_task, 4096, NULL, 15);

    aiio_ble_register_event_cb(aiio_ble_cb);

    ble_print_mac();

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

    aiio_wifi_register_event_cb(cb_wifi_event);

    ret=aiio_wifi_init();
    if(ret!=0){
        aiio_log_e("wifi init error!!");
    }

    aiio_os_thread_create(&aiio_test_sta_wifi_thread, "test_sta_wifi_entry", test_sta_wifi_entry, 4096, NULL, 15);
    aiio_os_thread_create(&aiio_test_ble_slave_thread, "test_ble_slave_thread", test_ble_slave_thread, 4096, NULL, 1);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

