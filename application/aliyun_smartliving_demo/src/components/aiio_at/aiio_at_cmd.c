/**
 * @brief   
 * 
 * @file    aiio_at_cmd.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-07          <td>1.0.0            <td>zhuolm             <td>
 */
#include "aiio_adapter_include.h"
#include "aiio_at_cmd.h"
#include "aiio_at_receive.h"
#include "aiio_common.h"




void AT_Test(char *cmd, uint16_t cmd_len)
{
    if(cmd == NULL)
    {
        aiio_log_e("parameter err \r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
    else
    {
        AT_RESPONSE("OK\r\n");
        aiio_log_i("response ok \r\n");
    }
}


void At_Key(char *cmd, uint16_t cmd_len)
{
    char *data = NULL;
    char *temp = NULL;
    uint16_t len = 0;

    if(cmd == NULL)
    {
        aiio_log_e("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    memset(&DeviceInfo_info, 0, sizeof(iotx_device_info_t));
    aiio_flash_clear_device_info(&DeviceInfo_info);
    if(cmd_len > 0)
    {
        if (cmd[0] == '=')
        {
            data = &cmd[1];
            if(data == NULL)
            {
                aiio_log_e("cmd data is NULL \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }

            temp= strstr(data, ",");
            if(temp == NULL)
            {
                aiio_log_e("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            aiio_log_d("len = %d \r\n", len);

            memcpy(DeviceInfo_info.product_key, data, len);
            aiio_log_d("DeviceInfo_info.product_key = %s \r\n", DeviceInfo_info.product_key);

            temp++;
            data = temp;
            temp = strstr(data, ",");
            if(temp == NULL)
            {
                aiio_log_e("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            aiio_log_d("len = %d \r\n", len);
            memcpy(DeviceInfo_info.product_Secret, data, len);
            aiio_log_d("DeviceInfo_info.product_Secret = %s", DeviceInfo_info.product_Secret);

            temp++;
            data = temp;
            temp = strstr(data, ",");
            if(temp == NULL)
            {
                aiio_log_e("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            aiio_log_d("len = %d \r\n", len);
            memcpy(DeviceInfo_info.device_name, data, len);
            aiio_log_d("DeviceInfo_info.device_name = %s", DeviceInfo_info.device_name);

            temp++;
            data = temp;
            temp = strstr(data, ",");
            if(temp == NULL)
            {
                aiio_log_e("not find ',' symbol \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = temp - data;
            aiio_log_d("len = %d \r\n", len);
            memcpy(DeviceInfo_info.device_secret, data, len);
            aiio_log_d("DeviceInfo_info.device_secret = %s", DeviceInfo_info.device_secret);

            temp++;
            if(temp == NULL)
            {
                aiio_log_e("password data err \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = strlen(temp);
            aiio_log_d("len = %d \r\n", len);
            char product_Id[32+1];
            memcpy(product_Id, temp, len);
            DeviceInfo_info.product_Id = atoi(product_Id);
            aiio_log_d("DeviceInfo.product_Id = %u", DeviceInfo_info.product_Id);

            aiio_flash_save_device_info(&DeviceInfo_info);
            aiio_restart();
        }
        else
        {
            aiio_log_e("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        aiio_log_e("cmd length err \r\n");
        AT_RESPONSE("ERR\r\n");
    }
}

