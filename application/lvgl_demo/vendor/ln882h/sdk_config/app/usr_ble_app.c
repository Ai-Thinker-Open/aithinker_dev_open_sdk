#include "rwip_config.h"    // RW SW configuration TASK_APP
#include "ke_task.h"        // Kernel Task

#include "osal/osal.h"
#include "utils/debug/ln_assert.h"
#include "utils/debug/log.h"
#include "ln_utils.h"
#include "ln_misc.h"
#include "gapm_task.h"
#include "gapc_task.h"
#include "gattc_task.h"
#include "gattm_task.h"
#include "gap.h"

#include "ln_ble_gap.h"
#include "ln_ble_gatt.h"
#include "ln_ble_advertising.h"
#include "ln_ble_scan.h"
#include "ln_ble_rw_app_task.h"
#include "ln_ble_app_kv.h"
#include "ln_ble_connection_manager.h"
#include "ln_ble_event_manager.h"
#include "ln_ble_smp.h"
#include "usr_ble_app.h"
#include "ln_ble_app_default_cfg.h"


enum ble_usr_msg_type
{
    BLE_MSG_CONN_IND,
    BLE_MSG_DISCONN_IND,
    BLE_MSG_SCAN_REPORT,
    BLE_MSG_GATT_READ_REQ,
    BLE_MSG_GATT_WRITE_DATA,
    BLE_MSG_GATT_READ_RSP,
    BLE_MSG_GATT_NOTIFY,
    BLE_MSG_GATT_INDICATION,
};

#if (BLE_DATA_TRANS_SERVER)
struct user_svc_desc {
    ln_trans_svr_desc_t desc;
    uint8_t ccc;//character client config
};
#endif

#if (BLE_DATA_TRANS_CLIENT)
struct user_cli_desc {
    uint8_t con_idx;
    uint16_t start_handle;
    uint16_t end_handle;
    uint8_t svc_uuid_len;
    uint8_t svc_uuid[16];
};
#endif

typedef struct
{
    uint16_t  id;
    uint16_t  len;
    void     *msg;
} ble_usr_msg_t;

#define BLE_USR_MSG_QUEUE_SIZE          (96)
#define BLE_USR_APP_TASK_STACK_SIZE     (1024)
#define DATA_TRANS_SVR_MAX              (2)

static OS_Thread_t ble_g_usr_app_thread;
static OS_Queue_t ble_usr_queue;
static OS_Timer_t ble_usr_timer;

static uint8_t ext_adv_data[ADV_DATA_EXTEND_MAX] = {0};

#define DATA_TRANS_1ST_SVR_UUID     {0x85,0x41,0xDC,0x24,0x0E,0xE5,0xA9,0xE0,0x93,0xF3,0xA3,0xB5,0x01,0x00,0x40,0x6E}
#define DATA_TRANS_1ST_RX_UUID      {0xff,0xcc,0xdb,0xe2,0x2a,0x2a,0x32,0xa3,0xe9,0x11,0x67,0xcd,0x4a,0x4a,0xbe,0x28} //28be4a4a-cd67-11e9-a32f-2a2ae2dbcce4
#define DATA_TRANS_1ST_TX_UUID      {0xee,0xcc,0xdb,0xe2,0x2a,0x2a,0x23,0xa3,0xe9,0x11,0x67,0xcd,0xb6,0x4c,0xbe,0x28}//28be4cb6-cd67-11e9-a32f-2a2ae2dbcce4

#define DATA_TRANS_2ND_SVR_UUID   {0xe1, 0xff}
#define DATA_TRANS_2ND_RX_UUID    {0xe2, 0xff}
#define DATA_TRANS_2ND_TX_UUID    {0xe3, 0xff}

