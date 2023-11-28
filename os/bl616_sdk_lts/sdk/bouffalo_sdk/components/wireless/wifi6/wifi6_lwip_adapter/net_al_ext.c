#include "fhost.h"
#include "net_al_ext.h"
#include "wifi_mgmr.h"
#include "platform_al.h"
#include "wifi_mgmr_ext.h"
#include "wifi_mgmr.h"

#include "lwip/netifapi.h"
#include "lwip/api.h"
#include "lwip/dns.h"
#include "lwip/prot/dhcp.h"
#include "dhcp_server.h"

#ifdef NET_AL_NO_IP
int net_al_ext_set_vif_ip(int fvif_idx, struct net_al_ext_ip_addr_cfg *cfg)
{
    return -1;
}

int net_al_ext_get_vif_ip(int fvif_idx, struct net_al_ext_ip_addr_cfg *cfg)
{
    return -1;
}
void net_al_ext_dhcp_connect(void)
{
}
void net_al_ext_dhcp_disconnect(void)
{
}
int net_dhcpd_start(net_if_t *net_if, int start, int limit)
{
    return -1
}
#else

static void net_if_disable_arp_for_us(net_if_t *netif)
{
    struct addr_ext ext;

    netif_get_addr_ext(netif, &ext);
    ext.arp_for_us_disable = 1;
    netif_set_addr_ext(netif, &ext);
}

static void net_if_enable_arp_for_us(net_if_t *netif)
{
    struct addr_ext ext;

    netif_get_addr_ext(netif, &ext);
    ext.arp_for_us_disable = 0;
    netif_set_addr_ext(netif, &ext);
}

static void net_if_set_default(net_if_t *net_if)
{
    netifapi_netif_set_default(net_if);
}

static void net_if_set_ip(net_if_t *net_if, uint32_t ip, uint32_t mask, uint32_t gw)
{
    if (!net_if)
        return;
    netifapi_netif_set_addr(net_if, (const ip4_addr_t *)&ip, (const ip4_addr_t *)&mask,
                   (const ip4_addr_t *)&gw);
}

int net_if_get_ip(net_if_t *net_if, uint32_t *ip, uint32_t *mask, uint32_t *gw)
{
    if (!net_if)
        return -1;

    if (ip)
        *ip = netif_ip4_addr(net_if)->addr;
    if (mask)
        *mask = netif_ip4_netmask(net_if)->addr;
    if (gw)
        *gw = netif_ip4_gw(net_if)->addr;

    return 0;
}

static uint32_t stop_dhcpc;
static int net_dhcp_start(net_if_t *net_if)
{
    #if LWIP_IPV4 && LWIP_DHCP
    if (netifapi_dhcp_start(net_if) ==  ERR_OK)
        return 0;
    #endif //LWIP_IPV4 && LWIP_DHCP
    return -1;
}

static void net_dhcp_stop(net_if_t *net_if)
{
    #if LWIP_IPV4 && LWIP_DHCP
    netifapi_dhcp_stop(net_if);
    #endif //LWIP_IPV4 && LWIP_DHCP
}

static int net_dhcp_release(net_if_t *net_if)
{
    #if LWIP_IPV4 && LWIP_DHCP
    if (netifapi_dhcp_release(net_if) ==  ERR_OK)
        return 0;
    #endif //LWIP_IPV4 && LWIP_DHCP
    return -1;
}

static int net_dhcp_address_obtained(net_if_t *net_if)
{
    #if LWIP_IPV4 && LWIP_DHCP
    if (dhcp_supplied_address(net_if))
        return 0;
    #endif //LWIP_IPV4 && LWIP_DHCP
    return -1;
}

static int net_set_dns(uint32_t dns_server)
{
    #if LWIP_DNS
    ip_addr_t ip;
    ip_addr_set_ip4_u32(&ip, dns_server);
    dns_setserver(0, &ip);
    return 0;
    #else
    return -1;
    #endif
}

