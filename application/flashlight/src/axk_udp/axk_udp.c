#include "axk_udp.h"
#include "axk_wifi_init.h"
#include "axk_at_network.h"
#include "axk_mqtt.h"

#include "aiio_os_port.h"
#include "aiio_log.h"
#include "cJSON.h"

#include "lwip/sockets.h"

#define AXK_UDP_PORT            (8887)
#define BUFF_SIZE               (512)

static int32_t axk_udp_init(void);
static int32_t axk_udp_deinit(void);
static int32_t axk_wifi_cfg_udp_sendto(uint8_t *buff, uint16_t len);
static aiio_os_thread_handle_t axk_get_wifi_udp_handle(void);

static aiio_os_thread_handle_t udp_pthread_handle = NULL;
static int server_socket = -1;
static struct sockaddr_in client_addr;
static int client_addr_len;

axk_udp_t g_axk_udp = {
    .init = axk_udp_init,
    .deinit = axk_udp_deinit,
    .wifi_cfg_udp_sendto = axk_wifi_cfg_udp_sendto,
    .get_udp_handle = axk_get_wifi_udp_handle,
};

static int32_t axk_wifi_cfg_udp_sendto(uint8_t *buff, uint16_t len)
{
    int32_t ret = -1;

    ssize_t send_len = sendto(server_socket, buff, len, 0, (struct sockaddr *)&client_addr, client_addr_len);
    if (send_len < 0)
    {
        aiio_log_e("udp sendto failed!!!");
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static void axk_app_cfg_wifi_parse(uint8_t *buff)
{
    int32_t ret = -1;
    cJSON *Root, *key, *host, *topic, *username, *mqtt_pwd;

    Root = cJSON_Parse((const char *)buff);
    if (Root == NULL)
    {
        aiio_log_e("cJSON Parse error!!!");
        goto __exit;
    }

    key = cJSON_GetObjectItem(Root, "cmdId");
    if (key == NULL)
    {
        aiio_log_e("cmdId is null!!!");
        goto __exit;
    }

    int id = key->valueint;

    switch (id)
    {
    case 1:
    {
        axk_wifi_cfg_udp_sendto((uint8_t *)UDP_CFG_WIFI_STEP2, strlen(UDP_CFG_WIFI_STEP2));
    }
    break;
    case 3:
    {
        key = cJSON_GetObjectItem(Root, "ssid");
        if (key == NULL)
        {
            aiio_log_e("ssid is null!!!");
            goto __exit;
        }

        axk_sta_info_t wifi_info = {0};

        if (strlen(key->valuestring) >= sizeof(wifi_info.ssid))
        {
            aiio_log_e("ssid is too long!!!");
            goto __exit;
        }
        strcpy((char *)wifi_info.ssid, key->valuestring);

        key = cJSON_GetObjectItem(Root, "pwd");
        if (key != NULL)
        {
            if (strlen(key->valuestring) < sizeof(wifi_info.pwd))
            {
                strcpy((char *)wifi_info.pwd, key->valuestring);
            }
        }

        host = cJSON_GetObjectItem(Root, "mqtt_host");
        topic = cJSON_GetObjectItem(Root, "mqtt_topic");
        username = cJSON_GetObjectItem(Root, "mqtt_username");
        mqtt_pwd = cJSON_GetObjectItem(Root, "mqtt_password");

        if ((host != NULL) && (topic != NULL) && (username != NULL) && (mqtt_pwd != NULL))
        {
            if (strlen(host->valuestring) > 0 && strlen(topic->valuestring) > 0 &&
                strlen(username->valuestring) > 0 && strlen(mqtt_pwd->valuestring) > 0)
            {
                g_axk_mqtt.modify_uri((uint8_t *)host->valuestring);
                g_axk_mqtt.modify_user_topic((uint8_t *)topic->valuestring);
                g_axk_mqtt.modify_username((uint8_t *)username->valuestring);
                g_axk_mqtt.modify_password((uint8_t *)mqtt_pwd->valuestring);

                // set flag
                g_axk_mqtt.set_info_modify_flag(1);
            }
        }

        g_axk_wifi.set_sta_info((const axk_sta_info_t *)&wifi_info);

        g_axk_network.set_wifi_cfg_type(WIFI_CFG_APP);
        g_axk_wifi.start_connect();
    }
    break;
    default:
        aiio_log_e("unknown cmdId!!!");
        goto __exit;
    }

__exit:
    if (Root != NULL)
    {
        cJSON_Delete(Root);
    }
    return;
}

static void axk_udp_task(void *param)
{
    struct sockaddr_in server_addr;
    uint8_t buffer[BUFF_SIZE];
    int recv_len;

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1)
    {
        aiio_log_e("socket failed!!!");
        goto __exit;
    }

    //绑定服务器IP地址和端口号
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(AXK_UDP_PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        aiio_log_e("bind failed!!!");
        close(server_socket);
        goto __exit;
    }

    client_addr_len = sizeof(client_addr);

    for (;;)
    {
        memset(buffer, 0, BUFF_SIZE);
        recv_len = recvfrom(server_socket, buffer, BUFF_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (recv_len < 0)
        {
            aiio_log_e("recvfrom failed!!!");
            close(server_socket);
            goto __exit;
        }

        aiio_log_i("Received data from %s:%d: %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);

        axk_app_cfg_wifi_parse(buffer);
    }

__exit:
    udp_pthread_handle = NULL;
    aiio_os_event_delete(NULL);
}

static int32_t axk_udp_init(void)
{
    int32_t ret;

    if (udp_pthread_handle != NULL)
    {
        aiio_log_w("udp task have already started");
        ret = 0;
        goto __exit;
    }

    ret = aiio_os_thread_create(&udp_pthread_handle, "udp_t", axk_udp_task, 2048, NULL, 9);
    if (ret != 0)
    {
        aiio_log_e("udp task create failed!!!");
        goto __exit;
    }

    ret = 0;

__exit:
    return ret;
}

static int32_t axk_udp_deinit(void)
{
    if (udp_pthread_handle != NULL)
    {
        aiio_os_thread_delete(udp_pthread_handle);
        udp_pthread_handle = NULL;
    }

    if (server_socket != -1)
    {
        close(server_socket);
        server_socket = -1;
    }

    return 0;
}

static aiio_os_thread_handle_t axk_get_wifi_udp_handle(void)
{
    return udp_pthread_handle;
}

