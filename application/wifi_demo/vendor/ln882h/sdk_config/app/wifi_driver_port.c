#include "wifi_driver_port.h"
#include "usr_ctrl.h"
#include "utils/system_parameter.h"
#include "utils/sysparam_factory_setting.h"
#include "transparent.h"
#include "ln_at_cmd_base_plus.h"
#include "utils/debug/log.h"

#define SOFTAPCFG_TASK_STACK_SIZE   (3 * 512)

static int ln_softapcfg_start(void);
static void ln_cfg_softap(const char* _ssid, const char* _pwd);
static void ln_wifi_stop(void);
static void ln_reset_wifi_connect_info(void);
static void ln_wifi_init_sta(void);
static void ln_connect_to_ap(void);
static void ln_sta_reset_ip_sem(void);
static void ln_sta_release_ip_sem(void);
static int ln_sta_wait_for_ip(int _ms);
static void wifi_scan_complete_cb(void * arg);
static char* ln_get_sta_ssid_buf(void);
static char* ln_get_sta_pwd_buf(void);
static uint8_t* ln_get_sta_bssid_buf(void);
static int ln_wifi_softap_startup(const char * ssid, const char * pwd, const char * local_ip);
static void ln_ap_startup_cb(void * arg);

static int ln_wifi_netdev_get_state(void);
static void ln_wifi_netdev_set_state(uint8_t dhcp);
static void ln_wifi_get_ip_cb(struct netif *nif);

static void at_smartconfig_start(uint8_t type, uint8_t auth, const char *key);
static void at_smartconfig_stop(void);
static void at_smartconfig_task_entry(void *param);

static smartconfig_config_t sm_cfg = {0};

static uint8_t 		psk_value[40]      = {0x0};
static uint8_t 		mac_addr[6]        = {0x00, 0x50, 0xC2, 0x5E, 0x88, 0x99};
static OS_Semaphore_t s_ipGot;

static wifi_scan_cfg_t s_scanCfg = {
        .channel   = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 20,
};

void    ln_at_printf(const char *format, ...);

static LN_WIFI_DRIVER_ADAPTER s_lnWifiDrvAdapter = {
	
	.sta={
		.dhcp_en = 1,
		.pci_en = 0,
		.scan_mode = 0,
		.pmf = 0,
		.listen_interval = 3,
		.reconn_interval = 1,
		.jap_timeout = 15,
	},
	
	.reConn = true,

	.init = ln_reset_wifi_connect_info,

	.stop = ln_wifi_stop,
	
	.init_sta = ln_wifi_init_sta,
	.sta_connect = ln_connect_to_ap,
	.reset_ip_sem = ln_sta_reset_ip_sem,
	.release_ip_sem = ln_sta_release_ip_sem,
	.wait_for_ip = ln_sta_wait_for_ip,
	
	.start_softap = ln_softapcfg_start,
	.cfg_softap = ln_cfg_softap,
	
	.ssid_buf = ln_get_sta_ssid_buf,
	.pwd_buf = ln_get_sta_pwd_buf,
	.bssid_buf = ln_get_sta_bssid_buf,
	
	.netdev = {
		.get_state = ln_wifi_netdev_get_state,
		.set_state = ln_wifi_netdev_set_state,
	},
};

LN_WIFI_DRIVER_ADAPTER* ln_get_wifi_drv_handle()
{
	return &s_lnWifiDrvAdapter;
}

/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/
//adapt esp8266 static func
static void at_wifi_adapter_init(void);
//sta
static void at_wifi_stop(void);
static void at_wifi_sta_connect(void);
//ap
static void at_wifi_ap_start(void);


at_wifi_adapter_t g_at_wifi_adapter = {

    .conn_cfg = {
        .ssid = "LN_2.4G",
        .pwd = "LN202!@#",
    },

     .conn_ext_cfg.sta_scan_cfg = {
        .channel   = 0,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
        .scan_time = 20,
     },
     
    .at_func.adapter_init_func = at_wifi_adapter_init,
};



//adapt cur
at_wifi_adapter_t* at_wifi_adapter_get_handle(void)
{
	return &g_at_wifi_adapter;
}

