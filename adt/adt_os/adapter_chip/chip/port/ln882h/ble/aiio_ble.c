#include "aiio_ble.h"

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

#include "aiio_log.h"

#define DATA_TRANS_SVR_MAX              (2)

static struct user_svc_desc {
    ln_trans_svr_desc_t desc;
    uint8_t ccc;//character client config
};

static volatile uint8_t ble_conn_flag;

static ln_attm_desc_t data_trans_1st_atts_db[] = {
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
        // .uuid = DATA_TRANS_1ST_RX_UUID,
        .perm = PERM_MASK_WRITE_REQ | PERM_MASK_WRITE_COMMAND ,
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
        // .uuid       = DATA_TRANS_1ST_TX_UUID,
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

static ln_attm_desc_t data_trans_2nd_atts_db[] = {
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
        // .uuid = DATA_TRANS_2ND_RX_UUID,
        .perm = PERM_MASK_WRITE_REQ | PERM_MASK_WRITE_COMMAND ,
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
        // .uuid       = DATA_TRANS_2ND_TX_UUID,
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

static struct user_svc_desc g_user_svc_desc_tab[DATA_TRANS_SVR_MAX];

static struct user_svc_desc g_user_svc_desc_tab[DATA_TRANS_SVR_MAX] = {
    {
        .desc = {
            .start_handle   = LN_ATT_INVALID_HANDLE,
            // .svr_uuid_len   = 2,
            // .svr_uuid       = DATA_TRANS_1ST_SVR_UUID,
            // .att_count      = sizeof(data_trans_1st_atts_db) / sizeof(data_trans_1st_atts_db[0]),
            // .att_desc       = &data_trans_1st_atts_db,
        },
        .ccc = 0,
    },
    {
        .desc = {
            .start_handle   = LN_ATT_INVALID_HANDLE,
            // .svr_uuid_len   = 2,
            // .svr_uuid       = DATA_TRANS_2ND_SVR_UUID,
            // .att_count      = sizeof(data_trans_2nd_atts_db) / sizeof(data_trans_2nd_atts_db[0]),
            // .att_desc       = &data_trans_2nd_atts_db,
        },
        .ccc = 0
    }
};

static aiio_ble_config_t ble_config_s;

aiio_ble_cb_t ble_cb;

aiio_err_t aiio_ble_register_event_cb(aiio_ble_cb_t cb)
{
    ble_cb = cb;

    return AIIO_OK;
}

static void ble_reverse_byte(uint8_t *arr, uint32_t size)
{
    uint8_t i, tmp;

    for (i = 0; i < size / 2; i++) {
        tmp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = tmp;
    }
}

static void ln_ble_gatt_read_req_cb(void *arg)
{
    ble_evt_gatt_read_req_t *p_gatt_read = (ble_evt_gatt_read_req_t *)arg;
    uint8_t send_data[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};

    aiio_log_v("ln_ble_gatt_read conn_id=%d,handle=%d",p_gatt_read->conidx, p_gatt_read->handle);

    p_gatt_read->length = sizeof(send_data);
    p_gatt_read->value = blib_malloc(p_gatt_read->length);
    if(NULL != p_gatt_read->value && p_gatt_read->length)
    {
        memcpy(p_gatt_read->value, send_data, p_gatt_read->length);
    }
}

static void ln_ble_gatt_write_req_cb(void *arg)
{
    uint8_t ccc_handle;
    ble_evt_gatt_write_req_t *p_gatt_write = (ble_evt_gatt_write_req_t *)arg;
    aiio_log_v( "ln_ble_gatt_write conn_id=%d,handle=%d\r\n", p_gatt_write->conidx, p_gatt_write->handle);
    for(int i = 0; i < DATA_TRANS_SVR_MAX; i++) {
        ccc_handle = g_user_svc_desc_tab[i].desc.start_handle + DATA_TRANS_DECL_CHAR_TX_CCC;
        if(ccc_handle == p_gatt_write->handle) {
            g_user_svc_desc_tab[i].ccc = *((uint16_t *)p_gatt_write->value);
        }
    }

    if (ble_cb) {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DATA;
        evt.data.handle = ccc_handle;
        evt.data.type = AIIO_BLE_WRITE;
        evt.data.conn = p_gatt_write->conidx;
        evt.data.data = p_gatt_write->value;
        evt.data.length = p_gatt_write->length;
        ble_cb(&evt);
    }
}


static void ln_ble_connect_cb(void *arg)
{
    ble_evt_connected_t * evt_conn = (ble_evt_connected_t *)arg;

    uint8_t conn_idx = evt_conn->conn_idx;

    ln_gatt_exc_mtu(evt_conn->conn_idx);

    ln_gap_set_le_pkt_size_t pkt_size;
    pkt_size.tx_octets = 251;
    pkt_size.tx_time   = 2120;
    ln_gap_set_le_pkt_size(evt_conn->conn_idx, &pkt_size);

    if(!evt_conn->role)
        OS_MsDelay(100);
    else
        OS_MsDelay(1000);
    ln_ble_conn_param_t conn_param;
    conn_param.intv_min = ble_config_s.conn_intv.min_interval;
    conn_param.intv_max = ble_config_s.conn_intv.max_interval;
    conn_param.latency = ble_config_s.conn_intv.latency;
    conn_param.time_out = ble_config_s.conn_intv.timeout;
    ln_ble_conn_param_update(evt_conn->conn_idx, &conn_param);
    
    if (ble_cb) {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_CONNECT;
        evt.connect.conn =  conn_idx;
        ble_cb(&evt);
    }

    ble_conn_flag = 1;
}


static void ln_ble_disconnect_cb(void *arg)
{
    ble_evt_disconnected_t *evt_disconn = (ble_evt_disconnected_t *)arg;

    for(int i = 0; i < DATA_TRANS_SVR_MAX; i++)
        g_user_svc_desc_tab[i].ccc = 0;

    if (ble_cb) {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DISCONNECT;
        evt.disconnect.conn = evt_disconn;
        // evt.disconnect.reason = reason;
        ble_cb(&evt);
    }
}


// static void ln_ble_gatt_disc_svc_cb(void *arg)
// {
//     ble_evt_gatt_svr_disc_t *p_gatt_disc_svc = (ble_evt_gatt_svr_disc_t *)arg;
//     aiio_log_v( "ln_ble_gatt_disc_svc_cb conn_id=%d\r\n",p_gatt_disc_svc->conidx);
//     for(int i = 0;i < DATA_TRANS_SVR_MAX; i++) {
//         if(!memcmp(g_user_cli_desc_tab[i].svc_uuid, p_gatt_disc_svc->uuid, p_gatt_disc_svc->uuid_len))
//         {
//             g_user_cli_desc_tab[i].con_idx      = p_gatt_disc_svc->conidx;
//             g_user_cli_desc_tab[i].end_handle   = p_gatt_disc_svc->end_hdl;
//             g_user_cli_desc_tab[i].start_handle = p_gatt_disc_svc->start_hdl;
//             aiio_log_v( "peer devide ble trans server start handle=%d\r\n", p_gatt_disc_svc->start_hdl);
//             //hexdump(LOG_LVL_INFO, "[server uuid]", (void *)p_gatt_disc_svc->uuid, p_gatt_disc_svc->uuid_len);
//         }
//     }
// }


static void ln_ble_gatt_read_rsp_cb(void *arg)
{
    ble_evt_gatt_read_rsp_t *p_gatt_read_rsp = (ble_evt_gatt_read_rsp_t *)arg;
    aiio_log_v(  "ln_ble_gatt_read_rsp_cb conn_id=%d,handle=%d\r\n", p_gatt_read_rsp->conidx, p_gatt_read_rsp->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_read_rsp->value, p_gatt_read_rsp->length);
}

static void ln_ble_gatt_notify_cb(void *arg)
{
    ble_evt_gatt_notify_t *p_gatt_ntf = (ble_evt_gatt_notify_t *)arg;
    aiio_log_v( "ln_ble_gatt_notify conn_id=%d,handle=%d\r\n", p_gatt_ntf->conidx, p_gatt_ntf->handle);
    // hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_ntf->value, p_gatt_ntf->length);

    if (ble_cb) {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DATA;
        evt.data.handle = p_gatt_ntf->handle;
        evt.data.type = AIIO_BLE_NOTIFY;
        evt.data.conn = p_gatt_ntf->conidx;
        evt.data.data = p_gatt_ntf->value;
        evt.data.length = p_gatt_ntf->length;
        ble_cb(&evt);
    }

}

static void ln_ble_gatt_indicate_cb(void *arg)
{
    ble_evt_gatt_indicate_t *p_gatt_ind = (ble_evt_gatt_indicate_t *)arg;
    aiio_log_v(  "ln_ble_gatt_indicate conn_id=%d,handle=%d\r\n", p_gatt_ind->conidx, p_gatt_ind->handle);
    hexdump(LOG_LVL_INFO, "[recv data]", (void *)p_gatt_ind->value, p_gatt_ind->length);
}


static void ln_ble_scan_report_cb(void *arg)
{
    ble_evt_scan_report_t *p_scan_rpt = (ble_evt_scan_report_t *)arg;
    adv_data_item_t *name_item = NULL;

    aiio_log_v(  "\r\n--------------adv_report-------------\r\n");
    aiio_log_v(  "rssi=%d, tx_pwr=%d\r\n", p_scan_rpt->rssi, p_scan_rpt->tx_pwr);
    aiio_log_v( "addr_type=%d, addr=0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\r\n",
            p_scan_rpt->trans_addr_type, p_scan_rpt->trans_addr[0], p_scan_rpt->trans_addr[1],
            p_scan_rpt->trans_addr[2], p_scan_rpt->trans_addr[3], p_scan_rpt->trans_addr[4], p_scan_rpt->trans_addr[5]);

    if (ble_cb) {
        aiio_ble_evt_t evt;

        memset(&evt, 0, sizeof evt);
        evt.type = AIIO_BLE_EVT_SCAN;
        evt.scan.rssi = p_scan_rpt->rssi;
        memcpy(evt.scan.mac, p_scan_rpt->trans_addr, 6);
        // ble_reverse_byte(evt.scan.mac, 6);

        evt.scan.ad_data = p_scan_rpt->data;
        evt.scan.ad_len = p_scan_rpt->length;

        ble_cb(&evt);
    }
}


static int ble_salve_init(void)
{
    ln_ble_adv_mgr_init();

    ln_ble_trans_svr_init();

    return AIIO_OK;
}

static int ble_master_init(void)
{
    ln_ble_scan_mgr_init();
    return AIIO_OK;
}

aiio_err_t aiio_ble_init(aiio_ble_mode_t mode, const aiio_ble_config_t *config)
{
    //1.controller init
    ln_bd_addr_t bt_addr = {0};

    {
        static uint32_t aiio_ble_init_flag;
        if (aiio_ble_init_flag) {
            return AIIO_OK;
        }
        aiio_ble_init_flag = 1;
    }

    // ln_ble_mac_get(bt_addr.addr);
    sysparam_sta_mac_get(bt_addr.addr);

    extern void rw_init(uint8_t mac[6]);
    rw_init(bt_addr.addr);

    //2.host init
    ln_gap_app_init();
    ln_gatt_app_init();

    //3.app component init
    ln_ble_conn_mgr_init();
    ln_ble_evt_mgr_init();
    ln_ble_smp_init();


    // if (mode == AIIO_BLE_MODE_PERIPHERAL) {
        ble_salve_init();
    // } else if (mode == AIIO_BLE_MODE_CENTRAL) {
        ble_master_init();
    // }

    ln_rw_app_task_init();

    //4.stack start
    ln_gap_reset();

    ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_CONNECTED,    ln_ble_connect_cb);
    ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_DISCONNECTED, ln_ble_disconnect_cb);

    // if (mode == AIIO_BLE_MODE_PERIPHERAL)
    // {
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_READ_REQ,  ln_ble_gatt_read_req_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_WRITE_REQ, ln_ble_gatt_write_req_cb);
    // }
    // else if(mode==AIIO_BLE_MODE_CENTRAL)
    // {
        //ln_ble_trans_cli_add();
        // ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_DISC_SVC, ln_ble_gatt_disc_svc_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_READ_RSP, ln_ble_gatt_read_rsp_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_NOTIFY, ln_ble_gatt_notify_cb);
        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_GATT_INDICATE, ln_ble_gatt_indicate_cb);

        ln_ble_evt_mgr_reg_evt(BLE_EVT_ID_SCAN_REPORT, ln_ble_scan_report_cb);

        /*scan activity init*/
        ln_ble_scan_actv_creat();

        /*connect activity init*/
        ln_ble_init_actv_creat();
    // }

    memcpy(&ble_config_s, config, sizeof ble_config_s);
    
    return AIIO_OK;
}

