#include "proj_config.h"
#include "lwip/netif.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/netifapi.h"
#include "netif/ethernetif.h"
#include "netif/etharp.h"
#include "dhcpd_api.h"
#include "wifi.h"
#include "utils/debug/log.h"
#include "ln_compiler.h"
#include "ln_utils.h"
#include "utils/system_parameter.h"


#define TCPIP_PKT_SIZE_MAX              (NETIF_MTU + PBUF_LINK_HLEN)//max in/output pocket size = MTU + LINK_HEAD_LEN

#define IF_NAME_STA "ST" // Only support two ascii characters
#define IF_NAME_AP  "AP" // Only support two ascii characters

typedef struct {
    struct netif              nif;
} netdev_t;

#define netdev2netif(ndev)   ((struct netif *)(ndev))
#define netif2netdev(nif)    ((netdev_t *)(nif))

typedef struct {
    netdev_t           netdev[NETIF_IDX_MAX];
    netif_idx_t        active;
} ethernetif_t;


static ethernetif_t  g_ethernetif = {0};
static netdev_get_ip_cb_t g_get_ip_cb  = NULL;

static ethernetif_t *ethernetif_get_handle(void) {
    return &g_ethernetif;
}

static netdev_t *ethernetif_get_netdev(netif_idx_t nif_idx) {
    ethernetif_t *eth_if = ethernetif_get_handle();
    netdev_t *netdev = NULL;

    if (nif_idx >= NETIF_IDX_MAX) {
        LOG(LOG_LVL_ERROR, "nif_idx(%d) invalid!\r\n", nif_idx);
        return NULL;
    }
    netdev = &(eth_if->netdev[nif_idx]);
    return netdev;
}

struct netif *netdev_get_netif(netif_idx_t nif_idx) {
    netdev_t *netdev = ethernetif_get_netdev(nif_idx);

    if (netdev) {
        return &(netdev->nif);
    }
    return (struct netif *)NULL;
}

static err_t low_level_output(struct netif *netif, struct pbuf *pkt)
{
    err_enum_t ret = ERR_OK;

    struct pbuf *q = NULL;
    uint8_t * frame_buf  = NULL;
    uint16_t payload_offset = 0;

    if(!netif_is_link_up(netif)) {
        LOG(LOG_LVL_INFO, "[%s, %d] netif is not link up.\r\n", __func__, __LINE__);
        ret = ERR_IF;
        return ret;
    }

    frame_buf = OS_Malloc(pkt->tot_len);
    if (frame_buf == NULL)
    {
        ret = ERR_MEM;
        return ret;
    }

    for(q = pkt; q != NULL; q = q->next) {
        memcpy(frame_buf + payload_offset, (u8_t*)q->payload, q->len );
        payload_offset += q->len;
    }
    wifi_send_ethernet_pkt(frame_buf, pkt->tot_len, 20, 5);

    OS_Free(frame_buf);
    return ret;
}

__STATIC_INLINE__ void low_level_input(netif_idx_t nif_idx, uint8_t *data, uint16_t len)
{
    struct pbuf *pkt = NULL, *q = NULL;
    struct netif * nif = NULL;

    if(NULL == (pkt =pbuf_alloc(PBUF_RAW, len, PBUF_POOL))) {
        LOG(LOG_LVL_WARN, "[%s, %d] Can not malloc pbuff(request len=%d).\r\n", __func__, __LINE__, len);
        return;
    }

    for(q = pkt; q != NULL; q = q->next) {
        memcpy(q->payload, data, q->len);
        data += q->len;
    }

    nif = &g_ethernetif.netdev[nif_idx].nif;
    if(nif->input(pkt, nif) != ERR_OK) {
        pbuf_free(pkt);
    }
}

//TODO:AP & STA will coexist in the future.
//static void netif_sta_low_level_input_callback(uint8_t *data, uint16_t len);
//static void netif_softap_low_level_input_callback(uint8_t *data, uint16_t len);
static void netif_low_level_input_callback(uint8_t *data, uint16_t len) {
    netif_idx_t nif_idx = netdev_get_active();

    //hexdump(LOG_LVL_ERROR, (const char *)"low_level_input", data, len);
    low_level_input(nif_idx, data, len);
}

static err_t ethernetif_init(struct netif *netif, netif_idx_t nif_idx)
{
    LWIP_ASSERT("netif != NULL", (netif != NULL));

    if (nif_idx == NETIF_IDX_STA) {
#if LWIP_NETIF_HOSTNAME
        netif->hostname = "sta";
#endif
        memcpy(netif->name, IF_NAME_STA, sizeof(netif->name));
    } else if (nif_idx == NETIF_IDX_AP) {
#if LWIP_NETIF_HOSTNAME
        netif->hostname = "ap";
#endif
        memcpy(netif->name, IF_NAME_AP, sizeof(netif->name));
    } else {
        LOG(LOG_LVL_ERROR, "netif idx error\r\n");
        return ERR_ARG;
    }

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    netif->hwaddr_len = NETIF_MAX_HWADDR_LEN;
    netif->mtu = NETIF_MTU;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IGMP
    /* make LwIP_Init do igmp_start to add group 224.0.0.1 */
    netif->flags |= NETIF_FLAG_IGMP;
#endif

    return ERR_OK;
}

