/**
 * tcp loopback demo base on BSD Socket
*/

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#undef  TAG
#define TAG "tcp.client"

#define USE_LN_PLATFORM (1)

#if USE_LN_PLATFORM
#include "utils/debug/ln_assert.h"
#include "utils/debug/log.h"

#define DEMO_TCP_LOG_I(fmt, ...)                                  \
    LOG(LOG_LVL_ERROR, "[" ## TAG ## "] " ## fmt ## "\r\n", ##__VA_ARGS__)
#else
#define DEMO_TCP_LOG_I(fmt, ...)                                \
                                do {                            \
                                    printf("["TAG"]");          \
                                    printf(fmt, ##__VA_ARGS__); \
                                } while(0)
#endif /* USE_LN_PLATFORM */

#define DEMO_TCP_SERVER_PORT          (8809)
#define DEMO_TCP_SERVER_ADDR          ("192.168.1.103")
#define DEMO_CLIENT_THREAD_STACK_SIZE (4096)
#define DEMO_CLIENT_THREAD_PRI        (3)

#define DEMO_TCP_DR_ONLY_RX           (1)
#define DEMO_TCP_DR_ONLY_TX           (2)
#define DEMO_TCP_DR_ONLY_ECHO         (DEMO_TCP_DR_ONLY_RX | DEMO_TCP_DR_ONLY_TX) /* default */

static int s_server_port = DEMO_TCP_SERVER_PORT;
static char s_server_addr[128] = {0};

static uint8_t s_tcp_client_dr = DEMO_TCP_DR_ONLY_ECHO;
static char s_client_rx_buffer[1600*6 + 2];
static char s_client_tx_buffer[1600*6 + 2];

static void do_recv_then_send(const int sock)
{
    int len = 0;
    uint32_t last_time = 0;
    uint32_t total_recv_size = 0;
    uint32_t used_time = 0;

    if (DEMO_TCP_DR_ONLY_TX == (s_tcp_client_dr & DEMO_TCP_DR_ONLY_TX) &&
        s_tcp_client_dr != DEMO_TCP_DR_ONLY_ECHO) {
        for (int i = 0; i < sizeof(s_client_tx_buffer); i++) {
            s_client_tx_buffer[i] = i % 256;
        }
    }

    last_time = xTaskGetTickCount();
    while(1)
    {
        if (DEMO_TCP_DR_ONLY_RX == (s_tcp_client_dr & DEMO_TCP_DR_ONLY_RX)) {
            len = recv(sock, s_client_rx_buffer, sizeof(s_client_rx_buffer) - 1, 0);
            if (len < 0) {
                DEMO_TCP_LOG_I("Error occurred during receiving: errno %d", errno);
                break;
            } else if (len == 0) {
                DEMO_TCP_LOG_I("Connection closed");
                break;
            } else {
                // DEMO_TCP_LOG_I("Received %d bytes", len);

                total_recv_size += len;
                used_time = xTaskGetTickCount() - last_time;
                if (used_time > 2000) {
                    DEMO_TCP_LOG_I("rx rate %.2fMbps",
                        (total_recv_size * 8 * 1.0f)/
                        (used_time * 1024.0f * 1024.0f / 1000));
                    total_recv_size = 0;
                    last_time = xTaskGetTickCount();
                }
            }
        }

        if (DEMO_TCP_DR_ONLY_TX == (s_tcp_client_dr & DEMO_TCP_DR_ONLY_TX)) {
            char *send_buf = s_client_tx_buffer;
            int buf_len = sizeof(s_client_tx_buffer);
            int to_write = 0;

            if (s_tcp_client_dr == DEMO_TCP_DR_ONLY_ECHO) {
                send_buf = s_client_rx_buffer;
                buf_len = len;
            }

            to_write = buf_len;
            while (to_write > 0) {
                int written = send(sock, send_buf + (buf_len - to_write), to_write, 0);
                if (written < 0) {
                    DEMO_TCP_LOG_I("Error occurred during sending: errno %d", errno);
                    if (errno == ENOTCONN) {
                        DEMO_TCP_LOG_I("No Connection");
                        return;
                    }
                }
                to_write -= written;
            }
        }
    }
}

static void tcp_client_task(void *arg)
{
    int addr_family = AF_INET;
    int ip_protocol = 0;
    int keep_alive = 1;
    int keep_idle = 60;
    int keep_interval = 3;
    int keep_count = 10;
    int sock = -1;
    struct sockaddr_storage dest_addr;

    memset(&dest_addr, 0, sizeof(struct sockaddr_storage));

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = inet_addr(s_server_addr);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(s_server_port);
        ip_protocol = IPPROTO_IP;
    } else {
        /* not support */
        DEMO_TCP_LOG_I("Not supported family: %d", addr_family);
        vTaskDelete(NULL);
        return;
    }

    sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        DEMO_TCP_LOG_I("Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    {
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(int));
    }

    DEMO_TCP_LOG_I("Socket created");
    DEMO_TCP_LOG_I("Will connect <%s:%d>", );

    if (connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_storage)) < 0) {
        DEMO_TCP_LOG_I("Socket conn failed");
        goto __exit;
    }

    DEMO_TCP_LOG_I("Socket connected! addr:%s, port:%d",
        s_server_addr, s_server_port);

    do_recv_then_send(sock);

__exit:
    close(sock);
    vTaskDelete(NULL);
}

/**
 * @param dr:
 *          1: only rx
 *          2: only tx
 *          3: echo mode, first rx then tx
*/
void demo_tcp_client_main(int port, const char *addr, uint8_t dr)
{
    if (port >= 0) {
        s_server_port = port;
    }

    if (addr != NULL) {
        strncpy(s_server_addr, addr, sizeof(s_server_addr));
    } else {
        strncpy(s_server_addr, DEMO_TCP_SERVER_ADDR, sizeof(s_server_addr));
    }

    if ( 1 <= dr && dr <= 3) {
        s_tcp_client_dr = dr;
    }

    xTaskCreate(tcp_client_task,
                "tcp_client",
                DEMO_CLIENT_THREAD_STACK_SIZE,
                NULL,
                DEMO_CLIENT_THREAD_PRI,
                NULL);
}
