#include "ln_at_tcpip.h"

#include "ln_at_transfer.h"
#include "usr_ctrl.h"
#include "utils/debug/log.h"
#include "ln_utils.h"
#include "ln_at.h"
#include "ping.h"
#include "iperf.h"
#include <stdlib.h>
#include <string.h>

#include "wifi.h"
#include "system_parameter.h"

#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/inet_chksum.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/ip_addr.h"
#include "lwip/dns.h"

#include "netif/ethernetif.h"
#include "transparent.h"
#include "ln_at_cmd.h"

ln_at_err_t ln_at_domain_excute(uint8_t para_num, const char *name)
{
    LN_UNUSED(para_num);
    LN_UNUSED(name);

    bool is_default = false;
    uint8_t para_index = 1;

    bool bRes = false;
    ip_addr_t dnsaddr;
    char *ptr = NULL;
    int int_param;

    if(para_num >= 1)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ptr))
        {
            goto __exit;
        }
    }
    if(para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
        {
            goto __exit;
        }
        if (int_param < 1 || int_param > 2)
        {
            goto __exit;
        }
    }

    for (size_t i = 0; i < 5; i++)
    {
        if (dns_gethostbyname((char *)ptr, &(dnsaddr), NULL, NULL) == ERR_OK)
        {
            bRes = true;
            break;
        }
        OS_MsDelay(500);
    }

    if (!bRes)
    {
        ln_at_printf("DNS Fail\r\n");
        goto __exit;
    }

    ptr = ip_ntoa(&dnsaddr);
    ln_at_printf("+CIPDOMAIN:%s\r\n", ptr);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
LN_AT_CMD_REG(CIPDOMAIN, NULL, ln_at_domain_excute, NULL, NULL);

static u16_t ping_seq_num = 0;
static u32_t ping_time;
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
    size_t i;
    size_t data_len = len - sizeof(struct icmp_echo_hdr);

    ICMPH_TYPE_SET(iecho, ICMP_ECHO);
    ICMPH_CODE_SET(iecho, 0);
    iecho->chksum = 0;
    iecho->id     = 0xAFAF;
    iecho->seqno  = htons(++ping_seq_num);

    /* fill the additional data buffer with some data */
    for(i = 0; i < data_len; i++) {
        ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
    }

    iecho->chksum = inet_chksum(iecho, len);
}

static err_t
ping_send(int s, ip_addr_t *addr)
{
    int err;
    struct icmp_echo_hdr *iecho;
    struct sockaddr_in to;
    size_t ping_size = sizeof(struct icmp_echo_hdr) + 1400;
    LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

    iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
    if (!iecho) {
        return ERR_MEM;
    }

    ping_prepare_echo(iecho, (u16_t)ping_size);

    to.sin_len = sizeof(to);
    to.sin_family = AF_INET;
    inet_addr_from_ip4addr(&to.sin_addr, addr);

    err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

    mem_free(iecho);

    return (err ? ERR_OK : ERR_VAL);
}