aiio_err_t aiio_ble_deinit(void)
{
    ln_ble_adv_stop();

    return AIIO_OK;
}

static int ble_notify_data(void *conn, void *char_val, uint16_t *data, uint16_t length)
{
    uint16_t mtu;
    ln_trans_svr_send_t svc_send_cmd;

    svc_send_cmd.conn_idx = conn;
    svc_send_cmd.data = data;
    svc_send_cmd.len = length;
    svc_send_cmd.hdl = char_val + DATA_TRANS_DECL_CHAR_TX_VAL;

    ln_ble_trans_svr_ntf(&svc_send_cmd);

    return AIIO_OK;
}


static int ble_write_norsp_data(void *conn, void *char_val, uint8_t *data, uint16_t length)
{
    ln_gatt_write_cmd_t p_param;
    p_param.operation = GATTC_WRITE_NO_RESPONSE;
    p_param.auto_execute = true;
    p_param.handle = char_val + DATA_TRANS_DECL_CHAR_RX_VAL;
    p_param.offset = 0;
    p_param.length = length;
    p_param.value = data;

    ln_gatt_write(conn, &p_param);
    
    return AIIO_OK;
}

aiio_err_t aiio_ble_send(const aiio_ble_send_t *data)
{
    switch (data->type) {
        case AIIO_BLE_NOTIFY:
            ble_notify_data(data->conn, data->handle, data->data, data->length);
            break;
        case AIIO_BLE_WRITE_WITHOUT_RESP:
            ble_write_norsp_data(data->conn, data->handle, data->data, data->length);
            break;
        default:
            return AIIO_ERROR;
    }

    return AIIO_OK;
}


