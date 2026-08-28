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
#include "rtk_bt_vendor.h"
#include <rtk_bt_gap.h>
#include <rtk_bt_le_gap.h>
#include <rtk_bt_att_defs.h>
#include <rtk_bt_gatts.h>
#include <rtk_bt_gattc.h>

// #include <rtk_service_config.h>
// #include <rtk_bas.h>
// #include <rtk_hrs.h>
// #include <rtk_simple_ble_service.h>
// #include <rtk_dis.h>
// #include <rtk_ias.h>
// #include <rtk_hids_kb.h>
// #include <rtk_gls.h>
// #include <rtk_long_uuid_service.h>
#include <bt_utils.h>

typedef struct
{
    uint8_t srv_id;
    uint8_t attr_index;
} ble_gatts_attr_t;

static aiio_ble_mode_t ble_mode;
static aiio_ble_mode_t ble_gap_stack_flag;
static aiio_ble_config_t ble_config_s;
static aiio_ble_cb_t ble_cb;
static volatile uint8_t ble_conn_flag;
static volatile uint8_t ble_disc_flag;

static uint8_t ble_connid;

#define SIMPLE_BLE_UUID_EMPTY 0x0000
#define RTK_BT_UUID_BLE_EMPTY BT_UUID_DECLARE_16(SIMPLE_BLE_UUID_EMPTY)

#define BLE_DEF_SRV_ID_1 0
#define BLE_DEF_SRV_ID_2 1
#define BLE_CLIENT_PROFILE_ID 0

#define BLE_DEF_SERVER_TX_INDEX 2
#define BLE_DEF_SERVER_TX_CCD_INDEX 3
#define BLE_DEF_SERVER_RX_INDEX 5

struct bt_uuid_128 def_srv_uuid[3];
struct bt_uuid_128 def_srv_tx_uuid[3];
struct bt_uuid_128 def_srv_rx_uuid[3];

static rtk_bt_gatt_attr_t ble_def_attrs_1[] = {
    RTK_BT_GATT_PRIMARY_SERVICE(&def_srv_uuid[0]),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_tx_uuid[0],
                               RTK_BT_GATT_CHRC_NOTIFY,
                               RTK_BT_GATT_PERM_NONE),
    RTK_BT_GATT_CCC(RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_rx_uuid[0],
                               RTK_BT_GATT_CHRC_WRITE | RTK_BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               RTK_BT_GATT_PERM_WRITE),
};
static struct rtk_bt_gatt_service ble_def_srv_1 = RTK_BT_GATT_SERVICE(ble_def_attrs_1, BLE_DEF_SRV_ID_1);

static rtk_bt_gatt_attr_t ble_def_attrs_2[] = {
    RTK_BT_GATT_PRIMARY_SERVICE(&def_srv_uuid[1]),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_tx_uuid[1],
                               RTK_BT_GATT_CHRC_NOTIFY,
                               RTK_BT_GATT_PERM_NONE),
    RTK_BT_GATT_CCC(RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_rx_uuid[1],
                               RTK_BT_GATT_CHRC_WRITE | RTK_BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               RTK_BT_GATT_PERM_WRITE),
};
static struct rtk_bt_gatt_service ble_def_srv_2 = RTK_BT_GATT_SERVICE(ble_def_attrs_2, BLE_DEF_SRV_ID_2);

#define BLE_BLUFI_SERVER_TX_INDEX 4
#define BLE_BLUFI_SERVER_TX_CCD_INDEX 5
#define BLE_BLUFI_SERVER_RX_INDEX 2
static rtk_bt_gatt_attr_t ble_def_attrs_blufi[] = {
    RTK_BT_GATT_PRIMARY_SERVICE(&def_srv_uuid[2]),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_rx_uuid[2],
                               RTK_BT_GATT_CHRC_WRITE | RTK_BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               RTK_BT_GATT_PERM_WRITE),

    RTK_BT_GATT_CHARACTERISTIC(&def_srv_tx_uuid[2],
                               RTK_BT_GATT_CHRC_NOTIFY,
                               RTK_BT_GATT_PERM_NONE),
    RTK_BT_GATT_CCC(RTK_BT_GATT_PERM_READ | RTK_BT_GATT_PERM_WRITE),
};
static struct rtk_bt_gatt_service ble_def_srv_blufi = RTK_BT_GATT_SERVICE(ble_def_attrs_blufi, BLE_DEF_SRV_ID_1);

ble_gatts_attr_t ble_def_attr_tx[2];
ble_gatts_attr_t ble_def_attr_rx[2];

static void ble_reverse_byte(uint8_t *arr, uint32_t size)
{
    uint8_t i, tmp;

    for (i = 0; i < size / 2; i++)
    {
        tmp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = tmp;
    }
}

