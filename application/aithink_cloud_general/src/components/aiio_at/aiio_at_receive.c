/**
 * @brief   Define interface of receiving at command
 * 
 * @file    aiio_at_receive.c
 * @copyright Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 * @note 
 * @par Change Logs:
 * <table>
 * <tr><th>Date               <th>Version             <th>Author           <th>Notes
 * <tr><td>2023-07-07          <td>1.0.0            <td>zhuolm             <td>
 */

#include "aiio_at_cmd.h"
#include "aiio_at_receive.h"
#include "aiio_log.h"
#include "aiio_uart.h"


#define AT_CMD_HEADER "AT"

static ATCmdInfo ATCmdInfoTab[] = 
{
    {"AT+KEY", At_Key},
    {"AT+RESTORE", AT_RESTORE},
    {"AT+GDLT", At_GDevLTime},
    {"AT+RST", AT_RST},
    {"AT+NodeMCUTEST", AT_NodeMCUTEST},
    {"AT+LEDTEST", AT_LEDTEST},
    {"AT+GMR", AT_GMR},
    {"AT+UARTCFG", AT_UARTCFG},
    {"AT+CIPSTAMAC?", AT_CIPSTAMAC},
    {"AT", AT_Test},
    {NULL, NULL}
};

void atCmdExecute(char *buff)
{
    char *cmdPoint = NULL;
    uint16_t cmd_data_len = 0;
    uint16_t cmd_len = 0;
    uint16_t len = 0;
    bool cmdStatus = false;


    if(memcmp(buff, AT_CMD_HEADER, strlen(AT_CMD_HEADER)) == 0)
    {
        cmdPoint = strstr(buff, AT_CMD_HEADER);

        if(cmdPoint)
        {
            cmd_data_len = strlen(cmdPoint);
            for(uint16_t num = 0; ATCmdInfoTab[num].cmd != NULL; num++)
            {
                cmd_len = strlen(ATCmdInfoTab[num].cmd);
                if(memcmp(cmdPoint, ATCmdInfoTab[num].cmd, cmd_len) == 0)
                {
                    len = (cmd_data_len - cmd_len);
                    ATCmdInfoTab[num].func((cmdPoint + cmd_len), len);
                    cmdStatus = true;
                    return;
                }
            }
            if(!cmdStatus)
            {
                aiio_log_e("Can not find the cmd[%s] \r\n", cmdPoint);
                AT_RESPONSE("ERR\r\n");
            }
        }
        else
        {
            AT_RESPONSE("ERR\r\n");
            aiio_log_e("cmd parse err \r\n");
        }
    }
    else
    {
        aiio_log_e("Can not find the cmd[%s] \r\n", buff);
        AT_RESPONSE("ERR\r\n");
    }
}



void aiio_at_cmd_response(char *cmd)
{
    if(cmd == NULL)
    {
        aiio_log_e("param err \r\n");
        return ;
    }

    aiio_uart_send_data(AIIO_UART1, (uint8_t *)cmd, strlen(cmd));
}



