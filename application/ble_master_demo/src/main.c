/** @brief ble demo
 *
 *  @file        main.c
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/03/09          <td>V1.0.0          <td>lins            <td>ble demo fist version
 *  <table>
 *
 */

#include "aiio_adapter_include.h"

#define BLE_CONN_TIMEOUT  10000
#define BLE_TARGET_UUID16 0x01A0

#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

#define BLE_SCAN_INTV   100
#define BLE_SCAN_WINDOW 50

#define AT_BLE_LOCAL_NAME_SIZE  29

static uint8_t target_mac[6];

static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};

static aiio_ble_default_server_t ble_server = {
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x01),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x03),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
};

struct ble_adv_field {
    uint8_t length;
    uint8_t type;
    uint8_t value[];
};
typedef int32_t (* ble_adv_parse_func_t) (const struct ble_adv_field *, void *);
static int32_t ble_adv_parse(const uint8_t *data, uint8_t length,
                 ble_adv_parse_func_t func, void *user_data)
{
    const struct ble_adv_field *field;

    while (length > 1) {
        field = (const void *) data;

        if (field->length >= length) {
            return AIIO_ERROR;
        }

        if (func(field, user_data) == AIIO_OK) {
            return AIIO_OK;
        }

        length -= 1 + field->length;
        data += 1 + field->length;
    }

    return AIIO_OK;
}

static int32_t ble_name_parse(const struct ble_adv_field *field, void *user_data)
{
    char *name = (char *)user_data;

#define BT_DATA_NAME_SHORTENED          0x08 /* Shortened name */
#define BT_DATA_NAME_COMPLETE           0x09 /* Complete name */
    switch (field->type) {
        case BT_DATA_NAME_SHORTENED:
        case BT_DATA_NAME_COMPLETE:
            memcpy(name, field->value, field->length - 1);
            return AIIO_OK;
        default:
            return AIIO_ERROR;
    }
}

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");
            break;
        case AIIO_BLE_EVT_MTU:
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            break;
        case AIIO_BLE_EVT_DATA:
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);
            break;
        case AIIO_BLE_EVT_SCAN: {
            char name[AT_BLE_LOCAL_NAME_SIZE] = {0};

            if (event->scan.ad_len > 0) {
                ble_adv_parse(event->scan.ad_data, event->scan.ad_len, ble_name_parse, name);
            }
            if (name[0] == '\0' && event->scan.sd_len > 0) {
                ble_adv_parse(event->scan.sd_data, event->scan.sd_len, ble_name_parse, name);
            }

            if (name[0] != '\0' && strcmp(name, "AXK") == 0) {
                /* stop scan */
                aiio_ble_scan_stop();
                memcpy(target_mac, event->scan.mac, 6);
                aiio_log_i("scan AXK, rssi:%d", event->scan.rssi);
            }
            break;
        }
        default:
            aiio_log_w("undef event!");
    }
}

void ble_print_mac(void)
{
    uint8_t mac[6];

    aiio_ble_get_mac(mac);
    aiio_log_i("ble mac:%02x:%02x:%02x:%02x:%02x:%02x",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

static void ble_conn_task(void *pvParameters)
{
    uint8_t empty_mac[6] = {0};
    while(1) {
        aiio_os_tick_dealy(aiio_os_ms2tick(100));

        if (memcmp(target_mac, empty_mac, 6) == 0) {
            continue;
        }

        if (aiio_ble_connect(AIIO_BLE_ADDR_PUBLIC, target_mac, BLE_CONN_TIMEOUT) != AIIO_OK) {
            aiio_log_e("ble connect fail!");
            continue;
        }

        break;
    }

    while(1) {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }

    aiio_os_thread_delete(NULL);
}

aiio_ble_scan_param_t scan_cfg = {
    .type = AIIO_BLE_SCAN_TYPE_ACTIVE,
    .interval = BLE_SCAN_INTV,
    .window = BLE_SCAN_WINDOW,
};
void ble_master_init(void)
{
    aiio_ble_init(AIIO_BLE_MODE_CENTRAL, &ble_config);

    if (aiio_ble_scan(&scan_cfg) != AIIO_OK) {
        aiio_log_i("scan fail");
        return ;
    }
}

void ble_main(void)
{
    aiio_ble_register_event_cb(aiio_ble_cb);

    ble_master_init();

    ble_print_mac();
}

void aiio_main(void *params)
{
    aiio_log_i("BLE DEMO!");

    ble_main();
    aiio_os_thread_create(NULL, "ble_task", ble_conn_task, 4096, NULL, 15);

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
