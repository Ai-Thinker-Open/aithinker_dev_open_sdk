/** @brief      mdns application interface.
 *
 *  @file       aiio_mdns.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       mdns application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/04/08      <td>1.0.0       <td>hongjz      <td>Define the first version of the MDNS interface
 *  </table>
 *
 */
#include "aiio_mdns.h"
#include "aiio_log.h"
#include "aiio_error.h"
#include "aiio_os_port.h"

#include <mdns.h>
// #include "wifi_conf.h"
#include "lwip_netconf.h"
#include "wifi_api_event.h"

static bool mdns_init = false;
static aiio_mdns_txt_item_t s_txt[TXT_MAX_NUM] = {0};
static aiio_mdns_config_t mdns_config = {0};
static unsigned char txt_buf[100];

#ifndef NET_IF_NUM
#define NET_IF_NUM 2
#endif
extern struct netif xnetif[NET_IF_NUM];
static bool mdnsinit_flag = false;
static bool mdnsservice_flag = false;
static s8_t slot_num = 0;

static void srv_txt(struct mdns_service *service, void *txt_userdata)
{
	(void)txt_userdata;
	char content[50] = {"Here is ameba lwip_mdns service!"};
	mdns_resp_add_service_txtitem(service, content, strlen(content));
}

aiio_ret_t aiio_mdns_init(aiio_mdns_config_t config)
{
    if (mdns_init)
    {
        aiio_log_e("mdns already init ");
        return AIIO_ERROR;
    } 

    // if ((wifi_get_join_status() != RTW_JOINSTATUS_SUCCESS))
    //  {
    //     aiio_log_e("wifi is not ready to transceive!");
    //     return AIIO_ERROR;
    // }

    if ((NULL == config.host_name) || (NULL == config.service_type))
    {
        aiio_log_e("param err ");
        return AIIO_ERROR;
    }

    if(!mdnsinit_flag){
        mdns_resp_init();
        mdnsinit_flag = true;
    }

	if (mdns_resp_add_netif(&xnetif[0], "realtek", 30)) {
		aiio_log_e("mdns_resp_add_netif fail \r\n");
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

    char service_type[128] = {0};
    snprintf(service_type, sizeof(service_type) - 1, "%s._udp", mdns_config.service_type);

    if(!mdnsservice_flag){
        slot_num = mdns_resp_add_service(&xnetif[0], mdns_config.host_name, mdns_config.service_type, DNSSD_PROTO_UDP, 5000, 30, srv_txt, NULL);
        if (slot_num) {
            aiio_log_e("mdns_resp_add_service fail \r\n");
            return AIIO_ERROR;
        }
    }else{
        if(mdns_resp_rename_service(&xnetif[0], slot_num, mdns_config.host_name)){
             aiio_log_e("mdns_resp_rename_service fail \r\n");
            return AIIO_ERROR;
        }
    }

    mdns_init = true;

    return AIIO_OK;
}

aiio_ret_t aiio_mdns_deinit(void)
{
    if (!mdns_init)
    {
        aiio_log_e("mdns already DeInit ");
        return AIIO_ERROR;
    }

    // mdns_resp_del_service(&xnetif[0], slot_num);
    
    mdns_resp_remove_netif(&xnetif[0]);

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
    return AIIO_OK;
}
