#include "aiio_ble.h"
#include "aiio_os_port.h"
#include "aiio_log.h"

#include <os_msg.h>
#include <os_task.h>
#include <app_msg.h>

#include <gap.h>
#include <gap_adv.h>
#include <gap_bond_le.h>
#include <profile_server.h>
#include <gap_msg.h>
#include <bas.h>
#include <bte.h>
#include <gap_config.h>
#include <bt_flags.h>
#include <os_sched.h>
#include <trace_app.h>
#include <gap_scan.h>
#include <profile_client.h>
#include <gcs_client.h>
#include <gap_conn_le.h>

#include <wifi/wifi_conf.h>

#define BLE_MAX_LINKS 1
#define MASTER_APP_MAX_DISCOV_TABLE_NUM  40
#define BLE_APP_MAX_LINKS 1
#define MAX_NUMBER_OF_GAP_MESSAGE 0x20
#define MAX_NUMBER_OF_IO_MESSAGE 0x20
#define BLE_APP_MAX_DISCOV_TABLE_NUM 40
#define MAX_NUMBER_OF_EVENT_MESSAGE (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE)
#define BLE_TASK_STACK_SIZE 1024 * 4

#define BLE_MAX_NUMBER_OF_GAP_MESSAGE 0x20
#define BLE_MAX_NUMBER_OF_IO_MESSAGE 0x20
#define BLE_MAX_NUMBER_OF_EVENT_MESSAGE (BLE_MAX_NUMBER_OF_GAP_MESSAGE + BLE_MAX_NUMBER_OF_IO_MESSAGE)

#define UUID_128_FORMAT "0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X:0x%2X"
#define UUID_128(x) x[0], x[1], x[2], x[3], x[4], x[5], x[6], x[7], x[8], x[9], x[10], x[11], x[12], x[13], x[14], x[15]




typedef struct
{
    uint8_t srv_id;
    uint8_t attr_index;
} ble_gatts_attr_t;

typedef struct
{
    uint8_t                con_id;
    uint8_t                rx_handle;
    T_GAP_CONN_STATE       conn_state;
    T_GAP_REMOTE_ADDR_TYPE bd_type;
    uint8_t                bd_addr[GAP_BD_ADDR_LEN];
} master_link_t;

static aiio_ble_mode_t ble_mode;
static aiio_ble_mode_t ble_gap_stack_flag;
static aiio_ble_config_t ble_config_s;
static aiio_ble_cb_t ble_cb;

static uint8_t         ble_connid;
static master_link_t   ble_link_table[1];
static T_CLIENT_ID     ble_gcs_client_id;
static T_GAP_DEV_STATE ble_gap_dev_state = {0, 0, 0, 0, 0};
static T_SERVER_ID     ble_def_srv_id_1;
static T_SERVER_ID     ble_def_srv_id_2;

static void *ble_task_handle = NULL;
static void *ble_evt_queue_handle = NULL;
static void *ble_io_queue_handle = NULL;

#define BLE_DEF_SERVER_TX_INDEX 2
#define BLE_DEF_SERVER_TX_CCD_INDEX 3
#define BLE_DEF_SERVER_RX_INDEX 5

uint8_t ble_def_srv_uuid1[16];
T_ATTRIB_APPL ble_def_service_tbl_1[] = {
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_128BIT_SIZE,
        ble_def_srv_uuid1,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_NOTIFY
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,
        {},
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),
        {
         LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         LO_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY),
         HI_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY)},
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE_NO_RSP | GATT_CHAR_PROP_WRITE)
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,
        {},
        0,
        NULL,
        GATT_PERM_WRITE | GATT_PERM_READ
    },
};
uint8_t *ble_def_tx_char_uuid_1 = ble_def_service_tbl_1[BLE_DEF_SERVER_TX_INDEX].type_value;
uint8_t *ble_def_rx_char_uuid_1 = ble_def_service_tbl_1[BLE_DEF_SERVER_RX_INDEX].type_value;
ble_gatts_attr_t ble_def_attr_tx_1;
ble_gatts_attr_t ble_def_attr_rx_1;

uint8_t ble_def_srv_uuid2[16];
T_ATTRIB_APPL ble_def_service_tbl_2[] = {
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_128BIT_SIZE,
        ble_def_srv_uuid2,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_NOTIFY
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,
        {},
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),
        {
         LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         LO_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY),
         HI_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY)},
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE_NO_RSP | GATT_CHAR_PROP_WRITE)
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,
        {},
        0,
        NULL,
        GATT_PERM_WRITE | GATT_PERM_READ
    },
};
ble_gatts_attr_t ble_def_attr_tx_2;
ble_gatts_attr_t ble_def_attr_rx_2;

#define BLE_BLUFI_SERVER_TX_INDEX 4
#define BLE_BLUFI_SERVER_TX_CCD_INDEX 5
#define BLE_BLUFI_SERVER_RX_INDEX 2
uint8_t ble_blufi_srv_uuid[16];
T_ATTRIB_APPL ble_blufi_service_tbl[] = {
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_16BIT_SIZE,
        ble_blufi_srv_uuid,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP | GATT_CHAR_PROP_WRITE
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL,
        {},
        0,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    },
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_NOTIFY
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        ATTRIB_FLAG_VALUE_APPL,
        {},
        1,
        NULL,
        GATT_PERM_READ
    },
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),
        {
         LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         LO_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY),
         HI_WORD(GATT_CLIENT_CHAR_CONFIG_NOTIFY)},
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    },
};