#if (BLE_DATA_TRANS_SERVER)
static const ln_attm_desc_t data_trans_1st_atts_db[] = {
    [DATA_TRANS_DECL_SVC] = {
        .uuid = {0x00, 0x28},
        .perm = PERM_MASK_RD,
        .ext_perm = (0 << PERM_POS_UUID_LEN),
        .max_size = 0,
    },
    [DATA_TRANS_DECL_CHAR_RX] = {
        .uuid = {0x03, 0x28},
        .perm = PERM_MASK_RD,
        .ext_perm = 0,
        .max_size = 0,
    },
    [DATA_TRANS_DECL_CHAR_RX_VAL] = {
        .uuid = DATA_TRANS_1ST_RX_UUID,
        .perm = PERM_MASK_WRITE_REQ | PERM_MASK_WRITE_COMMAND | PERM_MASK_RD,
        .ext_perm = (2 << PERM_POS_UUID_LEN) | (1 << PERM_POS_RI),
        .max_size = CHAR_VAL_MAX_LEN,
    },
    [DATA_TRANS_DECL_CHAR_TX] = {
        .uuid       = {0x03, 0x28},
        .perm       = PERM_MASK_RD,
        .ext_perm   = 0,
        .max_size   = 0,
    },
    [DATA_TRANS_DECL_CHAR_TX_VAL] = {
        .uuid       = DATA_TRANS_1ST_TX_UUID,
        .perm       = PERM_MASK_NTF | PERM_MASK_RD,
        .ext_perm   = (2 << PERM_POS_UUID_LEN) | (1 << PERM_POS_RI),
        .max_size   = CHAR_VAL_MAX_LEN,
    },
    [DATA_TRANS_DECL_CHAR_TX_CCC] = {
        .uuid       = {0x02, 0x29},
        .perm       = PERM_MASK_WRITE_REQ | PERM_MASK_RD,
        .ext_perm    = 0,
        .max_size    = 0,
    },
};

static const ln_attm_desc_t data_trans_2nd_atts_db[] = {
    [DATA_TRANS_DECL_SVC] = {
        .uuid = {0x00, 0x28},
        .perm = PERM_MASK_RD,
        .ext_perm = (0 << PERM_POS_UUID_LEN),
        .max_size = 0,
    },
    [DATA_TRANS_DECL_CHAR_RX] = {
        .uuid = {0x03, 0x28},
        .perm = PERM_MASK_RD,
        .ext_perm = 0,
        .max_size = 0,
    },
    [DATA_TRANS_DECL_CHAR_RX_VAL] = {
        .uuid = DATA_TRANS_2ND_RX_UUID,
        .perm = PERM_MASK_WRITE_REQ | PERM_MASK_WRITE_COMMAND | PERM_MASK_RD,
        .ext_perm = (1 << PERM_POS_RI),
        .max_size = CHAR_VAL_MAX_LEN,
    },
    [DATA_TRANS_DECL_CHAR_TX] = {
        .uuid       = {0x03, 0x28},
        .perm       = PERM_MASK_RD,
        .ext_perm   = 0,
        .max_size   = 0,
    },
    [DATA_TRANS_DECL_CHAR_TX_VAL] = {
        .uuid       = DATA_TRANS_2ND_TX_UUID,
        .perm       = PERM_MASK_NTF | PERM_MASK_RD,
        .ext_perm   = (1 << PERM_POS_RI),
        .max_size   = CHAR_VAL_MAX_LEN,
    },
    [DATA_TRANS_DECL_CHAR_TX_CCC] = {
        .uuid       = {0x02, 0x29},
        .perm       = PERM_MASK_WRITE_REQ | PERM_MASK_RD,
        .ext_perm    = 0,
        .max_size    = 0,
    },
};

struct user_svc_desc g_user_svc_desc_tab[DATA_TRANS_SVR_MAX] = {
    {
        .desc = {
            .start_handle   = LN_ATT_INVALID_HANDLE,
            .svr_uuid_len   = 16,
            .svr_uuid       = DATA_TRANS_1ST_SVR_UUID,
            .att_count      = sizeof(data_trans_1st_atts_db)/sizeof(data_trans_1st_atts_db[0]),
            .att_desc       = &data_trans_1st_atts_db,
        },
        .ccc = 0,
    },
    {
        .desc = {
            .start_handle   = LN_ATT_INVALID_HANDLE,
            .svr_uuid_len   = 2,
            .svr_uuid       = DATA_TRANS_2ND_SVR_UUID,
            .att_count      = sizeof(data_trans_2nd_atts_db)/sizeof(data_trans_2nd_atts_db[0]),
            .att_desc        = &data_trans_2nd_atts_db,
        },
        .ccc = 0
    }
};

