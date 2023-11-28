#ifndef __DHCPD_H__
#define __DHCPD_H__

#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#include "utils/debug/log.h"
#include "ln_types.h"
#include "ln_utils.h"

#define BOOTP_REPLAY_PORT    68
#define DHCPD_IP_POOL_SIZE   4

#ifdef __cplusplus /* If this is a C++ compiler, use C linkage */
extern "C" {
#endif


typedef struct{
    int             allocted;
    uint8_t         mac[6];
    ip4_addr_t      ip;
}dhcpd_ip_item_t;

#define DHCPD_PRINTF(...)				LOG(LOG_LVL_DEBUG, __VA_ARGS__)
#define DHCPD_ERR(...)					LOG(LOG_LVL_ERROR, __VA_ARGS__)


dhcpd_ip_item_t *dhcpd_get_ip_pool(void);
uint8_t          dhcpd_get_client_max(void);
int              dhcpd_ip_allocate(dhcpd_ip_item_t *ip_pool, char *mac, ip4_addr_t *ip_addr, int pre_allocated);


#ifdef __cplusplus /* If this is a C++ compiler, end C linkage */
}
#endif

#endif
