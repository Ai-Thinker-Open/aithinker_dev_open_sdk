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

#define AIIO_BLE_DEMO_SLAVE   1
#define AIIO_BLE_DEMO_MASTER  2

#define AIIO_BLE_DEMO_SEL     AIIO_BLE_DEMO_SLAVE

#define BLE_CONN_TIMEOUT 10000
#define BLE_TARGET_MAC {0xa4, 0xc1, 0x38, 0x80, 0xb8, 0x07}

#define BLE_MIN_INTV    200
#define BLE_MAX_INTV    220
#define BLE_LATENCY     0
#define BLE_TIMEOUT     400

#define BLE_ADV_MIN    500
#define BLE_ADV_MAX    520

static aiio_ble_default_handle_t srv_handle;
static aiio_ble_conn_t cur_conn=-1;
static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};
uint8_t adv_data[] = {
    /* Flags */
    0x02,             /* length */
    0x01,             /* type="Flags" */
    0x02 | 0x04,      /* GENERAL | BREDR_NOT_SUPPORTED */
    /* UUID */
    0x05,             /* length */
    0x03,             /* type="16-bit UUID, all listed" */
    0xa0,             /* type="16-bit UUID, all listed" */
    0x01,             /* type="16-bit UUID, all listed" */
    0xb0,             /* type="16-bit UUID, all listed" */
    0x01,             /* type="16-bit UUID, all listed" */
    /* Name */
    0x04,             /* length */
    0x09,             /* type="Complete name" */
    'A','X','K'
};

static aiio_ble_default_server_t ble_server = {
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x01),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x03),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
};

aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = BLE_ADV_MIN,
    .interval_max = BLE_ADV_MAX,
};

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
    switch (event->type) {
        case AIIO_BLE_EVT_DISCONNECT:
            aiio_log_w("ble disconn");
            cur_conn = -1;
#if (AIIO_BLE_DEMO_SEL == AIIO_BLE_DEMO_SLAVE)
            aiio_ble_adv_start(&param, adv_data, sizeof adv_data, NULL, 0);
#endif
            break;
        case AIIO_BLE_EVT_CONNECT:
            aiio_log_i("ble conn");
            cur_conn = event->connect.conn;
            break;
        case AIIO_BLE_EVT_MTU:
            aiio_log_i("mtu update:%d", event->mtu.mtu);
            break;
        case AIIO_BLE_EVT_DATA:
            aiio_log_i("data down handle:%d len:%d", event->data.handle, event->data.length);
            break;
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

static void ble_slave_task(void *pvParameters)
{
    aiio_ble_send_t send_param;
    uint8_t test_data[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};

    while(1) {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));

        if (cur_conn!=-1) {
            /* ble send data */
            send_param.conn = cur_conn;
            send_param.handle =srv_handle.tx_char_handle[0];
            send_param.type = AIIO_BLE_NOTIFY;
            send_param.data = (uint8_t *)test_data;
            send_param.length = (uint16_t)sizeof test_data;
            if (aiio_ble_send(&send_param) != AIIO_OK) {
                aiio_log_w("send data fail");
            }

            // aiio_log_i("srv_handle.tx_char_handle[0]:%d",srv_handle.tx_char_handle[0]);
        }
    }

    aiio_os_thread_delete(NULL);
}

void ble_slave_init(void)
{
    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
    ble_gatts_add_default_svcs(&ble_server, &srv_handle);

    aiio_ble_adv_start(&param, adv_data, sizeof adv_data, NULL, 0);

    aiio_os_thread_create(NULL, "ble_task", ble_slave_task, 4096, NULL, 15);
}

void ble_master_init(void)
{
    uint8_t mac[6] = BLE_TARGET_MAC;
    aiio_ble_init(AIIO_BLE_MODE_CENTRAL, &ble_config);

    if (aiio_ble_connect(AIIO_BLE_ADDR_PUBLIC, mac, BLE_CONN_TIMEOUT) != AIIO_OK) {
        aiio_log_e("ble connect fail!");
    }
}

void ble_main(void)
{
#if (AIIO_BLE_DEMO_SEL == AIIO_BLE_DEMO_SLAVE)
    ble_slave_init();
#else 
    ble_master_init();
#endif

    aiio_ble_register_event_cb(aiio_ble_cb);

    ble_print_mac();
}

void aiio_main(void *params)
{
    aiio_log_i("BLE DEMO!");

    ble_main();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}

