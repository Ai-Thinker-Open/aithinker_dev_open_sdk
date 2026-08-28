/** @brief      Wifi application interface.
 *
 *  @file       aiio_wifi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Wifi application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/03/7       <td>1.0.0       <td>hongjz      <td>first commit
 *  </table>
 * 
 */

#include "aiio_wifi.h"
#include "aiio_error.h"
#include "aiio_log.h"
#include "FreeRTOS.h"
#include "task.h"
#include "crypto_api.h"
#include "main.h"
#include <dhcp/dhcps.h>
#include <wifi/wifi_conf.h>
#include <lwip_netconf.h>
#include <lwip/sockets.h>
#include "wps/wps_defs.h"
#include <osdep_service.h>
#include "aiio_at_wifi_api.h"

#define MAX_SCAN_NUM    (60)

typedef int (*wlan_init_done_ptr)(void);
typedef int (*write_reconnect_ptr)(uint8_t *data, uint32_t len);

extern write_reconnect_ptr p_write_reconnect_ptr;    // SDK中wifi重连接回调函数
extern wlan_init_done_ptr p_wlan_init_done_callback; // SDK中wifi 初始化完成回调函数
extern struct netif xnetif[NET_IF_NUM];
extern rtw_mode_t wifi_mode;

//EAP相关 start
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
extern const configSTACK_DEPTH_TYPE *eap_eapol_recvd_stack;
extern u8 eap_method;

void eap_eapol_recvd_hdl(char *buf, int buf_len, int flags, void* handler_user_data);
void eap_eapol_start_hdl(char *buf, int buf_len, int flags, void* handler_user_data);
void eap_disconnected_hdl(char *buf, int buf_len, int flags, void* handler_user_data);
//EAP相关 end

static aiio_os_function_return_t rtl_wifi_connect_thread(void *params);

static uint8_t s_wifi_init_ok = 0;
static bool s_wifi_scan_done = false;	//用来记录扫描是否完成
static uint8_t s_scan_ap_num = 0;
static int32_t s_wifi_state = AIIO_WIFI_STATE_UNKNOWN;
static aiio_wifi_mode_t s_wifi_mode = AIIO_WIFI_MODE_NULL;   //启动默认sta模式
static aiio_sniffer_cb_t aiio_sniffer_cb = NULL;
static aiio_event_cb wifi_cb = NULL;
static aiio_wifi_config_t wifi_config = {0};
static aiio_wifi_sta_basic_info_t aiio_sta_info = {0};
static aiio_wifi_sta_basic_info_t aiio_sta_info_temp[3] = {0};
static aiio_wifi_ap_item_t *sp_ap_item = NULL;
static aiio_wifi_ip_params_t ap_ip_params =
{
    .ip = 0x012BA8C0,       //192.168.43.1
    .gateway = 0x012BA8C0,  //192.168.43.1
    .netmask = 0x00ffffff,  //255.255.255.0
};
static aiio_wifi_ip_params_t sta_ip_params = { 0 };

static void *aiio_socked_ssl_malloc(size_t nelements, size_t elementSize)
{
	size_t size;
	void *ptr = NULL;

	size = nelements * elementSize;
	ptr = pvPortMalloc(size);
	if(ptr != NULL)
    {
		memset(ptr, 0, size);
	}

	return ptr;
}

static uint8_t aiio_socked_ssl_free(void* ptr)
{
    vPortFree(ptr);
    return 0;
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

//wifi_scan_networks_with_ssid 的处理函数
//参数
//    buf：这个应该是扫描结果
//    buflen：wifi_scan_networks_with_ssid传入的第三个参数，应该是表示扫描接收buf的大小
//    target_ssid：wifi_scan_networks_with_ssid入的第四个参数，表示我们要处理的SSID名称
//    user_data：wifi_scan_networks_with_ssid入的第二个参数，用于接收扫描结果
static int aiio_scan_buf_process(char *buf, int buflen, char *target_ssid, void *user_data)
{
    rtw_wifi_setting_t *pwifi = (rtw_wifi_setting_t *)user_data;
    int plen = 0;

    while (plen < buflen)
    {
        u8 len, ssid_len, security_mode;
        char *ssid;

        // len offset = 0
        len = (int)*(buf + plen);
        // check end
        if (len == 0)
            break;
        // ssid offset = 14
        ssid_len = len - 14;
        ssid = buf + plen + 14;
        if ((ssid_len == strlen(target_ssid)) && (!memcmp(ssid, target_ssid, ssid_len)))
        {
            strncpy((char *)pwifi->ssid, target_ssid, 33);
            // channel offset = 13
            pwifi->channel = *(buf + plen + 13);
            // security_mode offset = 11
            security_mode = (u8) * (buf + plen + 11);
            if (security_mode == RTW_ENCODE_ALG_NONE)
            {
                pwifi->security_type = RTW_SECURITY_OPEN;
            }
            else if (security_mode == RTW_ENCODE_ALG_WEP)
            {
                pwifi->security_type = RTW_SECURITY_WEP_PSK;
            }
            else if (security_mode == RTW_ENCODE_ALG_CCMP)
            {
                pwifi->security_type = RTW_SECURITY_WPA2_AES_PSK;
            }
        }
        plen += len;
    }
    return 0;
}

//扫描wifi列表，查询指定 ssid 的加密方式(security_mode)和信道(channel)
//返回值
//    0：查询失败
//    1：查询成功
static int aiio_get_wifi_info_by_ssid_from_scan_list(IN char *ssid, OUT rtw_security_t *security_mode, OUT u8 *channel)
{
    rtw_wifi_setting_t tmpWifi;
    u32 scan_buflen = 1000;

    memset(&tmpWifi, 0, sizeof(tmpWifi));

    if (wifi_scan_networks_with_ssid(aiio_scan_buf_process, (void *)&tmpWifi, scan_buflen, ssid, strlen(ssid)) != RTW_SUCCESS)
    {
        return 0;
    }

    if (strcmp(tmpWifi.ssid, ssid) == 0)
    {
        *security_mode = tmpWifi.security_type;
        *channel = tmpWifi.channel;
        return 1;
    }

    return 0;
}

// wifi初始化完成后回调的函数
static int aiio_wifi_inited_cb(void)
{
    s_wifi_init_ok = 1;

    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_WIFI_READY;
        wifi_cb(&aiio_event, NULL);
    }

    return 0;
}

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

//wifi重连成功后的回调(这个是SDK中的重连回调combo框架有自己的重连方法)
static int aiio_wifi_reconnected_cb(u8 *data, uint32_t len)
{
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

    return 0;
}

