/** @brief      mdns application interface.
 *
 *  @file       aiio_mdns.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       mdns application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/16      <td>1.0.0       <td>yanch       <td>Define the first version of the MDNS interface
 *  </table>
 *
 */
#include "aiio_mdns.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_os_port.h"

#include <lwip/sockets.h>
//#include <utils_sha256.h>
#include <lwip/apps/mdns_priv.h>
#include <lwip/apps/mdns.h>
#include <lwip/netif.h>
#include <mdns_server.h>
// #include <lwip/apps/mdns.h>
#include <wifi_mgmr_ext.h>
#include <lwip/netifapi.h>

//#include <hal_sys.h>

static bool mdns_init = false;
static struct netif *netif = NULL;
static uint32_t pre_ip = 0, cur_ip = 0;

static uint8_t g_mdns_num = 0;
static char g_data[TXT_KEY_MAX_LEN + TXT_VALUE_MAX_LEN] = {0};
static aiio_mdns_txt_item_t g_txt[TXT_MAX_NUM] = {0};

static aiio_mdns_config_t mdns_config = {0};

/**
 * @brief Add callback function for reply data
 *
 * @param[in] service       mdns service
 * @param[in] txt_userdata
 *
 */
static void aiio_mdns_add_srvtxt(struct mdns_service *service, void *txt_userdata)
{
    uint8_t i = 0;
    int32_t ret = 0;

    for (i = 0; i < g_mdns_num; i++)
    {
        if ((strlen(g_txt[i].key) > 0) && (strlen(g_txt[i].value) > 0))
        {
            memset(g_data, 0, TXT_KEY_MAX_LEN + TXT_VALUE_MAX_LEN);
            snprintf(g_data, sizeof(g_data), "%s=%s", g_txt[i].key, g_txt[i].value);
            aiio_log_i("g_data = %s", g_data);
            ret = mdns_resp_add_service_txtitem(service, g_data, strlen(g_data));
            if (ret < 0)
            {
                aiio_log_e("mdns update txt error ");
            }
        }
    }
}

/**
 * @brief Add mdns service
 *
 * @param[in] my_netif    network handle
 *
 * @return aiio_ret_t
 *
 * @retval AIIO_OK     Added successfully
 * @retval AIIO_ERROR  Add failed
 */
static aiio_ret_t aiio_mdns_service_start(struct netif *my_netif)
{
    int32_t ret = AIIO_ERROR;

    if ((NULL == mdns_config.host_name) || (NULL == mdns_config.service_type))
    {
        aiio_log_e("param err ");
        return AIIO_ERROR;
    }

    if (my_netif == NULL)
    {
        aiio_log_e("netif is NULL ");
        return AIIO_ERROR;
    }
    mdns_resp_init();
    ret = mdns_resp_add_netif(my_netif, mdns_config.host_name, 3600);
    if (ret != 0)
    {
        mdns_resp_deinit();
        aiio_log_e("add my_netif failed:%d", ret);
        return AIIO_ERROR;
    }

    ret = mdns_resp_add_service(my_netif, mdns_config.host_name, mdns_config.service_type,
                                DNSSD_PROTO_UDP, mdns_config.port, 3600, aiio_mdns_add_srvtxt, NULL);
    aiio_log_i("mdns_responder_start ret = %d ", ret);

    if (ret < 0)
    {
        mdns_resp_remove_netif(my_netif);
        mdns_resp_deinit();
        aiio_log_e("add server failed:%d", ret);
        return AIIO_ERROR;
    }

    return AIIO_OK;
}