void ble_uuid_convert(struct bt_uuid *dest, aiio_ble_uuid_t *src)
{
    switch (src->type) {
        case AIIO_BLE_UUID_TYPE_16:
            dest->type = BT_UUID_TYPE_16;
            memcpy(&(BT_UUID_16(dest)->val), ((aiio_ble_uuid_16_t *)src)->val, 2);
            ble_reverse_byte(&(BT_UUID_16(dest)->val), 2);
            break;
        case AIIO_BLE_UUID_TYPE_128:
            dest->type = BT_UUID_TYPE_128;
            memcpy(BT_UUID_128(dest)->val, ((aiio_ble_uuid_128_t *)src)->val, 16);
            ble_reverse_byte(&(BT_UUID_128(dest)->val), 16);
            break;
        default:
            return ;
    }
}

void ble_uuid_convert_to_aiio(aiio_ble_uuid_t *dest, struct bt_uuid *src)
{
    switch (src->type) {
        case BT_UUID_TYPE_16:
            dest->type = AIIO_BLE_UUID_TYPE_16;
            memcpy(((aiio_ble_uuid_16_t *)dest)->val, &(BT_UUID_16(src)->val), 2);
            ble_reverse_byte(((aiio_ble_uuid_16_t *)dest)->val, 2);
            break;
        case BT_UUID_TYPE_128:
            dest->type = AIIO_BLE_UUID_TYPE_128;
            memcpy(((aiio_ble_uuid_128_t *)dest)->val, BT_UUID_128(src)->val, 16);
            ble_reverse_byte(((aiio_ble_uuid_128_t *)dest)->val, 16);
            break;
        default:
            return ;
    }
}

void ble_uuid_convert_to_aiio2(aiio_ble_uuid_t *dest, uint8_t type, uint8_t *uuid)
{
    switch (type) {
        case BT_UUID_TYPE_16:
            dest->type = AIIO_BLE_UUID_TYPE_16;
            memcpy(((aiio_ble_uuid_16_t *)dest)->val, uuid, 2);
            ble_reverse_byte(((aiio_ble_uuid_16_t *)dest)->val, 2);
            break;
        case BT_UUID_TYPE_128:
            dest->type = AIIO_BLE_UUID_TYPE_128;
            memcpy(((aiio_ble_uuid_128_t *)dest)->val, uuid, 16);
            ble_reverse_byte(((aiio_ble_uuid_128_t *)dest)->val, 16);
            break;
        default:
            return ;
    }
}

