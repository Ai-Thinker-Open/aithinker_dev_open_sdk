/*
 * Copyright (C) 2015-2019 Alibaba Group Holding Limited
 */
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include "aiio_adapter_include.h"
#include "aiio_smartliving_demo.h"
#include "aiio_common.h"
#include "utils_hmac.h"

static connect_method_e bootup_connect_method = CONNECT_DIRECT;
static unsigned char direct_connect_count = 0;
static unsigned char is_request_mqtt_host_by_regionid = 0;
static guider_env_e guider_env = GUIDER_ENV_ONLINE;

static char *get_secure_mode_str(secure_mode_e secure_mode)
{
    static char *secure_mode_str = NULL;

    switch (secure_mode)
    {
    case MODE_TLS_GUIDER:
    {
        secure_mode_str = "TLS + Guider";
    }
    break;
    case MODE_TCP_GUIDER_PLAIN:
    {
        secure_mode_str = "TCP + Guider + Plain";
    }
    break;
    case MODE_TCP_GUIDER_ID2_ENCRYPT:
    {
        secure_mode_str = "TCP + Guider + ID2";
    }
    break;
    case MODE_TLS_DIRECT:
    {
        secure_mode_str = "TLS + Direct";
    }
    break;
    case MODE_TCP_DIRECT_PLAIN:
    {
        secure_mode_str = "TCP + Direct + Plain";
    }
    break;
    case MODE_TCP_DIRECT_ID2_ENCRYPT:
    {
        secure_mode_str = "TCP + Direct + ID2";
    }
    break;
    case MODE_TLS_GUIDER_ID2_ENCRYPT:
    {
        secure_mode_str = "TLS + Guider + ID2";
    }
    break;
    case MODE_TLS_DIRECT_ID2_ENCRYPT:
    {
        secure_mode_str = "TLS + Guider + ID2";
    }
    break;
    case MODE_ITLS_DNS_ID2:
    {
        secure_mode_str = "ITLS + DNS + ID2";
    }
    break;
    default:
        break;
    }

    return secure_mode_str;
}

const char *domain_mqtt_direct[] = {
    "iot-as-mqtt.cn-shanghai.aliyuncs.com",    /* Shanghai */
    "iot-as-mqtt.ap-southeast-1.aliyuncs.com", /* Singapore */
    "iot-as-mqtt.ap-northeast-1.aliyuncs.com", /* Japan */
    "iot-as-mqtt.us-east-1.aliyuncs.com",      /* America East*/
    "iot-as-mqtt.eu-central-1.aliyuncs.com",   /* Germany */
    "iot-as-mqtt.us-west-1.aliyuncs.com",      /* America West*/
};

#define GUIDER_DIRECT_DOMAIN_DAILY "iot-test-daily.iot-as-mqtt.unify.aliyuncs.com"

const char *domain_mqtt_direct_pre[] = {
    "47.111.216.79",    /* Shanghai */
    "100.67.85.156",    /* Singapore: iot-auth-pre.ap-southeast-1.aliyuncs.com */
    "pre.Japan",        /* Japan */
    "pre.America.east", /* America east*/
    "100.67.117.159",   /* Germany */
    "pre.America.west", /* America west*/
};

#define GUIDER_HTTP_AUTH_DAILY "iot-auth.alibaba.net"
#define GUIDER_HTTP_AUTH_PRERELEASE "iot-auth-pre.cn-shanghai.aliyuncs.com"
#define GUIDER_HTTP_AUTH_ONLINE "iot-auth-global.aliyuncs.com"
#define GUIDER_HTTP_AUTH_DYNAMIC_REGSTER_ONLINE "iot-auth.cn-shanghai.aliyuncs.com"

#define GUIDER_DIRECT_DOMAIN_ITLS_PRE "100.67.207.143"
#define GUIDER_DIRECT_DOMAIN_ITLS "itls.cn-shanghai.aliyuncs.com"

static char iotx_domain_custom[GUIDER_DOMAIN_MAX][CUSTOME_DOMAIN_LEN_MAX] = {{0}};

static int guider_get_dynamic_mqtt_url(char *p_mqtt_url, int mqtt_url_buff_len)
{
    int len = mqtt_url_buff_len;

    return AilinkFlashRead(KV_MQTT_URL_KEY, (uint8_t *)p_mqtt_url, &len);
}

guider_env_e iotx_guider_get_env(void)
{
    return guider_env;
}

static connect_method_e get_connect_method(char *p_mqtt_url)
{
    connect_method_e connect_method = CONNECT_PREAUTH;

    if (GUIDER_ENV_DAILY == iotx_guider_get_env())
    {
        return CONNECT_DIRECT;
    }

    if (p_mqtt_url && strlen(p_mqtt_url) > 0)
    {
        connect_method = CONNECT_DIRECT;
    }

#ifdef SUPPORT_ITLS
    return CONNECT_DIRECT;
#else
    return connect_method;
#endif
}

