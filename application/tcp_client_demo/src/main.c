#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define SERVER_IP                        CONFIG_AXK_WIFI_SERVER_IP
#define WIFI_SSID                        CONFIG_AXK_WIFI_SSID
#define WIFI_PASSWORD                    CONFIG_AXK_WIFI_PASS
#define SERVER_PORT 8888
#define BUFFER_SIZE 1024

static aiio_os_thread_handle_t *aiio_tcp_client_thread = NULL;

void aiio_tcp_client_function(void)
{
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // 创建TCP socket对象
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 连接到指定的服务器主机和端口号
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(SERVER_PORT);
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // 发送消息给服务器
    char *message = "Hello, Server!";
    if (send(client_socket, message, strlen(message), 0) == -1)
    {
        perror("send");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // 接收服务器发送的响应消息
    int recv_len;
    if ((recv_len = recv(client_socket, buffer, BUFFER_SIZE, 0)) == -1)
    {
        perror("recv");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // 显示服务器发送的响应消息
    aiio_log_w("服务器发送的响应消息为：%.*s\n", recv_len, buffer);

    // 关闭客户端socket连接
    close(client_socket);
    aiio_os_thread_delete(aiio_tcp_client_thread);
}

static void cb_wifi_event(aiio_input_event_t *event, void *data)
{
    int32_t ret = 0;
    switch (event->code)
    {
    case AIIO_WIFI_EVENT_WIFI_READY:
        aiio_log_w("AIIO_WIFI_EVENT_WIFI_READY");
        break;
    case AIIO_WIFI_EVENT_SCAN_DONE:
        aiio_log_w("AIIO_WIFI_EVENT_SCAN_DONE");
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTING:
        aiio_log_w("AIIO_WIFI_EVENT_STA_CONNECTING");
        break;
    case AIIO_WIFI_EVENT_STA_CONNECTED:
        aiio_log_w("AIIO_WIFI_EVENT_STA_CONNECTED");
        break;
    case AIIO_WIFI_EVENT_STA_DISCONNECTED:
        aiio_log_w("AIIO_WIFI_EVENT_STA_DISCONNECTED");
        break;
    case AIIO_WIFI_EVENT_STA_GOT_IP:
        aiio_log_w("AIIO_WIFI_EVENT_STA_GOT_IP");
        // struct netif *nif_IP;
        // nif_IP = data;
        // aiio_log_d("ip:%d.%d.%d.%d",((nif_IP->ip_addr.addr)>>0)&0xff,((nif_IP->ip_addr.addr)>>8)&0xff,((nif_IP->ip_addr.addr)>>16)&0xff,((nif_IP->ip_addr.addr)>>24)&0xff);
        if (aiio_os_thread_create(&aiio_tcp_client_thread, "aiio_tcp_client_thread", aiio_tcp_client_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
        {
            return;
        }
        break;
    case AIIO_WIFI_EVENT_AP_START:
        aiio_log_w("AIIO_WIFI_EVENT_AP_START");
        break;
    case AIIO_WIFI_EVENT_AP_STOP:
        aiio_log_w("AIIO_WIFI_EVENT_AP_STOP");
        break;
    case AIIO_WIFI_EVENT_AP_STACONNECTED:
        aiio_log_w("AIIO_WIFI_EVENT_AP_STACONNECTED");
        break;
    case AIIO_WIFI_EVENT_AP_STADISCONNECTED:
        aiio_log_w("AIIO_WIFI_EVENT_AP_STADISCONNECTED");
        break;
    default:
        aiio_log_w("AIIO_WIFI_NONE");
        break;
    }
}

void aiio_main(void *params)
{
    int32_t ret = 0;

    aiio_log_a("Hello EasyLogger!");
    aiio_log_e("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_i("Hello EasyLogger!");
    aiio_log_w("Hello EasyLogger!");
    aiio_log_v("Hello EasyLogger!");

    aiio_wifi_register_event_cb(cb_wifi_event);

    ret = aiio_wifi_init();
    if (ret != 0)
    {
        aiio_log_e("wifi init error!!");
    }


    aiio_wifi_set_mode(AIIO_WIFI_MODE_STA);

    aiio_wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            .channel = 0,
            .use_dhcp = 1,
            .flags = 0,
        },
    };

    aiio_wifi_set_config(AIIO_WIFI_IF_STA, &wifi_sta_config);

    aiio_wifi_start();

    while (1)
    {
        aiio_os_tick_dealy(aiio_os_ms2tick(2000));
    }
}