#endif

#if (BLE_DATA_TRANS_CLIENT)
struct user_cli_desc g_user_cli_desc_tab[DATA_TRANS_SVR_MAX] = {
    {
        .con_idx        = APP_CONN_INVALID_IDX,
        .start_handle   = LN_ATT_INVALID_HANDLE,
        .end_handle     = LN_ATT_INVALID_HANDLE,
        .svc_uuid_len   = 16,
        .svc_uuid       = DATA_TRANS_1ST_SVR_UUID,
    },
    {
        .con_idx        = APP_CONN_INVALID_IDX,
        .start_handle   = LN_ATT_INVALID_HANDLE,
        .end_handle     = LN_ATT_INVALID_HANDLE,
        .svc_uuid_len   = 2,
        .svc_uuid       = DATA_TRANS_2ND_SVR_UUID,
    },
};
#endif

static void usr_queue_creat(void)
{
    if(OS_OK != OS_QueueCreate(&ble_usr_queue, BLE_USR_MSG_QUEUE_SIZE, sizeof(ble_usr_msg_t)))
    {
        LOG(LOG_LVL_INFO, "usr QueueCreate rw_queue failed!!!\r\n");
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

static void ble_test_send_func(void)
{
    uint8_t test_data[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    if(ln_kv_ble_usr_data_get()->ble_role & BLE_ROLE_PERIPHERAL)
    {
#if (BLE_DATA_TRANS_SERVER)
        ln_trans_svr_send_t svc_send_cmd;
        for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
            if(g_user_svc_desc_tab[i].ccc && LN_ATT_INVALID_HANDLE != g_user_svc_desc_tab[i].desc.start_handle) {
                svc_send_cmd.conn_idx = 0;
                svc_send_cmd.data = test_data;
                svc_send_cmd.len = sizeof(test_data);
                svc_send_cmd.hdl = g_user_svc_desc_tab[i].desc.start_handle+DATA_TRANS_DECL_CHAR_TX_VAL;
                ln_ble_trans_svr_ntf(&svc_send_cmd);
            }
        }
#endif
    }
    if(ln_kv_ble_usr_data_get()->ble_role & BLE_ROLE_CENTRAL)
    {
#if (BLE_DATA_TRANS_CLIENT)
        ln_trans_cli_send_t cli_send_cmd;
        for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
            if(LN_ATT_INVALID_HANDLE != g_user_cli_desc_tab[i].start_handle) {
                cli_send_cmd.conn_idx = 0;
                cli_send_cmd.data = test_data;
                cli_send_cmd.len = sizeof(test_data);
                cli_send_cmd.hdl = g_user_cli_desc_tab[i].start_handle+DATA_TRANS_DECL_CHAR_RX_VAL;
                ln_ble_trans_cli_write_cmd(&cli_send_cmd);
            }
        }
#endif
    }
}

void user_timer_callback (void *arg)
{
    LOG(LOG_LVL_TRACE,"user_timer_callback\r\n");
    ble_test_send_func();
}

static int user_timer_creat(void)
{
     if(OS_OK != OS_TimerCreate(&ble_usr_timer, OS_TIMER_PERIODIC, user_timer_callback, NULL, 1000))
     {
          LOG(LOG_LVL_TRACE,"usr timer create fail\r\n");
          return 1;
     }
    return 0;
}

int user_timer_start(void *timer)
{
    if (OS_OK != OS_TimerStart((OS_Timer_t *)timer))
    {
        LOG(LOG_LVL_TRACE,"user timer start fail\r\n");
        return 1;
    }

    return 0;
}

int user_timer_stop(void *timer)
{
    if (OS_OK != OS_TimerStop((OS_Timer_t *)timer))
    {
        LOG(LOG_LVL_TRACE,"user timer stop fail\r\n");
        return 1;
    }

    return 0;
}

int user_timer_delete(void *timer)
{
    if (OS_OK != OS_TimerDelete((OS_Timer_t *)timer))
    {
        LOG(LOG_LVL_TRACE,"user timer delete fail\r\n");
        return 1;
    }
    return 0;
}

#ifdef BLE_USE_LEGACY_ADV
static void app_set_adv_data(void)
{
    //adv data: adv length--adv type--adv string ASCII
    uint8_t adv_data_len = 0;
    uint8_t adv_data[ADV_DATA_LEGACY_MAX] = {0};
    ln_adv_data_t adv_data_param;

    uint16_t name_len = 0;
    uint8_t *name = ln_kv_ble_dev_name_get(&name_len);

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
    
    hexdump(LOG_LVL_INFO, (const char *)"legacy_adv", adv_data, adv_data_len);
}

static void app_set_scan_resp_data(void)
{
    //adv data: adv length--adv type--adv string ASCII
    uint8_t adv_data_len = 0;
    uint8_t adv_data[ADV_DATA_LEGACY_MAX] = {0};
    ln_adv_data_t adv_data_param;
    bd_addr_t addr = {0};

    ln_bd_addr_t *kv_addr = ln_kv_ble_pub_addr_get();
    memcpy(addr.addr, kv_addr->addr, 6);
    
    /*add item ble_addr*/
    adv_data[0] = BD_ADDR_LEN + 1;
    adv_data[1] = GAP_AD_TYPE_LE_BT_ADDR;
    adv_data_len += 2;
    memcpy(&adv_data[2], addr.addr, BD_ADDR_LEN);
    adv_data_len += BD_ADDR_LEN;

    adv_data_param.length = adv_data_len;
    adv_data_param.data = adv_data;
    ln_ble_adv_scan_rsp_data_set(&adv_data_param);
    
    hexdump(LOG_LVL_INFO, (const char *)"legacy_scan_rsp", adv_data, adv_data_len);
}
#else
static void app_set_ext_adv_data(void)
{
    //adv data: adv length--adv type--adv string ASCII
    uint16_t adv_data_len = 0;
    ln_adv_data_t adv_data_param;

    uint16_t name_len = 0;
    uint8_t *name = ln_kv_ble_dev_name_get(&name_len);

    if(name_len)
    {
        if(name_len > BLE_DEV_NAME_MAX_LEN)
            name_len = BLE_DEV_NAME_MAX_LEN;
        /*add item ble_name*/
        ext_adv_data[0] = name_len + 1;
        ext_adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;  //adv type :local name
        adv_data_len += 2;
        memcpy(&ext_adv_data[2], name, name_len);
        adv_data_len += name_len;
    }
    ext_adv_data[adv_data_len++] = 201;
    ext_adv_data[adv_data_len++] = 0xff;
    for(int i = adv_data_len; i < adv_data_len + 200; i++)
    {
        ext_adv_data[i] = i;
    }
    adv_data_len += 200;

    adv_data_param.length = adv_data_len;
    adv_data_param.data = ext_adv_data;
    ln_ble_adv_data_set(&adv_data_param);
    
    hexdump(LOG_LVL_INFO, (const char *)"ext_adv", ext_adv_data, adv_data_len);
}

static void app_set_ext_scan_resp_data(void)
{
    //adv data: adv length--adv type--adv string ASCII
    uint16_t adv_data_len = 0;
    ln_adv_data_t adv_data_param;

    uint16_t name_len = 0;
    uint8_t *name = ln_kv_ble_dev_name_get(&name_len);

    if(name_len)
    {
        if(name_len > BLE_DEV_NAME_MAX_LEN)
            name_len = BLE_DEV_NAME_MAX_LEN;
        /*add item ble_name*/
        ext_adv_data[0] = name_len + 1;
        ext_adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;  //adv type :local name
        adv_data_len += 2;
        memcpy(&ext_adv_data[2], name, name_len);
        adv_data_len += name_len;
    }
    ext_adv_data[adv_data_len++] = 201;
    ext_adv_data[adv_data_len++] = 0xff;
    for(int i = adv_data_len; i < adv_data_len + 200; i++)
    {
        ext_adv_data[i] = i;
    }
    adv_data_len += 200;

    adv_data_param.length = adv_data_len;
    adv_data_param.data = ext_adv_data;
    ln_ble_adv_scan_rsp_data_set(&adv_data_param);
}
#endif

static void ln_ble_connect_cb(void *arg)
{
    ble_evt_connected_t * evt_conn = (ble_evt_connected_t *)arg;

    uint8_t conn_idx = evt_conn->conn_idx;
    LOG(LOG_LVL_TRACE, "ln_ble_connect conn_id=%d\r\n", conn_idx);

    ble_evt_connected_t *data = blib_malloc(sizeof(ble_evt_connected_t));
    memcpy(data, evt_conn, sizeof(ble_evt_connected_t));
    usr_queue_msg_send(BLE_MSG_CONN_IND, sizeof(ble_evt_connected_t), data);

    user_timer_start(&ble_usr_timer);

#if (BLE_DATA_TRANS_CLIENT)
    if(ln_kv_ble_usr_data_get()->ble_role & BLE_ROLE_CENTRAL) {
        for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
            ln_ble_trans_disc_svc(evt_conn->conn_idx, g_user_cli_desc_tab[i].svc_uuid,
                        g_user_cli_desc_tab[i].svc_uuid_len);
        }
    }
#endif
}

