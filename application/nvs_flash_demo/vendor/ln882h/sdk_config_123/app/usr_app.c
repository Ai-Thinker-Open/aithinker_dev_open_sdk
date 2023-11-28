#include "osal/osal.h"
#include "utils/debug/log.h"
#include "utils/debug/ln_assert.h"
#include "utils/system_parameter.h"
#include "utils/ln_psk_calc.h"
#include "utils/power_mgmt/ln_pm.h"
#include "utils/sysparam_factory_setting.h"
#include "wifi.h"
#include "wifi_port.h"
#include "netif/ethernetif.h"
#include "wifi_manager.h"
#include "lwip/tcpip.h"
#include "drv_adc_measure.h"
#include "hal/hal_adc.h"
#include "ln_nvds.h"
#include "ln_wifi_err.h"
#include "ln_misc.h"
#include "ln882h.h"
#include "usr_app.h"

#include "rwip_config.h"
#include "llm_int.h"
#include "ln_app_gatt.h"
#include "ln_app_gap.h"
#include "gapm_task.h"
#include "ln_app_gap.h"
#include "ln_app_gatt.h"
#include "ln_app_callback.h"
#include "usr_ble_app.h"
#include "usr_send_data.h"

#include "usr_ctrl.h"
#include "wifi_driver_port.h"

#define DEVICE_NAME                  ("LN_data_trans+-*/")
#define DEVICE_NAME_LEN              (sizeof(DEVICE_NAME))
#define ADV_DATA_MAX_LENGTH          (28)

extern uint8_t svc_uuid[16];
extern uint8_t con_num;
static OS_Thread_t ble_g_usr_app_thread;
#define BLE_USR_APP_TASK_STACK_SIZE  (1024)

uint8_t adv_actv_idx  =0;
uint8_t init_actv_idx =0;

static OS_Thread_t g_usr_app_thread;
#define USR_APP_TASK_STACK_SIZE   6*256 //Byte

#define WIFI_TEMP_CALIBRATE             1//1

#if WIFI_TEMP_CALIBRATE
static OS_Thread_t g_temp_cal_thread;
#define TEMP_APP_TASK_STACK_SIZE   4*256 //Byte
#endif

/* declaration */
static void wifi_init_ap(void);
static void usr_app_task_entry(void *params);
static void temp_cal_app_task_entry(void *params);
void ln_set_wifi_mode_from_at(wifi_mode_t _mode);

static uint8_t mac_addr[6]        = {0x00, 0x50, 0xC2, 0x5E, 0xAA, 0xDA};
static uint8_t psk_value[40]      = {0x0};
// static uint8_t target_ap_bssid[6] = {0xC0, 0xA5, 0xDD, 0x84, 0x6F, 0xA8};

wifi_softap_cfg_t ap_cfg = {
    .ssid            = "LN_AP_8899",
    .pwd             = "12345678",
    .bssid           = mac_addr,
    .ext_cfg = {
        .channel         = 6,
        .authmode        = WIFI_AUTH_WPA_WPA2_PSK,//WIFI_AUTH_OPEN,
        .ssid_hidden     = 0,
        .beacon_interval = 100,
        .psk_value = NULL,
    }
};

static void ap_startup_cb(void * arg)
{
    netdev_set_state(NETIF_IDX_AP, NETDEV_UP);
}

static void wifi_init_ap(void)
{
    tcpip_ip_info_t  ip_info;
    server_config_t  server_config;

    ip_info.ip.addr      = ipaddr_addr((const char *)"192.168.4.1");
    ip_info.gw.addr      = ipaddr_addr((const char *)"192.168.4.1");
    ip_info.netmask.addr = ipaddr_addr((const char *)"255.255.255.0");

    server_config.server.addr   = ip_info.ip.addr;
    server_config.port          = 67;
    server_config.lease         = 2880;
    server_config.renew         = 2880;
    server_config.ip_start.addr = ipaddr_addr((const char *)"192.168.4.100");
    server_config.ip_end.addr   = ipaddr_addr((const char *)"192.168.4.150");
    server_config.client_max    = 3;
    dhcpd_curr_config_set(&server_config);

    //1. net device(lwip).
    netdev_set_mac_addr(NETIF_IDX_AP, mac_addr);
    netdev_set_ip_info(NETIF_IDX_AP, &ip_info);
    netdev_set_active(NETIF_IDX_AP);
    wifi_manager_reg_event_callback(WIFI_MGR_EVENT_SOFTAP_STARTUP, &ap_startup_cb);

    sysparam_softap_mac_update((const uint8_t *)mac_addr);

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
        LOG(LOG_LVL_ERROR, "[%s, %d]wifi_start() fail.\r\n", __func__, __LINE__);
    }
}

