#ifndef __DHCPD_API_H__
#define __DHCPD_API_H__

#include "lwip/ip4_addr.h"
#include "ln_types.h"

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif

typedef enum ln_dhcpd_err {
    DHCPD_ERR_NONE = 0u,

    DHCPD_ERR_INVALID_PARAM,
    DHCPD_ERR_SIZE_EXCEEDED,
    DHCPD_ERR_STATE,
    DHCPD_ERR_MEM,
    DHCPD_ERR_MSG,
    DHCPD_ERR_IP_USE_UP,
    DHCPD_ERR_OS_SERVICE,
} ln_dhcpd_err_t;

typedef struct server_config {
    ip4_addr_t      server;
    uint16_t        port;
    uint32_t        lease;
    uint32_t        renew;
    ip4_addr_t      ip_start;
    ip4_addr_t      ip_end;
    uint8_t         client_max;
}server_config_t;

int  dhcpd_start(void);
int  dhcpd_stop(void);
int  dhcpd_is_running(void);
int  dhcpd_ip_release(char *macaddr);

int  dhcpd_curr_config_set(server_config_t *server_config);//Warning: call it before start dhcpd.
int  dhcpd_curr_config_get(server_config_t *server_config);


#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif
