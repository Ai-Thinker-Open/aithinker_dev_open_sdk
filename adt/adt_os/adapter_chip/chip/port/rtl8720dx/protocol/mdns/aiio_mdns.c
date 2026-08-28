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

#include <mDNS/mDNS.h>
#include "wifi_constants.h"

static bool mdns_init = false;
static aiio_mdns_txt_item_t s_txt[TXT_MAX_NUM] = {0};
static aiio_mdns_config_t mdns_config = {0};
static DNSServiceRef dnsServiceRef = NULL;
static unsigned char txt_buf[100];
static TXTRecordRef txtRecord;

aiio_ret_t aiio_mdns_init(aiio_mdns_config_t config)
{
    if (mdns_init)
    {
        aiio_log_e("mdns already init ");
        return AIIO_ERROR;
    }

    if (wifi_is_ready_to_transceive(RTW_STA_INTERFACE) != RTW_SUCCESS)
    {
        aiio_log_e("wifi is not ready to transceive!");
        return AIIO_ERROR;
    }

    if ((NULL == config.host_name) || (NULL == config.service_type))
    {
        aiio_log_e("param err ");
        return AIIO_ERROR;
    }

    if(mDNSResponderInit() != 0)
    {
        aiio_log_e("mDNS Responder Init failed!");
        return AIIO_ERROR;
    }

    TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);
    for (size_t i = 0; i < TXT_MAX_NUM; i++)
    {
        if (strlen(s_txt[i].key) && strlen(s_txt[i].value))
        {
            TXTRecordSetValue(&txtRecord, (const char *)s_txt[i].key, strlen(s_txt[i].value), s_txt[i].value);
        }
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

    dnsServiceRef = mDNSRegisterService(mdns_config.host_name, service_type, "local", mdns_config.port, &txtRecord);
    TXTRecordDeallocate(&txtRecord);

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

    mDNSDeregisterService(dnsServiceRef);
    dnsServiceRef = NULL;
    mDNSResponderDeinit();

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
    if ((NULL == service_type) || (NULL == txt))
    {
        aiio_log_e("arg error ");
        return AIIO_ERROR;
    }

    if (!mdns_init)
    {
        aiio_log_e("mdns is not initialization!");
        return AIIO_ERROR;
    }

    for (size_t i = 0; i < TXT_MAX_NUM; i++)
    {
        memset(s_txt[i].key, 0, TXT_KEY_MAX_LEN);
        memset(s_txt[i].value, 0, TXT_VALUE_MAX_LEN);
    }

    TXTRecordCreate(&txtRecord, sizeof(txt_buf), txt_buf);
    for (uint16_t i = 0; i < num_items; i++)
    {
        if (txt[i].key)
        {
            strncpy(s_txt[i].key, txt[i].key, sizeof(s_txt[i].key) - 1);
            strncpy(s_txt[i].value, txt[i].value, sizeof(s_txt[i].value) - 1);
            TXTRecordSetValue(&txtRecord, s_txt[i].key, strlen(s_txt[i].value), s_txt[i].value);
        }
    }
    mDNSUpdateService(dnsServiceRef, &txtRecord, 0);
    TXTRecordDeallocate(&txtRecord);

    return AIIO_OK;
}
