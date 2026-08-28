#include "axk_ota.h"
#include "axk_device_info.h"
#include "axk_ota_flash.h"
#include "axk_at.h"
#include "axk_at_cmd.h"
#include "axk_sleep.h"
#include "axk_wifi_init.h"

#include "cJSON.h"
#include "mbedtls/md5.h"

#include "aiio_autoconf.h"
#include "aiio_log.h"
#include "aiio_os_port.h"
#include "http_parser.h"

#include <hal_boot2.h>
#include <sys/socket.h>

#define BUF_SIZE        (1024)
#define HEADER_BAK_LEN  (32)

static int32_t axk_ota_mqtt_data_parse(uint8_t *data, uint16_t len);
// static int32_t axk_ota_init(void);
int32_t axk_ota_init(void);

axk_remote_fw_info_t g_remote_fw_info = {
    .url = {"http://aithinker-static.oss-cn-shenzhen.aliyuncs.com/ota/FW_OTA_OR_TEST.bin.xz"},
    .md5 = {"3851032572963C9897325C9AFF606701"},
};
axk_ota_t g_axk_ota = {
    .remote_data_parse = axk_ota_mqtt_data_parse,
};

static uint8_t ota_start_flag = 0;
static uint8_t ota_type = 0;
static ota_parame ota_param;
static char ota_host[128];
static char ota_resource[128];
aiio_os_thread_handle_t ota_thread_handle = NULL;

static int32_t axk_ota_mqtt_data_parse(uint8_t *data, uint16_t len)
{
    int32_t ret = -1;

    cJSON *root = cJSON_Parse((const char *)data);
    if (root == NULL)
    {
        aiio_log_e("Error parsing JSON data.");
        return -1;
    }

    cJSON *ver = cJSON_GetObjectItem(root, "version");
    if (ver == NULL)
    {
        aiio_log_e("missing version key.");
        goto __exit;
    }

    aiio_log_i("remote version: %s", ver->valuestring);
    aiio_log_i("local version: %s", FW_VER);
    if (strncmp(FW_VER, ver->valuestring, strlen(FW_VER)) >= 0)
    {
        aiio_log_i("No upgrade required");
        ret = 1;
        goto __exit;
    }

    cJSON *url = cJSON_GetObjectItem(root, "url");
    if (url == NULL)
    {
        aiio_log_e("missing url key.");
        goto __exit;
    }
    strncpy((char *)g_remote_fw_info.url, url->valuestring, sizeof(g_remote_fw_info.url) - 1);
    aiio_log_i("url: %s", g_remote_fw_info.url);

    cJSON *md5 = cJSON_GetObjectItem(root, "md5");
    if (url == NULL)
    {
        aiio_log_e("missing md5 key.");
        goto __exit;
    }
    strncpy((char *)g_remote_fw_info.md5, md5->valuestring, sizeof(g_remote_fw_info.md5) - 1);
    aiio_log_i("md5: %s", g_remote_fw_info.md5);

    ret = 0;

    axk_ota_init();

__exit:
    cJSON_Delete(root);
    return ret;
}

//将int类型数据转换位字符串
static char *https_itoa(int value)
{
    char *val_str;
    int tmp = value, len = 1;
    while ((tmp /= 10) > 0)
        len++;

    val_str = (char *)aiio_os_malloc(len + 1);
    sprintf(val_str, "%d", value);
    return val_str;
}

static int is_valid_ip_address(const char *ipAddress)
{
    struct sockaddr_in sa;
    int result = inet_pton(AF_INET, ipAddress, &(sa.sin_addr));

    return result != 0;
}

static int connect_server(int server_socket, char *host, int port)
{
    struct sockaddr_in server_addr;
    in_addr_t dst_addr;

    aiio_log_i("host: %s", host);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        aiio_log_e("\n\r[%s] Create socket failed", __FUNCTION__);
        return -1;
    }
    aiio_log_i("[%s] Create socket: %d success!\n", __FUNCTION__, server_socket);

    if (is_valid_ip_address(host))
    {
        dst_addr = inet_addr(host);
    }
    else
    {
        struct hostent *host_info;
        host_info = gethostbyname(host);
        if (!host_info)
        {
            aiio_log_e("[ERROR] Get host ip failed");
            return -1;
        }
        dst_addr = ((struct in_addr *)host_info->h_addr)->s_addr;
        // printf("host addr is %08lX\n", *(uint32_t *)&dst_addr);
    }

    // server_addr.sin_addr.s_addr = inet_addr(host);
    aiio_log_i("gethostbyname ok");

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = dst_addr;
    // memcpy((void *)&server_addr.sin_addr,(void *)server->h_addr,4);

    if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        aiio_log_e("[%s] Socket connect failed", __FUNCTION__);
        return -1;
    }

    aiio_log_i("connect success");

    return server_socket;
}