static void ble_reverse_byte(uint8_t *arr, uint32_t size)
{
    uint8_t i, tmp;

    for (i = 0; i < size / 2; i++) {
        tmp = arr[i];
        arr[i] = arr[size - 1 - i];
        arr[size - 1 - i] = tmp;
    }
}

void ble_uuid_convert_srv(T_ATTRIB_APPL *tbl, aiio_ble_uuid_t *src)
{
    switch (src->type) {
        case AIIO_BLE_UUID_TYPE_16:
            tbl[0].value_len = UUID_16BIT_SIZE;
            memcpy(tbl[0].p_value_context, ((aiio_ble_uuid_16_t *)src)->val, 2);
            ble_reverse_byte(tbl[0].p_value_context, 2);
            break;
        case AIIO_BLE_UUID_TYPE_128:
            tbl[0].value_len = UUID_128BIT_SIZE;
            memcpy(tbl[0].p_value_context, ((aiio_ble_uuid_128_t *)src)->val, 16);
            ble_reverse_byte(tbl[0].p_value_context, 16);
            break;
        default:
            return ;
    }
}

void ble_uuid_convert_char(T_ATTRIB_APPL *tbl, aiio_ble_uuid_t *src)
{
    switch (src->type) {
        case AIIO_BLE_UUID_TYPE_16:
            tbl[0].flags = ATTRIB_FLAG_VALUE_APPL;
            memcpy(tbl[0].type_value, ((aiio_ble_uuid_16_t *)src)->val, 2);
            ble_reverse_byte(tbl[0].type_value, 2);
            break;
        case AIIO_BLE_UUID_TYPE_128:
            tbl[0].flags = ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT;
            memcpy(tbl[0].type_value, ((aiio_ble_uuid_128_t *)src)->val, 16);
            ble_reverse_byte(tbl[0].type_value, 16);
            break;
        default:
            return ;
    }
}

// void ble_uuid_convert_to_aiio(aiio_ble_uuid_t *dest, struct bt_uuid *src)
// {
//     switch (src->type) {
//         case BT_UUID_TYPE_16:
//             dest->type = AIIO_BLE_UUID_TYPE_16;
//             memcpy(((aiio_ble_uuid_16_t *)dest)->val, &(BT_UUID_16(src)->val), 2);
//             ble_reverse_byte(((aiio_ble_uuid_16_t *)dest)->val, 2);
//             break;
//         case BT_UUID_TYPE_32:
//             dest->type = AIIO_BLE_UUID_TYPE_32;
//             memcpy(((aiio_ble_uuid_32_t *)dest)->val, &(BT_UUID_32(src)->val), 4);
//             ble_reverse_byte(((aiio_ble_uuid_32_t *)dest)->val, 4);
//             break;
//         case BT_UUID_TYPE_128:
//             dest->type = AIIO_BLE_UUID_TYPE_128;
//             memcpy(((aiio_ble_uuid_128_t *)dest)->val, BT_UUID_128(src)->val, 16);
//             ble_reverse_byte(((aiio_ble_uuid_128_t *)dest)->val, 16);
//             break;
//         default:
//             return ;
//     }
// }

static T_APP_RESULT ble_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;
    char          adv_type[20];
    char          remote_addr_type[10];

    switch (cb_type) {
        case GAP_MSG_LE_SCAN_INFO:
        {
            T_LE_SCAN_INFO* scanf_buf = p_data->p_le_scan_info;

            #define ADV_SIZE 31
            #define ADV_ADDR_LEN 6
            static struct {
                uint8_t addr[ADV_ADDR_LEN];
                T_GAP_ADV_EVT_TYPE adv_type;
                uint16_t ad_len;
                uint8_t ad_data[ADV_SIZE];
            } adv_rec;

            /* scannable advertising */
            if(scanf_buf->adv_type == GAP_ADV_EVT_TYPE_UNDIRECTED || scanf_buf->adv_type == GAP_ADV_EVT_TYPE_SCANNABLE)
            {
                if (scanf_buf->data_len > ADV_SIZE) {
                    aiio_log_w("invalid adv size:%d", scanf_buf->data_len);
                    return ;
                }
                
                adv_rec.adv_type = scanf_buf->adv_type;
                memcpy(adv_rec.addr, scanf_buf->bd_addr, ADV_ADDR_LEN);
                memcpy(adv_rec.ad_data, scanf_buf->data, scanf_buf->data_len);
                adv_rec.ad_len = scanf_buf->data_len;

                return ;                
            }
            if (ble_cb) {
                aiio_ble_evt_t evt;

                memset(&evt, 0, sizeof evt);
                evt.type = AIIO_BLE_EVT_SCAN;
                evt.scan.rssi = scanf_buf->rssi;
                memcpy(evt.scan.mac, scanf_buf->bd_addr, ADV_ADDR_LEN);
                ble_reverse_byte(evt.scan.mac, ADV_ADDR_LEN);

                if (scanf_buf->adv_type == GAP_ADV_EVT_TYPE_SCAN_RSP) {
                    evt.scan.sd_data = scanf_buf->data;
                    evt.scan.sd_len = scanf_buf->data_len;

                    if (memcmp(adv_rec.addr, scanf_buf->bd_addr, ADV_ADDR_LEN) == 0) {
                        evt.scan.ad_data = adv_rec.ad_data;
                        evt.scan.ad_len = adv_rec.ad_len;
                    }
                } else {
                    evt.scan.ad_data = scanf_buf->data;
                    evt.scan.ad_len = scanf_buf->data_len;
                }

                ble_cb(&evt);
            }            
        }
            break;
        case GAP_MSG_LE_CONN_UPDATE_IND:
            result = APP_RESULT_ACCEPT;
            break;
        case GAP_MSG_LE_MODIFY_WHITE_LIST:
            break;
        case GAP_MSG_LE_REMOTE_FEATS_INFO:
        case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        case GAP_MSG_LE_PHY_UPDATE_INFO:
            break;
        default:
            printf("ble_gap_callback: unhandled cb_type 0x%x\r\n", cb_type);
            break;
    }

    return result;
}