static bool ping_recv(int s)
{
    char buf[64];
    int fromlen = sizeof(struct sockaddr_in), len;
    struct sockaddr_in from;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    while(1)
    {
        if((len = lwip_recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
        {
            if (len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
            {
                ip_addr_t fromaddr;
                inet_addr_to_ip4addr(&fromaddr, &from.sin_addr);
                Log_i("ping recv: %s  %"U32_F" ms.", ipaddr_ntoa(&fromaddr), (OS_GetTicks() - ping_time));

                Log_i("+%"U32_F"ms", (OS_GetTicks() - ping_time));
                ln_at_printf("+PING:%"U32_F"\r\n", (OS_GetTicks() - ping_time));
                Log_i("OK");

                iphdr = (struct ip_hdr *)buf;
                iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
                if ((iecho->id == 0xAFAF) && (iecho->seqno == htons(ping_seq_num)))
                {
                    /* do some ping result processing */
                    //PING_RESULT(ICMPH_TYPE(iecho) == ICMP_ER));
                    return true;
                }
                else
                {
                    if(2000 > (OS_GetTicks() - ping_time)) {
                        Log_i("ping: recive icmp packet mismatch");
                    } else {
                        return false;
                    }
                }
            }
        }
        else
        {
            Log_i("ping: recv - %"U32_F" ms - timeout", (OS_GetTicks() - ping_time));
            Log_i("+timeout");
            Log_i("ERROR");
            return false;
        }
    }

    /* do some ping result processing */
    //return false;
}

static bool ping_try_once(void *arg)
{
    int s;

    struct timeval timeout = { (2000 / OS_MSEC_PER_SEC), (2000 % OS_MSEC_PER_SEC)};

    ip_addr_t target_ip;
    tcpip_ip_info_t  ip_info;

    netdev_get_ip_info(netdev_get_active(), &ip_info);

    if(! ip4addr_aton(((ping_param_t *)arg)->host, &target_ip)) {
        return false;
    }

    if ((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0) {
        return false;
    }

    struct sockaddr name;
    socklen_t namelen = sizeof(struct sockaddr);
    name.sa_len       = sizeof(struct sockaddr);
    name.sa_family    = AF_INET;
    name.sa_data[0] = 0;
    name.sa_data[1] = 0;
    name.sa_data[2] = (uint8_t)(ip_info.ip.addr);
    name.sa_data[3] = (uint8_t)(ip_info.ip.addr >> 8 );
    name.sa_data[4] = (uint8_t)(ip_info.ip.addr >> 16);
    name.sa_data[5] = (uint8_t)(ip_info.ip.addr >> 24);

    lwip_bind(s, &name, namelen);

    lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (ping_send(s, &target_ip) == ERR_OK)
    {
        ping_time = OS_GetTicks();
        Log_i("ping send: %s success.", ipaddr_ntoa(&target_ip));
        if (ping_recv(s))
        {
            lwip_close(s);
            return true;
        }
    }
    else
    {
        Log_i("ping send: %s error.", ipaddr_ntoa(&target_ip));
    }

    lwip_close(s);
    return false;
}

ln_at_err_t ln_at_set_ping_once(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);

    bool is_default = false;
    uint8_t para_index = 1;
    char *ip_str = NULL;
    ip_addr_t dnsaddr;
    ping_param_t ping_param = {0};

    Log_i("ping argc:%d", para_num);

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip_str))
    {
        goto __exit;
    }

    if (is_default || !ip_str)
    {
        goto __exit;
    }

    if (ip4addr_aton(ip_str, &dnsaddr))
    {
        ping_param.host = ip_str;
    }
    else {
        if (dns_gethostbyname((char *)ip_str, &(dnsaddr), NULL, NULL) == ERR_OK)
        {
            char *ptr = ip_ntoa(&dnsaddr);
            ping_param.host = ptr;
        }
        else {
            goto __exit;
        }
    }

    if(NETDEV_LINK_UP != netdev_get_link_state(netdev_get_active()))
    {
        goto __exit;
    }

    if (!ping_try_once(&ping_param))
    {
        goto __exit;
    };

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__exit:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
LN_AT_CMD_REG(PING, NULL, ln_at_set_ping_once, NULL, NULL);

#include "netif/ethernetif.h"
ln_at_err_t ln_tcpip_at_get_cifsr(const char *name)
{
    LN_UNUSED(name);

    tcpip_ip_info_t ip_info;
    char ip[16] = {0};
    uint8_t bssid_hex[BSSID_LEN];
    char *pMode = NULL;

    wifi_mode_t mode = wifi_current_mode_get();
    if (mode == WIFI_MODE_STATION)
    {
        if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
        {
            goto __err;
        }

        if (0 != sysparam_sta_mac_get(bssid_hex))
        {
            goto __err;
        }
    }
    else
    {
        if (0 != netdev_get_ip_info(netdev_get_active(), &ip_info))
        {
            goto __err;
        }

        if (0 != sysparam_softap_mac_get(bssid_hex))
        {
            goto __err;
        }
    }

    if (inet_ntop(AF_INET, &ip_info.ip, ip, sizeof(ip)) == NULL)
    {
        goto __err;
    }

    pMode = mode == WIFI_MODE_STATION ? "STA" : "AP";
    ln_at_printf("+CIFSR:%sIP,\"%s\"\r\n", pMode, ip);
    ln_at_printf("+CIFSR:%sMAC,\"%02x:%02x:%02x:%02x:%02x:%02x\"\r\n", pMode,
                 bssid_hex[0], bssid_hex[1], bssid_hex[2], bssid_hex[3], bssid_hex[4], bssid_hex[5]);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_COMMON;
}
LN_AT_CMD_REG(CIFSR, NULL, NULL, NULL, ln_tcpip_at_get_cifsr);

static char *get_prot_name(ln_transparent_link_mode_t _type)
{
    switch (_type)
    {
    case TRANSPARENT_MODE_TCP:
        return "TCP";
    case TRANSPARENT_MODE_UDP:
        return "UDP";
    case TRANSPARENT_MODE_SSL:
        return "SSL";

    default:
        break;
    }

    return "ERR";
}

ln_at_err_t ln_tcpip_at_get_cipstatus(const char *name)
{
    LN_UNUSED(name);

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    ln_at_printf("STATUS:%d\r\n", p->cStaStat);
    if (p->cStaStat == STA_TCPUDP)
    {
        int i = 0;
        for (i = 0; i < 5; i++)
        {
            ln_netlink_handle_t pLink = &p->tLinkItems[i];
            if (pLink->type == TRANSPARENT_MODE_SSL)
            {
                if (!p->ssl_existed)
                {
                    continue;
                }
            }
            else if (pLink->tcpHandle == NULL)
            {
                continue;
            }

            ln_at_printf("+CIPSTATUS:%d,\"%s\",\"%s\",%d,%d,%d\r\n",
                         pLink->id, get_prot_name((ln_transparent_link_mode_t)pLink->type), pLink->remoteIp, pLink->remotePort, pLink->tcpHandle->port, pLink->tetype);
        }
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPSTATUS, NULL, NULL, NULL, ln_tcpip_at_get_cipstatus);

static bool check_id(int val, int *id, int _max)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    if (p->bCipMux == 0)
    {
        *id = 0;
        return true;
    }

    if (!(val >= 0 && val <= _max))
    {
        ln_at_printf("ID ERROR\r\n");
        return false;
    }

    *id = val;
    return true;
}

ln_at_err_t ln_tcpip_at_cipstart(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
    char* pLikType = NULL;
    char* pIp = NULL;
    int port = 0;
    int keep_alive = 0;
    char *local_ip = NULL;
    int local_port = -1;
    int udp_mode = 0;

    ln_netlink_handle_t pLink = NULL;
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    int id = 0;
    if (p->bCipMux)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
        {
            goto __err;
        }

        if (!check_id(int_param, &id, 4)) {
            Log_e("id error!");
            goto __err;
        }
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pLikType))
    {
        goto __err;
    }
    if (is_default || !pLikType)
    {
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &pIp))
    {
        goto __err;
    }
    if (is_default || !pIp)
    {
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &port))
    {
        goto __err;
    }

    if(!(p->bCipMux))        //if mux = 0 ,para num will add 1
    {
        para_num ++;
    }

    if(strstr(pLikType, "TCP") != NULL)
    {
        if(para_num >= 5)       //keep alive
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &keep_alive))
            {
                goto __err;
            }
            if(keep_alive < 0 || keep_alive > 7200)
            {
                goto __err;
            }
        }

        if(para_num >= 6)       //local ip
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &local_ip))
            {
                goto __err;
            }
            /* to do*/
        }
    }
    else if(strstr(pLikType, "UDP") != NULL)
    {
        if(para_num >= 5)
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &local_port))
            {
                goto __err;
            }
            if(local_port < 0)
            {
                goto __err;
            }
        }
        if(para_num >= 6)
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &udp_mode))
            {
                goto __err;
            }
            if(udp_mode < 0 || udp_mode > 2)
            {
                goto __err;
            }
        }
        if(para_num >= 7)
        {
            if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &local_ip))
            {
                goto __err;
            }
        }
    }


    pLink = &(p->tLinkItems[id]);
    pLink->id = id;

    if (pLink->tcpHandle != NULL)
    {
        //ln_at_printf("ALREADY CONNECTED\r\n");
        goto __err;
    }

    if (strstr(pLikType, "TCP") != NULL)
        pLink->type = TRANSPARENT_MODE_TCP;
    else if (strstr(pLikType, "UDP") != NULL)
    {
        pLink->type = TRANSPARENT_MODE_UDP;
        pLink->localPort = local_port;      //add local port
    }
    else if (strstr(pLikType, "SSL") != NULL) {
        pLink->type = TRANSPARENT_MODE_SSL;
        if (p->ssl_existed)
            goto __err;
    }

    strcpy(pLink->remoteIp, pIp);
    pLink->remotePort = port;
		pLink->tetype = 0;
    Log_i("%d, %s, %d", pLink->type, pLink->remoteIp, pLink->remotePort);

    if (0 != p->_start_link(pLink->id)) {
        ln_at_printf("CLOSED\r\n");
        goto __err;
    }

    if (p->bCipMux) {
        ln_at_printf("%d,", pLink->id);
    }
    ln_at_printf("CONNECT\r\n");

    p->cStaStat = STA_TCPUDP;

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPSTART, NULL, ln_tcpip_at_cipstart, NULL, NULL);