//wifi断开回调函数
static void aiio_wifi_disconnected_cb(char *buf, int buf_len, int flags, void *userdata)
{
    // AI_DataPrint(AI_PRINT_LEVE_INFO, AI_DATA_CH_LOG, "\r\n[%s()-%d]WIFI disconnect(%d)%s\r\nflags:%d\r\n", __func__, __LINE__, buf_len, buf, flags);
    // ai_Callbacks.wifiDisconnectCallback(NULL);
    aiio_log_i("wifi disconnected!!!");

    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_STA_DISCONNECTED;
        wifi_cb(&aiio_event, NULL);
    }

    s_wifi_state = AIIO_WIFI_STATE_DISCONNECT;
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
    if (wifi_cb)
    {
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
#if 1
    uint8_t *mac = (uint8_t *)buf;

    memcpy(aiio_sta_info.sta_mac, mac, 6);

    aiio_wifi_softap_compare_disassociated(&aiio_sta_info);
    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_AP_STADISCONNECTED;
        wifi_cb(&aiio_event, (void *)mac);
    }
#endif
}

// 系统回调，当扫描到一条数据就会执行一次回调
static rtw_result_t aiio_wifi_scan_result_cb(rtw_scan_handler_result_t *malloced_scan_result)
{
    if (s_scan_ap_num >= MAX_SCAN_NUM)
    {
        return RTW_SUCCESS;
    }

    //此处赋值
    if (malloced_scan_result->scan_complete != RTW_TRUE)
    {
        if (sp_ap_item != NULL)
        {
            rtw_scan_result_t* record = &malloced_scan_result->ap_details;
            record->SSID.val[record->SSID.len] = 0; /* Ensure the SSID is null terminated */

            switch (record->security)
            { // 加密类型
            case RTW_SECURITY_WPS_OPEN:
            case RTW_SECURITY_OPEN:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_OPEN;
                break;
            case RTW_SECURITY_WEP_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_PSK;
                break;
            case RTW_SECURITY_WEP_SHARED:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_SHARED;
                break;
            case RTW_SECURITY_WPS_SECURE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPS;
                break;
            case RTW_SECURITY_WPA_TKIP_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_TKIP_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_AES_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_PSK;
                break;
			case RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE:
				sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_ENTERPRISE;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_PSK:
				sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_MIXED_PSK;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE:
				sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_ENTERPRISE;
				break;
            case RTW_SECURITY_WPA_AES_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_PSK;
                break;
            case RTW_SECURITY_WPA_AES_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_MIXED_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA_MIXED_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA2_WPA3_MIXED:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_WPA3_MIXED;
                break;
            case RTW_SECURITY_WPA2_AES_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_PSK;
                break;
            case RTW_SECURITY_WPA2_AES_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_AES_CMAC:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_CMAC;
                break;
            case RTW_SECURITY_WPA2_TKIP_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA2_TKIP_ENTERPRISE:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA3_AES_PSK:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA3_AES_PSK;
                break;
            default:
                sp_ap_item[s_scan_ap_num].auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
                break;
            }

            memcpy(sp_ap_item[s_scan_ap_num].bssid, record->BSSID.octet, 6);
            memset(sp_ap_item[s_scan_ap_num].ssid,0,33);
            memcpy(sp_ap_item[s_scan_ap_num].ssid, record->SSID.val, record->SSID.len);
            sp_ap_item[s_scan_ap_num].channel = record->channel;
            sp_ap_item[s_scan_ap_num].rssi = record->signal_strength;
            sp_ap_item[s_scan_ap_num].ssid_len = record->SSID.len;
        
            s_scan_ap_num++;
        }
    }
    else
    {
        s_wifi_scan_done = true;
    }

    return RTW_SUCCESS;
}

static void aiio_wifi_sniffer_cb(unsigned char* buf, unsigned int len, void* user_data)
{
    if (aiio_sniffer_cb != NULL)
    {
        aiio_sniffer_cb(NULL, buf, len, NULL);
    }
}