static secure_mode_e guider_get_secure_mode(connect_method_e connect_method)
{
    secure_mode_e secure_mode = MODE_TLS_GUIDER;

    if (CONNECT_PREAUTH == connect_method)
    {
#ifdef SUPPORT_TLS
        secure_mode = MODE_TLS_GUIDER;
#else
        secure_mode = MODE_TCP_GUIDER_PLAIN;
#endif
    }
    else
    {
#ifdef SUPPORT_ITLS
        secure_mode = MODE_ITLS_DNS_ID2;
#else
#ifdef SUPPORT_TLS
        secure_mode = MODE_TLS_DIRECT;
#else
        secure_mode = MODE_TCP_DIRECT_PLAIN;
#endif
#endif
    }

    return secure_mode;
}

static sdk_impl_ctx_t g_sdk_impl_ctx = {0};

sdk_impl_ctx_t *sdk_impl_get_ctx(void)
{
    return &g_sdk_impl_ctx;
}

int iotx_guider_get_region_id(void)
{
    int region_id = -1;
    int len = KV_REGION_ID_VALUE_LEN;
    char region_id_str[KV_REGION_ID_VALUE_LEN] = {0};

    if (0 != AilinkFlashRead(KV_REGION_ID_KEY, (uint8_t *)region_id_str, &len))
    {
        aiio_log_e("kv get region id fail");
        return -1;
    }

    region_id = atoi(region_id_str);
    if (region_id < 0 || region_id > IOTX_CLOUD_REGION_MAX)
    {
        aiio_log_d("invalid regionid:%d", region_id);
        return -1;
    }

    aiio_log_d("region id(%d)", region_id);

    return region_id;
}

static void guider_get_timestamp_str(char *buf, int len)
{
    snprintf(buf, len, "%s", GUIDER_DEFAULT_TS_STR);

    return;
}

int iotx_guider_set_custom_domain(int domain_type, const char *domain)
{
    if ((domain_type >= GUIDER_DOMAIN_MAX) || (domain == NULL))
    {
        return -1;
    }

    int len = strlen(domain);
    if (len >= CUSTOME_DOMAIN_LEN_MAX)
    {
        return -1;
    }

    memset(iotx_domain_custom[domain_type], 0, CUSTOME_DOMAIN_LEN_MAX);
    memcpy(iotx_domain_custom[domain_type], domain, len);

    return 0;
}

int IOT_Ioctl(int option, void *data)
{
    int                 res = -1;
    sdk_impl_ctx_t     *ctx = sdk_impl_get_ctx();

    if (option < 0 || data == NULL) {
        aiio_log_e("Invalid Parameter");
        return -1;
    }

    switch (option) {
        case IOTX_IOCTL_SET_REGION: {
            ctx->domain_type = *(iotx_cloud_region_types_t *)data;

            res = 0;
        }
        break;
        case IOTX_IOCTL_GET_REGION: {
            *(iotx_cloud_region_types_t *)data = ctx->domain_type;

            res = 0;
        }
        break;
        case IOTX_IOCTL_SET_MQTT_DOMAIN: {
            ctx->domain_type = IOTX_CLOUD_REGION_CUSTOM;

            res = iotx_guider_set_custom_domain(GUIDER_DOMAIN_MQTT, (const char *)data);
        }
        break;
        case IOTX_IOCTL_SET_MQTT_PORT: {
            ctx->mqtt_port_num = *(uint16_t *)data;
            res = 0;
        }
        break;
        case IOTX_IOCTL_SET_ENV: {
            ctx->env = *(uint16_t *)data;
            res = 0;
        }
        break;
        case IOTX_IOCTL_SET_HTTP_DOMAIN: {
            ctx->domain_type = IOTX_CLOUD_REGION_CUSTOM;

            res = iotx_guider_set_custom_domain(GUIDER_DOMAIN_HTTP, (const char *)data);
        }
        break;
        case IOTX_IOCTL_SET_DYNAMIC_REGISTER: {
            ctx->dynamic_register = *(int *)data;

            res = 0;
        }
        break;
        case IOTX_IOCTL_GET_DYNAMIC_REGISTER: {
            *(int *)data = ctx->dynamic_register;

            res = 0;
        }
        break;
        default: {
            aiio_log_d("Unknown Ioctl Option");
            res = -1;
        }
        break;
    }

    return res;
}

iotx_cloud_region_types_t iotx_guider_get_region(void)
{
    iotx_cloud_region_types_t region_type;

    IOT_Ioctl(IOTX_IOCTL_GET_REGION, &region_type);

    return region_type;
}

/* return domain of mqtt direct or http auth */
const char *iotx_guider_get_domain(domain_type_t domain_type)
{
    iotx_cloud_region_types_t region_type;

    region_type = iotx_guider_get_region();

    if (IOTX_CLOUD_REGION_CUSTOM == region_type)
    {
        return iotx_domain_custom[domain_type];
    }

    if (GUIDER_DOMAIN_MQTT == domain_type)
    {
        if (GUIDER_ENV_PRERELEASE == iotx_guider_get_env())
        {
            return (char *)domain_mqtt_direct_pre[region_type];
        }
        else
        {
            return (char *)domain_mqtt_direct[region_type];
        }
    }
    else if (GUIDER_DOMAIN_HTTP == domain_type)
    {
        if (GUIDER_ENV_PRERELEASE == iotx_guider_get_env())
        {
            return GUIDER_HTTP_AUTH_PRERELEASE;
        }
        else if (GUIDER_ENV_DAILY == iotx_guider_get_env())
        {
            return GUIDER_HTTP_AUTH_DAILY;
        }
        else
        {
            return GUIDER_HTTP_AUTH_ONLINE;
        }
    }
    else if (GUIDER_DOMAIN_DYNAMIC_REGISTER_HTTP == domain_type)
    {
        if (GUIDER_ENV_PRERELEASE == iotx_guider_get_env())
        {
            return GUIDER_HTTP_AUTH_PRERELEASE;
        }
        else if (GUIDER_ENV_DAILY == iotx_guider_get_env())
        {
            return GUIDER_HTTP_AUTH_DAILY;
        }
        else
        {
            return GUIDER_HTTP_AUTH_DYNAMIC_REGSTER_ONLINE;
        }
    }
    else
    {
        aiio_log_e("domain type err");
        return NULL;
    }
}

