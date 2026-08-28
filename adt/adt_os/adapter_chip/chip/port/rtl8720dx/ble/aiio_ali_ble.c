#include "aiio_ali_ble.h"
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

#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};

typedef struct
{
    uint8_t srv_id;
    uint8_t attr_index;
} ble_gatts_attr_t;

typedef struct {
    aiio_ble_uuid_t *server_uuid[1];
    aiio_ble_uuid_t *rc_char_uuid[1];
    aiio_ble_uuid_t *wc_char_uuid[1];
    aiio_ble_uuid_t *ic_char_uuid[1];
    aiio_ble_uuid_t *wwnrc_char_uuid[1];
    aiio_ble_uuid_t *nc_char_uuid[1];
} aiio_ali_ble_default_server_t;

typedef struct
{
    aiio_ble_uuid_16_t srv_cmd_uuid;
    aiio_ble_uuid_16_t cmd_rc_uuid;
    aiio_ble_uuid_16_t cmd_wc_uuid;
    aiio_ble_uuid_16_t cmd_ic_uuid;
    aiio_ble_uuid_16_t cmd_wwnrc_uuid;
    aiio_ble_uuid_16_t cmd_nc_uuid;
} aiio_ali_ble_config_t;

typedef struct {
    /**
     * This is one of the aiio_ble_data_type_t codes.
     */
    uint8_t type;
    uint8_t *data;
    uint16_t length;
    ali_aiio_ble_att_handle_t handle;
    aiio_ble_conn_t conn;
} ali_aiio_ble_send_t;

static aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = 60,
    .interval_max = 60,
};

static aiio_ble_conn_t  ble_conn = NULL;
ais_bt_init_t * bt_init_info = NULL;

static void (*g_indication_txdone)(uint8_t res);

uint8_t ali_ble_def_srv_uuid1[16];
T_ATTRIB_APPL ali_ble_def_service_tbl_1[] = {
    //服务
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_16BIT_SIZE,
        ali_ble_def_srv_uuid1,
        GATT_PERM_READ
    },
    //特征1
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ
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
    //特征2
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_READ | GATT_CHAR_PROP_WRITE)
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
    //特征3
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_READ | GATT_CHAR_PROP_INDICATE)
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
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),
        {
         LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
         LO_WORD(GATT_CLIENT_CHAR_CONFIG_INDICATE),
         HI_WORD(GATT_CLIENT_CHAR_CONFIG_INDICATE)},
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    },
    //特征4
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_READ | GATT_CHAR_PROP_WRITE_NO_RSP)
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
    //特征5
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_READ | GATT_CHAR_PROP_NOTIFY)
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
     //客户端特征配置描述符（CCCD）
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

static ali_aiio_ble_default_handle_t srv_handle;

static T_SERVER_ID     ble_def_srv_id_1;
static aiio_ble_cb_t ble_cb;

ble_gatts_attr_t ble_def_attr_rc_1;
ble_gatts_attr_t ble_def_attr_wc_1;
ble_gatts_attr_t ble_def_attr_ic_1;
ble_gatts_attr_t ble_def_attr_wwnrc_1;
ble_gatts_attr_t ble_def_attr_nc_1;

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
    if(attrib_index == 4){
        if (bt_init_info && bt_init_info->wc.on_write) {
            bt_init_info->wc.on_write(p_value, length);
        }
    }
    // if (ble_cb) {
    //     aiio_ble_evt_t evt;

    //     evt.type = AIIO_BLE_EVT_DATA;
    //     evt.data.handle = attrib_index;
    //     evt.data.type = AIIO_BLE_WRITE_WITHOUT_RESP;
    //     evt.data.conn = (void *)conn_id;
    //     evt.data.data = p_value;
    //     evt.data.length = length;
    //     ble_cb(&evt);
    // }

    return cause;
}

static void ble_gatt_cccd_update_cb(uint8_t conn_id, T_SERVER_ID service_id, uint16_t index,
                                  uint16_t cccbits)
{
    ais_ccc_value_t val;
    printf("[ble]gatt cccd, index:%d, cccbits:0x%x\r\n", index, cccbits);
    if(index == 7){
        val = AIS_CCC_VALUE_INDICATE;
        if (bt_init_info && bt_init_info->ic.on_ccc_change) {
            bt_init_info->ic.on_ccc_change(val);
        }
    }else if(index == 12){
        val = AIS_CCC_VALUE_NOTIFY;
        if (bt_init_info && bt_init_info->nc.on_ccc_change) {
            bt_init_info->nc.on_ccc_change(val);
        }
    }
    
}

static const T_FUN_GATT_SERVICE_CBS ble_service_cbs = {
    ble_gatt_attr_read_cb,
    ble_gatt_attr_write_cb,
    ble_gatt_cccd_update_cb,
};

