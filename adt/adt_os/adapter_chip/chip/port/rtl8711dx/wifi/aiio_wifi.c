//标准库
#include <string.h>
//中间件相关
#include "aiio_type.h"
#include "aiio_wifi.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_at_wifi_api.h"
//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
// #include <wifi_conf.h>
#include "wifi_intf_drv_to_app_internal.h"
#include "wifi_api_types.h"
#include "wifi_api_event.h"
//第三方库
#include <lwip_netconf.h>
#include <dhcp/dhcps.h>
#include "wifi_api_ext.h"

#define MAX_SCAN_NUM    (60)
static aiio_sniffer_cb_t aiio_sniffer_cb = NULL;
static aiio_wifi_mode_t s_wifi_mode = AIIO_WIFI_MODE_NULL;
static aiio_event_cb wifi_cb = NULL;
static aiio_wifi_config_t wifi_config = {0};
static int32_t s_wifi_state = AIIO_WIFI_STATE_UNKNOWN;
static aiio_os_thread_handle_t rtl_wifi_connect_thread_handle = NULL;
static aiio_wifi_sta_basic_info_t aiio_sta_info = {0};
static aiio_wifi_sta_basic_info_t aiio_sta_info_temp[3] = {0};

//注意RTL8711端序
//LwIP_SetIP设置时需要使用bswap_32()转换一下端序
static aiio_wifi_ip_params_t ap_ip_params =
{
    .ip = 0x012BA8C0,       //192.168.43.1
    .gateway = 0x012BA8C0,  //192.168.43.1
    .netmask = 0x00ffffff,  //255.255.255.0
};
static aiio_wifi_ip_params_t sta_ip_params = { 0 };
static aiio_wifi_ap_item_t *sp_ap_item = NULL;
static bool s_wifi_scan_done = false;	//用来记录扫描是否完成
static uint8_t s_scan_ap_num = 0;
static uint8_t g_enable_wifi_event_cb=1;	//是否使能wifi断开回调的使能

//SDK相关的
static struct rtw_network_info wifi = {0};   //这个是从at指令那边copy来的
static struct rtw_softap_info ap = {0};  //这个是从at指令那边copy来的
uint8_t ai_wifi_init_ok = 0;  //原名称 s_wifi_init_ok 中间件标记wifi状态的，sdk初始化完成后需要设置该标志位
u32 use_hw_crypto_func=0;   //原始SDK中需要使用的
extern struct netif xnetif[NET_IF_NUM];
extern struct static_ip_config user_static_ip;

//EAP企业加密相关 get config arguments from wifi_eap_config.c
extern char *eap_target_ssid;
extern char *eap_identity;
extern char *eap_password;
extern const unsigned char *eap_ca_cert;
extern const unsigned char *eap_client_cert;
extern const unsigned char *eap_client_key;
extern int eap_ca_cert_len;
extern int eap_client_cert_len;
extern int eap_client_key_len;
extern char *eap_client_key_pwd;

static void *aiio_socked_ssl_malloc(size_t nelements, size_t elementSize);
static uint8_t aiio_socked_ssl_free(void* ptr);
static aiio_os_function_return_t rtl_wifi_connect_thread(void *params);
static void init_wifi_struct(void);
static aiio_err_t aiio_wifi_ap_start(void);
static s32 app_scan_result_handler(unsigned int scanned_AP_num, void *user_data);
static void aiio_wifi_client_connected_cb(char *buf, int buf_len, int flags, void *userdata);
static void aiio_wifi_client_disconnected_cb(char *buf, int buf_len, int flags, void *userdata);
static aiio_wifi_beacon_auth_t rtl_security2aiio(enum rtw_security rtl_security_val);
static void aiio_wifi_join_status_cb(char *buf, int buf_len, int flags, void *userdata);

