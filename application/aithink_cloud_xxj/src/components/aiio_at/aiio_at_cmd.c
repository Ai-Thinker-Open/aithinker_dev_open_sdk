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

    memset(&DeviceInfo, 0, sizeof(aiio_device_info));
    aiio_flash_clear_device_info(&DeviceInfo);
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

            memcpy(DeviceInfo.deviceId, data, len);
            aiio_log_d("DeviceInfo.deviceId = %s \r\n", DeviceInfo.deviceId);

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
            memcpy(DeviceInfo.userName, data, len);
            aiio_log_d("DeviceInfo.userName = %s", DeviceInfo.userName);

            temp++;
            if(temp == NULL)
            {
                aiio_log_e("password data err \r\n");
                AT_RESPONSE("ERR\r\n");
                return ;
            }
            len = strlen(temp);
            aiio_log_d("len = %d \r\n", len);
            memcpy(DeviceInfo.userPasswd, temp, len);
            aiio_log_d("DeviceInfo.userPasswd = %s", DeviceInfo.userPasswd);

            aiio_flash_save_device_info(&DeviceInfo);
#if CONFIG_BLE_DISTRIBUTION_NETWORK
            aiio_flash_clear_config_data();
#endif
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

