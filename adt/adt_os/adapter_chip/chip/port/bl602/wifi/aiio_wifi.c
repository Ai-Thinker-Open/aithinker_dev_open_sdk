/** @brief      Wifi application interface.
 *
 *  @file       aiio_wifi.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Wifi application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/27      <td>1.0.0       <td>wusen       <td>fix format
 *  <tr><td>2023/03/03      <td>1.0.1       <td>wusen       <td>fix func & bug
 *  <tr><td>2023/03/09      <td>1.0.2       <td>wusen       <td>add func & fix bug
 *  <tr><td>2023/03/10      <td>1.0.3       <td>wusen       <td>add sniffer mode interface
 *  </table>
 *
 */
#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vfs.h>
#include <aos/kernel.h>
#include <aos/yloop.h>
#include <bl_wifi.h>
#include <hal_wifi.h>
#include <bl_timer.h>
#include <hal_sys.h>
#include <easyflash.h>
#include <bl60x_fw_api.h>
#include <wifi_mgmr_ext.h>
#include "../wifi_mgmr.h"
#include <lwip/tcpip.h>
#include "aiio_wifi.h"

static aiio_wifi_conf_t aiio_conf =
{
    .country_code = "CN",
};

#define IP_SET_ADDR(a, b, c, d) (((uint32_t)((a)&0xff)) |       \
                                 ((uint32_t)((b)&0xff) << 8) |  \
                                 ((uint32_t)((c)&0xff) << 16) | \
                                 ((uint32_t)((d)&0xff) << 24))

static wifi_interface_t g_wifi_ap_interface=NULL;
static wifi_interface_t g_wifi_sta_interface=NULL;

static uint8_t g_wifi_ap_init_flag=0;
static uint8_t g_wifi_sta_init_flag=0;


static aiio_wifi_ip_params_t ap_ip_params = { 0 };
static aiio_wifi_ip_params_t sta_ip_params = { 0 };


static aiio_event_cb wifi_cb;

aiio_wifi_config_t wifi_config = { 0 };