uint32_t bswap_32(uint32_t x){
    x=((x<<8)&0xff00ff00)|((x>>8)&0x00ff00ff);
    return (x>>16)|(x<<16);
}
aiio_err_t aiio_wifi_init(void)
{
    aiio_log_i("aiio_wifi_init");
    //WPA3加密需要初始化ssl
// #if MBEDTLS_VERSION_MAJOR != 3
//     mbedtls_platform_set_calloc_free(aiio_socked_ssl_malloc, aiio_socked_ssl_free);
// #endif
    use_hw_crypto_func = 0;

    wifi_init(); // 初始化wifi

    aiio_log_d("wait wifi init");
    while (ai_wifi_init_ok == 0)
    {
        vTaskDelay(10);
    }
    if (wifi_cb)
    {
        aiio_log_i("wifi init ok");
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_WIFI_READY;
        wifi_cb(&aiio_event, NULL);
    }else{
        aiio_log_w("wifi_cb not init");
    }
    // AP模式下连接、断开回调
    wifi_reg_event_handler(RTW_EVENT_STA_ASSOC, aiio_wifi_client_connected_cb, NULL);
    wifi_reg_event_handler(RTW_EVENT_STA_DISASSOC, aiio_wifi_client_disconnected_cb, NULL);
    wifi_reg_event_handler(RTW_EVENT_JOIN_STATUS,aiio_wifi_join_status_cb, NULL);    //20240903同步SDK更新wifi回调事件
    s_wifi_mode = AIIO_WIFI_MODE_STA;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_register_event_cb(aiio_event_cb cb)
{
    wifi_cb = cb;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_unregister_event_cb(void)
{
    wifi_cb = NULL;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_set_mode(aiio_wifi_mode_t mode)
{
    aiio_err_t ret = AIIO_OK;
    uint8_t rtl_wifiMode=RTW_MODE_NONE;

    if (ai_wifi_init_ok == 0)
    {
        aiio_log_e("The wifi is not initialized!!!");
        ret = AIIO_ERROR;
        goto __err;
    }

    if (mode == s_wifi_mode)
    {
        aiio_log_d("There is no need to switch modes, mode(%d)", s_wifi_mode);
        goto __err;
    }

    if (mode >= AIIO_WIFI_MODE_MAX)
    {
        aiio_log_e("This mode is not supported! mode:%d", mode);
        ret = AIIO_ERROR;
        goto __err;
    }

    switch (mode)
    {
        case AIIO_WIFI_MODE_NULL:
            //wifi_off(); //WAIT_TODO 原始SDK好像没有实现
            ret = AIIO_OK;
            goto __err;
        case AIIO_WIFI_MODE_STA:
            rtl_wifiMode=RTW_MODE_STA;
            break;
        case AIIO_WIFI_MODE_AP:
            rtl_wifiMode=RTW_MODE_AP;
            break;
        case AIIO_WIFI_MODE_APSTA:
            rtl_wifiMode=RTW_MODE_NAN;
            break;
        default:
            ret = AIIO_ERROR;
            goto __err;
    }
    //wifi_off();   //WAIT_TODO 原始SDK好像没有实现
    vTaskDelay(20);
    if (wifi_on(rtl_wifiMode) < 0)
    {
        s_wifi_mode = AIIO_WIFI_MODE_NULL;
        ret = AIIO_ERROR;
        goto __err;
    }
    s_wifi_mode = mode;
    ret = AIIO_OK;

__err:
    return ret;
}

aiio_err_t aiio_wifi_get_mode(aiio_wifi_mode_t *mode)
{
    if (mode == NULL)
    {
        return AIIO_ERROR;
    }

    if (ai_wifi_init_ok == 0)
    {
        return AIIO_ERROR;
    }

    *mode = s_wifi_mode;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_set_config(aiio_wifi_port_t port, aiio_wifi_config_t *conf)
{
    if (conf == NULL)
    {
        aiio_log_e("The conf is null error");
        return AIIO_ERROR;
    }

    if (port == AIIO_WIFI_IF_STA)
    {   // sta config
        memset(&wifi_config.sta, 0, sizeof(wifi_config.sta));
        if (conf->sta.ssid)
        {
            strncpy((char *)wifi_config.sta.ssid, (char *)conf->sta.ssid, SSID_LEN);
        }
        if (conf->sta.password)
        {
            strncpy((char *)wifi_config.sta.password, (char *)conf->sta.password, PASSWORD_LEN);
        }
        if (conf->sta.bssid)
        {
            strncpy((char *)wifi_config.sta.bssid, (char *)conf->sta.bssid, MAC_LEN);
        }
        if (conf->sta.band)
        {
            wifi_config.sta.band = conf->sta.band;
        }
        if (conf->sta.channel)
        {
            wifi_config.sta.channel = conf->sta.channel;
        }
        wifi_config.sta.use_dhcp = conf->sta.use_dhcp;

        //SDK静态IP配置
        if(wifi_config.sta.use_dhcp){
            user_static_ip.use_static_ip=0;
        }else{
            user_static_ip.use_static_ip=1;
        }

        wifi_config.sta.flags = conf->sta.flags;
        wifi_config.sta.eapType = conf->sta.eapType;
        if (NULL!=conf->sta.staIdentity)
        {
            memset(wifi_config.sta.staIdentity,0,AIIO_WIFI_IDENTITY_SIZE);
            memcpy((char*)wifi_config.sta.staIdentity,conf->sta.staIdentity,strlen(conf->sta.staIdentity));
        }
    }
    else
    {   // ap config
        memset(&wifi_config.ap, 0, sizeof(wifi_config.ap));
        if (conf->ap.ssid)
        {
            strncpy((char *)wifi_config.ap.ssid, (char *)conf->ap.ssid, SSID_LEN);
        }
        if (conf->ap.password)
        {
            strncpy((char *)wifi_config.ap.password, (char *)conf->ap.password, PASSWORD_LEN);
        }
        if (conf->ap.max_connection)
        {
            wifi_config.ap.max_connection = conf->ap.max_connection;
        }
        else
        {
            wifi_config.ap.max_connection = 3;
        }
        if (conf->ap.channel)
        {
            wifi_config.ap.channel = conf->ap.channel;
        }
        else
        {
            wifi_config.ap.channel = 1;
        }
        if (conf->ap.ssid_hidden)
        {
            wifi_config.ap.ssid_hidden = conf->ap.ssid_hidden;
        }
        else
        {
            wifi_config.ap.ssid_hidden = 0;
        }
        if (conf->ap.ssid_len)
        {
            wifi_config.ap.ssid_len = conf->ap.ssid_len;
        }
        if (conf->ap.dhcp_params.lease_time)
        {
            wifi_config.ap.dhcp_params.lease_time = conf->ap.dhcp_params.lease_time;
        }
        else
        {
            wifi_config.ap.dhcp_params.lease_time = 120;
        }
        if (conf->ap.dhcp_params.start)
        {
            wifi_config.ap.dhcp_params.start = conf->ap.dhcp_params.start;
        }
        else
        {
            wifi_config.ap.dhcp_params.start = 100;
        }
        if (conf->ap.dhcp_params.end)
        {
            wifi_config.ap.dhcp_params.end = conf->ap.dhcp_params.end;
        }
        else
        {
            wifi_config.ap.dhcp_params.end = 200;
        }
        wifi_config.ap.use_dhcp = conf->ap.use_dhcp;
    }
    return AIIO_OK;
}

aiio_err_t aiio_wifi_get_config(aiio_wifi_port_t port, aiio_wifi_config_t *conf)
{
    if (conf == NULL)
    {
        aiio_log_e("The conf is null error");
        return AIIO_ERROR;
    }
    if (port == AIIO_WIFI_IF_STA)
    {   // sta config
        strncpy((char *)conf->sta.ssid, (char *)wifi_config.sta.ssid, SSID_LEN);
        strncpy((char *)conf->sta.password, (char *)wifi_config.sta.password, PASSWORD_LEN);
        strncpy((char *)conf->sta.bssid, (char *)wifi_config.sta.bssid, MAC_LEN);
        conf->sta.band = wifi_config.sta.band;
        conf->sta.channel = wifi_config.sta.channel;
        conf->sta.use_dhcp = wifi_config.sta.use_dhcp;
        conf->sta.flags = wifi_config.sta.flags;
    }
    else
    {   // ap config
        strncpy((char *)conf->ap.ssid, (char *)wifi_config.ap.ssid, SSID_LEN);
        strncpy((char *)conf->ap.password, (char *)wifi_config.ap.password, PASSWORD_LEN);
        conf->ap.max_connection = wifi_config.ap.max_connection;
        conf->ap.channel = wifi_config.ap.channel;
        conf->ap.ssid_hidden = wifi_config.ap.ssid_hidden;
        conf->ap.ssid_len = wifi_config.ap.ssid_len;
        conf->ap.use_dhcp = wifi_config.ap.use_dhcp;
        conf->ap.dhcp_params.lease_time = wifi_config.ap.dhcp_params.lease_time;
        conf->ap.dhcp_params.start = wifi_config.ap.dhcp_params.start;
        conf->ap.dhcp_params.end = wifi_config.ap.dhcp_params.end;
    }
    return AIIO_OK;
}

aiio_err_t aiio_wifi_connect(void)
{
    //异步连接
    if(rtl_wifi_connect_thread_handle){
        aiio_log_e("connect task is running\r\n");
        return AIIO_ERROR;
    }

    if (AIIO_OK != aiio_os_thread_create(&rtl_wifi_connect_thread_handle, "wifi_connect", rtl_wifi_connect_thread, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT))
    {
        aiio_log_e("connect task create error\r\n");
        return AIIO_ERROR;
    }else{
        aiio_log_i("connect task create success\r\n");
        return AIIO_OK;
    }
}

aiio_err_t aiio_wifi_disconnect(void)
{
    aiio_err_t ret = AIIO_ERROR;
    int timeout = 20;
    u8 join_status = RTW_JOINSTATUS_UNKNOWN;
	struct rtw_wifi_setting wifi_setting = {RTW_MODE_NONE, {0}, {0}, 0, RTW_SECURITY_OPEN, {0}, 0, 0, 0, 0, 0};

	if (wifi_get_setting(STA_WLAN_INDEX, &wifi_setting) >= 0) {
		if (wifi_setting.mode == RTW_MODE_AP) {
            aiio_log_i("No need do disconnect for softap mode\r\n");
            ret = AIIO_OK;
			goto end;
		}
	} else {
        aiio_log_i("No need do disconnect when netif is down\r\n");
        ret = AIIO_OK;
		goto end;
	}

    wifi_get_join_status(&join_status);
    // if (wifi_is_connected_to_ap() != RTK_SUCCESS) {
    if (join_status != RTW_JOINSTATUS_SUCCESS) {
        aiio_log_i("Not connected yet\r\n");
        ret = AIIO_OK;
		goto end;
	}

	//Disconnecting ......
	if (wifi_disconnect() < 0) {
		aiio_log_i("Disconnect ERROR\r\n");
        ret = AIIO_ERROR;
		goto end;
	}
	while (timeout > 0) {
		// if (wifi_is_connected_to_ap() != RTK_SUCCESS) {
        wifi_get_join_status(&join_status);
        if (join_status != RTW_JOINSTATUS_SUCCESS) {
			aiio_log_i("disconnect done\r\n");
            ret = AIIO_OK;
            goto end;
		}
		rtos_time_delay_ms(1000);
		timeout--;
	}
    aiio_log_i("disconnect timeout\r\n");
    ret = AIIO_ERROR;
    goto end;

end:
	//user_static_ip.use_static_ip = 0;
	LwIP_ReleaseIP(STA_WLAN_INDEX);
	init_wifi_struct();

    return ret;
}

aiio_err_t aiio_wifi_ap_stop(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if (ai_wifi_init_ok == 0)
    {
        aiio_log_e("The wifi is not initialized!!!");
        goto __err;
    }

    switch (s_wifi_mode)
    {
        case AIIO_WIFI_MODE_AP:
            wifi_stop_ap();
            s_wifi_mode = AIIO_WIFI_MODE_NULL;
            break;
        case AIIO_WIFI_MODE_APSTA:
            if (wifi_on(RTW_MODE_STA) < 0)
            {
                s_wifi_mode = AIIO_WIFI_MODE_NULL;
                aiio_log_e("apsta change to sta failed!");
                goto __err;
            }
            s_wifi_mode = AIIO_WIFI_MODE_STA;
            break;
        default:
            goto __err;
    }
    ret = AIIO_OK;

__err:
    return ret;
}

aiio_err_t aiio_wifi_ap_sta_info_get(aiio_wifi_sta_basic_info_t *sta_info_p, uint8_t idx)
{
    aiio_err_t ret = AIIO_ERROR;

    struct rtw_client_list client_info = {0};
	wifi_ap_get_connected_clients(&client_info);
    for (size_t i = 0; i < client_info.count; i++)
    {
        if (idx != i)
        {
            continue;
        }
        sta_info_p->is_used = 1;
        sta_info_p->sta_idx = i;
        sta_info_p->sta_mac[0] = client_info.mac_list[i].octet[0];
        sta_info_p->sta_mac[1] = client_info.mac_list[i].octet[1];
        sta_info_p->sta_mac[2] = client_info.mac_list[i].octet[2];
        sta_info_p->sta_mac[3] = client_info.mac_list[i].octet[3];
        sta_info_p->sta_mac[4] = client_info.mac_list[i].octet[4];
        sta_info_p->sta_mac[5] = client_info.mac_list[i].octet[5];
        ret = AIIO_OK;
    }

    return ret;
}

aiio_err_t aiio_wifi_ap_sta_list_get(uint8_t *sta_num, aiio_wifi_sta_basic_info_t *sta_info_p)
{
    if (s_wifi_mode != AIIO_WIFI_MODE_AP && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        aiio_log_e("wifi AP is not enabled");
        return AIIO_ERROR;
    }

    struct rtw_client_list client_info = {0};
	wifi_ap_get_connected_clients(&client_info);

    for (size_t i = 0; i < client_info.count; i++)
    {
        (sta_info_p + i)->is_used = 1;
        (sta_info_p + i)->sta_idx = i;
        (sta_info_p + i)->sta_mac[0] = client_info.mac_list[i].octet[0];
        (sta_info_p + i)->sta_mac[1] = client_info.mac_list[i].octet[1];
        (sta_info_p + i)->sta_mac[2] = client_info.mac_list[i].octet[2];
        (sta_info_p + i)->sta_mac[3] = client_info.mac_list[i].octet[3];
        (sta_info_p + i)->sta_mac[4] = client_info.mac_list[i].octet[4];
        (sta_info_p + i)->sta_mac[5] = client_info.mac_list[i].octet[5];
    }

    *sta_num = (uint8_t)client_info.count;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_get_state(int32_t *state)
{
    if (s_wifi_mode != AIIO_WIFI_MODE_STA && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        return AIIO_ERROR;
    }

    *state = s_wifi_state;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_start(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if (ai_wifi_init_ok == 0)
    {
        aiio_log_e("The wifi is not initialized!!!");
        goto __err;
    }

    if (s_wifi_mode & AIIO_WIFI_MODE_AP)
    {
        if (aiio_wifi_ap_start() != AIIO_OK)
        {
            goto __err;
        }
    }

    if (s_wifi_mode & AIIO_WIFI_MODE_STA)
    {
        ret = aiio_wifi_connect();
        if (ret != AIIO_OK)
        {
            goto __err;
        }
    }

__err:
    return ret;
}

aiio_err_t aiio_wifi_stop(void)
{
    if (s_wifi_mode & AIIO_WIFI_MODE_AP)
    {
        aiio_wifi_ap_stop();
    }
    if (s_wifi_mode & AIIO_WIFI_MODE_STA)
    {
        aiio_wifi_disconnect();
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_scan(void *data, aiio_scan_complete_cb_t cb, bool filter)
{
    (void)(filter);
#if 1
    return aiio_wifi_scan_adv(data,cb,false,NULL);
#else
    aiio_err_t ret = AIIO_ERROR;
    enum rtw_join_status_type join_status = RTW_JOINSTATUS_UNKNOWN;
    struct _rtw_scan_param_t scan_param = {RTW_SCAN_NOUSE, 0, 0, 0, {0}, 0, 0, 0, 0};

    if (sp_ap_item != NULL)
    {
        aiio_log_e("sp_ap_item is not null!");
        goto __err;
    }
    sp_ap_item = aiio_os_malloc(MAX_SCAN_NUM * sizeof(aiio_wifi_ap_item_t));
    if (sp_ap_item == NULL)
    {
        aiio_log_e("sp_ap_item aiio_os_malloc failed!");
        goto __err;
    }

	join_status = wifi_get_join_status();
	if ((join_status > RTW_JOINSTATUS_UNKNOWN) && (join_status < RTW_JOINSTATUS_SUCCESS)) {
        //wifi连接中
		aiio_log_e("wifi connecting");
		ret = AIIO_ERROR;
        goto __err_free;
	}
    scan_param.scan_user_callback = app_scan_result_handler;
    s_scan_ap_num = 0;
    s_wifi_scan_done = false;
	if (wifi_scan_networks(&scan_param, 0) != RTK_SUCCESS) {    //wifi_scan_networks(aiio_wifi_scan_result_cb, NULL)
		aiio_log_e("wifi_scan_networks ERROR");
		ret = AIIO_ERROR;
        goto __err_free;
	}

    //等等扫描完成
    for (int i = 0; i < (1000*10); i++)
    { // 等待扫描结束
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
        if (s_wifi_scan_done)
        {
            goto __scan_done;
        }
    }
    aiio_log_e("wifi_scan_networks timeout");
	ret = AIIO_ERROR;
    goto __err_free;

__scan_done:
    if (cb != NULL)
    {
        cb((void *)data, NULL); //扫描完成回调
    }
    //扫描完成事件
    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_SCAN_DONE;
        wifi_cb(&aiio_event, NULL);
    }

    ret = AIIO_OK;

__err_free:
    if(sp_ap_item){
        aiio_os_free(sp_ap_item);
        sp_ap_item = NULL;
    }
__err:
    return ret;
#endif
}

aiio_err_t aiio_wifi_scan_adv(void *data, aiio_scan_complete_cb_t cb, bool filter, aiio_wifi_scan_params_t *scan_params)
{
    aiio_err_t ret = AIIO_ERROR;
    uint8_t join_status = RTW_JOINSTATUS_UNKNOWN;
    struct rtw_scan_param rtl_scan_param = {RTW_SCAN_NOUSE, 0, 0, 0, {0}, 0, 0, 0, 0};

    if (sp_ap_item != NULL)
    {
        aiio_log_e("sp_ap_item is not null!");
        goto __err;
    }
    sp_ap_item = aiio_os_malloc(MAX_SCAN_NUM * sizeof(aiio_wifi_ap_item_t));
    if (sp_ap_item == NULL)
    {
        aiio_log_e("sp_ap_item malloc failed!");
        goto __err;
    }
    join_status = wifi_get_join_status();
	if ((join_status > RTW_JOINSTATUS_UNKNOWN) && (join_status < RTW_JOINSTATUS_SUCCESS)) {
        //wifi连接中
		aiio_log_e("wifi connecting");
		ret = AIIO_ERROR;
        goto __err_free;
	
    }
    rtl_scan_param.max_ap_record_num=MAX_SCAN_NUM;
    rtl_scan_param.scan_user_callback = app_scan_result_handler;
    if(filter && NULL==scan_params){
        //过滤条件
        switch(scan_params->scan_mode){
            case AIIO_SCAN_PASSIVE:
                rtl_scan_param.options=RTW_SCAN_PASSIVE;
                break;
            case AIIO_SCAN_ACTIVE:
                rtl_scan_param.options=RTW_SCAN_ACTIVE;
                break;
            default:
                rtl_scan_param.options=RTW_SCAN_NOUSE;
                break;
        }
        if(strlen(scan_params->ssid)){
            rtl_scan_param.ssid=scan_params->ssid;
        }
        if(scan_params->duration_scan){
            rtl_scan_param.chan_scan_time.active_scan_time=scan_params->duration_scan/1000;
            rtl_scan_param.chan_scan_time.passive_scan_time=scan_params->duration_scan/1000;
        }
        if(scan_params->channel_num){
            rtl_scan_param.channel_list_num=scan_params->channel_num;
            rtl_scan_param.channel_list=scan_params->channels;
        }
    }

    s_scan_ap_num = 0;
    s_wifi_scan_done = false;
    if (wifi_scan_networks(&rtl_scan_param, 0) < RTK_SUCCESS) {
		aiio_log_e("wifi_scan_networks ERROR");
		ret = AIIO_ERROR;
        goto __err_free;
	}

    //等等扫描完成
    for (int i = 0; i < (1000*10); i++)
    { // 等待扫描结束
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
        if (s_wifi_scan_done)
        {
            goto __scan_done;
        }
    }
    aiio_log_e("wifi_scan_networks timeout");
	ret = AIIO_ERROR;
    goto __err_free;

__scan_done:
    if (cb != NULL)
    {
        cb((void *)data, NULL);
    }

    //扫描完成事件
    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_SCAN_DONE;
        wifi_cb(&aiio_event, NULL);
    }
    ret = AIIO_OK;

__err_free:
    if(sp_ap_item){
        aiio_os_free(sp_ap_item);
        sp_ap_item = NULL;
    }
__err:
    return ret;
}

// env、param1 are cb input params
aiio_err_t aiio_wifi_scan_ap_all(aiio_wifi_ap_item_t *env, uint32_t *param1, aiio_scan_item_cb_t cb)
{
    aiio_err_t ret = AIIO_ERROR;

    if (s_scan_ap_num == 0 || sp_ap_item == NULL)
    {
        aiio_log_e("scan item error!");
        goto __err;
    }

    for (size_t i = 0; i < s_scan_ap_num; i++)
    {
        if (cb != NULL)
        {
            cb(env, param1, &sp_ap_item[i]);
        }
    }

    ret = AIIO_OK;

__err:
    return ret;
}

aiio_err_t aiio_wifi_sta_mac_get(uint8_t *mac)
{
    aiio_err_t ret = AIIO_ERROR;
	u8 *tmp_mac = LwIP_GetMAC(0);
	u8 *ip = LwIP_GetIP(0);
	u8 *gw = LwIP_GetGW(0);
	u8 *msk = LwIP_GetMASK(0);
	struct rtw_wifi_setting *p_wifi_setting = NULL;

	p_wifi_setting = (struct rtw_wifi_setting *)aiio_os_malloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_log_e("alloc p_wifi_setting fail");
		ret = AIIO_ERROR;
        goto __exit;
	}
	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, p_wifi_setting);
            if( RTW_MODE_STA != p_wifi_setting->mode){
                continue;
            }
			tmp_mac = LwIP_GetMAC(i);
            memcpy(mac, tmp_mac,6);
            aiio_log_d("get MAC %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            ret = AIIO_OK;
            goto __exit;
		}
	}
    aiio_log_e("sta not running");
	ret = AIIO_ERROR;

__exit:
    if(p_wifi_setting){
        aiio_os_free((void *)p_wifi_setting);
        p_wifi_setting=NULL;
    }
    return ret;
}

aiio_err_t aiio_wifi_ap_mac_get(uint8_t *mac)
{
    aiio_err_t ret = AIIO_ERROR;
	u8 *tmp_mac = LwIP_GetMAC(0);
	u8 *ip = LwIP_GetIP(0);
	u8 *gw = LwIP_GetGW(0);
	u8 *msk = LwIP_GetMASK(0);
	struct rtw_wifi_setting *p_wifi_setting = NULL;

	p_wifi_setting = (struct rtw_wifi_setting *)aiio_os_malloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_log_e("alloc p_wifi_setting fail");
		ret = AIIO_ERROR;
        goto __exit;
	}
	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, p_wifi_setting);
            if(RTW_MODE_AP != p_wifi_setting->mode){
                continue;
            }
			tmp_mac = LwIP_GetMAC(i);
            memcpy(mac, tmp_mac,6);
            aiio_log_d("get MAC %02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            ret = AIIO_OK;
            goto __exit;
		}
	}
    aiio_log_e("ap not running");
	ret = AIIO_ERROR;

__exit:
    if(p_wifi_setting){
        aiio_os_free((void *)p_wifi_setting);
        p_wifi_setting=NULL;
    }
    return ret;
}

