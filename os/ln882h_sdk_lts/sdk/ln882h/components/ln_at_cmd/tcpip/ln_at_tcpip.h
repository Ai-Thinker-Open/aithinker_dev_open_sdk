#ifndef  __LN_AT_TCPIP_H__
#define  __LN_AT_TCPIP_H__

#include <lwip/sockets.h>
#include "lwip/inet.h"


typedef struct
{
    uint8_t enable;
    
    char dns_ip_1[20];
    char dns_ip_2[20];
    char dns_ip_3[20];
}dns_ip_t;

extern dns_ip_t g_dns_ip_cur;


#endif