static void at_wifi_adapter_init(void)
{
    at_wifi_adapter_t *pAdapt = at_wifi_adapter_get_handle();

    memset(pAdapt, 0, sizeof(at_wifi_adapter_t));

    //init status
    wifi_mode_t cur_mode = 0;
    sysparam_poweron_wifi_mode_get(&cur_mode);
    pAdapt->status_info.cur_mode = cur_mode;

    //init sta reconn cfg
    uint8_t reconn = 0;
    sysparam_poweron_auto_conn_get(&reconn);
    pAdapt->conn_ext_cfg.reconn = reconn;

    //init sta ip
    uint8_t dhcp = 1;
    uint8_t dhcpd = 1;
    tcpip_ip_info_t ip_info = {0};
    //dhcp
    sysparam_dhcp_en_get(&dhcp);
    pAdapt->status_info.dhcp_cur = dhcp;
    //init sta ip
    sysparam_sta_ip_info_get(&ip_info);
    memcpy(&pAdapt->status_info.sta_ip_info_cur, &ip_info, sizeof(tcpip_ip_info_t));
    //init ap ip
    sysparam_dhcpd_en_get(&dhcpd);
    sysparam_softap_ip_info_get(&ip_info);
    memcpy(&pAdapt->status_info.ap_ip_info_cur, &ip_info, sizeof(tcpip_ip_info_t));

    //init sta conn cfg
    pAdapt->conn_cfg.ssid = pAdapt->conn_ext_cfg.ssid;
    pAdapt->conn_cfg.pwd = pAdapt->conn_ext_cfg.pwd;
    pAdapt->conn_cfg.bssid = pAdapt->conn_ext_cfg.bssid;
    sysparam_sta_conn_cfg_get(&pAdapt->conn_cfg);

    //init sta scan cfg
    sysparam_sta_scan_cfg_get(&pAdapt->conn_ext_cfg.sta_scan_cfg);
    pAdapt->conn_ext_cfg.sta_scan_cfg.scan_time = 20;   //default
    
    //init sta join ap ext cfg
    wifi_sta_join_ap_cfg_t jap_cfg = {0};
    sysparam_sta_jap_cfg_get(&jap_cfg);
    memcpy(&pAdapt->conn_ext_cfg.join_ap_cfg, &jap_cfg, sizeof(wifi_sta_join_ap_cfg_t));
    
    //init ap cfg
    char ssid[64]= {0};
    char pwd[64] = {0};
    sysparam_softap_ssidpwd_cfg_get(ssid, pwd);
    memcpy(pAdapt->ap_cfg.ssid, ssid, strlen(ssid));
    memcpy(pAdapt->ap_cfg.pwd, pwd, strlen(pwd));
    
    //at sta mode func regist
    pAdapt->at_func.adapter_init_func = at_wifi_adapter_init;
    pAdapt->at_func.wifi_stop_func = at_wifi_stop;
    pAdapt->at_func.wifi_sta_init_func = ln_wifi_init_sta;
    pAdapt->at_func.wifi_sta_connect_func = at_wifi_sta_connect;
    //smartconfig func regist
    pAdapt->at_func.smartconfig_func.wifi_sta_smartconfig_start_func = at_smartconfig_start;
    pAdapt->at_func.smartconfig_func.wifi_sta_smartconfig_stop_func = at_smartconfig_stop;
    pAdapt->at_func.smartconfig_func.smartconfig_task_entry = at_smartconfig_task_entry;
    //at ap mode func regist
    pAdapt->at_func.wifi_ap_start_func = at_wifi_ap_start;
}

static void at_wifi_stop(void)
{
    OS_MsDelay(100);
    wifi_stop();
}

static void at_wifi_sta_connect(void)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();
    //union Semphore
    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
    
    wifi_sta_connect_t staCfg = {0};
    wifi_scan_cfg_t scan_cfg = {0};
    memcpy(&scan_cfg, &(pada->conn_ext_cfg.sta_scan_cfg), sizeof(wifi_scan_cfg_t));

    //init sta
    pada->at_func.wifi_sta_init_func();
    //need to rebuild
    pWifi->reset_ip_sem();

    staCfg.ssid = pada->conn_ext_cfg.ssid;
    staCfg.pwd = pada->conn_ext_cfg.pwd;
    if (pada->conn_ext_cfg.bssid[0] == 0 &&
        pada->conn_ext_cfg.bssid[1] == 0 &&
        pada->conn_ext_cfg.bssid[2] == 0 &&
        pada->conn_ext_cfg.bssid[3] == 0 &&
        pada->conn_ext_cfg.bssid[4] == 0 &&
        pada->conn_ext_cfg.bssid[5] == 0) 
    {
    	staCfg.bssid = NULL;
    }
    else
    {
    	staCfg.bssid = pada->conn_ext_cfg.bssid;
    }

    Log_d("ssid: %s", staCfg.ssid);
    Log_d("pwd: %s", staCfg.pwd);

    staCfg.psk_value = NULL;
    if (strlen(staCfg.pwd) != 0) {
        if (0 == ln_psk_calc(staCfg.ssid, staCfg.pwd, psk_value, sizeof (psk_value))) {
            staCfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }

    if ( WIFI_ERR_NONE !=  wifi_sta_connect(&staCfg, &scan_cfg))
    {
    	Log_e("Join failed! ssid:%s, pwd:%s", staCfg.ssid, staCfg.pwd);
    }
}


//softap
static void at_wifi_ap_start(void)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();
    tcpip_ip_info_t ip_info = {0};
    char ssid[64] = {0};
    char pwd[64] = {0};
    uint8_t macaddr[6] = {0};
    
    //get mac addr from Flash OTP
    if(ln_get_read_param_from_fotp_flag() == 0x01){
        ln_fotp_get_mac_val(macaddr);
        sysparam_sta_mac_update((const uint8_t *)macaddr);	
    }
    else
    {
        ln_generate_random_mac(macaddr);
    }
    
    memcpy(ssid, pada->ap_cfg.ssid, strlen(pada->ap_cfg.ssid));
    memcpy(pwd, pada->ap_cfg.pwd, strlen(pada->ap_cfg.pwd));
    memcpy(&ip_info, &pada->status_info.ap_ip_info_cur, sizeof(tcpip_ip_info_t));
    //2. ap cfg
    wifi_softap_cfg_t ap_cfg = {
        .ssid            = ssid,
        .pwd             = pwd,
        .bssid           = macaddr,
        .ext_cfg = {
            .channel         = 7,
            .authmode        = WIFI_AUTH_WPA_WPA2_PSK,//WIFI_AUTH_OPEN,
            .ssid_hidden     = 0,
            .beacon_interval = 100,
            .psk_value = NULL,
        }
    };

    if (strlen(pwd) == 0) {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
    }
    else
    {
        uint16_t pwdlen = strlen(pwd);
        if (pwdlen < 8) {
            Log_e("wifi softap pwd len < 8!");
        }
        else if (pwdlen >= WLAN_PWD_MAX_LEN_PORT) {
            Log_e("wifi softap pwd len >= %d!", WLAN_PWD_MAX_LEN_PORT);
        }
        else
        {
            ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA2_PSK;
        }
    }
    //4. dhcp server
    server_config_t  server_config;
    server_config.server.addr   = ip_info.ip.addr;
    server_config.port          = 67;
    server_config.lease         = 2880;
    server_config.renew         = 2880;
    server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.100");
    server_config.ip_end.addr   = ipaddr_addr((const char *)"192.168.4.150");
    server_config.client_max    = 3;
    dhcpd_curr_config_set(&server_config);
  
    //1. net device(lwip).
	uint8_t psk_value[40]      = {0x0};
    netdev_set_mac_addr(NETIF_IDX_AP, macaddr);
    netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
    netdev_set_active(NETIF_IDX_AP);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &ln_ap_startup_cb);

    sysparam_softap_mac_update((const uint8_t *)macaddr);

    ap_cfg.ext_cfg.psk_value = NULL;
    if ((strlen(ap_cfg.pwd) != 0) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_OPEN) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_WEP)) {
        memset(psk_value, 0, sizeof(psk_value));
        if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof (psk_value))) {
            ap_cfg.ext_cfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }	



    Log_i("[APCFG] ssid:[%s],pwd:[%s],local ip:[%s]",ssid, pwd, inet_ntoa(ip_info.ip));

    if (LN_TRUE != ln_wifi_softap_startup(ssid, pwd, inet_ntoa(ip_info.ip))) {
        Log_e("softap start fail..\r\n");
    }

    OS_MsDelay(500);//wait for ap start

}

static void at_smartconfig_task_entry(void *param)
{
    smartconfig_config_t *cfg = (smartconfig_config_t *)param;
    uint16_t channel_map = 0;
    uint8_t * ssid = NULL;
    uint8_t * pwd = NULL;
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();
    wifi_sta_connect_ext_t sys_jap_conn_cfg = {0};
    wifi_sta_connect_t sys_sta_conn_cfg = {0};
    Log_i("cfg type = %d , auth = %d , key = %s\r\n",cfg->type, cfg->auth, cfg->key);
    //keep recent sys data
    memcpy(&sys_jap_conn_cfg, &pada->conn_ext_cfg, sizeof(wifi_sta_connect_ext_t));
    memcpy(&sys_sta_conn_cfg, &pada->conn_cfg, sizeof(wifi_sta_connect_t));
        
    //init sta
    pada->at_func.wifi_sta_init_func();
    //init channel map
    channel_map = CHANNEL1_MASK | CHANNEL2_MASK | CHANNEL3_MASK | CHANNEL4_MASK | \
                  CHANNEL5_MASK | CHANNEL6_MASK | CHANNEL7_MASK | CHANNEL8_MASK | \
                  CHANNEL9_MASK | CHANNEL10_MASK | CHANNEL11_MASK | CHANNEL12_MASK | CHANNEL13_MASK;
    
    //set smartconfig is running flag
    pada->at_func.smartconfig_func.is_running = true;

    //start smart config
    if (LN_TRUE != ln_smartconfig_start(channel_map)) {
        LOG(LOG_LVL_ERROR, "failed to start smartconfig...\r\n");
    }
    while (LN_TRUE != ln_smartconfig_is_complete()) {
        if(pada->at_func.smartconfig_func.stop_flag == true)
        {
            goto __stop;
        }
        OS_MsDelay(10);
    }
    ln_smartconfig_stop();
    OS_MsDelay(2000);

    ssid = ln_smartconfig_get_ssid();
    pwd  = ln_smartconfig_get_pwd();

    memset(pada->conn_ext_cfg.ssid, 0, SSID_MAX_LEN);
    memset(pada->conn_ext_cfg.pwd, 0, PASSWORD_MAX_LEN);
    memcpy(pada->conn_ext_cfg.ssid, ssid, strlen((char *)ssid));
    memcpy(pada->conn_ext_cfg.pwd, pwd, strlen((char *)pwd));

    //get ssid & pwd, connect to ap
    pada->at_func.wifi_sta_connect_func();

    //store ssi & pwd info in flash
    pada->conn_cfg.ssid = pada->conn_ext_cfg.ssid;
    pada->conn_cfg.pwd = pada->conn_cfg.pwd;
    sysparam_sta_conn_cfg_update(&pada->conn_cfg);

    while(!netdev_got_ip()){
        OS_MsDelay(1000);
    }
    
    ln_smartconfig_send_ack();

    pada->at_func.smartconfig_func.is_running = false;
    //delete sem
    OS_SemaphoreDelete(&pada->at_func.smartconfig_func.smartconfig_stop_sem);
    pada->at_func.smartconfig_func.smartconfig_stop_sem.handle = NULL;
    //delete task thread
    OS_ThreadDelete(&pada->at_func.smartconfig_func.smartconfig_thread);
    pada->at_func.smartconfig_func.smartconfig_thread.handle = NULL;
    
__stop:
    pada->at_func.smartconfig_func.is_running = false;
    pada->at_func.smartconfig_func.stop_flag = false;
    ln_smartconfig_stop();
    OS_MsDelay(1000);
    pada->at_func.wifi_sta_connect_func();
    //delete sem
    OS_SemaphoreDelete(&pada->at_func.smartconfig_func.smartconfig_stop_sem);
    pada->at_func.smartconfig_func.smartconfig_stop_sem.handle = NULL;
    //delete task thread
    OS_ThreadDelete(&pada->at_func.smartconfig_func.smartconfig_thread);
    pada->at_func.smartconfig_func.smartconfig_thread.handle = NULL;
}

static void at_smartconfig_start(uint8_t type, uint8_t auth, const char *key)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();

    sm_cfg.type = type;
    sm_cfg.auth = auth;
    memcpy(sm_cfg.key, key, strlen(key));
    
    //thread init
    pada->at_func.smartconfig_func.smartconfig_thread.handle = NULL;
    //semaphore init
    if(OS_OK != OS_SemaphoreCreateBinary(&pada->at_func.smartconfig_func.smartconfig_stop_sem))
    {
        Log_e("smartconfig stop sem create fail..\r\n");
        pada->at_func.smartconfig_func.smartconfig_stop_sem.handle = NULL;
    }
    //reset stop sem
    OS_SemaphoreWait(&pada->at_func.smartconfig_func.smartconfig_stop_sem, 0);
    
    //stop flag
    pada->at_func.smartconfig_func.stop_flag = false;

    //create smartconfig task
    if (OS_OK != OS_ThreadCreate(&pada->at_func.smartconfig_func.smartconfig_thread, "smartconfig", \
        pada->at_func.smartconfig_func.smartconfig_task_entry, (void *)&sm_cfg, OS_PRIORITY_NORMAL, SMARTCONFIG_TASK_STACK_SIZE))
    {
        Log_e("OS_ThreadCreate error!");
        OS_ThreadDelete(&pada->at_func.smartconfig_func.smartconfig_thread);
        pada->at_func.smartconfig_func.smartconfig_thread.handle = NULL;

        OS_SemaphoreDelete(&pada->at_func.smartconfig_func.smartconfig_stop_sem);
        pada->at_func.smartconfig_func.smartconfig_stop_sem.handle = NULL;
    }

}

static void at_smartconfig_stop(void)
{
    at_wifi_adapter_t *pada = at_wifi_adapter_get_handle();
    if(pada->at_func.smartconfig_func.smartconfig_stop_sem.handle == NULL)
    {
        Log_e("smart config sem is null!\r\n");
    }
    pada->at_func.smartconfig_func.stop_flag = true;
}
/***********************************************************************************************/
/***********************************************************************************************/
/***********************************************************************************************/

static void ln_reset_wifi_connect_info()
{
	LN_WIFI_DRIVER_ADAPTER *pWifi = ln_get_wifi_drv_handle();
	
	memset(&pWifi->sta, 0, sizeof(LN_WIFI_STA_CFG));
	memset(&pWifi->sap, 0, sizeof(LN_WIFI_STA_CFG));
	
	pWifi->conn.ssid = pWifi->sta.ssid;
	pWifi->conn.pwd = pWifi->sta.pwd;
	pWifi->conn.bssid = pWifi->sta.bssid;
	
	//default
	uint8_t dhcp = 0;
	if(0 != sysparam_dhcp_en_get(&dhcp))
	{
		Log_e("geti dhcp error.\r\n");
		dhcp = 1; //defualt
	}

	pWifi->sta.sta_ip.isSet = false;
	pWifi->sta.dhcp_en = dhcp;
	pWifi->sta.jap_timeout = 15;
	pWifi->sta.listen_interval = 3;
	pWifi->sta.reconn_interval = 1;
	pWifi->sta.scan_mode = 0;
	pWifi->sta.pmf = 0;
	pWifi->sta.pci_en = 0;
	
	if (OS_SemaphoreCreateBinary(&s_ipGot) != OS_OK)
    {
        Log_e("OS_SemaphoreCreate sem fail.");
        while(1);
    }
}

static int ln_wifi_netdev_get_state(void)
{
	if(NETDEV_LINK_UP == netdev_get_link_state(netdev_get_active()))
		return 0;
	else
		return -1;
}

static void ln_wifi_netdev_set_state(uint8_t dhcp)
{
	if(dhcp)
	{
		netdev_get_ip_cb_set(ln_wifi_get_ip_cb);
	}
	else
	{
		netdev_get_ip_cb_set(NULL);
	}
	netdev_set_state(NETIF_IDX_STA, NETDEV_UP);			//re got ip
}

static char* ln_get_sta_ssid_buf()
{
	return ln_get_wifi_drv_handle()->sta.ssid;
}

static char* ln_get_sta_pwd_buf()
{
	return ln_get_wifi_drv_handle()->sta.pwd;
}

static uint8_t* ln_get_sta_bssid_buf()
{
	return ln_get_wifi_drv_handle()->sta.bssid;
}

