#include "hello_h1.h"
#include "hello_h2.h"

#include "aiio_adapter_include.h"

static aiio_os_thread_handle_t *aiio_udp_server_thread = NULL;

#define SERVER_PORT 8888
#define BUFFER_SIZE 1024
#define WIFI_SSID                        CONFIG_AXK_WIFI_SSID
#define WIFI_PASSWORD                    CONFIG_AXK_WIFI_PASS

void aiio_udp_server_function(void)
{
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    int recv_len, client_addr_len;

    // 创建UDP socket对象
    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        // exit(EXIT_FAILURE);
    }

    // 绑定服务器IP地址和端口号
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        close(server_socket);
        // exit(EXIT_FAILURE);
    }

    aiio_log_w("UDP服务器已启动，等待客户端发送消息...\n");

    while (1) {
        // 接收客户端发送的消息
        client_addr_len = sizeof(client_addr);
        if ((recv_len = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len)) == -1) {
            perror("recvfrom");
            continue;
        }

        // 显示客户端发送的消息
        aiio_log_w("接收到客户端 %s:%d 发送的消息：%.*s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), recv_len, buffer);

        // 发送响应消息给客户端
        char *response = "Hello, Client!";
        if (sendto(server_socket, response, strlen(response), 0, (struct sockaddr *)&client_addr, sizeof(client_addr)) == -1) {
            perror("sendto");
            continue;
        }
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
        if (aiio_os_thread_create(&aiio_udp_server_thread, "aiio_udp_server_thread", aiio_udp_server_function, 4096, NULL, __AIIO_OS_PRIORITY_DEFAULT) == AIIO_ERROR)
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
