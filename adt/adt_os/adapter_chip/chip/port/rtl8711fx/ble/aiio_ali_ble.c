#include "aiio_ali_ble.h"
#include "aiio_ble.h"
#include "aiio_os_port.h"
#include "aiio_log.h"
#include <string.h>
#include <stdio.h>
#include <osif.h>
#include <bt_api_config.h>
#include <rtk_bt_def.h>
#include <rtk_bt_common.h>
#include <rtk_bt_device.h>
#include <rtk_bt_vendor.h>
#include <rtk_bt_le_gap.h>
#include <rtk_bt_att_defs.h>
#include <rtk_bt_gatts.h>

// 服务UUID定义
#define AIIO_ALI_BLE_SERVICE_UUID          0xFEB3
#define AIIO_ALI_BLE_RC_CHAR_UUID          0xFED4
#define AIIO_ALI_BLE_WC_CHAR_UUID          0xFED5
#define AIIO_ALI_BLE_IC_CHAR_UUID          0xFED6
#define AIIO_ALI_BLE_WWNRC_CHAR_UUID       0xFED7
#define AIIO_ALI_BLE_NC_CHAR_UUID          0xFED8

// 服务索引定义
#define SERVICE_INDEX                     0
#define RC_CHAR_INDEX                     2
#define WC_CHAR_INDEX                     4
#define IC_CHAR_INDEX                     6
#define WWNRC_CHAR_INDEX                  9
#define NC_CHAR_INDEX                     11
#define IC_CCCD_INDEX                     7
#define NC_CCCD_INDEX                     12

typedef struct {
    uint8_t srv_id;
    uint8_t attr_index;
} ble_gatts_attr_t;

static aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = 60,
    .interval_max = 60,
};

static rtk_bt_gatt_attr_t ali_ble_attrs[] = {
    // 主服务声明
    RTK_BT_GATT_PRIMARY_SERVICE(BT_UUID_DECLARE_16(AIIO_ALI_BLE_SERVICE_UUID)),
    
    // 特征1: 只读特征 (RC)
    RTK_BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(AIIO_ALI_BLE_RC_CHAR_UUID),
                               RTK_BT_GATT_CHRC_READ,
                               RTK_BT_GATT_PERM_READ),
                               
    // 特征2: 读写特征 (WC)
    RTK_BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(AIIO_ALI_BLE_WC_CHAR_UUID),
                               RTK_BT_GATT_CHRC_READ | RTK_BT_GATT_CHRC_WRITE,
                               RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),
                               
    // 特征3: 指示特征 (IC)
    RTK_BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(AIIO_ALI_BLE_IC_CHAR_UUID),
                               RTK_BT_GATT_CHRC_READ | RTK_BT_GATT_CHRC_INDICATE,
                               RTK_BT_GATT_PERM_READ),
    RTK_BT_GATT_CCC(RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),
    
    // 特征4: 无响应写特征 (WWNRC)
    RTK_BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(AIIO_ALI_BLE_WWNRC_CHAR_UUID),
                               RTK_BT_GATT_CHRC_READ | RTK_BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),
                               
    // 特征5: 通知特征 (NC)
    RTK_BT_GATT_CHARACTERISTIC(BT_UUID_DECLARE_16(AIIO_ALI_BLE_NC_CHAR_UUID),
                               RTK_BT_GATT_CHRC_READ | RTK_BT_GATT_CHRC_NOTIFY,
                               RTK_BT_GATT_PERM_READ),
    RTK_BT_GATT_CCC(RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),
};

static struct rtk_bt_gatt_service ali_ble_service = {
    .attrs = ali_ble_attrs,
    .attr_count = sizeof(ali_ble_attrs) / sizeof(ali_ble_attrs[0]),
    .app_id = SERVICE_INDEX,
    .type = GATT_SERVICE_OVER_BLE,
};

