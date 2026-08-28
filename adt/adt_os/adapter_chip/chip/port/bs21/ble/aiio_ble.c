#include "aiio_ble.h"
#include "aiio_os_port.h"
#include "aiio_log.h"
#include "osal_list.h"
#include "test_suite_uart.h"
#include "bts_device_manager.h"
#include "bts_le_gap.h"
#include "securec.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "bts_def.h"
#include "errcode.h"
#include "bts_gatt_stru.h"
#include "bts_gatt_server.h"
#include "bts_gatt_client.h"

#define BLE_GAP_BLE_ADV_HANDLE_DEFAULT 0x01

static aiio_ble_config_t ble_config_s;
static int32_t ble_init_flag = 0;
static volatile uint8_t ble_conn_flag; // 蓝牙连接标志，用于处理蓝牙是否连接成功
static aiio_ble_cb_t ble_cb = NULL;    // 蓝牙回调函数
                                       // BLE GAP 回调函数
static gap_ble_callbacks_t gap_cb = {0};
static gatts_callbacks_t service_cb = {0};
static gattc_callbacks_t client_cb = {0};
static bts_dev_manager_callbacks_t dev_mgr_cb = {0};
static char aiio_start_app_uuid[] = {0x00, 0x00};
static uint8_t server_id = 0;       // 用于创建服务ID
static uint8_t server_flag_b = 0;   // 注册服务器ID
static uint8_t server_id_b = 0;     // 注册服务器ID
static uint8_t server_handle_b = 0; // 注册服务器句柄
static uint16_t char_flag_b = 0;
static uint16_t char_handle_b = 0;
static bt_uuid_t app_uuid = {0};
static uint8_t gatts_register_server_flag = 0; // 注册函数标志位
static bt_uuid_t g_client_app_uuid = {2, {0}}; // 客户端UUID
static uint8_t g_aiio_client_id = 10;          // 客户端ID
static uint8_t g_discover_sever_status = 0;
static uint16_t g_discover_sever_status_cnt = 0;
static uint8_t g_discover_char_status = 0;
static uint16_t g_discover_char_status_cnt = 0;
static uint8_t g_discover_desc_status = 0;

/* Service UUID */
#define BLE_UUID_UUID_SERVER_SERVICE 0xABCD
/* Characteristic UUID */
#define BLE_UUID_UUID_SERVER_REPORT 0xCDEF
/* Client Characteristic Configuration UUID */
#define BLE_UUID_CLIENT_CHARACTERISTIC_CONFIGURATION 0x2902
/* Server ID */
#define BLE_UUID_SERVER_ID 1

#define UUID_SERVER_PROPERTIES (GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY)

/*************************************************************************************/

typedef struct connnode
{
    uint32_t conn_id; // 用于查找
    uint8_t mac[6];   // MAC 地址
    uint8_t type;     // 类型字段
    struct connnode *next;
} connNode;

// 创建节点
connNode *ble_create_node(uint32_t conn_id, uint8_t mac[6], uint8_t type)
{
    connNode *new_node = (connNode *)aiio_os_malloc(sizeof(connNode));
    if (!new_node)
    {
        aiio_log_w("内存分配失败！\n");
        return NULL;
    }
    new_node->conn_id = conn_id;
    memcpy(new_node->mac, mac, 6);
    new_node->type = type;
    new_node->next = NULL;
    return new_node;
}

// 尾插节点
void ble_insert_tail(connNode **head, uint32_t conn_id, uint8_t mac[6], uint8_t type)
{
    connNode *new_node = ble_create_node(conn_id, mac, type);
    if (*head == NULL)
    {
        *head = new_node;
    }
    else
    {
        connNode *temp = *head;
        while (temp->next != NULL)
            temp = temp->next;
        temp->next = new_node;
    }
}

// 查找节点（按 conn_id）
connNode *ble_search_node(connNode *head, uint32_t conn_id)
{
    while (head != NULL)
    {
        if (head->conn_id == conn_id)
            return head;
        head = head->next;
    }
    return NULL;
}

