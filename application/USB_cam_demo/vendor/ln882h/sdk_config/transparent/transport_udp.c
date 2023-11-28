#include "transport_udp.h"

#include <stdlib.h>
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "transport_utils.h"

typedef struct {
    int sock;
	char dest_ip[20];
	int dest_port;
	int timeout_ms;
	
} transport_udp_t;

static int udp_connect(ln_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    struct sockaddr_in remote_ip;
//    struct timeval tv;
    transport_udp_t *udp = ln_transport_get_context_data(t);

    memset(&remote_ip, 0, sizeof(struct sockaddr_in));

    //if stream_host is not ip address, resolve it AF_INET,servername,&serveraddr.sin_addr
    if (inet_pton(AF_INET, host, &remote_ip.sin_addr) != 1) {
        if (resolve_dns(host, &remote_ip) < 0) {
            return -1;
        }
    }

    udp->sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (udp->sock < 0) {
        LOG(LOG_LVL_ERROR, "Error create udp socket");
        return -1;
    }

	strcpy(udp->dest_ip, host);
	udp->dest_port = port;
	udp->timeout_ms = timeout_ms;
	
    return 0;
}

static int udp_write(ln_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    struct sockaddr_in addr = { 0 };
    transport_udp_t *udp = ln_transport_get_context_data(t);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(udp->dest_port);
	addr.sin_addr.s_addr = inet_addr(udp->dest_ip);

    return sendto(udp->sock, buffer, len, 0, (struct sockaddr *)(&addr), sizeof(struct sockaddr_in));
}

static int udp_read(ln_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
	struct sockaddr_in addr = { 0 };
	int addr_len = sizeof(struct sockaddr_in);
    transport_udp_t *udp = ln_transport_get_context_data(t);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(udp->dest_port);
	addr.sin_addr.s_addr = inet_addr(udp->dest_ip);
	
    return recvfrom(udp->sock, buffer, len, 0, (struct sockaddr *)(&addr), (socklen_t*)&addr_len);
}

static int udp_poll_read(ln_transport_handle_t t, int timeout_ms)
{
    return 0;
}

static int udp_poll_write(ln_transport_handle_t t, int timeout_ms)
{
    return 0;
}

static int udp_close(ln_transport_handle_t t)
{
    transport_udp_t *udp = ln_transport_get_context_data(t);
    int ret = -1;
    if (udp->sock >= 0) {
        ret = closesocket(udp->sock);
        udp->sock = -1;
    }
    return ret;
}

static ln_err_t udp_destroy(ln_transport_handle_t t)
{
    transport_udp_t *udp = ln_transport_get_context_data(t);
    ln_transport_close(t);
    OS_Free(udp);
    return 0;
}


ln_transport_handle_t ln_transport_udp_init()
{
	ln_transport_handle_t t = ln_transport_init();
    transport_udp_t *udp = OS_Malloc(sizeof(transport_udp_t));
    LN_TRANSPORT_MEM_CHECK(udp, return NULL);
    udp->sock = -1;
    ln_transport_set_func(t, udp_connect, udp_read, udp_write, udp_close, udp_poll_read, udp_poll_write, udp_destroy);
    ln_transport_set_context_data(t, udp);

	return t;
}

int ln_transport_udp_bind_socket(ln_transport_handle_t t, int sock)
{
	transport_udp_t *tcp = (transport_udp_t *)(t->data);
	tcp->sock = sock;
	
	return 0;
}


