#include "lwip/ip_addr.h"
#include "dhcpd.h"
#include "dhcpd_api.h"

static int is_mac_all_zero(char *mac)
{
    if(!memcmp("\x00\x00\x00\x00\x00\x00", mac, 6)){
        return LN_TRUE;
    }
    return LN_FALSE;
}

static dhcpd_ip_item_t *get_next_usable_ip_item(dhcpd_ip_item_t *ip_pool, char *sta_addr)
{
    uint8_t dhcp_client_max = dhcpd_get_client_max();
    int i, usable = -1, usable2 = -1;
    dhcpd_ip_item_t *ip_item;

    for (i = 0; i < MIN(DHCPD_IP_POOL_SIZE, dhcp_client_max); i++) {
        ip_item = &(ip_pool[i]);
        DHCPD_PRINTF("%s i=%d, mac="MACSTR", ip=%d.%d.%d.%d, allocted=%d\r\n", __func__, i, MAC2STR(ip_item->mac), ip4_addr1(&ip_item->ip), ip4_addr2(&ip_item->ip), ip4_addr3(&ip_item->ip), ip4_addr4(&ip_item->ip), ip_item->allocted);
        if(!memcmp(ip_item->mac, sta_addr, 6)){
            DHCPD_PRINTF("Got i=%d, mac="MACSTR", ip=%d.%d.%d.%d, allocted=%d\r\n", i, MAC2STR(ip_item->mac), ip4_addr1(&ip_item->ip), ip4_addr2(&ip_item->ip), ip4_addr3(&ip_item->ip), ip4_addr4(&ip_item->ip), ip_item->allocted);
            return ip_item;
        }

        if((usable < 0)  && (ip_item->allocted == LN_FALSE) && is_mac_all_zero((char *)ip_item->mac)){
            usable = i;
            DHCPD_PRINTF("Got i=%d, mac="MACSTR", ip=%d.%d.%d.%d, allocted=%d\r\n", i, MAC2STR(ip_item->mac), ip4_addr1(&ip_item->ip), ip4_addr2(&ip_item->ip), ip4_addr3(&ip_item->ip), ip4_addr4(&ip_item->ip), ip_item->allocted);
        }
        if((usable2 < 0) && (ip_item->allocted == LN_FALSE)) {
            usable2 = i;
            DHCPD_PRINTF("Got i=%d, mac="MACSTR", ip=%d.%d.%d.%d, allocted=%d\r\n", i, MAC2STR(ip_item->mac), ip4_addr1(&ip_item->ip), ip4_addr2(&ip_item->ip), ip4_addr3(&ip_item->ip), ip4_addr4(&ip_item->ip), ip_item->allocted);
        }
    }

    if((usable < 0) && (usable2 >= 0)){
        usable = usable2;
    }

    if((usable >= 0) && usable < MIN(DHCPD_IP_POOL_SIZE, dhcp_client_max)){
        return &(ip_pool[usable]);
    }

    return NULL;
}

static dhcpd_ip_item_t *get_ip_item(dhcpd_ip_item_t *ip_pool, char *sta_addr)
{
    uint8_t dhcp_client_max = dhcpd_get_client_max();
    int i;
    dhcpd_ip_item_t *ip_item;
    for (i = 0; i < MIN(DHCPD_IP_POOL_SIZE, dhcp_client_max); i++) {
        ip_item = &(ip_pool[i]);
        if(!memcmp(ip_item->mac, sta_addr, 6)){
            return ip_item;
        }
    }
    return NULL;
}

static int set_ip_item_status(dhcpd_ip_item_t *ip_item, int st)
{
    ip_item->allocted = st;
    return DHCPD_ERR_NONE;
}

int dhcpd_ip_allocate(dhcpd_ip_item_t *ip_pool, char *mac, ip4_addr_t *ip_addr, int pre_allocated)
{
    dhcpd_ip_item_t *ip_item = NULL;

    if(NULL == (ip_item = get_next_usable_ip_item(ip_pool, mac))){
        DHCPD_ERR("ip allocatno unused ip in the ip pools!\r\n");
        return DHCPD_ERR_IP_USE_UP;
    }

    memcpy(ip_item->mac, mac, 6);
	ip_addr_copy(*ip_addr, ip_item->ip);

    if(pre_allocated && ip_item->allocted == LN_FALSE) {
        set_ip_item_status(ip_item, LN_TRUE);
    }

	return DHCPD_ERR_NONE;
}

int dhcpd_ip_release(char *macaddr)
{
    dhcpd_ip_item_t *ip_item = NULL;

    ip_item = get_ip_item(dhcpd_get_ip_pool(), macaddr);
    set_ip_item_status(ip_item, LN_FALSE);
    return DHCPD_ERR_NONE;
}