// 删除节点（按 conn_id）
void ble_delete_node(connNode **head, uint32_t conn_id)
{
    connNode *temp = *head;
    connNode *prev = NULL;

    while (temp != NULL)
    {
        if (temp->conn_id == conn_id)
        {
            if (prev == NULL)
            {
                *head = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            aiio_os_free(temp);
            aiio_log_w("节点 conn_id = %d 已删除。\n", conn_id);
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    aiio_log_w("未找到 conn_id = %d 的节点。\n", conn_id);
}

// 打印链表
void ble_print_list(connNode *head)
{
    while (head != NULL)
    {
        aiio_log_w("[conn_id: %d, mac: %02X:%02X:%02X:%02X:%02X:%02X, type: %d] -> ",
                   head->conn_id,
                   head->mac[0], head->mac[1], head->mac[2],
                   head->mac[3], head->mac[4], head->mac[5],
                   head->type);
        head = head->next;
    }
    aiio_log_w("NULL\n");
}

// 释放链表
void ble_free_list(connNode *head)
{
    while (head != NULL)
    {
        connNode *temp = head;
        head = head->next;
        aiio_os_free(temp);
    }
}

connNode *head = NULL;
/************************************************************************************* */

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

void stream_data_to_uuid(uint16_t uuid_data, bt_uuid_t *out_uuid)
{
    char uuid[] = {(uint8_t)(uuid_data >> 8), (uint8_t)uuid_data};
    out_uuid->uuid_len = 2;
    if (memcpy_s(out_uuid->uuid, out_uuid->uuid_len, uuid, 2) != EOK)
    {
        return;
    }
}

aiio_err_t ble_character_add(bt_uuid_t bt_uuid, uint8_t properties, uint8_t permissions, uint8_t *value, uint16_t value_len)
{
    uint8_t cnt = 0;
    gatts_add_chara_info_t character;

    character.chara_uuid = bt_uuid;
    character.properties = properties;
    character.permissions = permissions;
    character.value_len = value_len;
    character.value = value;

    char_flag_b = 0;
    gatts_add_characteristic(server_id_b, server_handle_b, &character);

    while (!char_flag_b)
    {
        cnt++;
        if (cnt > 200)
        {
            return AIIO_ERROR;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }

    return AIIO_OK;
}

aiio_err_t ble_descriptor_add(uint8_t permissions, uint8_t *value, uint16_t value_len)
{
    uint8_t ccc_uuid_s[] = {0x29, 0x02};
    bt_uuid_t ccc_uuid = {0};
    gatts_add_desc_info_t descriptor;

    ccc_uuid.uuid_len = 2;
    if (memcpy_s(ccc_uuid.uuid, ccc_uuid.uuid_len, ccc_uuid_s, 2) != EOK)
    {
        return AIIO_ERROR;
    }

    descriptor.desc_uuid = ccc_uuid;
    descriptor.permissions = permissions;
    descriptor.value_len = value_len;
    descriptor.value = value;
    gatts_add_descriptor(server_id_b, server_handle_b, &descriptor);

    return AIIO_OK;
}

static aiio_err_t aiio_ble_service_is_register(void)
{
    if (gatts_register_server_flag == 0)
    {
        gatts_register_server_flag = 1;
        // 注册服务
        app_uuid.uuid_len = sizeof(aiio_start_app_uuid);
        if (memcpy_s(app_uuid.uuid, app_uuid.uuid_len, aiio_start_app_uuid, sizeof(aiio_start_app_uuid)) != EOK)
        {
            return AIIO_ERROR;
        }

        if (ERRCODE_SUCC != gatts_register_server(&app_uuid, &server_id))
        {
            return AIIO_ERROR;
        }

        aiio_log_i("server is %d", server_id);
    }

    return AIIO_OK;
}

void aiio_bt_uuid_convert_to_aiio(aiio_ble_uuid_t *dest, bt_uuid_t *bt_uuid)
{
    switch (bt_uuid->uuid_len)
    {
    case 2:
        dest->type = AIIO_BLE_UUID_TYPE_16;
        if (memcpy_s(((aiio_ble_uuid_16_t *)dest)->val, 2, bt_uuid->uuid, 2) != EOK)
        {
            return;
        }
        // ble_reverse_byte(((aiio_ble_uuid_16_t *)dest)->val, 2);
        break;
    case 4:
        dest->type = AIIO_BLE_UUID_TYPE_32;
        if (memcpy_s(((aiio_ble_uuid_32_t *)dest)->val, 4, bt_uuid->uuid, 4) != EOK)
        {
            return;
        }
        ble_reverse_byte(((aiio_ble_uuid_32_t *)dest)->val, 4);
        break;
    case 16:
        dest->type = AIIO_BLE_UUID_TYPE_128;
        if (memcpy_s(((aiio_ble_uuid_128_t *)dest)->val, 16, bt_uuid->uuid, 16) != EOK)
        {
            return;
        }
        ble_reverse_byte(((aiio_ble_uuid_128_t *)dest)->val, 16);
        break;
    default:
        return;
    }
}

aiio_err_t aiio_ble_uuid_conn_bt_uuid(bt_uuid_t *bt_uuid, aiio_ble_uuid_t *aiio_ble_uuid)
{
    switch (aiio_ble_uuid[0].type)
    {
    case AIIO_BLE_UUID_TYPE_16:
        bt_uuid->uuid_len = 2;
        if (memcpy_s(bt_uuid->uuid, sizeof(bt_uuid->uuid), &aiio_ble_uuid[1], 2) != EOK)
        {
            return AIIO_ERROR;
        }

        aiio_log_i("service uuid len is 2\n");
        break;
    case AIIO_BLE_UUID_TYPE_32:
        bt_uuid->uuid_len = 4;
        if (memcpy_s(bt_uuid->uuid, sizeof(bt_uuid->uuid), &aiio_ble_uuid[1], 4) != EOK)
        {
            return AIIO_ERROR;
        }
        aiio_log_i("service uuid len is 4\n");
        break;
    case AIIO_BLE_UUID_TYPE_128:
        bt_uuid->uuid_len = 16;
        if (memcpy_s(bt_uuid->uuid, sizeof(bt_uuid->uuid), &aiio_ble_uuid[1], 16) != EOK)
        {
            return AIIO_ERROR;
        }
        aiio_log_i("service uuid len is 16\n");
        break;
    default:
        aiio_log_i("service uuid type fail\n");
        break;
    }

    return AIIO_OK;
}

aiio_err_t ble_gatts_service_add(uint8_t server_id, bt_uuid_t *service_uuid, bool is_primary)
{
    uint8_t cnt = 0;
    server_flag_b = 0;
    if (ERRCODE_SUCC != gatts_add_service(server_id, service_uuid, is_primary))
    {
        return AIIO_ERROR;
    }

    while (!server_flag_b)
    {
        cnt++;
        if (cnt > 200)
        {
            return AIIO_ERROR;
        }
        aiio_os_tick_dealy(aiio_os_ms2tick(1));
    }

    return AIIO_OK;
}

/**************************蓝牙设备管理************************/

static void aiio_ble_server_enable_cbk(uint8_t status)
{
    aiio_log_w("ble enable: %d\n", status);
}

static void aiio_ble_client_enable_cbk(uint8_t status)
{
    aiio_log_w("ble enable: %d\n", status);
    aiio_log_w("gattc client:0x%08x", gattc_register_client(&g_client_app_uuid, &g_aiio_client_id));

    aiio_log_w("client_id:%d", g_aiio_client_id);
}

/**************************蓝牙设备管理************************/

// 设置广播数据回调函数
static void aiio_ble_adv_start_cbk(uint8_t adv_id, adv_status_t status)
{
    aiio_log_i("adv start cbk adv_id:%d status:%d\n", adv_id, status);
}

static void aiio_ble_adv_stop_cbk(uint8_t adv_id, adv_status_t status)
{
    aiio_log_i("adv stop cbk adv_id:%d status:%d\n", adv_id, status);
}

static void aiio_ble_adv_set_data_cbk(uint8_t adv_id, errcode_t status)
{
    aiio_log_i("adv set data cbk adv_id:%d status:%d\n", adv_id, status);
}

static void aiio_ble_adv_set_param_cbk(uint8_t adv_id, errcode_t status)
{
    aiio_log_i("adv set param cbk adv_id:%d status:%d\n", adv_id, status);
}

static void aiio_ble_connect_change_cbk(uint16_t conn_id, bd_addr_t *addr, gap_ble_conn_state_t conn_state,
                                        gap_ble_pair_state_t pair_state, gap_ble_disc_reason_t disc_reason)
{
    aiio_log_i("connect state change conn_id: %d, status: %d, pair_status:%d, disc_reason %x\n",
               conn_id, conn_state, pair_state, disc_reason);

    if (conn_state == GAP_BLE_STATE_CONNECTED && pair_state == GAP_BLE_PAIR_NONE)
    {
        // 蓝牙连接成功把标志置1
        ble_conn_flag = 1;
        ble_insert_tail(&head, conn_id, addr->addr, addr->type);
        if (ble_cb)
        {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_CONNECT;
            evt.connect.conn = (void *)(uint32_t)conn_id;
            ble_cb(&evt);
        }
    }
    else if (conn_state == GAP_BLE_STATE_DISCONNECTED)
    {
        ble_delete_node(&head, conn_id);
        if (ble_cb)
        {
            aiio_ble_evt_t evt;

            evt.type = AIIO_BLE_EVT_DISCONNECT;
            evt.disconnect.conn = (void *)(uint32_t)conn_id;
            evt.disconnect.reason = disc_reason;
            ble_cb(&evt);
        }
    }
}

/*********************************蓝牙GATT服务端回调处理*********************************************** */

static void aiio_ble_uuid_server_service_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t handle, errcode_t status)
{
    // bt_uuid_t service_uuid = {0};
    aiio_log_i("[uuid server] add service cbk: server: %d, status: %d, srv_handle: %d, uuid_len: %d,uuid:",
               server_id, status, handle, uuid->uuid_len);
    for (int8_t i = 0; i < uuid->uuid_len; i++)
    {
        aiio_log_i("%02x", (uint8_t)uuid->uuid[i]);
    }
    aiio_log_i("\n");

    server_flag_b = 1;
    server_id_b = server_id;
    server_handle_b = handle;
}

static void aiio_ble_uuid_server_characteristic_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
                                                        gatts_add_character_result_t *result, errcode_t status)
{
    int8_t i = 0;
    aiio_log_i("[uuid server] add characteristic cbk: server: %d, status: %d, srv_hdl: %d "
               "char_hdl: %x, char_val_hdl: %x, uuid_len: %d, uuid: ",
               server_id, status, service_handle, result->handle, result->value_handle, uuid->uuid_len);
    for (i = 0; i < uuid->uuid_len; i++)
    {
        aiio_log_i("%02x", (uint8_t)uuid->uuid[i]);
    }
    aiio_log_i("\n");
    char_flag_b = 1;
    char_handle_b = result->value_handle;
}

/* 描述符添加回调 */
static void aiio_ble_uuid_server_descriptor_add_cbk(uint8_t server_id, bt_uuid_t *uuid, uint16_t service_handle,
                                                    uint16_t handle, errcode_t status)
{
    int8_t i = 0;
    aiio_log_i("[uuid server] add descriptor cbk : server: %d, status: %d, srv_hdl: %d, desc_hdl: %x ,"
               "uuid_len:%d, uuid: ",
               server_id, status, service_handle, handle, uuid->uuid_len);
    for (i = 0; i < uuid->uuid_len; i++)
    {
        aiio_log_i("%02x", (uint8_t)uuid->uuid[i]);
    }
    aiio_log_i("\n");
}

/* 开始服务回调 */
static void aiio_ble_uuid_server_service_start_cbk(uint8_t server_id, uint16_t handle, errcode_t status)
{
    aiio_log_i("[uuid server] start service cbk : server: %d status: %d srv_hdl: %d\n",
               server_id, status, handle);
    gatts_set_mtu_size(server_id, 100);
}

static void aiio_ble_uuid_server_receive_write_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_write_cb_t *write_cb_para,
                                                       errcode_t status)
{
    aiio_log_i("ble uart write cbk server_id:%d, conn_id:%d, status%d\n", server_id, conn_id, status);
    aiio_log_i("ble uart write cbk len:%d, data:%s\n", write_cb_para->length, write_cb_para->value);

    if (ble_cb)
    {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DATA;
        evt.data.handle = (aiio_ble_att_handle_t)(uint32_t)write_cb_para->handle;
        evt.data.type = AIIO_BLE_WRITE_WITHOUT_RESP;
        evt.data.conn = (void *)(uint32_t)conn_id;
        evt.data.data = (uint8_t *)write_cb_para->value;
        evt.data.length = write_cb_para->length;
        ble_cb(&evt);
    }
}

static void aiio_ble_uuid_server_mtu_changed_cbk(uint8_t server_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    aiio_log_i("MtuChanged--server_id:%d conn_id:%d\n", server_id, conn_id);
    aiio_log_i("mtusize:%d, status:%d\n", mtu_size, status);

    // gap_le_set_data_length_t param = {0};
    // param.conn_handle = conn_id;
    // param.maxtxoctets = MAXTXOCTETS;
    // param.maxtxtime = MAXTXTIME;
    // errcode_t ret = gap_ble_set_data_length(&param);
    // if (ret != ERRCODE_BT_SUCCESS) {
    //     osal_printk("%s set data length failed ret = %d\n", BLE_UART_SERVER_ERROR, ret);
    // }
}

static void aiio_ble_uuid_server_receive_read_req_cbk(uint8_t server_id, uint16_t conn_id, gatts_req_read_cb_t *read_cb_para,
                                                      errcode_t status)
{
    aiio_log_i("ReceiveReadReq--server_id:%d conn_id:%d\n", server_id, conn_id);
    aiio_log_i("request_id:%d, att_handle:%d offset:%d, need_rsp:%d, is_long:%d\n", read_cb_para->request_id, read_cb_para->handle, read_cb_para->offset,
               read_cb_para->need_rsp, read_cb_para->is_long);
    aiio_log_i("status:%d\n", status);
}
/*********************************蓝牙GATT服务端回调处理*********************************************** */

/*********************************蓝牙GAP扫描回调处理*********************************************** */
static void aiio_ble_gap_adv_set_scanparam_cbk(errcode_t status)
{
    aiio_log_i("adv set scanparam cbk  status:%d\n", status);
}

static void aiio_ble_gap_scan_result_cbk(gap_scan_result_data_t *scan_result_data)
{
#define ADV_MAX_LEN 31
    static struct
    {
        bd_addr_t addr;
        uint8_t adv_data[ADV_MAX_LEN];
        uint8_t adv_len;
    } adv_cache;

    if (!scan_result_data || !ble_cb)
    {
        return;
    }

    aiio_ble_evt_t evt;
    memset(&evt, 0, sizeof(evt));
    evt.type = AIIO_BLE_EVT_SCAN;

    evt.scan.rssi = scan_result_data->rssi;

    // MAC地址需要字节序反转（如平台要求）
    memcpy(evt.scan.mac, scan_result_data->addr.addr, 6);
    ble_reverse_byte(evt.scan.mac, 6);

    uint8_t evt_type = scan_result_data->event_type;

    // 扫描响应事件（legacy scan rsp）
    if (evt_type == GAP_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV ||
        evt_type == GAP_BLE_EVT_LEGACY_SCAN_RSP_TO_ADV_SCAN)
    {

        evt.scan.sd_data = scan_result_data->adv_data;
        evt.scan.sd_len = scan_result_data->adv_len;

        // 若有对应的上一个广播缓存，则合并
        if (memcmp(adv_cache.addr.addr, scan_result_data->addr.addr, 6) == 0)
        {
            evt.scan.ad_data = adv_cache.adv_data;
            evt.scan.ad_len = adv_cache.adv_len;
        }
    }
    // 可缓存的广播类型
    else if (evt_type == GAP_BLE_EVT_CONNECTABLE ||
             evt_type == GAP_BLE_EVT_CONNECTABLE_DIRECTED ||
             evt_type == GAP_BLE_EVT_SCANNABLE ||
             evt_type == GAP_BLE_EVT_SCANNABLE_DIRECTED ||
             evt_type == GAP_BLE_EVT_LEGACY_CONNECTABLE ||
             evt_type == GAP_BLE_EVT_LEGACY_CONNECTABLE_DIRECTED ||
             evt_type == GAP_BLE_EVT_LEGACY_SCANNABLE)
    {

        // 缓存广播数据
        if (scan_result_data->adv_len <= ADV_MAX_LEN)
        {
            memcpy(adv_cache.addr.addr, scan_result_data->addr.addr, 6);
            memcpy(adv_cache.adv_data, scan_result_data->adv_data, scan_result_data->adv_len);
            adv_cache.adv_len = scan_result_data->adv_len;
        }

        evt.scan.ad_data = scan_result_data->adv_data;
        evt.scan.ad_len = scan_result_data->adv_len;
    }
    // 其他非缓存类广播事件，如扩展不可连接广播、定向广播等
    else
    {
        evt.scan.ad_data = scan_result_data->adv_data;
        evt.scan.ad_len = scan_result_data->adv_len;
    }

    // 通知上层处理
    ble_cb(&evt);
}
/*********************************蓝牙GAP扫描回调处理*********************************************** */

/************************************************************************************************* */
// 客户端回调处理
static void aiio_ble_client_discover_service_cbk(uint8_t client_id, uint16_t conn_id,
                                                 gattc_discovery_service_result_t *service, errcode_t status)
{
    gattc_discovery_character_param_t param = {0};
    aiio_log_e("Discovery service callback client:%d conn_id:%d\n", client_id, conn_id);
    aiio_log_e("start handle:%d end handle:%d uuid_len:%d uuid:\n",
               service->start_hdl, service->end_hdl, service->uuid.uuid_len);
    // for (uint8_t i = 0; i < service->uuid.uuid_len; i++)
    // {
    //     aiio_log_e("%02x", service->uuid.uuid[i]);
    // }
    aiio_log_e("status:%d\n", status);

    param.service_handle = service->start_hdl;
    param.uuid.uuid_len = service->uuid.uuid_len; /* uuid length is zero, discover all character */
    if (memcpy_s(param.uuid.uuid, param.uuid.uuid_len, service->uuid.uuid, service->uuid.uuid_len) != 0)
    {
        aiio_log_e("memcpy error\n");
    }

    if (g_discover_char_status)
    {
        g_discover_sever_status_cnt++;
    }

    gattc_discovery_character(g_aiio_client_id, conn_id, &param);

    if (ble_cb && g_discover_sever_status)
    {
        aiio_ble_evt_t evt;
        aiio_ble_gatt_attr_t aiio_attr;
        aiio_ble_uuid_128_t aiio_uuid = {0};

        aiio_bt_uuid_convert_to_aiio((aiio_ble_uuid_t *)&aiio_uuid, &service->uuid);

        aiio_attr.handle = service->start_hdl;

        aiio_attr.uuid = (aiio_ble_uuid_t *)&aiio_uuid;

        evt.type = AIIO_BLE_EVT_DISC;
        evt.disc.conn = (aiio_ble_conn_t)(uint32_t)conn_id;
        evt.disc.attr = &aiio_attr;
        ble_cb(&evt);
    }
}

static void aiio_ble_client_discover_character_cbk(uint8_t client_id, uint16_t conn_id,
                                                   gattc_discovery_character_result_t *character, errcode_t status)
{
    // for (uint8_t i = 0; i < character->uuid.uuid_len; i++)
    // {
    //     aiio_log_e("%02x", character->uuid.uuid[i]);
    // }
    aiio_log_e("discover character declare_handle:%d, value_handle:%d, properties:%2x\n",
               character->declare_handle, character->value_handle, character->properties);
    aiio_log_e("client_id:%d, conn_id = %d, status:%d\n", client_id, conn_id, status);

    if (g_discover_desc_status)
    {
        g_discover_char_status_cnt++;
    }

    gattc_discovery_descriptor(g_aiio_client_id, conn_id, character->declare_handle);

    if (ble_cb && g_discover_char_status)
    {
        aiio_ble_evt_t evt;
        aiio_ble_gatt_attr_t aiio_attr;
        aiio_ble_uuid_128_t aiio_uuid = {0};

        aiio_bt_uuid_convert_to_aiio((aiio_ble_uuid_t *)&aiio_uuid, &character->uuid);

        // 使用的是值句柄，用于发送数据
        aiio_attr.handle = character->value_handle;

        aiio_attr.uuid = (aiio_ble_uuid_t *)&aiio_uuid;

        evt.type = AIIO_BLE_EVT_DISC;
        evt.disc.conn = (aiio_ble_conn_t)(uint32_t)conn_id;
        evt.disc.attr = &aiio_attr;
        ble_cb(&evt);
    }
}

static void aiio_ble_client_discover_descriptor_cbk(uint8_t client_id, uint16_t conn_id,
                                                    gattc_discovery_descriptor_result_t *descriptor, errcode_t status)
{
    aiio_log_e("Discovery descriptor----client:%d conn_id:%d uuid len:%d, uuid:\n",
               client_id, conn_id, descriptor->uuid.uuid_len);
    // for (uint8_t i = 0; i < descriptor->uuid.uuid_len; i++)
    // {
    //     aiio_log_e("%02x", descriptor->uuid.uuid[i]);
    // }
    aiio_log_e("descriptor handle:%d, status:%d\n", descriptor->descriptor_hdl, status);

    if (ble_cb && g_discover_desc_status)
    {
        aiio_ble_evt_t evt;
        aiio_ble_gatt_attr_t aiio_attr;
        aiio_ble_uuid_128_t aiio_uuid = {0};

        aiio_bt_uuid_convert_to_aiio((aiio_ble_uuid_t *)&aiio_uuid, &descriptor->uuid);

        aiio_attr.handle = descriptor->descriptor_hdl;

        aiio_attr.uuid = (aiio_ble_uuid_t *)&aiio_uuid;

        evt.type = AIIO_BLE_EVT_DISC;
        evt.disc.conn = (aiio_ble_conn_t)(uint32_t)conn_id;
        evt.disc.attr = &aiio_attr;
        ble_cb(&evt);
    }
}

// 蓝牙服务回调完成
static void aiio_ble_client_discover_service_compl_cbk(uint8_t client_id, uint16_t conn_id, bt_uuid_t *uuid,
                                                       errcode_t status)
{
    aiio_log_e("Discovery service complete----client:%d conn_id:%d uuid len:%d uuid:\n", client_id, conn_id, uuid->uuid_len);
    // for (uint8_t i = 0; i < uuid->uuid_len; i++)
    // {
    //     aiio_log_w("%02x", uuid->uuid[i]);
    // }
    aiio_log_e(" status:%d\n", status);
    g_discover_sever_status = 0;
}

// 蓝牙特征回调完成
static void aiio_ble_client_discover_character_compl_cbk(uint8_t client_id, uint16_t conn_id,
                                                         gattc_discovery_character_param_t *param, errcode_t status)
{
    aiio_log_w("Discovery character complete----client:%d conn_id:%d uuid len:%d uuid: \n",
               client_id, conn_id, param->uuid.uuid_len);
    // for (uint8_t i = 0; i < param->uuid.uuid_len; i++) {
    //     osal_printk("%02x", param->uuid.uuid[i]);
    // }
    aiio_log_e("service handle:%d status:%d\n", param->service_handle, status);

    if (g_discover_char_status)
    {
        g_discover_sever_status_cnt--;
        if (g_discover_sever_status_cnt == 0)
        {
            g_discover_char_status = 0;
        }
    }
}

// 蓝牙描述回调完成
static void aiio_ble_client_discover_descriptor_compl_cbk(uint8_t client_id, uint16_t conn_id,
                                                          uint16_t character_handle, errcode_t status)
{
    aiio_log_e("Discovery descriptor complete----client:%d conn_id:%d\n", client_id, conn_id);
    aiio_log_e("charatcer handle:%d, status:%d\n", character_handle, status);

    if (g_discover_desc_status)
    {
        g_discover_char_status_cnt--;
        if (g_discover_char_status_cnt == 0)
        {
            g_discover_desc_status = 0;
        }
    }
}

static void aiio_ble_client_notification_cbk(uint8_t client_id, uint16_t conn_id, gattc_handle_value_t *data,
                                             errcode_t status)
{
    aiio_log_w("Receive notification----client:%d conn_id:%d\n", client_id, conn_id);
    aiio_log_w("handle:%d data_len:%d\ndata:", data->handle, data->data_len);
    aiio_log_w("ble_uart_client_notification_cbk %s", data->data);
    aiio_log_w("\nstatus:%d\n", status);

    if (ble_cb)
    {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DATA;
        evt.data.handle = (aiio_ble_att_handle_t)(uint32_t)data->handle;
        evt.data.type = AIIO_BLE_NOTIFY;
        evt.data.conn = (void *)(uint32_t)conn_id;
        evt.data.data = (uint8_t *)data->data;
        evt.data.length = data->data_len;
        ble_cb(&evt);
    }
}

static void aiio_ble_client_mtu_changed_cbk(uint8_t client_id, uint16_t conn_id, uint16_t mtu_size, errcode_t status)
{
    aiio_log_w("Mtu changed----client:%d conn_id:%d, mtu size:%d, status:%d\n", client_id, conn_id, mtu_size, status);
}

/************************************************************************************************* */

aiio_err_t aiio_ble_init(aiio_ble_mode_t mode, const aiio_ble_config_t *config)
{
    config = config;

    // 判断蓝牙是否已经初始化了
    if (ble_init_flag == 1)
    {
        aiio_log_w("ble already init");
        return AIIO_OK;
    }

    // 赋值ble_config_s的值
    memcpy_s(&ble_config_s, sizeof(ble_config_s), config, sizeof(ble_config_s));

    // 连接状态改变回调
    gap_cb.conn_state_change_cb = aiio_ble_connect_change_cbk;

    // 判断设备的工作模式
    switch (mode)
    {
    case AIIO_BLE_MODE_PERIPHERAL:
        // 蓝牙管理初始化
        // dev_mgr_cb.power_on_cb = ble_uart_server_power_on_cbk;
        dev_mgr_cb.ble_enable_cb = aiio_ble_server_enable_cbk;
        bts_dev_manager_register_callbacks(&dev_mgr_cb);

        // 注册回调函数
        gap_cb.start_adv_cb = aiio_ble_adv_start_cbk;
        gap_cb.stop_adv_cb = aiio_ble_adv_stop_cbk;
        gap_cb.set_adv_data_cb = aiio_ble_adv_set_data_cbk;
        gap_cb.set_adv_param_cb = aiio_ble_adv_set_param_cbk;
        gap_ble_register_callbacks(&gap_cb);

        /**************服务端回调处理****************/
        service_cb.add_service_cb = aiio_ble_uuid_server_service_add_cbk;
        service_cb.add_characteristic_cb = aiio_ble_uuid_server_characteristic_add_cbk;
        service_cb.add_descriptor_cb = aiio_ble_uuid_server_descriptor_add_cbk;
        service_cb.start_service_cb = aiio_ble_uuid_server_service_start_cbk;
        service_cb.read_request_cb = aiio_ble_uuid_server_receive_read_req_cbk;
        service_cb.write_request_cb = aiio_ble_uuid_server_receive_write_req_cbk;
        service_cb.mtu_changed_cb = aiio_ble_uuid_server_mtu_changed_cbk;
        gatts_register_callbacks(&service_cb);
        /**************服务端回调处理****************/
        break;

    case AIIO_BLE_MODE_CENTRAL:
        dev_mgr_cb.ble_enable_cb = aiio_ble_client_enable_cbk;
        bts_dev_manager_register_callbacks(&dev_mgr_cb);

        /**************主机扫描接口******************/
        gap_cb.set_scan_param_cb = aiio_ble_gap_adv_set_scanparam_cbk;
        gap_cb.scan_result_cb = aiio_ble_gap_scan_result_cbk;
        gap_ble_register_callbacks(&gap_cb);
        /**************主机扫描接口******************/

        /**************客户端回调处理****************/
        client_cb.discovery_svc_cb = aiio_ble_client_discover_service_cbk;
        client_cb.discovery_svc_cmp_cb = aiio_ble_client_discover_service_compl_cbk;
        client_cb.discovery_chara_cb = aiio_ble_client_discover_character_cbk;
        client_cb.discovery_chara_cmp_cb = aiio_ble_client_discover_character_compl_cbk;
        client_cb.discovery_desc_cb = aiio_ble_client_discover_descriptor_cbk;
        client_cb.discovery_desc_cmp_cb = aiio_ble_client_discover_descriptor_compl_cbk;
        client_cb.notification_cb = aiio_ble_client_notification_cbk;
        client_cb.mtu_changed_cb = aiio_ble_client_mtu_changed_cbk;
        gattc_register_callbacks(&client_cb);
        /**************客户端回调处理****************/
        break;

    default:
        break;
    }

    // 启用蓝牙
    if (ERRCODE_SUCC == enable_ble())
    {
        return AIIO_ERROR; // 禁用失败
    }

    ble_init_flag = 1;

    return AIIO_OK;
}

aiio_err_t aiio_ble_deinit(void)
{
    // 如果 BLE 尚未初始化，直接返回成功
    if (!ble_init_flag)
    {
        aiio_log_w("ble already deinit");
        return AIIO_OK;
    }

    // // 遍历所有连接并尝试断开
    // bt_conn_foreach(BT_CONN_TYPE_ALL, ble_disconnect_all, NULL);

    // // 等待所有连接断开，最多等待 10 次（每次 500ms，总计最多 5 秒）
    // int disconn_cnt = 0;
    // while (le_check_valid_conn() && disconn_cnt++ < 10) {
    //     aiio_log_w("[BLE] wait for ble_disconnect_all");
    //     aiio_os_tick_dealy(aiio_os_ms2tick(500));
    // }

    // // 停止 BLE 广播（如果正在广播）
    if (ERRCODE_SUCC != gap_ble_stop_adv(BLE_GAP_BLE_ADV_HANDLE_DEFAULT))
    {
        return AIIO_ERROR; // 禁用失败
    }

    // 禁用蓝牙
    if (ERRCODE_SUCC != disable_ble())
    {
        return AIIO_ERROR; // 禁用失败
    }

    // 清除 BLE 初始化标志
    ble_init_flag = 0;

    return AIIO_OK; // 去初始化成功
}

aiio_err_t aiio_ble_set_mac(const uint8_t *mac)
{
    bd_addr_t ble_addr = {0};
    ble_addr.type = 0x00;

    if (memcpy_s(ble_addr.addr, BD_ADDR_LEN, mac, BD_ADDR_LEN) != EOK)
    {
        aiio_log_e("aiio_ble_set_mac memcpy failed\n");
        return AIIO_ERROR;
    }

    // 设置蓝牙地址
    if (ERRCODE_SUCC != gap_ble_set_local_addr(&ble_addr))
    {
        aiio_log_e("aiio_ble_set_mac addr failed\n");
        return AIIO_ERROR; // 设置失败
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_mac(uint8_t *mac)
{
    bd_addr_t ble_addr = {0};
    ble_addr.type = 0x00;

    // 获取蓝牙地址
    if (ERRCODE_SUCC != gap_ble_get_local_addr(&ble_addr))
    {
        aiio_log_e("aiio_ble_get_mac addr failed\n");
        return AIIO_ERROR; // 设置失败
    }

    // 对获取的蓝牙地址复制到BUF里面
    if (memcpy_s(mac, BD_ADDR_LEN, ble_addr.addr, BD_ADDR_LEN) != EOK)
    {
        aiio_log_e("aiio_ble_get_mac memcpy failed\n");
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_set_name(const char *name)
{
    // 设置蓝牙名称
    if (ERRCODE_SUCC != gap_ble_set_local_name((const uint8_t *)name, strlen(name)))
    {
        return AIIO_ERROR; // 设置失败
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_adv_start(aiio_ble_adv_param_t *param,
                              const uint8_t *ad, uint16_t ad_len,
                              const uint8_t *sd, uint16_t sd_len)
{
    // 配置广播数据
    gap_ble_config_adv_data_t cfg_adv_data = {0};

    cfg_adv_data.adv_data = (uint8_t *)ad;
    cfg_adv_data.adv_length = ad_len;

    cfg_adv_data.scan_rsp_data = (uint8_t *)sd;
    cfg_adv_data.scan_rsp_length = sd_len;

    if (ERRCODE_SUCC != gap_ble_set_adv_data(BLE_GAP_BLE_ADV_HANDLE_DEFAULT, &cfg_adv_data))
    {
        return AIIO_ERROR; // 设置失败
    }

    // 配置广播参数
    gap_ble_adv_params_t adv_para = {0};

    adv_para.min_interval = param->interval_min;
    adv_para.max_interval = param->interval_max;

    switch (param->conn_mode)
    {
    case AIIO_BLE_CONN_MODE_NON:
        adv_para.adv_type = 0x03;
        break;
    case AIIO_BLE_CONN_MODE_DIR:
        adv_para.adv_type = 0x01;
        break;
    case AIIO_BLE_CONN_MODE_UND:
        adv_para.adv_type = 0x00;
        break;
    default:
        adv_para.adv_type = 0x00; // 默认兜底策略
        break;
    }

    adv_para.duration = 0;
    adv_para.peer_addr.type = 0;
    adv_para.channel_map = 0x07;
    adv_para.adv_type = 0;
    adv_para.adv_filter_policy = 0;

    if (ERRCODE_SUCC != gap_ble_set_adv_param(BLE_GAP_BLE_ADV_HANDLE_DEFAULT, &adv_para))
    {
        return AIIO_ERROR; // 设置失败
    }

    // 启动广播
    if (ERRCODE_SUCC != gap_ble_start_adv(BLE_GAP_BLE_ADV_HANDLE_DEFAULT))
    {
        return AIIO_ERROR; // 设置失败
    }

    // 广播启动成功，返回成功
    return AIIO_OK;
}

aiio_err_t aiio_ble_adv_stop(void)
{
    if (ERRCODE_SUCC != gap_ble_stop_adv(BLE_GAP_BLE_ADV_HANDLE_DEFAULT))
    {
        return AIIO_ERROR; // 禁用失败
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_scan(const aiio_ble_scan_param_t *cfg)
{
    gap_ble_scan_params_t scan_params = {0};

    // 假设 aiio_param 已经被赋值，开始赋值转换：
    scan_params.scan_interval = cfg->interval;
    scan_params.scan_window = cfg->window;
    scan_params.scan_type = cfg->type;

    // 对 gap_param 中无对应项的字段，设置默认值：
    scan_params.scan_phy = 1;           // 1: 1M PHY，例如默认使用1M PHY
    scan_params.scan_filter_policy = 0; // 0: Accept all, 默认过滤策略

    // 设置扫描参数
    if (ERRCODE_SUCC != gap_ble_set_scan_parameters(&scan_params))
    {
        return AIIO_ERROR; // 禁用失败
    }

    // 停止扫描
    if (ERRCODE_SUCC != gap_ble_start_scan())
    {
        return AIIO_ERROR; // 禁用失败
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_scan_stop(void)
{
    // 停止扫描
    if (ERRCODE_SUCC != gap_ble_stop_scan())
    {
        return AIIO_ERROR; // 禁用失败
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_register_event_cb(aiio_ble_cb_t cb)
{
    ble_cb = cb;

    return AIIO_OK;
}

aiio_err_t aiio_ble_restart_advertising(void)
{
    // 启动广播
    if (ERRCODE_SUCC != gap_ble_start_adv(BLE_GAP_BLE_ADV_HANDLE_DEFAULT))
    {
        return AIIO_ERROR; // 设置失败
    }

    return AIIO_OK;
}

aiio_err_t ble_gatts_add_default_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    bt_uuid_t service_uuid = {0};
    bt_uuid_t rx_uuid = {0};
    bt_uuid_t tx_uuid = {0};
    uint8_t ccc_buf[] = {0x00, 0x00};
    // 用来判断是否注册了GATT服务
    if (aiio_ble_service_is_register() != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    // 增加主服务
    aiio_ble_uuid_conn_bt_uuid(&service_uuid, cfg->server_uuid[0]);
    ble_gatts_service_add(server_id, &service_uuid, true);

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&tx_uuid, cfg->tx_char_uuid[0]);
    ble_character_add(tx_uuid, GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY, GATT_CHARACTER_PROPERTY_BIT_READ, NULL, 0);
    (*handle).tx_char_handle[0] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    // 增加特征值
    ble_descriptor_add(GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE, ccc_buf, sizeof(ccc_buf));

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&rx_uuid, cfg->rx_char_uuid[0]);
    ble_character_add(rx_uuid, GATT_CHARACTER_PROPERTY_BIT_WRITE | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP, GATT_CHARACTER_PROPERTY_BIT_WRITE, NULL, 0);
    (*handle).rx_char_handle[0] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    gatts_start_service(server_id_b, server_handle_b);

    // 增加主服务
    aiio_ble_uuid_conn_bt_uuid(&service_uuid, cfg->server_uuid[1]);
    ble_gatts_service_add(server_id, &service_uuid, true);

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&tx_uuid, cfg->tx_char_uuid[1]);
    ble_character_add(tx_uuid, GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY | GATT_CHARACTER_PROPERTY_BIT_INDICATE, GATT_CHARACTER_PROPERTY_BIT_READ, NULL, 0);
    (*handle).tx_char_handle[1] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;
    // 增加特征值
    ble_descriptor_add(GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE, ccc_buf, sizeof(ccc_buf));

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&rx_uuid, cfg->rx_char_uuid[1]);
    ble_character_add(rx_uuid, GATT_CHARACTER_PROPERTY_BIT_WRITE | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP, GATT_CHARACTER_PROPERTY_BIT_WRITE, NULL, 0);
    (*handle).rx_char_handle[1] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    gatts_start_service(server_id_b, server_handle_b);

    // 返回成功
    return AIIO_OK;
}

/* ble client write data to server */
errcode_t aiio_ble_gattc_write_cmd(uint16_t conn_id, uint16_t handle, uint8_t *data, uint16_t len)
{
    gattc_handle_value_t gattc_handle_value = {0};

    gattc_handle_value.handle = handle;
    gattc_handle_value.data_len = len;
    gattc_handle_value.data = data;

    aiio_log_w("ble_uart_client_write_cmd len: %d, g_aiio_client_id: %x conn_id:%d\n", len, g_aiio_client_id,conn_id);
    for (uint16_t i = 0; i < len; i++)
    {
        aiio_log_w("%02x", data[i]);
    }

    errcode_t ret = gattc_write_cmd(g_aiio_client_id, conn_id, &gattc_handle_value);
    if (ret != ERRCODE_BT_SUCCESS)
    {
        aiio_log_e("gattc_write_cmd failed\n");
        return ERRCODE_BT_FAIL;
    }
    return ERRCODE_BT_SUCCESS;
}

aiio_err_t aiio_ble_send(const aiio_ble_send_t *data)
{
    switch (data->type)
    {
    case AIIO_BLE_NOTIFY:
        if (server_flag_b)
        {
            gatts_ntf_ind_t param = {0};

            param.attr_handle = (uint32_t)data->handle;
            param.value_len = data->length;
            param.value = data->data;

            gatts_notify_indicate(server_id_b, (uint32_t)data->conn, &param);
        }
        break;
    case AIIO_BLE_INDICATE:
        if (server_flag_b)
        {
            gatts_ntf_ind_t param = {0};

            param.attr_handle = (uint32_t)data->handle;
            param.value_len = data->length;
            param.value = data->data;

            gatts_notify_indicate(server_id_b, (uint32_t)data->conn, &param);
        }
        break;
    case AIIO_BLE_WRITE_WITHOUT_RESP:
        aiio_ble_gattc_write_cmd((uint32_t)data->conn, (uint32_t)data->handle, data->data, data->length);
        break;
    default:
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t ble_gatts_add_blufi_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    bt_uuid_t service_uuid = {0};
    bt_uuid_t rx_uuid = {0};
    bt_uuid_t tx_uuid = {0};
    uint8_t ccc_buf[] = {0x00, 0x00};
    // 用来判断是否注册了GATT服务
    if (aiio_ble_service_is_register() != AIIO_OK)
    {
        return AIIO_ERROR;
    }
    // 增加主服务
    aiio_ble_uuid_conn_bt_uuid(&service_uuid, cfg->server_uuid[0]);
    ble_gatts_service_add(server_id, &service_uuid, true);

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&tx_uuid, cfg->tx_char_uuid[0]);
    ble_character_add(tx_uuid, GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY, GATT_CHARACTER_PROPERTY_BIT_READ, NULL, 0);
    (*handle).tx_char_handle[0] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    // 增加特征值
    ble_descriptor_add(GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE, ccc_buf, sizeof(ccc_buf));

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&rx_uuid, cfg->rx_char_uuid[0]);
    ble_character_add(rx_uuid, GATT_CHARACTER_PROPERTY_BIT_WRITE | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP, GATT_CHARACTER_PROPERTY_BIT_WRITE, NULL, 0);
    (*handle).rx_char_handle[0] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    gatts_start_service(server_id_b, server_handle_b);

    // 增加主服务
    aiio_ble_uuid_conn_bt_uuid(&service_uuid, cfg->server_uuid[1]);
    ble_gatts_service_add(server_id, &service_uuid, true);

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&tx_uuid, cfg->tx_char_uuid[1]);
    ble_character_add(tx_uuid, GATT_CHARACTER_PROPERTY_BIT_READ | GATT_CHARACTER_PROPERTY_BIT_NOTIFY | GATT_CHARACTER_PROPERTY_BIT_INDICATE, GATT_CHARACTER_PROPERTY_BIT_READ, NULL, 0);
    (*handle).tx_char_handle[1] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;
    // 增加特征值
    ble_descriptor_add(GATT_ATTRIBUTE_PERMISSION_READ | GATT_ATTRIBUTE_PERMISSION_WRITE, ccc_buf, sizeof(ccc_buf));

    // 增加特征服务
    aiio_ble_uuid_conn_bt_uuid(&rx_uuid, cfg->rx_char_uuid[1]);
    ble_character_add(rx_uuid, GATT_CHARACTER_PROPERTY_BIT_WRITE | GATT_CHARACTER_PROPERTY_BIT_WRITE_NO_RSP, GATT_CHARACTER_PROPERTY_BIT_WRITE, NULL, 0);
    (*handle).rx_char_handle[1] = (aiio_ble_att_handle_t)(uint32_t)char_handle_b;

    gatts_start_service(server_id_b, server_handle_b);

    // 返回成功
    return AIIO_OK;
}

aiio_err_t ble_gattc_ccc_cfg(const aiio_ble_ccc_t *cfg)
{
    uint8_t ccc_uuid_s[] = {0x29, 0x02};
    bt_uuid_t ccc_uuid = {0};
    gatts_add_desc_info_t descriptor;

    ccc_uuid.uuid_len = 2;
    if (memcpy_s(ccc_uuid.uuid, ccc_uuid.uuid_len, ccc_uuid_s, 2) != EOK)
    {
        return AIIO_ERROR;
    }

    descriptor.desc_uuid = ccc_uuid;
    descriptor.permissions = 0;
    descriptor.value_len = 2;
    descriptor.value = (uint8_t *)&cfg->cfg;
    gatts_add_descriptor((uint32_t)cfg->conn, (uint32_t)cfg->handle, &descriptor);

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_limit_power(int8_t *min, int8_t *max)
{
    *min = 0;
    *max = 20;

    return AIIO_OK;
}

aiio_err_t aiio_ble_connect(uint8_t addr_type, uint8_t *addr, uint32_t timeout)
{
    timeout = timeout;
    gap_conn_param_update_t params;
    bd_addr_t bt_addr;

    params.interval_max = ble_config_s.conn_intv.max_interval;
    params.interval_min = ble_config_s.conn_intv.min_interval;
    params.slave_latency = ble_config_s.conn_intv.latency;
    params.timeout_multiplier = ble_config_s.conn_intv.timeout;

    aiio_log_i("int_max:%d int_min:%d laten:%d multip:%d \r\n",
               params.interval_max, params.interval_min, params.slave_latency, params.timeout_multiplier);

    gap_ble_connect_param_update(&params);

    // 地址类型
    switch (addr_type)
    {
    case AIIO_BLE_ADDR_PUBLIC:
        bt_addr.type = BT_ADDRESS_TYPE_PUBLIC_DEVICE_ADDRESS;
        break;
    case AIIO_BLE_ADDR_RANDOM:
        bt_addr.type = BT_ADDRESS_TYPE_RANDOM_DEVICE_ADDRESS;
        break;
    default:
        break;
    }

    // 把MAC地址反序
    memcpy_s(bt_addr.addr, 6, addr, 6);
    ble_reverse_byte(bt_addr.addr, 6);

    gap_ble_connect_remote_device(&bt_addr);

    return AIIO_OK;
}

aiio_err_t aiio_ble_disconnect(aiio_ble_conn_t conn)
{
    connNode *found = ble_search_node(head, (uint32_t)conn);
    if (!found)
    {
        aiio_log_w("找到节点: conn_id = %d\n", found->conn_id);
        return AIIO_ERROR;
    }

    bd_addr_t bt_addr;

    bt_addr.type = found->type;
    memcpy_s(bt_addr.addr, 6, found->mac, 6);

    gap_ble_disconnect_remote_device(&bt_addr);

    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_req(aiio_ble_conn_t conn, uint16_t mtu)
{
    aiio_log_i("conn id:%d mtu:%d \r\n", (uint32_t)conn, mtu);

    aiio_log_w("gatt mtu:0x%08x", gattc_exchange_mtu_req(g_aiio_client_id, (uint32_t)conn, mtu));

    return AIIO_OK;
}

errcode_t aiio_ble_client_discover_all_service(uint16_t conn_id)
{
    bt_uuid_t service_uuid = {0}; /* uuid length is zero, discover all service */
    return gattc_discovery_service(g_aiio_client_id, conn_id, &service_uuid);
}

aiio_err_t aiio_ble_discover(const aiio_ble_disc_param_t *param, uint16_t timeout)
{
    timeout = timeout;

    switch (param->type)
    {
    case AIIO_BLE_DISCOVER_PRIMARY:
        g_discover_sever_status = 1;
        break;
    case AIIO_BLE_DISCOVER_CHARACTERISTIC:
        g_discover_char_status = 1;
        break;
    case AIIO_BLE_DISCOVER_DESCRIPTOR:
        g_discover_desc_status = 1;
        break;
    case AIIO_BLE_DISCOVER_ATTRIBUTE:
        /* code */
        g_discover_sever_status = 1;
        g_discover_char_status = 1;
        g_discover_desc_status = 1;
        break;
    default:
        break;
    }

    if (aiio_ble_client_discover_all_service((uint32_t)param->conn) != ERRCODE_SUCC)
    {
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

// aiio_err_t aiio_ble_mtu_get(aiio_ble_conn_t conn, uint16_t *mtu);
// 下面的不需要适配
// aiio_err_t aiio_ble_set_conn_power(aiio_ble_conn_t conn, int8_t power);
// aiio_err_t aiio_ble_set_adv_power(int8_t power);