static void aiio_lwip_use_static_ip(struct netif *pnetif)
{
    #if 1
    if(wifi_mode == RTW_MODE_STA){
        //设置IP
        sta_ip[0] = (unsigned char) sta_ip_params.ip & 0xff;
		sta_ip[1] = (unsigned char) (sta_ip_params.ip >> 8) & 0xff;
		sta_ip[2] = (unsigned char) (sta_ip_params.ip >> 16) & 0xff;
		sta_ip[3] = (unsigned char) (sta_ip_params.ip >> 24) & 0xff;
		//配置网关
		sta_gw[0] = (unsigned char) sta_ip_params.gateway & 0xff;
		sta_gw[1] = (unsigned char) (sta_ip_params.gateway >> 8) & 0xff;
		sta_gw[2] = (unsigned char) (sta_ip_params.gateway >> 16) & 0xff;
		sta_gw[3] = (unsigned char) (sta_ip_params.gateway >> 24) & 0xff;
		//配置掩码
		sta_netmask[0] = (unsigned char) sta_ip_params.netmask & 0xff;
		sta_netmask[1] = (unsigned char) (sta_ip_params.netmask >> 8) & 0xff;
		sta_netmask[2] = (unsigned char) (sta_ip_params.netmask >> 16) & 0xff;
		sta_netmask[3] = (unsigned char) (sta_ip_params.netmask >> 24) & 0xff;
		//加载静态IP信息
		LwIP_UseStaticIP(pnetif);
    }else if (wifi_mode == RTW_MODE_AP){
        //设置IP
		ap_ip[0] = (unsigned char) ap_ip_params.ip & 0xff;
		ap_ip[1] = (unsigned char) (ap_ip_params.ip >> 8) & 0xff;
		ap_ip[2] = (unsigned char) (ap_ip_params.ip >> 16) & 0xff;
		ap_ip[3] = (unsigned char) (ap_ip_params.ip >> 24) & 0xff;
		//设置网关
		ap_gw[0] = (unsigned char) ap_ip_params.gateway & 0xff;
		ap_gw[1] = (unsigned char) (ap_ip_params.gateway >> 8) & 0xff;
		ap_gw[2] = (unsigned char) (ap_ip_params.gateway >> 16) & 0xff;
		ap_gw[3] = (unsigned char) (ap_ip_params.gateway >> 24) & 0xff;
		//设置掩码
		ap_netmask[0] = (unsigned char) ap_ip_params.netmask & 0xff;
		ap_netmask[1] = (unsigned char) (ap_ip_params.netmask >> 8) & 0xff;
		ap_netmask[2] = (unsigned char) (ap_ip_params.netmask >> 16) & 0xff;
		ap_netmask[3] = (unsigned char) (ap_ip_params.netmask >> 24) & 0xff;
        //加载静态IP信息
		LwIP_UseStaticIP(pnetif);
    }
    #else
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

    /* Static address used */
    if (pnetif->name[1] == '0')
    {
        if (wifi_mode == RTW_MODE_STA)
        {
            IP4_ADDR(ip_2_ip4(&ipaddr), (uint8_t)sta_ip_params.ip, (uint8_t)(sta_ip_params.ip >> 8), (uint8_t)(sta_ip_params.ip >> 16), (uint8_t)(sta_ip_params.ip >> 24));
            IP4_ADDR(ip_2_ip4(&netmask), (uint8_t)sta_ip_params.netmask, (uint8_t)(sta_ip_params.netmask >> 8), (uint8_t)(sta_ip_params.netmask >> 16), (uint8_t)(sta_ip_params.netmask >> 24));
            IP4_ADDR(ip_2_ip4(&gw), (uint8_t)sta_ip_params.gateway, (uint8_t)(sta_ip_params.gateway >> 8), (uint8_t)(sta_ip_params.gateway >> 16), (uint8_t)(sta_ip_params.gateway >> 24));
        }
        else if (wifi_mode == RTW_MODE_AP)
        {
            IP4_ADDR(ip_2_ip4(&ipaddr), (uint8_t)ap_ip_params.ip, (uint8_t)(ap_ip_params.ip >> 8), (uint8_t)(ap_ip_params.ip >> 16), (uint8_t)(ap_ip_params.ip >> 24));
            IP4_ADDR(ip_2_ip4(&netmask), (uint8_t)ap_ip_params.netmask, (uint8_t)(ap_ip_params.netmask >> 8), (uint8_t)(ap_ip_params.netmask >> 16), (uint8_t)(ap_ip_params.netmask >> 24));
            IP4_ADDR(ip_2_ip4(&gw), (uint8_t)ap_ip_params.gateway, (uint8_t)(ap_ip_params.gateway >> 8), (uint8_t)(ap_ip_params.gateway >> 16), (uint8_t)(ap_ip_params.gateway >> 24));
        }
    }
    else
    {
        IP4_ADDR(ip_2_ip4(&ipaddr), (uint8_t)ap_ip_params.ip, (uint8_t)(ap_ip_params.ip >> 8), (uint8_t)(ap_ip_params.ip >> 16), (uint8_t)(ap_ip_params.ip >> 24));
        IP4_ADDR(ip_2_ip4(&netmask), (uint8_t)ap_ip_params.netmask, (uint8_t)(ap_ip_params.netmask >> 8), (uint8_t)(ap_ip_params.netmask >> 16), (uint8_t)(ap_ip_params.netmask >> 24));
        IP4_ADDR(ip_2_ip4(&gw), (uint8_t)ap_ip_params.gateway, (uint8_t)(ap_ip_params.gateway >> 8), (uint8_t)(ap_ip_params.gateway >> 16), (uint8_t)(ap_ip_params.gateway >> 24));
    }

    netif_set_addr(pnetif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
    #endif
}

aiio_err_t aiio_wifi_init(void)
{
    //WPA3加密需要初始化ssl
    mbedtls_platform_set_calloc_free(aiio_socked_ssl_malloc, aiio_socked_ssl_free);

    rtl_cryptoEngine_init();
    wlan_network(); // 初始化wifi
    // 注册回调
    p_wlan_init_done_callback = aiio_wifi_inited_cb;  // wifi 初始化完成回调函数
    p_write_reconnect_ptr = aiio_wifi_reconnected_cb; // wifi重连接回调函数

    while (s_wifi_init_ok == 0)
    {
        vTaskDelay(10);
    }

    // AP模式下连接、断开回调
    wifi_reg_event_handler(WIFI_EVENT_STA_ASSOC, aiio_wifi_client_connected_cb, NULL);
    wifi_reg_event_handler(WIFI_EVENT_STA_DISASSOC, aiio_wifi_client_disconnected_cb, NULL);

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

    if (s_wifi_init_ok == 0)
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
        wifi_off();
        break;
    case AIIO_WIFI_MODE_STA:
    case AIIO_WIFI_MODE_AP:
    case AIIO_WIFI_MODE_APSTA:
        if(wifi_set_mode(mode) != RTW_SUCCESS)
        {
            s_wifi_mode = AIIO_WIFI_MODE_NULL;
            ret = AIIO_ERROR;
            goto __err;
        }
        // wifi_off();
        // vTaskDelay(20);
        // if (wifi_on(mode) != RTW_SUCCESS)
        // {
        //     s_wifi_mode = AIIO_WIFI_MODE_NULL;
        //     ret = AIIO_ERROR;
        //     goto __err;
        // }
        break;
    default:
        ret = AIIO_ERROR;
        goto __err;
    }

    s_wifi_mode = mode;

__err:
    return ret;
}

