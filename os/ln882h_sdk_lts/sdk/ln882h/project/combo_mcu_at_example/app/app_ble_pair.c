#include "app_ble_pair.h"

static void init_ble_pair_manage(void);
static void deinit_ble_pair_manage(void);
static void ble_start_adv(uint8_t *adv_name, uint8_t len);
static void ble_response_link_info(void);
static void ble_stop_adv(void);
static ln_at_err_t ln_at_exec_ble_pair_start(const char *name); 
static void ble_pair_thread_entry(void * param);
static OS_Status Semaphore_Reset(OS_Semaphore_t *sem);
static OS_Status Semaphore_Wait(OS_Semaphore_t *sem, uint32_t timeout);

extern ln_ble_adv_manager_t ble_adv_mgr;
extern ble_dev_conn_manager_t ble_conn_mgr;
extern struct user_svc_desc g_user_svc_desc_tab;

static uint8_t ble_name[] = {'L', 'N', '_', 'B', 'L', 'E', '_', 'N', 'E', 'T', '_', 'C', 'O', 'N', 'F', 'I', 'G'};

at_ble_pair_manage_t g_ble_mgr = {
    0
};

static at_ble_pair_manage_t *ble_mgr_get_handle(void)
{
    return &g_ble_mgr;
}

static void init_ble_pair_manage(void)
{
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    
    p->ble_pair_start_flag = true;

    p->func.start_adv = ble_start_adv;
    p->func.response_get_link_info = ble_response_link_info;
    p->func.stop_adv = ble_stop_adv;
    
    OS_SemaphoreCreateBinary(&p->sem.connect_sem);
    OS_SemaphoreCreateBinary(&p->sem.get_pwd_sem); 
    OS_MutexCreate(&p->mutex);
    
    p->ssid = OS_Malloc(sizeof(char) * AT_BLE_SSID_LEN);
    p->pwd = OS_Malloc(sizeof(char)* AT_BLE_PWD_LEN);   
    memset(p->ssid, 0, sizeof(char) * AT_BLE_SSID_LEN);
    memset(p->pwd, 0, sizeof(char) * AT_BLE_PWD_LEN);
}

static void deinit_ble_pair_manage(void)
{
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    
    p->ble_pair_start_flag = false;
    
    OS_SemaphoreDelete(&p->sem.connect_sem);
    OS_SemaphoreDelete(&p->sem.get_pwd_sem);
    OS_MutexDelete(&p->mutex);
    
    OS_ThreadDelete(&p->thread);
    
    OS_Free(p->ssid);
    p->ssid = NULL;
    OS_Free(p->pwd);
    p->pwd = NULL;
}

static void ble_start_adv(uint8_t *adv_name, uint8_t len)
{
    //adv data: adv length--adv type--adv string ASCII
    uint8_t adv_data_len = 0;
    uint8_t adv_data[ADV_DATA_LEGACY_MAX] = {0};
    ln_adv_data_t adv_data_param;

    uint16_t name_len = len;
    uint8_t *name = adv_name;

   
    if(name_len)
    {
        if(name_len > (ADV_DATA_LEGACY_MAX-2))
            name_len = ADV_DATA_LEGACY_MAX-2;
        /*add item ble_name*/
        adv_data[0] = name_len + 1;
        adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;  //adv type :local name
        adv_data_len += 2;
        memcpy(&adv_data[2], name, name_len);
        adv_data_len += name_len;
    }

    adv_data_param.length = adv_data_len;
    adv_data_param.data = adv_data;

    ln_ble_adv_data_set(&adv_data_param);
    //start adv
    ln_ble_adv_start();
}

static void ble_response_link_info(void)
{
    ln_trans_svr_send_t svc_send_cmd;
    char response_data[20] = {0};
    
    memcpy(response_data, AT_BLE_RESPONSE_DATA, strlen(AT_BLE_RESPONSE_DATA));
    svc_send_cmd.conn_idx = 0;
    svc_send_cmd.data = (uint8_t *)response_data;
    svc_send_cmd.len = strlen(response_data);
    svc_send_cmd.hdl = 0x0E + DATA_TRANS_DECL_CHAR_TX_VAL;
    ln_ble_trans_svr_ntf(&svc_send_cmd);
    
}

