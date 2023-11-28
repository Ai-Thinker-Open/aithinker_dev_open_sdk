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
#define TAG "tcp.server"

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

#define DEMO_TCP_SERVER_PORT        (8899)

#define DEMO_SERVER_THREAD_STACK_SIZE (4096)
#define DEMO_SERVER_THREAD_PRI        (3)

static char s_server_rx_buffer[1600*6 + 2];

static void do_recv_then_send(const int sock)
{
    int len = 0;
    uint32_t last_time = 0;
    uint32_t total_recv_size = 0;
    uint32_t used_time = 0;

    last_time = xTaskGetTickCount();
    do {
        len = recv(sock, s_server_rx_buffer, sizeof(s_server_rx_buffer) - 1, 0);
        if (len < 0) {
            DEMO_TCP_LOG_I("Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            DEMO_TCP_LOG_I("Connection closed");
        } else {
            int to_write = len;

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

            while (to_write > 0) {
                int written = send(sock, s_server_rx_buffer + (len - to_write), to_write, 0);
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
    } while (len > 0);
}

static void tcp_server_task(void *arg)
{
    int err = 0;
    char addr_str[128];
    int addr_family = AF_INET;

    int listen_sock = -1;
    int ip_protocol = 0;
    int keep_alive  = 1;
    int keep_idle   = 60;
    int keep_interval = 3;
    int keep_count    = 10;
    int opt = 0;
    struct sockaddr_storage dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(DEMO_TCP_SERVER_PORT);
        ip_protocol = IPPROTO_IP;
    } else {
        /* not support */
        DEMO_TCP_LOG_I("Not supported family: %d", addr_family);
        vTaskDelete(NULL);
        return;
    }

    listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
        DEMO_TCP_LOG_I("Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    DEMO_TCP_LOG_I("Socket created");

    opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        DEMO_TCP_LOG_I("Socket unable to bind: errno %d", errno);
        DEMO_TCP_LOG_I("IPPROTO: %d", addr_family);
        goto __exit;
    }

    DEMO_TCP_LOG_I("Socket bound, port %d", DEMO_TCP_SERVER_PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        DEMO_TCP_LOG_I("Error occurred during listen: errno %d", errno);
        goto __exit;
    }

    while (1) {
        /* Large enough for both IPv4 or IPv6 */
        struct sockaddr_storage source_addr;
        socklen_t addr_len = sizeof(source_addr);
        int sock;

        DEMO_TCP_LOG_I("Socket listening");

        memset(&source_addr, 0x0, sizeof(source_addr));
        sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            DEMO_TCP_LOG_I("Unable to accept connection: errno %d", errno);
            vTaskDelay(300);
            continue;
        }

        // Set tcp keepalive option
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keep_alive, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keep_idle, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keep_interval, sizeof(int));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keep_count, sizeof(int));

        // Convert ip address to string
        if (source_addr.ss_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr,
                        addr_str,
                        sizeof(addr_str) - 1);
            DEMO_TCP_LOG_I("Socket accepted ip add: %s", addr_str);
        }

        do_recv_then_send(sock);

        shutdown(sock, 0);
        close(sock);
    }

__exit:
    close(listen_sock);
    vTaskDelete(NULL);
}

void demo_tcp_server_main(void)
{
    xTaskCreate(tcp_server_task,
                "tcp_server",
                DEMO_SERVER_THREAD_STACK_SIZE,
                NULL,
                DEMO_SERVER_THREAD_PRI, NULL);
}
