#include "aiio_adapter_include.h"

#define BLE_MIN_INTV 200
#define BLE_MAX_INTV 220
#define BLE_LATENCY 0
#define BLE_TIMEOUT 400

#define BLE_ADV_MIN 500
#define BLE_ADV_MAX 520

#define BLE_MODE 1
#if BLE_MODE
uint8_t adv_data[] = {
    /* Flags */
    0x02,        /* length */
    0x01,        /* type="Flags" */
    0x02 | 0x04, /* GENERAL | BREDR_NOT_SUPPORTED */
    /* UUID */
    0x05, /* length */
    0x03, /* type="16-bit UUID, all listed" */
    0xa0, /* type="16-bit UUID, all listed" */
    0x01, /* type="16-bit UUID, all listed" */
    0xb0, /* type="16-bit UUID, all listed" */
    0x01, /* type="16-bit UUID, all listed" */
    /* Name */
    0x04, /* length */
    0x09, /* type="Complete name" */
    'A', 'X', 'K'};

static aiio_ble_adv_param_t param = {
    .conn_mode = AIIO_BLE_CONN_MODE_UND,
    /* BL602 unsupport */
    .disc_mode = AIIO_BLE_DISC_MODE_GEN,
    .interval_min = BLE_ADV_MIN,
    .interval_max = BLE_ADV_MAX,
};

// 创建服务
static aiio_ble_default_handle_t srv_handle;
static aiio_ble_default_server_t ble_server = {
    .server_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x01),
    .tx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x02),
    .rx_char_uuid[0] = AIIO_BLE_UUID_DECLARE_16(0xa0, 0x03),

    .server_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x01),
    .tx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x02),
    .rx_char_uuid[1] = AIIO_BLE_UUID_DECLARE_16(0xb0, 0x03),
};

// 客户端被连接
int cur_conn_int = -1;
static aiio_ble_conn_t cur_conn = NULL;

#else
aiio_ble_scan_param_t scan_para = {
    .interval = 0x48,
    .type = 0x00,
    .window = 0x48,
};
#endif

static uint8_t bs21_mac_addr[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
static aiio_ble_config_t ble_config = {
    .conn_intv = {
        .min_interval = BLE_MIN_INTV,
        .max_interval = BLE_MAX_INTV,
        .latency = BLE_LATENCY,
        .timeout = BLE_TIMEOUT,
    },
};

static void aiio_ble_cb(aiio_ble_evt_t *event)
{
#if BLE_MODE

#else
    // uint8_t conn_mac_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x76};
#endif

    switch (event->type)
    {
    case AIIO_BLE_EVT_SCAN:
        aiio_log_i("%02x:%02x:%02x:%02x:%02x:%02x", event->scan.mac[0], event->scan.mac[1], event->scan.mac[2], event->scan.mac[3], event->scan.mac[4], event->scan.mac[5]);
        // if((event->scan.mac[0] == 0x11) && (event->scan.mac[1] == 0x22)&& (event->scan.mac[2] == 0x33) 
        //     && (event->scan.mac[3] == 0x44)&& (event->scan.mac[4] == 0x55)&& (event->scan.mac[5] == 0x76))
        // {
        //     // 停止扫描
        //     aiio_ble_scan_stop();

        //     // 连接蓝牙
        //     aiio_ble_connect(AIIO_BLE_ADDR_PUBLIC, event->scan.mac, 1000);
        // }
        break;
    case AIIO_BLE_EVT_CONNECT:
        aiio_log_i("conn id:%d \r\n", *(uint16_t *)event->connect.conn);
#if BLE_MODE
        // 蓝牙从机设备初始化(广播)
        *(int *)cur_conn = *(uint16_t *)event->connect.conn;
#else
        // 蓝牙中心设备初始化(扫描)
        aiio_ble_mtu_req(event->connect.conn,150);
        // aiio_ble_disconnect(conn_mac_addr);
#endif
        break;
    case AIIO_BLE_EVT_DATA:
        for (int i = 0; i < event->data.length; i++)
        {
            aiio_log_i("0x%02x", event->data.data[i]);
        }
        break;
    case AIIO_BLE_EVT_DISCONNECT:
        aiio_log_i("disconn id:%d \r\n", *(uint16_t *)event->disconnect.conn);
#if BLE_MODE
        // 蓝牙从机设备初始化(广播)
        *(int *)cur_conn = -1;
        aiio_ble_restart_advertising();
#else
        // 蓝牙中心设备初始化(扫描)

#endif
        break;
    default:
        aiio_log_w("undef event!");
    }
}

void aiio_main(void *params)
{
    params = params;
    uint8_t bs21_get_mac_addr[6];

#if BLE_MODE
    // 蓝牙从机设备初始化(广播)
    cur_conn = (aiio_ble_conn_t *)&cur_conn_int;
    aiio_ble_init(AIIO_BLE_MODE_PERIPHERAL, &ble_config);
#else
    // 蓝牙中心设备初始化(扫描)
    aiio_ble_init(AIIO_BLE_MODE_CENTRAL, &ble_config);
#endif
    // 设置蓝牙地址
    aiio_ble_set_mac(bs21_mac_addr);

    // 获取蓝牙地址
    aiio_ble_get_mac(bs21_get_mac_addr);
    aiio_log_i("%02x:%02x:%02x:%02x:%02x:%02x", bs21_get_mac_addr[0], bs21_get_mac_addr[1], bs21_get_mac_addr[2], bs21_get_mac_addr[3], bs21_get_mac_addr[4], bs21_get_mac_addr[5]);

    // 设置蓝牙名称
    aiio_ble_set_name("BS21 AXK TEST");

    // 注册事件回调函数
    aiio_ble_register_event_cb(aiio_ble_cb);

#if BLE_MODE
    ble_gatts_add_default_svcs(&ble_server, &srv_handle);

    // 启动蓝牙广播
    aiio_ble_adv_start(&param, adv_data, sizeof(adv_data), NULL, 0);

    aiio_ble_send_t send_param;
    uint8_t test_data[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
#else
    // 启动扫描
    aiio_ble_scan(&scan_para);
    // // 停止扫描
    // aiio_ble_scan_stop();

    // uint8_t conn_mac_addr[6] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x76};

    // aiio_ble_connect(AIIO_BLE_ADDR_PUBLIC, conn_mac_addr, 1000);
#endif

    while (1)
    {
#if BLE_MODE
        if (*(int *)cur_conn != -1)
        {
            /* ble send data */
            send_param.conn = cur_conn;
            send_param.handle = srv_handle.tx_char_handle[0];
            send_param.type = AIIO_BLE_NOTIFY;
            send_param.data = (uint8_t *)test_data;
            send_param.length = (uint16_t)sizeof test_data;
            if (aiio_ble_send(&send_param) != AIIO_OK)
            {
                aiio_log_w("send data fail");
            }

            // aiio_log_i("srv_handle.tx_char_handle[0]:%d",srv_handle.tx_char_handle[0]);
        }
#else

#endif
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