aiio_ali_ble_config_t aiio_ali_ble_config = {
    .srv_cmd_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XB3}},
    .cmd_rc_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XD4}},
    .cmd_wc_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XD5}},
    .cmd_ic_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XD6}},
    .cmd_wwnrc_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XD7}},
    .cmd_nc_uuid = {.uuid.type = AIIO_BLE_UUID_TYPE_16, .val = {0xFE, 0XD8}},
};

T_APP_RESULT send_callback_function(T_SERVER_ID service_id, void *p_data) {
    T_SERVER_APP_CB_DATA *p_param = (T_SERVER_APP_CB_DATA *)p_data;
    printf("==== p_param->eventId : %d\r\n", p_param->eventId);
    switch (p_param->eventId)
    {
        case PROFILE_EVT_SRV_REG_COMPLETE:// srv register result event.
            printf("PROFILE_EVT_SRV_REG_COMPLETE: result %d",
                            p_param->event_data.service_reg_result);
            break;

        case PROFILE_EVT_SEND_DATA_COMPLETE:
            printf("PROFILE_EVT_SEND_DATA_COMPLETE: conn_id %d, cause 0x%x, service_id %d, attrib_idx 0x%x, credits %d\r\n",
                            p_param->event_data.send_data_result.conn_id,
                            p_param->event_data.send_data_result.cause,
                            p_param->event_data.send_data_result.service_id,
                            p_param->event_data.send_data_result.attrib_idx,
                            p_param->event_data.send_data_result.credits);
            if (p_param->event_data.send_data_result.cause == GAP_SUCCESS)
            {
                printf("PROFILE_EVT_SEND_DATA_COMPLETE success\r\n");
                g_indication_txdone(0);
            }
    }
    return APP_SUCCESS;
}

extern void ble_uuid_convert_srv(T_ATTRIB_APPL *tbl, aiio_ble_uuid_t *src);

aiio_err_t ali_ble_gatts_add_default_svcs(aiio_ali_ble_default_server_t *cfg, ali_aiio_ble_default_handle_t *handle)
{
    ais_char_init_t *c;

    server_init(1);

    server_register_app_cb(send_callback_function);

    ble_uuid_convert_srv(&ali_ble_def_service_tbl_1[0], cfg->server_uuid[0]);

    /* rc */
    ble_uuid_convert_char(&ali_ble_def_service_tbl_1[2], cfg->rc_char_uuid[0]);

    /* wc */
    ble_uuid_convert_char(&ali_ble_def_service_tbl_1[4], cfg->wc_char_uuid[0]);

    /* ic */
    ble_uuid_convert_char(&ali_ble_def_service_tbl_1[6], cfg->ic_char_uuid[0]);

    /* wwnrc */
    ble_uuid_convert_char(&ali_ble_def_service_tbl_1[9], cfg->wwnrc_char_uuid[0]);

    /* nc */
    ble_uuid_convert_char(&ali_ble_def_service_tbl_1[11], cfg->nc_char_uuid[0]);
    

    if (false == server_add_service(&ble_def_srv_id_1, (uint8_t *)ali_ble_def_service_tbl_1, sizeof(ali_ble_def_service_tbl_1), ble_service_cbs)) {
        printf("[ble]reg service1 fail\r\n");
        return AIIO_ERROR;
    }

    printf("ble_def_srv_id:%d\r\n", ble_def_srv_id_1);

    // server_register_app_cb(ali_salve_profile_callback);
    // gap_config_cccd_not_check(CONFIG_GATT_CCCD_NOT_CHECK);

    ble_def_attr_rc_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_rc_1.attr_index = 2;
    ble_def_attr_wc_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_wc_1.attr_index = 4;
    ble_def_attr_ic_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_ic_1.attr_index = 6;
    ble_def_attr_wwnrc_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_wwnrc_1.attr_index = 9;
    ble_def_attr_nc_1.srv_id = ble_def_srv_id_1;
    ble_def_attr_nc_1.attr_index = 11;

    handle->rc_char_handle[0] = &ble_def_attr_rc_1;
    handle->wc_char_handle[0] = &ble_def_attr_wc_1;
    handle->ic_char_handle[0] = &ble_def_attr_ic_1;
    handle->wwnrc_char_handle[0] = &ble_def_attr_wwnrc_1;
    handle->nc_char_handle[0] = &ble_def_attr_nc_1;

    return AIIO_OK;
}

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) 
    {
        case AIIO_BLE_EVT_DISCONNECT:
        {
            aiio_log_w("ble disconn");
            ble_conn = event->disconnect.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
            ble_conn = NULL;
            if (bt_init_info && (bt_init_info->on_disconnected)) {
                bt_init_info->on_disconnected();
            }
        }
        break;

        case AIIO_BLE_EVT_CONNECT:
        {
            aiio_log_i("ble conn");
            ble_conn = event->connect.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
            if (bt_init_info && (bt_init_info->on_connected)) {
                bt_init_info->on_connected();
            }
        }
        break;

        case AIIO_BLE_EVT_MTU:
        {
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            ble_conn = event->mtu.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
        }
        break;

        case AIIO_BLE_EVT_DATA:
        {
            ble_conn = event->data.conn;
            aiio_log_i("ble_conn = %p \r\n", ble_conn);
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);
            // aiio_HexPrint("ble receive data", (uint8_t *)event->data.data, event->data.length);  
        }
        break;

        default:
            aiio_log_w("undef event!");
            break;
    }
}