aiio_err_t aiio_wifi_sta_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
	u8 *tmp_mac = LwIP_GetMAC(0);
	u8 *tmp_ip = LwIP_GetIP(0);
	u8 *tmp_gw = LwIP_GetGW(0);
	u8 *tmp_msk = LwIP_GetMASK(0);
	struct rtw_wifi_setting *p_wifi_setting = NULL;

	p_wifi_setting = (struct rtw_wifi_setting *)aiio_os_malloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_log_e("alloc p_wifi_setting fail");
		ret = AIIO_ERROR;
        goto __exit;
	}
	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, p_wifi_setting);
            if( RTW_MODE_STA != p_wifi_setting->mode){
                continue;
            }
			tmp_mac = LwIP_GetMAC(i);
            tmp_ip = LwIP_GetIP(i);
	        tmp_gw = LwIP_GetGW(i);
	        tmp_msk = LwIP_GetMASK(i);
            memcpy(ip, tmp_ip, 4);
            memcpy(gw, tmp_gw, 4);
            memcpy(mask, tmp_msk, 4);
            ret = AIIO_OK;
            goto __exit;
		}
	}
    aiio_log_e("sta not running");
	ret = AIIO_ERROR;

__exit:
    if(p_wifi_setting){
        aiio_os_free((void *)p_wifi_setting);
        p_wifi_setting=NULL;
    }
    return ret;
}

aiio_err_t aiio_wifi_sta_netif_get(uint32_t *ip, uint32_t *gw, uint32_t *mask){
    return aiio_wifi_sta_ip_get(ip,gw,mask);
}

aiio_err_t aiio_wifi_sta_ip_set(char* sta_ip)
{
    aiio_err_t ret = AIIO_ERROR;

    if (sta_ip == NULL)
    {
        aiio_log_e("input param is null");
        goto __err;
    }

    aiio_log_d("sta_ip:%s", sta_ip);

    sta_ip_params.ip = ipaddr_addr((const char *)sta_ip);
    if (sta_ip_params.ip == 0xffffffff)
    {
        aiio_log_e("invalid ip");
        goto __err;
    }
    sta_ip_params.gateway = sta_ip_params.ip & 0x00ffffff | 0x01000000;
    sta_ip_params.netmask = 0x00ffffff;

    //SDK静态IP配置
    user_static_ip.addr = PP_HTONL(sta_ip_params.ip);
    user_static_ip.gw = PP_HTONL(sta_ip_params.gateway);
    user_static_ip.netmask = PP_HTONL(sta_ip_params.netmask);

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)sta_ip_params.ip, (uint8_t)(sta_ip_params.ip >> 8), (uint8_t)(sta_ip_params.ip >> 16), (uint8_t)(sta_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)sta_ip_params.gateway, (uint8_t)(sta_ip_params.gateway >> 8), (uint8_t)(sta_ip_params.gateway >> 16), (uint8_t)(sta_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)sta_ip_params.netmask, (uint8_t)(sta_ip_params.netmask >> 8), (uint8_t)(sta_ip_params.netmask >> 16), (uint8_t)(sta_ip_params.netmask >> 24));

    ret = AIIO_OK;

__err:
    return ret;
}

