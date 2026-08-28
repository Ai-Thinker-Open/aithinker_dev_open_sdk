#include "axk_at_network.h"
#include "aiio_log.h"
#include "axk_wifi_init.h"
#include "axk_at.h"
#include "axk_at_cmd.h"
#include "aiio_wifi.h"
#include "axk_http_server.h"
#include "axk_udp.h"

static axk_wifi_cfg_type_t axk_get_wifi_cfg_type(void);
static void axk_set_wifi_cfg_type(uint8_t type);

static aiio_os_thread_handle_t start_ap_task_handle = NULL;
static axk_wifi_cfg_type_t wifi_cfg_type = WIFI_CFG_NONE;

axk_at_network_t g_axk_network = {
    .get_wifi_cfg_type = axk_get_wifi_cfg_type,
    .set_wifi_cfg_type = axk_set_wifi_cfg_type,
};

static axk_wifi_cfg_type_t axk_get_wifi_cfg_type(void)
{
    return wifi_cfg_type;
}

static void axk_set_wifi_cfg_type(uint8_t type)
{
    wifi_cfg_type = type;
}

static void axk_wifi_start_ap_task(void *param)
{
    aiio_os_tick_dealy(1000);
    g_axk_wifi.set_mode(AXK_WIFI_AP);
    g_axk_wifi.init();
    g_axk_http_server.init();
    start_ap_task_handle = NULL;
    aiio_os_thread_delete(NULL);
}

static int32_t axk_wifi_start_ap(void)
{
    return aiio_os_thread_create(&start_ap_task_handle, "ap_start", axk_wifi_start_ap_task, 2048, NULL, 10);
}

int at_setup_webcfg_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;
    int ctrl;

    if (argc != 1)
    {
        aiio_log_i("argc error");
        goto __exit;
    }

    ctrl = atoi(argv[0]);

    if (ctrl != 0 && ctrl != 1)
    {
        aiio_log_i("argv error");
        goto __exit;
    }

    if (ctrl == 1)  //start cfg
    {
        if (g_axk_wifi.get_ap_start_flag() == 0)
        {
            //启动网页配网
            // g_axk_wifi.set_mode(AXK_WIFI_AP);
            // g_axk_wifi.init();
            axk_wifi_start_ap();
        }
        else
        {
            g_axk_http_server.init();
        }
    }
    else            //stop cfg
    {
        g_axk_http_server.deinit();
        if (g_axk_wifi.get_ap_start_flag() != 0)
        {
            if (g_axk_udp.get_udp_handle() == NULL)
            {
                //关闭热点
                aiio_wifi_ap_stop();
            }
        }
    }

    ret = 0;
    axk_at_printf("OK\r\n");

__exit:
    return ret;
}

int at_setup_appcfg_cmd(uint32_t argc, const char** argv)
{
    int ret = -1;
    int ctrl;

    if (argc != 1)
    {
        aiio_log_i("argc error");
        goto __exit;
    }

    ctrl = atoi(argv[0]);

    if (ctrl != 0 && ctrl != 1)
    {
        aiio_log_i("argv error");
        goto __exit;
    }

    if (ctrl == 1)  //start cfg
    {
        if (g_axk_wifi.get_ap_start_flag() == 0)
        {
            g_axk_wifi.set_mode(AXK_WIFI_AP);
            g_axk_wifi.init();
        }
        g_axk_udp.init();
    }
    else            //stop cfg
    {
        //deinit
        g_axk_udp.deinit();
        if (g_axk_wifi.get_ap_start_flag() != 0)
        {
            if (g_axk_http_server.get_start_flag() == 0)
            {
                aiio_wifi_ap_stop();
            }
        }
    }

    ret = 0;
    axk_at_printf("OK\r\n");

__exit:
    return ret;
}