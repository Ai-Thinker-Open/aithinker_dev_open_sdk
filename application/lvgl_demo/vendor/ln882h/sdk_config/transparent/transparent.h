#ifndef _TRANSPARENT_H_
#define _TRANSPARENT_H_

#include "osal/osal.h"
#include "hal/hal_timer.h"
#include "usr_ctrl.h"

#include "transport.h"

#define CONNECT_AP_TIME_TOTAL	1000000
//extern TIMER_Index gTimerIndex;

#define TRANSP_STACK_SIZE 		1024
#define TRANSP_SSL_STACK_SIZE	256*20
#define TRANSP_BUF_LEN			2920

#define TRANSP_MAX_LINK_NUM 5

#define TRY_EXIT_TASK_STACK_SIZE 			6 * 256 //Byte
#define TCP_CIPSERVER_TASK_STACK_SIZE 		2048

typedef void (*init_transp_func)(void);
typedef int (*start_link_func)(int id);
typedef void (*exit_link_func)(int id);
typedef int (*push_back_func)(unsigned char *data, unsigned int data_len);
typedef int (*start_server)(int mode, int port);

typedef enum
{
	STA_GOT_IP = 2,
	STA_TCPUDP = 3,
	STA_DISCON = 4,
	STA_NOAP = 5
} ln_cipstatus_t;

typedef enum
{
    TRANSPARENT_NO_LINK,
	TRANSPARENT_MODE_TCP,
	TRANSPARENT_MODE_UDP,
	TRANSPARENT_MODE_SSL,
	TRANSPARENT_MODE_ERROR
}ln_transparent_link_mode_t;

struct server_ext_info
{
	bool ca_cert;
	int server_port;
	int server_mode;
};

struct ln_netlink_t
{
	int id;
	char type;
	char remoteIp[64];
	char peerName[64];

	int remotePort;
	int localPort;
	char tetype;

	bool bIsClosed;
	bool bCipRcvHasFetched;
	bool isMalloced;
	unsigned char *tx_buff;
	unsigned char *rx_buff;
	unsigned char *fifo_buff;
	int tx_offset;
	int sendLen;
	int rx_offset;
	int buff_size;

	OS_Timer_t transp_timer;
	OS_Semaphore_t transp_sem;
	OS_Thread_t  transp_task_tcp;
	fifo2_buffer_param_t transp_fifo;

	ln_transport_handle_t tcpHandle;
};

struct ln_trans_sys_stat_t
{
	char cStaStat;
	bool bCipMux;
	bool bCipMode;
	bool bSend;
	bool bCipRecvMode;
	bool bCipDinfo;

	int ServerSock;
	int ServerPort;
	int ServerMaxConn;

	bool bCwAutoConn;
	int curId;

    bool ssl_ca_en;
	bool ssl_cert_pk_en;

	bool ssl_existed;
	int ssl_buff_size;
	
	OS_Queue_t tryCloseQueue;
	struct server_ext_info Server_Ext_Info;
	struct ln_netlink_t tLinkItems[TRANSP_MAX_LINK_NUM];
	
	init_transp_func	_init;
	start_link_func		_start_link;
	exit_link_func		_exit_link;
	push_back_func		_push_back;
	start_server		_start_server;
};

typedef struct ln_netlink_t * ln_netlink_handle_t;
typedef struct ln_trans_sys_stat_t ln_trans_sys_stat_item_t;
typedef struct ln_trans_sys_stat_t * ln_trans_sys_stat_handle_t;

extern ln_trans_sys_stat_handle_t ln_get_tcpip_handle(void);

#endif