static void ble_stop_adv(void)
{
    ln_ble_adv_stop();
}

static OS_Status Semaphore_Reset(OS_Semaphore_t *sem)
{
    return OS_SemaphoreWait(sem, 0);
}

static OS_Status Semaphore_Wait(OS_Semaphore_t *sem, uint32_t timeout)
{
    return OS_SemaphoreWait(sem, timeout);
}

static OS_Status Semaphore_Release(OS_Semaphore_t *sem)
{
    return OS_SemaphoreRelease(sem);
}

static void ble_connected_cb(void *arg)
{
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    
    Semaphore_Release(&p->sem.connect_sem);
}

static void ble_recv_pair_info_cb(void *arg)
{
    ble_evt_gatt_write_req_t *p = (ble_evt_gatt_write_req_t *)arg;
    at_ble_pair_manage_t *p_ble_mgr = ble_mgr_get_handle();

    //recv ssid
    if(0 == memcmp(p->value, "ssid=", strlen("ssid=")))
    {
        LOG(LOG_LVL_INFO, "ssid : %s\r\n", p->value + strlen("ssid="));
        memcpy(p_ble_mgr->ssid, p->value + strlen("ssid="), p->length - strlen("ssid="));
    }
    else if(0 == memcmp(p->value, "pwd=", strlen("pwd=")))
    {
        LOG(LOG_LVL_INFO, "pwd : %s\r\n", p->value + strlen("pwd="));
        memcpy(p_ble_mgr->pwd, p->value + strlen("pwd="), p->length - strlen("pwd="));
        Semaphore_Release(&p_ble_mgr->sem.get_pwd_sem);
    }
}

static void ble_pair_thread_entry(void * param)
{
    wifi_mode_t wifi_mode = WIFI_MODE_MAX;
    wifi_sta_connect_ext_t sys_jap_conn_cfg = {0};
    wifi_sta_connect_t conn = {0};
    wifi_sta_status_t wifi_conn_status = 0;
    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();    
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    at_wifi_adapter_t *p_ada = at_wifi_adapter_get_handle();
    //init pair_manage
    init_ble_pair_manage(); 
    
    //check system ble status
    OS_MutexLock(&p->mutex,OS_WAIT_FOREVER);
    if(BLE_ROLE_PERIPHERAL != ln_kv_ble_usr_data_get()->ble_role)
    {
        ln_at_printf("ble role is not peripheral,need to set to peripheral!\r\n");
        goto __err;
    }
    /*TO-DO*/
    //need check ble stack init status & adv status & connecting status
    if(ln_ble_is_connected())
    {
        ln_at_printf("ble is connected ,pls disconnected the peer device!\r\n");
        goto __err;
    }
    OS_MutexUnlock(&p->mutex);
        
    //ble write cb regist
    ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_WRITE_REQ, ble_recv_pair_info_cb);
    //stop adv
    p->func.stop_adv();
    //start adv
    p->func.start_adv(ble_name, sizeof(ble_name));
    
    ln_at_printf("Start to BLE Pair Mode.\r\n");

    //wait 30s, if not get_pwd, pair fail
    if(OS_OK != Semaphore_Wait(&p->sem.get_pwd_sem, 180000))
    {
        ln_at_printf("BLE Pair Timeout, Exit Pair Mode...\r\n");
        goto __err;
    }
    
    //check if exec AT-BLEPAIRSTOP
    if(!p->ble_pair_start_flag)
    {
        ln_at_printf("Exit Pair Mode...\r\n");
        goto __err;
    }  
    //check wifi status
    //check wifi mode
    if(WIFI_MODE_STATION == (wifi_mode = wifi_current_mode_get()))
        wifi_get_sta_status(&wifi_conn_status);


    //save last connect info
    memcpy(&sys_jap_conn_cfg, &p_ada->conn_ext_cfg, sizeof(wifi_sta_connect_ext_t));
    memset(&p_ada->conn_ext_cfg, 0, sizeof(wifi_sta_connect_ext_t));
    
    //set ssid & pwd
    memcpy(&p_ada->conn_ext_cfg.ssid, p->ssid, strlen(p->ssid));
    memcpy(&p_ada->conn_ext_cfg.pwd, p->pwd, strlen(p->pwd));
    
    //connect wifi
    p_ada->at_func.wifi_sta_connect_func();
    
    if (0 != pWifi->wait_for_ip(20000))
    {
        ln_at_printf("Connect Wifi Fail...\r\n");
        goto __err;
    }
    //check if exec AT-BLEPAIRSTOP
    if(!p->ble_pair_start_flag)
    {
        ln_at_printf("Exit Pair Mode...\r\n");
        goto __err;
    }
    
    //ble send response
    p->func.response_get_link_info();
    
    //sysparam restore
    conn.ssid = p->ssid;
    conn.pwd = p->pwd;
    conn.psk_value = NULL;
    conn.bssid = NULL;
    sysparam_sta_conn_cfg_update(&conn);
    
    //deinit ble pair manage
    deinit_ble_pair_manage();      
