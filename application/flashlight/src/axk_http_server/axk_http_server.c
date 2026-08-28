#include "axk_http_server.h"
#include "aiio_adapter_include.h"
#include "axk_wifi_init.h"
#include "axk_http_server_data_parse.h"
#include "axk_at_network.h"
#include "aiio_http_server.h"
#include "axk_http_server_scan.h"
#include "cJSON.h"

#include "vfs.h"
#include "fs/vfs_romfs.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

static int32_t axk_http_server_init(void);
static int32_t axk_http_server_deinit(void);
static int32_t home_get_handler(aiio_httpd_req_t *req);
static int32_t save_Info_handler(aiio_httpd_req_t *req);
static int32_t get_NetWork_handler(aiio_httpd_req_t *req);
static int32_t get_Status_handler(aiio_httpd_req_t *req);
static int32_t scan_handler(aiio_httpd_req_t *req);
static int32_t mqtt_save_info_handler(aiio_httpd_req_t *req);
static uint8_t axk_http_get_start_flag(void);

static int fd_html = 0;
static romfs_filebuf_t filebuf_cert;
static uint8_t server_start_flag = 0;

axk_http_server_t g_axk_http_server = {
    .server_handle = NULL,
    .init = axk_http_server_init,
    .deinit = axk_http_server_deinit,
    .get_start_flag = axk_http_get_start_flag,
};

static const aiio_httpd_uri_t home = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = home_get_handler,
};

static const aiio_httpd_uri_t saveInfo = {
    .uri = "/saveInfo",
    .method = HTTP_POST,
    .handler = save_Info_handler,
    .user_ctx = NULL,
};

static const aiio_httpd_uri_t getNetWork = {
    .uri = "/getNetWork",
    .method = HTTP_GET,
    .handler = get_NetWork_handler,
    .user_ctx = NULL,
};

static const aiio_httpd_uri_t getStatus = {
    .uri = "/getStatus",
    .method = HTTP_GET,
    .handler = get_Status_handler,
    .user_ctx = NULL,
};

static const aiio_httpd_uri_t scan = {
    .uri = "/scan",
    .method = HTTP_GET,
    .handler = scan_handler,
    .user_ctx = NULL,
};

static const aiio_httpd_uri_t mqtt_info = {
    .uri = "/saveMQTTInfo",
    .method = HTTP_POST,
    .handler = mqtt_save_info_handler,
    .user_ctx = NULL,
};