static ali_aiio_ble_default_handle_t srv_handle;
static ble_gatts_attr_t ble_def_attr_rc;
static ble_gatts_attr_t ble_def_attr_wc;
static ble_gatts_attr_t ble_def_attr_ic;
static ble_gatts_attr_t ble_def_attr_wwnrc;
static ble_gatts_attr_t ble_def_attr_nc;

static ais_bt_init_t *bt_init_info = NULL;
static void (*g_indication_txdone)(uint8_t res);
static void *ble_conn = NULL;
static aiio_ble_cb_t ble_cb;

// 蓝牙事件回调
static rtk_bt_evt_cb_ret_t ble_gatts_app_callback(uint8_t event, void *data, uint32_t len) {
    (void)len;
    
    switch (event) {
    case RTK_BT_GATTS_EVT_WRITE_IND: {
        rtk_bt_gatts_write_ind_t *p_write_ind = (rtk_bt_gatts_write_ind_t *)data;
        aiio_log_i("GATTS_EVT_WRITE_IND: %d\r\n", p_write_ind->index);
        // 处理特征写入
        if (p_write_ind->index == WC_CHAR_INDEX) {
            aiio_log_i("WC_CHAR_INDEX: %d\r\n", p_write_ind->len);
            if (bt_init_info && bt_init_info->wc.on_write) {
                bt_init_info->wc.on_write(p_write_ind->value, p_write_ind->len);
            }
        }
        
        // 发送响应
        rtk_bt_gatts_write_resp_param_t write_resp = {
            .app_id = p_write_ind->app_id,
            .conn_handle = p_write_ind->conn_handle,
            .cid = p_write_ind->cid,
            .index = p_write_ind->index,
            .type = p_write_ind->type,
            // .err_code = RTK_BT_ATT_ERR_NONE,
        };
        rtk_bt_gatts_write_resp(&write_resp);
        break;
    }
    
    case RTK_BT_GATTS_EVT_CCCD_IND: {
        rtk_bt_gatts_cccd_ind_t *p_cccd_ind = (rtk_bt_gatts_cccd_ind_t *)data;
        ais_ccc_value_t val;
        
        if (p_cccd_ind->index == IC_CCCD_INDEX) {
            val = AIS_CCC_VALUE_INDICATE;
            if (bt_init_info && bt_init_info->ic.on_ccc_change) {
                bt_init_info->ic.on_ccc_change(val);
            }
        } 
        else if (p_cccd_ind->index == NC_CCCD_INDEX) {
            val = AIS_CCC_VALUE_NOTIFY;
            if (bt_init_info && bt_init_info->nc.on_ccc_change) {
                bt_init_info->nc.on_ccc_change(val);
            }
        }
        break;
    }
    
    case RTK_BT_GATTS_EVT_INDICATE_COMPLETE_IND: {
        rtk_bt_gatts_ntf_and_ind_ind_t *p_ind = (rtk_bt_gatts_ntf_and_ind_ind_t *)data;
        if (g_indication_txdone) {
            g_indication_txdone(0);
        }
        break;
    }
    
    default:
        break;
    }
    
    return RTK_BT_EVT_CB_OK;
}

// GAP事件回调
static rtk_bt_evt_cb_ret_t ble_gap_app_callback(uint8_t evt_code, void *param, uint32_t len) {
    (void)len;
    
    switch (evt_code) {
    case RTK_BT_LE_GAP_EVT_CONNECT_IND: {
        rtk_bt_le_conn_ind_t *conn_ind = (rtk_bt_le_conn_ind_t *)param;
        if (!conn_ind->err) {
            ble_conn = (void *)conn_ind->conn_handle;
            if (bt_init_info && bt_init_info->on_connected) {
                bt_init_info->on_connected();
                aiio_log_i("BLE connected\r\n");
            }
        }
        break;
    }
    
    case RTK_BT_LE_GAP_EVT_DISCONN_IND: {
        rtk_bt_le_disconn_ind_t *disconn_ind = (rtk_bt_le_disconn_ind_t *)param;
        ble_conn = NULL;
        if (bt_init_info && bt_init_info->on_disconnected) {
            bt_init_info->on_disconnected();
        }
        break;
    }
    
    default:
        break;
    }
    
    return RTK_BT_EVT_CB_OK;
}

