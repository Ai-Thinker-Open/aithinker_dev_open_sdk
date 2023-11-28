#include "ln_at_sntp.h"

#include "usr_ctrl.h"
#include "utils/debug/log.h"
#include "ln_utils.h"
#include "ln_at.h"
#include "ping.h"
#include "iperf.h"
#include <stdlib.h>
#include <string.h>

#include "hal/hal_misc.h"

#include "lwip/apps/sntp.h"
#include "sys/time.h" /* search from local sys dirctory */

static OS_Timer_t s_SntpTmr;

static void ln_sntp_init(void);

static ln_sntp_cfg_t s_sntpCfg = {
	.sntpTimeZone = 0,
	.tim = 0,
	.serverCnt = 3,
	
	.init = ln_sntp_init,
};

ln_sntp_cfg_t* ln_get_sntp_handle()
{
	return &s_sntpCfg;
}

static void ln_sntp_tmr_cb(void *arg)
{
	if (ln_get_sntp_handle()->tim > 0)
	{
		ln_get_sntp_handle()->tim++;
	}
}

void ln_sntp_init(void)
{
	if (OS_OK != OS_TimerCreate(&s_SntpTmr, OS_TIMER_PERIODIC, ln_sntp_tmr_cb, NULL, 1000))
	{
		Log_e("OS_TimerCreate fail!");
		while(1);
	}	
	
	strcpy(ln_get_sntp_handle()->sntpServer[0], SNTP_SERVER_ADDRESS1);
	strcpy(ln_get_sntp_handle()->sntpServer[1], SNTP_SERVER_ADDRESS2);
	strcpy(ln_get_sntp_handle()->sntpServer[2], SNTP_SERVER_ADDRESS3);
}

static void start_sntp_plus_timer(void)
{
	if (OS_OK != OS_TimerStart(&s_SntpTmr))
	{
		Log_e("OS_TimerStart fail!");
		while(1);
	}	
}

static void stop_sntp_plus_timer()
{
	OS_TimerStop(&s_SntpTmr);
}

/**
 * @brief ln_at_set_sntp_cfg
 * Usage
 *   AT+CIPSNTPCFG=<EN>,<TIME_ZONE>
 * Options:
 *   <EN>		      		Enable or Disable
 *   <TIME_ZONE>       	    time zone [-11, 13]
 * eg
 *   AT+CIPSNTPCFG=1,8
 *   AT+CIPSNTPCFG=0
 * @param name
 * @return ln_at_err_t
 */
ln_at_err_t ln_at_set_sntp_cfg(uint8_t para_num, const char *name)
{
    LN_UNUSED(para_num);
    LN_UNUSED(name);

    bool is_default = false;
    uint8_t para_index = 1;
	int sntpEn = 0;
	char *str[3] = {0};

	//en
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &sntpEn))
    {
        goto __exit;
    }

    if (is_default)
    {
        goto __exit;
    }
	
    if (sntpEn == 0)
    {
        sntp_stop();
        ln_get_sntp_handle()->sntpTimeZone = 0;
        ln_get_sntp_handle()->tim = 0;
        stop_sntp_plus_timer();
        goto __suc;
    }
	
	if (sntpEn != 1)
	{
		goto __exit;
	}
	
	//timeZone
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &ln_get_sntp_handle()->sntpTimeZone))
    {
        goto __exit;
    }

    if (is_default)
    {
        goto __exit;
    }
	
    if (ln_get_sntp_handle()->sntpTimeZone > 13 || ln_get_sntp_handle()->sntpTimeZone < -11)
    {
		ln_get_sntp_handle()->sntpTimeZone = 0;
        goto __exit;
    }
	
	//server
	if (para_num - 2 == 0)
	{
		ln_get_sntp_handle()->serverCnt = 3;
		str[0] = SNTP_SERVER_ADDRESS1;
		str[1] = SNTP_SERVER_ADDRESS2;
		str[2] = SNTP_SERVER_ADDRESS3;
	}
	else if (para_num - 2 >= 1 && para_num - 2 <= 3)
	{
		ln_get_sntp_handle()->serverCnt = para_num - 2;
		for (int i = 0; i < ln_get_sntp_handle()->serverCnt; i++)
		{
			if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &str[i]))
			{	
				goto __exit;
			}
		}
	}
	else{
		goto __exit;
	}

	for (int i = 0; i < 3; i++)
	{
		if (i < ln_get_sntp_handle()->serverCnt)
		{
			//server
			strcpy(ln_get_sntp_handle()->sntpServer[i], str[i]);
			sntp_setservername(i, ln_get_sntp_handle()->sntpServer[i]);
		}
		else{
			sntp_setservername(i, NULL);
		}
	}
	
    tz_set(ln_get_sntp_handle()->sntpTimeZone);
    
    if (sntp_enabled())
	{
		LOG(LOG_LVL_INFO, "sntp is already running!\r\n");
		goto __suc;
	}

    sntp_init();
    start_sntp_plus_timer();

__suc:
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}

ln_at_err_t ln_at_get_sntp_cfg(const char *name)
{
	if (!sntp_enabled())
	{
		//+CIPSNTPCFG:0
		ln_at_printf("+CIPSNTPCFG:0\r\n");
	}
	else
	{
		ln_at_printf("+CIPSNTPCFG:1,%d", ln_get_sntp_handle()->sntpTimeZone);
		for(int i = 0; i < ln_get_sntp_handle()->serverCnt; i++)
		{
			ln_at_printf(",\"%s\"", sntp_getservername(i));
		}
	}

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPSNTPCFG, ln_at_get_sntp_cfg, ln_at_set_sntp_cfg, NULL, NULL);

/**
 * @brief ln_at_get_sntp
 * Usage
 *   AT+CIPSNTPTIME?
 * eg
 *   AT+CIPSNTPTIME?
 *   +CIPSNTPTIME:Thu May 26 13:58:24 2022
 *
 *	 OK
 * @param name
 * @return ln_at_err_t
 */
ln_at_err_t ln_at_get_sntp(const char *name)
{
    LN_UNUSED(name);
	
	unsigned int currentTim = 0;
	unsigned int timePlus = ln_get_sntp_handle()->sntpTimeZone * 3600;
    char time_buf[32] = {0};
    
    if (timePlus > 0)
    {
        currentTim = ln_get_sntp_handle()->tim + timePlus;
	}
	else
	{
		if (ln_get_sntp_handle()->tim >= timePlus)
		{
			currentTim = ln_get_sntp_handle()->tim - timePlus;
		}
		else
		{
			currentTim = 0;
		}
	}

    ln_at_printf("+CIPSNTPTIME:%s", ctime_r(&ln_get_sntp_handle()->tim, time_buf));

    ln_at_printf("OK\r\n");
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPSNTPTIME, ln_at_get_sntp, NULL, NULL, NULL);
