#include "axk_nvs_info.h"
#include "axk_nvs.h"
#include "axk_at.h"
#include "axk_device_info.h"
#include "axk_at_cmd.h"
#include "axk_mqtt.h"
#include "axk_sleep.h"

#include "aiio_os_port.h"

#include "bl_sys.h"

static void axk_nvs_get_wifi_mode(axk_wifi_mode_t *mode);
static int32_t axk_nvs_set_wifi_mode(axk_wifi_mode_t mode);
static int32_t axk_nvs_get_sta_info(axk_sta_info_t *sta_info);
static int32_t axk_nvs_set_sta_info(const axk_sta_info_t sta_info);
static void axk_nvs_load_info(void);
static int32_t axk_nvs_set_product_code(uint8_t *code, uint8_t len);
static int32_t axk_nvs_get_product_code(uint8_t *code);
static int32_t axk_nvs_set_mqtt_info(axk_mqtt_info_t *info);
static int32_t axk_nvs_set_mcu_ver(uint8_t *version);
static int32_t axk_nvs_get_mcu_ver(uint8_t *version);
static int32_t axk_nvs_set_mqtt_save_flag(uint8_t *is_save);
static int32_t axk_nvs_get_auto_sleep_info(axk_sleep_info_t *info);
static int32_t axk_nvs_set_auto_sleep_info(const axk_sleep_info_t info);

axk_nvs_info_t g_nvs_info =
{
    .get_wifi_mode = axk_nvs_get_wifi_mode,
    .set_wifi_mode = axk_nvs_set_wifi_mode,
    .get_sta_info = axk_nvs_get_sta_info,
    .set_sta_info = axk_nvs_set_sta_info,
    .load_info = axk_nvs_load_info,
    .set_product_code = axk_nvs_set_product_code,
    .get_product_code = axk_nvs_get_product_code,
    .set_mqtt_info = axk_nvs_set_mqtt_info,
    .set_mqtt_save_flag = axk_nvs_set_mqtt_save_flag,
    .get_mcu_ver = axk_nvs_get_mcu_ver,
    .set_mcu_ver = axk_nvs_set_mcu_ver,
    .get_auto_sleep_info = axk_nvs_get_auto_sleep_info,
    .set_auto_sleep_info = axk_nvs_set_auto_sleep_info,
};

static void axk_nvs_get_wifi_mode(axk_wifi_mode_t *mode)
{
    uint8_t wifi_mode;
    size_t len = 0;

    g_axk_nvs.get_blob(WIFI_MODE_KEY, &wifi_mode, 1, &len);

    if (len > 0)
    {
        *mode = (axk_wifi_mode_t)wifi_mode;
    }
    else
    {
        aiio_log_w("nvs get wifi mode failed");
        *mode = AXK_WIFI_AP;
    }
}