static int ble_notify_data(void *conn, ble_gatts_attr_t *attr, uint8_t *data, uint16_t length, uint8_t type)
{
    int ret;
    uint16_t mtu;
    uint16_t offset;
    uint16_t send_len;
    rtk_bt_gatts_ntf_and_ind_param_t ntf_param = {0};

    offset = 0;
    if (rtk_bt_le_gap_get_mtu_size(conn, &mtu) != 0)
    {
        return AIIO_ERROR;
    }

    mtu -= 3;
    while (length > 0) {
        /* calculate send_len */
        send_len = length > mtu ? mtu : length;
        /* send data */
        ntf_param.conn_handle = conn;
        ntf_param.app_id = attr->srv_id;
        ntf_param.index = attr->attr_index;
        ntf_param.data = data + offset;
        ntf_param.len = send_len;
        if (type == 0) {
            ret = rtk_bt_gatts_notify(&ntf_param);
        } else {
            ret = rtk_bt_gatts_indicate(&ntf_param);
        }
        /* set offset */
        offset += send_len;
        length -= send_len;

        if (RTK_BT_OK != ret) {
            BT_LOGA("GATTS notify failed! err: 0x%x\r\n", ret);
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}

static int ble_write_data(void *conn, uint16_t val_handle, uint8_t *data, uint16_t length, uint8_t type)
{
    int ret;
    uint16_t mtu;
    uint16_t offset;
    uint16_t send_len;
    rtk_bt_gattc_write_param_t write_param = {0};

    offset = 0;
    if (rtk_bt_le_gap_get_mtu_size(conn, &mtu) != 0)
    {
        return AIIO_ERROR;
    }

    mtu -= 3;
    while (length > 0) {
        /* calculate send_len */
        send_len = length > mtu ? mtu : length;

        write_param.conn_handle = conn;
        write_param.profile_id = BLE_CLIENT_PROFILE_ID;
        write_param.type = type ? RTK_BT_GATT_CHAR_WRITE_REQ : RTK_BT_GATT_CHAR_WRITE_NO_RSP;
        write_param.handle = val_handle;
        write_param.length = send_len;
        write_param.data = data + offset;

        ret = rtk_bt_gattc_write(&write_param);
        /* set offset */
        offset += send_len;
        length -= send_len;

        if (RTK_BT_OK != ret) {
            BT_LOGA("GATTS write failed! err: 0x%x\r\n", ret);
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}

static rtk_bt_evt_cb_ret_t bt_common_gap_callback(uint8_t evt_code, void *param, uint32_t len)
{
    rtk_bt_evt_cb_ret_t ret = RTK_BT_EVT_CB_OK;

    switch (evt_code)
    {
    default:
        BT_LOGE("Unkown common gap cb evt type: %d\r\n", evt_code);
        break;
    }

    return ret;
}

static rtk_bt_evt_cb_ret_t ble_gap_app_callback(uint8_t evt_code, void *param, uint32_t len)
{
    char le_addr[30] = {0};

    switch (evt_code)
    {
    case RTK_BT_LE_GAP_EVT_ADV_START_IND:
    case RTK_BT_LE_GAP_EVT_ADV_STOP_IND:
    case RTK_BT_LE_GAP_EVT_DATA_LEN_CHANGE_IND:
    case RTK_BT_LE_GAP_EVT_SCAN_START_IND:
    case RTK_BT_LE_GAP_EVT_SCAN_STOP_IND:
        break;
    case RTK_BT_LE_GAP_EVT_CONNECT_IND:
    {
        rtk_bt_le_conn_ind_t *conn_ind = (rtk_bt_le_conn_ind_t *)param;
        rtk_bt_le_addr_to_str(&(conn_ind->peer_addr), le_addr, sizeof(le_addr));
        if (!conn_ind->err)
        {
            char *role = conn_ind->role ? "slave" : "master";
            BT_LOGA("Connected, handle: %d, role: %s, remote device: %s\r\n",
                    (int)conn_ind->conn_handle, role, le_addr);

            if (conn_ind->role == 0) {
                ble_conn_flag = 1;
            }
            if (ble_cb) {
                aiio_ble_evt_t evt;

                evt.type = AIIO_BLE_EVT_CONNECT;
                evt.connect.conn = (void *)conn_ind->conn_handle;
                ble_cb(&evt);
            }
        }
        else
        {
            BT_LOGE("Connection establish failed(err: 0x%x), remote device: %s\r\n",
                    conn_ind->err, le_addr);
        }
        BT_AT_PRINT("+BLEGAP:conn,%d,%d,%s\r\n", (conn_ind->err == 0) ? 0 : -1, (int)conn_ind->conn_handle, le_addr);
        break;
    }
    case RTK_BT_LE_GAP_EVT_DISCONN_IND:
    {
        rtk_bt_le_disconn_ind_t *disconn_ind = (rtk_bt_le_disconn_ind_t *)param;
        BT_LOGA("Disconnected, reason: 0x%x, handle: %d\r\n",
                disconn_ind->reason, disconn_ind->conn_handle);
        
        if (ble_cb) {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_DISCONNECT;
            evt.disconnect.conn = (void *)disconn_ind->conn_handle;
            evt.disconnect.reason = disconn_ind->reason;
            ble_cb(&evt);
        }
        break;
    }
    case RTK_BT_LE_GAP_EVT_CONN_UPDATE_IND:
    {
        rtk_bt_le_conn_update_ind_t *conn_update_ind = (rtk_bt_le_conn_update_ind_t *)param;
        if (conn_update_ind->err) {
            BT_LOGE("Update conn param failed, conn_handle: %d, err: 0x%x\r\n",
                    conn_update_ind->conn_handle, conn_update_ind->err);
        } else {
            BT_LOGA("conn_handle: %d, conn_interval: 0x%x, conn_latency: 0x%x, supervision_timeout: 0x%x\r\n",
                    conn_update_ind->conn_handle,
                    conn_update_ind->conn_interval,
                    conn_update_ind->conn_latency,
                    conn_update_ind->supv_timeout);
        }
        break;
    }
    case RTK_BT_LE_GAP_EVT_PHY_UPDATE_IND:
    {
        rtk_bt_le_phy_update_ind_t *phy_update_ind =
            (rtk_bt_le_phy_update_ind_t *)param;
        if (phy_update_ind->err) {
            BT_LOGE("Update PHY failed, conn_handle: %d, err: 0x%x\r\n",
                    phy_update_ind->conn_handle,
                    phy_update_ind->err);
        } else {
            BT_LOGA("PHY is updated, conn_handle: %d, tx_phy: %d, rx_phy: %d\r\n",
                    phy_update_ind->conn_handle,
                    phy_update_ind->tx_phy,
                    phy_update_ind->rx_phy);
        }
        break;
    }
    case RTK_BT_LE_GAP_EVT_SCAN_RES_IND:
    {
        rtk_bt_le_scan_res_ind_t *scan_res_ind = (rtk_bt_le_scan_res_ind_t *)param;
        if (ble_cb) {
            aiio_ble_evt_t evt;

            memset(&evt, 0, sizeof evt);
            evt.type = AIIO_BLE_EVT_SCAN;
            evt.scan.rssi = scan_res_ind->adv_report.rssi;
            memcpy(evt.scan.mac, scan_res_ind->adv_report.addr.addr_val, 6);
            ble_reverse_byte(evt.scan.mac, 6);

            if (scan_res_ind->adv_report.evt_type == RTK_BT_LE_EVT_SCAN_RSP) {
                evt.scan.sd_data = scan_res_ind->adv_report.data;
                evt.scan.sd_len = scan_res_ind->adv_report.len;
            } else {
                evt.scan.ad_data = scan_res_ind->adv_report.data;
                evt.scan.ad_len = scan_res_ind->adv_report.len;
            }

            ble_cb(&evt);
        }
        break;
    }
    default:
        BT_LOGE("Unkown gap cb evt type: %d\r\n", evt_code);
        break;
    }

    return RTK_BT_EVT_CB_OK;
}

static void simple_ble_service_callback(uint8_t event, void *data)
{
    switch (event) {
    case RTK_BT_GATTS_EVT_REGISTER_SERVICE: {
        rtk_bt_gatts_reg_ind_t *p_gatts_reg_ind = (rtk_bt_gatts_reg_ind_t *)data;
        if (p_gatts_reg_ind->reg_status == RTK_BT_OK) {
            BT_LOGA("ble service register succeed!\r\n");
        } else
            BT_LOGE("ble service register failed, err: 0x%x\r\n",
                    p_gatts_reg_ind->reg_status);
        break;
    }
    default:
        break;
    }
}

static rtk_bt_evt_cb_ret_t ble_gatts_app_callback(uint8_t event, void *data, uint32_t len)
{
    (void)len;
    int ret;
    uint16_t app_id = 0xFFFF;

    if (RTK_BT_GATTS_EVT_MTU_EXCHANGE == event)
    {
        rtk_bt_gatt_mtu_exchange_ind_t *p_gatt_mtu_ind = (rtk_bt_gatt_mtu_exchange_ind_t *)data;
        if (p_gatt_mtu_ind->result == RTK_BT_OK)
        {
            BT_LOGA("GATTS mtu exchange successfully, mtu_size: %d, conn_handle: %d \r\n",
                    p_gatt_mtu_ind->mtu_size, p_gatt_mtu_ind->conn_handle);
        }
        else
        {
            BT_LOGE("GATTS mtu exchange fail \r\n");
        }
        return RTK_BT_EVT_CB_OK;
    }

    if (RTK_BT_GATTS_EVT_CLIENT_SUPPORTED_FEATURES == event)
    {
        rtk_bt_gatts_client_supported_features_ind_t *p_ind = (rtk_bt_gatts_client_supported_features_ind_t *)data;
        if (p_ind->features & RTK_BT_GATTS_CLIENT_SUPPORTED_FEATURES_EATT_BEARER_BIT)
        {
            BT_LOGA("Client Supported features is writed: conn_handle %d, features 0x%02x. Remote client supports EATT.\r\n",
                    p_ind->conn_handle, p_ind->features);
        }
        return RTK_BT_EVT_CB_OK;
    }

    switch (event) {
    case RTK_BT_GATTS_EVT_READ_IND: {
        rtk_bt_gatts_read_ind_t *p_read_ind = (rtk_bt_gatts_read_ind_t *)data;
        break;
    }
    case RTK_BT_GATTS_EVT_WRITE_IND: {
        rtk_bt_gatts_write_ind_t *p_write_ind = (rtk_bt_gatts_write_ind_t *)data;
        rtk_bt_gatts_write_resp_param_t write_resp = {0};
        write_resp.app_id = p_write_ind->app_id;
        write_resp.conn_handle = p_write_ind->conn_handle;
        write_resp.cid = p_write_ind->cid;
        write_resp.index = p_write_ind->index;
        write_resp.type = p_write_ind->type;

        if (!p_write_ind->len || !p_write_ind->value) {
            BT_LOGA("BLE write value is empty!\r\n");
            write_resp.err_code = RTK_BT_ATT_ERR_INVALID_VALUE_SIZE;
            goto send_write_rsp;
        }

        if (ble_cb) {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_DATA;
            evt.data.handle = p_write_ind->index;
            evt.data.type = AIIO_BLE_WRITE_WITHOUT_RESP;
            evt.data.conn = (void *)p_write_ind->conn_handle;
            evt.data.data = p_write_ind->value;
            evt.data.length = p_write_ind->len;
            ble_cb(&evt);
        }
send_write_rsp:
        ret = rtk_bt_gatts_write_resp(&write_resp);
        if (RTK_BT_OK != ret) {
            BT_LOGE("BLE response for client write failed, err: 0x%x\r\n", ret);
        }
        break;
    }
    case RTK_BT_GATTS_EVT_CCCD_IND: {
        rtk_bt_gatts_cccd_ind_t *p_cccd_ind = (rtk_bt_gatts_cccd_ind_t *)data;
        break;
    }
    case RTK_BT_GATTS_EVT_NOTIFY_COMPLETE_IND:
    case RTK_BT_GATTS_EVT_INDICATE_COMPLETE_IND: {
        rtk_bt_gatts_ntf_and_ind_ind_t *p_ind_ntf = (rtk_bt_gatts_ntf_and_ind_ind_t *)data;
        break;
    }
    default:
        break;
    }

    return RTK_BT_EVT_CB_OK;
}

static rtk_bt_evt_cb_ret_t ble_gattc_app_callback(uint8_t event, void *data, uint32_t len)
{
    (void)len;
    uint16_t profile_id = 0xFFFF;

    if (RTK_BT_GATTC_EVT_MTU_EXCHANGE == event) {
        rtk_bt_gatt_mtu_exchange_ind_t *p_gatt_mtu_ind = (rtk_bt_gatt_mtu_exchange_ind_t *)data;
        if (p_gatt_mtu_ind->result == RTK_BT_OK) {
            BT_LOGA("GATTC mtu exchange success, mtu_size: %d, conn_handle: %d \r\n",
                    p_gatt_mtu_ind->mtu_size, p_gatt_mtu_ind->conn_handle);
        } else {
            BT_LOGE("GATTC mtu exchange fail \r\n");
        }
        return RTK_BT_EVT_CB_OK;
    }

    switch (event) {
    case RTK_BT_GATTC_EVT_READ_RESULT_IND:
    case RTK_BT_GATTC_EVT_WRITE_RESULT_IND:
        break;
    case RTK_BT_GATTC_EVT_NOTIFY_IND: {
        rtk_bt_gattc_cccd_value_ind_t *ntf_ind = (rtk_bt_gattc_cccd_value_ind_t *)data;

        if (!ntf_ind->len || !ntf_ind->value) {
            return;
        }
        BT_LOGA("RTK_BT_GATTC_EVT_NOTIFY_IND:%d \r\n", ntf_ind->len);
        if (ble_cb) {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_DATA;
            evt.data.handle = (aiio_ble_att_handle_t)ntf_ind->value_handle;
            evt.data.type = AIIO_BLE_NOTIFY;
            evt.data.conn = (void *)ntf_ind->conn_handle;
            evt.data.data = (uint8_t *)ntf_ind->value;
            evt.data.length = ntf_ind->len;
            ble_cb(&evt);
        }
    }
        break;
    case RTK_BT_GATTC_EVT_INDICATE_IND: {
        rtk_bt_gattc_cccd_value_ind_t *indicate_ind = (rtk_bt_gattc_cccd_value_ind_t *)data;

        if (!indicate_ind->len || !indicate_ind->value) {
            BT_LOGE("[APP] GATTC indicate received value is empty!\r\n");
        }
        BT_LOGA("RTK_BT_GATTC_EVT_INDICATE_IND:%d \r\n", indicate_ind->len);
        if (ble_cb) {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_DATA;
            evt.data.handle = (aiio_ble_att_handle_t)indicate_ind->value_handle;
            evt.data.type = AIIO_BLE_INDICATE;
            evt.data.conn = (void *)indicate_ind->conn_handle;
            evt.data.data = (uint8_t *)indicate_ind->value;
            evt.data.length = indicate_ind->len;
            ble_cb(&evt);
        }
    }
        break;
    case RTK_BT_GATTC_EVT_DISCOVER_RESULT_IND: {
        rtk_bt_gattc_discover_ind_t *disc_res = (rtk_bt_gattc_discover_ind_t *)data;
        if (ble_cb) {
            aiio_ble_evt_t evt;
            aiio_ble_gatt_attr_t aiio_attr;
            aiio_ble_uuid_128_t aiio_uuid = {0};

            if (disc_res->type == RTK_BT_GATT_DISCOVER_CHARACTERISTIC_ALL) {
                ble_uuid_convert_to_aiio2(&aiio_uuid, disc_res->disc_char_all_per.uuid_type, disc_res->disc_char_all_per.uuid);
                aiio_attr.handle = disc_res->disc_char_all_per.value_handle;
            }
            aiio_attr.uuid = &aiio_uuid;

            evt.type = AIIO_BLE_EVT_DISC;
            evt.disc.conn = (aiio_ble_conn_t)disc_res->conn_handle;
            evt.disc.attr = &aiio_attr;
            ble_cb(&evt);
        }

        if (disc_res->status == RTK_BT_STATUS_DONE ||
            disc_res->status == RTK_BT_STATUS_FAIL) {
            ble_disc_flag = 1;
        }
    }
        break;
    default:
        BT_LOGA("ble_gatts_app_callback unk:%d\r\n", event);
        break;
    }

    return RTK_BT_EVT_CB_OK;
}

aiio_err_t aiio_ble_init(aiio_ble_mode_t mode, const aiio_ble_config_t *config)
{
    memcpy(&ble_config_s, config, sizeof ble_config_s);

    ble_mode = mode;

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
    aiio_log_i("aiio_ble_init enter");
    BT_APP_PROCESS(rtk_bt_enable(&bt_app_conf));
    aiio_log_i("rtk_bt_enable");
    BT_APP_PROCESS(rtk_bt_le_gap_get_bd_addr(&bd_addr));
    aiio_log_i("rtk_bt_le_gap_get_bd_addr");
    rtk_bt_le_addr_to_str(&bd_addr, addr_str, sizeof(addr_str));
    aiio_log_i("rtk_bt_le_addr_to_str");
    BT_LOGA("BD_ADDR: %s\r\n", addr_str);

    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_COMMON_GP_GAP, bt_common_gap_callback));
    aiio_log_i("bt_common_gap_callback");
    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GAP, ble_gap_app_callback));
    aiio_log_i("ble_gap_app_callback");
    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GATTS, ble_gatts_app_callback));
    aiio_log_i("ble_gatts_app_callback");
    BT_APP_PROCESS(rtk_bt_evt_register_callback(RTK_BT_LE_GP_GATTC, ble_gattc_app_callback));
    aiio_log_i("ble_gattc_app_callback");

    BT_APP_PROCESS(rtk_bt_le_gap_set_appearance(RTK_BT_LE_GAP_APPEARANCE_UNKNOWN));
    aiio_log_i("rtk_bt_le_gap_set_appearance");
    rtk_bt_gattc_register_profile(BLE_CLIENT_PROFILE_ID);
    aiio_log_i("rtk_bt_gattc_register_profile");
    BT_LOGA("aiio_ble_init success\r\n");
    aiio_log_i("aiio_ble_init end");
    return AIIO_OK;
}