__err:
    //restore sys param
    memcpy(&p_ada->conn_ext_cfg, &sys_jap_conn_cfg, sizeof(wifi_sta_connect_ext_t));
    //back to last status
    if(WIFI_MODE_STATION == wifi_mode)
    {
        if(WIFI_STA_STATUS_CONNECTED == wifi_conn_status)
        {
            //reconnect
            ln_at_printf("now reconnect...\r\n");
            p_ada->at_func.wifi_sta_connect_func();
        }
        else
        {
            p_ada->at_func.wifi_sta_init_func();
        }
    }
    else if(WIFI_MODE_AP == wifi_mode)
    {
        pWifi->start_softap();
    }
    //delete thread
    deinit_ble_pair_manage();
    
}

static int ble_pair_function(void)
{
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    
    if(OS_OK != OS_ThreadCreate(&p->thread, "ble_pair", ble_pair_thread_entry, NULL, OS_PRIORITY_BELOW_NORMAL, BLE_PAIR_TASK_STACK_SIZE))
    {
        LOG(LOG_LVL_ERROR, "create thread fail...\r\n");
        return -1;
    }
    
    return 0;
}

static ln_at_err_t ln_at_exec_ble_pair_start(const char *name) 
{
    if(0 == ble_pair_function())
    {
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;    
    }
    else
    {
        ln_at_printf(LN_AT_RET_ERR_STR);
        return LN_AT_ERR_COMMON;            
    }
}

static ln_at_err_t ln_at_exec_ble_pair_stop(const char *name)
{
    ln_at_err_t ret = 0;
    at_ble_pair_manage_t *p = ble_mgr_get_handle();
    LN_WIFI_DRIVER_ADAPTER* pWifi = ln_get_wifi_drv_handle();
    
    if(false == p->ble_pair_start_flag)
    {
        ln_at_printf("BLE is not in pair mode...\r\n");
        goto __err;
    }
    else
    {
        OS_MutexLock(&p->mutex, OS_WAIT_FOREVER);
        p->ble_pair_start_flag = false;
        OS_MutexUnlock(&p->mutex);
        
        Semaphore_Release(&p->sem.get_pwd_sem);
        pWifi->release_ip_sem();      
    }
    
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;    
    
__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;    
}

LN_AT_CMD_REG(BLEPAIRSTART, NULL, NULL, NULL, ln_at_exec_ble_pair_start);

LN_AT_CMD_REG(BLEPAIRSTOP, NULL, NULL, NULL, ln_at_exec_ble_pair_stop);