static void usr_app_task_entry(void *params)
{
    LN_UNUSED(params);

    wifi_mode_t mode = WIFI_MODE_AP;
    uint8_t autoConn = 1;

    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
    wifi_sta_connect_t *conn = &pWifi->conn;

    ln_pm_sleep_mode_set(ACTIVE);
    wifi_manager_init();

    if (SYSPARAM_ERR_NONE != sysparam_poweron_wifi_mode_get(&mode))
    {
        //default
        Log_e("sysparam_poweron_wifi_mode_get fail!!!");
    }
    ln_set_wifi_mode_from_at(mode);

    if (mode == WIFI_MODE_AP)
    {
        wifi_init_ap();
    }
    else if (mode == WIFI_MODE_STATION)
    {
        if (SYSPARAM_ERR_NONE != sysparam_poweron_auto_conn_get(&autoConn))
        {
            Log_e("sysparam_poweron_auto_conn_get fail!!!");
        }

        if (autoConn)
        {
            if (SYSPARAM_ERR_NONE != sysparam_sta_conn_cfg_get(conn))
            {
                Log_e("sysparam_sta_conn_cfg_get fail!!!");
            }

            pWifi->sta_connect();
        }
    }
    else if (mode == WIFI_MODE_OFF)
    {
        pWifi->stop();
    }
    else
    {
        Log_e("unsupport mode: %d", mode);
    }

    while(1)
    {
        //Log_i("wifi mode: %d", wifi_current_mode_get());
        OS_MsDelay(1000);
    }
}

static void temp_cal_app_task_entry(void *params)
{
    LN_UNUSED(params);

    int8_t cap_comp = 0;
    uint16_t adc_val = 0;
    int16_t curr_adc = 0;
    uint8_t cnt = 0;

    if (NVDS_ERR_OK == ln_nvds_get_xtal_comp_val((uint8_t *)&cap_comp)) {
        if ((uint8_t)cap_comp == 0xFF) {
            cap_comp = 0;
        }
    }

    drv_adc_init();

    wifi_temp_cal_init(drv_adc_read(ADC_CH0), cap_comp);

    while (1)
    {
        OS_MsDelay(1000);

        adc_val = drv_adc_read(ADC_CH0);
        wifi_do_temp_cal_period(adc_val);

        curr_adc = (adc_val & 0xFFF);

        cnt++;
        if ((cnt % 60) == 0) {
            LOG(LOG_LVL_INFO, "adc raw: %4d, temp_IC: %4d\r\n",
                    curr_adc, (int16_t)(25 + (curr_adc - 770) / 2.54f));
            LOG(LOG_LVL_INFO, "Total:%d; Free:%ld;\r\n",
                    OS_HeapSizeGet(), OS_GetFreeHeapSize());
        }
    }
}

static void app_create_advertising(void)
{
#define APP_ADV_CHMAP                (0x07)  // Advertising channel map - 37, 38, 39
#define APP_ADV_INT_MIN              (640)   // Advertising minimum interval - 40ms (64*0.625ms)
#define APP_ADV_INT_MAX              (640)   // Advertising maximum interval - 40ms (64*0.625ms)

    struct ln_gapm_activity_create_adv_cmd  adv_creat_param = {0};

    adv_creat_param.own_addr_type                     = GAPM_STATIC_ADDR;
    adv_creat_param.adv_param.type                    = GAPM_ADV_TYPE_LEGACY;//GAPM_ADV_TYPE_EXTENDED;//GAPM_ADV_TYPE_LEGACY;
    adv_creat_param.adv_param.filter_pol              = ADV_ALLOW_SCAN_ANY_CON_ANY;
    adv_creat_param.adv_param.prim_cfg.chnl_map       = APP_ADV_CHMAP;
    adv_creat_param.adv_param.prim_cfg.phy            = GAP_PHY_1MBPS;
    adv_creat_param.adv_param.prop                    = GAPM_ADV_PROP_UNDIR_CONN_MASK;//GAPM_ADV_PROP_NON_CONN_SCAN_MASK;//GAPM_ADV_PROP_UNDIR_CONN_MASK;//GAPM_ADV_PROP_UNDIR_CONN_MASK;//GAPM_EXT_ADV_PROP_UNDIR_CONN_MASK;//GAPM_ADV_PROP_UNDIR_CONN_MASK;
    adv_creat_param.adv_param.disc_mode               = GAPM_ADV_MODE_GEN_DISC;
    adv_creat_param.adv_param.prim_cfg.adv_intv_min   = APP_ADV_INT_MIN;
    adv_creat_param.adv_param.prim_cfg.adv_intv_max   = APP_ADV_INT_MAX;
    adv_creat_param.adv_param.max_tx_pwr              = 0;
    //adv_creat_param.adv_param.second_cfg.phy        = GAP_PHY_1MBPS;//GAP_PHY_1MBPS;//GAP_PHY_CODED;
    adv_creat_param.adv_param.second_cfg.max_skip     = 0x00;
    adv_creat_param.adv_param.second_cfg.phy          = 0x01;
    adv_creat_param.adv_param.second_cfg.adv_sid      = 0x00;
    adv_creat_param.adv_param.period_cfg.adv_intv_min = 0x0400;
    adv_creat_param.adv_param.period_cfg.adv_intv_max = 0x0400;
    ln_app_advertise_creat(&adv_creat_param);
}

