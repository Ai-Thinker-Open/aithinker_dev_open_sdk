#include <stdlib.h>
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "transport_utils.h"
#include "transport.h"

typedef struct {
    int sock;
} transport_tcp_t;

/**
 * @brief      Convert milliseconds to timeval struct
 *
 * @param[in]  timeout_ms  The timeout milliseconds
 * @param[out] tv          Timeval struct
 */
static void ln_transport_utils_ms_to_timeval(int timeout_ms, struct timeval *tv)
{
    tv->tv_sec = timeout_ms / 1000;
    tv->tv_usec = (timeout_ms - (tv->tv_sec * 1000)) * 1000;
}

static int tcp_connect(ln_transport_handle_t t, const char *host, int port, int timeout_ms)
{
    struct sockaddr_in remote_ip;
    struct timeval tv;
    transport_tcp_t *tcp = ln_transport_get_context_data(t);

    memset(&remote_ip, 0, sizeof(struct sockaddr_in));

    //if stream_host is not ip address, resolve it AF_INET,servername,&serveraddr.sin_addr
    if (inet_pton(AF_INET, host, &remote_ip.sin_addr) != 1) {
        if (resolve_dns(host, &remote_ip) < 0) {
            return -1;
        }
    }

    tcp->sock = socket(PF_INET, SOCK_STREAM, 0);

    if (tcp->sock < 0) {
        Log_e("Error create tcp socket");
        return -1;
    }

    remote_ip.sin_family = AF_INET;
    remote_ip.sin_port = htons(port);

    ln_transport_utils_ms_to_timeval(timeout_ms, &tv);

    setsockopt(tcp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    Log_i("[sock=%d],connecting to server IP:%s,Port:%d...",
             tcp->sock, ipaddr_ntoa((const ip_addr_t*)&remote_ip.sin_addr.s_addr), port);
    if (connect(tcp->sock, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr)) != 0) {
        close(tcp->sock);
        tcp->sock = -1;
        return -1;
    }
    return 0;
}

static int tcp_write(ln_transport_handle_t t, const char *buffer, int len, int timeout_ms)
{
    int poll;
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    if ((poll = ln_transport_poll_write(t, timeout_ms)) <= 0) {
        return poll;
    }
    return write(tcp->sock, buffer, len);
}

static int tcp_read(ln_transport_handle_t t, char *buffer, int len, int timeout_ms)
{
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    int poll = -1;
    if ((poll = ln_transport_poll_read(t, timeout_ms)) <= 0) {
        return poll;
    }

	Log_d("tcp_read [sock=%d]\r\n",tcp->sock);
    int read_len = read(tcp->sock, buffer, len);
    if (read_len <= 0) {
        return -1;
    }
    return read_len;
}

static int tcp_poll_read(ln_transport_handle_t t, int timeout_ms)
{
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(tcp->sock, &readset);
    struct timeval timeout;
    ln_transport_utils_ms_to_timeval(timeout_ms, &timeout);
    return select(tcp->sock + 1, &readset, NULL, NULL, &timeout);
}

static int tcp_poll_write(ln_transport_handle_t t, int timeout_ms)
{
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    fd_set writeset;
    FD_ZERO(&writeset);
    FD_SET(tcp->sock, &writeset);
    struct timeval timeout;
    ln_transport_utils_ms_to_timeval(timeout_ms, &timeout);
    return select(tcp->sock + 1, NULL, &writeset, NULL, &timeout);
}

static int tcp_close(ln_transport_handle_t t)
{
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    int ret = -1;
    if (tcp->sock >= 0) {
        ret = close(tcp->sock);
        tcp->sock = -1;
    }
    return ret;
}

static ln_err_t tcp_destroy(ln_transport_handle_t t)
{
    transport_tcp_t *tcp = ln_transport_get_context_data(t);
    ln_transport_close(t);
    OS_Free(tcp);
    return 0;
}

ln_transport_handle_t ln_transport_tcp_init(void)
{
    ln_transport_handle_t t = ln_transport_init();
    transport_tcp_t *tcp = OS_Malloc(sizeof(transport_tcp_t));
    LN_TRANSPORT_MEM_CHECK(tcp, return NULL);
    tcp->sock = -1;
    ln_transport_set_func(t, tcp_connect, tcp_read, tcp_write, tcp_close, tcp_poll_read, tcp_poll_write, tcp_destroy);
    ln_transport_set_context_data(t, tcp);

    return t;
}

int ln_transport_tcp_bind_socket(ln_transport_handle_t t, int sock)
{
	struct timeval tv;
	transport_tcp_t *tcp = (transport_tcp_t *)(t->data);
	tcp->sock = sock;
	ln_transport_utils_ms_to_timeval(5000, &tv);
    setsockopt(tcp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	
	return 0;
}
