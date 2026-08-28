#include "axk_wifi_init.h"
#include "axk_nvs_info.h"
#include "aiio_adapter_include.h"
#include "axk_http_server.h"
#include "axk_mqtt.h"
#include "axk_udp.h"
#include "axk_at.h"
#include "axk_at_network.h"
#include "aiio_os_port.h"
#include "aiio_blufi.h"
#include "axk_sleep.h"

#include <string.h>
#include <stdarg.h>

#define AXK_WIFI_AP_IP          CONFIG_AXK_WIFI_AP_IP
#define AXK_WIFI_AP_SSID        CONFIG_AXK_WIFI_AP_SSID
#define AXK_WIFI_AP_PWD         CONFIG_AXK_WIFI_AP_PASS
#define AXK_WIFI_AP_CH          CONFIG_AXK_WIFI_AP_CHANNEL
#define AXK_WIFI_AP_MAX_CONN    CONFIG_AXK_WIFI_MAX_STA_CONN

static int32_t axk_wifi_init(void);
static int32_t axk_wifi_set_sta_info(const axk_sta_info_t *info);
static void axk_wifi_set_mode(axk_wifi_mode_t mode);
static axk_wifi_mode_t axk_wifi_get_mode(void);
static int32_t axk_wifi_start_connect(void);
static uint8_t axk_get_ap_start_flag(void);
static axk_sta_info_t axk_wifi_get_sta_info(void);

static axk_wifi_mode_t g_wifi_mode;
static uint8_t ap_start_flag = 0;
static aiio_wifi_sta_basic_info_t sta_info_test[1] = {0};
static uint8_t sta_num = 0;
static axk_sta_info_t g_sta_info = {0};
static uint8_t auto_web_cfg_flag = 0;
axk_wifi_init_t g_axk_wifi =
{
    .got_ip = 0,
    .init = axk_wifi_init,
    .set_sta_info = axk_wifi_set_sta_info,
    .get_mode = axk_wifi_get_mode,
    .set_mode = axk_wifi_set_mode,
    .start_connect = axk_wifi_start_connect,
    .get_ap_start_flag = axk_get_ap_start_flag,
};

static void axk_wifi_connected_cb(void)
{
    axk_wifi_cfg_type_t cfg_type;

    g_axk_wifi.got_ip = 1;
    g_axk_wifi.discon_cnt = 0;
    cfg_type = g_axk_network.get_wifi_cfg_type();
    switch (cfg_type)
    {
    case WIFI_CFG_APP:
        g_axk_udp.wifi_cfg_udp_sendto((uint8_t *)UDP_CFG_WIFI_SUCCEED, strlen(UDP_CFG_WIFI_SUCCEED));
        break;
    
    default:
        break;
    }

    axk_at_printf("WIFI CONNECTED\r\n");

#if CONFIG_AXK_AUTO_SLEEP
    //连接上网络启动睡眠定时器
    axk_sleep_timer_reset();
#endif

    aiio_blufi_wifi_event(AIIO_WIFI_EVENT_STA_CONNECTED, NULL);

    if ((cfg_type != WIFI_CFG_NONE) && (cfg_type != WIFI_CFG_BLE))
    {
        //save sta info
        g_axk_network.set_wifi_cfg_type((uint8_t)WIFI_CFG_NONE);
        g_nvs_info.set_wifi_mode(AXK_WIFI_STA);
        aiio_wifi_channel_get(&g_sta_info.channel);
        aiio_log_i("g_sta_info.channel: %d", g_sta_info.channel);
        axk_sta_info_t sta_info = axk_wifi_get_sta_info();
        g_nvs_info.set_sta_info(sta_info);
    }

    // 打印断开wifi超过10次开启的web配网，需要在连接完毕后关闭
    if (auto_web_cfg_flag != 0)
    {
        auto_web_cfg_flag = 0;
        g_axk_http_server.deinit();
        if (g_axk_wifi.get_ap_start_flag() != 0)
        {
            if (g_axk_udp.get_udp_handle() == NULL)
            {
                //关闭热点
                aiio_wifi_ap_stop();
            }
        }
    }
}

static void axk_wifi_disconnect_cb(void)
{
    g_axk_wifi.got_ip = 0;
    if (g_axk_wifi.discon_cnt++ >= 10)
    {
        auto_web_cfg_flag = 1;
        g_axk_wifi.discon_cnt = 0;
        //开启网页配网
        if (g_axk_wifi.get_ap_start_flag() == 0)
        {
            //启动网页配网
            g_axk_wifi.set_mode(AXK_WIFI_AP);
            g_axk_wifi.init();
        }

        g_axk_http_server.init();
    }

#if CONFIG_AXK_AUTO_SLEEP
    //没连接网络，不休眠
    axk_sleep_timer_stop();
#endif

    axk_at_printf("WIFI DISCONNECT\r\n");

    aiio_blufi_wifi_event(AIIO_WIFI_EVENT_STA_DISCONNECTED, NULL);
}