static void app_set_adv_data(void)
{
    //adv data: adv length--adv type--adv string ASCII
    uint8_t adv_data[ADV_DATA_MAX_LENGTH] = {0};
    adv_data[0] = DEVICE_NAME_LEN + 1;
    adv_data[1] = 0x09;  //adv type :local name
    memcpy(&adv_data[2],DEVICE_NAME,DEVICE_NAME_LEN);
    struct ln_gapm_set_adv_data_cmd adv_data_param;
    adv_data_param.actv_idx = adv_actv_idx;
    adv_data_param.length = sizeof(adv_data);
    adv_data_param.data = adv_data;
    ln_app_set_adv_data(&adv_data_param);

}

static void app_start_advertising(void)
{
    struct ln_gapm_activity_start_cmd  adv_start_param;
    adv_start_param.actv_idx = adv_actv_idx;
    adv_start_param.u_param.adv_add_param.duration = 0;
    adv_start_param.u_param.adv_add_param.max_adv_evt = 0;
    ln_app_advertise_start(&adv_start_param);
}

void app_restart_adv(void)
{
    app_start_advertising();
}

void app_create_init(void)
{
    struct ln_gapm_activity_create_cmd init_creat_param;
    init_creat_param.own_addr_type = GAPM_STATIC_ADDR;
    ln_app_init_creat(&init_creat_param);
}

static void app_start_init(void)
{
    uint8_t peer_addr[6]= {0x12,0x34,0x56,0x78,0x90,0xab};
    struct ln_gapm_activity_start_cmd  init_start_param = {0};

    init_start_param.actv_idx                                        = init_actv_idx;
    init_start_param.u_param.init_param.type                         = GAPM_INIT_TYPE_DIRECT_CONN_EST;//GAPM_INIT_TYPE_DIRECT_CONN_EST;
    init_start_param.u_param.init_param.prop                         = GAPM_INIT_PROP_1M_BIT;//GAPM_INIT_PROP_CODED_BIT;
    init_start_param.u_param.init_param.conn_to                      = 0;
    init_start_param.u_param.init_param.scan_param_1m.scan_intv      = 16; //16*0.625 ms=10ms
    init_start_param.u_param.init_param.scan_param_1m.scan_wd        = 16;// 16*0.625ms=10ms
    init_start_param.u_param.init_param.conn_param_1m.conn_intv_min  = 80; // 10x1.25ms  (7.5ms--4s)
    init_start_param.u_param.init_param.conn_param_1m.conn_intv_max  = 80; // 10x1.25ms  (7.5ms--4s)
    init_start_param.u_param.init_param.conn_param_1m.conn_latency   = 0;
    init_start_param.u_param.init_param.conn_param_1m.supervision_to = 500; //100x10ms= 1 s
    init_start_param.u_param.init_param.conn_param_1m.ce_len_min     = 0;
    init_start_param.u_param.init_param.conn_param_1m.ce_len_max     = 0;
    init_start_param.u_param.init_param.peer_addr.addr_type          = 0;
    memcpy(init_start_param.u_param.init_param.peer_addr.addr.addr, peer_addr, GAP_BD_ADDR_LEN);

    ln_app_init_start(&init_start_param);
}

 void app_restart_init(void)
{
    app_start_init();
}

static void start_adv(void)
{
    app_create_advertising();
    app_set_adv_data();
    app_start_advertising();
}

static void start_init(void)
{
    app_create_init();
    app_start_init();
}

static OS_Queue_t ble_usr_queue;

void usr_creat_queue(void)
{
    if(OS_OK != OS_QueueCreate(&ble_usr_queue, BLE_USR_MSG_QUEUE_SIZE, sizeof(ble_usr_msg_t)))
    {
        BLIB_LOG(BLIB_LOG_LVL_E, "usr QueueCreate rw_queue failed!!!\r\n");
    }
}

void usr_queue_msg_send(uint16_t id, uint16_t length, void *msg)
{
    ble_usr_msg_t usr_msg;
    usr_msg.id = id;
    usr_msg.len = length;
    usr_msg.msg = msg;
    OS_QueueSend(&ble_usr_queue, &usr_msg, OS_WAIT_FOREVER);
}