static void ln_wifi_connected_cb(void * arg)
{
	uint8_t dhcp = 0;
	LN_WIFI_DRIVER_ADAPTER *p_adapt = ln_get_wifi_drv_handle();
	
	sysparam_dhcp_en_get(&dhcp);

	Log_i("connect ap success!!");
  ln_at_printf("WIFI CONNECTED\r\n");
	if(0 == dhcp)
	{
		ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
		Log_i("IP get success!!");
		p->cStaStat = STA_GOT_IP;
		ln_at_printf("WIFI GOT IP\r\n");
		ln_get_wifi_drv_handle()->release_ip_sem();
	}
}

static void ln_wifi_disconnected_cb(void * arg)
{
	Log_i("connect ap success!!");
  ln_at_printf("WIFI DISCONNECT\r\n");
}

static void ln_wifi_get_ip_cb(struct netif *nif)
{
	ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

	Log_i("IP get success!!");
	p->cStaStat = STA_GOT_IP;
	ln_at_printf("WIFI GOT IP\r\n");
	ln_get_wifi_drv_handle()->release_ip_sem();		
}

static void ln_wifi_init_sta(void)
{
	sta_ps_mode_t ps_mode = PM_WIFI_DEFAULT_PS_MODE;
	uint8_t dhcp_en = 0;
	
    if(0 != sysparam_dhcp_en_get(&dhcp_en))
	{
		Log_e("dhcp get error.\r\n");
	}

    //1. sta mac get
    //get mac addr from Flash OTP
    if(ln_get_read_param_from_fotp_flag() == 0x01){
        ln_fotp_get_mac_val(mac_addr);
        sysparam_softap_mac_update((const uint8_t *)mac_addr);
    }else{
        if (SYSPARAM_ERR_NONE != sysparam_sta_mac_get(mac_addr)) {
            Log_e("sta mac get filed!!!");
            return;
        }
        if (mac_addr[0] == STA_MAC_ADDR0 &&
            mac_addr[1] == STA_MAC_ADDR1 &&
            mac_addr[2] == STA_MAC_ADDR2 &&
            mac_addr[3] == STA_MAC_ADDR3 &&
            mac_addr[4] == STA_MAC_ADDR4 &&
            mac_addr[5] == STA_MAC_ADDR5) {
            ln_generate_random_mac(mac_addr);
        }
    }
	sysparam_sta_mac_update((const uint8_t *)mac_addr);	
    
	//2. stop wifi
    wifi_sta_status_t staStat;
    wifi_get_sta_status(&staStat);
    Log_i("sta: %d", staStat);
    if (staStat != WIFI_STA_STATUS_SCANING && staStat != WIFI_STA_STATUS_DISCONNECTED)
    {
        wifi_sta_disconnect();
    }

    wifi_stop();
    netdev_set_state(NETIF_IDX_STA, NETDEV_DOWN);
		netdev_get_ip_cb_set(NULL);

    //3. net device(lwip)
    netdev_set_mac_addr(NETIF_IDX_STA, mac_addr);
    netdev_set_active(NETIF_IDX_STA);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_CONNECTED, &ln_wifi_connected_cb);
	wifi_manager_reg_event_callback(WIFI_MGR_EVENT_STA_DISCONNECTED, &ln_wifi_disconnected_cb);
	if(!dhcp_en)
	{
		tcpip_ip_info_t ip_info;
		if (0 != sysparam_sta_ip_info_get(&ip_info))
		{
			Log_e("kv get ip error.\r\n");
		}
		netdev_set_ip_info(NETIF_IDX_STA, &ip_info);
	}
	else
	{
		netdev_get_ip_cb_set(ln_wifi_get_ip_cb);
	}

    //4. wifi start
	if (SYSPARAM_ERR_NONE != sysparam_sta_powersave_get(&ps_mode))
    {
        Log_e("sysparam_sta_powersave_get filed!!!");
    }

    if(WIFI_ERR_NONE != wifi_sta_start(mac_addr, ps_mode)){
        Log_e("wifi sta start filed!!!");
    }
}

