#ifndef  __LN_AT_SNTP_H__
#define  __LN_AT_SNTP_H__

#include <time.h>

#define SNTP_SERVER_ADDRESS1 "cn.ntp.org.cn"
#define SNTP_SERVER_ADDRESS2 "ntp.sjtu.edu.cn"
#define SNTP_SERVER_ADDRESS3 "us.pool.ntp.org"

typedef struct ln_sntp_cfg
{
	int sntpTimeZone;
	time_t tim;
	int serverCnt;
	
	char sntpServer[3][32];
	
	void (*init)(void);
	
}ln_sntp_cfg_t;

extern ln_sntp_cfg_t* ln_get_sntp_handle(void);


#endif /* __LN_AT_SNTP_H__ */
