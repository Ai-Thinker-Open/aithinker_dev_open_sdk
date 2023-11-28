#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/ip4_addr.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define NETDEV_HOSTNAME_LEN_MAX    (20)

typedef enum{
    NETDEV_DOWN = 0,
    NETDEV_UP
} netdev_state_t;

typedef enum{
    NETDEV_LINK_DOWN = 0,
    NETDEV_LINK_UP
} netdev_link_state_t;

typedef enum {
    NETIF_IDX_STA = 0,     /**< LN882H station interface */
    NETIF_IDX_AP,          /**< LN882H soft-AP interface */
    NETIF_IDX_MAX
} netif_idx_t;

typedef struct {
    ip4_addr_t ip; 
    ip4_addr_t netmask;
    ip4_addr_t gw;
} tcpip_ip_info_t;

typedef void (*netdev_get_ip_cb_t)(struct netif *nif);


/*------------------------ lwip netdev api ---------------------------------------*/
int                  lwip_tcpip_init(void);
int                  netdev_set_state(netif_idx_t nif_idx, netdev_state_t state);
void                 netdev_set_active(netif_idx_t nif_idx);
netif_idx_t          netdev_get_active(void);
netdev_link_state_t  netdev_get_link_state(netif_idx_t nif_idx);
int                  netdev_set_ip_info(netif_idx_t nif_idx, tcpip_ip_info_t *ip_info);
int                  netdev_get_ip_info(netif_idx_t nif_idx, tcpip_ip_info_t *ip_info);
void                 netdev_get_ip_cb_set(netdev_get_ip_cb_t cb);
netdev_get_ip_cb_t   netdev_get_ip_cb_get(void);
int                  netdev_got_ip(void);
int                  netdev_set_mac_addr(netif_idx_t nif_idx, uint8_t *mac_addr);
int                  netdev_get_mac_addr(netif_idx_t nif_idx, uint8_t *mac_addr);
struct netif *       netdev_get_netif(netif_idx_t nif_idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ETHERNETIF_H__ */