static void ln_set_scan_cfg(uint8_t *_cfg)
{
	//cfg 0:show_hidden, 1:scan_type(0:active,1:passive,2:mixed), 2:scan_time(s) 
	s_scanCfg.scan_type = (wifi_scan_type_t)_cfg[1];
	s_scanCfg.scan_time = _cfg[2] * 1000 / 13;
}

static void ln_connect_to_ap()
{
	LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
	wifi_sta_connect_t staCfg = {0};
	//uint8_t bssid_hex[BSSID_LEN];
	
	ln_wifi_init_sta();
	pWifi->reset_ip_sem();
	
	staCfg.ssid = pWifi->sta.ssid;
	staCfg.pwd = pWifi->sta.pwd;
	if (pWifi->sta.bssid[0] == 0 &&
        pWifi->sta.bssid[1] == 0 &&
        pWifi->sta.bssid[2] == 0 &&
        pWifi->sta.bssid[3] == 0 &&
        pWifi->sta.bssid[4] == 0 &&
        pWifi->sta.bssid[5] == 0) 
	{
		staCfg.bssid = NULL;
    }
	else
	{
		staCfg.bssid = pWifi->sta.bssid;
	}
	
	Log_d("ssid: %s", staCfg.ssid);
	Log_d("pwd: %s", staCfg.pwd);
	
	if (SYSPARAM_ERR_NONE != sysparam_sta_scan_cfg_get(&s_scanCfg))
    {
        Log_e("sysparam_sta_scan_cfg_get filed!!!");
    }
	
	staCfg.psk_value = NULL;
    if (strlen(staCfg.pwd) != 0) {
        if (0 == ln_psk_calc(staCfg.ssid, staCfg.pwd, psk_value, sizeof (psk_value))) {
            staCfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }
	
	if ( WIFI_ERR_NONE !=  wifi_sta_connect(&staCfg, &s_scanCfg))
	{
		Log_e("Join failed! ssid:%s, pwd:%s", staCfg.ssid, staCfg.pwd);
	}
}

static void ln_sta_release_ip_sem(void)
{
	OS_SemaphoreRelease(&s_ipGot);
}

static int ln_sta_wait_for_ip(int _ms)
{
	if (OS_OK == OS_SemaphoreWait(&s_ipGot, _ms))
    {
		return LN_OK;
	}
	
	return LN_FAIL;
}

static void ln_sta_reset_ip_sem(void)
{
	ln_sta_wait_for_ip(0);
}

static void wifi_scan_complete_cb(void * arg)
{
    LN_UNUSED(arg);

    ln_list_t *list;
    uint8_t node_count = 0;
    ap_info_node_t *pnode;

    wifi_manager_ap_list_update_enable(LN_FALSE);

    // 1.get ap info list.
    wifi_manager_get_ap_list(&list, &node_count);

    // 2.print all ap info in the list.
    LN_LIST_FOR_EACH_ENTRY(pnode, ap_info_node_t, list,list)
    {
        uint8_t * mac = (uint8_t*)pnode->info.bssid;
        ap_info_t *ap_info = &pnode->info;

        LOG(LOG_LVL_INFO, "\tCH=%2d,RSSI= %3d,", ap_info->channel, ap_info->rssi);
        LOG(LOG_LVL_INFO, "BSSID:[%02X:%02X:%02X:%02X:%02X:%02X],SSID:\"%s\"\r\n", \
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ap_info->ssid);
    }

    wifi_manager_ap_list_update_enable(LN_TRUE);
}

static void ln_ap_startup_cb(void * arg)
{
    netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
}

static int ln_wifi_softap_startup(const char * ssid, const char * pwd, const char * local_ip)
{
	//1. macaddr
	char apssid[64 + 1] = {0};
	char appwd[64 + 1] = {0};
    uint8_t macaddr[6] = {0};
	ln_generate_random_mac(macaddr);
	
	//2. ap cfg
	wifi_softap_cfg_t ap_cfg = {
		.ssid            = apssid,
		.pwd             = appwd,
		.bssid           = macaddr,
		.ext_cfg = {
			.channel         = 7,
			.authmode        = WIFI_AUTH_WPA_WPA2_PSK,//WIFI_AUTH_OPEN,
			.ssid_hidden     = 0,
			.beacon_interval = 100,
			.psk_value = NULL,
		}
	};

    if ((ssid == NULL) || ((ssid != NULL) && (strlen(ssid) > WLAN_SSID_MAX_LEN_PORT))) {
        Log_e("wifi softap ssid invalid!");
        return LN_FALSE;
    }
    strcpy((char *)ap_cfg.ssid, ssid);

    if (pwd == NULL) {
        ap_cfg.ext_cfg.authmode = WIFI_AUTH_OPEN;
    }
    else {
        uint16_t pwdlen = strlen(pwd);
        if (pwdlen < 8) {
            Log_e("wifi softap pwd len < 8!");
            return LN_FALSE;
        }
        else if (pwdlen >= WLAN_PWD_MAX_LEN_PORT) {
            Log_e("wifi softap pwd len >= %d!", WLAN_PWD_MAX_LEN_PORT);
            return LN_FALSE;
        }
        else{
            strcpy((char *)ap_cfg.pwd, pwd);
            ap_cfg.ext_cfg.authmode = WIFI_AUTH_WPA2_PSK;
        }
    }

	//3. tcpip info
    tcpip_ip_info_t ip_info;
    ip_info.ip.addr = ipaddr_addr(local_ip);
	ip_info.gw.addr = ipaddr_addr(local_ip);
	ip_info.netmask.addr = ipaddr_addr((const char *)"255.255.255.0");

    //4. dhcp server
    server_config_t  server_config;
    server_config.server.addr   = ip_info.ip.addr;
    server_config.port          = 67;
    server_config.lease         = 2880;
    server_config.renew         = 2880;
    server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.100");
    server_config.ip_end.addr   = ipaddr_addr((const char *)"192.168.4.150");
    server_config.client_max    = 3;
    dhcpd_curr_config_set(&server_config);
  
    //1. net device(lwip).
	uint8_t psk_value[40]      = {0x0};
    netdev_set_mac_addr(NETIF_IDX_AP, macaddr);
    netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
    netdev_set_active(NETIF_IDX_AP);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &ln_ap_startup_cb);

    sysparam_softap_mac_update((const uint8_t *)macaddr);

    ap_cfg.ext_cfg.psk_value = NULL;
    if ((strlen(ap_cfg.pwd) != 0) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_OPEN) &&
        (ap_cfg.ext_cfg.authmode != WIFI_AUTH_WEP)) {
        memset(psk_value, 0, sizeof(psk_value));
        if (0 == ln_psk_calc(ap_cfg.ssid, ap_cfg.pwd, psk_value, sizeof (psk_value))) {
            ap_cfg.ext_cfg.psk_value = psk_value;
            hexdump(LOG_LVL_INFO, "psk value ", psk_value, sizeof(psk_value));
        }
    }	
	
	//2. wifi
    if(WIFI_ERR_NONE !=  wifi_softap_start(&ap_cfg)){
        Log_e("wifi softap start failed!");
		return LN_FALSE;
    }

    return LN_TRUE;
}