static int net_get_dns(uint32_t *dns_server)
{
    #if LWIP_DNS
    const ip_addr_t *ip;

    if (dns_server == NULL)
        return -1;

    ip = dns_getserver(0);
    *dns_server = ip_addr_get_ip4_u32(ip);
    return 0;
    #else
    return -1;
    #endif
}

static int fhost_dhcp_stop(net_if_t *net_if)
{
    // Release DHCP lease
    if (!net_dhcp_address_obtained(net_if))
    {
        if (net_dhcp_release(net_if))
        {
            TRACE_FHOST(ERR, "Failed to release DHCP");
            return -1;
        }

        TRACE_FHOST(DHCP, "IP released");
    }

    // Stop DHCP
    net_dhcp_stop(net_if);

    return 0;
}

static int fhost_dhcp_start(net_if_t *net_if, uint32_t to_ms)
{
    uint32_t start_ms;

    // Run DHCP client
    if (net_dhcp_start(net_if))
    {
        TRACE_FHOST(ERR, "Failed to start DHCP");
        return DHCPC_START_FAILED;
    }

    start_ms = rtos_now(false);
    stop_dhcpc = 0;
    while ((net_dhcp_address_obtained(net_if)) &&
          (rtos_now(false) - start_ms < to_ms) &&
          (!stop_dhcpc))
    {
        rtos_task_suspend(100);
    }

    if (stop_dhcpc) {
        fhost_dhcp_stop(net_if);
        return DHCPC_START_ABORT;
    }

    if (net_dhcp_address_obtained(net_if))
    {
        TRACE_FHOST(ERR, "DHCP start timeout");
        fhost_dhcp_stop(net_if);
        return DHCPC_START_TIMEOUT;
    }

    return 0;
}

int net_al_ext_set_vif_ip(int fvif_idx, struct net_al_ext_ip_addr_cfg *cfg)
{
    int ret = 0;
    net_if_t *net_if;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return VIF_IDX_ERR;

    net_if = fhost_to_net_if(fvif_idx);
    if (!net_if)
        return VIF_NETIF_NULL;

    if (cfg->mode == IP_ADDR_NONE)
    {
        // clear current IP address
        fhost_dhcp_stop(net_if);
        wifiMgmr.wlan_sta.dhcp_started = 0;
        net_if_set_ip(net_if, 0, 0, 0);
        stop_dhcpc = 1;
        return 0;
    }

    if (cfg->mode == IP_ADDR_STATIC_IPV4)
    {
        // To be safe
        net_if_enable_arp_for_us(net_if);
        fhost_dhcp_stop(net_if);
        wifiMgmr.wlan_sta.dhcp_started = 0;
        net_if_set_ip(net_if, cfg->ipv4.addr, cfg->ipv4.mask, cfg->ipv4.gw);

        if (cfg->ipv4.dns)
            net_set_dns(cfg->ipv4.dns);
        else
            net_get_dns(&cfg->ipv4.dns);
    }
    else if (cfg->mode == IP_ADDR_DHCP_CLIENT)
    {
        if (0 == wifiMgmr.wlan_sta.dhcp_started) {
            ret = fhost_dhcp_start(net_if, cfg->dhcp.to_ms);
            if (ret) {
                return ret;
            } else {
                wifiMgmr.wlan_sta.dhcp_started = 1;
            }
        }

        net_if_get_ip(net_if, &(cfg->ipv4.addr), &(cfg->ipv4.mask), &(cfg->ipv4.gw));
        net_get_dns(&cfg->ipv4.dns);

        TRACE_FHOST(DHCP, "{FVIF-%d} ip=%pI4 gw=%pI4",
                    fvif_idx, TR_IP4(&(cfg->ipv4.addr)), TR_IP4(&(cfg->ipv4.gw)));
    }
    else
    {
        return VIF_MODE_ERR;
    }

    if (cfg->default_output)
         net_if_set_default(net_if);


    return VIF_SET_SUCCESS;
}