static void ln_ble_disconnect_cb(void *arg)
{
    ble_evt_disconnected_t *evt_disconn = (ble_evt_disconnected_t *)arg;
    uint8_t ble_role = ln_kv_ble_usr_data_get()->ble_role;
    LOG(LOG_LVL_TRACE, "ln_ble_disconnect conn_id=%d\r\n", evt_disconn->conn_idx);

    if((ble_role & BLE_ROLE_PERIPHERAL)) {
        if(BLE_CONFIG_AUTO_ADV && LE_ADV_STATE_STOPED == le_adv_state_get())
        {
            ln_ble_adv_start();
        }
#if (BLE_DATA_TRANS_SERVER)
        for(int i=0;i<DATA_TRANS_SVR_MAX;i++)
            g_user_svc_desc_tab[i].ccc = 0;
#endif
    }

    if((ble_role & BLE_ROLE_CENTRAL)) {
#if (BLE_DATA_TRANS_CLIENT)
        for(int i=0;i<DATA_TRANS_SVR_MAX;i++) {
            g_user_cli_desc_tab[i].con_idx        = APP_CONN_INVALID_IDX;
            g_user_cli_desc_tab[i].start_handle   = LN_ATT_INVALID_HANDLE;
            g_user_cli_desc_tab[i].end_handle     = LN_ATT_INVALID_HANDLE;
        }
#endif
    }

    user_timer_stop(&ble_usr_timer);
}

