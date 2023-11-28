#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

#define PORT 8888
#define BUFFER_SIZE 1024

#define WIFI_SSID                        CONFIG_AXK_WIFI_SSID
#define WIFI_PASSWORD                    CONFIG_AXK_WIFI_PASS

static aiio_os_thread_handle_t *aiio_tcp_server_thread = NULL;

void aiio_tcp_server_function(void)
{
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;
    char buffer[BUFFER_SIZE];

    // 创建TCP socket对象
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // 设置socket地址重用选项
    int enable = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) == -1)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // 绑定socket到指定的主机和端口号
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // 开始监听客户端连接请求，最多允许1个客户端排队等待连接
    if (listen(server_socket, 1) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    aiio_log_w("服务器已启动，等待客户端连接...\n");

    while (1)
    {
        memset(buffer,0,strlen(buffer));
        // 等待客户端连接
        client_addr_len = sizeof(client_addr);
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) == -1)
        {
            perror("accept");
            continue;
        }

        // 显示客户端连接信息
        aiio_log_w("客户端已连接，地址为：%s\n", inet_ntoa(client_addr.sin_addr));

        // 接收客户端发送的消息
        int recv_len;
        if ((recv_len = recv(client_socket, buffer, BUFFER_SIZE, 0)) == -1)
        {
            perror("recv");
            close(client_socket);
            continue;
        }

        // 处理客户端发送的消息
        aiio_log_w("客户端发送的消息为：%s\n", buffer);

        // 发送响应消息给客户端
        char *response = "Hello, Client!";
        if (send(client_socket, response, strlen(response), 0) == -1)
        {
            perror("send");
            close(client_socket);
            continue;
        }

        // 关闭客户端socket连接
        close(client_socket);
    }

    // 关闭服务器socket连接
    close(server_socket);
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
        if (aiio_os_thread_create(&aiio_tcp_server_thread, "aiio_tcp_server_thread", aiio_tcp_server_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
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