static void ble_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    printf("[ble]dev_state: new_state 0x%x, cause 0x%x\r\n", new_state, cause);

    ble_gap_dev_state = new_state;
}

static void ble_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    switch (new_state) {
        case GAP_CONN_STATE_CONNECTING:
            break;
        case GAP_CONN_STATE_DISCONNECTED:
            printf("[ble]disconn: conn_id %d disc_cause 0x%x\r\n", conn_id, disc_cause);
            if (ble_cb) {
                aiio_ble_evt_t evt;

                evt.type = AIIO_BLE_EVT_DISCONNECT;
                evt.disconnect.conn = (void *)conn_id;
                evt.disconnect.reason = disc_cause;
                ble_cb(&evt);
            }
            break;
        case GAP_CONN_STATE_CONNECTED:
            if (ble_mode == AIIO_BLE_MODE_CENTRAL)
            {
                client_send_exchange_mtu_req(conn_id);
            }
            if (ble_cb) {
                aiio_ble_evt_t evt;

                evt.type = AIIO_BLE_EVT_CONNECT;
                evt.connect.conn = (void *)conn_id;
                ble_cb(&evt);
            }
            break;
        case GAP_CONN_STATE_DISCONNECTING:
            break;
        default:
            printf("ble_handle_conn_state_evt: unhandled state %d\r\n", new_state);
            break;
    }
}

static void ble_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t      conn_id;

    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));
    switch (p_gap_msg->subtype) {
        case GAP_MSG_LE_DEV_STATE_CHANGE:
            ble_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                     gap_msg.msg_data.gap_dev_state_change.cause);
            break;
        case GAP_MSG_LE_CONN_STATE_CHANGE:
            ble_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                      gap_msg.msg_data.gap_conn_state_change.new_state,
                                      gap_msg.msg_data.gap_conn_state_change.disc_cause);
            break;
        case GAP_MSG_LE_CONN_PARAM_UPDATE:
            break;
        case GAP_MSG_LE_CONN_MTU_INFO:
            printf("[ble][mtu]id:%d mtu:%d\r\n", conn_id, gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
            if (ble_cb) {
                aiio_ble_evt_t evt;

                evt.type = AIIO_BLE_EVT_MTU;
                evt.mtu.conn = (void *)conn_id;
                evt.mtu.mtu = gap_msg.msg_data.gap_conn_mtu_info.mtu_size;
                ble_cb(&evt);
            }
            break;
        default:
            printf("[ble][gap]unknown subtype %d\r\n", p_gap_msg->subtype);
            break;
    }
}

static void ble_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type) {
        case IO_MSG_TYPE_BT_STATUS:
            ble_handle_gap_msg(&io_msg);
            break;
        default:
            break;
    }
}

static void ble_task(void *p_param)
{
    (void)p_param;
    uint8_t event;
    os_msg_queue_create(&ble_io_queue_handle, MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&ble_evt_queue_handle, MAX_NUMBER_OF_EVENT_MESSAGE, sizeof(uint8_t));

    gap_start_bt_stack(ble_evt_queue_handle, ble_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);

    while (true) {
        if (os_msg_recv(ble_evt_queue_handle, &event, 0xFFFFFFFF) == true) {
            if (event == EVENT_IO_TO_APP) {
                T_IO_MSG io_msg;
                if (os_msg_recv(ble_io_queue_handle, &io_msg, 0) == true) {
                    ble_handle_io_msg(io_msg);
                }
            } else {
                gap_handle_msg(event);
            }
        }
    }
}

static void ble_gap_init(void)
{
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = true;

    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
#if F_BT_LE_SMP_OOB_SUPPORT
    uint8_t auth_oob = false;
#endif
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t  auth_sec_req_enable = false;
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req), &slave_init_mtu_req);

    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
#if F_BT_LE_SMP_OOB_SUPPORT
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
#endif
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);

    gap_config_max_mtu_size(247);

    le_register_app_cb(ble_gap_callback);
}

static int ble_notify_data(void *conn, ble_gatts_attr_t *attr, uint8_t *data, uint16_t length, uint8_t type)
{
    int ret;
    uint16_t mtu;
    uint16_t offset;
    uint16_t send_len;

    if (type == 1) {
        type = GATT_PDU_TYPE_INDICATION;
    } else {
        type = GATT_PDU_TYPE_NOTIFICATION;
    }

    offset = 0;
    if (GAP_CAUSE_SUCCESS != le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu, conn)) {
        return false;
    }

    mtu -= 3;
    while (length > 0) {
        /* calculate send_len */
        send_len = length > mtu ? mtu : length;
        /* send data */
        ret = server_send_data(conn, attr->srv_id, attr->attr_index, data + offset, send_len, type);
        /* set offset */
        offset += send_len;
        length -= send_len;

        if (ret != GAP_CAUSE_SUCCESS) {
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}

static int ble_write_norsp_data(void *conn, uint16_t handle, uint8_t *data, uint16_t length)
{
    int ret;
    uint16_t mtu;
    uint16_t offset;
    uint16_t send_len;

    offset = 0;
    if (GAP_CAUSE_SUCCESS != le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu, conn)) {
        return false;
    }

    mtu -= 3;
    while (length > 0) {
        /* calculate send_len */
        send_len = length > mtu ? mtu : length;
        /* send data */
       // ret = server_send_data(conn, attr->srv_id, attr->attr_index, data + offset, send_len, type);
        ret = gcs_attr_write(conn, GATT_WRITE_TYPE_CMD, handle, send_len, data + offset);
        /* set offset */
        offset += send_len;
        length -= send_len;

        if (ret != GAP_CAUSE_SUCCESS) {
            return AIIO_ERROR;
        }
    }

    return AIIO_OK;
}