adv_data_item_t *ln_ble_adv_data_find_item_by_type(uint8_t type, uint8_t *data, uint8_t len, int* data_offset)
{
    adv_data_item_t *item = NULL;
    int offset = 0;
    *data_offset=offset;
    do {
        item = (adv_data_item_t *)(data + offset);
        //LOG(LOG_LVL_INFO, "ln_ble_scan_report_data_parser item->len=%d,item->type=%d\r\n", item->len, item->type);
        if(type == item->type)
        {
            return item;
        }
        offset += item->len + 1;
        *data_offset=offset;
    } while(offset < len);

    return NULL;
}


aiio_err_t aiio_ble_adv_start(aiio_ble_adv_param_t *param,
                              const uint8_t *ad, uint16_t ad_len,
                              const uint8_t *sd, uint16_t sd_len)
{
    int err;
    /*advertising activity init*/
    adv_param_t *adv_param = &le_adv_mgr_info_get()->adv_param;
    // adv_param_t *adv_param;
    adv_param->adv_intv_max=param->interval_max;
    adv_param->adv_intv_min=param->interval_min;
    adv_param->adv_type = GAPM_ADV_TYPE_LEGACY;
    adv_param->adv_prop = GAPM_ADV_PROP_UNDIR_CONN_MASK;
    if (param->conn_mode == AIIO_BLE_CONN_MODE_NON) {
        adv_param->adv_prop = GAPM_ADV_PROP_NON_CONN_NON_SCAN_MASK;
    }
    err=ln_ble_adv_actv_creat(adv_param);
    if(err!=AIIO_OK)
    {
        return AIIO_ERROR;
    }

    /*set advertising data*/
    ln_adv_data_t adv_data_param;

    adv_data_item_t *item = NULL;
    int data_offset=0;
    item = ln_ble_adv_data_find_item_by_type(GAP_AD_TYPE_FLAGS,ad, ad_len,&data_offset);        //GAP_AD_TYPE_FLAGS 字段不能设置
    if(item)
    {
        adv_data_param.length = ad_len-3;
        if(data_offset==0)
        {
            adv_data_param.data = ad+3;
        }
        else
        {
            uint8_t adv_data[255]={0};
            memcpy(adv_data,ad,data_offset);
            memcpy(adv_data+data_offset,ad+(data_offset+3),adv_data_param.length-data_offset);
            adv_data_param.data = adv_data;
        }
    }
    else
    {
        adv_data_param.length = ad_len;
        adv_data_param.data = ad;
    }

    err=ln_ble_adv_data_set(&adv_data_param);
    if(err!=AIIO_OK)
    {
        return AIIO_ERROR; 
    }

    if (sd_len) {
        adv_data_param.data = sd;
        adv_data_param.length = sd_len;
        err = ln_ble_adv_scan_rsp_data_set(&adv_data_param);
        if (err != AIIO_OK) {
            return AIIO_ERROR;
        }
    }

    /*start advertising*/
    ln_ble_adv_start();

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_mac(uint8_t *mac)
{
    ln_bd_addr_t bt_addr = {0};

    int8_t err=ln_ble_mac_get(bt_addr.addr);

    memcpy(mac, bt_addr.addr, 6);
    ble_reverse_byte(mac, 6);

    return err;
}

aiio_err_t aiio_ble_set_mac(const uint8_t *mac)
{
    int err=ln_ble_mac_set((const uint8_t *)mac);

    return err;
}

aiio_err_t aiio_ble_set_name(const char *name)
{
    uint8_t adv_data_len = 0;
    uint8_t adv_data[ADV_DATA_LEGACY_MAX] = {0};
    ln_adv_data_t adv_data_param;

    return 0;

    uint16_t name_len = 0;

    if(name_len)
    {
        if(name_len > (ADV_DATA_LEGACY_MAX - 2))
            name_len = ADV_DATA_LEGACY_MAX - 2;
        /*add item ble_name*/
        adv_data[0] = name_len + 1;
        adv_data[1] = GAP_AD_TYPE_COMPLETE_NAME;  //adv type :local name
        adv_data_len += 2;
        memcpy(&adv_data[2], name, name_len);
        adv_data_len += name_len;
    }

    adv_data_param.length = adv_data_len;
    adv_data_param.data = adv_data;
    int err=ln_ble_adv_data_set(&adv_data_param);

    return err;
}

 aiio_err_t aiio_ble_adv_stop(void)
 {
    ln_ble_adv_stop();

    return AIIO_OK;
 }

aiio_err_t aiio_ble_get_limit_power(int8_t *min, int8_t *max)
{
    *min = 0;
    *max = 20;

    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_req(aiio_ble_conn_t conn, uint16_t mtu)
{
    ln_gatt_exc_mtu(conn);
    
    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_get(aiio_ble_conn_t conn, uint16_t *mtu)
{
    *mtu=(uint16_t)gattc_get_mtu(conn);
    
    return AIIO_OK;
}

aiio_err_t aiio_ble_restart_advertising(void)
{
    ln_ble_adv_start();
    
    return AIIO_OK;
}

void ble_uuid_convert(uint8_t *dest, aiio_ble_uuid_t *src)
{
    switch (src->type) {
        case AIIO_BLE_UUID_TYPE_16:
            memcpy(dest, ((aiio_ble_uuid_16_t *)src)->val, 2);
            ble_reverse_byte(dest, 2);
            break;
        case AIIO_BLE_UUID_TYPE_32:
            memcpy(dest, ((aiio_ble_uuid_16_t *)src)->val, 4);
            ble_reverse_byte(dest, 4);
            break;
        case AIIO_BLE_UUID_TYPE_128:
            memcpy(dest, ((aiio_ble_uuid_16_t *)src)->val, 16);
            ble_reverse_byte(dest, 16);
            break;
        default:
            return ;
    }
}

aiio_err_t ble_gatts_add_default_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    ble_uuid_convert(g_user_svc_desc_tab[0].desc.svr_uuid, cfg->server_uuid[0]);
    ble_uuid_convert(data_trans_1st_atts_db[DATA_TRANS_DECL_CHAR_RX_VAL].uuid, cfg->rx_char_uuid[0]);
    ble_uuid_convert(data_trans_1st_atts_db[DATA_TRANS_DECL_CHAR_TX_VAL].uuid, cfg->tx_char_uuid[0]);

    ble_uuid_convert(g_user_svc_desc_tab[1].desc.svr_uuid, cfg->server_uuid[1]);
    ble_uuid_convert(data_trans_2nd_atts_db[DATA_TRANS_DECL_CHAR_RX_VAL].uuid, cfg->rx_char_uuid[1]);
    ble_uuid_convert(data_trans_2nd_atts_db[DATA_TRANS_DECL_CHAR_TX_VAL].uuid, cfg->tx_char_uuid[1]);

    g_user_svc_desc_tab[0].desc.att_count=sizeof(data_trans_1st_atts_db) / sizeof(data_trans_1st_atts_db[0]);
    g_user_svc_desc_tab[0].desc.att_desc=&data_trans_1st_atts_db;
    g_user_svc_desc_tab[1].desc.att_count=sizeof(data_trans_2nd_atts_db) / sizeof(data_trans_2nd_atts_db[0]);
    g_user_svc_desc_tab[1].desc.att_desc=&data_trans_2nd_atts_db;
    switch (cfg->server_uuid[0]->type)
    {
        case AIIO_BLE_UUID_TYPE_16:
            g_user_svc_desc_tab[0].desc.svr_uuid_len=2;
            break;
        case AIIO_BLE_UUID_TYPE_32:
            g_user_svc_desc_tab[0].desc.svr_uuid_len=4;
            break;
        case AIIO_BLE_UUID_TYPE_128:
            g_user_svc_desc_tab[0].desc.svr_uuid_len=16;
            break;
        default:
            return ;
    }
    switch (cfg->server_uuid[1]->type)
    {
        case AIIO_BLE_UUID_TYPE_16:
            g_user_svc_desc_tab[1].desc.svr_uuid_len=2;
            break;
        case AIIO_BLE_UUID_TYPE_32:
            g_user_svc_desc_tab[1].desc.svr_uuid_len=4;
            break;
        case AIIO_BLE_UUID_TYPE_128:
            g_user_svc_desc_tab[1].desc.svr_uuid_len=16;
            break;
        default:
            return ;
    }

    for(int i = 0; i < DATA_TRANS_SVR_MAX; i++)
    {
        int err=ln_ble_trans_svr_add(&g_user_svc_desc_tab[i].desc);
        if(err!=AIIO_OK)
        {
            return AIIO_ERROR;
        }
    }

    handle->tx_char_handle[0] = g_user_svc_desc_tab[0].desc.start_handle;
    handle->rx_char_handle[0] = g_user_svc_desc_tab[0].desc.start_handle;

    handle->tx_char_handle[1] = g_user_svc_desc_tab[1].desc.start_handle;
    handle->rx_char_handle[1] = g_user_svc_desc_tab[1].desc.start_handle;


    return AIIO_OK;
}


static void ble_scan_start(le_scan_parameters_t *param,aiio_ble_scan_param_t *cfg)
{
    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD, TASK_GAPM, TASK_APP, gapm_activity_start_cmd);
    p_cmd->operation                                        = GAPM_START_ACTIVITY;
    p_cmd->actv_idx                                         = ble_scan_mgr.scan_actv_idx;
    p_cmd->u_param.scan_param.type                          = param->type;
    p_cmd->u_param.scan_param.prop                          = param->prop;
    p_cmd->u_param.scan_param.dup_filt_pol                  = param->dup_filt_pol;
    p_cmd->u_param.scan_param.scan_param_1m.scan_intv       = cfg->interval; //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.scan_param_1m.scan_wd         = cfg->window;   //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.duration                      = 0;
    p_cmd->u_param.scan_param.scan_param_coded.scan_intv    = cfg->interval; //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.scan_param_coded.scan_wd      = cfg->window;   //(T = N * 0.625 ms);
    p_cmd->u_param.scan_param.period                        = 2;

    ln_ke_msg_send(p_cmd);

    ble_scan_mgr.scan_param.type            = param->type;
    ble_scan_mgr.scan_param.prop            = param->prop;
    ble_scan_mgr.scan_param.dup_filt_pol    = param->dup_filt_pol;
    ble_scan_mgr.scan_param.scan_intv       = cfg->interval;
    ble_scan_mgr.scan_param.scan_wd         = cfg->window;
    ble_scan_mgr.state                      = LE_SCAN_STATE_STARTING;
}

aiio_err_t aiio_ble_scan(const aiio_ble_scan_param_t *cfg)
{
    ble_scan_start(&le_scan_mgr_info_get()->scan_param,cfg);

    return AIIO_OK;
}

aiio_err_t aiio_ble_scan_stop(void)
{
    ln_ble_scan_stop();

    return AIIO_OK;
}

aiio_err_t aiio_ble_connect(uint8_t addr_type, uint8_t *addr, uint32_t timeout)
{
    ble_conn_flag = 0;

    struct gapm_activity_start_cmd *p_cmd = KE_MSG_ALLOC(GAPM_ACTIVITY_START_CMD,
                                        TASK_GAPM, TASK_APP,
                                        gapm_activity_start_cmd);
    p_cmd->operation = GAPM_START_ACTIVITY;
    p_cmd->actv_idx = ble_conn_mgr.init_actv_idx;
    p_cmd->u_param.init_param.type = GAPM_INIT_TYPE_DIRECT_CONN_EST;//GAPM_INIT_TYPE_DIRECT_CONN_EST;;
    p_cmd->u_param.init_param.prop = GAPM_INIT_PROP_1M_BIT | GAPM_INIT_PROP_CODED_BIT;
    p_cmd->u_param.init_param.conn_to = 0;
    p_cmd->u_param.init_param.scan_param_1m.scan_intv = 16;
    p_cmd->u_param.init_param.scan_param_1m.scan_wd = 16;
    p_cmd->u_param.init_param.scan_param_coded.scan_intv =16;
    p_cmd->u_param.init_param.scan_param_coded.scan_wd = 16;

    p_cmd->u_param.init_param.conn_param_1m.conn_intv_min = 80;
    p_cmd->u_param.init_param.conn_param_1m.conn_intv_max = 80;
    p_cmd->u_param.init_param.conn_param_1m.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_1m.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_1m.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_1m.supervision_to= 500;

    p_cmd->u_param.init_param.conn_param_2m.conn_intv_min = 80;
    p_cmd->u_param.init_param.conn_param_2m.conn_intv_max = 80;
    p_cmd->u_param.init_param.conn_param_2m.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_2m.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_2m.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_2m.supervision_to= 500;

    p_cmd->u_param.init_param.conn_param_coded.conn_intv_min = ble_config_s.conn_intv.min_interval;
    p_cmd->u_param.init_param.conn_param_coded.conn_intv_max = ble_config_s.conn_intv.max_interval;
    p_cmd->u_param.init_param.conn_param_coded.conn_latency =ble_config_s.conn_intv.latency;
    p_cmd->u_param.init_param.conn_param_coded.ce_len_min = 0;
    p_cmd->u_param.init_param.conn_param_coded.ce_len_max = 0xFFFF;
    p_cmd->u_param.init_param.conn_param_coded.supervision_to= ble_config_s.conn_intv.timeout;

    p_cmd->u_param.init_param.conn_param_coded.conn_intv_min = 200;
    p_cmd->u_param.init_param.conn_param_coded.conn_intv_max = 240;
    p_cmd->u_param.init_param.conn_param_coded.conn_latency = 0;
    p_cmd->u_param.init_param.conn_param_coded.supervision_to= 450;

    p_cmd->u_param.init_param.peer_addr.addr_type = GAPM_STATIC_ADDR;
    memcpy(&p_cmd->u_param.init_param.peer_addr.addr, addr, GAP_BD_ADDR_LEN);
    ble_reverse_byte(&p_cmd->u_param.init_param.peer_addr.addr, 6);

    ln_ke_msg_send(p_cmd);


    timeout /= 10;
    while (ble_conn_flag == 0) {
        timeout--;
        if (timeout == 0) {
            aiio_ble_disconnect(p_cmd->actv_idx);
            return AIIO_ERROR;
        }

        aiio_os_tick_dealy(aiio_os_ms2tick(10));
    }
    
    return AIIO_OK;
}

aiio_err_t aiio_ble_disconnect(aiio_ble_conn_t conn)
{
    ln_ble_disc_req(conn);

    return AIIO_OK;
}

// aiio_err_t aiio_ble_discover(const aiio_ble_disc_param_t *param, uint16_t timeout)
// {

//     return AIIO_OK;
// }


