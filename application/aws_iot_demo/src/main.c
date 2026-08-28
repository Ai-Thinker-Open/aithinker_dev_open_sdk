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

const char root_ca[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n"
"ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n"
"b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n"
"MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n"
"b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n"
"ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n"
"9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n"
"IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n"
"VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n"
"93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n"
"jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n"
"AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n"
"A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n"
"U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n"
"N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n"
"o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n"
"5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n"
"rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"
"-----END CERTIFICATE-----\r\n";

const char device_cert[] = "-----BEGIN CERTIFICATE-----\r\n"
"MIIDWTCCAkGgAwIBAgIUOKzq23KYQubSk3WdXDNZIdDfpVcwDQYJKoZIhvcNAQEL\r\n"
"BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\r\n"
"SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTIzMTIwODA4MTc1\r\n"
"OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\r\n"
"ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMfKaEq0qTrpkWWIxbr1\r\n"
"/yUNRrQ2SPecG8dW+7UBgF3q074lE1tY+2sb8qH3UAo/pyNCUIwKQIRA4tlb7zZc\r\n"
"otmFTeUpPnB0BLTd0WSQYtMbO6ToVS2r4k7UjohC6nvdbFYJZeTfCn4SylfxcHs/\r\n"
"FAD6m1mlkZFO8rdrAVLqAJU2dzmnrPNPJZ8iymYwAgx8UDPRBFV5E1GlmvH4TRTD\r\n"
"bT5RzLZ5PEDDp0KMrHdZTfL+u7NW0TLOfmKCMGZ8E3twYcH9Pl6XVVC/AnIwI/q/\r\n"
"krdvgLWcLtCcFL7qcQMXiFR/75i7JmnmhEEi/7bP+OhMvZ141xZUp4l5B1TUwoR4\r\n"
"exkCAwEAAaNgMF4wHwYDVR0jBBgwFoAUDJ2HOx0lMAs07KS2nc+MRrIOl3gwHQYD\r\n"
"VR0OBBYEFFHPPe1glpDi1h9ZjUSccrIMt/KyMAwGA1UdEwEB/wQCMAAwDgYDVR0P\r\n"
"AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBDcoxUkU/CublOjbsdWJ52bLgb\r\n"
"AwS8FITXQZpRLyA7jUOWFFZg7LVaYjJB8Uyw3uML0J7uiNHIRZz4t+Cd/MFObTkA\r\n"
"MvVR/foMm/4HZr3HV6XUoyuUyenSex1OLoCs3reZMEvfvwyE7EtKpoWDjPtjduNY\r\n"
"I0wXa4rSJfZus/md9HHoCO92TIvFTTtSyqkg35itMegd0SbQJMTf7qYzL6H79Q6w\r\n"
"ptE6nGFmKPbEqMUfV0zjizd8d1OxCScecJsOMG2Kwvw85oCve0dCB3WxlhhK3Fk0\r\n"
"DX+i7FGiiRCNIbYHGANrApEVlkD6kxNM7AJ7B0KSCRBAX3E+d92oOe1Za4el\r\n"
"-----END CERTIFICATE-----\r\n";

const char private_key[] = "-----BEGIN RSA PRIVATE KEY-----\r\n"
"MIIEpQIBAAKCAQEAx8poSrSpOumRZYjFuvX/JQ1GtDZI95wbx1b7tQGAXerTviUT\r\n"
"W1j7axvyofdQCj+nI0JQjApAhEDi2VvvNlyi2YVN5Sk+cHQEtN3RZJBi0xs7pOhV\r\n"
"LaviTtSOiELqe91sVgll5N8KfhLKV/Fwez8UAPqbWaWRkU7yt2sBUuoAlTZ3Oaes\r\n"
"808lnyLKZjACDHxQM9EEVXkTUaWa8fhNFMNtPlHMtnk8QMOnQoysd1lN8v67s1bR\r\n"
"Ms5+YoIwZnwTe3Bhwf0+XpdVUL8CcjAj+r+St2+AtZwu0JwUvupxAxeIVH/vmLsm\r\n"
"aeaEQSL/ts/46Ey9nXjXFlSniXkHVNTChHh7GQIDAQABAoIBAHheHc9EYOC5b3u4\r\n"
"l0g4hOuRwBM60WM8jg/e++7lZK0wmTlppOvYBEFXqSqjkXlbOBsqDNUSKCSF4yYJ\r\n"
"GWmrDgH3KiLyYkfl3iAJs+neg2ukcJ69yvOlJsdPkD/P8O2HTYnYR6dIKU4ji/zH\r\n"
"3XijXC3qzkWIVtcwtVKECVwm1wsNnyAnCbzO+Zgql7m4TpcGiI7wYOdaxwviPmSi\r\n"
"A0/tDxPq2SJOz1TQDLTCofaMsqxprEcxYnwQyvl3JfqI+kVrhQU49uBZsQebDG9l\r\n"
"GEbEmVnBJSyHQ7hDM3htaUrovA4NHMAcmsTvs+Z10y3b7QZ8BWNmL6kzGsxps+0T\r\n"
"2twgi3ECgYEA/bBU3OyxH0xrAHFzqB6i8G2eBSrwKIWoJp3w4LkX4zzlXKTXQ7WD\r\n"
"+p5AvoT/3VhJB1sjsh0L2Xf4Qvxw6MIRp+SK4R69V8LlZpWh33gqvRZWtnSjhKhr\r\n"
"caJ21dXunTA/Hlw/aslApJEbkVR/c04MOThtKWn1BJ4suiaIsO3AMyMCgYEAyZxf\r\n"
"EUCTjrfwdON9Mtmc5b4Z/zGd9eErk8So/vQcaaipvYCA14F5tMEJT9eKroC/PnC/\r\n"
"uks9+CAhudhtiAwJq7nTYoFvs6HYVniEJ7/ij0DqUoDLHzqIYB/lKZS6tBCCaeEO\r\n"
"JAeyIqmcrIZ5Hd/ny76TEBlVKKqxG15UUPkRSpMCgYEAtfnd9hfnOj9T5h57y1Ym\r\n"
"pXbMSoKxgS/XHdRf7KN5SiztjlZEgDJduNH6OXAoJfvlbtxmr6Z8UIPpVwvEZoKH\r\n"
"rbqYEMI/KgwJCFw/oJCPf9HdY8YpYch6Qio7OkrafnEUZc/gQdaZgzL/5nEJd4m9\r\n"
"5PHgBDwYYTaheOqZVOlG3KcCgYEAjVPtnJVtGs28ZOQXnq7HepFxfd/VQEiRGKvy\r\n"
"X47V4AdYvbu4FdnwbBIpqeuJjtw3s9irvjcCgM5ddf7OgWEW4meIU/QyTnT0x2EC\r\n"
"Gy867XrvB9i7CvAoK2IQbb6RxqoQEZKvK/itGflWF+bIPnX38m5sfwuTE8fcEThl\r\n"
"2MFQKrsCgYEA7Mv0JL9FBV2a3q+9iuwFZ+a/exdjOHk9UCmSiMlYLbAPKmixLRZb\r\n"
"+cVwyc9Y3SHT5mqHF0ZW2sKwL/kXo8IOxumsW6NnyDCnWKwdLORsWOlovk/+hOIC\r\n"
"Fk6onF1IPRlP4JMbTQDFaxnDUN3mr9w6vfl/jK0NioNDzkIVAenFEdo=\r\n"
"-----END RSA PRIVATE KEY-----\r\n";



#define BROKER_URI_TCP "mqtt://mqtt.eclipseprojects.io"
#define BROKER_URI_SSL "mqtts://a2i4jqjtr9fl1t-ats.iot.us-east-1.amazonaws.com"//"mqtts://mqtt.eclipseprojects.io:8883"
#define BROKER_URI_WS "ws://mqtt.eclipseprojects.io:80/mqtt"
#define BROKER_URI_WSS "wss://mqtt.eclipseprojects.io:443/mqtt"
#define BROKER_URI_SSL_PSK "mqtts://mqttsserver"

#define         DEVICE_FW_VERSION           ("1.0.1")

static aiio_os_thread_handle_t *aiio_test_sta_wifi_thread = NULL;
static aiio_os_thread_handle_t *aiio_test_ap_wifi_thread = NULL;

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

static bool mqtt_connect_flag = false;


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        aiio_log_e("Last error %s: 0x%x", message, error_code);
    }
}