static int printf_parting_line(void)
{
    aiio_log_i(".................................\r\n");
    return 0;
}

static void guider_print_dev_guider_info(iotx_device_info_t *dev,
                                         char *partner_id,
                                         char *module_id,
                                         char *guider_url,
                                         int secure_mode,
                                         char *time_stamp,
                                         char *guider_sign)
{
    char ds[11];

    memset(ds, 0, sizeof(ds));
    memcpy(ds, dev->device_secret, sizeof(ds) - 1);

    printf_parting_line();
    aiio_log_i("%5s : %-s\r\n", "PK", dev->product_key);
    aiio_log_i("%5s : %-s\r\n", "DN", dev->device_name);
    aiio_log_i("%5s : %-s\r\n", "DS", ds);
    aiio_log_i("%5s : %-s\r\n", "PID", partner_id);
    aiio_log_i("%5s : %-s\r\n", "MID", module_id);

    if (guider_url && strlen(guider_url) > 0)
    {
        aiio_log_i("%5s : %s\r\n", "URL", guider_url);
    }

    aiio_log_i("%5s : %s\r\n", "SM", get_secure_mode_str(secure_mode));
    aiio_log_i("%5s : %s\r\n", "TS", time_stamp);
#if CONFIG_GUIDER_DUMP_SECRET
    aiio_log_i("%5s : %s\r\n", "Sign", guider_sign);
#endif
    printf_parting_line();

    return;
}

int iotx_guider_clear_dynamic_url(void)
{
    int ret = 0;

    // ret = HAL_Kv_Del(KV_MQTT_URL_KEY);
    // ret |= HAL_Kv_Del(KV_REGION_ID_KEY);

    aiio_log_d("reset dyna url and regionid:%d", ret);

    return ret;
}

int iotx_guider_set_dynamic_mqtt_url(char *p_mqtt_url)
{
    int len = 0;
    int ret = 0;

    if (!p_mqtt_url)
        return -1;

    len = strlen(p_mqtt_url) + 1;
    if (len > GUIDER_DYNAMIC_URL_LEN - 2)
    {
        aiio_log_e("len is err");
        return -1;
    }

    ret = AilinkFlashWrite(KV_MQTT_URL_KEY, (uint8_t *)p_mqtt_url, len);
    aiio_log_d("set dyna mqtt:%s,ret=%d", p_mqtt_url, ret);

    if (0 == ret)
    {
        bootup_connect_method = CONNECT_DIRECT;
    }

    return ret;
}

// static int _http_response(char *response_buf,
//                           const int response_buf_len,
//                           const char *request_string,
//                           const char *url,
//                           const int port_num,
//                           const char *pkey)
// {
//     int ret = -1;
//     httpclient_t httpc;
//     httpclient_data_t httpc_data;

//     memset(&httpc, 0, sizeof(httpclient_t));
//     memset(&httpc_data, 0, sizeof(httpclient_data_t));

//     httpc.header = "Accept: application/json\r\n";

//     httpc_data.response_buf = response_buf;
//     httpc_data.response_buf_len = GUIDER_PREAUTH_RESPONSE_LEN;

//     if (NULL == request_string) //Have valid region id
//     {
//         ret = httpclient_common(&httpc, url, port_num, pkey, HTTPCLIENT_GET,
//                                 CONFIG_GUIDER_AUTH_TIMEOUT, &httpc_data);
//     }
//     else
//     {
//         httpc_data.post_content_type = "application/x-www-form-urlencoded;charset=utf-8";
//         httpc_data.post_buf = (char *)request_string;
//         httpc_data.post_buf_len = strlen(request_string);
//         ret = httpclient_common(&httpc, url, port_num, pkey, HTTPCLIENT_POST,
//                                 CONFIG_GUIDER_AUTH_TIMEOUT, &httpc_data);
//     }

//     httpclient_close(&httpc);

//     return ret;
// }