aiio_err_t aiio_wifi_sta_ip_parameter_set(uint32_t ip, uint32_t mask, uint32_t gw)
{
    sta_ip_params.ip = ip;
    sta_ip_params.gateway = gw;
    sta_ip_params.netmask = mask;

    //SDK静态IP配置
    user_static_ip.addr = PP_HTONL(sta_ip_params.ip);
    user_static_ip.gw = PP_HTONL(sta_ip_params.gateway);
    user_static_ip.netmask = PP_HTONL(sta_ip_params.netmask);

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)sta_ip_params.ip, (uint8_t)(sta_ip_params.ip >> 8), (uint8_t)(sta_ip_params.ip >> 16), (uint8_t)(sta_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)sta_ip_params.gateway, (uint8_t)(sta_ip_params.gateway >> 8), (uint8_t)(sta_ip_params.gateway >> 16), (uint8_t)(sta_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)sta_ip_params.netmask, (uint8_t)(sta_ip_params.netmask >> 8), (uint8_t)(sta_ip_params.netmask >> 16), (uint8_t)(sta_ip_params.netmask >> 24));

    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_ip_set(char* ap_ip)
{
    aiio_err_t ret = AIIO_ERROR;

    if (ap_ip == NULL)
    {
        goto __err;
    }

    aiio_log_d("input ip:%s", ap_ip);

    ap_ip_params.ip = ipaddr_addr((const char *)ap_ip);
    ap_ip_params.gateway = ap_ip_params.ip & 0x00ffffff | 0x01000000;
    ap_ip_params.netmask = ipaddr_addr((const char *)"255.255.255.0");

    aiio_log_d("ip:%d.%d.%d.%d", (uint8_t)ap_ip_params.ip, (uint8_t)(ap_ip_params.ip >> 8), (uint8_t)(ap_ip_params.ip >> 16), (uint8_t)(ap_ip_params.ip >> 24));
    aiio_log_d("gw:%d.%d.%d.%d", (uint8_t)ap_ip_params.gateway, (uint8_t)(ap_ip_params.gateway >> 8), (uint8_t)(ap_ip_params.gateway >> 16), (uint8_t)(ap_ip_params.gateway >> 24));
    aiio_log_d("netmask:%d.%d.%d.%d", (uint8_t)ap_ip_params.netmask, (uint8_t)(ap_ip_params.netmask >> 8), (uint8_t)(ap_ip_params.netmask >> 16), (uint8_t)(ap_ip_params.netmask >> 24));

    ret = AIIO_OK;

__err:
    return ret;
}

aiio_err_t aiio_wifi_ap_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
	u8 *tmp_mac = LwIP_GetMAC(0);
	u8 *tmp_ip = LwIP_GetIP(0);
	u8 *tmp_gw = LwIP_GetGW(0);
	u8 *tmp_msk = LwIP_GetMASK(0);
	struct rtw_wifi_setting *p_wifi_setting = NULL;

	p_wifi_setting = (struct rtw_wifi_setting *)aiio_os_malloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_log_e("alloc p_wifi_setting fail");
		ret = AIIO_ERROR;
        goto __exit;
	}
	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, p_wifi_setting);
            if( RTW_MODE_AP != p_wifi_setting->mode){
                continue;
            }
			tmp_mac = LwIP_GetMAC(i);
            tmp_ip = LwIP_GetIP(i);
	        tmp_gw = LwIP_GetGW(i);
	        tmp_msk = LwIP_GetMASK(i);
            memcpy(ip, tmp_ip, 4);
            memcpy(gw, tmp_gw, 4);
            memcpy(mask, tmp_msk, 4);
            ret = AIIO_OK;
            goto __exit;
		}
	}
    aiio_log_e("sta not running");
	ret = AIIO_ERROR;

__exit:
    if(p_wifi_setting){
        aiio_os_free((void *)p_wifi_setting);
        p_wifi_setting=NULL;
    }
    return ret;
}

aiio_err_t aiio_wifi_set_country_code(char *country_code){
    char country[3]={0,0,0};
    if(0==strcmp("JP",country_code)){
        country[0]='J';
        country[1]='P';
    }else if(0==strcmp("US",country_code)){
        country[0]='U';
        country[1]='S';
    }else if(0==strcmp("CN",country_code)){
        country[0]='C';
        country[1]='N';
    }else if(0==strcmp("MN",country_code)){
        country[0]='M';
        country[1]='N';
    }else if(0==strcmp("CL",country_code)){
        country[0]='C';
        country[1]='L';
    }else{
        aiio_log_e("unknow country_code:%s,set default 00\r\n",country_code,country);
        country[0]=0;
        country[1]=0;
    }
    wifi_set_countrycode(country);
    aiio_log_i("set country_code:%s value:%s\r\n",country_code,country);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_channel_get(int32_t *channel)
{
    u8 join_status = RTW_JOINSTATUS_UNKNOWN;
    struct rtw_wifi_setting wifi_setting = {0};

    wifi_get_join_status(&join_status);
    if (wifi_is_running(STA_WLAN_INDEX) && (join_status == RTW_JOINSTATUS_SUCCESS)) {
        wifi_get_setting(STA_WLAN_INDEX, &wifi_setting);
    } else {
        return AIIO_ERROR;
    }
    *channel = wifi_setting.channel;

    return AIIO_OK;
}

aiio_err_t aiio_wifi_channel_set(int32_t channel)
{
    if (wifi_set_channel(STA_WLAN_INDEX,channel) != RTK_SUCCESS)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_rssi_get(int32_t *rssi)
{
    union rtw_phy_stats phy_stats;
	wifi_get_phy_stats(STA_WLAN_INDEX, NULL, &phy_stats);
    *rssi = phy_stats.sta.rssi;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_connect_ind_stat_get(aiio_wifi_sta_connect_ind_stat_info_t *wifi_ind_stat)
{
    aiio_err_t ret = AIIO_ERROR;
	u8 *tmp_mac = LwIP_GetMAC(0);
	struct rtw_wifi_setting *p_wifi_setting = NULL;

	p_wifi_setting = (struct rtw_wifi_setting *)aiio_os_malloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_log_e("alloc p_wifi_setting fail");
		ret = AIIO_ERROR;
        goto __exit;
	}
	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, p_wifi_setting);
            if( RTW_MODE_STA != p_wifi_setting->mode){
                continue;
            }
            wifi_ind_stat->security = rtl_security2aiio(p_wifi_setting->security_type);// 加密类型
            strncpy(wifi_ind_stat->ssid, p_wifi_setting->ssid, strlen(p_wifi_setting->ssid)+1);
            strncpy(wifi_ind_stat->passphr, p_wifi_setting->password, strlen(p_wifi_setting->password)+1);
            memcpy(wifi_ind_stat->bssid, p_wifi_setting->bssid, 6);
            wifi_ind_stat->chan_id = p_wifi_setting->channel;
            ret = AIIO_OK;
            goto __exit;
		}
	}
    aiio_log_e("sta not running");
	ret = AIIO_ERROR;

__exit:
    if(p_wifi_setting){
        aiio_os_free((void *)p_wifi_setting);
        p_wifi_setting=NULL;
    }
    return ret;
}

aiio_wifi_beacon_auth_t aiio_wifi_get_sta_security(void){
    aiio_err_t ret = AIIO_ERROR;
	struct rtw_wifi_setting wifi_setting = {0};

	for (int i = 0; i < NET_IF_NUM; i++) {
		if (wifi_is_running(i)) {
            wifi_get_setting(i, &wifi_setting);
            if( RTW_MODE_STA != wifi_setting.mode){
                continue;
            }
            return rtl_security2aiio(wifi_setting.security_type);// 加密类型
		}
	}
    aiio_log_e("sta not running");
	return AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
}

//------------------------------------------------ SDK调用 ------------------------------------------------
void rtl_dhcp_get_ip_event(void)
{
    aiio_log_i("DHCP get IP");

    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_STA_GOT_IP;
        wifi_cb(&aiio_event, NULL);
    }
    s_wifi_state = AIIO_WIFI_STATE_CONNECTED_IP_GOT;
    return;
}
//------------------------------------------------ 内部调用 ------------------------------------------------
static void *aiio_socked_ssl_malloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = aiio_os_malloc(size);
	return ptr;
}

static uint8_t aiio_socked_ssl_free(void* ptr)
{
    rtos_mem_free(ptr);
    return 0;
}

static void init_wifi_struct(void)
{
	memset(wifi.ssid.val, 0, sizeof(wifi.ssid.val));
	memset(wifi.bssid.octet, 0, ETH_ALEN);
	// memset(password, 0, sizeof(password));
	wifi.ssid.len = 0;
	wifi.password = NULL;
	wifi.password_len = 0;
	wifi.key_id = -1;
	wifi.channel = 0;
	wifi.pscan_option = 0;
	memset(ap.ssid.val, 0, sizeof(ap.ssid.val));
	ap.ssid.len = 0;
	ap.password = NULL;
	ap.password_len = 0;
	ap.channel = 1;
	ap.hidden_ssid = 0;
	// security = -1;
}