static void axk_wifi_got_ip_cb(void)
{
    aiio_blufi_wifi_event(AIIO_WIFI_EVENT_STA_GOT_IP, NULL);
    // if (g_axk_mqtt.get_start_flag() == 0)
    {
        g_axk_mqtt.init();
    }
}

static void cb_wifi_event(aiio_input_event_t *event, void *data)
{
    int32_t ret = 0;
    // uint8_t bssid[6] = {0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5};
    switch (event->code)
    {
    case AIIO_WIFI_EVENT_WIFI_READY:
        aiio_log_i("<<<<<<<<<  WIFI INIT OK <<<<<<<<<<");
        break;
    case AIIO_WIFI_EVENT_SCAN_DONE:
        aiio_log_i("<<<<<<<<<  SCAN DONE OK <<<<<<<<<<");
        aiio_blufi_wifi_event(AIIO_WIFI_EVENT_SCAN_DONE, NULL);
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTING:
        aiio_log_i("<<<<<<<<< STA_CONNECTING <<<<<<<<<<<");
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTED:
        aiio_log_i("<<<<<<<<< CONNECTED<<<<<<<<<<<");
        axk_wifi_connected_cb();
        break;
    case AIIO_WIFI_EVENT_STA_DISCONNECTED:
        aiio_log_i("<<<<<<<<<  DISCONNECTED <<<<<<<<<<");
        axk_wifi_disconnect_cb();
        if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_ERROR_PSK_TIMEOUT_FAILURE ||
            event->value == AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE)
        {
            // connect timeout
            aiio_log_i("connect timeout");
        }
        else if (event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE ||
                 event->value == AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_ALLOCATE_FAIILURE ||
                 event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION)
        {
            // password error
            aiio_log_i("password error");
        }
        else if (event->value == AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL)
        {
            // not found AP
            aiio_log_i("not found AP");
        }
        else if ((event->value == AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_CONNECTION) || (event->value == AIIO_WLAN_FW_DISCONNECT_BY_USER_WITH_DEAUTH))
        {
            // wifi disconnect
            aiio_log_i("wifi disconnect");
        }
        else
        {
            // connect error
            aiio_log_i("connect error");
        }
        
        break;
    case AIIO_WIFI_EVENT_STA_GOT_IP:
        aiio_log_i("<<<<<<<<< CONNECTED GOT IP <<<<<<<<<<<");
        axk_wifi_got_ip_cb();
        break;
    case AIIO_WIFI_EVENT_AP_START:
        aiio_log_i("<<<<<<<<< start soft ap OK<<<<<<<<<<<");
        ap_start_flag = 1;
        // g_axk_http_server.init();
        break;
    case AIIO_WIFI_EVENT_AP_STOP:
        aiio_log_i("<<<<<<<<< stop soft ap OK<<<<<<<<<<<");
        ap_start_flag = 0;
        break;
    case AIIO_WIFI_EVENT_AP_STACONNECTED:
        aiio_log_i("<<<<<<<<< station connent ap <<<<<<<<<<<");
        break;
    case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
        aiio_log_i("<<<<<<<<< station disconnet ap <<<<<<<<<<<");
        break;
    default:
        break;
    }
}

static int32_t wifi_ap_init(void)
{
    int32_t ret;
    aiio_wifi_config_t wifi_config = {0};

    //set ip
    ret = aiio_wifi_ap_ip_set(AXK_WIFI_AP_IP);
    if (ret != 0)
    {
        ret = -1;
        aiio_log_e("wifi ap_ip_set error!!");
        goto __exit;
    }

    //set wifi mode
    aiio_wifi_set_mode(AIIO_WIFI_MODE_AP);
    // g_nvs_info.set_wifi_mode(AXK_WIFI_AP);

    //ap cfg
    memcpy(wifi_config.ap.ssid, AXK_WIFI_AP_SSID, strlen(AXK_WIFI_AP_SSID));
    memcpy(wifi_config.ap.password, AXK_WIFI_AP_PWD, strlen(AXK_WIFI_AP_PWD));
    if (g_sta_info.channel != 0)
    {
        wifi_config.ap.channel = g_sta_info.channel;
    }
    else
    {
        wifi_config.ap.channel = AXK_WIFI_AP_CH;
    }
    wifi_config.ap.max_connection = AXK_WIFI_AP_MAX_CONN;
    wifi_config.ap.ssid_hidden = 0;
    wifi_config.ap.ssid_len = strlen(AXK_WIFI_AP_SSID);
    wifi_config.ap.use_dhcp = 1;
    wifi_config.ap.dhcp_params.lease_time = 120;
    wifi_config.ap.dhcp_params.start = 2;
    wifi_config.ap.dhcp_params.end = 100;

    aiio_wifi_set_config(AIIO_WIFI_IF_AP,&wifi_config);

__exit:
    return ret;
}

