
#ifndef _LN_TRANSPORT_UTILS_H_
#define _LN_TRANSPORT_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include "lwip/sockets.h" 
#include "usr_ctrl.h"

#define	STAILQ_ENTRY(type)						\
struct {								\
	struct type *stqe_next;	/* next element */			\
}

#define LN_TRANSPORT_MEM_CHECK(a, action) if (!(a)) {                                                      \
        LOG(LOG_LVL_ERROR,"%s:%d (%s): %s", __FILE__, __LINE__, __FUNCTION__, "Memory exhausted");       \
        action;                                                                                         \
        }
		
		
typedef struct
{
	uint8_t*  buffer_p;
	uint32_t  buffer_tail;
	uint32_t  buffer_head;
	uint32_t  buffer_len;
	int		  set_flag;	//是否处于数据发送过程中
	int		  blocks_flag; // 专门为分块发送数据设置
	int		  blocks_num; // 专门为分块发送数据设置
	int 	  data_len;

}fifo2_buffer_param_t;

void Fifo_2_Init_Buffer(fifo2_buffer_param_t *uart_no,uint8_t* buffer_addr,uint32_t buffer_max);
uint32_t Fifo_2_Save_Char(fifo2_buffer_param_t *uart_no,uint8_t ch);
uint32_t Fifo_2_Save_Data(fifo2_buffer_param_t *uart_no,uint8_t *data,uint32_t len);
uint32_t Fifo_2_Get_Char(fifo2_buffer_param_t *uart_no,uint8_t *ch);
uint32_t Fifo_2_Get_Data(fifo2_buffer_param_t *uart_no,uint8_t *data,uint32_t len);
uint32_t Fifo_2_Get_Buflen(fifo2_buffer_param_t *uart_no);

int resolve_dns(const char *host, struct sockaddr_in *ip);

#ifdef __cplusplus
}
#endif
#endif /* _LN_TRANSPORT_UTILS_H_ */