int net_al_ext_get_vif_ip(int fvif_idx, struct net_al_ext_ip_addr_cfg *cfg)
{
    net_if_t *net_if;

    if (fvif_idx >= NX_VIRT_DEV_MAX)
        return -1;

    net_if = fhost_to_net_if(fvif_idx);
    if (!net_if)
        return -1;

    if (!net_dhcp_address_obtained(net_if))
        cfg->mode = IP_ADDR_DHCP_CLIENT;
    else
        cfg->mode = IP_ADDR_STATIC_IPV4;

    cfg->default_output = false;

    net_if_get_ip(net_if, &(cfg->ipv4.addr), &(cfg->ipv4.mask), &(cfg->ipv4.gw));
    net_get_dns(&(cfg->ipv4.dns));

    return 0;
}

static int show_ip(uint8_t fhost_vif_idx)
{
    char fmt[] = "[%d] %s: MAC=%02x:%02x:%02x:%02x:%02x:%02x ip=%d.%d.%d.%d/%d %s%s\n";
    char ifname[NET_AL_MAX_IFNAME];
    char *state, *connected;
    struct net_al_ext_ip_addr_cfg ip_cfg;
    const uint8_t *mac_addr;

    if (fhost_env.vif[fhost_vif_idx].mac_vif &&
        (mac_vif_get_type(fhost_env.vif[fhost_vif_idx].mac_vif) != VIF_UNKNOWN))
    {
        state = "UP";
        if (mac_vif_get_active(fhost_env.vif[fhost_vif_idx].mac_vif))
            connected = ",CONNECTED";
        else
            connected = "";
    }
    else
    {
        state = "DOWN";
        connected = "";
    }
    mac_addr = net_if_get_mac_addr(fhost_to_net_if(fhost_vif_idx));

    if (fhost_vif_name(fhost_vif_idx, ifname, NET_AL_MAX_IFNAME) < 0)
        return -1;


    if (net_al_ext_get_vif_ip(fhost_vif_idx, &ip_cfg))
        return -1;

    printf(fmt, fhost_vif_idx, ifname,
                mac_addr[0], mac_addr[1], mac_addr[2],
                mac_addr[3], mac_addr[4], mac_addr[5],
                ip_cfg.ipv4.addr & 0xff, (ip_cfg.ipv4.addr >> 8) & 0xff,
                (ip_cfg.ipv4.addr >> 16) & 0xff, (ip_cfg.ipv4.addr >> 24) & 0xff,
                32 - co_clz(ip_cfg.ipv4.mask), state, connected);
    return 0;
}

static int wifi_sta_dhcpc_start(uint8_t fhost_vif_idx)
{
    struct net_al_ext_ip_addr_cfg ip_cfg;
    ip_cfg.mode = IP_ADDR_DHCP_CLIENT;
    ip_cfg.default_output = true;
    ip_cfg.dhcp.to_ms = 15000;
    int ret = 0;

    ret = net_al_ext_set_vif_ip(fhost_vif_idx, &ip_cfg);
    if (ret)
    {
        if (ret != DHCPC_START_ABORT) {
            if (ret == DHCPC_START_TIMEOUT) {
                printf("dhcpc obtain ip failed\n");
            } else {
                printf("dhcpc start error, ret is %d\n", ret);
            }
            if (!wifiMgmr.disable_autoreconnect) {
                wifi_mgmr_sta_connect(&wifiMgmr.sta_connect_param);
            } else {
                wifi_sta_disconnect();
            }
        } else {
            printf("dhcpc start abort\n");
        }
        PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 0);
        return -1;
    } else {
        platform_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP);
        show_ip(fhost_vif_idx);
        PLATFORM_HOOK(prevent_sleep, PSM_EVENT_CONNECT, 0);
        if(wifi_mgmr_sta_connect_params_get() & LOW_RATE_CONNECT) {
            wifi_mgmr_rate_config(0xFFFF);
        }
    }
    return 0;
}

static RTOS_TASK_FCT(fhost_wpa_connected_task) {
    wifi_sta_dhcpc_start(MGMR_VIF_STA);

    rtos_task_delete(NULL);
}