ln_at_err_t ln_tcpip_at_send_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
    int cnt = 0;

    ln_netlink_handle_t pLink = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    int id = 0;
    if (p->bCipMux)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
        {
            goto __err;
        }

        if (!check_id(int_param, &id, 4)) {
            Log_e("id error!");
            goto __err;
        }
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }
		
		//2023.02.09 CIPSEND add maximum limit 8192
		if(int_param>8192)
		{
			goto __err;
		}
		
    cnt = int_param;

    pLink = &(p->tLinkItems[id]);
    if (pLink->type == TRANSPARENT_MODE_SSL)
    {
        if (!p->ssl_existed)
        {
            ln_at_printf("link is not valid\r\n");
            goto __err;
        }
    }
    else
    {
        if (pLink->tcpHandle == NULL)
        {
            ln_at_printf("link is not valid\r\n");
            goto __err;
        }
    }
    p->curId = id;

    pLink->tx_offset = 0;
    pLink->sendLen = cnt;
    pLink->tx_buff[0] = 0;

    p->bSend = 1;
    ln_at_printf(LN_AT_RET_OK_STR);
    ln_at_printf(">");

    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

ln_at_err_t ln_tcpip_at_send(const char *name)
{
    LN_UNUSED(name);

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
    if(p->bCipMode == 0)		//cipmode = 0
    {
        goto __err;
    }
		
    if (p->bCipMux)
    {
        ln_at_printf("MUX=1\r\n");
        goto __err;
    }

    if (p->tLinkItems[0].tcpHandle == NULL && p->cStaStat != STA_TCPUDP)
    {
        ln_at_printf("link is not valid\r\n");
        goto __err;
    }

    p->bSend = 1;
    p->curId = 0;

    ln_at_printf(LN_AT_RET_OK_STR);
    ln_at_printf(">");

    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPSEND, NULL, ln_tcpip_at_send_set, NULL, ln_tcpip_at_send);

