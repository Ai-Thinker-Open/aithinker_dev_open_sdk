#ifndef LN_AT_HTTP_H
#define LN_AT_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include "httpsclient.h"
#include "httpsclient_wrapper.h"
#include "httpsclient_debug.h"

typedef struct ln_http_cfg
{
	HTTP_INFO *pHttp;
	
	void (*init)(void);
}ln_http_cfg_t;
	
	
	
	
extern ln_http_cfg_t* ln_get_http_handle(void);
	
	
#ifdef __cplusplus
}
#endif

#endif /* LN_AT_HTTP_H */