static err_t ethernetif_sta_init(struct netif *nif) {
    return ethernetif_init(nif, NETIF_IDX_STA);
}

static err_t ethernetif_softap_init(struct netif *nif) {
    return ethernetif_init(nif, NETIF_IDX_AP);
}

//void    ln_at_printf(const char *format, ...);
//wifi_mode_t ln_get_wifi_mode_from_at(void);
static void print_netdev_info(struct netif *nif)
{
    uint8_t * mac = nif->hwaddr;
	//ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	//ln_wifi_base_plus_cfg_t *pWifi = ln_get_wifi_base_plus_handle();

    LOG(LOG_LVL_INFO, "+--------------- net device info ------------+\r\n");
    LOG(LOG_LVL_INFO, "|netif hostname: %-16s            |\r\n", nif->hostname);
    LOG(LOG_LVL_INFO, "|netif ip      = %-16s            |\r\n", ip4addr_ntoa(&nif->ip_addr));
    LOG(LOG_LVL_INFO, "|netif mask    = %-16s            |\r\n", ip4addr_ntoa(&nif->netmask));
    LOG(LOG_LVL_INFO, "|netif gateway = %-16s            |\r\n", ip4addr_ntoa(&nif->gw));
    LOG(LOG_LVL_INFO, "|netif mac     : [%02X:%02X:%02X:%02X:%02X:%02X] %-7s |\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], "");
    LOG(LOG_LVL_INFO, "+--------------------------------------------+\r\n");
	
//	if (WIFI_MODE_STATION == ln_get_wifi_mode_from_at())
//	{
//		p->cStaStat = STA_GOT_IP;
//		ln_at_printf("WIFI GOT IP\r\n");
//		pWifi->ip_got();
//	}
}

static void sta_netif_link_changed_cb(struct netif *nif) {
    LN_UNUSED(nif);
    LOG(LOG_LVL_DEBUG, "[%s:%d]\r\n", __func__, __LINE__);
}

static void sta_netif_status_changed_cb(struct netif *nif)
{
		uint8_t dhcp_en = 0;
		sysparam_dhcp_en_get(&dhcp_en);

		if (dhcp_en)
		{
				if (dhcp_supplied_address(nif) && (nif->flags & NETIF_FLAG_UP) && (netif_is_link_up(nif))) {
						print_netdev_info(nif);
						if (g_get_ip_cb)
						{
								g_get_ip_cb(nif);
								wifi_set_allow_cpu_sleep_flag(1);
						}
				}

		}
		else
		{
				if ((nif->flags & NETIF_FLAG_UP) && (netif_is_link_up(nif))) {
						print_netdev_info(nif);
						if (g_get_ip_cb)
						{
								g_get_ip_cb(nif);
								wifi_set_allow_cpu_sleep_flag(1);
						}
				}
		}
}

static void tcpip_init_done(void *arg)
{
    LN_UNUSED(arg);

    struct netif *nif = NULL;
    netif_idx_t nif_idx;

    //for STA
    nif_idx = NETIF_IDX_STA;
    nif = netdev_get_netif(nif_idx);
    netif_add(nif, NULL, NULL, NULL, NULL, ethernetif_sta_init, tcpip_input);
    netif_set_default(nif);

    //for AP
    nif_idx = NETIF_IDX_AP;
    nif = netdev_get_netif(nif_idx);
    netif_add(nif, NULL, NULL, NULL, NULL, ethernetif_softap_init, tcpip_input);

    wifi_if_reg_recv_ethernet_pkt_callback(&netif_low_level_input_callback);

    LOG(LOG_LVL_INFO, "TCP/IP initialized.\r\n");
}

////////////////////////////lwip netdev api/////////////////////////////////////////
int lwip_tcpip_init(void)
{
    /**
     * When powered on, if the tcp task is not the first task to run, 
     * synchronous initialization is recommended.
     * 
     * sync init:
     *    tcpip_init(NULL, NULL);
     *    tcpip_init_done(NULL);
     * async init:
     *    tcpip_init(tcpip_init_done, NULL);
    */
    tcpip_init(NULL, NULL);
    tcpip_init_done(NULL);
    return 0;
}

int netdev_set_state(netif_idx_t nif_idx, netdev_state_t state)
{
		struct netif *nif = NULL;
		uint8_t dhcp_en = 0;
		sysparam_dhcp_en_get(&dhcp_en);

		if (nif_idx >= NETIF_IDX_MAX) {
				LOG(LOG_LVL_ERROR, "netif idx(%d) invalid!\r\n", nif_idx);
				return -1;
		}

		nif = netdev_get_netif(nif_idx);

		if (nif_idx == NETIF_IDX_STA)
		{
				if (state == NETDEV_UP)
				{
						netif_set_status_callback(nif, sta_netif_status_changed_cb);
						netif_set_link_callback(nif, sta_netif_link_changed_cb);

						netifapi_netif_set_link_up(nif); /* netifapi_netif_set_link_up 在前 */
						netifapi_netif_set_up(nif);      /* netifapi_netif_set_up 在后 */

						if (dhcp_en)
						{
								netifapi_dhcp_release(nif);
								netifapi_dhcp_stop(nif);
								netifapi_dhcp_start(nif);
						}
				}
				else
				{
						netifapi_netif_set_down(nif);
						netifapi_netif_set_link_down(nif);

						if (dhcp_en)
						{
								netifapi_dhcp_release(nif);
								netifapi_dhcp_stop(nif);
						}
						netif_set_status_callback(nif, NULL);
						netif_set_link_callback(nif, NULL);
				}
		}
		else if (nif_idx == NETIF_IDX_AP)
		{
				if (state == NETDEV_UP)
				{
						netifapi_netif_set_up(nif);
						netifapi_netif_set_link_up(nif);
						dhcpd_stop();
						dhcpd_start();
				}
				else
				{
						dhcpd_stop();
						netifapi_netif_set_link_down(nif);
						netifapi_netif_set_down(nif);
				}
		}
		return 0;
}
void netdev_set_active(netif_idx_t nif_idx)
{
    ethernetif_t *eth_if = ethernetif_get_handle();
    eth_if->active = nif_idx;
    netif_set_default(netdev_get_netif(nif_idx));
}

netif_idx_t netdev_get_active(void)
{
    ethernetif_t *eth_if = ethernetif_get_handle();
    return eth_if->active;
}

netdev_link_state_t netdev_get_link_state(netif_idx_t nif_idx)
{
    netdev_t *ndev = ethernetif_get_netdev(nif_idx);
                
    if (netif_is_link_up(&ndev->nif) && (ndev->nif.flags & NETIF_FLAG_UP)) {
        return NETDEV_LINK_UP;
    } else {
        return NETDEV_LINK_DOWN;
    }
}


int netdev_got_ip(void)
{
    struct netif *nif = netdev_get_netif(netdev_get_active());
    /* (DHCP supplied) or ((Static IP) and (LinkUp)) */
    return ((0 != dhcp_supplied_address(nif)) || \
            ((IPADDR_ANY != nif->ip_addr.addr) && (nif->flags & (NETIF_FLAG_LINK_UP | NETIF_FLAG_UP))));       
}
int netdev_set_mac_addr(netif_idx_t nif_idx, uint8_t *mac_addr)
{
    struct netif *nif = netdev_get_netif(nif_idx);

    if (nif && mac_addr) {
        memcpy(nif->hwaddr, mac_addr, NETIF_MAX_HWADDR_LEN);
        return 0;
    }
    return -1;
}

int netdev_get_mac_addr(netif_idx_t nif_idx, uint8_t *mac_addr)
{
    struct netif *nif = netdev_get_netif(nif_idx);

    if (nif && mac_addr) {
        memcpy(mac_addr, nif->hwaddr, NETIF_MAX_HWADDR_LEN);
        return 0;
    }
    return -1;
}

int netdev_set_ip_info(netif_idx_t nif_idx, tcpip_ip_info_t *ip_info)
{
    netdev_t *ndev = ethernetif_get_netdev(nif_idx);

    if (ndev && ip_info) {
        netifapi_netif_set_addr(netdev2netif(ndev), &ip_info->ip, &ip_info->netmask, &ip_info->gw);
        return 0;
    }
    return -1;
}

int netdev_get_ip_info(netif_idx_t nif_idx, tcpip_ip_info_t *ip_info)
{
    netdev_t *ndev = ethernetif_get_netdev(nif_idx);

    if (ndev && ip_info) {
        memcpy(&ip_info->ip,      &(ndev->nif.ip_addr), sizeof(ip4_addr_t));
        memcpy(&ip_info->netmask, &(ndev->nif.netmask), sizeof(ip4_addr_t));
        memcpy(&ip_info->gw,      &(ndev->nif.gw),      sizeof(ip4_addr_t));
        return 0;
    }
    return -1;
}

void netdev_get_ip_cb_set(netdev_get_ip_cb_t cb)
{
    g_get_ip_cb = cb;
}

netdev_get_ip_cb_t netdev_get_ip_cb_get(void)
{
    return g_get_ip_cb;
}