static int guider_get_host_port(
    const char *guider_addr,
    const char *request_string,
    char *host,
    uint16_t *pport)
{
    int ret = -1;
    int iotx_port = 443;
    int ret_code = 0;
    char port_str[6];
    char *iotx_payload = NULL;
    const char *pvalue;

#ifndef SUPPORT_TLS
    iotx_port = 80;
#endif

#if defined(TEST_OTA_PRE)
    iotx_port = 80;
#endif

    /*
    {
        "code": 200,
        "data": {
            "resources": {
                "mqtt": {
                        "host":"public.iot-as-mqtt.cn-shanghai.aliyuncs.com",
                        "port":1883
                    }
                }
        },
        "message":"success"
    }
    */
    iotx_payload = malloc(GUIDER_PREAUTH_RESPONSE_LEN);
    // LITE_ASSERT(iotx_payload);
    memset(iotx_payload, 0, GUIDER_PREAUTH_RESPONSE_LEN);
//     _http_response(iotx_payload,
//                    GUIDER_PREAUTH_RESPONSE_LEN,
//                    request_string,
//                    guider_addr,
//                    iotx_port,
// #if defined(TEST_OTA_PRE)
//                    NULL
// #else
//                    iotx_ca_get()
// #endif
//     );
    aiio_log_d("MQTT URL:");
    // iotx_facility_json_print(iotx_payload, LOG_INFO_LEVEL, '>');

    // pvalue = LITE_json_value_of("code", iotx_payload, MEM_MAGIC, "sys.preauth");
    // if (!pvalue)
    // {
    //     // dump_http_status(SUB_ERRCODE_HTTP_PREAUTH_TIMEOUT_FAIL, "no code in resp");
    //     goto EXIT;
    // }

    // ret_code = atoi(pvalue);
    // free(pvalue);
    // pvalue = NULL;

    if (200 != ret_code)
    {
        // dump_http_status(SUB_ERRCODE_HTTP_PREAUTH_IDENT_AUTH_FAIL, "ret_code = %d (!= 200), abort!", ret_code);
        goto EXIT;
    }

    // pvalue = LITE_json_value_of("data.resources.mqtt.host", iotx_payload, MEM_MAGIC, "sys.preauth");
    // if (NULL == pvalue)
    // {
    //     // dump_http_status(SUB_ERRCODE_HTTP_PREAUTH_TIMEOUT_FAIL, "no resources.mqtt.host in resp");
    //     goto EXIT;
    // }
    // strcpy(host, pvalue);
    // free(pvalue);
    // pvalue = NULL;

    // pvalue = LITE_json_value_of("data.resources.mqtt.port", iotx_payload, MEM_MAGIC, "sys.preauth");
    // if (NULL == pvalue)
    // {
    //     // dump_http_status(SUB_ERRCODE_HTTP_PREAUTH_TIMEOUT_FAIL, "no resources.mqtt.port in resp");
    //     goto EXIT;
    // }
    // strcpy(port_str, pvalue);
    // free(pvalue);
    // pvalue = NULL;
    *pport = atoi(port_str);

    ret = 0;

EXIT:
    if (iotx_payload)
    {
        free(iotx_payload);
        iotx_payload = NULL;
    }
    // if (pvalue)
    // {
    //     free(pvalue);
    //     pvalue = NULL;
    // }

    return ret;
}

static int guider_set_auth_req_str(char *request_buf, iotx_device_info_t *p_dev, char ts[])
{
    int rc = -1;

    rc = snprintf(request_buf, GUIDER_PREAUTH_REQUEST_LEN,
                      "productKey=%s&"
                      "deviceName=%s&"
                      "clientId=%s&"
                      "timestamp=%s&"
                      "resources=mqtt",
                      p_dev->product_key, p_dev->device_name, p_dev->product_Secret, ts);

    // LITE_ASSERT(rc < GUIDER_PREAUTH_REQUEST_LEN);

    return 0;
}

static int guider_get_conn_info(iotx_conn_info_t *conn, iotx_device_info_t *p_dev,
                                char *p_pid, char *p_mid, char *p_guider_url,
                                secure_mode_e secure_mode)
{
    int gw = 0;
    int ext = 0;
    int len = 0;
    int rc = -1;
    int region_id = -1;
    char *p_request_buf = NULL;
    char timestamp_str[GUIDER_TS_LEN] = {0};
    sdk_impl_ctx_t *ctx = sdk_impl_get_ctx();

    const char *p_domain = NULL;
    uint16_t iotx_conn_port = 1883;

    char *p_iotx_conn_host = NULL;
    char *p_mqtt_url = NULL;

    region_id = iotx_guider_get_region_id();
    guider_get_timestamp_str(timestamp_str, sizeof(timestamp_str));
    p_domain = iotx_guider_get_domain(GUIDER_DOMAIN_HTTP);

    if (-1 != region_id) //Have valid region id
    {
        snprintf(p_guider_url, GUIDER_URL_LEN, GUIDER_PREAUTH_GET_URL_FMT, p_domain, region_id);
    }
    else
    {
        snprintf(p_guider_url, GUIDER_URL_LEN, GUIDER_PREAUTH_URL_FMT, p_domain);

        p_request_buf = malloc(GUIDER_PREAUTH_REQUEST_LEN);
        if (!p_request_buf)
        {
            rc = -1;
            goto EXIT;
        }
        memset(p_request_buf, 0, GUIDER_PREAUTH_REQUEST_LEN);

        guider_set_auth_req_str(p_request_buf, p_dev, timestamp_str);

        aiio_log_d("req_str = '%s'", p_request_buf);
    }

    guider_print_dev_guider_info(p_dev, p_pid, p_mid, p_guider_url, secure_mode,
                                 timestamp_str, "no sign");

    p_iotx_conn_host = malloc(HOST_ADDRESS_LEN);
    if (!p_iotx_conn_host)
    {
        rc = -1;
        goto EXIT;
    }

    if (0 != guider_get_host_port(p_guider_url, p_request_buf,
                                  p_iotx_conn_host, &iotx_conn_port))
    {
        aiio_log_e("Request MQTT URL failed:%d", region_id);
        if (-1 != region_id)
        {
            //clear dynamic url info when can not get mqtt connect info by regionid
            iotx_guider_clear_dynamic_url();
        }
        goto EXIT;
    }

    p_mqtt_url = malloc(GUIDER_URL_LEN);
    if (!p_mqtt_url)
    {
        aiio_log_e("no mem");
        goto EXIT;
    }

    memset(p_mqtt_url, '\0', GUIDER_URL_LEN);
    snprintf(p_mqtt_url, GUIDER_URL_LEN, "%s:%d", p_iotx_conn_host, iotx_conn_port);

    if (0 == iotx_guider_set_dynamic_mqtt_url(p_mqtt_url))
    {
        bootup_connect_method = CONNECT_DIRECT; //reset this flag for do direct connect immediately
        if (-1 != region_id)
        {
            is_request_mqtt_host_by_regionid = 1;
        }
        else
        {
            is_request_mqtt_host_by_regionid = 0;
        }
    }

    if (p_request_buf)
    {
        free(p_request_buf);
    }
    if (p_iotx_conn_host != NULL)
    {
        free(p_iotx_conn_host);
    }
    if (p_mqtt_url != NULL)
    {
        free(p_mqtt_url);
    }

    if (bootup_connect_method == CONNECT_DIRECT)
    {
        return GUIDER_BOOTSTRAP_DONE; //Will do mqtt connect immediately
    }
    else
    {
        return -1;
    }

EXIT:
    if (p_request_buf)
    {
        free(p_request_buf);
    }
    if (p_iotx_conn_host != NULL)
    {
        free(p_iotx_conn_host);
    }
    if (p_mqtt_url != NULL)
    {
        free(p_mqtt_url);
    }

    return -1;
}