static T_APP_RESULT ble_gatt_attr_read_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                        uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;

    printf("[ble]gatt read:%d\r\n", service_id);

    return (cause);
}

static T_APP_RESULT ble_gatt_attr_write_cb(uint8_t conn_id, T_SERVER_ID service_id,
                                         uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                         P_FUN_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;

    printf("[ble]gatt write:%d,%d\r\n", service_id, attrib_index);
    if (ble_cb) {
        aiio_ble_evt_t evt;

        evt.type = AIIO_BLE_EVT_DATA;
        evt.data.handle = attrib_index;
        evt.data.type = AIIO_BLE_WRITE_WITHOUT_RESP;
        evt.data.conn = (void *)conn_id;
        evt.data.data = p_value;
        evt.data.length = length;
        ble_cb(&evt);
    }

    return cause;
}

static void ble_gatt_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index,
                                  uint16_t cccbits)
{
    printf("[ble]gatt cccd, index:%d, cccbits:0x%x\r\n", index, cccbits);
}

static const T_FUN_GATT_SERVICE_CBS ble_service_cbs = {
    ble_gatt_attr_read_cb,
    ble_gatt_attr_write_cb,
    ble_gatt_cccd_update_cb,
};

static void ble_gap_stack_init(void)
{
    T_GAP_DEV_STATE new_state;

    if (ble_gap_stack_flag) {
        return ;
    }

    os_task_create(&ble_task_handle, "ble_task", ble_task, 0, BLE_TASK_STACK_SIZE, 1);

    do {
        os_delay(100);
        le_get_gap_param(GAP_PARAM_DEV_STATE, &new_state);
    } while (new_state.gap_init_state != GAP_INIT_STATE_STACK_READY);

    printf("ble_gap_stack_init success \r\n");
    ble_gap_stack_flag = 1;
}

static SemaphoreHandle_t sem_disc;
void ble_master_gcs_handle_discovery_result(uint8_t conn_id, T_GCS_DISCOVERY_RESULT discov_result)
{
    aiio_ble_evt_t evt;
    aiio_ble_gatt_attr_t aiio_attr;
    aiio_ble_uuid_128_t aiio_uuid = {0};
    if(!ble_cb)
    {
        return ;
    }

    uint16_t i;
    T_GCS_DISCOV_RESULT *p_result_table;
    uint16_t    properties;
    switch (discov_result.discov_type)
    {
    case GCS_ALL_PRIMARY_SRV_DISCOV:
        break;

    case GCS_BY_UUID128_SRV_DISCOV:
        break;

    case GCS_BY_UUID_SRV_DISCOV:
        break;

    case GCS_ALL_CHAR_DISCOV: 
        for (i = 0; i < discov_result.result_num; i++)
        {
            memset(&evt, 0, sizeof(evt));
            memset(&aiio_attr, 0, sizeof(aiio_attr));
            memset(&aiio_uuid, 0, sizeof(aiio_uuid));
            p_result_table = &(discov_result.p_result_table[i]);
            switch (p_result_table->result_type)
            {
            case DISC_RESULT_CHAR_UUID16:
            {
                properties = p_result_table->result_data.char_uuid16_disc_data.properties;
                // printf("CHAR UUID16[%d]: decl_handle 0x%x, properties 0x%x, value_handle 0x%x, uuid16 0x%x\r\n",
                //                 i, p_result_table->result_data.char_uuid16_disc_data.decl_handle,
                //                 p_result_table->result_data.char_uuid16_disc_data.properties,
                //                 p_result_table->result_data.char_uuid16_disc_data.value_handle,
                //                 p_result_table->result_data.char_uuid16_disc_data.uuid16);

                aiio_attr.handle = p_result_table->result_data.char_uuid16_disc_data.value_handle;
                aiio_uuid.uuid.type = AIIO_BLE_UUID_TYPE_16;

                memcpy(((aiio_ble_uuid_16_t *)&aiio_uuid)->val,&(p_result_table->result_data.char_uuid16_disc_data.uuid16), 2);
                ble_reverse_byte(((aiio_ble_uuid_16_t *)&aiio_uuid)->val, 2);

                aiio_attr.uuid = &aiio_uuid;

                evt.type = AIIO_BLE_EVT_DISC;
                evt.disc.conn = (aiio_ble_conn_t)conn_id;
                evt.disc.attr = &aiio_attr;
                ble_cb(&evt);
                break;
            }

            case DISC_RESULT_CHAR_UUID128:
            {
                properties = p_result_table->result_data.char_uuid128_disc_data.properties;
                // printf("CHAR UUID128[%d]:  decl hndl=0x%x, prop=0x%x, value hndl=0x%x, uuid128="UUID_128_FORMAT"\r\n",
                //                 i, p_result_table->result_data.char_uuid128_disc_data.decl_handle,
                //                 p_result_table->result_data.char_uuid128_disc_data.properties,
                //                 p_result_table->result_data.char_uuid128_disc_data.value_handle,
                //                 UUID_128(p_result_table->result_data.char_uuid128_disc_data.uuid128));

                aiio_attr.handle = p_result_table->result_data.char_uuid128_disc_data.value_handle;
                aiio_uuid.uuid.type = AIIO_BLE_UUID_TYPE_128;

                memcpy(((aiio_ble_uuid_128_t *)&aiio_uuid)->val, (p_result_table->result_data.char_uuid128_disc_data.uuid128), 16);
                ble_reverse_byte(((aiio_ble_uuid_128_t *)&aiio_uuid)->val, 16);

                aiio_attr.uuid = &aiio_uuid;

                evt.type = AIIO_BLE_EVT_DISC;
                evt.disc.conn = (aiio_ble_conn_t)conn_id;
                evt.disc.attr = &aiio_attr;
                ble_cb(&evt);
            }

                               
                break;
            default:
                printf("Invalid Discovery Result Type!\r\n");
                break;
            }
        }
        if (sem_disc)
        {
            xSemaphoreGive(sem_disc);
        }
        break;

    case GCS_BY_UUID_CHAR_DISCOV:
        break;

    case GCS_BY_UUID128_CHAR_DISCOV:
        break;

    case GCS_ALL_CHAR_DESC_DISCOV:
        printf("conn_id %d, GCS_ALL_CHAR_DESC_DISCOV, is_success %d\r\n",
                        conn_id, discov_result.is_success);
        for (i = 0; i < discov_result.result_num; i++)
        {
            p_result_table = &(discov_result.p_result_table[i]);
            switch (p_result_table->result_type)
            {
            case DISC_RESULT_CHAR_DESC_UUID16:
                printf("DESC UUID16[%d]: Descriptors handle=0x%x, uuid16=<0x%x>\r\n",
                                i, p_result_table->result_data.char_desc_uuid16_disc_data.handle,
                                p_result_table->result_data.char_desc_uuid16_disc_data.uuid16);


                aiio_attr.handle = p_result_table->result_data.char_desc_uuid16_disc_data.handle;
                break;
            case DISC_RESULT_CHAR_DESC_UUID128:
                printf("DESC UUID128[%d]: Descriptors handle=0x%x, uuid128="UUID_128_FORMAT"\r\n",
                                i, p_result_table->result_data.char_desc_uuid128_disc_data.handle,
                                UUID_128(p_result_table->result_data.char_desc_uuid128_disc_data.uuid128));
                break;

                aiio_attr.handle = p_result_table->result_data.char_desc_uuid128_disc_data.handle;
            default:
                printf("Invalid Discovery Result Type!\r\n");
                break;
            }
        }
        break;

    default:
        printf("Invalid disc type: conn_id %d, discov_type %d\r\n",
                         conn_id, discov_result.discov_type);
        break;
    }

}