// 发送数据函数
static aiio_err_t ali_ble_send_data(void *conn, ble_gatts_attr_t *attr, 
                                  uint8_t *data, uint16_t length, 
                                  uint8_t type) {
    if (!conn || !attr) {
        return AIIO_ERROR;
    }
    
    rtk_bt_gatts_ntf_and_ind_param_t param = {
        .conn_handle = conn,
        .app_id = attr->srv_id,
        .index = attr->attr_index,
        .data = data,
        .len = length,
    };
    
    int8_t ret;
    if (type == AIIO_BLE_INDICATE) {
        ret = rtk_bt_gatts_indicate(&param);
    } else {
        ret = rtk_bt_gatts_notify(&param);
    }
    return (ret == RTK_BT_OK) ? AIIO_OK : AIIO_ERROR;
}

// 初始化蓝牙栈
ais_err_t ble_stack_init(ais_bt_init_t *info) {
    bt_init_info = info;

    rtk_bt_app_conf_t bt_app_conf = {0};
    rtk_bt_le_addr_t bd_addr = {(rtk_bt_le_addr_type_t)0, {0}};
    char addr_str[30] = {0};

    bt_app_conf.app_profile_support = RTK_BT_PROFILE_GATTS | RTK_BT_PROFILE_GATTC;
    bt_app_conf.mtu_size = 247;
    bt_app_conf.master_init_mtu_req = true;
    bt_app_conf.slave_init_mtu_req = true;
    bt_app_conf.prefer_all_phy = 0;
    bt_app_conf.prefer_tx_phy = 1 | 1 << 1 | 1 << 2;
    bt_app_conf.prefer_rx_phy = 1 | 1 << 1 | 1 << 2;
    bt_app_conf.max_tx_octets = 0x40;
    bt_app_conf.max_tx_time = 0x200;
    bt_app_conf.user_def_service = false;
    bt_app_conf.cccd_not_check = true;

    BT_APP_PROCESS(rtk_bt_enable(&bt_app_conf));
    BT_APP_PROCESS(rtk_bt_le_gap_get_bd_addr(&bd_addr));
    rtk_bt_le_addr_to_str(&bd_addr, addr_str, sizeof(addr_str));
    BT_LOGA("BD_ADDR: %s\r\n", addr_str);

    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GAP, ble_gap_app_callback));
    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GATTS, ble_gatts_app_callback));

    BT_APP_PROCESS(rtk_bt_le_gap_set_appearance(RTK_BT_LE_GAP_APPEARANCE_UNKNOWN));

    rtk_bt_gattc_register_profile(0);

    aiio_log_i("aiio_ble_init success\r\n");
    
    // 注册服务
    if (rtk_bt_gatts_register_service(&ali_ble_service) != RTK_BT_OK) {
        return -1;
    }
    
    // 设置属性句柄
    ble_def_attr_rc.srv_id = SERVICE_INDEX;
    ble_def_attr_rc.attr_index = RC_CHAR_INDEX;
    
    ble_def_attr_wc.srv_id = SERVICE_INDEX;
    ble_def_attr_wc.attr_index = WC_CHAR_INDEX;
    
    ble_def_attr_ic.srv_id = SERVICE_INDEX;
    ble_def_attr_ic.attr_index = IC_CHAR_INDEX;
    
    ble_def_attr_wwnrc.srv_id = SERVICE_INDEX;
    ble_def_attr_wwnrc.attr_index = WWNRC_CHAR_INDEX;
    
    ble_def_attr_nc.srv_id = SERVICE_INDEX;
    ble_def_attr_nc.attr_index = NC_CHAR_INDEX;
    
    srv_handle.rc_char_handle[0] = &ble_def_attr_rc;
    srv_handle.wc_char_handle[0] = &ble_def_attr_wc;
    srv_handle.ic_char_handle[0] = &ble_def_attr_ic;
    srv_handle.wwnrc_char_handle[0] = &ble_def_attr_wwnrc;
    srv_handle.nc_char_handle[0] = &ble_def_attr_nc;
    
    return AIS_ERR_SUCCESS;
}