int iotx_guider_fill_conn_string(char *dst, int len, const char *fmt, ...)
{
    int rc = -1;
    va_list ap;
    char *ptr = NULL;

    va_start(ap, fmt);
    rc = vsnprintf(dst, len, fmt, ap);
    va_end(ap);
    // LITE_ASSERT(rc <= len);

    ptr = strstr(dst, "||");
    if (ptr)
    {
        *ptr = '\0';
    }

    return 0;
}

static int get_non_itls_host_and_port(sdk_impl_ctx_t *ctx, char *product_key, iotx_conn_info_t *conn, char *p_dynamic_mqtt_url)
{
    int port_num = 0;
    int host_name_len = 0;
    char *p_port = NULL;
    char *p_host = NULL;

    if (GUIDER_ENV_DAILY == iotx_guider_get_env())
    {
        conn->port = 1883;
        conn->host_name = malloc(sizeof(GUIDER_DIRECT_DOMAIN_DAILY));
        if (conn->host_name == NULL)
        {
            return -1;
        }
        iotx_guider_fill_conn_string(conn->host_name, sizeof(GUIDER_DIRECT_DOMAIN_DAILY),
                                     GUIDER_DIRECT_DOMAIN_DAILY);
    }
    else
    {
        if (NULL != ctx)
        {
            port_num = ctx->mqtt_port_num;
            if (0 != port_num)
            {
                conn->port = port_num;
            }
        }

        if (0 == conn->port)
        {
#ifdef SUPPORT_TLS
            conn->port = 443;
#else
            conn->port = 1883;
#endif
        }

        if (p_dynamic_mqtt_url && strlen(p_dynamic_mqtt_url) > 0)
        {
            host_name_len = strlen(p_dynamic_mqtt_url) + 1;
            conn->host_name = malloc(host_name_len);
            if (conn->host_name == NULL)
            {
                return -1;
            }

            p_port = strrchr(p_dynamic_mqtt_url, ':');
            if (p_port)
            {
                port_num = atoi(p_port + 1);
                if (port_num > 0 && port_num < UINT16_MAX)
                {
                    conn->port = port_num;
                }
                //delete chars behind of :
                *(p_dynamic_mqtt_url + (p_port - p_dynamic_mqtt_url)) = '\0';
            }

            p_host = strstr(p_dynamic_mqtt_url, "://");
            if (p_host)
            {
                p_dynamic_mqtt_url = p_host + 3;
            }

            iotx_guider_fill_conn_string(conn->host_name, host_name_len,
                                         "%s", p_dynamic_mqtt_url);
            aiio_log_i("conn->host_name is %s\r\n",conn->host_name);
        }
        else
        {
            if (GUIDER_ENV_PRERELEASE == iotx_guider_get_env())
            {
                conn->port = 80;
                host_name_len = strlen(iotx_guider_get_domain(GUIDER_DOMAIN_MQTT)) + 1;
                conn->host_name = malloc(host_name_len);
                if (conn->host_name == NULL)
                {
                    return -1;
                }

                iotx_guider_fill_conn_string(conn->host_name, host_name_len,
                                             "%s", iotx_guider_get_domain(GUIDER_DOMAIN_MQTT));
            }
            else
            {
                host_name_len = strlen(product_key) + 2 + strlen(iotx_guider_get_domain(GUIDER_DOMAIN_MQTT));
                conn->host_name = malloc(host_name_len);
                if (conn->host_name == NULL)
                {
                    return -1;
                }
                iotx_guider_fill_conn_string(conn->host_name, host_name_len,
                                             "%s.%s", product_key,
                                             iotx_guider_get_domain(GUIDER_DOMAIN_MQTT));
            }
        }
    }

    return 0;
}

