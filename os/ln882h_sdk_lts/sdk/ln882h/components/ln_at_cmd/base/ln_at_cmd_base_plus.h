#ifndef  __LN_AT_BASE_PLUS_H__
#define  __LN_AT_BASE_PLUS_H__

#include "osal/osal.h"
#include "usr_ctrl.h"
#include "wifi.h"

typedef struct ln_base_wifi_plus_cfg
{
	void (*init)(void);
}ln_wifi_base_plus_cfg_t;


extern ln_wifi_base_plus_cfg_t* ln_get_wifi_base_plus_handle(void);




#endif