aiio_ret_t aiio_mdns_init(aiio_mdns_config_t config)
{

    int32_t ret = AIIO_ERROR;

    if (mdns_init)
    {
        aiio_log_e("mdns already init ");
        return AIIO_ERROR;
    }

    if ((NULL == config.host_name) || (NULL == config.service_type))
    {
        aiio_log_e("param err ");
        return AIIO_ERROR;
    }
    memset(&mdns_config, 0, sizeof(aiio_mdns_config_t));

    mdns_config.host_name = malloc(strlen(config.host_name) + 1);
    if (mdns_config.host_name == NULL)
    {
        aiio_log_e("malloc failed");
        return AIIO_ERROR;
    }
    mdns_config.service_type = malloc(strlen(config.service_type) + 1);
    if (mdns_config.service_type == NULL)
    {
        aiio_log_e("malloc failed");
        free(mdns_config.host_name);
        mdns_config.host_name = NULL;
        return AIIO_ERROR;
    }

    memset(mdns_config.host_name, 0, strlen(config.host_name) + 1);
    memcpy(mdns_config.host_name, config.host_name, strlen(config.host_name));

    memset(mdns_config.service_type, 0, strlen(config.service_type) + 1);
    memcpy(mdns_config.service_type, config.service_type, strlen(config.service_type));

    mdns_config.port = config.port;

    aiio_log_d("host name : %s", mdns_config.host_name);
    aiio_log_d("port : %d", mdns_config.port);
    aiio_log_d("server type : %s", mdns_config.service_type);

    netif = NULL;
    //netif = wifi_mgmr_sta_netif_get();
    netif=net_if_find_from_name("wl1");
    if (netif == NULL)
    {
        aiio_log_e("find failed");
        free(mdns_config.service_type);
        mdns_config.service_type = NULL;
        free(mdns_config.host_name);
        mdns_config.host_name = NULL;
        return AIIO_ERROR;
    }
    cur_ip = netif->ip_addr.addr;
    pre_ip = cur_ip;

    aiio_log_d("pre_ip = 0x%08x ", pre_ip);
    aiio_log_d("cur_ip = 0x%08x ", cur_ip);

    ret = netifapi_netif_common(netif, NULL, aiio_mdns_service_start);
    if (ret < 0)
    {
        aiio_log_e("start mdns failed");
        mdns_resp_remove_netif(netif);
        mdns_resp_deinit();
        netif = NULL;
        return AIIO_ERROR;
    }

    mdns_init = true;

    return ret;
}

aiio_ret_t aiio_mdns_deinit(void)
{
    if (!mdns_init)
    {
        aiio_log_e("mdns already DeInit ");
        return AIIO_ERROR;
    }

    mdns_resp_remove_netif(netif);
    mdns_resp_deinit();
    netif = NULL;

    if (mdns_config.service_type)
    {
        free(mdns_config.service_type);
        mdns_config.service_type = NULL;
    }

    if (mdns_config.host_name)
    {
        free(mdns_config.host_name);
        mdns_config.host_name = NULL;
    }
    memset(&mdns_config, 0, sizeof(aiio_mdns_config_t));

    mdns_init = false;

    return AIIO_OK;
}

aiio_ret_t aiio_mdns_update_txt(char *service_type, uint16_t protocol, aiio_mdns_txt_item_t txt[], uint8_t num_items)
{
    uint16_t i = 0;

    if ((NULL == service_type) || (NULL == txt))
    {
        aiio_log_e("arg error ");
        return AIIO_ERROR;
    }
    if (NULL == netif)
    {
        aiio_log_e("netif is NULL ");
        return AIIO_ERROR;
    }
    cur_ip = netif->ip_addr.addr;
    if (pre_ip != cur_ip)
    {
        aiio_log_i("pre_ip = 0x%08x ", pre_ip);
        aiio_log_i("cur_ip = 0x%08x ", cur_ip);
        netifapi_netif_common(netif, mdns_resp_announce, NULL);
        pre_ip = cur_ip;
    }

    for (i = 0; i < TXT_MAX_NUM; i++)
    {
        memset(g_txt[i].key, 0, TXT_KEY_MAX_LEN);
        memset(g_txt[i].value, 0, TXT_VALUE_MAX_LEN);
    }
    g_mdns_num = num_items;
    for (i = 0; i < num_items; i++)
    {
        if (txt[i].key)
        {
            strncpy(g_txt[i].key, txt[i].key, sizeof(g_txt[i].key));
            strncpy(g_txt[i].value, txt[i].value, sizeof(g_txt[i].value));
        }
    }
    netifapi_netif_common(netif, mdns_resp_announce, NULL);
    netifapi_netif_common(netif, mdns_resp_announce, NULL);
    netifapi_netif_common(netif, mdns_resp_announce, NULL);

    return AIIO_OK;
}