char        device_id[DEVICE_ID_LEN + 1];

static int _calc_hmac_signature(char *hmac_sigbuf, const int hmac_buflen,
                                const char *timestamp_str, iotx_device_info_t *p_dev_info, ext_params_e ext_params)
{
    snprintf(device_id, DEVICE_ID_LEN, "%s.%s", p_dev_info->product_key, p_dev_info->device_name);

    int rc = -1;
    char *hmac_source_buf = NULL;

    hmac_source_buf = malloc(GUIDER_SIGN_SOURCE_LEN);
    if (!hmac_source_buf)
    {
        aiio_log_e("no mem");
        return -1;
    }

    memset(hmac_source_buf, 0, GUIDER_SIGN_SOURCE_LEN);

    if (EXT_SMART_ROUTE == ext_params)
    {
        rc = snprintf(hmac_source_buf,
                          GUIDER_SIGN_SOURCE_LEN,
                          "clientId%s"
                          "deviceName%s"
                          "ext%d"
                          "productKey%s"
                          "timestamp%s",
                          device_id,
                          p_dev_info->device_name,
                          ext_params,
                          p_dev_info->product_key,
                          timestamp_str);
    }
    else
    {
        aiio_log_i("p_dev_info->device_id is %s\r\n",device_id);
        rc = snprintf(hmac_source_buf,
                          GUIDER_SIGN_SOURCE_LEN,
                          "clientId%s"
                          "deviceName%s"
                          "productKey%s"
                          "timestamp%s",
                          device_id,
                          p_dev_info->device_name,
                          p_dev_info->product_key,
                          timestamp_str);
    }

    // LITE_ASSERT(rc < GUIDER_SIGN_SOURCE_LEN);

    utils_hmac_sha1(hmac_source_buf, strlen(hmac_source_buf), hmac_sigbuf,
                    p_dev_info->device_secret, strlen(p_dev_info->device_secret));

    if (hmac_source_buf)
    {
        free(hmac_source_buf);
    }
    aiio_log_i("111 hmac_sigbuf is %s\r\n",hmac_sigbuf);
    return 0;
}

static void guider_print_conn_info(iotx_conn_info_t *conn)
{
    int pub_key_len = 0;

    // LITE_ASSERT(conn);
    printf_parting_line();
    aiio_log_i("%10s : %-s\r\n", "Host", conn->host_name);
    aiio_log_i("%10s : %d\r\n", "Port", conn->port);
    aiio_log_i("%10s : %-s\r\n", "User", conn->username);
    aiio_log_i("%10s : %-s\r\n", "PW", conn->password);
    aiio_log_i("%10s : %-s\r\n", "ClientID", conn->client_id);
    if (conn->pub_key)
    {
        pub_key_len = strlen(conn->pub_key);
        if (pub_key_len > 63)
            aiio_log_i("%10s : ('... %.16s ...')\r\n", "CA", conn->pub_key + strlen(conn->pub_key) - 63);
    }

    printf_parting_line();
}