static void ln_ble_scan_report_cb(void *arg)
{
    ble_evt_scan_report_t *p_scan_rpt = (ble_evt_scan_report_t *)arg;
    adv_data_item_t *name_item = NULL;

    LOG(LOG_LVL_TRACE, "\r\n--------------adv_report-------------\r\n");
    LOG(LOG_LVL_TRACE, "rssi=%d, tx_pwr=%d\r\n", p_scan_rpt->rssi, p_scan_rpt->tx_pwr);
    LOG(LOG_LVL_TRACE, "addr_type=%d, addr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
            p_scan_rpt->trans_addr_type, p_scan_rpt->trans_addr[0], p_scan_rpt->trans_addr[1],
            p_scan_rpt->trans_addr[2], p_scan_rpt->trans_addr[3], p_scan_rpt->trans_addr[4], p_scan_rpt->trans_addr[5]);
    name_item = ln_ble_scan_data_find_item_by_type(GAP_AD_TYPE_COMPLETE_NAME, p_scan_rpt->data, p_scan_rpt->length);
    if(name_item)
    {
        char name[40];
        memcpy(name, name_item->data, name_item->len-1);
        name[name_item->len-1] = '\0';
        LOG(LOG_LVL_TRACE, "device name:%s\r\n", name);
    }
    //hexdump(LOG_LVL_INFO, "adv data: ", (void *)p_scan_rpt->data, p_scan_rpt->length);
    LOG(LOG_LVL_TRACE, "\r\n\r\n");
}

