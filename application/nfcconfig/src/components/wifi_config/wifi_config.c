#include "aiio_adapter_include.h"
#include "wifi_config_comm_service.h"
#include "wifi_config.h"
#include "aiio_wifi.h"

int g_apConnected=0;

static aiio_os_thread_handle_t *aiio_mdns_thread = NULL;

static int isAPInit = 0; /* AP是否打开 */
static int isWifiConnected = 0;
static aiio_wifi_conf_t aiio_conf_test =
    {
        .country_code = "CN",
};

int aiio_wifi_connect_status_get(void)
{
    return isWifiConnected;
}

int aiio_wifi_ap_status_get(void)
{
    return isAPInit;
}

void aiio_device_mdns_start(void *pvParameters)
{
    udp_radiate_result();
    aiio_os_tick_dealy(aiio_os_ms2tick(100));

    aiio_mdns_config_t mdns_config =
        {
            .service_type = "_aithinker",
            .port = 602,
        };
    uint8_t mac_address[6] = {0};
    aiio_wifi_sta_mac_get(mac_address);
    char mac_str[17] = {0};

    aiio_sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
    uint32_t ip, gw, mask;
    aiio_wifi_sta_ip_get(&ip, &gw, &mask);
    mdns_config.host_name = aiio_os_malloc(38);
    aiio_sprintf(mdns_config.host_name, "axk_%s_%d.%d.%d.%d_%d", mac_str, ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, mdns_config.port);
    aiio_mdns_init(mdns_config);
    aiio_os_free(mdns_config.host_name);

    int send_num = 60;
    while (send_num--)
    {
        udp_radiate_result();
        aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    }
    aiio_os_thread_delete(NULL);
}

static void wifi_event_handler(aiio_input_event_t *event, void *data)
{
    switch (event->code)
    {
    case AIIO_WIFI_EVENT_WIFI_READY:
        /* code */
        break;
    case AIIO_WIFI_EVENT_SCAN_DONE:
        /* code */
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTING:
        aiio_log_d("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
        /* code */
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTED:
        aiio_log_d("<<<<<<<<< CONNECTED<<<<<<<<<<<");
        /* code */
        break;
    case AIIO_WIFI_EVENT_STA_DISCONNECTED:
        aiio_log_d("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
        if (isWifiConnected == true)
        {
            aiio_mdns_deinit();
        }
        isWifiConnected = 0;
        /* code */
        break;
    case AIIO_WIFI_EVENT_STA_GOT_IP:
        aiio_log_d("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");
        aiio_os_thread_create(&aiio_mdns_thread, "mdns_task", aiio_device_mdns_start, 4 * 1024, NULL, 8);
        isWifiConnected = 1;
        /* code */
        break;
    case AIIO_WIFI_EVENT_AP_START:
        isAPInit = 1;
        /* code */
        break;
    case AIIO_WIFI_EVENT_AP_STOP:
        isAPInit = 0;
        /* code */
        break;
    case AIIO_WIFI_EVENT_AP_STACONNECTED:
        /* code */
		aiio_log_d("\r\n\r\nAIIO_WIFI_EVENT_AP_STACONNECTED\r\n\r\n\r\n");
		g_apConnected++;
        break;
    case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
		aiio_log_d("\r\n\r\nAIIO_WIFI_EVENT_AP_STACONNECTED\r\n\r\n\r\n");
		g_apConnected--;
        /* code */
        break;
    case AIIO_WIFI_EVENT_MAX:
        /* code */
        break;
    default:
        break;
    }
}

/**
 * @brief NFC distribution network initialization
 *
 */
void aiio_nfcconfig_init(void)
{
    aiio_wifi_ap_ip_set("192.168.4.1");
    aiio_wifi_register_event_cb(wifi_event_handler);
    aiio_wifi_init();
    aiio_wifi_set_country_code(aiio_conf_test.country_code);
}

/**
 * @brief
 *
 */
void aiio_softAP_start(void)
{
    aiio_err_t ret=-1;
    if (!isAPInit)
    {
        aiio_wifi_config_t wifi_config = {0};
        char mac[6] = {0};

        aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);
        //ret=aiio_wifi_ap_mac_get(&mac);
        ret=aiio_wifi_sta_mac_get(mac);
        if(0!=ret){
            aiio_log_e("get sta mac error\r\n");
            return;
        }
        aiio_log_a("mac:%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        ret=aiio_wifi_ap_mac_set(mac);
        if(0!=ret){
            aiio_log_e("set ap mac error\r\n");
            return;
        }
        aiio_sprintf(&wifi_config.ap.ssid, "axk_light_%02x%02x%02x", mac[3], mac[4], mac[5]);
        wifi_config.ap.channel = 6;
        wifi_config.ap.max_connection = 1;
		wifi_config.ap.use_dhcp=1;
		wifi_config.ap.dhcp_params.lease_time=120;
		wifi_config.ap.dhcp_params.start=2;
		wifi_config.ap.dhcp_params.end=100;
        strcpy((char *)wifi_config.ap.password, "");
        aiio_wifi_set_config(AIIO_WIFI_IF_AP, &wifi_config);
        aiio_wifi_start();
        isAPInit = 1;
    }
}

void aiio_softAP_stop(void)
{
    isAPInit = 0;
    aiio_wifi_set_mode(AIIO_WIFI_MODE_NULL);
}

/**
 * @brief Device connected to WiFi
 *
 * @param config WiFi parameters
 *
 * @return aiio_ret_t
 * @retval AIIO_OK      WiFi connect success
 * @retval AIIO_ERROR   WiFi connect error
 */
aiio_ret_t aiio_wifistation_connect(wifi_sta_config_t config)
{
    int ret = 0;

    aiio_wifi_ap_stop();
    isAPInit = 0;
    aiio_os_tick_dealy(aiio_os_ms2tick(200));

    ret = aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);
    if (ret != AIIO_OK)
    {
        aiio_log_e("wifi set mode failed: %d", ret);
        return AIIO_ERROR;
    }

    aiio_wifi_config_t wifi_config = {0};

    strcpy(&wifi_config.sta.ssid, (char *)config.ssid);
    if (config.password[0] != 0)
    {
        strcpy(&wifi_config.sta.password, (char *)config.password);
    }

    if (config.bssid[0] != 0)
    {
        // strncpy(&wifi_config.sta.bssid, (char *)config.bssid,6);
    }
	wifi_config.sta.use_dhcp=1;
	wifi_config.sta.flags=0;
    aiio_wifi_set_config(AIIO_WIFI_IF_STA, &wifi_config);
    aiio_log_d("start connect,ssid=%s pw=%s channel=%d\r\n",wifi_config.sta.ssid,wifi_config.sta.password,wifi_config.sta.channel);
    ret = aiio_wifi_start();
    if (ret != AIIO_OK)
    {
        aiio_log_e("wifi connect failed: %d", ret);
        return AIIO_ERROR;
    }

    return AIIO_OK;
}