ln_at_err_t ln_tcpip_at_cipmode_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    if (p->bCipMux && int_param == 1)
    {
        ln_at_printf("CIPMUX and CIPSERVER must be 0\r\n");
        goto __err;
    }
    else if (int_param == 1)
    {
        p->bCipMode = 1;
    }
    else if (int_param == 0)
    {
        p->bCipMode = 0;
    }
    else
    {
        goto __err;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipmode_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    ln_at_printf("+CIPMODE:%d\r\n", p->bCipMode);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPMODE, ln_tcpip_at_cipmode_get, ln_tcpip_at_cipmode_set, NULL, NULL);

ln_at_err_t ln_tcpip_at_cipmux_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if (p->cStaStat == STA_TCPUDP)
    {
        ln_at_printf("link is builded\r\n");
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    if (int_param == 0)
    {
        p->bCipMux = 0;
    }
    else if (int_param == 1)
    {
        if (p->bCipMode == 1)
        {
            ln_at_printf("IPMODE must be 0\r\n");
            goto __err;
        }
        p->bCipMux = 1;
    }
    else
    {
        goto __err;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipmux_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    ln_at_printf("+CIPMUX:%d\r\n", p->bCipMux);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPMUX, ln_tcpip_at_cipmux_get, ln_tcpip_at_cipmux_set, NULL, NULL);

ln_at_err_t ln_tcpip_at_cipclose_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
    int id = 0;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if (p->cStaStat == STA_NOAP || p->cStaStat == STA_GOT_IP)
    {
        ln_at_printf(LN_AT_RET_OK_STR);
        return LN_AT_ERR_NONE;
    }

    if (!p->bCipMux)
    {
        ln_at_printf("MUX=0\r\n");
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    if (!check_id(int_param, &id, 4)) {
        Log_e("id error!");
        goto __err;
    }

    if (id == 5)
    {
        for (int i = 0; i < 5; i++)
        {
            if (p->tLinkItems[i].type != TRANSPARENT_NO_LINK)
            {
                p->_exit_link(i);
            }
        }
        p->cStaStat = STA_DISCON;
    }
    else
    {
        if (p->tLinkItems[id].type != TRANSPARENT_NO_LINK)
        {
            p->_exit_link(id);
        }

        p->cStaStat = STA_DISCON;
        for (int i = 0; i < 5; i++)
        {
            if (p->tLinkItems[i].type != TRANSPARENT_NO_LINK)
            {
                p->cStaStat = STA_TCPUDP;
                break;
            }
        }
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

ln_at_err_t ln_tcpip_at_cipclose(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if (p->cStaStat == STA_NOAP || p->cStaStat == STA_GOT_IP || p->cStaStat == STA_DISCON)
    {
        goto __err;
    }

    if (p->bCipMux)
    {
        ln_at_printf("MUX=1\r\n");
        goto __err;
    }

    p->_exit_link(0);
    //at_printf("CLOSED\r\n");
    p->cStaStat = STA_DISCON;

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPCLOSE, NULL, ln_tcpip_at_cipclose_set, NULL, ln_tcpip_at_cipclose);

ln_at_err_t ln_tcpip_at_ciprecvmode_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    if (int_param == 1)
    {
        p->bCipRecvMode = 1;
    }
    else if (int_param == 0)
    {
        p->bCipRecvMode = 0;
    }
    else
    {
        goto __err;
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_ciprecvmode_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    ln_at_printf("+CIPRECVMODE:%d\r\n", p->bCipRecvMode);
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPRECVMODE, ln_tcpip_at_ciprecvmode_get, ln_tcpip_at_ciprecvmode_set, NULL, NULL);

static ln_at_err_t ln_tcpip_at_ciprecvlen_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    int _len[5] = {0};
    int i = 0;

    for (i = 0; i < 5; i++)
    {
        _len[i] = p->tLinkItems[i].rx_offset;
    }

    if (p->bCipMux == 0)
    {
        ln_at_printf("+CIPRECVLEN:%d\r\n", _len[0]);
    }
    else
    {
        ln_at_printf("+CIPRECVLEN:%d,%d,%d,%d,%d\r\n", _len[0], _len[1], _len[2], _len[3], _len[4]);
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPRECVLEN, ln_tcpip_at_ciprecvlen_get, NULL, NULL, NULL);

ln_at_err_t ln_tcpip_at_ciprecvdata_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    int dtaLen = 0;

    ln_netlink_handle_t pLink = NULL;
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    int id = 0;
    if (p->bCipMux)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
        {
            goto __err;
        }

        if (!check_id(int_param, &id, 4)) {
            Log_e("id error!");
            goto __err;
        }
    }

    pLink = &(p->tLinkItems[id]);
	if (pLink->tcpHandle == NULL)
	{
		goto __err;
	}

    if (pLink->rx_offset == 0)
    {
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    dtaLen = (int_param > pLink->rx_offset) ? pLink->rx_offset : int_param;

    ln_at_printf("+CIPRECVDATA:%d", dtaLen);
    if (p->bCipDinfo)
    {
        ln_at_printf(",\"%s\",%d", pLink->remoteIp, pLink->remotePort);
    }
    ln_at_printf(",");
    ln_at_write((char *)pLink->rx_buff, dtaLen);
    ln_at_printf("\r\n");

    for (int i = dtaLen; i < pLink->rx_offset; i++)
    {
        pLink->rx_buff[i - dtaLen] = pLink->rx_buff[i];
    }
    pLink->rx_offset -= dtaLen;

    ln_at_printf("OK\r\n");

	//ciprecvmode == 1
	if (p->bCipRecvMode)
	{
		if (pLink->rx_offset > 0)
		{
			if (p->bCipMux)
			{
				ln_at_printf("\r\n+IPD,%d,%d\r\n", pLink->id, pLink->rx_offset);
			}
			else
			{
				ln_at_printf("\r\n+IPD,%d\r\n", pLink->rx_offset);
			}			
		} 
		else if (pLink->rx_offset == 0)
		{
			pLink->bCipRcvHasFetched = 1;
			if (pLink->bIsClosed)
			{
				pLink->bIsClosed = false;
				OS_QueueSend(&p->tryCloseQueue, (void*)&pLink->id, 100);
			}
		}
	}

    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

LN_AT_CMD_REG(CIPRECVDATA, NULL, ln_tcpip_at_ciprecvdata_set, NULL, NULL);

ln_at_err_t ln_tcpip_at_cipserver_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;
	
		int server_en = 0;
		int server_port_or_close_client_link = 333;
		char *server_type = NULL;
		int server_mode = 0;
		int ca_en = 0;
		
		ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
		//2023.02.09 add CIPSERVER para2,"type";para3,CA enable
		switch(para_num)
		{
			case 1:
			{
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_en))		//para1 on off
				{					
					goto __err;
				}
			}break;
			
			case 2:
			{
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_en))		//para1 on off
				{
					goto __err;
				}
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_port_or_close_client_link))		//para1 port
				{
					goto __err;
				}
			}break;
			
			case 3:
			{
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_en))		//para1 on off
				{
					goto __err;
				}
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_port_or_close_client_link))		//para2 port
				{
					goto __err;
				}
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &server_type))		//para3 type
				{
					goto __err;
				}				
			}break;
			
			case 4:
			{
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_en))		//para1 on off
				{
					goto __err;
				}
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &server_port_or_close_client_link))		//para2 port
				{
					goto __err;
				}
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &server_type))		//para3 type
				{
					goto __err;
				}				
				if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &ca_en))		//para4 ca_en
				{
					goto __err;
				}								
			}break;
			
			default:
				break;
		}
		
		//para is valid 
		if(server_en != 1 && server_en != 0)
		{
			goto __err;
		}
		else
		{
			if(server_en == 0)
			{
				if(para_num == 1)
				{
					server_port_or_close_client_link = 0;
				}
				else if(para_num == 2)
				{
					if(server_port_or_close_client_link != 1 && server_port_or_close_client_link != 0) 
					{
						goto __err;
					}
				}
			}
		}
		
		if(server_type == NULL)
		{
			//default
			server_mode = 0; //tcp
		}
		else if(strstr(server_type,"TCP") != NULL)
		{
			server_mode = 0; 
		}
		else if(strstr(server_type,"TCPv6") != NULL)
		{
			server_mode = 1; 
		}
		else if(strstr(server_type,"SSL") != NULL)
		{
			server_mode = 2; 
		}
		else if(strstr(server_type,"SSLv6") != NULL)
		{
			server_mode = 3; 
		}
		else
		{
			server_mode = -1;
			goto __err;
		}
		
		if(ca_en != 1 && ca_en != 0)
		{
			goto __err;
		}
		
		p->ServerPort = server_port_or_close_client_link;
		p->Server_Ext_Info.server_port = server_port_or_close_client_link;
		p->Server_Ext_Info.server_mode = server_mode;
		p->Server_Ext_Info.ca_cert = ca_en;
		
		if (LN_OK != ln_get_tcpip_handle()->_start_server(server_en, server_port_or_close_client_link))
		{
				goto __err;
		}


    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipserver_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	
		char mode_str[6]={0};

    if (p->ServerSock == -1)
    {
        ln_at_printf("+CIPSERVER:0\r\n");
    }
    else
    {
				switch(p->Server_Ext_Info.server_mode)
				{
					case 0: strcpy(mode_str,"TCP"); break;
					case 1: strcpy(mode_str,"TCPv6"); break;
					case 2: strcpy(mode_str,"SSL"); break;
					case 3: strcpy(mode_str,"SSLv6"); break;
					default:
						break;
				}
        ln_at_printf("+CIPSERVER:1,%d,\"%s\",%d", p->ServerPort,mode_str,p->Server_Ext_Info.ca_cert);
    }

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPSERVER, ln_tcpip_at_cipserver_get, ln_tcpip_at_cipserver_set, NULL, NULL);