static int wifi_sta_dhcpc_stop(uint8_t fhost_vif_idx)
{
    struct net_al_ext_ip_addr_cfg ip_cfg;
    ip_cfg.mode = IP_ADDR_NONE;
    if (net_al_ext_set_vif_ip(fhost_vif_idx, &ip_cfg)) {
        printf("dhcpc stop error\n");
        return -1;
    }
    return 0;
}

int net_dhcpd_start(net_if_t *net_if, int start, int limit)
{
    netifapi_netif_set_link_up(net_if);
    dhcpd_start(net_if, start, limit);

    return 0;
}

static ip4_addr_t saved_ip_addr = {IPADDR_ANY};
static ip4_addr_t saved_ip_mask = {IPADDR_ANY};
static ip4_addr_t saved_ip_gw = {IPADDR_ANY};

static void qc_callback(struct netif *netif) {
  printf("[quick_connect] dhcp bind new ip callback\r\n"
         "  IP: %s\r\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  printf("  MK: %s\r\n", ip4addr_ntoa(netif_ip4_netmask(netif)));
  printf("  GW: %s\r\n", ip4addr_ntoa(netif_ip4_gw(netif)));

  ip4_addr_copy(saved_ip_addr, *netif_ip4_addr(netif));
  ip4_addr_copy(saved_ip_mask, *netif_ip4_netmask(netif));
  ip4_addr_copy(saved_ip_gw, *netif_ip4_gw(netif));
#define  EV_WIFI                  0x0002
#define  CODE_WIFI_ON_GOT_IP      7
  platform_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP);
}

static void net_quick_dhcp_restore(net_if_t *netif) {
    struct addr_ext ext;

    netifapi_netif_set_default(netif);

    netif_get_addr_ext(netif, &ext);
    ext.dhcp_qc_callback = qc_callback;
    netif_set_addr_ext(netif, &ext);

    if (!ip4_addr_isany_val(saved_ip_addr)) {
        netifapi_netif_set_addr(netif, &saved_ip_addr, &saved_ip_mask, &saved_ip_gw);
        platform_post_event(EV_WIFI, CODE_WIFI_ON_GOT_IP);
        /* already get ip */
        netifapi_dhcp_renew(netif);
    } else {
        /* start dhcp */
        netifapi_dhcp_start(netif);
    }
}

static void net_quick_dhcp_stop(net_if_t *netif) {
    struct addr_ext ext;
    struct dhcp *dhcp = netif_dhcp_data(netif);

    netif_get_addr_ext(netif, &ext);
    ext.dhcp_qc_callback = NULL;
    netif_set_addr_ext(netif, &ext);

    if(dhcp) {
        // stop dhcp timer
        dhcp->state = DHCP_STATE_OFF;
        dhcp->request_timeout = 0;
    }
}

void net_al_ext_dhcp_connect(void)
{
    net_if_t *netif = fhost_to_net_if(MGMR_VIF_STA);

#if !LWIP_TCPIP_CORE_LOCKING
    #error To do add netif msg call
#endif
    net_if_disable_arp_for_us(netif);
    if (wifiMgmr.sta_connect_param.use_dhcp) {
        if (wifiMgmr.sta_connect_param.quick_connect) {
            net_quick_dhcp_restore(netif);
        } else {
            printf("start dhcping ... \r\n");
            rtos_task_create(fhost_wpa_connected_task, "fhost_wpa_connected_task",
                             WPA_CONNECTED_TASK, 512, NULL, FHOST_WPA_PRIORITY, NULL);
        }
    }
}

void net_al_ext_dhcp_disconnect(void)
{
    if (wifiMgmr.sta_connect_param.use_dhcp) {
        if (wifiMgmr.sta_connect_param.quick_connect) {
            net_if_t *netif = fhost_to_net_if(MGMR_VIF_STA);
            net_quick_dhcp_stop(netif);
        } else {
            wifi_sta_dhcpc_stop(MGMR_VIF_STA);
        }
    }
}
#endif // NET_AL_NO_IP