static void bubbleSort(aiio_wifi_ap_item_t *arr, int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++)
    {
        // 最后的 i 个元素已经排好序，不需要再比较
        for (j = 0; j < n - i - 1; j++)
        {
            // 如果相邻的元素逆序，则交换它们
            if (arr[j].rssi < arr[j + 1].rssi)
            {
                aiio_wifi_ap_item_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/* An HTTP GET handler */
static int32_t home_get_handler(aiio_httpd_req_t *req)
{
    //read flash html info
    if (fd_html == 0)
    {
        fd_html = aos_open("/romfs/index.html", 0);
        if (fd_html < 0)
        {
            aiio_log_e("open html failed, ret:%d", fd_html);
            goto __exit;
        }

        if (aos_ioctl(fd_html, IOCTL_ROMFS_GET_FILEBUF, (long unsigned int)&filebuf_cert) != 0)
        {
            aiio_log_e("aos_ioctl error");
            aos_close(fd_html);
            goto __exit;
        }
    }

    aiio_log_i("read size:%d", filebuf_cert.bufsize);

    aiio_httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, filebuf_cert.buf);

__exit:
    return 0;
}

/* An HTTP POST handler */
static int32_t save_Info_handler(aiio_httpd_req_t *req)
{
    char buf[512];
    char temp_buffer[512];
    int ret = HTTPD_SOCK_ERR_TIMEOUT, remaining = req->content_len;
    memset(temp_buffer, 0, sizeof(temp_buffer));
    memset(buf, 0, sizeof(buf));
    while (remaining > 0)
    {
        char buffer_chunk[300];
        memset(buffer_chunk, 0, sizeof(buffer_chunk));
        /* Read the data for the request */
        if ((ret = aiio_httpd_req_recv(req, buffer_chunk, MIN(remaining, sizeof(buffer_chunk)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }

            return -1;
        }

        /* Send back the same data */
        // httpd_resp_send_chunk(req, buffer_chunk, ret);
        remaining -= ret;
        sprintf(temp_buffer, "%s%s", buf, buffer_chunk);
        memcpy(buf, temp_buffer, sizeof(temp_buffer));
    }

    /* Log data received */
    if (buf != NULL)
    {
        aiio_httpd_resp_send(req, "CONFIG OK", strlen("CONFIG OK"));
        aiio_log_i("receive config data length: %d, data: %s \r\n", req->content_len, buf);
        //data process
        if (g_axk_http_server_data_parse.http_server_wifi_data_parse((const char *)buf) == 0)
        {
            //connect wifi
            // g_axk_wifi.set_mode(AXK_WIFI_STA);
            // g_axk_wifi.init();
            g_axk_network.set_wifi_cfg_type(WIFI_CFG_WEB);
            g_axk_wifi.start_connect();
        }
        return 0;
    }

    aiio_httpd_resp_send_chunk(req, NULL, 0);

    return 0;
}

static int32_t get_NetWork_handler(aiio_httpd_req_t *req)
{
    char *json_str = NULL;

    uint32_t len = g_axk_http_server_scan.get_ap_num();
    extern aiio_wifi_ap_item_t *ap_ary_p;
    if (len == 0 || ap_ary_p == NULL)
    {
        aiio_log_e("ap ap_ary_p is null!!!");
        goto __exit;
    }

    cJSON *p_root = cJSON_CreateObject();
    cJSON *p_arry = cJSON_CreateArray();

    //按rssi排序
    bubbleSort(ap_ary_p, len);

    for (size_t i = 0; i < len; i++)
    {
        cJSON_AddItemToArray(p_arry, cJSON_CreateString(ap_ary_p[i].ssid));
    }

    cJSON_AddNumberToObject(p_root, "length", len);
    cJSON_AddItemToObject(p_root, "data", p_arry);
    // cJSON_AddItemToObject(p_root, "ssid", cJSON_CreateString(device_info_params.ssid));
    json_str = cJSON_Print(p_root);
    aiio_log_i("cached scan json_str: %s, length: %d\n", json_str, sizeof(json_str));
    aiio_httpd_resp_send(req, json_str, strlen(json_str));
    aiio_os_free(json_str);
    cJSON_Delete(p_root);
    // aiio_os_free(ap_ary_p);
    // ap_ary_p = NULL;
    
    return 0;

__exit:
    aiio_httpd_resp_send(req, NULL, 0);
    return 0;
}

static int32_t get_Status_handler(aiio_httpd_req_t *req)
{
    if (g_axk_wifi.got_ip != 0)
    {
        aiio_httpd_resp_send(req, "OK", strlen("OK"));
    }
    else
    {
        aiio_httpd_resp_send(req, "NO", strlen("NO"));
    }

    return 0;
}

static int32_t scan_handler(aiio_httpd_req_t *req)
{
    g_axk_http_server_scan.init();

    /* Respond with empty body */
    aiio_httpd_resp_send(req, NULL, 0);
    return 0;
}

/* An HTTP POST handler */
static int32_t mqtt_save_info_handler(aiio_httpd_req_t *req)
{
    char buf[512];
    char temp_buffer[512];
    int ret = HTTPD_SOCK_ERR_TIMEOUT, remaining = req->content_len;
    memset(temp_buffer, 0, sizeof(temp_buffer));
    memset(buf, 0, sizeof(buf));
    while (remaining > 0)
    {
        char buffer_chunk[300];
        memset(buffer_chunk, 0, sizeof(buffer_chunk));
        /* Read the data for the request */
        if ((ret = aiio_httpd_req_recv(req, buffer_chunk, MIN(remaining, sizeof(buffer_chunk)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* Retry receiving if timeout occurred */
                continue;
            }

            return -1;
        }

        /* Send back the same data */
        // httpd_resp_send_chunk(req, buffer_chunk, ret);
        remaining -= ret;
        sprintf(temp_buffer, "%s%s", buf, buffer_chunk);
        memcpy(buf, temp_buffer, sizeof(temp_buffer));
    }

    /* Log data received */
    if (buf != NULL)
    {
        aiio_httpd_resp_send(req, "MQTT SET INFO DONE", strlen("MQTT SET INFO DONE"));
        aiio_log_i("receive config data length: %d, data: %s \r\n", req->content_len, buf);

        //data process
        g_axk_http_server_data_parse.http_server_mqtt_data_parse((const char *)buf);

        return 0;
    }

    aiio_httpd_resp_send_chunk(req, NULL, 0);

    return 0;
}

static aiio_httpd_handle_t start_webserver(void)
{
    aiio_httpd_handle_t server = NULL;
    aiio_httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.lru_purge_enable = true;

    // Start the httpd server
    aiio_log_i("Starting server on port: '%d'", config.server_port);
    if (aiio_httpd_start(&server, &config) == AIIO_OK) 
    {
        // Set URI handlers
        aiio_log_i("Registering URI handlers");
        aiio_httpd_register_uri_handler(server, &home);
        aiio_httpd_register_uri_handler(server, &saveInfo);
        aiio_httpd_register_uri_handler(server, &getNetWork);
        aiio_httpd_register_uri_handler(server, &getStatus);
        aiio_httpd_register_uri_handler(server, &scan);
        aiio_httpd_register_uri_handler(server, &mqtt_info);

        return server;
    }

    aiio_log_e("Error starting server!");
    return NULL;
}

static void http_server_task(void *param)
{
    (void)(param);

    g_axk_http_server.server_handle = start_webserver();
    if (g_axk_http_server.server_handle != NULL)
    {
        server_start_flag = 1;
    }

    aiio_os_thread_delete(NULL);
}

static int32_t axk_http_server_init(void)
{
    int32_t ret;

    if (server_start_flag == 0)
    {
        ret = aiio_os_thread_create(NULL, "http_server", http_server_task, 2048, NULL, 15);
    }
    else
    {
        aiio_log_i("http server have already start");
        ret = -1;
    }

    return ret;
}

static int32_t axk_http_server_deinit(void)
{
    int32_t ret = -1;

    if (server_start_flag != 1)
    {
        goto __exit;
    }

    ret = aiio_httpd_stop(g_axk_http_server.server_handle);
    if (ret != 0)
    {
        aiio_log_e("httpd stop failed!!!");
        goto __exit;
    }

    server_start_flag = 0;
    g_axk_http_server.server_handle = NULL;

__exit:
    return ret;
}

static uint8_t axk_http_get_start_flag(void)
{
    return server_start_flag;
}