ais_err_t ble_stack_init(ais_bt_init_t *info)
{
    bt_init_info = info;

    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);

    aiio_ali_ble_default_server_t cfg = {
        .server_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.srv_cmd_uuid,
        .rc_char_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.cmd_rc_uuid,
        .wc_char_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.cmd_wc_uuid,
        .ic_char_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.cmd_ic_uuid,
        .wwnrc_char_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.cmd_wwnrc_uuid,
        .nc_char_uuid[0] = (aiio_ble_uuid_t *)&aiio_ali_ble_config.cmd_nc_uuid,
    };

    ali_ble_gatts_add_default_svcs(&cfg, &srv_handle);

    aiio_ble_register_event_cb(aiio_ble_cb);

    return AIS_ERR_SUCCESS;
}

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
    printf("adv->vdata.len:%d\n", adv->vdata.len);
    for(int i = 0; i < adv->vdata.len; i++){
        printf("%x ", adv->vdata.data[i],adv->vdata.data);
    }
    printf("\n");
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
        printf("Advertising failed to start (err %d)\n", err);
        // dump_awss_status(SUB_ERRCODE_BLE_START_ADV_FAIL, "start adv fail:%d", err);
        return AIS_ERR_ADV_FAIL;
    }

    return 0;
}
static int ali_ble_notify_data(void *conn, ble_gatts_attr_t *attr, uint8_t *data, uint16_t length, uint8_t type)
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
        printf("length = %d, mtu = %d\n", length, mtu);
        /* calculate send_len */
        send_len = length > mtu ? mtu : length;
        /* send data */
        printf("ali_ble_notify_data attr->attr_index=%d\n", attr->attr_index);
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

aiio_err_t ali_aiio_ble_send(const ali_aiio_ble_send_t *data)
{
    switch (data->type) {
        case AIIO_BLE_NOTIFY:
            ali_ble_notify_data(data->conn, data->handle, data->data, data->length, 0);
            break;
        case AIIO_BLE_INDICATE:
            ali_ble_notify_data(data->conn, data->handle, data->data, data->length, 1);
            break;
        default:
            return AIIO_ERROR;
    }

    return AIIO_OK;
}

ais_err_t ble_send_indication(uint8_t *p_data, uint16_t length, void (*txdone)(uint8_t res))
{
    int err;
    ali_aiio_ble_send_t ble_send_data = {0};

    ble_send_data.type = AIIO_BLE_INDICATE;
    ble_send_data.data = p_data;
    ble_send_data.length = length;
    ble_send_data.handle = srv_handle.ic_char_handle[0];
    ble_send_data.conn = ble_conn;

    err = ali_aiio_ble_send(&ble_send_data);
    printf("ble_send_indication err:%d\n", err);
    if (err) {
        return AIS_ERR_GATT_INDICATE_FAIL;
    } else {
        g_indication_txdone = txdone;
        return AIS_ERR_SUCCESS;
    }
}

ais_err_t ble_send_notification(uint8_t *p_data, uint16_t length)
{
    int err;
    ali_aiio_ble_send_t ble_send_data = {0};

    ble_send_data.type = AIIO_BLE_NOTIFY;
    ble_send_data.data = p_data;
    ble_send_data.length = length;
    ble_send_data.handle = srv_handle.nc_char_handle[0];
    ble_send_data.conn = ble_conn;

    err = ali_aiio_ble_send(&ble_send_data);

    if (err) {
        return AIS_ERR_GATT_NOTIFY_FAIL;
    } else {
        return AIS_ERR_SUCCESS;
    }
}

ais_err_t ble_get_mac(uint8_t *mac)
{
    ais_err_t    err;

    err = aiio_wifi_sta_mac_get(mac);

    return err;
}

int ble_get_att_mtu(uint16_t *att_mtu)
{
    return aiio_ble_mtu_get(ble_conn, att_mtu);
}