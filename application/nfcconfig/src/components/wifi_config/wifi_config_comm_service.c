#include "wifi_config_comm_service.h"
#include "aiio_errcode.h"
#include "utils_aes.h"
#include "wifi_config.h"
#include "aiio_utils_filling.h"
#include "aiio_local_control.h"

#include "lwip/sockets.h" /* 这个头文件要适配层包含 */

#define SELECT_WAIT_TIME_SECONDS (3) /*seconds*/
#define WAIT_CNT_5MIN_SECONDS (300)  /*5 minutes*/

int sockfd = -1; /* socket num */

udp_socket_client_t my_udp_socket_client;
extern int g_apConnected;
/**
 * @brief NFC配网与APP交互数据AES128解密
 *
 * @param[in]   recv_data           解密前的数据
 * @param[in]   len                 解密前数据长度
 * @param[out]  decrypted_data      解密后的数据
 * @param[out]  out_len             解密后的数据长度
 *
 * @return aiio_ret_t   加密结果返回值
 * @retval AIIO_OK      加密成功
 * @retval AIIO_ERROR   加密失败
 *
 *
 */
CHIP_API aiio_ret_t aiio_decrypt_app_data(uint8_t *recv_data, int len, uint8_t *decrypted_data, int *out_len)
{
    unsigned char key[UTILS_AES_BLOCK_LEN] = "1234567890abcdef";

    unsigned char iv[16];
    int datalen = 1024;
    unsigned int keybits;

    keybits = AES_KEY_BITS_128;
    memset(iv, '0', UTILS_AES_BLOCK_LEN);
    if (AIIO_OK == utils_aes_cbc(recv_data, len, decrypted_data, datalen, UTILS_AES_DECRYPT, key, keybits, iv))
    {
        *out_len = len;
        return AIIO_OK;
    }
    *out_len = 0;
    return AIIO_ERROR;
}

/**
 * @brief NFC配网与APP交互数据AES128加密
 *
 * @param[in] recv_data 加密前的数据
 * @param[in] len       加密前数据长度
 * @param[out] encryption_data  加密后的数据
 * @param[out] out_len          加密后的数据长度
 *
 * @return aiio_ret_t
 */
CHIP_API aiio_ret_t aiio_encryption_app_data(uint8_t *recv_data, int len, uint8_t *encryption_data, int *out_len)
{
    unsigned char key[UTILS_AES_BLOCK_LEN] = "1234567890abcdef";

    unsigned char iv[16];
    unsigned int keybits;

    keybits = AES_KEY_BITS_128;
    memset(iv, '0', UTILS_AES_BLOCK_LEN);
    if (AIIO_OK == utils_aes_cbc(recv_data, len, encryption_data, len, UTILS_AES_ENCRYPT, key, keybits, iv))
    {
        *out_len = len;
        return AIIO_OK;
    }
    *out_len = 0;
    return AIIO_ERROR;
}

/**
 * @brief Send a reply message to the APP
 *
 * @param[in] udp_client
 * @param[in] cmd 命令类型
 * @param[in] massage
 *
 * @return aiio_ret_t
 */