aiio_err_t aiio_ble_deinit(void)
{
    rtk_bt_disable();

    return AIIO_OK;
}

aiio_err_t aiio_ble_register_event_cb(aiio_ble_cb_t cb)
{
    ble_cb = cb;

    return AIIO_OK;
}

aiio_err_t aiio_ble_adv_start(aiio_ble_adv_param_t *param,
                              const uint8_t *ad, uint16_t ad_len,
                              const uint8_t *sd, uint16_t sd_len)
{
    rtk_bt_le_adv_param_t adv_param = {
        .interval_min = 200,
        .interval_max = 250,
        .type = RTK_BT_LE_ADV_TYPE_IND,
        .own_addr_type = RTK_BT_LE_ADDR_TYPE_PUBLIC,
        .peer_addr = {
            .type = (rtk_bt_le_addr_type_t)0,
            .addr_val = {0},
        },
        .channel_map = RTK_BT_LE_ADV_CHNL_ALL,
        .filter_policy = RTK_BT_LE_ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
    };

    adv_param.interval_min = param->interval_min;
    adv_param.interval_max = param->interval_max;

    if (param->conn_mode != AIIO_BLE_CONN_MODE_NON)
    {
        adv_param.type = RTK_BT_LE_ADV_TYPE_IND;
    }

    rtk_bt_le_gap_set_adv_data(ad, ad_len);
    rtk_bt_le_gap_set_scan_rsp_data(sd, sd_len);
    rtk_bt_le_gap_start_adv(&adv_param);

    return AIIO_OK;
}

aiio_err_t aiio_ble_adv_stop(void)
{
    le_adv_stop();

    return AIIO_OK;
}

aiio_err_t ble_gatts_add_default_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    ble_uuid_convert(&def_srv_uuid[0], cfg->server_uuid[0]);
    ble_uuid_convert(&def_srv_tx_uuid[0], cfg->tx_char_uuid[0]);
    ble_uuid_convert(&def_srv_rx_uuid[0], cfg->rx_char_uuid[0]);

    ble_uuid_convert(&def_srv_uuid[1], cfg->server_uuid[1]);
    ble_uuid_convert(&def_srv_tx_uuid[1], cfg->tx_char_uuid[1]);
    ble_uuid_convert(&def_srv_rx_uuid[1], cfg->rx_char_uuid[1]);

    ble_def_srv_1.type = GATT_SERVICE_OVER_BLE;
    ble_def_srv_1.server_info = 0;
    ble_def_srv_1.user_data = NULL;
    ble_def_srv_1.register_status = 0;
    if (0 != rtk_bt_gatts_register_service(&ble_def_srv_1))
    {
        BT_LOGA("[ble]reg service1 fail\r\n");
        return AIIO_ERROR;
    }

    ble_def_srv_2.type = GATT_SERVICE_OVER_BLE;
    ble_def_srv_2.server_info = 0;
    ble_def_srv_2.user_data = NULL;
    ble_def_srv_2.register_status = 0;
    if (0 != rtk_bt_gatts_register_service(&ble_def_srv_2))
    {
        BT_LOGA("[ble]reg service2 fail\r\n");
        return AIIO_ERROR;
    }

    ble_def_attr_tx[0].srv_id = BLE_DEF_SRV_ID_1;
    ble_def_attr_tx[0].attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx[0].srv_id = BLE_DEF_SRV_ID_1;
    ble_def_attr_rx[0].attr_index = BLE_DEF_SERVER_RX_INDEX;

    ble_def_attr_tx[1].srv_id = BLE_DEF_SRV_ID_2;
    ble_def_attr_tx[1].attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx[1].srv_id = BLE_DEF_SRV_ID_2;
    ble_def_attr_rx[1].attr_index = BLE_DEF_SERVER_RX_INDEX;

    handle->tx_char_handle[0] = &ble_def_attr_tx[0];
    handle->rx_char_handle[0] = &ble_def_attr_rx[0];

    handle->tx_char_handle[1] = &ble_def_attr_tx[1];
    handle->rx_char_handle[1] = &ble_def_attr_tx[1];

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_mac(uint8_t *mac)
{
    static rtk_bt_le_addr_t bd_addr = {(rtk_bt_le_addr_type_t)0, {0}};
    rtk_bt_le_addr_t empty_addr = {(rtk_bt_le_addr_type_t)0, {0}};

    if (memcmp(&bd_addr, &empty_addr, 6) == 0) {
        rtk_bt_le_gap_get_bd_addr(&bd_addr);
    }

    memcpy(mac, bd_addr.addr_val, 6);
    ble_reverse_byte(mac, 6);

    return AIIO_OK;
}