aiio_err_t aiio_wifi_get_mode(aiio_wifi_mode_t *mode)
{
    if (mode == NULL)
    {
        return AIIO_ERROR;
    }

    if (s_wifi_init_ok == 0)
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

aiio_os_thread_handle_t rtl_wifi_connect_thread_handle = NULL;
aiio_err_t aiio_wifi_connect(void)
{
    #if 1   //切换为异步连接
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
    #else
    aiio_err_t ret = AIIO_ERROR;
    int mode;
    uint8_t assoc_by_bssid = 0, connect_channel, pscan_config;
    rtw_network_info_t wificfg = {0};	//sdk中使用的配置wifi信息的变量

    if (s_wifi_mode != AIIO_WIFI_MODE_STA && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        goto __err;
    }

    wificfg.key_id = -1;
    //ssid
    strcpy((char *)wificfg.ssid.val, wifi_config.sta.ssid);
    wificfg.ssid.len = strlen(wifi_config.sta.ssid);
	//pwd
    if (wifi_config.sta.password != NULL && 0 != strlen(wifi_config.sta.password))
    {
        int pwd_len = strlen(wifi_config.sta.password);
        if (pwd_len > 64 || (pwd_len < 8 && pwd_len != 5))
        {
            aiio_log_e("pwd len error: %d!!!", pwd_len);
            goto __err;
        }

        wificfg.password = wifi_config.sta.password;
        wificfg.password_len = pwd_len;
        wificfg.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }
    else
    {
        wificfg.security_type = RTW_SECURITY_OPEN;
    }
	//bssid
    if (wifi_config.sta.bssid[0] != 0 || wifi_config.sta.bssid[1] != 0 || wifi_config.sta.bssid[2] != 0 ||
        wifi_config.sta.bssid[3] != 0 || wifi_config.sta.bssid[4] != 0 || wifi_config.sta.bssid[5] != 0)
    {
        memcpy(wificfg.bssid.octet, wifi_config.sta.bssid, ETH_ALEN);
        assoc_by_bssid = 1;
    }
    wext_get_mode(WLAN0_NAME, &mode);
    if (mode == RTW_MODE_MASTER)
    {
        dhcps_deinit();
        if (wifi_set_mode(RTW_MODE_STA) < 0)
        {
            aiio_log_e("wifi set mode failed!!!");
            goto __err;
        }
    }

	//从扫描列表中获取密码类型和信道
	//the keyID may be not set for WEP which may be confued with WPA2
    if ((wificfg.security_type == RTW_SECURITY_UNKNOWN) || (wificfg.security_type == RTW_SECURITY_WPA2_AES_PSK))
    {
        int security_retry_count = 0;
        while (1)
        {
            if (aiio_get_wifi_info_by_ssid_from_scan_list((char *)wificfg.ssid.val, &wificfg.security_type, &connect_channel))
            {
                break;
            }
            security_retry_count++;
            if (security_retry_count >= 3)
            {
                aiio_log_e("get security timeout!!!");
                goto __err;
            }
        }

        if (wificfg.security_type == RTW_SECURITY_WEP_PSK || wificfg.security_type == RTW_SECURITY_WEP_SHARED)
        {
            wificfg.key_id = (wificfg.key_id < 0 || wificfg.key_id > 3) ? 0 : wificfg.key_id;
        }
    }
    pscan_config = PSCAN_ENABLE;
    if (connect_channel > 0 && connect_channel < 14)
    {
        wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
    }

    // wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb);
    if (assoc_by_bssid)
    {
        ret = wifi_connect_bssid(wificfg.bssid.octet, (char *)wificfg.ssid.val, wificfg.security_type, (char *)wificfg.password,
                                 ETH_ALEN, wificfg.ssid.len, wificfg.password_len, wificfg.key_id, NULL);
    }
    else
    {
        ret = wifi_connect((char *)wificfg.ssid.val, wificfg.security_type, (char *)wificfg.password, wificfg.ssid.len,
                           wificfg.password_len, wificfg.key_id, NULL);
    }

    if (ret != RTW_SUCCESS)
    {
        aiio_log_e("wifi connect failed!!!");
        goto __err;
    }

    s_wifi_state = AIIO_WIFI_STATE_CONNECTING;

    if (wifi_config.sta.use_dhcp)
    {
        // 使用DHCP
        ret = LwIP_DHCP(0, DHCP_START);
        if (ret != DHCP_ADDRESS_ASSIGNED)
        {
            aiio_log_e("LwIP_DHCP failed!!!");
            goto __err;
        }
    }
    else
    {
        // 加载静态IP信息
        struct netif *pnetif = &xnetif[0];
        aiio_lwip_use_static_ip(pnetif);
    }

    wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb, NULL); // 注册wifi断开的回调函数

    ret = AIIO_OK;
__err:
    return ret;
    #endif
}