T_APP_RESULT master_gcs_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    if (client_id == ble_gcs_client_id)
    {
        T_GCS_CLIENT_CB_DATA *p_gcs_cb_data = (T_GCS_CLIENT_CB_DATA *)p_data;
        printf("p_gcs_cb_data->cb_type[%d]\r\n", p_gcs_cb_data->cb_type);
        switch (p_gcs_cb_data->cb_type)
        {
        case GCS_CLIENT_CB_TYPE_DISC_RESULT:
            ble_master_gcs_handle_discovery_result(conn_id, p_gcs_cb_data->cb_content.discov_result);
            break;
        case GCS_CLIENT_CB_TYPE_READ_RESULT:
                printf("READ RESULT: cause 0x%x, handle 0x%x, value_len %d\r\n",
                       p_gcs_cb_data->cb_content.read_result.cause,
                       p_gcs_cb_data->cb_content.read_result.handle, p_gcs_cb_data->cb_content.read_result.value_size);
            break;
        case GCS_CLIENT_CB_TYPE_WRITE_RESULT:
                printf("WRITE RESULT: cause 0x%x, handle 0x%x, type %d \r\n",
                       p_gcs_cb_data->cb_content.write_result.cause,
                       p_gcs_cb_data->cb_content.write_result.handle,
                       p_gcs_cb_data->cb_content.write_result.type);
            break;
        case GCS_CLIENT_CB_TYPE_NOTIF_IND:
                printf("INDICATION: handle 0x%x, value_size %d, value[%s] \r\n",
                       p_gcs_cb_data->cb_content.notif_ind.handle,
                       p_gcs_cb_data->cb_content.notif_ind.value_size,
                       p_gcs_cb_data->cb_content.notif_ind.p_value);

                if (ble_cb) {
                    aiio_ble_evt_t evt;

                    evt.type = AIIO_BLE_EVT_DATA;
                    evt.data.handle = (aiio_ble_att_handle_t)p_gcs_cb_data->cb_content.notif_ind.handle;
                    evt.data.type = AIIO_BLE_NOTIFY;
                    evt.data.conn = (void *)conn_id;
                    evt.data.data = (uint8_t *)p_gcs_cb_data->cb_content.notif_ind.p_value;
                    evt.data.length = p_gcs_cb_data->cb_content.notif_ind.value_size;
                    ble_cb(&evt);
                }
            break;
            break;
        default:
            break;
        }
    }

    return result;
}