#if (BLE_DATA_TRANS_SERVER)
static void ln_ble_gatt_read_req_cb(void *arg)
{
    ble_evt_gatt_read_req_t *p_gatt_read = (ble_evt_gatt_read_req_t *)arg;
    uint8_t send_data[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};

    LOG(LOG_LVL_TRACE, "ln_ble_gatt_read conn_id=%d,handle=%d",p_gatt_read->conidx, p_gatt_read->handle);

    p_gatt_read->length = sizeof(send_data);
    p_gatt_read->value = blib_malloc(p_gatt_read->length);
    if(NULL != p_gatt_read->value && p_gatt_read->length)
    {
        memcpy(p_gatt_read->value, send_data, p_gatt_read->length);
    }
}

static void ln_ble_gatt_write_req_cb(void *arg)
{
    ble_evt_gatt_write_req_t *p_gatt_write = (ble_evt_gatt_write_req_t *)arg;
    LOG(LOG_LVL_TRACE, "ln_ble_gatt_write conn_id=%d,handle=%d\r\n",p_gatt_write->conidx,p_gatt_write->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_write->value, p_gatt_write->length);
    uint16_t len = sizeof(ble_evt_gatt_write_req_t) +  p_gatt_write->length;
    ble_evt_gatt_write_req_t *data = blib_malloc(len);
	if (data == NULL) {
		LOG(LOG_LVL_ERROR, "gatt_write_req malloc fail\r\n");
		return;
	}
    memcpy(data, p_gatt_write, len);
    usr_queue_msg_send(BLE_MSG_GATT_WRITE_DATA, len, data);
}
#endif

#if (BLE_DATA_TRANS_CLIENT)
static void ln_ble_gatt_disc_svc_cb(void *arg)
{
    ble_evt_gatt_svr_disc_t *p_gatt_disc_svc = (ble_evt_gatt_svr_disc_t *)arg;
    LOG(LOG_LVL_TRACE, "ln_ble_gatt_disc_svc_cb conn_id=%d\r\n",p_gatt_disc_svc->conidx);
    for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
        if(!memcmp(g_user_cli_desc_tab[i].svc_uuid, p_gatt_disc_svc->uuid, p_gatt_disc_svc->uuid_len))
        {
            g_user_cli_desc_tab[i].con_idx      = p_gatt_disc_svc->conidx;
            g_user_cli_desc_tab[i].end_handle   = p_gatt_disc_svc->end_hdl;
            g_user_cli_desc_tab[i].start_handle = p_gatt_disc_svc->start_hdl;
            LOG(LOG_LVL_TRACE,"peer devide ble trans server start handle=%d\r\n", p_gatt_disc_svc->start_hdl);
            //hexdump(LOG_LVL_INFO, "[server uuid]", (void *)p_gatt_disc_svc->uuid, p_gatt_disc_svc->uuid_len);
        }
    }
}

static void ln_ble_gatt_read_rsp_cb(void *arg)
{
    ble_evt_gatt_read_rsp_t *p_gatt_read_rsp = (ble_evt_gatt_read_rsp_t *)arg;
    LOG(LOG_LVL_TRACE, "ln_ble_gatt_read_rsp_cb conn_id=%d,handle=%d\r\n",p_gatt_read_rsp->conidx,p_gatt_read_rsp->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_read_rsp->value, p_gatt_read_rsp->length);
}

static void ln_ble_gatt_notify_cb(void *arg)
{
    ble_evt_gatt_notify_t *p_gatt_ntf = (ble_evt_gatt_notify_t *)arg;
    LOG(LOG_LVL_TRACE, "ln_ble_gatt_notify conn_id=%d,handle=%d\r\n", p_gatt_ntf->conidx, p_gatt_ntf->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_ntf->value, p_gatt_ntf->length);
}