static aiio_os_function_return_t rtl_wifi_connect_default(void){
    struct rtw_wifi_setting *p_wifi_setting = NULL;
    uint8_t assoc_by_bssid = 0;
    int ret = 0;
    unsigned long tick1 = rtos_time_get_current_system_time_ms();
	unsigned long tick2=0,tick3=0;
    // aiio_input_event_t aiio_event = {.code = AIIO_WIFI_EVENT_STA_DISCONNECTED};
    aiio_input_event_t aiio_event = {0};

    init_wifi_struct();
    if (s_wifi_mode != AIIO_WIFI_MODE_STA && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        aiio_log_e("mode error!!!");
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
    }
    //ssid
    strcpy((char *)wifi.ssid.val, wifi_config.sta.ssid);
    wifi.ssid.len = strlen(wifi_config.sta.ssid);
    //pwd
    if (wifi_config.sta.password != NULL && 0 != strlen(wifi_config.sta.password))
    {
        int pwd_len = strlen(wifi_config.sta.password);
        if (pwd_len > 64 || (pwd_len < 8 && pwd_len != 5))
        {
            aiio_log_e("pwd len error: %d!!!", pwd_len);
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_PW_ERROR;
            goto __err;
        }

        wifi.password = wifi_config.sta.password;
        wifi.password_len = pwd_len;
        wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }
    else
    {
        wifi.security_type = RTW_SECURITY_OPEN;
    }
    //bssid
    if (wifi_config.sta.bssid[0] != 0 || wifi_config.sta.bssid[1] != 0 || wifi_config.sta.bssid[2] != 0 ||
        wifi_config.sta.bssid[3] != 0 || wifi_config.sta.bssid[4] != 0 || wifi_config.sta.bssid[5] != 0)
    {
        memcpy(wifi.bssid.octet, wifi_config.sta.bssid, ETH_ALEN);
        assoc_by_bssid = 1;
    }
    
    p_wifi_setting = (struct rtw_wifi_setting *)rtos_mem_zmalloc(sizeof(struct rtw_wifi_setting));
	if (p_wifi_setting == NULL) {
		aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        aiio_log_e("alloc p_wifi_setting fail!!!");
        goto __err;
	}
	//Check password.
	if (wifi.password != NULL) {
		wifi.security_type = ((wifi.key_id >= 0) && (wifi.key_id <= 3)) ? RTW_SECURITY_WEP_SHARED : RTW_SECURITY_WPA2_AES_PSK;
	}else{
		wifi.security_type = RTW_SECURITY_OPEN;
	}
	g_enable_wifi_event_cb=0; //20240903同步SDK更新wifi回调事件,修改断开回调事件逻辑
	//Check AP mode
	wifi_get_setting(STA_WLAN_INDEX, p_wifi_setting);
	if (p_wifi_setting->mode == RTW_MODE_AP) {
		dhcps_deinit();
		if (wifi_on(RTW_MODE_STA) < 0) {
			aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            aiio_log_e("Wifi on failed!!!");
            goto __err;
		}
	}

	//Connecting ......
	if (assoc_by_bssid) {
        aiio_log_i("Joining BSS by BSSID "MAC_FMT" ...\r\n", MAC_ARG(wifi.bssid.octet));
	} else {
        aiio_log_i("Joining BSS by SSID %s...\r\n", (char *)wifi.ssid.val);
	}
    #if 0
    if(wifi.password){
		printf("[%s()-%d]password(%d):%s\r\n",__func__,__LINE__,wifi.password_len,wifi.password);
	}else{
		printf("[%s()-%d]password(%d)\r\n",__func__,__LINE__,wifi.password_len);
	}
	printf("[%s()-%d]ssid(%d):%s\r\n"
		"bssid:%02x:%02x:%02x:%02x:%02x:%02x\r\n"
		"key_id=%d\r\n"
		"channel=%d\r\n"
		"pscan_option=%d\r\n"
		"is_wps_trigger=%d\r\n"
		"\r\n",__func__,__LINE__,wifi.ssid.len,wifi.ssid.val,
		wifi.bssid.octet[0],wifi.bssid.octet[1],wifi.bssid.octet[2],wifi.bssid.octet[3],wifi.bssid.octet[4],wifi.bssid.octet[5],
		wifi.key_id,wifi.channel,wifi.pscan_option,wifi.is_wps_trigger);
	printf("wpa_supp:",wifi.password_len);
	for(int i=0;i<RSNXE_MAX_LEN;i++){
		printf("%02x ",wifi.wpa_supp.rsnxe_ie[i]);
	}
	printf("\r\nprev_bssid:%02x:%02x:%02x:%02x:%02x:%02x\r\n",wifi.prev_bssid.octet[0],wifi.prev_bssid.octet[1],wifi.prev_bssid.octet[2],wifi.prev_bssid.octet[3],wifi.prev_bssid.octet[4],wifi.prev_bssid.octet[5]);
	#endif
	ret = wifi_connect(&wifi, 1);
	if (ret != RTK_SUCCESS) {
		if (ret == -RTK_ERR_WIFI_CONN_INVALID_KEY) {
            aiio_log_e("Invalid Key set!!!");
		}
		aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        aiio_log_e("Can not connect to AP!!!");
        goto __err;
	}

	tick2 = rtos_time_get_current_system_time_ms();
    aiio_log_d("Connected after %d ms.",(unsigned int)(tick2 - tick1));
    //Start DHCPClient
	LwIP_DHCP(0, DHCP_START);
	tick3 = rtos_time_get_current_system_time_ms();
    aiio_log_d("Got IP after %d ms.",(unsigned int)(tick3 - tick1));

    // 注册wifi断开的回调函数
    g_enable_wifi_event_cb=1; //20240903同步SDK更新wifi回调事件,修改断开回调事件逻辑
    init_wifi_struct();
    return AIIO_OK;

__err:
    if(p_wifi_setting){
        rtos_mem_free((void *)p_wifi_setting);
        p_wifi_setting = NULL;
    }
    init_wifi_struct();
    if (wifi_cb)
    {
        wifi_cb(&aiio_event, NULL);
    }
    return AIIO_ERROR;
}