static int ln_softapcfg_start(void)
{
	LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
	tcpip_ip_info_t ip_info;
	uint8_t sysstore = 0;
	char ssid[64] = {0};
	char pwd[64] = {0};
	
	sysparam_sys_store_get(&sysstore);
	if(sysstore)
	{
		if(0 != sysparam_softap_ip_info_get(&ip_info))
		{
			Log_e("get ap ip fail.\r\n");
			return LN_FALSE;
		}
	
		if(0 != sysparam_softap_ssidpwd_cfg_get(ssid, pwd))
		{
			Log_e("get ssid pwd fail.\r\n");
			return LN_FALSE;		
		}
	}
	
    Log_i("[APCFG] ssid:[%s],pwd:[%s],local ip:[%s]",ssid, pwd, inet_ntoa(ip_info.ip));

    if (LN_TRUE != ln_wifi_softap_startup(ssid, pwd, inet_ntoa(ip_info.ip))) {
        return LN_FALSE;
    }
	
	OS_MsDelay(500);//wait for ap start

    return LN_TRUE;
}

static void ln_cfg_softap(const char* _ssid, const char* _pwd)
{
	strcpy(ln_get_wifi_drv_handle()->sap.ssid, _ssid);
    strcpy(ln_get_wifi_drv_handle()->sap.pwd, _pwd);
}

static void ln_wifi_stop(void)
{
    OS_MsDelay(100);
    wifi_stop();
}