static aiio_err_t event_cb(aiio_mqtt_event_handle_t event)
{
    int32_t event_id;
    aiio_mqtt_client_handle_t client = event->client;

    event_id = event->event_id;
    aiio_log_i("Event dispatched, event_id=%d", event_id);
    int msg_id;
    switch ((aiio_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        aiio_log_i("MQTT_EVENT_CONNECTED");
        mqtt_connect_flag = true;
        msg_id = aiio_mqtt_client_subscribe(client, "/topic/qos0", 0);
        aiio_log_i("sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        aiio_log_i("MQTT_EVENT_DISCONNECTED");

        break;
    case MQTT_EVENT_SUBSCRIBED:
        aiio_log_i("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = aiio_mqtt_client_publish(client, "/topic/qos1", "subscribe /topic/qos0 ok!", 0, 0, 0);
        aiio_log_i("sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        aiio_log_i("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        aiio_log_i("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        aiio_log_i("MQTT_EVENT_DATA");
        aiio_log_i("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        aiio_log_i("SUBSCRIBED DATA=%.*s\r\n", event->data_len, event->data);
        msg_id = aiio_mqtt_client_publish(client, "/topic/qos1", "welcome aws iot!", 0, 0, 0);
        aiio_log_i("PUBLISHED DATA= welcome aws iot!\r\n");
        aiio_log_i("sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_ERROR:
        aiio_log_i("MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->aiio_tls_last_aiio_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->aiio_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->aiio_transport_sock_errno);
            aiio_log_i("Last errno string (%s)", strerror(event->error_handle->aiio_transport_sock_errno));
        }
        break;
    default:
        aiio_log_i("Other event id:%d", event->event_id);
        break;
    }
    return AIIO_OK;
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

            if(!mqtt_connect_flag){
                aiio_mqtt_client_config_t mqtt_cfg = {
                    .event_handle = event_cb,
                };
                mqtt_cfg.uri = BROKER_URI_SSL;   //MQTTS
                mqtt_cfg.port = 8883;
                mqtt_cfg.cert_pem = root_ca;
                mqtt_cfg.client_cert_pem = device_cert;
                mqtt_cfg.client_key_pem = private_key;

                // mqtt_cfg.host="192.168.202.10";
                
                aiio_log_i("[%s()-%d]MQTT connect config\r\n",__func__,__LINE__);
                aiio_log_i("  host:%s:%d\r\n",mqtt_cfg.uri,mqtt_cfg.port);

                aiio_mqtt_client_handle_t client = aiio_mqtt_client_init(&mqtt_cfg);
                aiio_mqtt_client_start(client);
            }

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
            .channel = EXAMPLE_AXK_WIFI_STA_CHANNEL,
            .use_dhcp=1,
            .flags=0,
        },
    };
    // memcpy(wifi_sta_config.sta.bssid,bssid,sizeof(bssid));

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_sta_config);

    aiio_wifi_start();

    aiio_os_thread_delete(aiio_test_sta_wifi_thread);
}


void aiio_main(void *params){

    int32_t ret=0;

    aiio_log_i("device version: %s \r\n", DEVICE_FW_VERSION);

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

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