ln_at_err_t ln_tcpip_at_cipservermaxconn_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    //Todo:
    p->ServerMaxConn = int_param;

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipservermaxconn_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    ln_at_printf("+CIPSERVERMAXCONN:%d\r\n", p->ServerMaxConn);

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPSERVERMAXCONN, ln_tcpip_at_cipservermaxconn_get, ln_tcpip_at_cipservermaxconn_set, NULL, NULL);

ln_at_err_t ln_tcpip_at_cipdinfo_set(uint8_t para_num, const char *name)
{
    LN_UNUSED(name);
    Log_i("argc:%d", para_num);

    bool is_default = false;
    uint8_t para_index = 1;
    int int_param = NULL;

    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &int_param))
    {
        goto __err;
    }

    if (int_param != 0 && int_param != 1)
    {
        goto __err;
    }

    p->bCipDinfo = int_param;

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipdinfo_get(const char *name)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    ln_at_printf("+CIPDINFO:%s", p->bCipDinfo ? "true" : "false");

    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}
LN_AT_CMD_REG(CIPDINFO, ln_tcpip_at_cipdinfo_get, ln_tcpip_at_cipdinfo_set, NULL, NULL);


/*********************************************************************************/
/***********************************DNS SET***************************************/
/*********************************************************************************/
dns_ip_t g_dns_ip_cur = {0, "208.67.222.222", "114.114.114.114", "8.8.8.8"};


