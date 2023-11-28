#ifndef HTTPS_CLIENT_HTTPS_H
#define HTTPS_CLIENT_HTTPS_H

#include <stdbool.h>
#include "mbedtls/net.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "httpsclient_wrapper.h"
//ota status
#include "usr_ctrl.h"

#define FIELD_URL_HOST_MAX_LEN                   (256)
#define FIELD_URL_PATH_MAX_LEN                   (256 * 2)

#define FIELD_REQUEST_CONTENT_TYPE_MAX_LEN       (256 * 2)
#define FIELD_REQUEST_LOCATION_MAX_LEN           (256)
#define FIELD_REQUEST_REFERRER_MAX_LEN           (256)
#define FIELD_REQUEST_COOKIE_MAX_LEN             (256)
#define FIELD_REQUEST_BOUNDARY_MAX_LEN           (256)

#define FIELD_RESPONSE_CONTENT_TYPE_MAX_LEN      (256)
#define FIELD_RESPONSE_LOCATION_MAX_LEN          (256)
#define FIELD_RESPONSE_REFERRER_MAX_LEN          (256)
#define FIELD_RESPONSE_COOKIE_MAX_LEN            (256)
#define FIELD_RESPONSE_BOUNDARY_MAX_LEN          (256)

#define FIELD_HTTP_RECV_BUF_MAX_LEN              (1024 * 2)
// NOTE: in case the buffer is overflow.
#define HTTP_READ_ONE_TIME_MAX_LEN               (FIELD_HTTP_RECV_BUF_MAX_LEN * 3 / 4)

#define FIELD_HTTP_HEADER_METHOD_MAX_LEN         (8)
#define FIELD_HTTP_HEADER_PORT_MAX_LEN           (8)

// should not be greater than 10.
#define INTERNAL_CHUNKED_PREFIX_STR_LEN          (10)

// FIELD_HTTP_HEADER_TOTAL_MAX_LEN might be large, which includes:
// 1. request line (like "GET / HTTP/1.1")
// 2. request header field
// 3. general header field
// 4.  entity header field
// 5. others
#define FIELD_HTTP_HEADER_TOTAL_MAX_LEN          (1024 * 2)

#define HTTP_PARSE_ERROR        (1 << 0) // 1
// HTTP need to read more data.
#define HTTP_PARSE_READ         (1 << 1) // 2
// HTTP can send data to user app.
#define HTTP_PARSE_WRITE        (1 << 2) // 4
#define HTTP_PARSE_CHUNK        (1 << 3) // 8
#define HTTP_PARSE_END          (1 << 4) // 16

// PARSE Step: parse response head.
#define HTTP_PARSE_HEADER       (1 << 6)
// PARSE Step: parse response body.
#define HTTP_PARSE_BODY         (2 << 7)

typedef enum method_type {
    HTTP_METHOD_INVALID     = 0,
    HTTP_METHOD_GET         = 1,
    HTTP_METHOD_POST        = 2,
#if 0 // NOT Supported by now.
    HTTP_METHOD_PUT         = 3,
    HTTP_METHOD_HEAD        = 4,
    HTTP_METHOD_DELETE      = 5,
    HTTP_METHOD_OPTIONS     = 6,
    HTTP_METHOD_TRACE       = 7,
    HTTP_METHOD_CONNECT     = 8,
#endif
} http_method_type_t;


typedef struct
{
    char        method[FIELD_HTTP_HEADER_METHOD_MAX_LEN];
    uint16_t    status_code;

    char        *content_type;
    uint16_t    content_type_maxlen;

    int32_t    content_length;

    bool        chunked;

    bool        close;

    char        *location;
    uint32_t    location_maxlen;

    char        *referrer;
    uint32_t    referrer_maxlen;

    char        *cookie;
    uint32_t    cookie_maxlen;

    char        *boundary;
    uint32_t    boundary_maxlen;
} HTTP_HEADER;

typedef struct
{
    bool                        verify;
    mbedtls_net_context         ssl_fd;
    mbedtls_entropy_context     entropy;
    mbedtls_ctr_drbg_context    ctr_drbg;
    mbedtls_ssl_context         ssl;
    mbedtls_ssl_config          conf;
    mbedtls_x509_crt            cacert;
} HTTP_SSL;

typedef struct {
    bool        is_https;

    char        *host;
    uint32_t    host_maxlen;

    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN];

    char        *path;
    uint16_t    path_maxlen;
} HTTP_URL;

typedef struct
{
    HTTP_URL    *url;
    HTTP_HEADER *request;
    HTTP_HEADER *response;
    HTTP_SSL    tls;
    bool        isInited;
    int         parse_state;

    char        *parse_ptr;

    char        *recv_buf;
    int32_t        recv_buf_len;
    int32_t        recv_buf_maxlen;

    int32_t        chunk_size;
    int32_t        remain_size;
    char        *body;
    int32_t        body_len;
} HTTP_INFO;

/**
 * @brief An HTTP GET callback.
 *
 * @param recvbuf Buffer for HTTP GET store received data.
 * @param recvbuf_len Length of the buffer.
 * @param total_bytes How many bytes will HTTP GET to fetch.
 * @param is_recv_finished Indicates whether HTTP GET fetch all data.
 * @return int Return how many bytes this callback has done with.
 */
typedef int (*http_get_cb)(const char *recvbuf, int32_t recvbuf_len, int32_t total_bytes, int8_t is_recv_finished);

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

    char* strtoken(char* src, char* dst, int size);

    HTTP_INFO *httpinfo_new(void);
    void httpinfo_delete(HTTP_INFO *hi);

    int  http_init(HTTP_INFO** hi, bool verify);
    int  http_close(HTTP_INFO* hi);
    void http_set_method(HTTP_INFO* hi, http_method_type_t type);

    int  http_get(HTTP_INFO* hi, char* url_str, char* response, int size);
    int  http_get_with_callback(HTTP_INFO* hi, char* url_str, http_get_cb cbFunc);
    int  http_get_ext(HTTP_INFO* hi, char* url_str, http_get_cb cbFunc, const char *usr_header);

    int  http_post(HTTP_INFO* hi, char* url_str, char* data, char* response, int size);
    int http_post_ext(HTTP_INFO *hi, char *url, char *data, http_get_cb cbFunc, const char *usr_header);

    int  http_parse(HTTP_INFO* hi);
    void http_dump_err_msg(void);
    int  http_open(HTTP_INFO* hi, char* url_str);
    int  http_write_header(HTTP_INFO* hi);
    int  http_write(HTTP_INFO* hi, char* data, int len);
    int  http_write_end(HTTP_INFO* hi);
    int  http_read_init(HTTP_INFO* hi);
    int  http_read(HTTP_INFO* hi);

    int parse_url(char *request_url, int *is_https, char *host, char *port, char *filepath);
    int parse_url_ext(char *request_url, int *is_https, char *host, char *port, int *is_default_port, char *filepath);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif //HTTPS_CLIENT_HTTPS_H