static void ln_ble_gatt_indicate_cb(void *arg)
{
    ble_evt_gatt_indicate_t *p_gatt_ind = (ble_evt_gatt_indicate_t *)arg;
    LOG(LOG_LVL_TRACE, "ln_ble_gatt_indicate conn_id=%d,handle=%d\r\n",p_gatt_ind->conidx, p_gatt_ind->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_ind->value, p_gatt_ind->length);
}
#endif

static void ln_ble_stack_init(void)
{
    //1.controller init
    ln_bd_addr_t bt_addr = {0};
#ifdef BLE_USE_STATIC_PUBLIC_ADDR
    ln_bd_addr_t *kv_addr = ln_kv_ble_pub_addr_get();
    memcpy(&bt_addr, kv_addr, sizeof(ln_bd_addr_t));
#else
    ln_bd_addr_t *kv_addr = ln_kv_ble_pub_addr_get();
    memcpy(&bt_addr, kv_addr, sizeof(ln_bd_addr_t));
    ln_bd_addr_t static_addr = {BLE_DEFAULT_PUBLIC_ADDR};

    if(!memcmp(kv_addr->addr, static_addr.addr, LN_BD_ADDR_LEN)) {
        ln_generate_random_mac(bt_addr.addr);
        bt_addr.addr[5] |= 0xC0; // This address is random generated, so assign 0x11 => Static Random Address
        ln_kv_ble_addr_store(bt_addr);
    }
#endif
    extern void rw_init(uint8_t mac[6]);
    rw_init(bt_addr.addr);

    //2.host init
    ln_gap_app_init();
    ln_gatt_app_init();

    //3.app component init
    ln_ble_conn_mgr_init();
    ln_ble_evt_mgr_init();
    ln_ble_smp_init();

    uint8_t role = ln_kv_ble_usr_data_get()->ble_role;

    if(role & BLE_ROLE_PERIPHERAL) {
        ln_ble_adv_mgr_init();
    }

    if((role & BLE_ROLE_CENTRAL)) {
        ln_ble_scan_mgr_init();
    }

#if (BLE_DATA_TRANS_SERVER)
    ln_ble_trans_svr_init();
#endif
#if (BLE_DATA_TRANS_CLIENT)
    ln_ble_trans_cli_init();
#endif

    ln_rw_app_task_init();

    //4.stack start
    ln_gap_reset();
    
    uint8_t *mac = bt_addr.addr;
    LOG(LOG_LVL_INFO, "+--------------- ble stack init ok -----------+\r\n");
    LOG(LOG_LVL_INFO, "|ble role : %-22d            |\r\n",  role);
    LOG(LOG_LVL_INFO, "|ble mac  : [%02X:%02X:%02X:%02X:%02X:%02X] %-13s |\r\n", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0], "");
    LOG(LOG_LVL_INFO, "+---------------------------------------------+\r\n");
}

static void ln_ble_app_init(void)
{
    //full stack init
    ln_ble_stack_init();

    uint8_t role = ln_kv_ble_usr_data_get()->ble_role;
    LOG(LOG_LVL_TRACE, "ble_app_init role=%d\r\n", role);

    ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_CONNECTED,    ln_ble_connect_cb);
    ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_DISCONNECTED, ln_ble_disconnect_cb);

    if(role & BLE_ROLE_PERIPHERAL)
    {
#if (BLE_DATA_TRANS_SERVER)
        for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
            ln_ble_trans_svr_add(&g_user_svc_desc_tab[i].desc);
        }
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_READ_REQ,  ln_ble_gatt_read_req_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_WRITE_REQ, ln_ble_gatt_write_req_cb);
#endif
        /*advertising activity init*/
        adv_param_t *adv_param = &le_adv_mgr_info_get()->adv_param;
#ifdef BLE_USE_LEGACY_ADV
        adv_param->adv_type = GAPM_ADV_TYPE_LEGACY;
        adv_param->adv_prop = GAPM_ADV_PROP_UNDIR_CONN_MASK;
        ln_ble_adv_actv_creat(adv_param);
        /*set advertising data*/
        app_set_adv_data();
        /*set scan respond data*/
        app_set_scan_resp_data();