aiio_err_t aiio_ble_init(aiio_ble_mode_t mode, const aiio_ble_config_t *config)
{
    T_GAP_DEV_STATE new_state;

    memcpy(&ble_config_s, config, sizeof ble_config_s);

    while (!(wifi_is_up(RTW_STA_INTERFACE) || wifi_is_up(RTW_AP_INTERFACE))) {
        os_delay(1000);
    }

    le_get_gap_param(GAP_PARAM_DEV_STATE, &new_state);
    if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
    {
        return 0;
    }
    else
    {
        printf("ble_common_init %d \r\n", mode);
        ble_mode = mode;

        bt_trace_init();
        gap_config_max_le_link_num(BLE_MAX_LINKS);
        gap_config_max_le_paired_device(BLE_MAX_LINKS);
        bte_init();
        le_gap_init(BLE_MAX_LINKS);
        ble_gap_init();
        bt_coex_init();
        wifi_btcoex_set_bt_on();

        if (ble_mode == AIIO_BLE_MODE_CENTRAL)
        {
            client_init(1);
            ble_gcs_client_id = gcs_add_client(master_gcs_client_callback, BLE_MAX_LINKS, MASTER_APP_MAX_DISCOV_TABLE_NUM);
            ble_gap_stack_init();
        }

        printf("aiio_ble_init success\r\n");
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_deinit(void)
{
    if (ble_io_queue_handle) {
        os_msg_queue_delete(ble_io_queue_handle);
        ble_io_queue_handle = NULL;
    }
    if (ble_evt_queue_handle) {
        os_msg_queue_delete(ble_evt_queue_handle);
        ble_evt_queue_handle = NULL;
    }
    if (ble_task_handle) {
        os_task_delete(ble_task_handle);
        ble_task_handle = NULL;
    }

    memset(&ble_gap_dev_state, 0, sizeof(T_GAP_DEV_STATE));

    T_GAP_DEV_STATE state;
    le_get_gap_param(GAP_PARAM_DEV_STATE, &state);
    if (state.gap_init_state != GAP_INIT_STATE_STACK_READY) {
        printf("[ble]BT Stack is not running\r\n");
    } else {
        // extern void gcs_delete_client(void);
        // gcs_delete_client();
        bte_deinit();
        bt_trace_uninit();
        ble_gap_stack_flag = 0;
        printf("[ble]BT Stack deinitalized\r\n");
    }

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
    uint8_t  adv_evt_type = GAP_ADTYPE_ADV_NONCONN_IND;
    uint8_t  adv_direct_type = GAP_REMOTE_ADDR_LE_PUBLIC;
    uint8_t  adv_direct_addr[GAP_BD_ADDR_LEN] = {0};
    uint8_t  adv_chann_map = GAP_ADVCHAN_ALL;
    uint8_t  adv_filter_policy = GAP_ADV_FILTER_ANY;
    uint16_t adv_int_min = param->interval_min;
    uint16_t adv_int_max = param->interval_max;

    ble_gap_stack_init();

    if (param->conn_mode != AIIO_BLE_CONN_MODE_NON) {
        adv_evt_type = GAP_ADTYPE_ADV_IND;
    }

    le_adv_set_param(GAP_PARAM_ADV_EVENT_TYPE, sizeof(adv_evt_type), &adv_evt_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR_TYPE, sizeof(adv_direct_type), &adv_direct_type);
    le_adv_set_param(GAP_PARAM_ADV_DIRECT_ADDR, sizeof(adv_direct_addr), adv_direct_addr);
    le_adv_set_param(GAP_PARAM_ADV_CHANNEL_MAP, sizeof(adv_chann_map), &adv_chann_map);
    le_adv_set_param(GAP_PARAM_ADV_FILTER_POLICY, sizeof(adv_filter_policy), &adv_filter_policy);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MIN, sizeof(adv_int_min), &adv_int_min);
    le_adv_set_param(GAP_PARAM_ADV_INTERVAL_MAX, sizeof(adv_int_max), &adv_int_max);

    le_adv_set_param(GAP_PARAM_ADV_DATA, ad_len, (void *)ad);
    le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sd_len, (void *)sd);

    le_adv_start();

    return AIIO_OK;
}

aiio_err_t aiio_ble_adv_stop(void)
{
    le_adv_stop();

    return AIIO_OK;
}

T_APP_RESULT salve_profile_callback(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;

    printf("[ble]profile cb:%d\r\n", service_id);

    if (service_id == SERVICE_PROFILE_GENERAL_ID) {
        T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
        switch (p_param->eventId) {
            case PROFILE_EVT_SRV_REG_COMPLETE: // srv register result event.
                printf("PROFILE_EVT_SRV_REG_COMPLETE: result %d\r\n",
                                p_param->event_data.service_reg_result);
                break;

            case PROFILE_EVT_SEND_DATA_COMPLETE:
                if (p_param->event_data.send_data_result.cause != GAP_SUCCESS) {
                    printf("ble send failed\r\n");
                }
                break;

            default:
                break;
        }
    }

    return app_result;
}