/*****************************
 * 描述：解析HTTP响应头
 * response：http响应的数据包
 * response_len：http响应数据包的长度
 * result：解析出来的头部信息
 *****************************/
int ai_parse_http_response(uint8_t *response, int response_len, axk_http_response_result *result)
{
    uint32_t i, p, q, m;
    uint32_t header_end = 0;
    if (0 == result->parse_status)
    {
        uint8_t status[4] = {0};
        i = p = q = m = 0;
        for (; i < response_len; ++i)
        {
            if (' ' == response[i])
            {
                ++m;
                if (1 == m)
                {
                    p = i;
                }
                else if (2 == m)
                {
                    q = i;
                    break;
                }
            }
        }

        if (!p || !q || q - p != 4)
        { // Didn't get the status code
            return -1;
        }
        memcpy(status, response + p + 1, 3); // get the status code
        result->status_code = atoi((char const *)status);
        if (result->status_code == 200)
        {
            result->parse_status = 1;
        }
        else
        {
            aiio_log_e("\r\nThe http response status code is %d\r\n", result->status_code);
            return -1;
        }
    }

    // if didn't receive the full http header
    if (3 == result->parse_status)
    { // didn't get the http response
        p = q = 0;
        for (i = 0; i < response_len; ++i)
        {
            if (response[i] == '\r' && response[i + 1] == '\n' &&
                response[i + 2] == '\r' && response[i + 3] == '\n')
            { // the end of header
                header_end = i + 4;
                result->parse_status = 4;
                result->header_len = header_end;
                result->body = response + header_end;
                break;
            }
        }
        if (3 == result->parse_status)
        {
            // Still didn't receive the full header
            result->header_bak = aiio_os_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((const char *)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
    }

    // Get Content-Length
    if (1 == result->parse_status)
    { // didn't get the content length
        const char *content_length_buf1 = "CONTENT-LENGTH";
        const char *content_length_buf2 = "Content-Length";
        const char *content_length_buf3 = "content-length";
        const uint32_t content_length_buf_len = strlen((const char *)content_length_buf1);
        p = q = 0;

        for (i = 0; i < response_len; ++i)
        {
            if (response[i] == '\r' && response[i + 1] == '\n')
            {
                q = i; // the end of the line
                if (!memcmp(response + p, content_length_buf1, content_length_buf_len) ||
                    !memcmp(response + p, content_length_buf2, content_length_buf_len) ||
                    !memcmp(response + p, content_length_buf3, content_length_buf_len))
                { // get the content length
                    unsigned int j1 = p + content_length_buf_len, j2 = q - 1;
                    while (j1 < q && (*(response + j1) == ':' || *(response + j1) == ' '))
                        ++j1;
                    while (j2 > j1 && *(response + j2) == ' ')
                        --j2;
                    uint8_t len_buf[12] = {0};
                    memcpy(len_buf, response + j1, j2 - j1 + 1);
                    result->body_len = atoi((char *)len_buf);
                    result->parse_status = 2;
                }
                p = i + 2;
            }
            if (response[i] == '\r' && response[i + 1] == '\n' &&
                response[i + 2] == '\r' && response[i + 3] == '\n')
            {                       // Get the end of header
                header_end = i + 4; // p is the start of the body
                if (result->parse_status == 2)
                { // get the full header and the content length
                    result->parse_status = 4;
                    result->header_len = header_end;
                    result->body = response + header_end;
                }
                else
                { // there are no content length in header
                    aiio_log_e("\n\r[%s] No Content-Length in header", __FUNCTION__);
                    return -1;
                }
                break;
            }
        }

        if (1 == result->parse_status)
        { // didn't get the content length and the full header
            result->header_bak = aiio_os_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((char *)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
        else if (2 == result->parse_status)
        { // didn't get the full header but get the content length
            result->parse_status = 3;
            result->header_bak = aiio_os_malloc(HEADER_BAK_LEN + 1);
            memset(result->header_bak, 0, strlen((char *)result->header_bak));
            memcpy(result->header_bak, response + response_len - HEADER_BAK_LEN, HEADER_BAK_LEN);
        }
    }
    return result->parse_status;
}

/*************************************************
Function: 		string2hex
Description: 	字符串转换成hex,要求str只能是大写字母ABCDEF和数字
Input: 			str:要转换的字符串
Output: 		hex:转换后的hex字符数组
Return: 		0 成功
                1 不符合规则，失败
*************************************************/
static int string2hex(char* str,char* hex)
{
    int i = 0;
    int j = 0;
    unsigned char temp = 0;
    int str_len = 0;
    char str_cpy[100] = {'0'};
    strcpy(str_cpy,str);
    str_len = strlen(str_cpy);
    if(str_len==0)
    {
        return 1;
    }
    while(i < str_len)
    {
        if(str_cpy[i]>='0' && str_cpy[i]<='F') 
        {
            if((str_cpy[i]>='0' && str_cpy[i]<='9'))
            {
                temp = (str_cpy[i] & 0x0f)<<4;
            }
            else if(str_cpy[i]>='A' && str_cpy[i]<='F')
            {
                temp = ((str_cpy[i] + 0x09) & 0x0f)<<4;
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        }   
        i++;
        if(str_cpy[i]>='0' && str_cpy[i]<='F') 
        {
            if(str_cpy[i]>='0' && str_cpy[i]<='9')
            {
                temp |= (str_cpy[i] & 0x0f);
            }
            else if(str_cpy[i]>='A' && str_cpy[i]<='F')
            {
                temp |= ((str_cpy[i] + 0x09) & 0x0f);
            }
            else
            {
                return 1;
            }
        }
        else
        {
            return 1;
        } 
        i++;
        hex[j] = temp;
        // printf("%02x",temp);
        j++;
    }
    // printf("\n");
    return 0 ;
}

void axk_http_update_ota(void *param)
{
    int fd = -1;
    int ret = -1, len = 0, start_pos = 0;
    int writelen = 0;
    int read_bytes = 0;
    uint32_t idx = 0;
    char *port_str = NULL;
    unsigned char *request = NULL;
    unsigned char recv_buf[BUF_SIZE] = {0};
    axk_http_response_result rsp_result = {0};
    mbedtls_md5_context md5_ctx;
    unsigned char md5[33] = {0};
    char remote_md5[16] = {0};
    // ai_pack_head pack_head_t = {0};

    ota_parame *ota_parame_t = (ota_parame*)aiio_os_malloc(sizeof(ota_parame));
    if(ota_parame_t == NULL) 
    {
        goto exit;
    }
    memcpy(ota_parame_t, (ota_parame *)param, sizeof(ota_parame));
    port_str = https_itoa(ota_parame_t->port);

    mbedtls_md5_init(&md5_ctx);
    mbedtls_md5_starts(&md5_ctx);

    fd = connect_server(fd, ota_parame_t->host, ota_parame_t->port);
    if (fd == -1)
    {
        goto exit;
    }
    aiio_log_i("connect server success");

    request = aiio_os_malloc(strlen("GET ") + strlen(ota_parame_t->resoure) + strlen(" HTTP/1.1\r\nHost: ") + strlen(ota_parame_t->host) + strlen("\r\n\r\n") + 1);
    sprintf((char *)request, "GET %s HTTP/1.1\r\nHost: %s\r\n\r\n", ota_parame_t->resoure, ota_parame_t->host);

    ret = write(fd, request, strlen((char *)request));
    if (ret < 0)
    {
        aiio_log_e("send http requst failed");
        goto exit;
    }

    /* 解析https响应*/
    while (3 >= rsp_result.parse_status)
    {
        if (0 == rsp_result.parse_status)
        { // didn't get the http response
            memset(recv_buf, 0, BUF_SIZE);
            read_bytes = read(fd, recv_buf, BUF_SIZE);
            if (read_bytes <= 0)
            {
                aiio_log_e("read socket failed");
                goto exit;
            }

            aiio_log_i("len = %d", read_bytes);
            aiio_log_i("http head = %s", recv_buf);
            idx = read_bytes;
            memset(&rsp_result, 0, sizeof(rsp_result));
            if (ai_parse_http_response(recv_buf, read_bytes, &rsp_result) == -1)
            {
                goto exit;
            }
        }
        else if ((1 == rsp_result.parse_status) || (3 == rsp_result.parse_status))
        { // just get the status code
            memset(recv_buf, 0, BUF_SIZE);
            memcpy(recv_buf, rsp_result.header_bak, HEADER_BAK_LEN);
            free(rsp_result.header_bak);
            rsp_result.header_bak = NULL;
            read_bytes = read(fd, recv_buf + HEADER_BAK_LEN, (BUF_SIZE - HEADER_BAK_LEN));
            if (read_bytes <= 0)
            {
                aiio_log_e("read socket failed");
                goto exit;
            }
            idx = read_bytes + HEADER_BAK_LEN;
            if (ai_parse_http_response(recv_buf, read_bytes + HEADER_BAK_LEN, &rsp_result) == -1)
            {
                goto exit;
            }
        }
    }

    if (0 == rsp_result.body_len)
    {
        aiio_log_e("New firmware size = 0");
        goto exit;
    }
    else
    {
        aiio_log_i("Download new firmware begin, total size : %d", rsp_result.body_len);
    }

    writelen = idx - rsp_result.header_len;
    aiio_log_i("head_len = %d writelen=%d index = %d", rsp_result.header_len, writelen, idx);
    if (writelen != 0)
    {
        memset(recv_buf, 0, rsp_result.header_len);
        memcpy(recv_buf, recv_buf + rsp_result.header_len, writelen);
        memset(recv_buf + writelen, 0, rsp_result.header_len);
        len = writelen;
    }
    else
    {
        memset(recv_buf, 0, BUF_SIZE);
        read_bytes = recv(fd, recv_buf, BUF_SIZE, 0);
        if (read_bytes <= 0)
        {
            aiio_log_e("recv ota data failed\r\n");
            goto exit;
        }
        len = read_bytes;
    }

    // off固件头部偏移地址
    // int off = parse_ai_pack_head(recv_buf, ret, &pack_head_t);
    // memset(recv_buf, 0, off);
    // len -= off;
    // memcpy(recv_buf, recv_buf + off, len);
    mbedtls_md5_update(&md5_ctx, recv_buf, len);
    // for(int i=0; i<len; i++){
    //     printf("%02X ", recv_buf[i]);
    // }
    // printf("\r\n");

    //擦除flash,4kb对齐
    if (-1 == ota_parame_t->erase_cb(0, (rsp_result.body_len / 4096 + 1) * 4096))
    {
        goto exit;
    }
    // 写flash
    if (-1 == ota_parame_t->write_cb(start_pos, recv_buf, len))
    {
        goto exit;
    }
    int writeTopos = start_pos + len;
    do
    {
        len = sizeof(recv_buf) - 1;
        memset(recv_buf, 0, BUF_SIZE);
        read_bytes = recv(fd, recv_buf, BUF_SIZE, 0);
        if (read_bytes == 0)
            break;
        if (read_bytes < 0)
        {
            aiio_log_e("read socket failed\r\n");
            goto exit;
        }
        len = read_bytes;
        mbedtls_md5_update(&md5_ctx, recv_buf, len);
        ota_parame_t->write_cb(writeTopos, recv_buf, len);
        writeTopos += len;
    } while (writeTopos < rsp_result.body_len);
    aiio_log_i("\r\n\r\nend\r\n\r\n");
    mbedtls_md5_finish(&md5_ctx, md5);

    // aiio_log_i("raw md5: %s", g_remote_fw_info.md5);
    string2hex((char *)g_remote_fw_info.md5, &remote_md5[0]);

    aiio_log_i("mbedtls md5:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
               md5[0], md5[1], md5[2], md5[3], md5[4], md5[5], md5[6], md5[7], md5[8],
               md5[9], md5[10], md5[11], md5[12], md5[13], md5[14], md5[15]);

    aiio_log_i("remote md5:%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
               remote_md5[0], remote_md5[1], remote_md5[2], remote_md5[3], remote_md5[4], remote_md5[5], remote_md5[6], remote_md5[7], remote_md5[8],
               remote_md5[9], remote_md5[10], remote_md5[11], remote_md5[12], remote_md5[13], remote_md5[14], remote_md5[15]);

    if (0 == memcmp(md5, remote_md5, sizeof(remote_md5)))
    {
        // MD5校验成功
        aiio_log_i("MD5 verify success");
        ota_parame_t->set_boot_partition_cb();
    }
    else
    {
        aiio_log_e("MD5 verify failed");
        goto exit;
    }
    axk_at_printf("OTA SUCCESS\r\n");
    aiio_log_i("OTA Success");
    ota_parame_t->rebooot_cb(true);

exit:
#if CONFIG_AXK_AUTO_SLEEP
    if (g_axk_wifi.got_ip == 1)
    {
        axk_sleep_timer_reset();
    }
#endif

    axk_at_printf("OTA FAILED\r\n");
    aiio_log_e("OTA Failed");
    if (fd >= 0)
        close(fd);
    // ota_parame_t->rebooot_cb(false);
    ota_thread_handle = NULL;
    aiio_os_thread_delete(NULL);
}

ota_parame ai_ota_parame_init(char *host, int port, char *resource)
{
    ota_parame ota_parame_t;
    ota_parame_t.host = host;
    ota_parame_t.port = port;
    ota_parame_t.resoure = resource;
    ota_parame_t.write_cb = partition_write_ota_farmware;
    ota_parame_t.erase_cb = partition_erase;
    ota_parame_t.set_boot_partition_cb = set_boot_partition;
    ota_parame_t.rebooot_cb = set_reboot;

    return ota_parame_t;
}

static int32_t axk_user_ota_init(void)
{
    int port;
    char schema[8] = {0};
    struct http_parser_url purl;
    HALPartition_Entry_Config otaEntry;

    if (hal_boot2_get_active_entries(BOOT2_PARTITION_TYPE_FW, &otaEntry))
    {
        aiio_log_e("[OTA] get otaEntry fail");
        return -1;
    }

    http_parser_url_init(&purl);

    int parser_status = http_parser_parse_url((const char *)g_remote_fw_info.url, strlen((const char *)g_remote_fw_info.url), 0, &purl);

    if (parser_status != 0)
    {
        aiio_log_e("[OTA] Error parse url:%s", g_remote_fw_info.url);
        return -1;
    }

    memset(ota_host, 0, sizeof(ota_host));
    memset(ota_resource, 0, sizeof(ota_resource));

    if (purl.field_data[UF_SCHEMA].len > 8)
    {
        aiio_log_e("[OTA] schema ovfl");
        return -1;
    }

    memcpy(schema, g_remote_fw_info.url + purl.field_data[UF_SCHEMA].off, purl.field_data[UF_SCHEMA].len);
    if (strcasecmp(schema, "http") == 0)
    {
        ota_type = 0;
        port = 80;
    }
    else if (strcasecmp(schema, "https") == 0)
    {
        ota_type = 1;
        port = 443;
    }
    else
    {
        aiio_log_e("[OTA] undef schema");
        return -1;
    }

    port = purl.port ? purl.port : port;
    memcpy(ota_host, g_remote_fw_info.url + purl.field_data[UF_HOST].off, purl.field_data[UF_HOST].len);
    memcpy(ota_resource, g_remote_fw_info.url + purl.field_data[UF_PATH].off, purl.field_data[UF_PATH].len);

    aiio_log_i("[OTA] port:%d host:%s path:%s", port, ota_host, ota_resource);

    ota_param = ai_ota_parame_init(ota_host, port, ota_resource);

    return 0;
}

static void axk_ota_task(void *param)
{
    if (axk_user_ota_init() != 0)
    {
        aiio_log_e("axk_user_ota_init failed");
        goto __exit;
    }

    aiio_os_tick_dealy(200);

    axk_http_update_ota(&ota_param);

__exit:
#if CONFIG_AXK_AUTO_SLEEP
    if (g_axk_wifi.got_ip == 1)
    {
        axk_sleep_timer_reset();
    }
#endif
    axk_at_printf("OTA FAILED\r\n");
    ota_thread_handle = NULL;
    aiio_os_thread_delete(NULL);
}

// static int32_t axk_ota_init(void)
int32_t axk_ota_init(void)
{
    int32_t ret = -1;

    if (ota_thread_handle != NULL)
    {
        aiio_log_w("ota task have already start");
        goto __exit;
    }

#if CONFIG_AXK_AUTO_SLEEP
    //停止睡眠定时器
    axk_sleep_timer_stop();
#endif

    axk_at_printf("OTA UPDATING\r\n");

    ret = aiio_os_thread_create(&ota_thread_handle, "axk_ota", axk_ota_task, 4096 * 4, NULL, 10);
    if (ret != 0)
    {
        aiio_log_e("ota task create failed");
        axk_at_printf("OTA FAILED\r\n");
    #if CONFIG_AXK_AUTO_SLEEP
        if (g_axk_wifi.got_ip == 1)
        {
            axk_sleep_timer_reset();
        }
    #endif

        goto __exit;
    }

__exit:
    return ret;
}