static int direct_get_conn_info(iotx_conn_info_t *conn, iotx_device_info_t *p_dev,
                                char *p_pid, char *p_mid, char *p_guider_url,
                                char *p_guider_sign, secure_mode_e secure_mode, char *p_dynamic_mqtt_url)
{
    int rc = -1;
    int len = 0;

    char *authtype = "";
    char timestamp_str[GUIDER_TS_LEN] = {0};
#ifdef REPORT_UUID_ENABLE
    char uuid[UUID_STRLEN_MAX] = {0};
#endif
    unsigned char token_str[RANDOM_STR_MAX_LEN] = {0};
    bind_token_type_t token_type;

    char reset_and_token[GUIDER_RESET_AND_TOKEN_LEN] = {0};

    sdk_impl_ctx_t *ctx = sdk_impl_get_ctx();
    iotx_vendor_dev_reset_type_t reset_type = IOTX_VENDOR_DEV_RESET_TYPE_INVALID;

    guider_get_timestamp_str(timestamp_str, sizeof(timestamp_str));

#if defined(SUPPORT_ITLS)
    rc = get_itls_host_and_port(p_dev->product_key, conn);
    if (rc < 0)
    {
        aiio_log_e("host info err");
        return -1;
    }
#else
    aiio_log_i("1111 get_non_itls_host_and_port\r\n");
    rc = get_non_itls_host_and_port(ctx, p_dev->product_key, conn, p_dynamic_mqtt_url);
    if (rc < 0)
    {
        aiio_log_e("host info err");
        return -1;
    }
#endif

    _calc_hmac_signature(p_guider_sign, GUIDER_SIGN_LEN, timestamp_str, p_dev, EXT_PLAIN_ROUTE);

    guider_print_dev_guider_info(p_dev, p_pid, p_mid, p_guider_url, secure_mode,
                                 timestamp_str, p_guider_sign);

    len = strlen(p_dev->device_name) + strlen(p_dev->product_key) + 2;
    conn->username = malloc(len);
    if (conn->username == NULL)
    {
        goto FAILED;
    }

    /* fill up username and password */
    iotx_guider_fill_conn_string(conn->username, len,
                                 "%s&%s",
                                 p_dev->device_name,
                                 p_dev->product_key);

    len = GUIDER_SIGN_LEN + 1;
    conn->password = malloc(len);
    if (conn->password == NULL)
    {
        goto FAILED;
    }

    iotx_guider_fill_conn_string(conn->password, len,
                                 "%s", p_guider_sign);

    aiio_log_i("conn->username is %s\r\n",conn->username);
    aiio_log_i("conn->password is %s\r\n",conn->password);


    conn->pub_key = iotx_ca_get();

    conn->client_id = malloc(CLIENT_ID_LEN);
    if (conn->client_id == NULL)
    {
        goto FAILED;
    }

#ifdef SUPPORT_ITLS
    authtype = ",authtype=id2";
#else
    authtype = ",authtype=custom-ilop";
#endif

    awss_check_reset(&reset_type);
    awss_get_token(token_str, RANDOM_STR_MAX_LEN, &token_type);

    aiio_log_i("reset_type is %d\r\n",reset_type);
    aiio_log_i("token_str is %s token_type is %d\r\n",token_str,token_type);

    if ((reset_type != IOTX_VENDOR_DEV_RESET_TYPE_INVALID))
    {
        if (TOKEN_TYPE_NOT_CLOUD == token_type || TOKEN_TYPE_CLOUD == token_type)
        {
            snprintf(reset_and_token, GUIDER_RESET_AND_TOKEN_LEN, ",reset=%d,tokenType=%d,token=%s", reset_type, token_type, token_str);
        }
        else
        {
            snprintf(reset_and_token, GUIDER_RESET_AND_TOKEN_LEN, ",reset=%d", reset_type);
        }
    }
    else
    {
        if (TOKEN_TYPE_NOT_CLOUD == token_type || TOKEN_TYPE_CLOUD == token_type)
        {
            snprintf(reset_and_token, GUIDER_RESET_AND_TOKEN_LEN, ",tokenType=%d,token=%s", token_type, token_str);
        }
    }

    aiio_log_i("reset_and_token is %s\r\n",reset_and_token);


    aiio_log_i("device_id is %s\r\n",device_id);
    iotx_guider_fill_conn_string(conn->client_id, CLIENT_ID_LEN,
                                 "%s"
                                 "|securemode=%d"
                                 "%s"
                                 ",_v=sdk-c-" LINKKIT_VERSION
                                 ",timestamp=%s"
                                 ",signmethod=" SHA_METHOD ",lan=C"
                                 ",pid=%s"
                                 ",mid=%s"
                                 "%s"
                                 ",_fy=%s"
                                 "|",
                                 device_id, secure_mode, reset_and_token, timestamp_str, p_pid,
#if (defined(TG7100CEVB)) // report chip info, dont change please!!!
                                 "TG7100CEVB",
#else
                                 p_mid,
#endif
                                 authtype, LIVING_SDK_VERSION
    );

    aiio_log_i("conn->client_id is %s\r\n",conn->client_id);

    guider_print_conn_info(conn);

    return 0;

FAILED:
    if (conn->username)
    {
        free(conn->username);
    }
    if (conn->password)
    {
        free(conn->password);
    }
    if (conn->client_id)
    {
        free(conn->client_id);
    }

    return -1;
}


