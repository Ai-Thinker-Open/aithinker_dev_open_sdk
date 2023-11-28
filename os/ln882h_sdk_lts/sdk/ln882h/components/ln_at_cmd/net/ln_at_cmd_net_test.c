/**
 * @file   ln_at_cmd_ping.c
 * @author LightningSemi WLAN Team
 * Copyright (C) 2018-2020 LightningSemi Technology Co., Ltd. All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-02-01     MurphyZhao   the first version
 */

#include "utils/debug/log.h"
#include "ln_utils.h"
#include "ln_at.h"
#include "ping.h"
#include "iperf.h"
#include <stdlib.h>
#include <string.h>


/* declarations */
ln_at_err_t ln_at_set_ping(uint8_t para_num, const char *name);
ln_at_err_t ln_at_set_iperf(uint8_t para_num, const char *name);

/**
 * @brief ln_at_set_ping
 * Usage
 *   AT+PING=<destination>,[options]
 * Options:
 *   <destination>      dns name or ip address
 *   -t <ttl>           define time to live
 *   -s <size>          use <size> as number of data bytes to be sent
 *   -i <interval>      seconds between sending each packet
 * eg
 *   AT+PING="192.168.1.1"
 *   AT+PING="192.168.1.1", "-t", 1000, "-s", 512, "-i", 1000
 * @param name
 * @return ln_at_err_t
 */
ln_at_err_t ln_at_set_ping(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);

    bool is_default = false;
    uint8_t para_index = 1;
    char *ip_str = NULL;
    char *str_param = NULL;
    int int_param = 0;
    ping_param_t ping_param = {0};

    LOG(LOG_LVL_INFO, "ping argc:%d\r\n", para_num);

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip_str))
    {
        goto __exit;
    }

    if (is_default || !ip_str)
    {
        goto __exit;
    }
    ping_param.host = ip_str;

    if ((para_num > 1) && ((para_num % 2) != 0))
    {
        while (para_index <= para_num)
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &str_param))
            {
                goto __exit;
            }
            if (is_default || !str_param)
            {
                goto __exit;
            }

            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
            {
                goto __exit;
            }
            if (is_default)
            {
                goto __exit;
            }

            if (strcasecmp(str_param, "-t") == 0)
            {
                ping_param.cnt = int_param;
            }
            else if (strcasecmp(str_param, "-s") == 0)
            {
                LOG(LOG_LVL_INFO, "ping packet size:%d\r\n", int_param);
            }
            else if (strcasecmp(str_param, "-i") == 0)
            {
                ping_param.interval_ms = int_param;
            }
        }
    }

    if (!create_ping_task(&ping_param))
    {
        LOG(LOG_LVL_INFO, "ping task is running\r\n");
        ln_at_printf("\r\nbusy p...\r\n");
        return LN_AT_ERR_NONE;
    };

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
LN_AT_CMD_REG(LN_PING, NULL, ln_at_set_ping, NULL, NULL);


/**
 * @brief ln_at_set_iperf
 * Usage
 *   AT+IPERF=<iperf cmd string>
 * Options:
 *   <iperf cmd string>   iperf origin command
 * iperf command:
 *   Client/Server:
 *     -u                 use UDP rather than TCP
 *     -p                 server port to listen on/connect to (default 5001)
 *   Server specific:
 *     -s                 run in server mode
 *   Client specific:
 *     -c <ipaddr>        run in client mode, connecting to <ipaddr>
 *     -d <interval>      seconds between sending each packet
 *     -l <size>          use <size> as number of data bytes to be sent
 *   Miscellaneous:
 *     -h                 print help message
 *     --stop             stop iperf program
 * eg:
 *   Iperf TCP Server: AT+IPERF="iperf -s"
 *   Iperf UDP Server: AT+IPERF="iperf -u -s"
 *   Iperf TCP Client: AT+IPERF="iperf -c 192.168.1.100 -p 5001 -d 2 -l 1460"
 *   Iperf UDP Client: AT+IPERF="iperf -u -c 192.168.1.100 -p 5001 -d 2 -l 1460"
 * @param name
 * @return ln_at_err_t
 */
ln_at_err_t ln_at_set_iperf(uint8_t para_num, const char *name)
{
    LN_UNUSED(para_num);
    LN_UNUSED(name);

    bool is_default = false;
    uint8_t para_index = 1;
    char *str_param = NULL;

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &str_param))
    {
        goto __exit;
    }

    if (is_default || !str_param)
    {
        goto __exit;
    }

    if (0 != iperf(str_param))
    {
        goto __exit;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
LN_AT_CMD_REG(IPERF, NULL, ln_at_set_iperf, NULL, NULL);
