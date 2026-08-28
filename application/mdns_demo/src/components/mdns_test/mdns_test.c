#include "aiio_adapter_include.h"

#define ERROR_COED_SUCCESS  (0)
#define ERROR_COED_MDNS_TASK_CREATE_ERROR  (-2)

static aiio_os_thread_handle_t *aiio_mdns_thread = NULL;

static void aiio_device_mdns_start(void *pvParameters)
{
    aiio_os_tick_dealy(aiio_os_ms2tick(100));

    aiio_mdns_config_t mdns_config =
        {
            .service_type = "_aithinker",
            .port = 602,
        };
    uint8_t mac_address[6] = {0};
    aiio_wifi_sta_mac_get(mac_address);
    char mac_str[17] = {0};

    aiio_sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5]);
    uint32_t ip, gw, mask;
    aiio_wifi_sta_ip_get(&ip, &gw, &mask);
    mdns_config.host_name = aiio_os_malloc(38);
    aiio_sprintf(mdns_config.host_name, "axk_%s_%d.%d.%d.%d_%d", mac_str, ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff, mdns_config.port);
    aiio_mdns_init(mdns_config);
    aiio_log_a("mdns start!\r\n\r\n\r\n");

    aiio_os_free(mdns_config.host_name);
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    aiio_os_thread_delete(NULL);
}

int32_t start_mdns_test()
{
    aiio_mdns_deinit();
    if(NULL!=aiio_mdns_thread)
    {
        aiio_os_thread_delete(*aiio_mdns_thread);
    }
    aiio_os_thread_create(&aiio_mdns_thread, "mdns_task", aiio_device_mdns_start, 4 * 1024, NULL, 8);
    if(NULL==aiio_mdns_thread)
    {
        aiio_log_a("aiio_mdns_thread create error!");
        return ERROR_COED_MDNS_TASK_CREATE_ERROR;
    }
    return ERROR_COED_SUCCESS;
}