static void event_cb_wifi_event(input_event_t *event, void *private_data)
{
    static char *ssid;
    static char *password;
    struct sm_connect_tlv_desc* ele = NULL;
    aiio_input_event_t aiio_event={0};
    memcpy(&aiio_event,event,sizeof(aiio_input_event_t));
    switch (event->code) {
        case CODE_WIFI_ON_INIT_DONE:
        {
            aiio_log_d("[APP] [EVT] INIT DONE %lld", aos_now_ms());
            wifi_mgmr_start_background(&aiio_conf);
        }
        break;
        case CODE_WIFI_ON_MGMR_DONE:
        {
            aiio_log_d("[APP] [EVT] MGMR DONE %lld, now %lums", aos_now_ms(), bl_timer_now_us()/1000);
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_WIFI_READY;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_MGMR_DENOISE:
        {
            aiio_log_d("[APP] [EVT] Microwave Denoise is ON %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE:
        {
            aiio_log_d("[APP] [EVT] SCAN Done %lld", aos_now_ms());
            // wifi_mgmr_cli_scanlist();
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_SCAN_DONE;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_SCAN_DONE_ONJOIN:
        {
            aiio_log_d("[APP] [EVT] SCAN On Join %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_DISCONNECT:
        {
            aiio_log_d("[APP] [EVT] disconnect %lld, Reason: %s",
                aos_now_ms(),
                wifi_mgmr_status_code_str(event->value)
            );
            /* Get every ele in diagnose tlv data */
            while ((ele = wifi_mgmr_diagnose_tlv_get_ele()))
            {
                aiio_log_d("[APP] [EVT] diagnose tlv data %lld, id: %d, len: %d, data: %p", aos_now_ms(),
                    ele->id, ele->len, ele->data);

                /* MUST free diagnose tlv data ele */
                wifi_mgmr_diagnose_tlv_free_ele(ele);
            }

            aiio_log_d("[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_STA_DISCONNECTED;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_CONNECTING:
        {
            aiio_log_d("[APP] [EVT] Connecting %lld", aos_now_ms());
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_STA_CONNECTING;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_CMD_RECONNECT:
        {
            aiio_log_d("[APP] [EVT] Reconnect %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_CONNECTED:
        {
            /* Get every ele in diagnose tlv data */
            while ((ele = wifi_mgmr_diagnose_tlv_get_ele()))
            {
                aiio_log_d("[APP] [EVT] diagnose tlv data %lld, id: %d, len: %d, data: %p", aos_now_ms(),
                    ele->id, ele->len, ele->data);

                /* MUST free diagnose tlv data */
                wifi_mgmr_diagnose_tlv_free_ele(ele);
            }

            aiio_log_d("[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());
            aiio_log_d("[APP] [EVT] connected %lld", aos_now_ms());
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_STA_CONNECTED;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_PRE_GOT_IP:
        {
            aiio_log_d("[APP] [EVT] connected %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_GOT_IP:
        {
            aiio_log_d("[APP] [EVT] GOT IP %lld", aos_now_ms());
            aiio_log_d("[SYS] Memory left is %d Bytes", xPortGetFreeHeapSize());
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_STA_GOT_IP;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_EMERGENCY_MAC:
        {
            aiio_log_d("[APP] [EVT] EMERGENCY MAC %lld", aos_now_ms());
            hal_reboot();//one way of handling emergency is reboot. Maybe we should also consider solutions
        }
        break;
        case CODE_WIFI_ON_PROV_SSID:
        {
            aiio_log_d("[APP] [EVT] [PROV] [SSID] %lld: %s",
                    aos_now_ms(),
                    event->value ? (const char*)event->value : "UNKNOWN"
            );
            if (ssid) {
                vPortFree(ssid);
                ssid = NULL;
            }
            ssid = (char*)event->value;
        }
        break;
        case CODE_WIFI_ON_PROV_BSSID:
        {
            aiio_log_d("[APP] [EVT] [PROV] [BSSID] %lld: %s",
                    aos_now_ms(),
                    event->value ? (const char*)event->value : "UNKNOWN"
            );
            if (event->value) {
                vPortFree((void*)event->value);
            }
        }
        break;
        case CODE_WIFI_ON_PROV_PASSWD:
        {
            aiio_log_d("[APP] [EVT] [PROV] [PASSWD] %lld: %s", aos_now_ms(),
                    event->value ? (const char*)event->value : "UNKNOWN"
            );
            if (password) {
                vPortFree(password);
                password = NULL;
            }
            password = (char*)event->value;
        }
        break;
        case CODE_WIFI_ON_PROV_CONNECT:
        {
            aiio_log_d("[APP] [EVT] [PROV] [CONNECT] %lld", aos_now_ms());

        }
        break;
        case CODE_WIFI_ON_PROV_DISCONNECT:
        {
            aiio_log_d("[APP] [EVT] [PROV] [DISCONNECT] %lld", aos_now_ms());
        }
        break;
        case CODE_WIFI_ON_AP_STARTED:
        {
            aiio_log_d("[APP] [EVT] [AP] [STARTED] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_AP_START;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_AP_STOPPED:
        {
            aiio_log_d("[APP] [EVT] [AP] [STOPPED] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_AP_STOP;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_AP_STA_ADD:
        {
            aiio_log_d("[APP] [EVT] [AP] [ADD] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_AP_STACONNECTED;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        case CODE_WIFI_ON_AP_STA_DEL:
        {
            aiio_log_d("[APP] [EVT] [AP] [DEL] %lld, sta idx is %lu", aos_now_ms(), (uint32_t)event->value);
            if(wifi_cb)
            {
                aiio_event.code=AIIO_WIFI_EVENT_AP_STADISCONNECTED;
                wifi_cb(&aiio_event, private_data);
            }
        }
        break;
        default:
        {
            aiio_log_d("[APP] [EVT] Unknown code %u, %lld", event->code, aos_now_ms());
            /*nothing*/
        }
    }
}

aiio_err_t aiio_wifi_init(void)
{

    /*wifi fw stack and thread stuff*/
    static uint8_t stack_wifi_init  = 0;

    if (1 == stack_wifi_init) {
        aiio_log_e("Wi-Fi Stack Started already!!!");
        return AIIO_ERROR;
    }
    stack_wifi_init = 1;

    tcpip_init(NULL, NULL);

    aos_register_event_filter(EV_WIFI, event_cb_wifi_event, NULL);

    hal_wifi_start_firmware_task();
    /*Trigger to start Wi-Fi*/
    aos_post_event(EV_WIFI, CODE_WIFI_ON_INIT_DONE, 0);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_register_event_cb(aiio_event_cb cb)
{
    wifi_cb = cb;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_unregister_event_cb(void)
{
    wifi_cb=NULL;
    return AIIO_OK;
}



aiio_err_t aiio_wifi_set_mode(aiio_wifi_mode_t mode)
{

    if(mode==AIIO_WIFI_MODE_STA){
        g_wifi_sta_interface = wifi_mgmr_sta_enable();
    }else if(mode==AIIO_WIFI_MODE_AP){
        g_wifi_ap_interface = wifi_mgmr_ap_enable();
    }else if(mode==AIIO_WIFI_MODE_APSTA){
        g_wifi_ap_interface = wifi_mgmr_ap_enable();
        g_wifi_sta_interface = wifi_mgmr_sta_enable();
    }else{
        aiio_log_e("The mode dont support");
        g_wifi_ap_interface=NULL;
        g_wifi_sta_interface=NULL;
        g_wifi_sta_init_flag=0;
        g_wifi_ap_init_flag=0;
        return AIIO_ERROR;
    }
    return AIIO_OK;

}

aiio_err_t aiio_wifi_get_mode(aiio_wifi_mode_t *mode)
{
    if(mode==NULL){
        aiio_log_e("The get mode is null error");
        return AIIO_ERROR;
    }

    if(g_wifi_ap_interface!=NULL &&g_wifi_sta_interface==NULL){
        *mode=AIIO_WIFI_MODE_AP;
    }else if(g_wifi_sta_interface!=NULL && g_wifi_ap_interface==NULL){
        *mode=AIIO_WIFI_MODE_STA;
    }else if(g_wifi_sta_interface!=NULL && g_wifi_ap_interface!=NULL){
        *mode=AIIO_WIFI_MODE_APSTA;
    }else{
        *mode=AIIO_WIFI_MODE_NULL;
    }
    return AIIO_OK;
}


aiio_err_t aiio_wifi_set_config(aiio_wifi_port_t port,aiio_wifi_config_t *conf)
{
    if(conf==NULL){
        aiio_log_e("The conf is null error");
        return AIIO_ERROR;
    }

    if(port==AIIO_WIFI_IF_STA){  //sta config
        if(conf->sta.ssid){
            strncpy((char*)wifi_config.sta.ssid,(char*)conf->sta.ssid, SSID_LEN);
        }
        if(conf->sta.password){
            strncpy((char*)wifi_config.sta.password,(char*)conf->sta.password, PASSWORD_LEN);
        }
        if(conf->sta.bssid){
            strncpy((char*)wifi_config.sta.bssid,(char*)conf->sta.bssid, MAC_LEN);
        }
        if(conf->sta.band){
            wifi_config.sta.band=conf->sta.band;
        }
        if(conf->sta.channel){
            wifi_config.sta.channel=conf->sta.channel;
        }
        wifi_config.sta.use_dhcp=conf->sta.use_dhcp;
        wifi_config.sta.flags=conf->sta.flags;
    }else{   //ap config
        if(conf->ap.ssid){
            strncpy((char*)wifi_config.ap.ssid,(char*)conf->ap.ssid, SSID_LEN);
        }
        if(conf->ap.password){
            strncpy((char*)wifi_config.ap.password,(char*)conf->ap.password, PASSWORD_LEN);
        }
        if(conf->ap.max_connection){
            wifi_config.ap.max_connection=conf->ap.max_connection;
        }else{
            wifi_config.ap.max_connection=4;
        }
        if(conf->ap.channel){
            wifi_config.ap.channel=conf->ap.channel;
        }else{
            wifi_config.ap.channel=1;
        }
        if(conf->ap.ssid_hidden){
            wifi_config.ap.ssid_hidden=conf->ap.ssid_hidden;
        }else{
            wifi_config.ap.ssid_hidden=0;
        }
        if(conf->ap.ssid_len){
            wifi_config.ap.ssid_len=conf->ap.ssid_len;
        }
        if(conf->ap.dhcp_params.lease_time){
            wifi_config.ap.dhcp_params.lease_time=conf->ap.dhcp_params.lease_time;
        }else{
            wifi_config.ap.dhcp_params.lease_time=120;
        }
        if(conf->ap.dhcp_params.start){
            wifi_config.ap.dhcp_params.start=conf->ap.dhcp_params.start;
        }else{
            wifi_config.ap.dhcp_params.start=2;
        }
        if(conf->ap.dhcp_params.end){
            wifi_config.ap.dhcp_params.end=conf->ap.dhcp_params.end;
        }else{
            wifi_config.ap.dhcp_params.end=100;
        }
        wifi_config.ap.use_dhcp=conf->ap.use_dhcp;
    }
   return AIIO_OK;
}

aiio_err_t aiio_wifi_get_config(aiio_wifi_port_t port,aiio_wifi_config_t *conf)
{
    if(conf==NULL){
        aiio_log_e("The conf is null error");
        return AIIO_ERROR;
    }
    if(port==AIIO_WIFI_IF_STA){  //sta config
        strncpy((char*)conf->sta.ssid, (char*)wifi_config.sta.ssid,SSID_LEN);
        strncpy((char*)conf->sta.password, (char*)wifi_config.sta.password,PASSWORD_LEN);
        strncpy((char*)conf->sta.bssid,(char*)wifi_config.sta.bssid, MAC_LEN);
        conf->sta.band=wifi_config.sta.band;
        conf->sta.channel=wifi_config.sta.channel;
        conf->sta.use_dhcp=wifi_config.sta.use_dhcp;
        conf->sta.flags=wifi_config.sta.flags;
   }else{   //ap config
        strncpy((char*)conf->ap.ssid,(char*)wifi_config.ap.ssid, SSID_LEN);
        strncpy((char*)conf->ap.password, (char*)wifi_config.ap.password,PASSWORD_LEN);
        conf->ap.max_connection=wifi_config.ap.max_connection;
        conf->ap.channel=wifi_config.ap.channel;
        conf->ap.ssid_hidden=wifi_config.ap.ssid_hidden;
        conf->ap.ssid_len=wifi_config.ap.ssid_len;
        conf->ap.use_dhcp=wifi_config.ap.use_dhcp;
        conf->ap.dhcp_params.lease_time=wifi_config.ap.dhcp_params.lease_time;
        conf->ap.dhcp_params.start=wifi_config.ap.dhcp_params.start;
        conf->ap.dhcp_params.end=wifi_config.ap.dhcp_params.end;

   }
   return AIIO_OK;
}

aiio_err_t aiio_wifi_connect(void)
{
    char pmk[PMK_LEN];
    memset(pmk, 0, PMK_LEN);
    if(g_wifi_sta_interface){
        wifi_mgmr_sta_autoconnect_enable();
        // wifi_mgmr_psk_cal(
        //             (char*)wifi_config.sta.password,
        //             (char*)wifi_config.sta.ssid,
        //             strlen((char*)wifi_config.sta.ssid),
        //             pmk
        //     );
        return wifi_mgmr_sta_connect_mid(g_wifi_sta_interface, (char*)wifi_config.sta.ssid, (char*)wifi_config.sta.password,
                                    NULL,wifi_config.sta.bssid, wifi_config.sta.band, wifi_config.sta.channel,
                                    wifi_config.sta.use_dhcp,wifi_config.sta.flags);
    }
    return AIIO_ERROR;
}
aiio_err_t aiio_wifi_disconnect(void)
{
    int32_t state;
    if(g_wifi_sta_interface){
        wifi_mgmr_sta_autoconnect_disable();
        wifi_mgmr_sta_disconnect();
        /*XXX Must make sure sta is already disconnect, otherwise sta disable won't work*/
        bl_os_msleep(CONFIG_AXK_WIFI_STA_DISCONNECT_DELAY);
        for (int32_t i = 0; i < 200; i++){
            wifi_mgmr_sta_state_get(&state);
            if (state == WIFI_STATE_IDLE || state == WIFI_STATE_WITH_AP_IDLE ||
                state == WIFI_STATE_DISCONNECT || state == WIFI_STATE_WITH_AP_DISCONNECT){
                break;
            }
            aiio_os_tick_dealy(aiio_os_ms2tick(100));
        }
        wifi_mgmr_sta_disable(NULL);
		for (int i = 0; i < 200; i++){
			wifi_mgmr_sta_state_get(&state);
			if (state == WIFI_STATE_IDLE || state == WIFI_STATE_WITH_AP_IDLE){
				break;
			}
			aiio_os_tick_dealy(aiio_os_ms2tick(100));
		}
		g_wifi_sta_interface = NULL;
        g_wifi_sta_init_flag=0;
        return AIIO_OK;
    }
    return AIIO_ERROR;

}

aiio_err_t aiio_wifi_ap_sta_info_get(aiio_wifi_sta_basic_info_t *sta_info_p,uint8_t idx)
{
    return wifi_mgmr_ap_sta_info_get(sta_info_p,idx);
}

aiio_err_t aiio_wifi_ap_sta_list_get(uint8_t *sta_num,aiio_wifi_sta_basic_info_t *sta_info_p)
{
    int32_t state = WIFI_STATE_UNKNOWN;
    uint8_t sta_cnt = 0;
    uint8_t i=0;
    uint8_t j=0;
    struct wifi_sta_basic_info sta_info;
    long long sta_time;
    uint8_t index = 0;
    uint8_t counter =0;

    wifi_mgmr_state_get(&state);
    if (!WIFI_STATE_AP_IS_ENABLED(state)){
        aiio_log_e("wifi AP is not enabled, state = %d", state);
        return AIIO_ERROR;
    }

    wifi_mgmr_ap_sta_cnt_get(&sta_cnt);
    if (!sta_cnt){
        aiio_log_e("no sta connect current AP, sta_cnt = %d", sta_cnt);
        return AIIO_ERROR;
    }

    memset(&sta_info, 0, sizeof(struct wifi_sta_basic_info));
    for(i = 0;i < sta_cnt;i++){
        wifi_mgmr_ap_sta_info_get(&sta_info, i);
        if (!sta_info.is_used || (sta_info.sta_idx == 0xef)){
            continue;
        }
        counter++;

        sta_time = (long long)sta_info.tsfhi;
        sta_time = (sta_time << 32) | sta_info.tsflo;

        for(j = 0;j < sizeof(data_rate_list)/sizeof(data_rate_list[0]);j++) {
            if(data_rate_list[j].data_rate == sta_info.data_rate) {
                index = j;
                break;
            }
        }

        (sta_info_p+i)->sta_idx=sta_info.sta_idx;
        (sta_info_p+i)->sta_mac[0]=sta_info.sta_mac[0];
        (sta_info_p+i)->sta_mac[1]=sta_info.sta_mac[1];
        (sta_info_p+i)->sta_mac[2]=sta_info.sta_mac[2];
        (sta_info_p+i)->sta_mac[3]=sta_info.sta_mac[3];
        (sta_info_p+i)->sta_mac[4]=sta_info.sta_mac[4];
        (sta_info_p+i)->sta_mac[5]=sta_info.sta_mac[5];
        (sta_info_p+i)->rssi=sta_info.rssi;
        (sta_info_p+i)->tsfhi=sta_info.tsfhi;
        (sta_info_p+i)->tsflo=sta_info.tsflo;
        (sta_info_p+i)->data_rate=sta_info.data_rate;
        (sta_info_p+i)->is_used=sta_info.is_used;
    }
    *sta_num=counter;
    return AIIO_OK;
}

aiio_err_t aiio_wifi_ap_stop(void)
{
    int32_t i;
    struct wifi_sta_basic_info sta_info;
    uint8_t sta_cnt;
    uint8_t empty[6]={0};
    if (g_wifi_ap_interface)
    {
        wifi_mgmr_ap_sta_cnt_get(&sta_cnt);
        for (i = 0; i < sta_cnt; i++)
        {
            wifi_mgmr_ap_sta_info_get(&sta_info, i);
            if (!sta_info.is_used || (sta_info.sta_idx == 0xef))
                continue;

            if (memcmp(sta_info.sta_mac, empty, MAC_LEN) == 0)
                wifi_mgmr_ap_sta_delete(i);
        }
        wifi_mgmr_ap_stop(NULL);
        aiio_os_tick_dealy(aiio_os_ms2tick(500));
        g_wifi_ap_interface = NULL;
        g_wifi_ap_init_flag=0;
    }
    return AIIO_OK;

}


aiio_err_t aiio_wifi_conn_result_get(uint16_t *status_code, uint16_t *reason_code)
{
    wifi_mgmr_conn_result_get(status_code,reason_code);
    return AIIO_OK;
}

char *aiio_wifi_status_code_str(uint16_t status_code)
{
    return wifi_mgmr_status_code_str(status_code);
}


aiio_err_t aiio_wifi_get_state(int32_t *state)
{
    return wifi_mgmr_state_get(state);
}

aiio_err_t aiio_wifi_start(void)
{
    aiio_err_t ret=AIIO_ERROR;
    if(g_wifi_ap_interface && g_wifi_ap_init_flag ==0){
        wifi_mgmr_conf_max_sta(wifi_config.ap.max_connection);
        if(wifi_config.ap.use_dhcp){
            wifi_mgmr_ap_dhcp_enable();
            wifi_mgmr_ap_dhcp_range_set(ap_ip_params.ip, ap_ip_params.netmask, wifi_config.ap.dhcp_params.start, wifi_config.ap.dhcp_params.end);
        }else{
            wifi_mgmr_ap_dhcp_disable();
        }
        ret=wifi_mgmr_ap_start(g_wifi_ap_interface, (char*)wifi_config.ap.ssid, wifi_config.ap.ssid_hidden, (char*)wifi_config.ap.password, wifi_config.ap.channel);
        g_wifi_ap_init_flag=1;
    }
    if(g_wifi_sta_interface && g_wifi_sta_init_flag ==0){
        ret=aiio_wifi_connect();
        g_wifi_sta_init_flag=1;
    }
    return ret;

}



aiio_err_t aiio_wifi_stop(void)
{
    if(g_wifi_ap_interface){
        aiio_wifi_ap_stop();
    }
    if(g_wifi_sta_interface){
        aiio_wifi_disconnect();
    }
    return AIIO_OK;
}



aiio_err_t aiio_wifi_scan(void *data, aiio_scan_complete_cb_t cb,bool filter)
{
    wifi_mgmr_scan_filter_hidden_ssid(filter);
    return wifi_mgmr_scan(data,cb);
}

aiio_err_t aiio_wifi_scan_adv(void *data, aiio_scan_complete_cb_t cb,bool filter,aiio_wifi_scan_params_t *scan_params)
{
    int32_t i;
    uint16_t channel_num = 0;
    uint16_t channels[CONFIG_MAX_FIXED_CHANNELS_LIMIT];
    for (i = 0; i < scan_params->channel_num; i++) {
        channels[i] = scan_params->channels[i];
    }
    channel_num = scan_params->channel_num;

    wifi_mgmr_scan_filter_hidden_ssid(filter);
    wifi_mgmr_scan_adv(data, cb, channels, channel_num, scan_params->bssid, scan_params->ssid, scan_params->scan_mode, scan_params->duration_scan);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_scan_ap_all(aiio_wifi_ap_item_t *env, uint32_t *param1, aiio_scan_item_cb_t cb)
{
    wifi_mgmr_scan_ap_all(env,param1,cb);
    return AIIO_OK;
}



aiio_err_t aiio_wifi_get_wifi_channel_conf(aiio_wifi_conf_t *wifi_chan_conf)
{
    wifi_mgmr_get_wifi_channel_conf(wifi_chan_conf);
    return AIIO_OK;
}


aiio_err_t aiio_wifi_sta_mac_set(uint8_t *mac)
{
    return wifi_mgmr_sta_mac_set(mac);
}

aiio_err_t aiio_wifi_sta_mac_get(uint8_t *mac)
{
    return wifi_mgmr_sta_mac_get(mac);
}


aiio_err_t aiio_wifi_ap_mac_set(uint8_t *mac)
{

    return wifi_mgmr_ap_mac_set(mac);
}

aiio_err_t aiio_wifi_ap_mac_get(uint8_t *mac)
{

    return wifi_mgmr_ap_mac_get(mac);
}

aiio_err_t aiio_wifi_sta_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    int32_t state = WIFI_STATE_IDLE;
    wifi_mgmr_state_get(&state);
    if (state == AIIO_WIFI_STATE_CONNECTED_IP_GOT || state == AIIO_WIFI_STATE_WITH_AP_CONNECTED_IP_GOT || state == CODE_WIFI_ON_GOT_IP)
    {
        wifi_mgmr_sta_ip_get(&sta_ip_params.ip, &sta_ip_params.gateway, &sta_ip_params.netmask);
        *ip = sta_ip_params.ip;
        *gw = sta_ip_params.gateway;
        *mask = sta_ip_params.netmask;
    }
    else
    {
        *ip = 0;
        *gw= 0;
        *mask = 0;
    }
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_netif_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    struct netif *netip = wifi_mgmr_sta_netif_get();
    if (ip)
        *ip = netif_ip4_addr(netip)->addr;
    if (gw)
        *gw = netif_ip4_gw(netip)->addr;
    if (mask)
        *mask = netif_ip4_netmask(netip)->addr;
    return AIIO_OK;
}

static int8_t IpStringToByte(char *ip,uint8_t *container){
    if(container==NULL || ip==NULL){
        return AIIO_ERROR;
    }
    int tmp[4]={0};
    if(sscanf(ip,"%d.%d.%d.%d", &tmp[0], &tmp[1], &tmp[2], &tmp[3])==4){
        for (uint8_t i = 0; i < 4; i++){
            container[i]=(uint8_t)tmp[i];
        }
    }else{
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_ip_parameter_set(uint32_t ip, uint32_t mask, uint32_t gw)
{
   return wifi_mgmr_sta_ip_set(ip, mask, gw, gw, 0);
}

aiio_err_t aiio_wifi_sta_ip_set(char* sta_ip)
{
    uint8_t ipArray[4]={0};
    uint8_t netmaskArray[4]={255,255,255,0};
    int8_t i_ret=IpStringToByte(sta_ip ,ipArray);
    if(AIIO_OK!=i_ret){
        aiio_log_e("static IP:%s is error\r\n",sta_ip);
        return AIIO_ERROR;
    }
    uint32_t ip = (uint32_t)ipArray[0] | (uint32_t)ipArray[1] << 8 |
            (uint32_t)ipArray[2] << 16 | (uint32_t)ipArray[3] << 24;

    uint32_t netmask = (uint32_t)netmaskArray[0] | (uint32_t)netmaskArray[1] << 8 |
            (uint32_t)netmaskArray[2] << 16 | (uint32_t)netmaskArray[3] << 24;

    uint32_t gateway = (uint32_t)ipArray[0] | (uint32_t)ipArray[1] << 8 |
            (uint32_t)ipArray[2] << 16 | (uint32_t)1 << 24;

    wifi_mgmr_sta_ip_set(ip, netmask, gateway, gateway, 0);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sta_ip_unset(void)
{
    return wifi_mgmr_sta_ip_unset();
}

aiio_err_t aiio_wifi_ap_ip_get(uint32_t *ip, uint32_t *gw, uint32_t *mask)
{
    return wifi_mgmr_ap_ip_get(ip,gw,mask);
}

aiio_err_t aiio_wifi_ap_ip_set(char* ap_ip)
{

    uint8_t ip[4] = { 0 }, digit= 0;
    char* temp_arg = (char*)calloc(1, 6);
    int32_t ret = aiio_wifi_ap_stop();
    memset(ip, 0, sizeof(ip));
    memset(&ap_ip_params, 0, sizeof(ap_ip_params));
    memset(temp_arg, 0, sizeof(temp_arg));

    for (int32_t i = 0; i < 4; i++)
    {
        temp_arg = ap_ip;
        while (*(ap_ip++) != '.')
        {
            digit++;
        }
        temp_arg[digit] = '\0';
        ip[i] = atoi(temp_arg);
        digit = 0;
    }
    if (!ret)
    {
        ap_ip_params.ip = IP_SET_ADDR(ip[0], ip[1], ip[2], ip[3]);
        ap_ip_params.gateway = IP_SET_ADDR(ip[0], ip[1], ip[2], 1);
        ap_ip_params.netmask = IP_SET_ADDR(255, 255, 255, 0);
    }
    wifi_mgmr_ap_ip_set(ap_ip_params.ip, ap_ip_params.gateway, ap_ip_params.netmask);
    return ret;
}


aiio_err_t aiio_wifi_set_country_code(char *country_code)
{
    strncpy(aiio_conf.country_code, country_code, sizeof(aiio_conf.country_code));
    aiio_conf.country_code[2] = '\0';
    wifi_mgmr_set_country_code(country_code);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_get_country_code(char *country_code)
{
    strcpy(country_code, aiio_conf.country_code);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_channel_get(int32_t *channel)
{
    return wifi_mgmr_channel_get(channel);
}
aiio_err_t aiio_wifi_channel_set(int32_t channel)
{
    return wifi_mgmr_channel_set(channel,0);
}

aiio_err_t aiio_wifi_rssi_get(int32_t *rssi)
{
    return wifi_mgmr_rssi_get(rssi);
}


aiio_err_t aiio_wifi_sta_connect_ind_stat_get(aiio_wifi_sta_connect_ind_stat_info_t *wifi_ind_stat)
{
    wifi_mgmr_sta_connect_ind_stat_get(wifi_ind_stat);
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_enable(void)
{
    wifi_mgmr_sniffer_enable();
    return AIIO_OK;
}

aiio_err_t aiio_wifi_sniffer_disable(void)
{
    wifi_mgmr_sniffer_disable();
    return AIIO_OK;
}
aiio_err_t aiio_wifi_sniffer_register(void *env, aiio_sniffer_cb_t cb)
{
    wifi_mgmr_sniffer_register(env,cb);
    return AIIO_OK;
}
aiio_err_t aiio_wifi_sniffer_unregister(void *env)
{
    wifi_mgmr_sniffer_unregister(env);
    return AIIO_OK;
}


aiio_wifi_beacon_auth_t aiio_wifi_get_sta_security(void)
{
    int32_t state = WIFI_STATE_UNKNOWN;

    wifi_mgmr_state_get(&state);
    if (state == WIFI_STATE_CONNECTED_IP_GETTING || state == WIFI_STATE_CONNECTED_IP_GOT) {
        uint8_t *bssid = wifiMgmr.wifi_mgmr_stat_info.bssid;
        for (int i = 0; i < sizeof(wifiMgmr.scan_items)/sizeof(wifiMgmr.scan_items[0]); i++) {
            if (memcmp(bssid, wifiMgmr.scan_items[i].bssid, 6) == 0) {
                return wifiMgmr.scan_items[i].auth;
            }
        }
    }
    return AIIO_WIFI_EVENT_BEACON_IND_AUTH_UNKNOWN;
}
