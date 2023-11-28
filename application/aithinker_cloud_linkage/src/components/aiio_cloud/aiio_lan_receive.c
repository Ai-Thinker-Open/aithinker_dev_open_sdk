/**
 * @brief   Defining custom LAN communication interface
 * 
 * @file    aiio_lan_receive.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-06          <td>1.0.0            <td>zhuolm             <td> The custom LAN communication interface
 */
#include "aiio_adapter_include.h"
#include "aiio_lan_receive.h"
#include "aiio_common.h"


static bool user_lan_service_init = false;
static void aiio_receive_lan_service_data(aiio_lan_service_data_t *lan_service_data);


#if CONFIG_LAN_NETWORK_MODULE

void aiio_user_lan_service_init(aiio_user_lan_config_t *lan_config)
{
    aiio_lan_service_config_t lan_service_config = {0};

    if(lan_config == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    if(user_lan_service_init)
    {
        aiio_log_e("lan service already init \r\n");
        return ;
    }

    lan_service_config.device_id = lan_config->device_id;
    lan_service_config.local_ip = lan_config->local_ip;
    lan_service_config.lan_service_data_cb = aiio_receive_lan_service_data;
    aiio_lan_service_init(&lan_service_config);
}


void aiio_user_lan_service_deinit(void)
{
    if(!user_lan_service_init)
    {
        aiio_log_e("lan service already deinit \r\n");
        return ;
    }

    aiio_lan_service_deinit();
}


static void aiio_receive_lan_service_data(aiio_lan_service_data_t *lan_service_data)
{
    aiio_log_d("########### receive LAN data ################\r\n");
    aiio_parse_control_data(lan_service_data->msgMid, lan_service_data->from, lan_service_data->data);
}

#endif
