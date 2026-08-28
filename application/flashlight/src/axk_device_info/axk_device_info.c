#include "axk_device_info.h"
#include "axk_at.h"
#include "axk_at_cmd.h"
#include "axk_mqtt.h"
#include "axk_nvs_info.h"

#include "aiio_wifi.h"

#include <stdlib.h>

static void axk_device_info_init(void);

axk_device_info_t g_axk_device_info = {
    .init = axk_device_info_init,
    .mac_str = {0},
    .deviceCode = {0},
    .productCode = {0},
};

static void axk_device_info_init(void)
{
    uint8_t mac[6];
    aiio_wifi_sta_mac_get(mac);
    sprintf((char *)g_axk_device_info.mac_str, "%02X%02X%02X%02X%02X%02X",
            mac[0], mac[1], mac[2],
            mac[3], mac[4], mac[5]);

    aiio_log_i("mac:%02x%02x%02x%02x%02x%02x", mac[0], mac[1],
               mac[2], mac[3], mac[4], mac[5]);
    aiio_log_i("mac str: %s", g_axk_device_info.mac_str);

    uint64_t mac_u64 = mac[0];
    mac_u64 <<= 8;
    mac_u64 |= mac[1];
    mac_u64 <<= 8;
    mac_u64 |= mac[2];
    mac_u64 <<= 8;
    mac_u64 |= mac[3];
    mac_u64 <<= 8;
    mac_u64 |= mac[4];
    mac_u64 <<= 8;
    mac_u64 |= mac[5];
    sprintf((char *)g_axk_device_info.deviceCode, "%lld", mac_u64);
    aiio_log_i("deviceCode: %s", g_axk_device_info.deviceCode);

    g_axk_mqtt.modify_client_id(g_axk_device_info.mac_str);
}

int at_query_productCode_cmd(uint32_t argc, const char** argv)
{
    axk_at_printf("\"pCode\":\"%s\"\r\n", g_axk_device_info.productCode);
    AT_RESPONSE_OK;

    return 0;
}

int at_setup_productCode_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;
    uint8_t productCode[16] = {0};
    int len = strlen((const char *)argv[0]);

    if (len <= 2)
    {
        goto __exit;
    }

    if (argv[0][0] != '\"' || argv[0][len - 1] != '\"')
    {
        goto __exit;
    }

    int cp_size = (len - 2) <= (sizeof(productCode) - 1) ? (len - 2) : (sizeof(productCode) - 1);

    strncpy((char *)productCode, &argv[0][1], cp_size);
    if (memcmp((const char *)productCode, (const char *)g_axk_device_info.productCode, sizeof(g_axk_device_info.productCode)) != 0)
    {
        memcpy(g_axk_device_info.productCode, productCode, sizeof(g_axk_device_info.productCode));
        g_nvs_info.set_product_code(g_axk_device_info.productCode, sizeof(g_axk_device_info.productCode));
    }

    AT_RESPONSE_OK;

    ret = 0;

__exit:
    return ret;
}

int at_query_fw_version_cmd(uint32_t argc, const char** argv)
{
    axk_at_printf("\"fv\":\"%s\"\r\n", FW_VER);
    AT_RESPONSE_OK;

    return 0;
}

int at_query_deviceCode_cmd(uint32_t argc, const char** argv)
{
    axk_at_printf("\"dCode\":\"%s\"\r\n", g_axk_device_info.deviceCode);
    AT_RESPONSE_OK;

    return 0;
}

int at_query_mcu_version_cmd(uint32_t argc, const char** argv)
{
    axk_at_printf("\"mv\":\"%s\"\r\n", g_axk_device_info.mcu_ver);
    AT_RESPONSE_OK;

    return 0;
}

int at_setup_mcu_version_cmd(uint32_t argc, const char** argv)
{
    //AT+MCUVER="1.0.0"
    int ret = -1;
    uint8_t mcu_ver[6] = {0};
    int len = strlen((const char *)argv[0]);

    if (len != 7)
    {
        goto __exit;
    }

    if (argv[0][0] != '\"' || argv[0][len - 1] != '\"')
    {
        goto __exit;
    }

    if (argv[0][2] != '.' || argv[0][4] != '.')
    {
        goto __exit;
    }

    strncpy((char *)mcu_ver, &argv[0][1], sizeof(mcu_ver) - 1);
    if (memcmp((const char *)mcu_ver, (const char *)g_axk_device_info.mcu_ver, sizeof(g_axk_device_info.mcu_ver)) != 0)
    {
        memcpy(g_axk_device_info.mcu_ver, mcu_ver, sizeof(g_axk_device_info.mcu_ver));
        g_nvs_info.set_mcu_ver(g_axk_device_info.mcu_ver);
    }

    AT_RESPONSE_OK;

    ret = 0;

__exit:
    return ret;
}

int at_query_sta_mac_cmd(uint32_t argc, const char** argv)
{
    axk_at_printf("+STAMAC:%s\r\n", g_axk_device_info.mac_str);
    AT_RESPONSE_OK;

    return 0;
}