aiio_err_t ble_gatts_add_default_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    server_init(2);

    ble_uuid_convert_srv(&ble_def_service_tbl_1[0], cfg->server_uuid[0]);
    ble_uuid_convert_char(&ble_def_service_tbl_1[BLE_DEF_SERVER_TX_INDEX], cfg->tx_char_uuid[0]);
    ble_uuid_convert_char(&ble_def_service_tbl_1[BLE_DEF_SERVER_RX_INDEX], cfg->rx_char_uuid[0]);

    ble_uuid_convert_srv(&ble_def_service_tbl_2[0], cfg->server_uuid[1]);
    ble_uuid_convert_char(&ble_def_service_tbl_2[BLE_DEF_SERVER_TX_INDEX], cfg->tx_char_uuid[1]);
    ble_uuid_convert_char(&ble_def_service_tbl_2[BLE_DEF_SERVER_RX_INDEX], cfg->rx_char_uuid[1]);

    if (false == server_add_service(&ble_def_srv_id_1, (uint8_t *)ble_def_service_tbl_1, sizeof(ble_def_service_tbl_1), ble_service_cbs)) {
        printf("[ble]reg service1 fail\r\n");
        return AIIO_ERROR;
    }

    if (false == server_add_service(&ble_def_srv_id_2, (uint8_t *)ble_def_service_tbl_2, sizeof(ble_def_service_tbl_2), ble_service_cbs)) {
        printf("[ble]reg service2 fail\r\n");
        return AIIO_ERROR;
    }

    printf("ble_def_srv_id:%d %d\r\n", ble_def_srv_id_1, ble_def_srv_id_2);

    server_register_app_cb(salve_profile_callback);
    gap_config_cccd_not_check(CONFIG_GATT_CCCD_NOT_CHECK);

    ble_def_attr_tx_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_tx_1.attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_rx_1.attr_index = BLE_DEF_SERVER_RX_INDEX;

    ble_def_attr_tx_2.srv_id = ble_def_srv_id_2;
    ble_def_attr_tx_2.attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx_2.srv_id = ble_def_srv_id_2;
    ble_def_attr_rx_2.attr_index = BLE_DEF_SERVER_RX_INDEX;

    handle->tx_char_handle[0] = &ble_def_attr_tx_1;
    handle->rx_char_handle[0] = &ble_def_attr_rx_1;

    handle->tx_char_handle[1] = &ble_def_attr_tx_2;
    handle->rx_char_handle[1] = &ble_def_attr_tx_2;

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_mac(uint8_t *mac)
{
    gap_get_param(GAP_PARAM_BD_ADDR, mac);
    ble_reverse_byte(mac, 6);

    return AIIO_OK;
}

aiio_err_t aiio_ble_set_mac(const uint8_t *mac)
{
    uint8_t bt_addr[6];
    uint8_t local_bd_type = GAP_LOCAL_ADDR_LE_RANDOM;

    memcpy(bt_addr, mac, 6);
    ble_reverse_byte(bt_addr, 6);

    le_cfg_local_identity_address(bt_addr, GAP_IDENT_ADDR_RAND);
	le_set_gap_param(GAP_PARAM_RANDOM_ADDR, 6, bt_addr);
    le_adv_set_param(GAP_PARAM_ADV_LOCAL_ADDR_TYPE, sizeof(local_bd_type), &local_bd_type);

    return AIIO_OK;
}

aiio_err_t aiio_ble_get_limit_power(int8_t *min, int8_t *max)
{
    *min = -10;
    *max = 5;

    return AIIO_OK;
}

uint8_t ble_power2(int power_cfg) {
    // 0x06      -10 dBm
    // 0x1A      0   dBm
    // 0x23      4.5 dBm
    if (power_cfg > 2) {
        return 0x23;
    } else if (power_cfg > -3) {
        return 0x1A;
    } else {
        return 0x06;
    }
}

aiio_err_t aiio_ble_set_conn_power(aiio_ble_conn_t conn, int8_t power)
{
    T_GAP_CAUSE cause;
    bool reset = false;
    uint8_t tx_gain = ble_power2(power);
    cause = le_set_conn_tx_power(conn, reset, tx_gain);
    if( GAP_CAUSE_SUCCESS != cause)
    {
        printf("set conn power error\r\n");
        return AIIO_ERROR;            
    }
    return AIIO_OK;
}

aiio_err_t aiio_ble_set_adv_power(int8_t power)
{
    T_GAP_CAUSE cause;
    uint8_t tx_gain = ble_power2(power);

    cause = le_adv_set_tx_power(0, tx_gain);
    if( GAP_CAUSE_SUCCESS != cause)
    {
        printf("set adv power error\r\n");
        return AIIO_ERROR;            
    }
    return AIIO_OK;

}
aiio_err_t aiio_ble_mtu_req(aiio_ble_conn_t conn, uint16_t mtu)
{
    /* gap_config_max_mtu_size */
    return AIIO_OK;
}

aiio_err_t aiio_ble_mtu_get(aiio_ble_conn_t conn, uint16_t *mtu)
{
    if (le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, mtu, conn) != GAP_CAUSE_SUCCESS)
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
    return (GAP_CAUSE_SUCCESS == le_disconnect(conn)) ? AIIO_OK : AIIO_ERROR;
}

aiio_err_t aiio_ble_connect(uint8_t addr_type, uint8_t *addr, uint32_t timeout)
{
    T_GAP_LE_CONN_REQ_PARAM conn_req_param;
    T_GAP_REMOTE_ADDR_TYPE bd_type = addr_type;
    uint8_t bd_addr[6] = {0};
    memcpy(bd_addr, addr, 6);
    ble_reverse_byte(bd_addr, 6);
    conn_req_param.scan_interval = 0x10;
    conn_req_param.scan_window = 0x10;
    conn_req_param.conn_interval_min = ble_config_s.conn_intv.min_interval;
    conn_req_param.conn_interval_max = ble_config_s.conn_intv.max_interval;
    conn_req_param.conn_latency = ble_config_s.conn_intv.latency;
    conn_req_param.supv_tout = timeout/10;
    conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
    conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);
    le_set_conn_param(GAP_CONN_PARAM_1M, &conn_req_param);

    if( GAP_CAUSE_SUCCESS != le_connect(0, bd_addr, bd_type, addr_type, 2000))  
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