static aiio_os_function_return_t rtl_wifi_connect_eap(void){
    int mode;
    int ret = -1;

    aiio_input_event_t aiio_event = {.code = AIIO_WIFI_EVENT_STA_DISCONNECTED};

    if (s_wifi_mode != AIIO_WIFI_MODE_STA && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
    }
    //ssid
    if (NULL==wifi_config.sta.ssid || strlen(wifi_config.sta.ssid)>32)
    {
        aiio_log_e("ssid error");
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
    }

    eap_target_ssid = wifi_config.sta.ssid;
	eap_identity = wifi_config.sta.staIdentity;
	eap_password = wifi_config.sta.password;

	/*
		Set client cert is only used for EAP-TLS connection.
		If you are not using EAP-TLS method, no need to set eap_client_cert and eap_client_key value. (leave them to NULL value)
	*/
	/*
		eap_client_cert = \
	"-----BEGIN CERTIFICATE-----\r\n" \
	"MIIC9TCCAd0CAQIwDQYJKoZIhvcNAQEEBQAwgZMxCzAJBgNVBAYTAkZSMQ8wDQYD\r\n" \
	"VQQIEwZSYWRpdXMxEjAQBgNVBAcTCVNvbWV3aGVyZTEVMBMGA1UEChMMRXhhbXBs\r\n" \
	"ZSBJbmMuMSAwHgYJKoZIhvcNAQkBFhFhZG1pbkBleGFtcGxlLmNvbTEmMCQGA1UE\r\n" \
	"AxMdRXhhbXBsZSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMTYwMzE0MTEzNjMy\r\n" \
	"WhcNMTcwMzE0MTEzNjMyWjBxMQswCQYDVQQGEwJGUjEPMA0GA1UECBMGUmFkaXVz\r\n" \
	"MRUwEwYDVQQKEwxFeGFtcGxlIEluYy4xGTAXBgNVBAMUEHVzZXJAZXhhbXBsZS5j\r\n" \
	"b20xHzAdBgkqhkiG9w0BCQEWEHVzZXJAZXhhbXBsZS5jb20wgZ8wDQYJKoZIhvcN\r\n" \
	"AQEBBQADgY0AMIGJAoGBAODvCWRRjVQnUyQS/OqHS8MA94Dc5UOtLagKTOMJayB5\r\n" \
	"3MZyreWBkNg6sDfDG6OSD9tkVzwcp8CtZNflJc3i+d+nAnPM+kJedPJN5YVO+uwc\r\n" \
	"+K+QObH7fEOq8hnFIvOtYOfnMAxQKaVIKk0EOqqQv06BDvLyxoDCZNpAn4NQ8ZkR\r\n" \
	"AgMBAAEwDQYJKoZIhvcNAQEEBQADggEBAItqpmFftRu8ugTy4fRFwpjJNUuMRe83\r\n" \
	"Pm5Dv3V/byCHHdmIy0UI+6ZiMEtYrpvz4ZPgk0BDeytYooT7/kEUb8niQ64bDLYo\r\n" \
	"NcXctCmn5fjyX2M6Z3lQXCxX0XdFiukWlR21w4HO0nx7OJjrcjdpP9Tyk/kzCFl7\r\n" \
	"pblIavkfSmFtcxzcp0IoCupkUjFkA+MftZF82eQx4bE0jjiw2KgGwnzyYAdgtFXv\r\n" \
	"Ednj3ZyOuTlOQNGJgLQxyHooEJ/Tol/8p9EO5S6eQaHgZhbGP3AZ3SWV5oA0e6eT\r\n" \
	"D5JXti/LhyZhcbbJFawGXFI96ZOpHJ0EW12Osx/21oqmMp12AotS5Vw=\r\n" \
	"-----END CERTIFICATE-----\r\n";
		eap_client_key = \
	"-----BEGIN RSA PRIVATE KEY-----\r\n" \
	"Proc-Type: 4,ENCRYPTED\r\n" \
	"DEK-Info: DES-EDE3-CBC,79675299AD6E2237\r\n" \
	"\r\n" \
	"ZYY2hv1PYEsrhYbCip98XNpS6XxbntynEEp6aO9UgWeQ4I1pNOUptPUE+yNhbA7X\r\n" \
	"59ueT3yzx5L2ObImlJ3eIEvWq+iB8DdcPqFAo3c4dgfw/wPEhmxVPKvIyDQfaEuA\r\n" \
	"kWUno6b07n5uLTpQjIXQSdMTMYjYS+yPQy7ONC/vl/Ce+RMzrQAZkp5xcNNarUpl\r\n" \
	"2J1D2t+eRih/zRrgeVXztMiW2uyIT5a0IPoeBTPkPVb00kWYzn8eT9doN/ZCyr83\r\n" \
	"mv/uXF5ZOHnSNleOn1NiCZ8Uu3SHnmGhMBBMI75OghpEezQQCmtefYvtRxzGjMVB\r\n" \
	"UoRIlbATAleUjk3bmqRxfA2QZJj/GFWc9grxEerHWrdThSQ0w+fvwKBjTmEtUO2+\r\n" \
	"stKBJQi9RKFq4naM8UhtxojHIscXCx/wKrRZHS4QJYOQYelzfhTRUuTf3Czm/iTh\r\n" \
	"MQvX7dITNlLE3SW2MjzHb2ON9qUaKVnQPk53DO1zYgoxgDbQrw6FXDNMtYVv8SYf\r\n" \
	"JJZp66jGX6e1t4ziPHVqlDi5D2nWQ2DPNHO/rsoydA7icncKsC0iVzeUm7XgesxD\r\n" \
	"QEZoQIQDVS1aRE7qJCk9S2Hfe5Gfqnrp4110YuN/4khjMW2cOCKa/Yjgjyy2QQXT\r\n" \
	"nn6dBAeSWGzRM059VzhOyls5FIfnJIisZvF3JG518SzBU/YUGHEVN1XsfDS2M9/q\r\n" \
	"VkqhJ8/vbmIddKGeYULYW+xs3LvU1hnWiOodd9tuSeg5PxAbkJsV1nW06mVkgBqA\r\n" \
	"zqqEvwvY+6+9QW4PClKNKSocvM6yC+uhRi0sOZ+ckOv7f+uuMyw5FQ==\r\n" \
	"-----END RSA PRIVATE KEY-----\r\n";
		eap_client_key_pwd = "testca";
	*/
	eap_client_cert = \
					  "-----BEGIN CERTIFICATE-----\r\n" \
					  "MIIC9zCCAd8CAQMwDQYJKoZIhvcNAQEEBQAwgZMxCzAJBgNVBAYTAkZSMQ8wDQYD\r\n" \
					  "VQQIEwZSYWRpdXMxEjAQBgNVBAcTCVNvbWV3aGVyZTEVMBMGA1UEChMMRXhhbXBs\r\n" \
					  "ZSBJbmMuMSAwHgYJKoZIhvcNAQkBFhFhZG1pbkBleGFtcGxlLmNvbTEmMCQGA1UE\r\n" \
					  "AxMdRXhhbXBsZSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwHhcNMTYwMzE1MDgwNzEx\r\n" \
					  "WhcNMTcwMzE1MDgwNzExWjBzMQswCQYDVQQGEwJGUjEPMA0GA1UECBMGUmFkaXVz\r\n" \
					  "MRUwEwYDVQQKEwxFeGFtcGxlIEluYy4xGjAYBgNVBAMUEXVzZXIyQGV4YW1wbGUu\r\n" \
					  "Y29tMSAwHgYJKoZIhvcNAQkBFhF1c2VyMkBleGFtcGxlLmNvbTCBnzANBgkqhkiG\r\n" \
					  "9w0BAQEFAAOBjQAwgYkCgYEAqESlV4OYfBcIgZ+Cs8mWpiBjhvKoa0/kIe7saqhC\r\n" \
					  "e5q4snox0jdkUpLcc4vOs3vQ7ZGnimqTltA9oF6XNUzTWW4vlJTKEfrCWK085l7c\r\n" \
					  "DHFvHavH3E6vuP71lI7jq4PLXbo2TvZK+uBul4ozjzVWihaZBtz8eLHq446h/D/p\r\n" \
					  "kzkCAwEAATANBgkqhkiG9w0BAQQFAAOCAQEAAfhVAIkNdeeUNJud720uUHVnIcxz\r\n" \
					  "GXWI+Svi1qchuTEnRNhLwXmnE+A0WWSHyfdR6FvzdT3xtz3K50iOif8jY2gCGkSK\r\n" \
					  "8RjKr97228SwbrGO9y9+dYIjH1uz9cBpoVKcpzdsWpKObrDPDYyReHSWo99jM2+O\r\n" \
					  "vfJxnBw4PLiBj7Q0/dpd6o4JXyp7Cxa0mB4/+cZqjCzzuKfuK3WP7j6laMCV6mg4\r\n" \
					  "wRZ528IdwDqB7OOqsDm1PVQM8vzny9PM6ikWUCRTVNQJN8RDLkrHR3FRjy15YLdt\r\n" \
					  "yOfDqVnT/z0wGBaxnNziSJjqPGHPpRi4bJFGXwXOhtknKmciKzfj9/npoQ==\r\n" \
					  "-----END CERTIFICATE-----\r\n";
	eap_client_key = \
					 "-----BEGIN RSA PRIVATE KEY-----\r\n" \
					 "MIICXQIBAAKBgQCoRKVXg5h8FwiBn4KzyZamIGOG8qhrT+Qh7uxqqEJ7mriyejHS\r\n" \
					 "N2RSktxzi86ze9DtkaeKapOW0D2gXpc1TNNZbi+UlMoR+sJYrTzmXtwMcW8dq8fc\r\n" \
					 "Tq+4/vWUjuOrg8tdujZO9kr64G6XijOPNVaKFpkG3Px4serjjqH8P+mTOQIDAQAB\r\n" \
					 "AoGARI+LyweshssfxSkIKVc3EcNaqi6PHwJzUrw2ChM624AkR1xwllXJg7ehKVdK\r\n" \
					 "xmjprRLO8CASuL1qjsBb3fTKnBl+sIVxIFS0AI4Y3ri8VUKbangvSsI7pCzAFry7\r\n" \
					 "p1gmy9WWRV2ZEa+dV8xcrjb3bloT7hcdeLehgBCvExJIQM0CQQDXlSAKdW3AhYyj\r\n" \
					 "1A+pfyBSGxJbpSwNyyWgwHIHHjxendxmdUbrc8EbAu1eNKbP58TLgdCZsKcMonAv\r\n" \
					 "MY1Y2/nnAkEAx9CrUaCU8pJqXTRypM5JtexLKnYMJhpnA9uUILBQOq4Oe0eruyF5\r\n" \
					 "SaSxhyJYXY491ahWYPF0PTb3jkUhoN+l3wJBAJZthjgGDJlEFwjSFkOtYz4nib3N\r\n" \
					 "GVpeoFj1MBvrazCScpJDz0LIOLzCZCNSFfwIu3dNk+NKMqZMSn+D0h9pD40CQQC5\r\n" \
					 "K9n4NXaTLbjAU2CC9mE85JPr76XmkcUxwAWQHZTcLH1jJdIyAx1hb+zNLLjzSmRn\r\n" \
					 "Yi9ae6ibKhtUjyBQ87HFAkA2Bb3z7NUx+AA2g2HZocFZFShBxylACyQkl8FAFZtf\r\n" \
					 "osudmKdFQHyAWuBMex4tpz/OLTqJ1ecL1JQeC7OvlpEX\r\n" \
					 "-----END RSA PRIVATE KEY-----\r\n";
	eap_client_key_pwd = "";

	/*
		Verify server's certificate is an optional feature.
		If you want to use it please make sure ENABLE_EAP_SSL_VERIFY_SERVER in autoconf_eap.h is set to 1,
		and the eap_ca_cert is set correctly.
	*/
	eap_ca_cert = \
				  "-----BEGIN CERTIFICATE-----\r\n" \
				  "MIIEpzCCA4+gAwIBAgIJAPvZaozpdfjkMA0GCSqGSIb3DQEBCwUAMIGTMQswCQYD\r\n" \
				  "VQQGEwJGUjEPMA0GA1UECBMGUmFkaXVzMRIwEAYDVQQHEwlTb21ld2hlcmUxFTAT\r\n" \
				  "BgNVBAoTDEV4YW1wbGUgSW5jLjEgMB4GCSqGSIb3DQEJARYRYWRtaW5AZXhhbXBs\r\n" \
				  "ZS5jb20xJjAkBgNVBAMTHUV4YW1wbGUgQ2VydGlmaWNhdGUgQXV0aG9yaXR5MB4X\r\n" \
				  "DTE2MDMxNDExMjU0OVoXDTE2MDQxMzExMjU0OVowgZMxCzAJBgNVBAYTAkZSMQ8w\r\n" \
				  "DQYDVQQIEwZSYWRpdXMxEjAQBgNVBAcTCVNvbWV3aGVyZTEVMBMGA1UEChMMRXhh\r\n" \
				  "bXBsZSBJbmMuMSAwHgYJKoZIhvcNAQkBFhFhZG1pbkBleGFtcGxlLmNvbTEmMCQG\r\n" \
				  "A1UEAxMdRXhhbXBsZSBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkwggEiMA0GCSqGSIb3\r\n" \
				  "DQEBAQUAA4IBDwAwggEKAoIBAQC9pireu0aCDLNfMaGv3vId7RXjUhQwSK0jV2Oc\r\n" \
				  "SyvlKWH3P/N+5kLrP2iL6SCzyETVDXZ0vOsAMjcBF0zHp16prXV0d51cTUqeWBb0\r\n" \
				  "I5UnGxleIuuOfSg8zLUJoBWZPqLv++eZ5WgOKHt7SXocjvg7TU5t/TMB0Y8OCz3H\r\n" \
				  "CW2vJ/XKMgMA9HDUu4g57cJu88i1JPRpyFaz/HIQBc7+UNb9z+q09uTZKWTmEMqi\r\n" \
				  "E2U0EEIs7EtbxnOze1/8C4XNlmztrEdwvu6UEBU/TFkUoh9M646NkkBK7wP9n9pv\r\n" \
				  "T0nPQRJiiCrICzVqUtlEi9lIKpbBSMbQ0KzrGF7lGTgm4rz9AgMBAAGjgfswgfgw\r\n" \
				  "HQYDVR0OBBYEFIVyecka74kvOKIW0BjlTc/B+a2NMIHIBgNVHSMEgcAwgb2AFIVy\r\n" \
				  "ecka74kvOKIW0BjlTc/B+a2NoYGZpIGWMIGTMQswCQYDVQQGEwJGUjEPMA0GA1UE\r\n" \
				  "CBMGUmFkaXVzMRIwEAYDVQQHEwlTb21ld2hlcmUxFTATBgNVBAoTDEV4YW1wbGUg\r\n" \
				  "SW5jLjEgMB4GCSqGSIb3DQEJARYRYWRtaW5AZXhhbXBsZS5jb20xJjAkBgNVBAMT\r\n" \
				  "HUV4YW1wbGUgQ2VydGlmaWNhdGUgQXV0aG9yaXR5ggkA+9lqjOl1+OQwDAYDVR0T\r\n" \
				  "BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAQEAZYHM26sxbKOckVqJJ1QY0U2QFlGP\r\n" \
				  "1GYd8v27znxdnRmSonDvv3GjFfhwoyDk0JUuxkK/33ikCxihrgoO/EQTY9BV2OpW\r\n" \
				  "qkB1PDtb3i5ZRNvfjmW0pVA4p+GmdTGaEE5pTlcVnorzVrUeFKaZakb+IDFYzmeF\r\n" \
				  "xp8B3Bb5wvinDligLOaJnSlgS8QeeIab9HZfaVTTuPmVK6zE6D54Y0dJPnykvDdE\r\n" \
				  "cGN0FC+migfilFjJgkDJ0r78nwes55L8zjoofiZuO03rrHww6ARc3v1jYzAufddk\r\n" \
				  "QTiZHgjlMQb2XXMmXLn8kBgoDnqkXFNe8j0h8uxIJSrjOoIyn1h1wvX5/w==\r\n" \
				  "-----END CERTIFICATE-----\r\n";

	eap_client_cert_len = strlen(eap_client_cert) + 1;
	eap_client_key_len = strlen(eap_client_key) + 1;
	eap_ca_cert_len = strlen(eap_ca_cert) + 1;

    aiio_log_e("[%s()-%d]Type=%d ssid=%s identity=%s password=%s\r\n",__func__,__LINE__,wifi_config.sta.eapType,eap_target_ssid,eap_identity,eap_password);

    switch(wifi_config.sta.eapType){
        case AIIO_WIFI_EAP_TYPE_PEAP:   //PEAP认证
           ret = eap_start("peap");
            break;
        case AIIO_WIFI_EAP_TYPE_TLS:    //TLS认证
            // tls must present client_cert, client_key
            ret = eap_start("tls");
            break;
        case AIIO_WIFI_EAP_TYPE_TTLS:   //TTLS认证
            ret = eap_start("ttls");
            break;
        default:
            aiio_log_e("eapType type=%d error",wifi_config.sta.eapType);
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            goto __err;
    }
    if(ret == -1){ 
        aiio_log_e("[%s()-%d]eap method %d not supported\r\n",__func__,__LINE__,wifi_config.sta.eapType);
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
    }
    aiio_log_d("eapType type=%d start finish",wifi_config.sta.eapType);
    return AIIO_OK;

__err:
    if (wifi_cb)
    {
        wifi_cb(&aiio_event, NULL);
    }
    return AIIO_ERROR;
}

static aiio_os_function_return_t rtl_wifi_connect_thread(void *params)
{
    if( wifi_config.sta.eapType>AIIO_WIFI_EAP_TYPE_DISABLE && wifi_config.sta.eapType<AIIO_WIFI_EAP_TYPE_MAX){
        rtl_wifi_connect_eap();
    }else{
        rtl_wifi_connect_default();
    }
    rtl_wifi_connect_thread_handle = NULL;
    aiio_os_thread_delete(NULL);
}

