#include "ln_at_cmd_base_plus.h"

#include "usr_ctrl.h"
#include "utils/debug/log.h"
#include "ln_utils.h"
#include "ln_at.h"

typedef struct {
    char  ssid[SSID_MAX_LEN]; 
    char  pwd[PASSWORD_MAX_LEN];  
    uint8_t  bssid[BSSID_LEN];
} sta_conn_param_t; 

static sta_conn_param_t _conn;

static void ln_wifi_base_plus_init(void);

static ln_wifi_base_plus_cfg_t s_wifiBasePlusCfg = {	
	.init = ln_wifi_base_plus_init,
};

ln_wifi_base_plus_cfg_t* ln_get_wifi_base_plus_handle(void)
{
	return &s_wifiBasePlusCfg;
}

static void ln_wifi_base_plus_init(void)
{	
	
}

ln_at_err_t ln_at_cmd_base_get(const char *name)
{
    LN_UNUSED(name);
	
	ln_at_server_t *pSer = ln_at_server_get();
	for (int i = 0; i < pSer->tbl->flash_cmd_num; i++)
	{
		if (pSer->tbl->flash_cmd_tbl[i].name[0] == 'A' 
			&& pSer->tbl->flash_cmd_tbl[i].name[1] == 'T')
		{
			ln_at_printf("+CMD:%d,\"%s\"", 
					i, 
					pSer->tbl->flash_cmd_tbl[i].name);			
		}
		else
		{
			ln_at_printf("+CMD:%d,\"AT+%s\"", 
					i, 
					pSer->tbl->flash_cmd_tbl[i].name);
		}

		
		ln_at_printf(",%d,%d,%d,%d\r\n", 
					pSer->tbl->flash_cmd_tbl[i].test == NULL ? 0 : 1,
					pSer->tbl->flash_cmd_tbl[i].get == NULL ? 0 : 1,
					pSer->tbl->flash_cmd_tbl[i].set == NULL ? 0 : 1,
					pSer->tbl->flash_cmd_tbl[i].exec == NULL ? 0 : 1);
	}

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CMD, ln_at_cmd_base_get, NULL, NULL, NULL);