aiio_err_t aiio_ble_scan(const aiio_ble_scan_param_t *cfg)
{
    uint8_t  scan_mode = cfg->type;
    uint16_t scan_interval = cfg->interval;
    uint16_t scan_window = cfg->window;
    uint8_t  scan_filter_policy = GAP_SCAN_FILTER_ANY;
    uint8_t  scan_filter_duplicate = GAP_SCAN_FILTER_DUPLICATE_ENABLE;

    if (cfg->type == AIIO_BLE_SCAN_TYPE_ACTIVE) {
        scan_mode = GAP_SCAN_MODE_ACTIVE;
    }

    int ret = 0;
    ret |=le_scan_set_param(GAP_PARAM_SCAN_MODE, sizeof(scan_mode), &scan_mode);
    ret |=le_scan_set_param(GAP_PARAM_SCAN_INTERVAL, sizeof(scan_interval), &scan_interval);
    ret |=le_scan_set_param(GAP_PARAM_SCAN_WINDOW, sizeof(scan_window), &scan_window);
    ret |=le_scan_set_param(GAP_PARAM_SCAN_FILTER_POLICY, sizeof(scan_filter_policy),
                      &scan_filter_policy);
    ret |=le_scan_set_param(GAP_PARAM_SCAN_FILTER_DUPLICATES, sizeof(scan_filter_duplicate),
                      &scan_filter_duplicate);

    ret |=le_scan_start();
    if(ret !=GAP_CAUSE_SUCCESS)
    {
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

aiio_err_t aiio_ble_scan_stop(void)
{
    le_scan_stop();
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
            ble_write_norsp_data(data->conn, (uint16_t)data->handle, data->data, data->length);
            break;
        default:
            return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_err_t aiio_ble_discover(const aiio_ble_disc_param_t *param, uint16_t timeout)
{
    int ret = AIIO_ERROR;

    sem_disc = xSemaphoreCreateBinary();
    if (!sem_disc) {
        aiio_log_e("[BLE] sem_disc fail\r\n");
        goto _exit;
    }

    T_GAP_CAUSE case_relt = gcs_all_char_discovery(param->conn, param->start_handle, param->end_handle);
    if(GAP_CAUSE_SUCCESS != case_relt)
    {
       aiio_log_e("[BLE] discover failed(err %d)\r\n", case_relt);
       goto _exit;
    }

    if (xSemaphoreTake(sem_disc, timeout / portTICK_PERIOD_MS) != pdTRUE) {
        aiio_log_e("[BLE] disc timeout\r\n");
        goto _exit;
    }

    ret = AIIO_OK;

_exit:
    if (sem_disc) {
        /* disable interrupt? */
        vSemaphoreDelete(sem_disc);
        sem_disc = NULL;
        /* enable interrupt? */
    }

    return ret;
}

aiio_err_t aiio_ble_set_name(const char *name)
{
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, strlen(name), name);

    return AIIO_OK;
}

aiio_err_t ble_gatts_add_blufi_svcs(aiio_ble_default_server_t *cfg, aiio_ble_default_handle_t *handle)
{
    server_init(2);

    ble_uuid_convert_srv(&ble_blufi_service_tbl[0], cfg->server_uuid[0]);
    ble_uuid_convert_char(&ble_blufi_service_tbl[BLE_BLUFI_SERVER_TX_INDEX], cfg->tx_char_uuid[0]);
    ble_uuid_convert_char(&ble_blufi_service_tbl[BLE_BLUFI_SERVER_RX_INDEX], cfg->rx_char_uuid[0]);

    ble_uuid_convert_srv(&ble_def_service_tbl_2[0], cfg->server_uuid[1]);
    ble_uuid_convert_char(&ble_def_service_tbl_2[BLE_DEF_SERVER_TX_INDEX], cfg->tx_char_uuid[1]);
    ble_uuid_convert_char(&ble_def_service_tbl_2[BLE_DEF_SERVER_RX_INDEX], cfg->rx_char_uuid[1]);

    if (false == server_add_service(&ble_def_srv_id_1, (uint8_t *)ble_blufi_service_tbl, sizeof(ble_blufi_service_tbl), ble_service_cbs)) {
        printf("[ble]reg service1 fail\r\n");
        return AIIO_ERROR;
    }

    if (false == server_add_service(&ble_def_srv_id_2, (uint8_t *)ble_def_service_tbl_2, sizeof(ble_def_service_tbl_2), ble_service_cbs)) {
        printf("[ble]reg service2 fail\r\n");
        return AIIO_ERROR;
    }

    printf("ble_blufi_srv_id:%d %d\r\n", ble_def_srv_id_1, ble_def_srv_id_2);

    server_register_app_cb(salve_profile_callback);
    gap_config_cccd_not_check(CONFIG_GATT_CCCD_NOT_CHECK);

    ble_def_attr_tx_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_tx_1.attr_index = BLE_BLUFI_SERVER_TX_INDEX;
    ble_def_attr_rx_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_rx_1.attr_index = BLE_BLUFI_SERVER_RX_INDEX;

    ble_def_attr_tx_2.srv_id = ble_def_srv_id_2;
    ble_def_attr_tx_2.attr_index = BLE_DEF_SERVER_TX_INDEX;
    ble_def_attr_rx_2.srv_id = ble_def_srv_id_2;
    ble_def_attr_rx_2.attr_index = BLE_DEF_SERVER_RX_INDEX;

    handle->tx_char_handle[0] = &ble_def_attr_tx_1;
    handle->rx_char_handle[0] = &ble_def_attr_rx_1;

    handle->tx_char_handle[1] = &ble_def_attr_tx_2;
    handle->rx_char_handle[1] = &ble_def_attr_tx_2;

    return AIIO_OK;
}

aiio_err_t ble_gattc_ccc_cfg(const aiio_ble_ccc_t *cfg)
{
    uint16_t handle = cfg->handle;
    uint16_t length = sizeof(uint16_t);
    uint16_t cccd_bits = cfg->cfg;
    if (gcs_attr_write(cfg->conn, GATT_WRITE_TYPE_REQ, handle, length, &cccd_bits) == GAP_CAUSE_SUCCESS)
    {
        return AIIO_OK;
    }
    return AIIO_ERROR;
}