CHIP_API static aiio_ret_t _app_reply_dev_info(udp_socket_client_t *udp_client, WiFiConfigCmd_t cmd, char *massage)
{
    int ret;
    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *reply_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(reply_json, "cmdType", 2);
    cJSON_AddNumberToObject(reply_json, "code", cmd);

    uint8_t mac[6] = {0};
    aiio_wifi_sta_mac_get(mac);
    char *mac_str = aiio_os_malloc(20);
    aiio_sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(reply_json, "message", massage);
    cJSON_AddStringToObject(reply_json, "deviceName", mac_str);
    aiio_os_free(mac_str);
    cJSON_AddStringToObject(reply_json, "protoVersion", SOFTAP_BOARDING_VERSION);

    char *json_str = cJSON_Print(reply_json);
    if (!json_str)
    {
        aiio_log_e("cJSON_PrintPreallocated failed!");
        cJSON_Delete(reply_json);
        // app_send_error_log(peer, CUR_ERR, ERR_APP_CMD, ERR_JSON_PRINT);
        return AIIO_ERROR;
    }

    uint8_t array[1024] = {0};
    strcpy((char *)array, json_str);
    pkcs7_padding((char *)array, sizeof(array), strlen((char *)array));
    uint16_t len = 0;
    uint8_t encryption_data[1024] = {0};

    aiio_encryption_app_data(array, strlen((char *)array), encryption_data, &len);

    /* 写功能 */
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(udp_client->source_address);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(udp_client->port);
    aiio_log_i("port:%d", udp_client->port);
	for(int i=0;i<10;i++){
		/* append msg delimiter */
	    ret = sendto(udp_client->sockfd, (unsigned char *)encryption_data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	    if (ret < 0)
	    {
	        aiio_log_e("Error occurred during sending: errno %d", errno);
	    }
	    aiio_log_d("Report dev info(%d): %s", strlen(json_str), json_str);
		aiio_log_d("\r\n\r\nclient number: %d\r\n\r\n", g_apConnected);
		if(0==g_apConnected){
			//wifi断开后跳出
			aiio_log_d("wifi station disconnect,break");
			break;
		}
		aiio_os_tick_dealy(aiio_os_ms2tick(1000));;
	}

	cJSON_Delete(reply_json);

    return ret;
}

int str_to_hex(char *out_arry, int length, const char *in_string)
{
    for (int i = 0; i < length; i += 2)
    {
        char hex_unit[3];
        strncpy(hex_unit, in_string + i, 2);          // 复制每两个字符为一个单位
        hex_unit[2] = '\0';                           // 添加字符串结束符
        out_arry[i / 2] = strtol(hex_unit, NULL, 16); // 将每个单位转换为对应的整数
    }
    return 0;
}

/**
 * @brief 处理来自APP的配网数据
 *
 * @param udp_client udp客户端句柄
 * @param buf
 * @param len
 *
 * @return int
 */
static int _app_handle_recv_data(udp_socket_client_t *udp_client, char *buf, int len)
{
    int ret = 0;
    cJSON *root = cJSON_Parse(buf);
    if (!root)
    {
        aiio_log_e("Parsing JSON Error: [%s]", cJSON_GetErrorPtr());
        // _app_reply_dev_info(udp_client, -1, "error data");
        return AIIO_ERROR;
    }

    cJSON *cmd_json = cJSON_GetObjectItem(root, "cmdType");
    if (cmd_json == NULL)
    {
        aiio_log_e("Invalid cmd JSON: %s", buf);
        cJSON_Delete(root);
        // _app_reply_dev_info(udp_client, -1, "error data");
        return AIIO_ERROR;
    }
    int cmd = cmd_json->valueint;
    aiio_log_i("cmd:%d", cmd);
    switch (cmd)
    {
    case 1: /* AP配网指令 */
    {
        cJSON *ssid_json = cJSON_GetObjectItem(root, "ssid");
        if (ssid_json)
        {
            aiio_log_d("ssid:%s", ssid_json->valuestring);
            wifi_sta_config_t config = {};
            strcpy(&config.ssid, ssid_json->valuestring);
            cJSON *psw_json = cJSON_GetObjectItem(root, "password");
            cJSON *bssid_json = cJSON_GetObjectItem(root, "bssid");
            if (psw_json)
            {
                strcpy(&config.password, psw_json->valuestring);
                aiio_log_d("password:%s", config.password);
            }
            if (bssid_json)
            {
                str_to_hex(&config.bssid, strlen((char *)config.bssid), bssid_json->valuestring);
                aiio_log_d("bssid:%02x%02x%02x%02x%02x%02x", config.bssid[0], config.bssid[1], config.bssid[2], config.bssid[3], config.bssid[4], config.bssid[5]);
            }
            /*这里要回复APP，收到WiFi数据*/
            _app_reply_dev_info(udp_client, 0, "success recv data");
            aiio_wifistation_connect(config);
        }
        else /* 发送密码错误回去给APP */
        {
            _app_reply_dev_info(udp_client, -1, "error data");
        }
    }
    break;
    }
    return 0;
}

/**
 * @brief Send a reply message to the APP
 *
 * @param[in] udp_client
 * @param[in] cmd 命令类型
 * @param[in] massage
 *
 * @return aiio_ret_t
 */
aiio_ret_t udp_radiate_result(void)
{
    int ret;
    cJSON_Hooks memoryHook;

    memoryHook.malloc_fn = (void *(*)(size_t))aiio_os_malloc;
    memoryHook.free_fn = (void (*)(void *))aiio_os_free;

    cJSON_InitHooks(&memoryHook);

    cJSON *reply_json = cJSON_CreateObject();
    cJSON_AddNumberToObject(reply_json, "cmdType", 3);
    // cJSON_AddStringToObject(reply_json, "bssid", "112233445566");

    uint32_t ip, gw, mask;
    aiio_wifi_sta_ip_get(&ip, &gw, &mask);
    char *ip_str = aiio_os_malloc(32);
    aiio_sprintf(ip_str, "%d.%d.%d.%d", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, (ip >> 24) & 0xff);
    cJSON_AddStringToObject(reply_json, "ip", ip_str);
    aiio_os_free(ip_str);

    uint8_t mac[6] = {0};
    aiio_wifi_sta_mac_get(mac);
    char *mac_str = aiio_os_malloc(20);
    aiio_sprintf(mac_str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    cJSON_AddStringToObject(reply_json, "deviceName", mac_str);
    aiio_os_free(mac_str);

    cJSON_AddNumberToObject(reply_json, "port", 602);
    cJSON_AddStringToObject(reply_json, "protoVersion", SOFTAP_BOARDING_VERSION);

    char *json_str = cJSON_Print(reply_json);
    if (!json_str)
    {
        aiio_log_e("cJSON_PrintPreallocated failed!");
        cJSON_Delete(reply_json);
        // app_send_error_log(peer, CUR_ERR, ERR_APP_CMD, ERR_JSON_PRINT);
        return AIIO_ERROR;
    }

    uint8_t array[1024] = {0};
    strcpy((char *)array, json_str);
    pkcs7_padding((char *)array, sizeof(array), strlen((char *)array));
    uint16_t len = 0;
    uint8_t encryption_data[1024] = {0};

    aiio_encryption_app_data(array, strlen((char *)array), encryption_data, &len);

    /* 写功能 */
    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(my_udp_socket_client.port);
    aiio_log_i("port:%d", my_udp_socket_client.port);
    /* append msg delimiter */
    ret = sendto(my_udp_socket_client.sockfd, (unsigned char *)encryption_data, len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0)
    {
        aiio_log_e("Error occurred during sending: errno %d", errno);
    }

    cJSON_Delete(reply_json);
    aiio_log_d("Report dev info(%d): %s", strlen(json_str), json_str);

    return ret;
}

aiio_ret_t aiio_udp_write(udp_socket_client_t *udp_client, const uint8_t *write_data, uint16_t write_len)
{
    struct sockaddr_in dest_addr;

    dest_addr.sin_addr.s_addr = inet_addr(udp_client->source_address);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(udp_client->port);

    aiio_log_i("udp write data:%s:%d", udp_client->source_address, udp_client->port);

    int ret = sendto(udp_client->sockfd, write_data, write_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (ret < 0)
    {
        aiio_log_e("Error occurred during sending: errno %d", errno);
        return AIIO_ERROR;
    }
    return AIIO_OK;
}

/**
 * @brief 启动UDP客户端配网
 *
 * @return uint32_t
 */
uint32_t aiio_udp_start(void *pvParameters)
{
    uint8_t rx_buffer[512] = {0};
    char addr_str[512] = {0};

    /* connect */
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(602);

    /* socket create */
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sockfd < 0)
    {
        aiio_log_e("socket init fail");
        return AIIO_ERROR;
    }

    // Set timeout
    struct timeval timeout;
    timeout.tv_sec = SELECT_WAIT_TIME_SECONDS;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        aiio_log_e("ubp server bind failed ");
        close(sockfd);
        return AIIO_ERROR;
    }

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    /* stay longer than 5 minutes to handle error log */
    uint32_t server_count = WAIT_CNT_5MIN_SECONDS / SELECT_WAIT_TIME_SECONDS + 2;
    uint8_t dataIn[1024] = {0};
    int datalen = 0;

    while (1)
    {
        server_count--;
        int len = recvfrom(sockfd, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);
        if (len < 0)
        {
            aiio_log_d("recvfrom failed: errno");
            continue;
        }
        else
        {
            // Get the sender's ip address as string
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);
            rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
            aiio_log_d("socke id :%dReceived %d bytes from %s", sockfd, len, addr_str);

            my_udp_socket_client.sockfd = sockfd;
            my_udp_socket_client.source_address = addr_str;
            my_udp_socket_client.port = ntohs(((struct sockaddr_in *)&source_addr)->sin_port);

            aiio_decrypt_app_data(rx_buffer, len, dataIn, &datalen);
            pkcs7_cuttingg((char *)dataIn, datalen);             /* 切除多余数据 */
            if (aiio_wifi_ap_status_get() && (server_count > 0)) /* 打开了AP就是解析配网数据 */
            {
                _app_handle_recv_data(&my_udp_socket_client, (char *)dataIn, datalen);
            }
            if (aiio_wifi_connect_status_get()) /* 连上WIFI了就解析控制数据 */
            {
                aiio_local_control_protocol_analysis(&my_udp_socket_client, dataIn, datalen);
            }
        }
    }
    vTaskDelete(NULL);
    return AIIO_OK;
}
