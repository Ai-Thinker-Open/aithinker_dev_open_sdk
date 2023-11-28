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

void AT_LEDTEST(char *cmd, uint16_t cmd_len)
{

    if(cmd == NULL)
    {
        aiio_log_e("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        if (cmd[0] == '=')
        {
            if(cmd[1] == '1')
            {
                aiio_log_d("waterfall_light turn on \r\n");
                waterfall_light = true;
                AT_RESPONSE("OK\r\n");
            }
            else if(cmd[1] == '0')
            {
                aiio_log_d("waterfall_light turn off \r\n");
                waterfall_light = false;
                waterfall_light_state = false;
                AT_RESPONSE("OK\r\n");
            }
            else
            {
                aiio_log_i("not find parameter \r\n");
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            aiio_log_i("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
}

void AT_NodeMCUTEST(char *cmd, uint16_t cmd_len)
{

    if(cmd == NULL)
    {
        aiio_log_e("params err \r\n");
        return ;
    }

    if(cmd_len == 2)
    {
        if (cmd[0] == '=')
        {
            if(cmd[1] == '1')
            {
                at_node_mcutest = 1;
                AT_RESPONSE("OK\r\n");
            }
            else if(cmd[1] == '0')
            {
                at_node_mcutest = 0;
                AT_RESPONSE("OK\r\n");
            }
            else
            {
                aiio_log_i("not find parameter \r\n");
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            aiio_log_i("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
}

void AT_RESTORE(char *cmd, uint16_t cmd_len)
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
        aiio_repair();
        aiio_log_d("Enter Pair \r\n");
        aiio_os_tick_dealy(aiio_os_ms2tick(20));
        aiio_restart();
    }
}

void At_GDevLTime(char *cmd, uint16_t cmd_len)
{
    char TimeBuff[200] = {0};

    if(cmd == NULL)
    {
        aiio_log_e("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
    else
    {
        AilinkLoadTimeInfo(&UpTime);
        snprintf(TimeBuff, sizeof(TimeBuff), "the Date:%02d:%02d:%02d\r\nthe time:%02d:%02d:%02d\r\n", 
            UpTime.year, UpTime.mon, UpTime.day, UpTime.hour,UpTime.minute, UpTime.second);
        AT_RESPONSE(TimeBuff);
        aiio_log_d("response ok \r\n");
    }
}

void AT_RST(char *cmd, uint16_t cmd_len)
{
    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
    else
    {
        AT_RESPONSE("OK\r\n");
        aiio_os_tick_dealy(aiio_os_ms2tick(20));
        aiio_restart();
    }
    
}

void AT_UARTCFG(char *cmd, uint16_t cmd_len)
{
    uint32_t bound = 0;
    char buff[10] = {0};

    if(cmd == NULL)
    {
        aiio_log_e("params err \r\n");
        return ;
    }
    IotClearDeviceParams();
    if(cmd_len > 2)
    {
        if (cmd[0] == '=')
        {
            memcpy(buff, &cmd[1], strlen(&cmd[1]));
            aiio_log_d("buff = %s \r\n", buff);
            bound = atoi(buff);
            aiio_log_d("bound = %d \r\n", bound);

            DeviceParams.uart_bound = bound;
            DeviceParams.work_mode = 1;
            IotSaveDeviceParams(&DeviceParams);

            AT_RESPONSE("OK\r\n");
        }
        else
        {
            aiio_log_e("not find parameter \r\n");
            AT_RESPONSE("ERR\r\n");
        }
    }
    else
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
}


void AT_CIPSTAMAC(char *cmd, uint16_t cmd_len)
{
    uint8_t mac[6];
    char macstr[30]; 
    char buff[50] = {0};

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
    else
    {
        aiio_wifi_sta_mac_get(mac);
        memset(macstr, 0, sizeof(macstr));
        snprintf(macstr, sizeof(macstr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2], mac[3], mac[4], mac[5]);
        macstr[sizeof(macstr) - 1] = '\0';
        aiio_log_d("macstr = %s \r\n", macstr);
        snprintf(buff, sizeof(buff), "+CIPSTAMAC:\"%s\"\r\n", macstr);
        AT_RESPONSE(buff);
        AT_RESPONSE("OK\r\n");
    }
}

void AT_GMR(char *cmd, uint16_t cmd_len)
{
    char buff[100] = {0};

    if(cmd == NULL)
    {
        aiio_log_e("parameter err \r\n");
        AT_RESPONSE("ERR\r\n");
        return;
    }

    if (cmd_len != 0)
    {
        AT_RESPONSE("ERR\r\n");
        aiio_log_e("response err \r\n");
    }
    else
    {
        char build_time[100] = {0};
        snprintf(build_time, sizeof(build_time), "%s , %s \r\n", __TIME__, __DATE__);
        printf("build time: %s \r\n", build_time);
        snprintf(buff, sizeof(buff), "build time: %s\r\n", build_time);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "fw version: %s\r\n", DEVICE_FW_VERSION);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "userName: %s\r\n", DeviceInfo.userName);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "userPasswd: %s\r\n", DeviceInfo.userPasswd);
        AT_RESPONSE(buff);
        memset(buff, 0, sizeof(buff));
        snprintf(buff, sizeof(buff), "deviceId: %s\r\n", DeviceInfo.deviceId);
        AT_RESPONSE(buff);
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
            // temp = strrstr(data, "\r\n");
            // if(temp == NULL)
            // {
            //     IOT_ErrorLog("not find enter \r\n");
            //     AT_RESPONSE("ERR\r\n");
            //     return ;
            // }
            // temp[0] = '\0';
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
            aiio_log_i("DeviceInfo.deviceId = %s \r\n", DeviceInfo.deviceId);

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
            aiio_log_i("DeviceInfo.userName = %s", DeviceInfo.userName);

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
            aiio_log_i("DeviceInfo.userPasswd = %s", DeviceInfo.userPasswd);

            IotSaveDeviceInfo();
            AT_RESPONSE("OK\r\n");
            aiio_os_tick_dealy(aiio_os_ms2tick(20));
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

