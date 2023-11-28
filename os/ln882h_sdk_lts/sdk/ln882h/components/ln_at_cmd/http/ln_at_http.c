#include "ln_at_http.h"

#include "lwip/inet.h"
#include "ln_at.h"
#include "ln_utils.h"
#include "netif/ethernetif.h"

#include "usr_ctrl.h"

void http_set_content_type(int _idx);
void ln_http_init(void);

static ln_http_cfg_t s_httpCfg = {	
	.init = ln_http_init,
};

ln_http_cfg_t* ln_get_http_handle()
{
	return &s_httpCfg;
}

void ln_http_init(void)
{
	if (NULL == (ln_get_http_handle()->pHttp = httpinfo_new())) {
		Log_e("malloc for @http_info failed.");
		return;
	}

    if (0 != http_init(&ln_get_http_handle()->pHttp, FALSE)) {
        Log_e("http init failed.!!!");
    }
}

static int http_get_callback(const char *recvbuf, int32_t recvbuf_len, int32_t total_bytes, int8_t is_recv_finished)
{
	LOG_I("cb info: %d bytes arrives", recvbuf_len);
    if (!is_recv_finished) {
		LOG_I("cb info: recv %d , wait for more data to deal with.", total_bytes);
    } else {
        LOG_I("cb info: recv %d finished, no more data to deal with.", total_bytes);
		ln_at_printf("+HTTPCLIENT:%d,%s\r\n", recvbuf_len, recvbuf);
		
		ln_at_printf(LN_AT_RET_OK_STR);
	}

    return recvbuf_len;
}

ln_at_err_t ln_http_at_client_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int opt = NULL;
	int contentType = NULL;
	int transportType = NULL;
	char *pUrl = NULL;
	char *pHost = NULL;
	char *pPath = NULL;
	int recv_total = 0;

	ln_http_cfg_t* p = ln_get_http_handle();
	
	//check connect stat
    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active())){
        goto __err;
    }
	
	//opt
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &opt))
    {
        goto __err;
    }
	
	if (opt < 1 || opt > 5)
	{
		goto __err;
	}

	//content-type
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &contentType))
    {
        goto __err;
    }
	
	if (contentType < 0 || contentType > 3)
	{
		goto __err;
	}
	
	//url
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pUrl))
    {
        goto __err;
    }
	
	if (is_default || !pUrl)
    {
        goto __err;
    }

	//host
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pHost))
    {
        goto __err;
    }
	
	if (is_default || !pHost)
    {
        goto __err;
    }

	//path
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pPath))
    {
        goto __err;
    }
	
	if (is_default || !pPath)
    {
        goto __err;
    }
	
	//transport-type
	if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &transportType))
    {
        goto __err;
    }
	
	if (transportType < 1 || transportType > 2)
	{
		goto __err;
	}
	
	http_set_content_type(contentType);
	recv_total = http_get_with_callback(p->pHttp, pUrl, http_get_callback);
	
    //ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

//LN_AT_CMD_REG(HTTPCLIENT, NULL, ln_http_at_client_set, NULL, NULL);