static ln_at_err_t ln_tcpip_at_cipdns_set(ln_at_cmd_type_e type, uint8_t para_num, const char *name)
{
    bool is_default = false;
    uint8_t para_index = 1;
    int enable = 0;
    char *ip1 = NULL;
    char *ip2 = NULL;
    char *ip3 = NULL;
    dns_ip_t *pdns = &g_dns_ip_cur;

    if(para_num < 1 || para_num > 4)
    {
        goto __err;
    }

    if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_int_param(para_index++, &is_default, &enable))
    {
        goto __err;
    }

    if(para_num >= 2)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip1))
        {
            goto __err;
        }
    }
    if(para_num >= 3)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip2))
        {
            goto __err;
        }
    }
    if(para_num >= 4)
    {
        if(LN_AT_PSR_ERR_NONE != ln_at_parser_get_str_param(para_index++, &is_default, &ip3))
        {
            goto __err;
        }
    }
    

    pdns->enable = (uint8_t)enable;
    if(ip1 != NULL)
    {
        memset(pdns->dns_ip_1, 0, sizeof(pdns->dns_ip_1));
        strcpy(pdns->dns_ip_1, ip1);
    }
    if(ip2 != NULL)
    {
        memset(pdns->dns_ip_2, 0, sizeof(pdns->dns_ip_2));
        strcpy(pdns->dns_ip_2, ip2);
    }
    if(ip3 != NULL)
    {
        memset(pdns->dns_ip_3, 0, sizeof(pdns->dns_ip_3));
        strcpy(pdns->dns_ip_3, ip3);
    }

    if(type == LN_AT_CMD_TYPE_DEF)
    {
        sysparam_tcpip_dns_update(pdns);
    }
    
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;