static int32_t axk_nvs_set_wifi_mode(axk_wifi_mode_t mode)
{
    axk_wifi_mode_t wifi_mode = mode;
    aiio_nvs_err_code_t ret;

    ret = g_axk_nvs.set_blob(WIFI_MODE_KEY, &wifi_mode, 1);
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set wifi mode failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_sta_info(axk_sta_info_t *sta_info)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(WIFI_CONN_STA_INFO_KEY, sta_info, sizeof(axk_sta_info_t), &len);
    if (len != sizeof(axk_sta_info_t))
    {
        aiio_log_e("nvs get sta info failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_sta_info(const axk_sta_info_t sta_info)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(WIFI_CONN_STA_INFO_KEY, &sta_info, sizeof(axk_sta_info_t));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set sta info failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static uint8_t axk_nvs_get_magic(void)
{
    uint8_t magic = 0;
    size_t len = 0;

    g_axk_nvs.get_blob(NVS_MAGIC_KEY, &magic, sizeof(magic), &len);
    if (len != sizeof(magic))
    {
        aiio_log_e("nvs get magic info failed");
    }

    return magic;
}

static int32_t axk_nvs_set_magic(void)
{
    int32_t ret;
    uint8_t magic = NVS_MAGIC_VALUE;

    ret = g_axk_nvs.set_blob(NVS_MAGIC_KEY, &magic, sizeof(magic));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set sta info failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_product_code(uint8_t *code, uint8_t len)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(PRODUCT_CODE_KEY, code, len);
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set product code failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_product_code(uint8_t *code)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(PRODUCT_CODE_KEY, code, sizeof(g_axk_device_info.productCode), &len);
    if (len != sizeof(g_axk_device_info.productCode))
    {
        aiio_log_e("nvs get product code failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_mcu_ver(uint8_t *version)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(MCU_VER_KEY, version, sizeof(g_axk_device_info.mcu_ver));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set mcu version failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_mcu_ver(uint8_t *version)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(MCU_VER_KEY, version, sizeof(g_axk_device_info.mcu_ver), &len);
    if (len != sizeof(g_axk_device_info.mcu_ver))
    {
        aiio_log_e("nvs get mcu version failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_mqtt_info(axk_mqtt_info_t *info)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(MQTT_INFO_KEY, info, sizeof(axk_mqtt_info_t));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set mqtt info failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_mqtt_info(axk_mqtt_info_t *info)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(MQTT_INFO_KEY, info, sizeof(axk_mqtt_info_t), &len);
    if (len != sizeof(axk_mqtt_info_t))
    {
        aiio_log_e("nvs get mqtt info failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_mqtt_save_flag(uint8_t *is_save)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(MQTT_SAVE_KEY, is_save, sizeof(uint8_t), &len);
    if (len != sizeof(uint8_t))
    {
        aiio_log_e("nvs get mqtt save flag failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_mqtt_save_flag(uint8_t *is_save)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(MQTT_SAVE_KEY, is_save, sizeof(uint8_t));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set mqtt save flag failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_get_auto_sleep_info(axk_sleep_info_t *info)
{
    int32_t ret = -1;
    size_t len = 0;

    g_axk_nvs.get_blob(AUTO_SLEEP_KEY, info, sizeof(axk_sleep_info_t), &len);
    if (len != sizeof(axk_sleep_info_t))
    {
        aiio_log_e("nvs get auto sleep info failed");
        ret = -1;
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_nvs_set_auto_sleep_info(const axk_sleep_info_t info)
{
    int32_t ret;

    ret = g_axk_nvs.set_blob(AUTO_SLEEP_KEY, &info, sizeof(axk_sleep_info_t));
    if (ret != AIIO_EF_NO_ERR)
    {
        aiio_log_e("nvs set auto sleep info failed, ret:%d", ret);
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static void axk_nvs_restore_info(void)
{
    axk_nvs_set_wifi_mode(AXK_WIFI_AP);
    axk_sta_info_t sta_info = {0};
    axk_nvs_set_sta_info(sta_info);
    memset(g_axk_device_info.productCode, 0, sizeof(g_axk_device_info.productCode));
    axk_nvs_set_product_code(g_axk_device_info.productCode, sizeof(g_axk_device_info.productCode));
    memset(g_axk_device_info.mcu_ver, 0, sizeof(g_axk_device_info.mcu_ver));
    axk_nvs_set_mcu_ver(g_axk_device_info.mcu_ver);
    uint8_t save_flag = 0;
    axk_nvs_set_mqtt_save_flag(&save_flag);
    //MQTT INFO
    g_axk_mqtt.set_info_default();
    axk_mqtt_info_t *mqtt_conn_info = g_axk_mqtt.get_conn_info();
    axk_nvs_set_mqtt_info(mqtt_conn_info);
    //auto sleep
    axk_sleep_info_t sleep_info = {.enable = 1};
    axk_nvs_set_auto_sleep_info((const axk_sleep_info_t)sleep_info);
}

int at_exe_restore_cmd(uint32_t argc, const char** argv)
{
    axk_nvs_restore_info();

    AT_RESPONSE_OK;
    aiio_os_tick_dealy(100);
    bl_sys_reset_por();
    return 0;
}

int at_exe_echo_on_cmd(uint32_t argc, const char** argv)
{
    g_at_cmd_config.echo = 1;

    AT_RESPONSE_OK;
    return 0;
}

int at_exe_echo_off_cmd(uint32_t argc, const char** argv)
{
    g_at_cmd_config.echo = 0;

    AT_RESPONSE_OK;
    return 0;
}

static void axk_nvs_load_info(void)
{
    if (axk_nvs_get_magic() == NVS_MAGIC_VALUE)
    {
        // wifi mode
        axk_wifi_mode_t wifi_mode;
        axk_nvs_get_wifi_mode(&wifi_mode);
        g_axk_wifi.set_mode(wifi_mode);
        // sta info
        axk_sta_info_t sta_info;
        axk_nvs_get_sta_info(&sta_info);
        g_axk_wifi.set_sta_info(&sta_info);
        g_nvs_info.get_product_code(g_axk_device_info.productCode);
        axk_nvs_get_mcu_ver(g_axk_device_info.mcu_ver);
        // auto sleep
        axk_sleep_info_t sleep_info = {.enable = 1};
        axk_nvs_get_auto_sleep_info(&sleep_info);
        g_axk_sleep.set_info((const axk_sleep_info_t *)&sleep_info);
        
        uint8_t save_flag = 0;
        if (axk_nvs_get_mqtt_save_flag(&save_flag) == 0)
        {
            if (save_flag != 0)
            {
                //MQTT INFO
                axk_mqtt_info_t *mqtt_conn_info = g_axk_mqtt.get_conn_info();
                axk_nvs_get_mqtt_info(mqtt_conn_info);
            }
        }
    }
    else
    {
        axk_nvs_set_magic();
        axk_nvs_restore_info();
    }
}