static int32_t wifi_sta_init(void)
{
    int32_t ret;
    aiio_wifi_config_t wifi_config = {0};
#if 0
    axk_sta_info_t sta_info = {0};

    memcpy(sta_info.ssid, "HUAWEI", sizeof("HUAWEI"));
    memcpy(sta_info.pwd, "12345678909", sizeof("12345678909"));

    ret = g_nvs_info.set_sta_info((const axk_sta_info_t)sta_info);
    if (ret != 0)
    {
        aiio_log_e("nvs set sta info failed");
        goto __exit; 
    }

    memset(&sta_info, 0, sizeof(sta_info));
#endif
    if (strlen((const char *)g_sta_info.ssid) == 0)
    {
        //get sta nvs info
        ret = g_nvs_info.get_sta_info(&g_sta_info);
        if (ret != 0)
        {
            aiio_log_e("nvs get sta info failed");
            goto __exit;
        }
    }

    //set wifi mode
    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    //sta cfg
    memcpy(wifi_config.sta.ssid, g_sta_info.ssid, sizeof(g_sta_info.ssid));
    memcpy(wifi_config.sta.password, g_sta_info.pwd, sizeof(g_sta_info.pwd));
    wifi_config.sta.channel = g_sta_info.channel;   //指定信道连接
    wifi_config.sta.use_dhcp = 1;
    wifi_config.sta.flags |= WIFI_CONNECT_PMF_CAPABLE;

    aiio_wifi_set_config(AIIO_WIFI_IF_STA,&wifi_config);

__exit:
    return ret;
}

static int32_t axk_wifi_init(void)
{
    int32_t ret;

    if ((g_wifi_mode != AXK_WIFI_AP) && (g_wifi_mode != AXK_WIFI_STA))
    {
        ret = -1;
        aiio_log_e("nvs wifi mode is not AP or STA");
        goto __exit;
    }
    aiio_log_i("wifi mode:%s", g_wifi_mode == AXK_WIFI_AP ? "AP" : "STA");

    //init wifi
    ret = aiio_wifi_init();
    if (ret != 0)
    {
        aiio_log_w("The wifi protocol stack has been initialized");
    }

    //register wifi event cb
    aiio_wifi_register_event_cb(cb_wifi_event);

    if (g_wifi_mode == AXK_WIFI_AP)
    {
        aiio_wifi_ap_stop();

        ret = wifi_ap_init();
        if (ret != 0)
        {
            aiio_log_e("wifi init ap failed");
            goto __exit;
        }
    }
    else if (g_wifi_mode == AXK_WIFI_STA)
    {
        aiio_wifi_disconnect();

        ret = wifi_sta_init();
        if (ret != 0)
        {
            aiio_log_e("wifi init sta failed");
            goto __exit;
        }
    }

    //start wifi
    aiio_wifi_start();

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_wifi_set_sta_info(const axk_sta_info_t *info)
{
    int32_t ret = -1;

    if (info == NULL)
    {
        goto __exit;
    }

    memcpy(&g_sta_info, info, sizeof(axk_sta_info_t));

    aiio_log_i("ssid:%s", g_sta_info.ssid);
    aiio_log_i("pwd:%s", g_sta_info.pwd);

    ret = 0;

__exit:
    return ret;
}

static axk_sta_info_t axk_wifi_get_sta_info(void)
{
    return g_sta_info;
}

static void axk_wifi_set_mode(axk_wifi_mode_t mode)
{
    g_wifi_mode = mode;
}

static axk_wifi_mode_t axk_wifi_get_mode(void)
{
    return g_wifi_mode;
}

static uint8_t axk_get_ap_start_flag(void)
{
    return ap_start_flag;
}

static void axk_wifi_start_task(void *param)
{
    aiio_os_tick_dealy(100);
    axk_wifi_set_mode(AXK_WIFI_STA);
    axk_wifi_init();

    aiio_os_thread_delete(NULL);
}

static int32_t axk_wifi_start_connect(void)
{
    return aiio_os_thread_create(NULL, "wifi_conn", axk_wifi_start_task, 2048, NULL, 10);
}