int usr_queue_msg_recv(void *msg, uint32_t timeout)
{
    return OS_QueueReceive(&ble_usr_queue, msg, timeout);
}

static void ble_app_task_entry(void *params)
{
    ble_usr_msg_t usr_msg;

    usr_creat_queue();

    extern void ble_app_init(void);
    ble_app_init();
#if (SLAVE)
    start_adv();
#endif
#if (MASTER)
    start_init();
#endif
#if SERVICE
    data_trans_svc_add();
#endif

    while(1)
    {
        if(OS_OK == usr_queue_msg_recv((void *)&usr_msg, OS_WAIT_FOREVER))
        {
            LOG(LOG_LVL_TRACE, "connect device number :%d \r\n",con_num);
            switch(usr_msg.id)
            {
                case BLE_MSG_WRITE_DATA:
                {
                    struct ln_attc_write_req_ind *p_param = (struct ln_attc_write_req_ind *)usr_msg.msg;
                    struct ln_gattc_send_evt_cmd send_data;
                    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_param->value, p_param->length);
                    send_data.handle = p_param->handle + 2;
                    send_data.length = p_param->length;
                    send_data.value = p_param->value;
                    ln_app_gatt_send_ntf(p_param->conidx,&send_data);
                }
                break;

                case BLE_MSG_CONN_IND:
                {
                    struct ln_gapc_connection_req_info *p_param=(struct ln_gapc_connection_req_info *)usr_msg.msg;
#if (CLIENT)
                    struct ln_gattc_disc_cmd param_ds;
                    param_ds.operation = GATTC_DISC_BY_UUID_SVC;
                    param_ds.start_hdl = 1;
                    param_ds.end_hdl   = 0xFFFF;
                    param_ds.uuid_len  =sizeof(svc_uuid);
                    param_ds.uuid = svc_uuid;
                    ln_app_gatt_discovery(p_param->conidx, &param_ds);
#endif
                    ln_app_gatt_exc_mtu(p_param->conidx);
                    struct ln_gapc_set_le_pkt_size_cmd pkt_size;
                    pkt_size.tx_octets = 251;
                    pkt_size.tx_time   = 2120;
                    OS_MsDelay(1000);
                    ln_app_param_set_pkt_size(p_param->conidx,  &pkt_size);

                    struct ln_gapc_conn_param conn_param;
                    conn_param.intv_min = 80;  // 10x1.25ms  (7.5ms--4s)
                    conn_param.intv_max = 90;  // 10x1.25ms  (7.5ms--4s)
                    conn_param.latency  = 10;
                    conn_param.time_out = 3000;  //ms*n
                    ln_app_update_param(p_param->conidx, &conn_param);
                }
                break;

                case BLE_MSG_SVR_DIS:
                {
#if (CLIENT)
                    struct ln_gattc_disc_svc *p_param = (struct ln_gattc_disc_svc *)usr_msg.msg;
                    uint8_t data[] = {0x12,0x78,0x85};
                    struct ln_gattc_write_cmd param_wr;
                    param_wr.operation    = GATTC_WRITE;
                    param_wr.auto_execute = true;
                    param_wr.handle       = p_param->start_hdl + 2;
                    param_wr.length       = sizeof(data);
                    param_wr.offset = 0;
                    param_wr.value = data;
                    ln_app_gatt_write(p_param->conidx,&param_wr);
#endif
                }
                break;

                default:
                    break;
            }
            blib_free(usr_msg.msg);
        }
    }
}

void creat_usr_app_task(void)
{
    create_at_cmd_plus_task();

    if(OS_OK != OS_ThreadCreate(&g_usr_app_thread, "WifiUsrAPP", usr_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, USR_APP_TASK_STACK_SIZE)) {
        LN_ASSERT(1);
    }

    if(OS_OK != OS_ThreadCreate(&ble_g_usr_app_thread, "BleUsrAPP", ble_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, BLE_USR_APP_TASK_STACK_SIZE))
    {
        LN_ASSERT(1);
    }

#if  WIFI_TEMP_CALIBRATE
    if(OS_OK != OS_ThreadCreate(&g_temp_cal_thread, "TempAPP", temp_cal_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, TEMP_APP_TASK_STACK_SIZE)) {
        LN_ASSERT(1);
    }
#endif

    /* print sdk version */
    {
        LOG(LOG_LVL_INFO, "LN882H SDK Ver: %s [build time:%s][0x%08x]\r\n",
                LN882H_SDK_VERSION_STRING, LN882H_SDK_BUILD_DATE_TIME, LN882H_SDK_VERSION);
    }
}