aiio_err_t aiio_ble_set_mac(const uint8_t *mac)
{
    uint8_t bt_addr[6];
    uint8_t local_bd_type = RTK_BT_LE_RAND_ADDR_STATIC;

    memcpy(bt_addr, mac, 6);
    ble_reverse_byte(bt_addr, 6);

    rtk_bt_le_gap_set_rand_addr(false, local_bd_type, bt_addr);

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_limit_power(int8_t *min, int8_t *max)
{
    *min = -10;
    *max = 7;

    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_req(aiio_ble_conn_t conn, uint16_t mtu)
{
    /* gap_config_max_mtu_size */
    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_get(aiio_ble_conn_t conn, uint16_t *mtu)
{
    if (rtk_bt_le_gap_get_mtu_size(conn, mtu) != 0)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_restart_advertising(void)
{
    le_adv_start();

    return AIIO_OK;
}

aiio_err_t aiio_ble_disconnect(aiio_ble_conn_t conn)
{
    return rtk_bt_le_gap_disconnect(conn);
}

aiio_err_t aiio_ble_connect(uint8_t addr_type, uint8_t *addr, uint32_t timeout)
{
    static rtk_bt_le_create_conn_param_t le_conn_param = {
        .peer_addr = {
            .type = (rtk_bt_le_addr_type_t)0,
            .addr_val = {0},
        },
        .scan_interval = 0x20,
        .scan_window = 0x20,
        .filter_policy = RTK_BT_LE_CONN_FILTER_WITHOUT_WHITELIST,
    };

    le_conn_param.conn_interval_max = ble_config_s.conn_intv.max_interval;
    le_conn_param.conn_interval_min = ble_config_s.conn_intv.min_interval;
    le_conn_param.conn_latency = ble_config_s.conn_intv.latency;
    le_conn_param.supv_timeout = ble_config_s.conn_intv.timeout;
    le_conn_param.scan_timeout = timeout;

    memcpy(le_conn_param.peer_addr.addr_val, addr, 6);
    ble_reverse_byte(le_conn_param.peer_addr.addr_val, 6);

    ble_conn_flag = 0;
    rtk_bt_le_gap_connect(&le_conn_param);
    timeout /= 10;
    while (ble_conn_flag == 0) {
        timeout--;
        if (timeout == 0) {
            rtk_bt_le_gap_connect_cancel(&le_conn_param.peer_addr);
            return AIIO_ERROR;
        }

        aiio_os_tick_dealy(aiio_os_ms2tick(10));
    }
    return AIIO_OK;
}

aiio_err_t aiio_ble_scan(const aiio_ble_scan_param_t *cfg)
{
    uint8_t scan_mode = cfg->type;
    uint16_t scan_interval = cfg->interval;
    uint16_t scan_window = cfg->window;
    rtk_bt_le_scan_param_t scan_param = {
        .type = RTK_BT_LE_SCAN_TYPE_PASSIVE,
        .interval = scan_interval,
        .window = scan_window,
        .own_addr_type = RTK_BT_LE_ADDR_TYPE_PUBLIC,
        .filter_policy = RTK_BT_LE_SCAN_FILTER_ALLOW_ALL,
        .duplicate_opt = true,
    };

    if (cfg->type == AIIO_BLE_SCAN_TYPE_ACTIVE)
    {
        scan_param.type = RTK_BT_LE_SCAN_TYPE_ACTIVE;
    }

    rtk_bt_send_cmd(RTK_BT_LE_GP_GAP, RTK_BT_LE_GAP_ACT_SET_SCAN_PARAM, (void *)&scan_param, sizeof(rtk_bt_le_scan_param_t));
    rtk_bt_le_gap_start_scan();

    return AIIO_OK;
}

aiio_err_t aiio_ble_scan_stop(void)
{
    rtk_bt_le_gap_stop_scan();
    return AIIO_OK;
}

aiio_err_t aiio_ble_send(const aiio_ble_send_t *data)
{
    struct bt_gatt_attr *char_val;

    switch (data->type) {
        case AIIO_BLE_NOTIFY:
            ble_notify_data(data->conn, data->handle, data->data, data->length, 0);
            break;
        case AIIO_BLE_INDICATE:
            ble_notify_data(data->conn, data->handle, data->data, data->length, 1);
            break;
        case AIIO_BLE_WRITE_WITHOUT_RESP:
            ble_write_data(data->conn, data->handle, data->data, data->length, 0);
            break;
        case AIIO_BLE_WRITE:
            ble_write_data(data->conn, data->handle, data->data, data->length, 1);
            break;
        default:
            return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_discover(const aiio_ble_disc_param_t *param, uint16_t timeout)
{
    int ret = AIIO_ERROR;
    rtk_bt_gattc_discover_param_t disc_param = {
        .profile_id = 0,
    };

    if (param->type != AIIO_BLE_DISCOVER_CHARACTERISTIC)
    {
        return AIIO_ERROR;
    }

    ble_disc_flag = 0;

    disc_param.conn_handle = (uint16_t)param->conn;
    disc_param.profile_id = BLE_CLIENT_PROFILE_ID;
    disc_param.type = RTK_BT_GATT_DISCOVER_CHARACTERISTIC_ALL;
    disc_param.disc_char_all.start_handle = param->start_handle;
    disc_param.disc_char_all.end_handle = param->end_handle;

    if (rtk_bt_gattc_discover(&disc_param) != 0)
    {
        return AIIO_ERROR;
    }
    timeout /= 10;
    while (ble_disc_flag == 0) {
        timeout--;
        if (timeout == 0) {
            return AIIO_ERROR;
        }

        aiio_os_tick_dealy(aiio_os_ms2tick(10));
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_set_name(const char *name)
{
    rtk_bt_le_gap_set_device_name((uint8_t *)name);

    return AIIO_OK;
}

aiio_err_t ble_gatts_add_blufi_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    ble_uuid_convert(&def_srv_uuid[2], cfg->server_uuid[0]);
    ble_uuid_convert(&def_srv_tx_uuid[2], cfg->tx_char_uuid[0]);
    ble_uuid_convert(&def_srv_rx_uuid[2], cfg->rx_char_uuid[0]);

    ble_uuid_convert(&def_srv_uuid[1], cfg->server_uuid[1]);
    ble_uuid_convert(&def_srv_tx_uuid[1], cfg->tx_char_uuid[1]);
    ble_uuid_convert(&def_srv_rx_uuid[1], cfg->rx_char_uuid[1]);

    ble_def_srv_blufi.type = GATT_SERVICE_OVER_BLE;
    ble_def_srv_blufi.server_info = 0;
    ble_def_srv_blufi.user_data = NULL;
    ble_def_srv_blufi.register_status = 0;
    if (0 != rtk_bt_gatts_register_service(&ble_def_srv_blufi))
    {
        BT_LOGA("[ble]reg service1 fail\r\n");
        return AIIO_ERROR;
    }

    ble_def_srv_2.type = GATT_SERVICE_OVER_BLE;
    ble_def_srv_2.server_info = 0;
    ble_def_srv_2.user_data = NULL;
    ble_def_srv_2.register_status = 0;
    if (0 != rtk_bt_gatts_register_service(&ble_def_srv_2))
    {
        BT_LOGA("[ble]reg service2 fail\r\n");
        return AIIO_ERROR;
    }

    ble_def_attr_tx[0].srv_id = BLE_DEF_SRV_ID_1;
    ble_def_attr_tx[0].attr_index = BLE_BLUFI_SERVER_TX_INDEX;
    ble_def_attr_rx[0].srv_id = BLE_DEF_SRV_ID_1;
    ble_def_attr_rx[0].attr_index = BLE_BLUFI_SERVER_RX_INDEX;

    ble_def_attr_tx[1].srv_id = BLE_DEF_SRV_ID_2;
    ble_def_attr_tx[1].attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx[1].srv_id = BLE_DEF_SRV_ID_2;
    ble_def_attr_rx[1].attr_index = BLE_DEF_SERVER_RX_INDEX;

    handle->tx_char_handle[0] = &ble_def_attr_tx[0];
    handle->rx_char_handle[0] = &ble_def_attr_rx[0];

    handle->tx_char_handle[1] = &ble_def_attr_tx[1];
    handle->rx_char_handle[1] = &ble_def_attr_tx[1];

    return AIIO_OK;
}

aiio_err_t ble_gattc_ccc_cfg(const aiio_ble_ccc_t *cfg)
{
    return AIIO_ERROR;
}

aiio_err_t aiio_ble_set_conn_power(aiio_ble_conn_t conn, int8_t power)
{
    rtk_bt_vendor_tx_power_param_t tx_power = {0};
    tx_power.tx_power_type = 1;
    tx_power.conn_tx_power.conn_handle = conn;
    tx_power.tx_gain = power;
    rtk_bt_set_tx_power(&tx_power);

    return AIIO_OK;
}

aiio_err_t aiio_ble_set_adv_power(int8_t power)
{
    rtk_bt_vendor_tx_power_param_t tx_power = {0};
    tx_power.tx_power_type = 0;
    tx_power.adv_tx_power.type = 0;
    tx_power.tx_gain = power;
    rtk_bt_set_tx_power(&tx_power);

    return AIIO_OK;
}