__err:
    ln_at_printf(LN_AT_RET_ERR_STR);
    return LN_AT_ERR_NONE;
}

static ln_at_err_t ln_tcpip_at_cipdns_get(ln_at_cmd_type_e type, const char *name)
{
    dns_ip_t *pdns = &g_dns_ip_cur;
    
    if(type != LN_AT_CMD_TYPE_DEF)
    {
        ln_at_printf("%s:\"%s\"\r\n", name, pdns->dns_ip_1);
    }
    else
    {
        dns_ip_t dns;
        sysparam_tcpip_dns_get(&dns);
        ln_at_printf("%s:\"%s\"\r\n", name, dns.dns_ip_1);
    }
    
    ln_at_printf(LN_AT_RET_OK_STR);
    return LN_AT_ERR_NONE;
}


static ln_at_err_t ln_tcpip_at_cipdns_set_cur(uint8_t para_num, const char *name)
{
    return ln_tcpip_at_cipdns_set(LN_AT_CMD_TYPE_CUR, para_num, name);
}

static ln_at_err_t ln_tcpip_at_cipdns_set_def(uint8_t para_num, const char *name)
{
    return ln_tcpip_at_cipdns_set(LN_AT_CMD_TYPE_DEF, para_num, name);
}

static ln_at_err_t ln_tcpip_at_cipdns_get_cur(const char *name)
{
    return ln_tcpip_at_cipdns_get(LN_AT_CMD_TYPE_CUR, name);
}

static ln_at_err_t ln_tcpip_at_cipdns_get_def(const char *name)
{
    return ln_tcpip_at_cipdns_get(LN_AT_CMD_TYPE_DEF, name);
}

LN_AT_CMD_REG(CIPDNS, ln_tcpip_at_cipdns_get_cur, ln_tcpip_at_cipdns_set_cur, NULL, NULL);
LN_AT_CMD_REG(CIPDNS_CUR, ln_tcpip_at_cipdns_get_cur, ln_tcpip_at_cipdns_set_cur, NULL, NULL);
LN_AT_CMD_REG(CIPDNS_DEF, ln_tcpip_at_cipdns_get_def, ln_tcpip_at_cipdns_set_def, NULL, NULL);