static aiio_err_t aiio_wifi_ap_start(void)
{
    aiio_err_t ret = AIIO_ERROR;
    u32 ip_addr, netmask, gw;
	struct netif *pnetif = &xnetif[SOFTAP_WLAN_INDEX];
    struct rtw_wifi_setting *setting = NULL;
    int timeout = 20;
    struct ip_addr start_ip, end_ip;
    uint32_t start_ip_u32, end_ip_u32;

    start_ip_u32 = (ap_ip_params.ip & 0x00FFFFFF) | ((uint32_t)wifi_config.ap.dhcp_params.start << 24);
    end_ip_u32 = (ap_ip_params.ip & 0x00FFFFFF) | ((uint32_t)wifi_config.ap.dhcp_params.end << 24);

    //设置DHCP路由池
    ip_addr_set_ip4_u32(&start_ip, start_ip_u32);
    ip_addr_set_ip4_u32(&end_ip, end_ip_u32);
    dhcps_set_addr_pool(1, &start_ip, &end_ip);
    //SSID
    if (wifi_config.ap.ssid_len) {
		ap.ssid.len = wifi_config.ap.ssid_len;
		if (ap.ssid.len >= 32) {
            aiio_log_e("Invalid SSID length");
			ret = AIIO_ERROR;
			goto __err;
		}
		strncpy((char *)ap.ssid.val, wifi_config.ap.ssid, sizeof(ap.ssid.val) - 1);
	} else {
		aiio_log_e("NULL SSID");
		ret = AIIO_ERROR;
		goto __err;
	}
    //Channel
	ap.channel = wifi_config.ap.channel;
	//Security <open/wep/tkip/wpa2/wpa3> (maybe not exist)
    #if 0   //无密码时使用 RTW_SECURITY_OPEN 其余默认使用 RTW_SECURITY_WPA2_AES_PSK
	if (0 == strcmp("open", argv[security_idx])) {
		ap.security_type = RTW_SECURITY_OPEN;
	} else if (0 == strcmp("wep", argv[security_idx])) {
		ap.security_type = RTW_SECURITY_WEP_PSK;
	} else if (0 == strcmp("tpic", argv[security_idx])) {
		ap.security_type = RTW_SECURITY_WPA2_TKIP_PSK;
	} else if (0 == strcmp("wpa2", argv[security_idx])) {
		ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
	} else if (0 == strcmp("wpa3", argv[security_idx])) {
		ap.security_type = RTW_SECURITY_WPA3_AES_PSK;
	} else {
		RTK_LOGW(NOTAG, "[+WLSTARTAP] Invalid security value\r\n");
		error_no = 2;
		goto end;
	}
    #endif
	//Password (maybe not exist)
    ap.password = wifi_config.ap.password;
    ap.password_len = strlen(wifi_config.ap.password);
    if(ap.password_len == 0){
        ap.security_type = RTW_SECURITY_OPEN;
    }else if(ap.password_len >= RTW_MIN_PSK_LEN && ap.password_len <= RTW_WPA2_MAX_PSK_LEN){
        ap.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }else{
        aiio_log_e("AP password length invalid, must be 8-63 ASCII or 64 hex characters");
        ret = AIIO_ERROR;
        goto __err;
    }

    dhcps_deinit();
    ip_addr = bswap_32(ap_ip_params.ip);
	netmask = bswap_32(ap_ip_params.netmask);
	gw = bswap_32(ap_ip_params.gateway);
	LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
    ap.hidden_ssid = wifi_config.ap.ssid_hidden;
	//Starting ......
	wifi_stop_ap();
	if (wifi_start_ap(&ap) < 0) {
		aiio_log_e("wifi_start_ap failed");
		ret = AIIO_ERROR;
		goto __err;
	}
	setting = rtos_mem_zmalloc(sizeof(struct rtw_wifi_setting));
	if (setting == NULL) {
		aiio_log_e("memory failed for setting");
		ret = AIIO_ERROR;
		goto __err;
	}
	while (timeout > 0) {
		wifi_get_setting(SOFTAP_WLAN_INDEX, setting);
		if (strlen((char *)setting->ssid) > 0) {
			if (strcmp((char *) setting->ssid, (char *)ap.ssid.val) == 0) {
				aiio_log_i("%s started",ap.ssid.val);
                //启动AP事件
                if (wifi_cb)
                {
                    aiio_input_event_t aiio_event = {0};
                    aiio_event.code = AIIO_WIFI_EVENT_AP_START;
                    wifi_cb(&aiio_event, NULL);
                }
                goto AP_STARTED;
			}
		}
		rtos_time_delay_ms(1000);
		timeout--;
	}
    aiio_log_e("%s start timeout",ap.ssid.val);
	ret = AIIO_ERROR;
	goto __err;
AP_STARTED:
    ip_addr = bswap_32(ap_ip_params.ip);
	netmask = bswap_32(ap_ip_params.netmask);
	gw = bswap_32(ap_ip_params.gateway);
	LwIP_SetIP(SOFTAP_WLAN_INDEX, ip_addr, netmask, gw);
    if (wifi_config.ap.use_dhcp)
    {
        aiio_log_d("start dhcp");
        dhcps_init(pnetif); // 开启DHCP
    }else{
        aiio_log_d("use static IP");
    }
    ret = AIIO_OK;
__err:
    if(setting){
        rtos_mem_free((void *)setting);
    }
	init_wifi_struct();
    return ret;
}

static s32 app_scan_result_handler(unsigned int scanned_AP_num, void *user_data)
{
	struct rtw_scan_result *scanned_AP_info;
	char *scan_buf = NULL;
	unsigned int i = 0;
    int32_t ret=RTK_FAIL;
	UNUSED(user_data);

	if (scanned_AP_num == 0) {//scanned no AP
        aiio_log_e("scanned no AP");
		ret=RTK_FAIL;
        goto __exit;
	}
    if (scanned_AP_num >= MAX_SCAN_NUM)
    {
        scanned_AP_num=MAX_SCAN_NUM;
    }

	scan_buf = (char *)aiio_os_malloc(scanned_AP_num * sizeof(struct rtw_scan_result));
	if (scan_buf == NULL) {
        aiio_log_e("aiio_os_malloc error");
        ret=RTK_FAIL;
        goto __exit;
	}

	if (wifi_get_scan_records(&scanned_AP_num, scan_buf) < 0) {
        aiio_log_e("wifi_get_scan_records error");
		ret=RTK_FAIL;
        goto __exit;
	}

	for (i = 0; i < scanned_AP_num; i++) {
		scanned_AP_info = (struct rtw_scan_result *)(scan_buf + i * sizeof(struct rtw_scan_result));
		scanned_AP_info->ssid.val[scanned_AP_info->ssid.len] = 0; // Ensure the SSID is null terminated
        //填充一条扫描结果
        aiio_log_d("wps_type:%d",scanned_AP_info->wps_type);
        sp_ap_item[s_scan_ap_num].auth = rtl_security2aiio(scanned_AP_info->security);// 加密类型
        memcpy(sp_ap_item[s_scan_ap_num].bssid, scanned_AP_info->bssid.octet, 6);
        memset(sp_ap_item[s_scan_ap_num].ssid,0,33);
        memcpy(sp_ap_item[s_scan_ap_num].ssid, scanned_AP_info->ssid.val, scanned_AP_info->ssid.len);
        sp_ap_item[s_scan_ap_num].channel = scanned_AP_info->channel;
        sp_ap_item[s_scan_ap_num].rssi = scanned_AP_info->signal_strength;
        sp_ap_item[s_scan_ap_num].ssid_len = scanned_AP_info->ssid.len;
        
        s_scan_ap_num++;
	}
    ret=RTK_SUCCESS;

__exit:
    if(scan_buf){
        aiio_os_free((void *)scan_buf);
        scan_buf=NULL;
    }
    s_wifi_scan_done = true;
	return ret;
}

//参数
//    mac：需要对比的MAC，是个uint8_t mac[6]数据
//    idx：查询到的索引序号
//返回值
//    0：查询成功
//    -1：没有找到对应mac
static int aiio_wifi_softap_getindex_by_mac(uint8_t *mac,uint8_t *idx)
{
    uint8_t i = 0;
    struct rtw_client_list client_info = {0};
	wifi_ap_get_connected_clients(&client_info);
    for (size_t i = 0; i < client_info.count; i++)
    {
        if(0==memcmp(mac,client_info.mac_list[i].octet,6)){
            *idx = i;
            return 0;
        }
    }

    return -1;
}

static void aiio_wifi_softap_compare_associated(aiio_wifi_sta_basic_info_t *aiio_sta_info_compare,uint8_t *idx)
{
    uint8_t i = 0;
    uint8_t mac_null[6] = {0};
    for (i = 0; i < (sizeof(aiio_sta_info_temp) / sizeof(aiio_sta_info_temp[0])); i++)
    {
        if (memcmp(&aiio_sta_info_temp[i].sta_mac, &mac_null, 6) == 0)
        {
            aiio_sta_info_temp[i].is_used = 1;
            aiio_sta_info_temp[i].sta_idx = i;
            memcpy(aiio_sta_info_temp[i].sta_mac, aiio_sta_info_compare->sta_mac, 6);
            break;
        }
    }
    *idx = i;

    return AIIO_OK;
}

static void aiio_wifi_softap_compare_disassociated(aiio_wifi_sta_basic_info_t *aiio_sta_info_compare)
{
    for (uint8_t i = 0; i < (sizeof(aiio_sta_info_temp) / sizeof(aiio_sta_info_temp[0])); i++)
    {
        if (memcmp(aiio_sta_info_compare->sta_mac, &aiio_sta_info_temp[i].sta_mac, 6) == 0)
        {
            memset(&aiio_sta_info_temp[i], 0, sizeof(aiio_wifi_sta_basic_info_t));
            break;
        }
    }

    return AIIO_OK;
}

// AP模式有客户端连接上后的回调
static void aiio_wifi_client_connected_cb(char *buf, int buf_len, int flags, void *userdata)
{
    // buf[10]~buf[15]是新加入设备的MAC地址
    // ai_Callbacks.apClientConnectCallback(6, buf + 10);
    aiio_log_i("wifi sta connected!!!");

    // buf[10]~buf[15]是新加入设备的MAC地址
    uint8_t *mac = buf + 10;
    uint8_t idx = 0;
    memcpy(aiio_sta_info.sta_mac, mac, 6);
    aiio_wifi_softap_compare_associated(&aiio_sta_info, &idx);
    aiio_log_d("self store idx=%d\r\n",idx);
    if (wifi_cb)
    {
        aiio_log_d("aiio_wifi_softap_getindex_by_mac ret=%d\r\n",aiio_wifi_softap_getindex_by_mac(mac,&idx));   //中间件指令是从sdk获取的列表，不是使用自己创建的列表
        aiio_log_d("aiio_wifi_softap_getindex_by_mac idx=%d\r\n",idx);
        aiio_input_event_t aiio_event = {0};
        aiio_event.value = idx;
        aiio_event.code = AIIO_WIFI_EVENT_AP_STACONNECTED;
        wifi_cb(&aiio_event, (void *)mac);
    }
}