int iotx_guider_authenticate(iotx_conn_info_t *conn_info, void (*cb)(void *), void *client)
{
    int ret = -1;
    char *p_pid = NULL;
    char *p_mid = NULL;
    char *p_guider_url = NULL;
    char *p_guider_sign = NULL;
    char *p_dynamic_mqtt_url = NULL;

    secure_mode_e secure_mode;
    connect_method_e connect_method;
    iotx_cloud_region_types_t region_type;

    if (conn_info->init != 0)
    {
        aiio_log_d("conn info inited!");
        return 0;
    }

    p_pid = malloc(PID_STRLEN_MAX);
    if (!p_pid)
    {
        ret = -1;
        goto EXIT;
    }
    HAL_GetPartnerID(p_pid);

    p_mid = malloc(MID_STRLEN_MAX);
    if (!p_mid)
    {
        ret = -1;
        goto EXIT;
    }
    HAL_GetModuleID(p_mid);

    p_guider_sign = malloc(GUIDER_SIGN_LEN);
    if (!p_guider_sign)
    {
        ret = -1;
        goto EXIT;
    }
    memset(p_guider_sign, 0, GUIDER_SIGN_LEN);

    p_dynamic_mqtt_url = malloc(GUIDER_DYNAMIC_URL_LEN);
    if (!p_dynamic_mqtt_url)
    {
        ret = -1;
        goto EXIT;
    }

    memset(p_dynamic_mqtt_url, '\0', GUIDER_DYNAMIC_URL_LEN);

    // if (CONNECT_DIRECT == bootup_connect_method)
    // {
    //     ret = guider_get_dynamic_mqtt_url(p_dynamic_mqtt_url, GUIDER_DYNAMIC_URL_LEN - 1);
    //     aiio_log_i("ret is %d\r\n",ret);
    //     if (ret == 0)
    //     {
    //         memset(p_dynamic_mqtt_url, '\0', GUIDER_DYNAMIC_URL_LEN);
    //     }

    //     if (strlen(p_dynamic_mqtt_url) > 0)
    //     {
    //         direct_connect_count += 1;
    //         aiio_log_d("dyna url:%s count:%d", p_dynamic_mqtt_url, direct_connect_count);
    //         if (direct_connect_count > GUIDER_MAX_DIRECT_CONNECT_COUNT)
    //         {
    //             direct_connect_count = 0;
    //             bootup_connect_method = CONNECT_PREAUTH;
    //             memset(p_dynamic_mqtt_url, '\0', GUIDER_DYNAMIC_URL_LEN); //for do bootstrap
    //             if (is_request_mqtt_host_by_regionid == 1)
    //             {
    //                 iotx_guider_clear_dynamic_url();
    //                 is_request_mqtt_host_by_regionid = 0;
    //             }
    //         }
    //     }
    //     else
    //     {
    //         aiio_log_d("no dyna url in kv");
    //     }
    // }
    strcpy(p_dynamic_mqtt_url,"public.iot-as-mqtt.cn-shanghai.aliyuncs.com:1883");
    connect_method = get_connect_method(p_dynamic_mqtt_url);
    connect_method = 1;
    secure_mode = guider_get_secure_mode(connect_method);
    secure_mode = 2;
    aiio_log_i("connect_method is %d\r\n",connect_method);
    aiio_log_i("secure_mode is %d\r\n",secure_mode);
    // if (CONNECT_PREAUTH == connect_method)
    // {
    //     p_guider_url = malloc(GUIDER_URL_LEN);
    //     if (!p_guider_url)
    //     {
    //         ret = -1;
    //         goto EXIT;
    //     }

    //     ret = guider_get_conn_info(conn_info, &DeviceInfo_info, p_pid, p_mid, p_guider_url, secure_mode);
    // }
    // else
    // {
        ret = direct_get_conn_info(conn_info, &DeviceInfo_info, p_pid, p_mid, NULL, p_guider_sign, secure_mode, p_dynamic_mqtt_url);
    // }

EXIT:
    if (p_pid)
    {
        free(p_pid);
    }
    if (p_mid)
    {
        free(p_mid);
    }
    if (p_guider_sign)
    {
        free(p_guider_sign);
    }
    if (p_guider_url)
    {
        free(p_guider_url);
    }
    if (p_dynamic_mqtt_url)
    {
        free(p_dynamic_mqtt_url);
    }

    if (ret == GUIDER_BOOTSTRAP_DONE)
    {
        // ret = iotx_guider_authenticate(conn_info);
        if ( cb != NULL)
        {
            cb(client);
        }
        return ret;
    }

    if (ret < 0)
    {
        aiio_log_e("guider auth fail");
    }
    else
    {
        conn_info->init = 1;
    }

    return ret;
}

int iotx_guider_set_dynamic_region(int region)
{
    int ret = 0;
    // char *p_dynamic_mqtt_url = NULL;

    if (region == IOTX_CLOUD_REGION_INVALID)
    {
        aiio_log_d("no region from app so clear region in kv");
        // iotx_guider_clear_dynamic_url();
        return 0;
    }

    if (region < 0 || region > IOTX_CLOUD_REGION_MAX)
    {
        aiio_log_e("region:%d err", region);
        return -1;
    }

    aiio_log_d("APP region id:%d", region);

    // ret = guider_set_region_id(region);

    // if (region > IOTX_CLOUD_REGION_USA_WEST)
    // {
    //     ret = HAL_Kv_Del(KV_MQTT_URL_KEY);
    //     sys_warning("del mqtt_url:%d", ret);
    //     return 0;
    // }

    // p_dynamic_mqtt_url = malloc(GUIDER_URL_LEN);
    // if (!p_dynamic_mqtt_url)
    // {
    //     aiio_log_e("no mem");
    //     return -1;
    // }
    // aiio_log_d("%s, %d, url %p\r\n", __FUNCTION__, __LINE__, p_dynamic_mqtt_url);

    // // iotx_guider_get_kv_env(); //update guider_env
    // // HAL_GetProductKey(product_key);
    // memset(p_dynamic_mqtt_url, '\0', GUIDER_URL_LEN);

    // if (GUIDER_ENV_PRERELEASE == iotx_guider_get_env())
    // {
    //     int port = 80;

    //     if (!strcmp("47.111.216.79", domain_mqtt_direct_pre[region]))
    //     {
    //         port = 1883;
    //     }

    //     HAL_Snprintf(p_dynamic_mqtt_url, GUIDER_URL_LEN, "%s:%d", domain_mqtt_direct_pre[region], port);
    // }
    // else
    // {
        snprintf(apinfo.region_mqtturl, GUIDER_URL_LEN, "mqtt://%s.%s", DeviceInfo_info.product_key, domain_mqtt_direct[region]);
    // }

    // ret |= iotx_guider_set_dynamic_mqtt_url(p_dynamic_mqtt_url);
    // if (p_dynamic_mqtt_url)
        aiio_log_i(apinfo.region_mqtturl);

    return 0;
}