// 开始广播
ais_err_t ble_advertising_start(ais_adv_init_t *adv)
{
    int            err;
    uint8_t adv_data[27] = {
        /* Flags */
        0x02,             /* length */
        0x01,             /* type="Flags" */
        0x02 | 0x04,      /* GENERAL | BREDR_NOT_SUPPORTED */
        0x03,             /* length */
        0x03,             /*type */
        0xb3,0xfe,        /* company id */
        /* UUID */
        0x0f,             /* length */
        0xff,             /*type */ 
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
    };
    uint8_t rsp_data[31] = {
        /* Name */
        0x00,             /* length */
        0x09,             /* type="Complete name" */
    };
    aiio_log_i("adv->vdata.len:%d\n", adv->vdata.len);
    for(int i = 0; i < adv->vdata.len; i++){
        aiio_log_i("%x ", adv->vdata.data[i],adv->vdata.data);
    }
    aiio_log_i("\n");
    uint16_t adv_len = 9;
    uint16_t rsp_len;
    memcpy(adv_data + 9, adv->vdata.data, adv->vdata.len);
    adv_len += adv->vdata.len;
    /* pack rsp */
    rsp_len = strlen(adv->name.name);
    rsp_data[0] = rsp_len + 1;
    memcpy(rsp_data + 2, adv->name.name, rsp_len);
    rsp_len = rsp_len + 2;

    /* set gap name */
    aiio_ble_set_name(adv->name.name);

    err = aiio_ble_adv_start(&param, adv_data, adv_len, rsp_data, rsp_len);
    if (err) {
        aiio_log_i("Advertising failed to start (err %d)\n", err);
        // dump_awss_status(SUB_ERRCODE_BLE_START_ADV_FAIL, "start adv fail:%d", err);
        return AIS_ERR_ADV_FAIL;
    }

    return 0;
}

// 发送指示
ais_err_t ble_send_indication(uint8_t *p_data, uint16_t length, void (*txdone)(uint8_t res)) {
    if (!ble_conn) {
        return -1;
    }
    
    g_indication_txdone = txdone;
    aiio_log_i("send indication: %d\r\n", length);
    return ali_ble_send_data(ble_conn, srv_handle.ic_char_handle[0], 
                           p_data, length, AIIO_BLE_INDICATE);
}


// 发送通知
ais_err_t ble_send_notification(uint8_t *p_data, uint16_t length) {
    if (!ble_conn) {
        return -1;
    }
    aiio_log_i("send notification: %d\r\n", length);
    return ali_ble_send_data(ble_conn, srv_handle.nc_char_handle[0], 
                           p_data, length, AIIO_BLE_NOTIFY);
}

// 获取MAC地址
ais_err_t ble_get_mac(uint8_t *mac) {
    rtk_bt_le_addr_t addr;
    if (rtk_bt_le_gap_get_bd_addr(&addr) != RTK_BT_OK) {
        return AIS_ERR_MEM_FAIL;
    }
    
    memcpy(mac, addr.addr_val, 6);
    return AIS_ERR_SUCCESS;
}

// 获取ATT MTU
int ble_get_att_mtu(uint16_t *att_mtu) {
    if (!ble_conn) {
        return -1;
    }
    
    return rtk_bt_le_gap_get_mtu_size(ble_conn, att_mtu);
}