aiio_err_t aiio_wifi_disconnect(void)
{
    char essid[33];
    if(get_eap_ctx_method()){
        //非普通连接模式(eg:EAP)
        return AIIO_OK;
    }
    if (wext_get_ssid(WLAN0_NAME, (unsigned char *) essid) >= 0)
    {
        wifi_disconnect();
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_stop(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if (s_wifi_init_ok == 0)
    {
        aiio_log_e("The wifi is not initialized!!!");
        goto __err;
    }

    switch (s_wifi_mode)
    {
    case AIIO_WIFI_MODE_AP:
        // wifi_off();
        s_wifi_mode = AIIO_WIFI_MODE_NULL;
        // wifi_suspend_softap();
        break;
    case AIIO_WIFI_MODE_APSTA:
        if (wifi_on(RTW_MODE_STA) != RTW_SUCCESS)
        {
            s_wifi_mode = AIIO_WIFI_MODE_NULL;
            aiio_log_e("apsta change to sta failed!");
            goto __err;
        }
        s_wifi_mode = AIIO_WIFI_MODE_STA;
        // wifi_suspend_softap();
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
#if 0
    sta_info_p->is_used = aiio_sta_info_temp[idx].is_used;
    sta_info_p->sta_idx = aiio_sta_info_temp[idx].sta_idx;
    sta_info_p->sta_mac[0] = aiio_sta_info_temp[idx].sta_mac[0];
    sta_info_p->sta_mac[1] = aiio_sta_info_temp[idx].sta_mac[1];
    sta_info_p->sta_mac[2] = aiio_sta_info_temp[idx].sta_mac[2];
    sta_info_p->sta_mac[3] = aiio_sta_info_temp[idx].sta_mac[3];
    sta_info_p->sta_mac[4] = aiio_sta_info_temp[idx].sta_mac[4];
    sta_info_p->sta_mac[5] = aiio_sta_info_temp[idx].sta_mac[5];

    // aiio_log_i("%02x:%02x:%02x:%02x:%02x:%02x",
    //             sta_info_p->sta_mac[0], sta_info_p->sta_mac[1], sta_info_p->sta_mac[2],
    //             sta_info_p->sta_mac[3], sta_info_p->sta_mac[4], sta_info_p->sta_mac[5]);

    return AIIO_OK;

#else
    aiio_err_t ret = AIIO_ERROR;

    struct
    {
        int count;
        rtw_mac_t mac_list[wifi_config.ap.max_connection];
    } client_info;

    client_info.count = wifi_config.ap.max_connection;

    wifi_get_associated_client_list(&client_info, sizeof(client_info));

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
#endif
}

aiio_err_t aiio_wifi_ap_sta_list_get(uint8_t *sta_num, aiio_wifi_sta_basic_info_t *sta_info_p)
{
    if (s_wifi_mode != AIIO_WIFI_MODE_AP && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        aiio_log_e("wifi AP is not enabled");
        return AIIO_ERROR;
    }

    struct
    {
        int count;
        rtw_mac_t mac_list[wifi_config.ap.max_connection];
    } client_info;

    client_info.count = wifi_config.ap.max_connection;

    wifi_get_associated_client_list(&client_info, sizeof(client_info));

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
    

#if 0
    for (uint8_t i = 0, counter = 0; i < (sizeof(aiio_sta_info_temp) / sizeof(aiio_sta_info_temp[0])); i++)
    {
        if (!aiio_sta_info_temp[i].is_used)
        {
            continue;
        }
        (sta_info_p + counter)->sta_idx = aiio_sta_info_temp[i].sta_idx;
        (sta_info_p + counter)->sta_mac[0] = aiio_sta_info_temp[i].sta_mac[0];
        (sta_info_p + counter)->sta_mac[1] = aiio_sta_info_temp[i].sta_mac[1];
        (sta_info_p + counter)->sta_mac[2] = aiio_sta_info_temp[i].sta_mac[2];
        (sta_info_p + counter)->sta_mac[3] = aiio_sta_info_temp[i].sta_mac[3];
        (sta_info_p + counter)->sta_mac[4] = aiio_sta_info_temp[i].sta_mac[4];
        (sta_info_p + counter)->sta_mac[5] = aiio_sta_info_temp[i].sta_mac[5];
        counter++;
    }
    *sta_num = counter;
#endif



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

static aiio_err_t aiio_wifi_ap_start(void)
{
    aiio_err_t ret = AIIO_ERROR;
    struct netif * pnetif;
    rtw_ap_info_t ap_info = { 0 };

    struct ip_addr start_ip, end_ip;
    uint32_t start_ip_u32, end_ip_u32;

    start_ip_u32 = (ap_ip_params.ip & 0x00FFFFFF) | ((uint32_t)wifi_config.ap.dhcp_params.start << 24);
    end_ip_u32 = (ap_ip_params.ip & 0x00FFFFFF) | ((uint32_t)wifi_config.ap.dhcp_params.end << 24);

    //设置DHCP路由池
    ip_addr_set_ip4_u32(&start_ip, start_ip_u32);
    ip_addr_set_ip4_u32(&end_ip, end_ip_u32);
    dhcps_set_addr_pool(1, &start_ip, &end_ip);

    ap_info.ssid.len = wifi_config.ap.ssid_len;
    strncpy((char *)ap_info.ssid.val, wifi_config.ap.ssid, sizeof(ap_info.ssid.val) - 1);
    ap_info.password = wifi_config.ap.password;
    ap_info.password_len = strlen(wifi_config.ap.password);
    ap_info.password_len >= 8 ? (ap_info.security_type = RTW_SECURITY_WPA2_AES_PSK) : (ap_info.security_type = RTW_SECURITY_OPEN);
    ap_info.channel = wifi_config.ap.channel;

    if (wext_set_sta_num(wifi_config.ap.max_connection) != 0)
    {
        aiio_log_e("wext_set_sta_num failed!");
        goto __err;
    }

    dhcps_deinit();

    if (wifi_config.ap.ssid_hidden)
    {
        if (RTW_SUCCESS != wifi_start_ap_with_hidden_ssid((char*)ap_info.ssid.val, ap_info.security_type, (char*)ap_info.password, ap_info.ssid.len, ap_info.password_len, ap_info.channel))
        {
            aiio_log_e("start hidden ap failed!");
            goto __err;
        }
    }
    else
    {
        if(RTW_SUCCESS != wifi_start_ap((char*)ap_info.ssid.val, ap_info.security_type, (char*)ap_info.password, ap_info.ssid.len, ap_info.password_len, ap_info.channel))
        {
            aiio_log_e("start ap failed!");
            goto __err;
        }
    }

    //启动AP
    int timeout = 200;
    while (1)
    {
        char essid[33];
        if (s_wifi_mode == AIIO_WIFI_MODE_AP)
        {
            if (wext_get_ssid(WLAN0_NAME, (unsigned char *)essid) > 0)
            {
                if (strcmp((const char *)essid, (const char *)ap_info.ssid.val) == 0)
                {
                    break;
                }
            }
        }
        else if (s_wifi_mode == AIIO_WIFI_MODE_APSTA)
        {
            if (wext_get_ssid(WLAN1_NAME, (unsigned char *)essid) > 0)
            {
                if (strcmp((const char *)essid, (const char *)ap_info.ssid.val) == 0)
                {
                    break;
                }
            }
        }

        if (--timeout == 0)
        {
            aiio_log_e("start ap timeout!");
            goto __err;
        }

        vTaskDelay(100);
    }

    if (s_wifi_mode == RTW_MODE_STA_AP)
    {
        pnetif = &xnetif[1];
    }
    else
    {
        pnetif = &xnetif[0];
    }

    aiio_lwip_use_static_ip(pnetif); // 先设置静态IP，随后再看是否需要开启DHCP
    if (wifi_config.ap.use_dhcp)
    {
        dhcps_init(pnetif); // 开启DHCP
    }

    //启动AP事件
    if (wifi_cb)
    {
        aiio_input_event_t aiio_event = {0};
        aiio_event.code = AIIO_WIFI_EVENT_AP_START;
        wifi_cb(&aiio_event, NULL);
    }

    ret = AIIO_OK;
__err:
    return ret;
}

aiio_err_t aiio_wifi_start(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if (s_wifi_init_ok == 0)
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
#if 0
    aiio_err_t ret = AIIO_ERROR;

    if (s_wifi_init_ok == 0)
    {
        aiio_log_e("The wifi is not initialized!!!");
        goto __err;
    }

    if (wifi_off() != RTW_SUCCESS)
    {
        aiio_log_e("wifi off failed!!!");
        goto __err;
    }

    ret = AIIO_OK;

__err:
    return ret;
#endif
}

aiio_err_t aiio_wifi_scan(void *data, aiio_scan_complete_cb_t cb, bool filter)
{
    (void)(filter);

    aiio_err_t ret = AIIO_ERROR;

    if (sp_ap_item != NULL)
    {
        aiio_log_e("sp_ap_item is not null!");
        goto __err;
    }

    sp_ap_item = malloc(MAX_SCAN_NUM * sizeof(aiio_wifi_ap_item_t));
    if (sp_ap_item == NULL)
    {
        aiio_log_e("sp_ap_item malloc failed!");
        goto __err;
    }

    s_scan_ap_num = 0;
    s_wifi_scan_done = false;
    if (wifi_scan_networks(aiio_wifi_scan_result_cb, NULL) != RTW_SUCCESS)
    {
        aiio_log_e("wifi_scan_networks error!");
        goto __err_free;
    }

    //等等扫描完成
    for (int i = 0; i < 10000; i++)
    { // 等待扫描结束，BW16扫描大概需要7s
        vTaskDelay(1);
        if (s_wifi_scan_done)
        {
            break;
        }
    }

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
    free(sp_ap_item);
    sp_ap_item = NULL;
__err:
    return ret;
}

//FIXME
aiio_err_t aiio_wifi_scan_adv(void *data, aiio_scan_complete_cb_t cb, bool filter, aiio_wifi_scan_params_t *scan_params)
{
    (void)(filter);
    (void)(scan_params);

    aiio_err_t ret = AIIO_ERROR;

    if (sp_ap_item != NULL)
    {
        aiio_log_e("sp_ap_item is not null!");
        goto __err;
    }

    sp_ap_item = malloc(MAX_SCAN_NUM * sizeof(aiio_wifi_ap_item_t));
    if (sp_ap_item == NULL)
    {
        aiio_log_e("sp_ap_item malloc failed!");
        goto __err;
    }

    s_scan_ap_num = 0;
    s_wifi_scan_done = false;
    if (wifi_scan_networks(aiio_wifi_scan_result_cb, NULL) != RTW_SUCCESS)
    {
        aiio_log_e("wifi_scan_networks error!");
        goto __err_free;
    }

    //等等扫描完成
    for (int i = 0; i < 10000; i++)
    { // 等待扫描结束，BW16扫描大概需要7s
        vTaskDelay(1);
        if (s_wifi_scan_done)
        {
            break;
        }
    }

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
    free(sp_ap_item);
    sp_ap_item = NULL;
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
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    uint8_t *tmp_mac = NULL;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            tmp_mac = LwIP_GetMAC(&xnetif[i]);
            rltk_wlan_statistic(i);
            wifi_get_setting((const char*)ifname[i], &setting);

            if (setting.mode != RTW_MODE_STA)
            {
                continue;
            }

            memcpy(mac, tmp_mac, 6);
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
}

aiio_err_t aiio_wifi_ap_mac_set(uint8_t *mac)
{
    aiio_log_e("This API is not supported!");
    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_mac_get(uint8_t *mac)
{
    aiio_err_t ret = AIIO_ERROR;
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    uint8_t *tmp_mac = NULL;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            tmp_mac = LwIP_GetMAC(&xnetif[i]);
            rltk_wlan_statistic(i);
            wifi_get_setting((const char*)ifname[i], &setting);

            if (setting.mode != RTW_MODE_AP)
            {
                continue;
            }

            memcpy(mac, tmp_mac, 6);
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
}

aiio_err_t aiio_wifi_sta_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    u8 *tmp_ip =NULL;
	u8 *tmp_gw = NULL;
	u8 *tmp_msk = NULL;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            tmp_ip = LwIP_GetIP(&xnetif[i]);
            tmp_gw = LwIP_GetGW(&xnetif[i]);
            tmp_msk = LwIP_GetMASK(&xnetif[i]);
            rltk_wlan_statistic(i);
            wifi_get_setting((const char *)ifname[i], &setting);

            if (setting.mode != RTW_MODE_STA)
            {
                continue;
            }

            memcpy(ip, tmp_ip, 4);
            memcpy(gw, tmp_gw, 4);
            memcpy(mask, tmp_msk, 4);
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
}

aiio_err_t aiio_wifi_sta_netif_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    aiio_err_t ret = AIIO_ERROR;
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    u8 *tmp_ip =NULL;
	u8 *tmp_gw = NULL;
	u8 *tmp_msk = NULL;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            tmp_ip = LwIP_GetIP(&xnetif[i]);
            tmp_gw = LwIP_GetGW(&xnetif[i]);
            tmp_msk = LwIP_GetMASK(&xnetif[i]);
            rltk_wlan_statistic(i);
            wifi_get_setting((const char *)ifname[i], &setting);

            if (setting.mode != RTW_MODE_STA)
            {
                continue;
            }

            memcpy(ip, tmp_ip, 4);
            memcpy(gw, tmp_gw, 4);
            memcpy(mask, tmp_msk, 4);
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
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
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    u8 *tmp_ip =NULL;
	u8 *tmp_gw = NULL;
	u8 *tmp_msk = NULL;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            tmp_ip = LwIP_GetIP(&xnetif[i]);
            tmp_gw = LwIP_GetGW(&xnetif[i]);
            tmp_msk = LwIP_GetMASK(&xnetif[i]);
            rltk_wlan_statistic(i);
            wifi_get_setting((const char *)ifname[i], &setting);

            if (setting.mode != RTW_MODE_AP)
            {
                continue;
            }

            memcpy(ip, tmp_ip, 4);
            memcpy(gw, tmp_gw, 4);
            memcpy(mask, tmp_msk, 4);
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
}

aiio_err_t aiio_wifi_set_country_code(char *country_code){
	uint8_t wifi_country_code=0;
	if(0==strcmp(country_code,"JP")){	//日本
		wifi_country_code=0x7D;
	}else if(0==strcmp(country_code,"US")){	//美国
		wifi_country_code=0x1B;
	}else if(0==strcmp(country_code,"EU")){	//欧盟
		wifi_country_code=0x5E;
	}else if(0==strcmp(country_code,"MN")){	//蒙古国
		wifi_country_code=0x5E;
	}else if(0==strcmp(country_code,"CN")){	//中国
		wifi_country_code=0x06;
	}else{
		aiio_log_e("not find country_code:%s",country_code);
		return AIIO_ERROR;
	}
	aiio_log_e("set country_code:%s val=0x%02x",country_code,wifi_country_code);
	wifi_change_channel_plan(wifi_country_code); //这个设置国家码支持BW16的2.4G和5G
	return AIIO_OK;
}

aiio_err_t aiio_wifi_channel_get(int32_t *channel)
{
    if (wifi_get_channel(channel) != RTW_SUCCESS)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_channel_set(int32_t channel)
{
     if (wifi_set_channel(channel) != RTW_SUCCESS)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_rssi_get(int32_t *rssi)
{
    if (wifi_get_rssi(rssi) != RTW_SUCCESS)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_connect_ind_stat_get(aiio_wifi_sta_connect_ind_stat_info_t *wifi_ind_stat)
{
    aiio_err_t ret = AIIO_ERROR;
    rtw_wifi_setting_t setting;
    u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};
    u8 mac_1[6];

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            wext_get_bssid((const char *)ifname[i], mac_1);
            rltk_wlan_statistic(i);
            if (wifi_get_setting((const char*)ifname[i], &setting) != 0)
            {
                continue;
            }

            if (setting.mode != RTW_MODE_STA)
            {
                continue;
            }

            switch (setting.security_type)
            { // 加密类型
            case RTW_SECURITY_WPS_OPEN:
            case RTW_SECURITY_OPEN:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_OPEN;
                break;
            case RTW_SECURITY_WEP_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_PSK;
                break;
            case RTW_SECURITY_WEP_SHARED:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_SHARED;
                break;
            case RTW_SECURITY_WPS_SECURE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPS;
                break;
            case RTW_SECURITY_WPA_TKIP_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_TKIP_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_AES_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_PSK;
                break;
			case RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE:
				wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_ENTERPRISE;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_PSK:
				wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_MIXED_PSK;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE:
				wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_ENTERPRISE;
				break;
            case RTW_SECURITY_WPA_AES_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_PSK;
                break;
            case RTW_SECURITY_WPA_AES_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_MIXED_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA_MIXED_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA2_WPA3_MIXED:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_WPA3_MIXED;
                break;
            case RTW_SECURITY_WPA2_AES_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_PSK;
                break;
            case RTW_SECURITY_WPA2_AES_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_AES_CMAC:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_CMAC;
                break;
            case RTW_SECURITY_WPA2_TKIP_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA2_TKIP_ENTERPRISE:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA3_AES_PSK:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA3_AES_PSK;
                break;
            default:
                wifi_ind_stat->security = AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
                break;
            }

            strncpy(wifi_ind_stat->ssid, setting.ssid, sizeof(wifi_ind_stat->ssid) - 1);
            strncpy(wifi_ind_stat->passphr, setting.password, sizeof(wifi_ind_stat->passphr) - 1);
            memcpy(wifi_ind_stat->bssid, mac_1, 6);
            wifi_ind_stat->chan_id = setting.channel;
            
            ret = AIIO_OK;
            break;
        }
    }

    return ret;
}

aiio_err_t aiio_wifi_sniffer_enable(void)
{
    wifi_enter_promisc_mode();
    wifi_set_promisc(RTW_PROMISC_ENABLE_2, aiio_wifi_sniffer_cb, 1);

    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_disable(void)
{
    aiio_err_t ret = AIIO_ERROR;

    if(is_promisc_enabled())
    {
        wifi_set_promisc(RTW_PROMISC_DISABLE, NULL, 0);
        ret = AIIO_OK;
    }

    return ret;
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

aiio_wifi_beacon_auth_t aiio_wifi_get_sta_security(void)
{
    aiio_wifi_beacon_auth_t sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;

    for (size_t i = 0; i < 2; i++)
    {
        if (rltk_wlan_running(i))
        {
            rtw_wifi_setting_t setting;
            u8 *ifname[2] = {(u8*)WLAN0_NAME,(u8*)WLAN1_NAME};

            rltk_wlan_statistic(i);
            if (wifi_get_setting((const char*)ifname[i], &setting) != 0)
            {
                continue;
            }

            if (setting.mode != RTW_MODE_STA)
            {
                continue;
            }

            switch (setting.security_type)
            { // 加密类型
            case RTW_SECURITY_WPS_OPEN:
            case RTW_SECURITY_OPEN:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_OPEN;
                break;
            case RTW_SECURITY_WEP_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_PSK;
                break;
            case RTW_SECURITY_WEP_SHARED:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WEP_SHARED;
                break;
            case RTW_SECURITY_WPS_SECURE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPS;
                break;
            case RTW_SECURITY_WPA_TKIP_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_TKIP_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA_WPA2_TKIP_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_WPA2_AES_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_PSK;
                break;
			case RTW_SECURITY_WPA_WPA2_AES_ENTERPRISE:
				sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_AES_ENTERPRISE;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_PSK:
				sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_MIXED_PSK;
				break;
			case RTW_SECURITY_WPA_WPA2_MIXED_ENTERPRISE:
				sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_WPA2_ENTERPRISE;
				break;
            case RTW_SECURITY_WPA_AES_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_PSK;
                break;
            case RTW_SECURITY_WPA_AES_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA_MIXED_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA_MIXED_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA_MIXED_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_MIXED_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_MIXED_PSK;
                break;
            case RTW_SECURITY_WPA2_WPA3_MIXED:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_WPA3_MIXED;
                break;
            case RTW_SECURITY_WPA2_AES_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_PSK;
                break;
            case RTW_SECURITY_WPA2_AES_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA2_AES_CMAC:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_AES_CMAC;
                break;
            case RTW_SECURITY_WPA2_TKIP_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_PSK;
                break;
            case RTW_SECURITY_WPA2_TKIP_ENTERPRISE:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA2_TKIP_ENTERPRISE;
                break;
            case RTW_SECURITY_WPA3_AES_PSK:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_WPA3_AES_PSK;
                break;
            default:
                sta_auth = AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
                break;
            }

            break;
        }
    }
    
    return sta_auth;
}

static aiio_os_function_return_t rtl_wifi_connect_default(void){
    aiio_input_event_t aiio_event = {.code = AIIO_WIFI_EVENT_STA_DISCONNECTED};
    int mode;
    uint8_t assoc_by_bssid = 0, connect_channel, pscan_config;
    rtw_network_info_t wificfg = {0};	//sdk中使用的配置wifi信息的变量

    if (s_wifi_mode != AIIO_WIFI_MODE_STA && s_wifi_mode != AIIO_WIFI_MODE_APSTA)
    {
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
    }

    wificfg.key_id = -1;
    //ssid
    strcpy((char *)wificfg.ssid.val, wifi_config.sta.ssid);
    wificfg.ssid.len = strlen(wifi_config.sta.ssid);
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

        wificfg.password = wifi_config.sta.password;
        wificfg.password_len = pwd_len;
        wificfg.security_type = RTW_SECURITY_WPA2_AES_PSK;
    }
    else
    {
        wificfg.security_type = RTW_SECURITY_OPEN;
    }
	//bssid
    if (wifi_config.sta.bssid[0] != 0 || wifi_config.sta.bssid[1] != 0 || wifi_config.sta.bssid[2] != 0 ||
        wifi_config.sta.bssid[3] != 0 || wifi_config.sta.bssid[4] != 0 || wifi_config.sta.bssid[5] != 0)
    {
        memcpy(wificfg.bssid.octet, wifi_config.sta.bssid, ETH_ALEN);
        assoc_by_bssid = 1;
    }
    wext_get_mode(WLAN0_NAME, &mode);
    if (mode == RTW_MODE_MASTER)
    {
        dhcps_deinit();
        if (wifi_set_mode(RTW_MODE_STA) < 0)
        {
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            aiio_log_e("wifi set mode failed!!!");
            goto __err;
        }
    }

	//从扫描列表中获取密码类型和信道
	//the keyID may be not set for WEP which may be confued with WPA2
    if ((wificfg.security_type == RTW_SECURITY_UNKNOWN) || (wificfg.security_type == RTW_SECURITY_WPA2_AES_PSK))
    {
        int security_retry_count = 0;
        while (1)
        {
            if (aiio_get_wifi_info_by_ssid_from_scan_list((char *)wificfg.ssid.val, &wificfg.security_type, &connect_channel))
            {
                break;
            }
            security_retry_count++;
            if (security_retry_count >= 3)
            {
                aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_NOT_FOUND_AP;
                aiio_log_e("get security timeout!!!");
                goto __err;
            }
        }

        if (wificfg.security_type == RTW_SECURITY_WEP_PSK || wificfg.security_type == RTW_SECURITY_WEP_SHARED)
        {
            wificfg.key_id = (wificfg.key_id < 0 || wificfg.key_id > 3) ? 0 : wificfg.key_id;
        }
    }
    pscan_config = PSCAN_ENABLE;
    if (connect_channel > 0 && connect_channel < 14)
    {
        wifi_set_pscan_chan(&connect_channel, &pscan_config, 1);
    }

    wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb);
    int ret=RTW_ERROR;
    s_wifi_state = AIIO_WIFI_STATE_CONNECTING;
    if (assoc_by_bssid)
    {
        ret = wifi_connect_bssid(wificfg.bssid.octet, (char *)wificfg.ssid.val, wificfg.security_type, (char *)wificfg.password,
                                 ETH_ALEN, wificfg.ssid.len, wificfg.password_len, wificfg.key_id, NULL);
    }
    else
    {
        ret = wifi_connect((char *)wificfg.ssid.val, wificfg.security_type, (char *)wificfg.password, wificfg.ssid.len,
                           wificfg.password_len, wificfg.key_id, NULL);
    }

    if (ret != RTW_SUCCESS)
    {
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        aiio_log_e("wifi connect failed!!!");
        goto __err;
    }

    if (wifi_config.sta.use_dhcp)
    {
        // 使用DHCP
        ret = LwIP_DHCP(0, DHCP_START);
        if (ret != DHCP_ADDRESS_ASSIGNED)
        {
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            aiio_log_e("LwIP_DHCP failed!!!");
            goto __err;
        }
    }
    else
    {
        // 加载静态IP信息
        struct netif *pnetif = &xnetif[0];
        aiio_lwip_use_static_ip(pnetif);
    }

    wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb, NULL); // 注册wifi断开的回调函数
    return AIIO_OK;

__err:
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

    wifi_set_autoreconnect(0);
    wifi_unreg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb);
    wext_get_mode(WLAN0_NAME, &mode);
    if (mode == RTW_MODE_MASTER)
    {
        dhcps_deinit();
        if (wifi_set_mode(RTW_MODE_STA) < 0)
        {
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            aiio_log_e("wifi set mode failed!!!");
            goto __err;
        }
    }
    
    eap_target_ssid=wifi_config.sta.ssid;
	eap_identity=wifi_config.sta.staIdentity;
	eap_password=wifi_config.sta.password;
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
	
/*
	Verify server's certificate is an optional feature.
	If you want to use it please make sure ENABLE_EAP_SSL_VERIFY_SERVER in platform_opts.h is set to 1,
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

	eap_client_cert_len = strlen(eap_client_cert)+1;
	eap_client_key_len = strlen(eap_client_key)+1;
	eap_ca_cert_len = strlen(eap_ca_cert)+1;
	aiio_log_e("[%s()-%d]Type=%d ssid=%s identity=%s password=%s\r\n",__func__,__LINE__,wifi_config.sta.eapType,eap_target_ssid,eap_identity,eap_password);
    //RtlEapStart(wifi_config.sta.eapType);
    while(!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
	if(rltk_wlan_running(WLAN1_IDX)){
		aiio_log_d("[%s()-%d]wifi not running,Not support con-current mode!\r\n",__func__,__LINE__);
		aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
	}
    judge_station_disconnect();
	switch(wifi_config.sta.eapType){
		case AIIO_WIFI_EAP_TYPE_PEAP:	//PEAP认证
			ret = set_eap_peap_method();
			break;
		case AIIO_WIFI_EAP_TYPE_TLS:	//TLS认证
			ret = set_eap_tls_method();
			break;
		case AIIO_WIFI_EAP_TYPE_TTLS:	//TTLS认证
			ret = set_eap_ttls_method();
			break;
		case AIIO_WIFI_EAP_TYPE_FAST:	//FAST认证
			ret = set_eap_fast_method();
			break;
		default:
			aiio_log_e("eapType type=%d error",wifi_config.sta.eapType);
            aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
            goto __err;
	}
	if(ret == -1){
		aiio_log_e("[%s()-%d]neap method %d not supported\r\n",__func__,__LINE__,wifi_config.sta.eapType);
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
	}
	eap_method = get_eap_ctx_method();
	aiio_log_e("[%s()-%d]start type=%d\r\n",__func__,__LINE__,wifi_config.sta.eapType);
	//eap_config();
	set_eap_phase(ENABLE);
	wifi_reg_event_handler(WIFI_EVENT_EAPOL_START, eap_eapol_start_hdl, NULL);
	if (eap_eapol_recvd_stack!=NULL){
		wifi_reg_event_handler(WIFI_EVENT_EAPOL_RECVD, eap_eapol_recvd_hdl, (void *)eap_eapol_recvd_stack);
	}else{
		wifi_reg_event_handler(WIFI_EVENT_EAPOL_RECVD, eap_eapol_recvd_hdl, NULL);
	}
	ret = connect_by_open_system(eap_target_ssid);
	/* Start DHCPClient */
	if(ret == 0){
		LwIP_DHCP(0, DHCP_START);
	}
	wifi_unreg_event_handler(WIFI_EVENT_EAPOL_START, eap_eapol_start_hdl);
	// for re-authentication when session timeout
	//wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, eap_disconnected_hdl, NULL);
    wifi_reg_event_handler(WIFI_EVENT_DISCONNECT, aiio_wifi_disconnected_cb, NULL); // 注册wifi断开的回调函数
	//wifi_unreg_event_handler(WIFI_EVENT_EAPOL_RECVD, eap_eapol_recvd_hdl);
	set_eap_phase(DISABLE);	
	// eap failed, disconnect
	if(ret != 0){
		judge_station_disconnect();
		eap_disconnected_hdl(NULL, 0, 0, NULL);
		rtw_msleep_os(200);	//wait handler done
		aiio_log_e("[%s()-%d]ERROR: connect to AP by %d failed\r\n",__func__,__LINE__,wifi_config.sta.eapType);
        aiio_event.value = AIIO_AT_CONNECT_STATUS_CONNECT_FAIL;
        goto __err;
	}
    wifi_set_autoreconnect(1);
	aiio_log_i("[%s()-%d]finish\r\n",__func__,__LINE__);

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

aiio_err_t aiio_wifi_set_hostname(char *hostname)
{
    extern void rtl_set_hostname(const char *hostname);
    rtl_set_hostname(hostname);
    return AIIO_OK;
}