#else
        adv_param->adv_type = GAPM_ADV_TYPE_EXTENDED;
        adv_param->adv_prop = GAPM_EXT_ADV_PROP_UNDIR_CONN_MASK;
        ln_ble_adv_actv_creat(adv_param);
        /*set extended advertising data*/
        app_set_ext_adv_data();
        /*set extended scan respond data*/
        //app_set_ext_scan_resp_data();
#endif
        /*start advertising*/
        ln_ble_adv_start();
    }

    if((role & BLE_ROLE_CENTRAL))
    {
#if (BLE_DATA_TRANS_CLIENT)
        //ln_ble_trans_cli_add();
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_DISC_SVC, ln_ble_gatt_disc_svc_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_READ_RSP, ln_ble_gatt_read_rsp_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_NOTIFY, ln_ble_gatt_notify_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_INDICATE, ln_ble_gatt_indicate_cb);
#endif
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_SCAN_REPORT, ln_ble_scan_report_cb);

        /*scan activity init*/
        ln_ble_scan_actv_creat();
        /*start scan*/
        ln_ble_scan_start(&le_scan_mgr_info_get()->scan_param);
        /*connect activity init*/
        ln_ble_init_actv_creat();
    }
}

void ble_app_task_entry(void *params)
{
    ble_usr_msg_t usr_msg;

    ln_kv_ble_app_init();

    usr_queue_creat();
    user_timer_creat();
    ln_ble_app_init();

    while(1)
    {
        if(OS_OK == usr_queue_msg_recv((void *)&usr_msg, OS_WAIT_FOREVER))
        {
            switch(usr_msg.id)
            {
                case BLE_MSG_CONN_IND:
                {
                    ble_evt_connected_t *p_param = (ble_evt_connected_t *)(usr_msg.msg);
                    ln_gatt_exc_mtu(p_param->conn_idx);

                    ln_gap_set_le_pkt_size_t pkt_size;
                    pkt_size.tx_octets = 251;
                    pkt_size.tx_time   = 2120;
                    ln_gap_set_le_pkt_size(p_param->conn_idx,  &pkt_size);

                    if(!p_param->role)
                        OS_MsDelay(100);
                    else
                        OS_MsDelay(1000);
                    ln_ble_conn_param_t conn_param;
                    conn_param.intv_min = 80;
                    conn_param.intv_max = 90;
                    conn_param.latency = 0;
                    conn_param.time_out = 3000;
                    ln_ble_conn_param_update(p_param->conn_idx, &conn_param);
                }
                    break;

                case BLE_MSG_DISCONN_IND:
                    break;

                case BLE_MSG_SCAN_REPORT:
                    break;

                case BLE_MSG_GATT_READ_REQ:
                    break;

                case BLE_MSG_GATT_WRITE_DATA:
                {
#if (BLE_DATA_TRANS_SERVER)
                    ble_evt_gatt_write_req_t *p_param = (ble_evt_gatt_write_req_t *)(usr_msg.msg);
                    for(int i = 0;i<DATA_TRANS_SVR_MAX;i++) {
                        uint8_t ccc_handle = g_user_svc_desc_tab[i].desc.start_handle+DATA_TRANS_DECL_CHAR_TX_CCC;
                        if(ccc_handle == p_param->handle) {
                            g_user_svc_desc_tab[i].ccc = *((uint16_t *)p_param->value);
                        }
                    }
#endif
                }
                    break;

                case BLE_MSG_GATT_READ_RSP:
                break;

                case BLE_MSG_GATT_NOTIFY:
                break;

                case BLE_MSG_GATT_INDICATION:
                break;

                default:
                    break;
            }
            blib_free(usr_msg.msg);
        }
    }
}

void ble_creat_usr_app_task(void)
{
    if(OS_OK != OS_ThreadCreate(&ble_g_usr_app_thread, "BleUsrAPP", ble_app_task_entry, NULL, OS_PRIORITY_BELOW_NORMAL, BLE_USR_APP_TASK_STACK_SIZE)) 
    {
        LN_ASSERT(1);
    }
}