// AP模式有客户端断开的回调
static void aiio_wifi_client_disconnected_cb(char *buf, int buf_len, int flags, void *userdata)
{
    // buf[0]~buf[5]是当前断开设备的MAC地址
    aiio_log_i("wifi sta disconnected!!!");

    uint8_t *mac = (uint8_t *)buf;
    memcpy(aiio_sta_info.sta_mac, mac, 6);
    aiio_wifi_softap_compare_disassociated(&aiio_sta_info);
    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_AP_STADISCONNECTED;
        wifi_cb(&aiio_event, (void *)mac);
    }
}

//瑞昱SDK的wifi加密类型转换为中间件wifi加密类型
static aiio_wifi_beacon_auth_t rtl_security2aiio(enum rtw_security rtl_security_val){
    switch (rtl_security_val)
    { // 加密类型
        case RTW_SECURITY_OPEN:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_OPEN;
        case RTW_SECURITY_WEP_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_PSK;
        case RTW_SECURITY_WPA_TKIP_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_PSK;
        case (RTW_SECURITY_WPA_TKIP_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_ENTERPRISE;
        case RTW_SECURITY_WPA_WPA2_TKIP_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_PSK;
        case (RTW_SECURITY_WPA_WPA2_TKIP_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_ENTERPRISE;
        case RTW_SECURITY_WPA_WPA2_AES_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_PSK;
        case (RTW_SECURITY_WPA_WPA2_AES_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_ENTERPRISE;
        case RTW_SECURITY_WPA_WPA2_MIXED_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_MIXED_PSK;
        case (RTW_SECURITY_WPA_WPA2_MIXED_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_ENTERPRISE;
        case RTW_SECURITY_WPA_AES_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_PSK;
        case RTW_SECURITY_WPA_MIXED_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_PSK;
        case (RTW_SECURITY_WPA_MIXED_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_ENTERPRISE;
        case (RTW_SECURITY_WPA2_MIXED_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_ENTERPRISE;
        case RTW_SECURITY_WPA2_MIXED_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_MIXED_PSK;
        case RTW_SECURITY_WPA2_WPA3_MIXED:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_WPA3_MIXED;
        case RTW_SECURITY_WPA2_AES_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_PSK;
        case (RTW_SECURITY_WPA2_AES_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_ENTERPRISE;
        case RTW_SECURITY_WPA2_TKIP_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_PSK;
        case (RTW_SECURITY_WPA2_TKIP_PSK | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_ENTERPRISE;
        case RTW_SECURITY_WPA3_AES_PSK:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA3_AES_PSK;
        case (WPA3_SECURITY | ENTERPRISE_ENABLED):
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA3_ENTERPRISE;
        default:
            return AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
    }
}

//wifi连接事件回调
static void aiio_wifi_join_status_cb(char *buf, int buf_len, int flags, void *userdata)
{
	UNUSED(buf_len);
	UNUSED(userdata);

	u8 join_status = (u8)flags;
	struct rtw_event_info_joinstatus_joinfail *fail_info = (struct rtw_event_info_joinstatus_joinfail *)buf;
	struct rtw_event_info_joinstatus_disconn *disconn_info = (struct rtw_event_info_joinstatus_disconn *)buf;

	if (join_status == RTW_JOINSTATUS_SUCCESS) {
		/*Include 4 way handshake but not include DHCP */
		aiio_log_i("wifi reconnected!!!");
        if (wifi_cb)
        {
            aiio_input_event_t aiio_event = {0};
            aiio_event.code = AIIO_WIFI_EVENT_STA_CONNECTED;
            wifi_cb(&aiio_event, NULL);
            if(wifi_config.sta.use_dhcp){
                aiio_log_i("use dhcp,wait AIIO_WIFI_EVENT_STA_GOT_IP");
            }else{
                aiio_log_i("use static IP,call AIIO_WIFI_EVENT_STA_GOT_IP");
                aiio_event.code = AIIO_WIFI_EVENT_STA_GOT_IP;
                wifi_cb(&aiio_event, NULL);
            }
        }
        if(wifi_config.sta.use_dhcp){
            s_wifi_state = AIIO_WIFI_STATE_CONNECTED_IP_GETTING;
        }else{
            s_wifi_state = AIIO_WIFI_STATE_CONNECTED_IP_GOT;
        }
		return;
	}//end of RTW_JOINSTATUS_SUCCESS

	if (join_status == RTW_JOINSTATUS_FAIL) {/*Include 4 way handshake but not include DHCP, */
		/*Get fail reason*/
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_STA_DISCONNECTED;
		aiio_log_e("Join fail, reason = %d ", fail_info->fail_reason);
		switch (fail_info->fail_reason) {
            case -RTK_ERR_WIFI_CONN_SCAN_FAIL:
                aiio_log_e("(Join fail:(Can not found target AP)\r\n");
                aiio_event.value = AIIO_WLAN_FW_SCAN_NO_BSSID_AND_CHANNEL;
                break;
            case -RTK_ERR_WIFI_CONN_AUTH_PASSWORD_WRONG:
            case -RTK_ERR_WIFI_CONN_4WAY_PASSWORD_WRONG:
                aiio_log_e("Join fail:(Password wrong)\r\n");
                aiio_event.value = AIIO_WLAN_FW_4WAY_HANDSHAKE_TX_DEAUTH_FRAME_TRANSMIT_FAILURE;
                break;
            case -RTK_ERR_WIFI_CONN_AUTH_FAIL:
                aiio_log_e("Join fail:(Auth fail)\r\n");
                aiio_event.value = AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE;
                break;
            case -RTK_ERR_WIFI_CONN_ASSOC_FAIL:
                aiio_log_e("Join fail:(Assoc fail)\r\n");
                aiio_event.value = AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE;
                break;
            case -RTK_ERR_WIFI_CONN_4WAY_HANDSHAKE_FAIL:
                aiio_log_e("Join fail:(4 way handshake fail)\r\n");
                aiio_event.value = AIIO_WLAN_FW_AUTH_OR_ASSOC_RESPONSE_TIMEOUT_FAILURE;
                break;
            case RTK_ERR_WIFI_CONN_INVALID_KEY:
                aiio_log_e("Join fail:(Invalid key)\r\n");
                aiio_event.value = AIIO_WLAN_FW_DEAUTH_BY_AP_WHEN_NOT_CONNECTION;
                break;
            default:
                aiio_log_e("Join fail:(unknow)\r\n");
                aiio_event.value = 0xFF;
                break;
		}
		/*Get more detail fail info*/
		if (fail_info->fail_reason == -RTK_ERR_WIFI_CONN_AUTH_FAIL || fail_info->fail_reason == -RTK_ERR_WIFI_CONN_ASSOC_FAIL ||
			fail_info->fail_reason == -RTK_ERR_WIFI_CONN_AUTH_PASSWORD_WRONG) {
			/*status code is valid during auth and assoc*/
			aiio_log_e("Join fail status code from AP = %d\n", fail_info->reason_or_status_code);
		} else if (fail_info->fail_reason == -RTK_ERR_WIFI_CONN_4WAY_HANDSHAKE_FAIL || fail_info->fail_reason == -RTK_ERR_WIFI_CONN_4WAY_PASSWORD_WRONG) {
			/*reason code is valid after assoc success*/
			aiio_log_e("Join fail reason code from AP = %d\n", fail_info->reason_or_status_code);
		}
        
        aiio_log_e("Can not connect to AP!!!");
		if (wifi_cb)
        {
            wifi_cb(&aiio_event, NULL);
        }
		return;
	}//end of RTW_JOINSTATUS_FAIL

	if (join_status == RTW_JOINSTATUS_DISCONNECT) {
		aiio_log_i("wifi disconnected!!!");
		/*Get disconnect reason, definition enum rtw_disconn_reason*/
		aiio_log_d("Disconnect, reason = %d\n", disconn_info->disconn_reason);
		/*Get more detail disconnect info*/
		if (disconn_info->disconn_reason < RTW_DISCONN_RSN_DRV_BASE) {
			aiio_log_d("Disconnect by AP, reason code =%d\n", disconn_info->disconn_reason);
		} else {
			switch (disconn_info->disconn_reason) {
			case RTW_DISCONN_RSN_DRV_AP_LOSS:
				aiio_log_d("Disconnect by Driver, detect AP loss\n");
				break;
			case RTW_DISCONN_RSN_DRV_AP_CHANGE:
				aiio_log_d("Disconnect by Driver, detect AP change\n");
				break;
			case RTW_DISCONN_RSN_APP_DISCONN:
				aiio_log_d("Disconnect by APP call disconnect API\n");
				break;
			case RTW_DISCONN_RSN_APP_CONN_WITHOUT_DISCONN:
				aiio_log_d("Disconnect by APP call connect without calling disconnect first\n");
				break;
			}
		}
		if(g_enable_wifi_event_cb){
			if (wifi_cb){
				aiio_input_event_t aiio_event = {0};
				aiio_event.code = AIIO_WIFI_EVENT_STA_DISCONNECTED;
				wifi_cb(&aiio_event, NULL);
			}
		}else{
			aiio_log_i("skip wifi disconnected cb");
		}
		s_wifi_state = AIIO_WIFI_STATE_DISCONNECT;
		return;
	}//end of RTW_JOINSTATUS_DISCONNECT
}//end of aiio_wifi_join_status_cb

static u8 aiio_wifi_sniffer_cb(struct rtw_rx_pkt_info *pkt_info)
{
	u8 *buf = pkt_info->buf;
	u32 buf_len = pkt_info->len;
    
    if (aiio_sniffer_cb != NULL)
    {
        aiio_sniffer_cb(NULL, buf, buf_len, NULL);
    }
    return 0;
}
aiio_err_t aiio_wifi_sniffer_enable(void)
{
	struct rtw_promisc_para promiscpara;
	memset(&promiscpara, 0, sizeof(struct rtw_promisc_para));
	promiscpara.filter_mode = RTW_PROMISC_FILTER_ALL_PKT;
	promiscpara.callback = aiio_wifi_sniffer_cb;
	wifi_promisc_enable(ENABLE, &promiscpara);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_disable(void)
{
    aiio_err_t ret = AIIO_ERROR;

    wifi_promisc_enable(DISABLE, NULL);

    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_register(void *env, aiio_sniffer_cb_t cb)
{
    aiio_sniffer_cb = cb;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_unregister(void *env)
{
    aiio_sniffer_cb = NULL;
    return AIIO_OK;
}

extern struct netif xnetif[NET_IF_NUM];

aiio_err_t aiio_wifi_set_hostname(char *hostname)
{
    if (hostname == NULL || strlen(hostname) == 0) {
        return AIIO_ERROR;
    }

#if LWIP_NETIF_HOSTNAME
    netif_set_hostname(&xnetif[0], hostname);
#else
    return AIIO_ERROR;
#endif

    /* 关键：让 DHCP 重新带上 hostname */
#if LWIP_DHCP
    dhcp_stop(&xnetif[0]);
    dhcp_start(&xnetif[0]);
#endif

    return AIIO_OK;
}
