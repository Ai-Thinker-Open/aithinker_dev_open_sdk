#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "httpsclient_wrapper.h"
#include "httpsclient_debug.h"
#include "osal/osal.h"


#if (defined(_WIN32) || defined(_WIN32_WCE)) && !defined(EFIX64) && \
    !defined(EFI32)

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
/* Enables getaddrinfo() & Co */
#define _WIN32_WINNT 0x0501
#include <ws2tcpip.h>

#include <winsock2.h>
#include <windows.h>

#if defined(_MSC_VER)
#if defined(_WIN32_WCE)
#pragma comment( lib, "ws2.lib" )
#else
#pragma comment( lib, "ws2_32.lib" )
#endif
#endif /* _MSC_VER */

#define read(fd,buf,len)        recv(fd,(char*)buf,(int) len,0)
#define write(fd,buf,len)       send(fd,(char*)buf,(int) len,0)
#define close(fd)               closesocket(fd)

static int wsa_init_done = 0;

#elif defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)

    #include "lwip/sockets.h"
    #include "lwip/errno.h"

#else /* ( _WIN32 || _WIN32_WCE ) && !EFIX64 && !EFI32 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>

#endif /* ( _WIN32 || _WIN32_WCE ) && !EFIX64 && !EFI32 */

/* Some MS functions want int and MSVC warns if we pass size_t,
 * but the standard fucntions use socklen_t, so cast only for MSVC */
#if defined(_MSC_VER)
#define MSVC_INT_CAST   (int)
#else
#define MSVC_INT_CAST
#endif

#include "ca_cert_def.h"
#include "httpsclient.h"

static int s_contentTypeIdx = 0;

static char s_contentType[4][48] = {
						"application/x-www-form-urlencoded",
						"application/json",
						"multipart/form-data",
						"text/xml"
};

static int _error;
static int http_parse_header_field(HTTP_INFO *hi, char *param);
static int https_init(HTTP_INFO **hi, bool is_https, bool verify);
static int https_close(HTTP_INFO **hi);
static int https_connect(HTTP_INFO *hi, char *host, char *port);
static int https_write(HTTP_INFO *hi, char *buffer, int len);
static int https_read(HTTP_INFO *hi, char *buffer, int len);

/**
 * @brief Get the first token from @src and copy to @dst.
 * @dst can only contain at most @size bytes.
 *
 * @param src src string.
 * @param dst where to store a token.
 * @param size max bytes of @dst.
 * @return char* start point of next token in @src.
 */
char *strtoken(char *src, char *dst, int size)
{
    char *ptr, *start, *end;
    int  len = 0;

    ptr = src;

    /////////////////////////////////////////////
    ////// remove ' ' and '\t' on left.  ///////
    /////////////////////////////////////////////
    while(TRUE) {
        if((*ptr == '\n') || (*ptr == 0)) return NULL; /* value is not exists */
        if((*ptr != ' ') && (*ptr != '\t')) break;
        ptr++;
    }
    // remember start point.
    start = ptr;

    // remember end point of a token (ternimated by ' ' or '\0' or '\n').
    while(TRUE) {
        end = ptr;
        if(*ptr == ' ') {
            ptr++;
            break;
        }
        if((*ptr == '\n') || (*ptr == 0)) break;
        ptr++;
    }

    /////////////////////////////////////////////
    ////// remove ' ' and '\t' on right.  ///////
    /////////////////////////////////////////////
    while(TRUE) {
        end--;
        if(start == end) break;
        if((*end != ' ') && (*end != '\t')) break;
    }

    len = (int)(end - start + 1);
    if((size > 0) && (len >= size)) len = size - 1;

    strncpy(dst, start, len);
    dst[len]=0;

    return ptr;
}

/**
 * @brief Parse information from @request_url.
 *
 * @param request_url
 * @param is_https A flag to indicate this @request_url is http or is_https.
 * @param host IP address for the server, like "180.101.49.12"
 * @param port Port of server, like "80" or "443" or "6321".
 * @param filepath
 * @return int Return 0 on success, others on failure.
 */
int parse_url(char *request_url, int *is_https, char *host, char *port, char *filepath)
{
    char *p1, *p2;
    char* tempStr = NULL;
    uint16_t tempStrSize = FIELD_URL_HOST_MAX_LEN + FIELD_URL_PATH_MAX_LEN;
    int retCode = 0;

    tempStr = (char *)hc_malloc(tempStrSize);
    if (tempStr == NULL) {
        retCode = -1;
        goto flag_exit;
    }
    memset(tempStr, 0, tempStrSize);

    // NOTE: no prefix like "http://" or "https://" means it's NOT a https connection.
    if(strncmp(request_url, "http://", 7)==0) {
        p1=&request_url[7];
        *is_https = 0;
    } else if(strncmp(request_url, "https://", 8)==0) {
        p1=&request_url[8];
        *is_https = 1;
    } else {
        p1 = &request_url[0];
        *is_https = 0;
    }

    // NOTE: default filepath is "/"
    if((p2=strstr(p1, "/")) == NULL) {
        sprintf(tempStr, "%s", p1);
        sprintf(filepath, "/");
    } else {
        strncpy(tempStr, p1, p2-p1);
        snprintf(filepath, FIELD_URL_PATH_MAX_LEN, "%s", p2);
    }

    if((p1=strstr(tempStr, ":")) != NULL) {
        // find user defined PORT
        *p1=0;
        snprintf(host, FIELD_URL_HOST_MAX_LEN, "%s", tempStr);
        snprintf(port, FIELD_HTTP_HEADER_PORT_MAX_LEN, "%s", p1+1);
    } else {
        // use default PORT
        snprintf(host, FIELD_URL_HOST_MAX_LEN, "%s", tempStr);

        if(*is_https == 0)
            snprintf(port, FIELD_HTTP_HEADER_PORT_MAX_LEN, "80"); // HTTP
        else
            snprintf(port, FIELD_HTTP_HEADER_PORT_MAX_LEN, "443"); // HTTPS
    }

    if (tempStr != NULL) {
        hc_free(tempStr);
        tempStr = NULL;
    }

flag_exit:
    return retCode;
}

/**
 * @brief Parse one line of header, like
 * "HTTP/1.1 200 OK"
 * "Server: Apache/2.4.43 (Win64)"
 * "Content-Length: 1730411"
 *
 * @param hi
 * @param param one line of header.
 * @return int
 */
static int http_parse_header_field(HTTP_INFO *hi, char *param)
{
    char *token;
    char t1[256], t2[256];
    int  len;

    LOG_D("\r\n* header: %s \r\n", param);

    token = param;

    if( (token=strtoken(token, t1, 256)) == NULL ) {
        // first token is not found.
        // this is for the key.
        return -1;
    }

    if(strtoken(token, t2, 256) == NULL) {
        // second token is not found.
        // this is for the value.
        return -1;
    }

    if(strncasecmp(t1, "HTTP", 4) == 0)
    {
        // status code, like 200 or 404.
        hi->response->status_code = atoi(t2);

    } else if(strncasecmp(t1, "set-cookie:", 11) == 0) {
        // get cookie from server.
        len = (int)strlen(t2);
        strncpy(hi->response->cookie, t2, len);
        hi->response->cookie[len] = 0;

    } else if(strncasecmp(t1, "location:", 9) == 0) {
        // get location
        len = (int)strlen(t2);
        strncpy(hi->response->location, t2, len);
        hi->response->location[len] = 0;

    } else if(strncasecmp(t1, "content-type:", 13) == 0) {
        // get Content-Type
        len = (int)strlen(t2);
        strncpy(hi->response->content_type, t2, len);
        hi->response->content_type[len] = 0;

    } else if(strncasecmp(t1, "content-length:", 15) == 0) {
        // get Content-Length
        uint32_t length = atoi(t2);
        hi->response->content_length = length;

    } else if(strncasecmp(t1, "transfer-encoding:", 18) == 0) {
        // check if Chunked is supported by server.
        if(strncasecmp(t2, "chunked", 7) == 0)
        {
            hi->response->chunked = TRUE;
        }

    } else if(strncasecmp(t1, "connection:", 11) == 0) {
        if(strncasecmp(t2, "close", 5) == 0)
        {
            hi->response->close = TRUE;
        }
    }

    return 1;
}

/**
 * @brief Parse recv buffer, and return parse state.
 *
 * @param hi
 * @return int
 */
int http_parse(HTTP_INFO *hi)
{
    char    *p1, *p2;
    long    len;

    p1 = hi->parse_ptr;

    if(p1 == NULL || hi->recv_buf_len == 0) {
        if(p1 == NULL && hi->recv_buf_len == 0) {
            // first time to parse, so we need to read.
            return HTTP_PARSE_READ;
        } else {
            // data pointer is NULL or no data recv data, NO need to parse.
            return HTTP_PARSE_ERROR;
        }
    }

    while(1) {
        if(HTTP_PARSE_HEADER == hi->parse_state) {
            // parse header first.
            if((p2 = strstr(p1, "\r\n")) != NULL) {
                // find a valid header entry, like "content-type: text/html; charset=utf-8\r\n"
                len = (long)(p2 - p1);
                *p2 = 0;

                if(len > 0) {
                    // p1 --> one header line, like "Content-Length: 1730411"
                    http_parse_header_field(hi, p1);
                    p1 = p2 + 2; // skip CR+LF
                } else {
                    // reach the header end
                    hi->parse_state = HTTP_PARSE_BODY;
                    p1 = p2 + 2; // skip CR+LF
                    if(TRUE == hi->response->chunked) {
                        len = hi->recv_buf_len - (p1 - hi->recv_buf);
                        if(len > 0) {
                            if((p2 = strstr(p1, "\r\n")) != NULL) {
                                *p2 = 0;
                                if((hi->chunk_size = strtol(p1, NULL, 16)) > 0) {
                                    // read one chunk
                                    hi->response->content_length += hi->chunk_size;
                                    hi->remain_size = hi->chunk_size;
                                    hi->parse_ptr = p2 + 2; // skip CR+LF
                                    return HTTP_PARSE_CHUNK;
                                } else {
                                    // reach end.
                                    hi->parse_state = HTTP_PARSE_END;
                                    hi->remain_size = 0;
                                    return HTTP_PARSE_END;
                                }
                            } else {
                                // copy the data as chunked size
                                memcpy(hi->recv_buf, p1, len);
                                hi->recv_buf[len] = 0;
                                hi->recv_buf_len = len;
                                hi->remain_size = -1;
                                return HTTP_PARSE_READ;
                            }
                        } else {
                            hi->recv_buf_len = 0;
                            hi->remain_size = -1;
                            return HTTP_PARSE_READ;
                        }
                    } else {
                        // In HTTP/1.0, Read in content_length size
                        if(hi->response->content_length == 0)
                            hi->remain_size = -1;
                        else
                            hi->remain_size = hi->response->content_length;
                    }
                }
            } else {
                len = hi->recv_buf_len - (p1 - hi->recv_buf);
                if(len  > 0) {
                    // keep the partial header data
                    memcpy(hi->recv_buf, p1, len);
                    hi->recv_buf[len] = 0;
                    hi->recv_buf_len = len;
                } else {
                    hi->recv_buf_len = 0;
                }
                return HTTP_PARSE_READ;
            }
        } else if(HTTP_PARSE_BODY == hi->parse_state) {
            // body parser
            if(TRUE == hi->response->chunked && hi->remain_size == -1) {
                len = hi->recv_buf_len - (p1 - hi->recv_buf);
                if(len > 0) {
                    if ((p2 = strstr(p1, "\r\n")) != NULL) {
                        *p2 = 0;
                        if((hi->chunk_size = strtol(p1, NULL, 16)) > 0) {
                            hi->response->content_length += hi->chunk_size;
                            hi->remain_size = hi->chunk_size;
                            hi->parse_ptr = p2 + 2;    // skip CR+LF
                            return HTTP_PARSE_CHUNK;
                        } else {
                            hi->parse_state = HTTP_PARSE_END;
                            hi->remain_size = 0;
                            return HTTP_PARSE_END;
                        }
                    } else {
                        // copy the remain data as chunked size
                        memcpy(hi->recv_buf, p1, len);
                        hi->recv_buf[len] = 0;
                        hi->recv_buf_len = len;
                        hi->remain_size = -1;

                        return HTTP_PARSE_READ;
                    }
                } else {
                    hi->recv_buf_len = 0;
                    return HTTP_PARSE_READ;
                }
            } else {
                if(hi->remain_size > 0) {
                    len = hi->recv_buf_len - (p1 - hi->recv_buf);
                    if(hi->remain_size < len) {
                        // copy the data for response
                        hi->body = p1;
                        hi->body_len = hi->remain_size;

                        p1 += hi->remain_size;
                        len -= hi->remain_size;

                        if(hi->response->chunked == TRUE && len >= 2) {
                            p1 += 2;    // skip CR+LF
                            hi->remain_size = -1;
                            hi->parse_ptr = p1;
                            return HTTP_PARSE_WRITE;
                        } else {
                            return HTTP_PARSE_WRITE|HTTP_PARSE_ERROR;
                        }
                    } else {
                        hi->body = p1;
                        hi->body_len = len;

                        hi->remain_size -= len;
                        hi->recv_buf_len = 0;

                        if(FALSE == hi->response->chunked && hi->remain_size == 0) {
                            return HTTP_PARSE_WRITE|HTTP_PARSE_END;
                        }

                        return HTTP_PARSE_WRITE|HTTP_PARSE_READ;
                    }
                } else {
                    if(TRUE == hi->response->chunked) {
                        // chunked size check
                        if ((hi->recv_buf_len > 2) && (memcmp(p1, "\r\n", 2) == 0)) {
                            p1 += 2;
                            hi->remain_size = -1;
                        } else {
                            hi->remain_size = -1;
                            hi->recv_buf_len = 0;
                        }
                    } else {
                        len = hi->recv_buf_len - (p1 - hi->recv_buf);

                        hi->body = p1;
                        hi->body_len = len;

                        if(hi->remain_size == -1)
                            hi->response->content_length += len;

                        hi->recv_buf_len = 0;
                        return HTTP_PARSE_WRITE|HTTP_PARSE_READ;
                    } // hi->response->chunked
                } // hi->remain_size > 0 or not
            } // body parse
        } // HTTP_PARSE_HEADER or HTTP_PARSE_BODY
    } // while(1)
}

/*---------------------------------------------------------------------*/
/**
 * @brief Init TLS if @is_https is set, check CA if verify is TRUE.
 *
 * @param hi
 * @param is_https
 * @param verify
 * @return int return 0 on success, -1 on failure.
 */
static int https_init(HTTP_INFO **hi, bool is_https, bool verify)
{
    LOG_D("https_init\r\n");
    if(is_https == TRUE) {
        mbedtls_ssl_init( &(*hi)->tls.ssl );
        mbedtls_ssl_config_init( &(*hi)->tls.conf );
        mbedtls_x509_crt_init( &(*hi)->tls.cacert );
        mbedtls_ctr_drbg_init( &(*hi)->tls.ctr_drbg );
    }

    mbedtls_net_init(&(*hi)->tls.ssl_fd);

    (*hi)->tls.verify = verify;
    (*hi)->url->is_https = is_https;
    (*hi)->isInited = TRUE;

    return 0;
}

/**
 * @brief Close network and free TLS resources if TLS in enabled.
 *
 * @param hi
 * @return int return 0 on success, others on failure.
 */
static int https_close(HTTP_INFO **hi)
{
    if(*hi == NULL || (*hi)->isInited == FALSE) return -1;

    if((*hi)->url->is_https == 1)
    {
        mbedtls_ssl_close_notify( &(*hi)->tls.ssl );
    }

    mbedtls_net_free( &(*hi)->tls.ssl_fd );

    if( (*hi)->url->is_https == 1)
    {
        mbedtls_x509_crt_free( &(*hi)->tls.cacert );
        mbedtls_ssl_free( &(*hi)->tls.ssl );
        mbedtls_ssl_config_free( &(*hi)->tls.conf );
        mbedtls_ctr_drbg_free( &(*hi)->tls.ctr_drbg );
        mbedtls_entropy_free( &(*hi)->tls.entropy );
    }

    (*hi)->isInited = FALSE;

    return 0;
}

#if 0
/*
 * Initiate a TCP connection with host:port and the given protocol
 * waiting for timeout (ms)
 */
static int mbedtls_net_connect_timeout( mbedtls_net_context *ctx,
                                        const char *host, const char *port,
                                        int proto, uint32_t timeout )
{
    int ret;
    struct addrinfo hints, *addr_list, *cur;

    //signal( SIGPIPE, SIG_IGN );

    /* Do name resolution with both IPv6 and IPv4 */
    memset( &hints, 0, sizeof(hints) );

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = proto == MBEDTLS_NET_PROTO_UDP ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = proto == MBEDTLS_NET_PROTO_UDP ? IPPROTO_UDP : IPPROTO_TCP;

    if( getaddrinfo( host, port, &hints, &addr_list ) != 0 )
        return( MBEDTLS_ERR_NET_UNKNOWN_HOST );

    /* Try the sockaddrs until a connection succeeds */
    ret = MBEDTLS_ERR_NET_UNKNOWN_HOST;
    for( cur = addr_list; cur != NULL; cur = cur->ai_next )
    {
        ctx->fd = (int) socket( cur->ai_family, cur->ai_socktype,
                                cur->ai_protocol );
        if( ctx->fd < 0 )
        {
            ret = MBEDTLS_ERR_NET_SOCKET_FAILED;
            continue;
        }

        if( mbedtls_net_set_nonblock( ctx ) < 0 )
        {
            close( ctx->fd );
            ctx->fd = -1;
            ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
            break;
        }

        if( connect( ctx->fd, cur->ai_addr, cur->ai_addrlen ) == 0 )
        {
            ret = 0;
            break;
        }
        else if( errno == EINPROGRESS )
        {
            int            fd = (int)ctx->fd;
            int            opt;
            socklen_t      slen;
            struct timeval tv;
            fd_set         fds;

            while(1)
            {
                FD_ZERO( &fds );
                FD_SET( fd, &fds );

                tv.tv_sec  = timeout / 1000;
                tv.tv_usec = ( timeout % 1000 ) * 1000;

                ret = select( fd+1, NULL, &fds, NULL, timeout == 0 ? NULL : &tv );
                if( ret == -1 )
                {
                    if(errno == EINTR) continue;
                }
                else if( ret == 0 )
                {
                    close( fd );
                    ctx->fd = -1;
                    ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
                }
                else
                {
                    ret = 0;

                    slen = sizeof(int);
                    if( (getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&opt, &slen) == 0) && (opt > 0) )
                    {
                        close( fd );
                        ctx->fd = -1;
                        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
                    }
                }

                break;
            }

            break;
        }

        close( ctx->fd );
        ctx->fd = -1;
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    freeaddrinfo( addr_list );

    if( (ret == 0) && (mbedtls_net_set_block( ctx ) < 0) )
    {
        close( ctx->fd );
        ctx->fd = -1;
        ret = MBEDTLS_ERR_NET_CONNECT_FAILED;
    }

    return( ret );
}
#endif

/**
 * @brief Connect to server according to @host and @port.
 *
 * @param hi
 * @param host
 * @param port
 * @return int Return 0 on success, others on failure.
 */
static int https_connect(HTTP_INFO *hi, char *host, char *port)
{
    int ret, is_https;

    is_https = hi->url->is_https;

    if(is_https == 1) {
        mbedtls_entropy_init( &hi->tls.entropy );

        ret = mbedtls_ctr_drbg_seed( &hi->tls.ctr_drbg, mbedtls_entropy_func, &hi->tls.entropy, NULL, 0);
        if( ret != 0 ) {
            return ret;
        }

        // ca_crt_rsa[ca_crt_rsa_size - 1] = 0;
        ret = mbedtls_x509_crt_parse(&hi->tls.cacert, (uint8_t *)ca_crt_rsa, ca_crt_rsa_size);
        if( ret != 0 ) {
            return ret;
        }

        ret = mbedtls_ssl_config_defaults( &hi->tls.conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT );
        if( ret != 0 ) {
            return ret;
        }

        /* OPTIONAL is not optimal for security,
         * but makes interop easier in this simplified example */
        mbedtls_ssl_conf_authmode( &hi->tls.conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
        mbedtls_ssl_conf_ca_chain( &hi->tls.conf, &hi->tls.cacert, NULL );
        mbedtls_ssl_conf_rng( &hi->tls.conf, mbedtls_ctr_drbg_random, &hi->tls.ctr_drbg );
        mbedtls_ssl_conf_read_timeout( &hi->tls.conf, 5000 );

        ret = mbedtls_ssl_setup( &hi->tls.ssl, &hi->tls.conf );
        if( ret != 0 ) {
            return ret;
        }

        // NOTE: no need to set hostname
        // ret = mbedtls_ssl_set_hostname( &hi->tls.ssl, host );
        // if( ret != 0 )
        // {
        //     return ret;
        // }
    }

    // ret = mbedtls_net_connect_timeout(&hi->tls.ssl_fd, host, port, MBEDTLS_NET_PROTO_TCP, 5000);
    ret = mbedtls_net_connect(&hi->tls.ssl_fd, host, port, MBEDTLS_NET_PROTO_TCP);
    if( ret != 0 ) {
        return ret;
    }

    if(is_https == 1) {
        mbedtls_ssl_set_bio( &hi->tls.ssl, &hi->tls.ssl_fd,
                            mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout );

        while ( (ret = mbedtls_ssl_handshake(&hi->tls.ssl)) != 0 ) {
            if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
                return ret;
            }
        }

        /* In real life, we probably want to bail out when ret != 0 */
        if( hi->tls.verify && (mbedtls_ssl_get_verify_result(&hi->tls.ssl) != 0) ) {
            return MBEDTLS_ERR_X509_CERT_VERIFY_FAILED;
        }
    }

    return 0;
}

/**
 * @brief Send data.
 *
 * @param hi
 * @param buffer
 * @param len
 * @return int Return how many bytes are actually send, or negative number on failure.
 */
static int https_write(HTTP_INFO *hi, char *buffer, int len)
{
    int ret, slen = 0;

    while(1)
    {
        if(hi->url->is_https == 1)
            ret = mbedtls_ssl_write(&hi->tls.ssl, (u_char *)&buffer[slen], (size_t)(len-slen));
        else
            ret = mbedtls_net_send(&hi->tls.ssl_fd, (u_char *)&buffer[slen], (size_t)(len-slen));

        if(ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        } else if(ret <= 0) {
            return ret;
        }

        slen += ret;

        if(slen >= len) {
            break;
        }
    }

    return slen;
}

/**
 * @brief Read data.
 *
 * @param hi
 * @param buffer
 * @param len
 * @return int
 */
static int https_read(HTTP_INFO *hi, char *buffer, int len)
{
    int ret;

    while(1)
    {
        if(hi->url->is_https == 1)
            ret = mbedtls_ssl_read(&hi->tls.ssl, (u_char *) buffer, (size_t) len);
        else
            ret = mbedtls_net_recv_timeout(&hi->tls.ssl_fd, (u_char *)buffer, (size_t)len, 5000);

        if (ret != MBEDTLS_ERR_SSL_WANT_READ) break;
    }

    return ret;
}

/**
 * @brief Allocate memory for a new HTTP_INFO
 *
 * @return HTTP_INFO* Return valid pointer on success, NULL on failure
 */
HTTP_INFO *httpinfo_new(void)
{
    HTTP_INFO *hi = NULL;

    hi = (HTTP_INFO *)hc_malloc(sizeof(HTTP_INFO));
    if (!hi) {
        return NULL;
    }
    memset(hi, 0, sizeof(HTTP_INFO));

    // HTTP_URL *url;
    hi->url = (HTTP_URL *)hc_malloc(sizeof(HTTP_URL));
    if (!hi->url) {
        // error when malloc
        goto error_flag_url;
    }
    memset(hi->url, 0, sizeof(HTTP_URL));

    hi->url->is_https = FALSE;

    hi->url->host = (char *)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (!hi->url->host) {
        goto error_flag_url_host;
    }
    hi->url->host_maxlen = FIELD_URL_HOST_MAX_LEN;
    memset(hi->url->host, 0, FIELD_URL_HOST_MAX_LEN);

    memset(hi->url->port, 0, sizeof(hi->url->port));

    hi->url->path = (char *)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (!hi->url->path) {
        goto error_flag_url_path;
    }
    hi->url->path_maxlen = FIELD_URL_PATH_MAX_LEN;
    memset(hi->url->path, 0, FIELD_URL_PATH_MAX_LEN);

    // HTTP_HEADER *request;
    hi->request = (HTTP_HEADER *)hc_malloc(sizeof(HTTP_HEADER));
    if (!hi->request) {
        goto error_flag_request;
    }
    memset(hi->request, 0, sizeof(HTTP_HEADER));
    memset(hi->request->method, 0, sizeof(hi->request->method));

    hi->request->status_code = 0;

    hi->request->content_type = (char *)hc_malloc(FIELD_REQUEST_CONTENT_TYPE_MAX_LEN);
    if (!hi->request->content_type) {
        goto error_flag_request_content_type;
    }
    memset(hi->request->content_type, 0, FIELD_REQUEST_CONTENT_TYPE_MAX_LEN);
    hi->request->content_type_maxlen = FIELD_REQUEST_CONTENT_TYPE_MAX_LEN;
    hi->request->content_length = 0;

    hi->request->chunked = FALSE;

    hi->request->close = FALSE;

    hi->request->location = (char *)hc_malloc(FIELD_REQUEST_LOCATION_MAX_LEN);
    if (!hi->request->location) {
        goto error_flag_request_location;
    }
    memset(hi->request->location, 0, FIELD_REQUEST_LOCATION_MAX_LEN);
    hi->request->location_maxlen = FIELD_REQUEST_LOCATION_MAX_LEN;

    hi->request->referrer = (char *)hc_malloc(FIELD_REQUEST_REFERRER_MAX_LEN);
    if (!hi->request->referrer) {
        goto error_flag_request_referrer;
    }
    memset(hi->request->referrer, 0, FIELD_REQUEST_REFERRER_MAX_LEN);
    hi->request->referrer_maxlen = FIELD_REQUEST_REFERRER_MAX_LEN;

    hi->request->cookie = (char *)hc_malloc(FIELD_REQUEST_COOKIE_MAX_LEN);
    if (!hi->request->cookie) {
        goto error_flag_request_cookie;
    }
    memset(hi->request->cookie, 0, FIELD_REQUEST_COOKIE_MAX_LEN);
    hi->request->cookie_maxlen = FIELD_REQUEST_COOKIE_MAX_LEN;

    hi->request->boundary = (char *)hc_malloc(FIELD_REQUEST_BOUNDARY_MAX_LEN);
    if (!hi->request->boundary) {
        goto error_flag_request_boundary;
    }
    memset(hi->request->boundary, 0, FIELD_REQUEST_BOUNDARY_MAX_LEN);
    hi->request->boundary_maxlen = FIELD_REQUEST_BOUNDARY_MAX_LEN;

    // HTTP_HEADER *response;
    hi->response = (HTTP_HEADER *)hc_malloc(sizeof(HTTP_HEADER));
    if (!hi->response) {
        goto error_flag_response;
    }
    memset(hi->response, 0, sizeof(HTTP_HEADER));
    memset(hi->response->method, 0, sizeof(hi->response->method));

    hi->response->status_code = 0;

    hi->response->content_type = (char *)hc_malloc(FIELD_RESPONSE_CONTENT_TYPE_MAX_LEN);
    if (!hi->response->content_type) {
        goto error_flag_response_content_type;
    }
    memset(hi->response->content_type, 0, FIELD_RESPONSE_CONTENT_TYPE_MAX_LEN);
    hi->response->content_type_maxlen = FIELD_RESPONSE_CONTENT_TYPE_MAX_LEN;
    hi->response->content_length = 0;

    hi->response->chunked = FALSE;

    hi->response->close = FALSE;

    hi->response->location = (char *)hc_malloc(FIELD_RESPONSE_LOCATION_MAX_LEN);
    if (!hi->response->location) {
        goto error_flag_response_location;
    }
    memset(hi->response->location, 0, FIELD_RESPONSE_LOCATION_MAX_LEN);
    hi->response->location_maxlen = FIELD_RESPONSE_LOCATION_MAX_LEN;

    hi->response->referrer = (char *)hc_malloc(FIELD_RESPONSE_REFERRER_MAX_LEN);
    if (!hi->response->referrer) {
        goto error_flag_response_referrer;
    }
    memset(hi->response->referrer, 0, FIELD_RESPONSE_REFERRER_MAX_LEN);
    hi->response->referrer_maxlen = FIELD_RESPONSE_REFERRER_MAX_LEN;

    hi->response->cookie = (char *)hc_malloc(FIELD_RESPONSE_COOKIE_MAX_LEN);
    if (!hi->response->cookie) {
        goto error_flag_response_cookie;
    }
    memset(hi->response->cookie, 0, FIELD_RESPONSE_COOKIE_MAX_LEN);
    hi->response->cookie_maxlen = FIELD_RESPONSE_COOKIE_MAX_LEN;

    hi->response->boundary = (char *)hc_malloc(FIELD_RESPONSE_BOUNDARY_MAX_LEN);
    if (!hi->response->boundary) {
        goto error_flag_response_boundary;
    }
    memset(hi->response->boundary, 0, FIELD_RESPONSE_BOUNDARY_MAX_LEN);
    hi->response->boundary_maxlen = FIELD_RESPONSE_BOUNDARY_MAX_LEN;

    // NOTE: append 4 bytes to avoid heap corruption.
    hi->recv_buf = (char *)hc_malloc(FIELD_HTTP_RECV_BUF_MAX_LEN + 4);
    if (!hi->recv_buf) {
        goto error_flag_recv_buf;
    }
    memset(hi->recv_buf, 0, FIELD_HTTP_RECV_BUF_MAX_LEN + 4);
    hi->recv_buf_maxlen = FIELD_HTTP_RECV_BUF_MAX_LEN;

    // OK
    return hi;

error_flag_recv_buf:
    hc_free(hi->response->boundary);
    hi->response->boundary = NULL;

error_flag_response_boundary:
    hc_free(hi->response->cookie);
    hi->response->cookie = NULL;

error_flag_response_cookie:
    hc_free(hi->response->referrer);
    hi->response->referrer = NULL;

error_flag_response_referrer:
    hc_free(hi->response->location);
    hi->response->location = NULL;

error_flag_response_location:
    hc_free(hi->response->content_type);
    hi->response->content_type = NULL;

error_flag_response_content_type:
    hc_free(hi->response);
    hi->response = NULL;

error_flag_response:
    hc_free(hi->request->boundary);
    hi->request->boundary = NULL;

error_flag_request_boundary:
    hc_free(hi->request->cookie);
    hi->request->cookie = NULL;

error_flag_request_cookie:
    hc_free(hi->request->referrer);
    hi->request->referrer = NULL;

error_flag_request_referrer:
    hc_free(hi->request->location);
    hi->request->location = NULL;

error_flag_request_location:
    hc_free(hi->request->content_type);
    hi->request->content_type = NULL;

error_flag_request_content_type:
    hc_free(hi->request);
    hi->request = NULL;

error_flag_request:
    hc_free(hi->url->path);
    hi->url->path = NULL;

error_flag_url_path:
    hc_free(hi->url->host);
    hi->url->host = NULL;

error_flag_url_host:
    hc_free(hi->url);
    hi->url = NULL;

error_flag_url:
    hc_free(hi);
    hi = NULL;
    return hi;
}

/**
 * @brief Deallocate memory for HTTP_INFO
 * NOTE: It can not be called on the same HTTP_INFO *hi twice.
 * @param hi
 */
void httpinfo_delete(HTTP_INFO *hi)
{
    if (!hi) {
        return;
    }

    hc_free(hi->recv_buf);
    hi->recv_buf = NULL;

    // response
    hc_free(hi->response->boundary);
    hi->response->boundary = NULL;

    hc_free(hi->response->cookie);
    hi->response->cookie = NULL;

    hc_free(hi->response->referrer);
    hi->response->referrer = NULL;

    hc_free(hi->response->location);
    hi->response->location = NULL;

    hc_free(hi->response->content_type);
    hi->response->content_type = NULL;

    hc_free(hi->response);
    hi->response = NULL;

    // request
    hc_free(hi->request->boundary);
    hi->request->boundary = NULL;

    hc_free(hi->request->cookie);
    hi->request->cookie = NULL;

    hc_free(hi->request->referrer);
    hi->request->referrer = NULL;

    hc_free(hi->request->location);
    hi->request->location = NULL;

    hc_free(hi->request->content_type);
    hi->request->content_type = NULL;

    hc_free(hi->request);
    hi->request = NULL;

    hc_free(hi->url->path);
    hi->url->path = NULL;

    hc_free(hi->url->host);
    hi->url->host = NULL;

    hc_free(hi->url);
    hi->url = NULL;

    hc_free(hi);
    hi = NULL;
}

/**
 * @brief Init data structure
 *
 * @param hi
 * @param verify Choose True to verify CA
 * @return int
 */
int http_init(HTTP_INFO **hi, bool verify)
{
    return https_init(hi, 0, verify);
}

/**
 * @brief Close connection.
 *
 * @param hi
 * @return int
 */
int http_close(HTTP_INFO *hi)
{
    return https_close(&hi);
}

/**
 * @brief Set HTTP method
 *
 * @param hi
 * @param type
 */
void http_set_method(HTTP_INFO *hi, http_method_type_t type)
{
    if (!hi) {
        return;
    }

    switch (type) {
    case HTTP_METHOD_GET:
        snprintf(hi->request->method, FIELD_HTTP_HEADER_METHOD_MAX_LEN, "GET");
        break;
    case HTTP_METHOD_POST:
        snprintf(hi->request->method, FIELD_HTTP_HEADER_METHOD_MAX_LEN, "POST");
        break;
    case HTTP_METHOD_INVALID:
    default:
        // NOTE: default method is "GET"
        snprintf(hi->request->method, FIELD_HTTP_HEADER_METHOD_MAX_LEN, "GET");
        break;
    }
}


/**
 * @brief Fetch data from a given URL.
 * Close the connection if error occurs or the server send close notify.
 * @param hi which must be init before this call.
 * @param url_str URL string.
 * @param response where to keep data from HTTP.
 * @param maxSize max size of data.
 * @return int return how many bytes received; return -1 on error.
 */
int http_get(HTTP_INFO *hi, char *url_str, char *response, int maxSize)
{
    char        *request = NULL, *host = NULL, *path = NULL;
    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN] = {0};
    int         sock_fd, is_https, verify;
    int         ret, mode, opt;
    int         write_len = 0, recv_len = 0;
    int         errCode = 0;
    socklen_t   socklen;

    if(NULL == hi) {
        errCode = -1;
        LOG_EMSG("hi MUST NOT be NULL\r\n");
        goto flag_exit;
    }

    request = (char *)hc_malloc(FIELD_HTTP_HEADER_TOTAL_MAX_LEN);
    if (request == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @request failed!\r\n");
        goto flag_exit;
    }

    host = (char *)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (host == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @host failed!\r\n");
        goto flag_release_request;
    }

    path = (char *)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (path == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @path failed!\r\n");
        goto flag_release_host;
    }

    verify = hi->tls.verify;

    if (parse_url(url_str, &is_https, host, port, path) != 0) {
        errCode = -2;
        goto flag_release_all;
    }

    if( (hi->tls.ssl_fd.fd == -1) || (hi->url->is_https != is_https) ||
        (strcmp(hi->url->host, host) != 0) || (strcmp(hi->url->port, port) != 0) ) {

        https_close(&hi);
        https_init(&hi, is_https, verify);
        if((ret=https_connect(hi, host, port)) < 0) {
            https_close(&hi);
            errCode = -1;
            goto flag_release_all;
        }
    } else {
        sock_fd = hi->tls.ssl_fd.fd;
        socklen = sizeof(int);
        if((getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void *)&opt, &socklen) < 0) || (opt > 0)) {
            https_close(&hi);
            https_init(&hi, is_https, verify);
            if((ret=https_connect(hi, host, port)) < 0) {
                https_close(&hi);
                http_dump_err_msg();
                errCode = -1;
                goto flag_release_all;
            }
        }
    }

    /* Send HTTP request */
    write_len = snprintf(request, FIELD_HTTP_HEADER_TOTAL_MAX_LEN,
            "GET %s HTTP/1.1\r\n"
            "User-Agent: Mozilla/4.0\r\n"
            "Host: %s:%s\r\n"
            "Content-Type: application/json; charset=utf-8\r\n"
            "Connection: Keep-Alive\r\n"
            "%s\r\n",
            path, host, port, hi->request->cookie);

    if((ret = https_write(hi, request, write_len)) != write_len) {
        https_close(&hi);

        http_dump_err_msg();
        errCode = -1;
        goto flag_release_all;
    }

    http_read_init(hi);

    recv_len = 0;

    while(1) {
        mode = http_parse(hi);
        if((mode & HTTP_PARSE_WRITE) == HTTP_PARSE_WRITE) {
            // NOTE: fetch data from HTTP.
            if(recv_len < maxSize - 1) {
                if (recv_len + hi->body_len > maxSize) {
                    strncpy(&response[recv_len], hi->body, maxSize - recv_len - 1);
                    recv_len = maxSize - 1;
                    response[recv_len] = 0;
                } else {
                    strncpy(&response[recv_len], hi->body, hi->body_len);
                    recv_len += hi->body_len;
                    response[recv_len] = 0;
                }
            }
        }

        if((mode & HTTP_PARSE_READ) == HTTP_PARSE_READ) {
            ret = https_read(hi, &hi->recv_buf[hi->recv_buf_len], (int) (FIELD_HTTP_RECV_BUF_MAX_LEN - hi->recv_buf_len));
            if (ret < 0) {
                https_close(&hi);

                http_dump_err_msg();
                errCode = -1;
                goto flag_release_all;
            } else if (ret == 0) {
                https_close(&hi);
                break;
            }

            hi->recv_buf_len += ret;
            hi->recv_buf[hi->recv_buf_len] = 0;
            hi->parse_ptr = hi->recv_buf;
        } else if((mode & HTTP_PARSE_CHUNK) == HTTP_PARSE_CHUNK) {
            LOG_I("return: HTTP_PARSE_CHUNK: chunk_size: %ld \r\n", hi->chunk_size);
        } else if((mode & HTTP_PARSE_END) == HTTP_PARSE_END) {
            LOG_I("return: HTTP_PARSE_END: content_length: %ld \r\n", hi->response->content_length);
            break;
        } else if((mode & HTTP_PARSE_ERROR) == HTTP_PARSE_ERROR) {
            LOG_EMSG("http_get() parse error.\r\n");
            errCode = -1;
            goto flag_release_all;
        }
    } // while(1)

    if(hi->response->close == TRUE) {
        https_close(&hi);
    } else {
        strncpy(hi->url->host, host, strlen(host));
        strncpy(hi->url->port, port, strlen(port));
        strncpy(hi->url->path, path, strlen(path));
    }

    LOG_I("status_code: %d \r\n", hi->response->status_code);
    LOG_I("cookie: %s      \r\n", hi->response->cookie);
    LOG_I("location: %s    \r\n", hi->response->location);
    LOG_I("referrer: %s    \r\n", hi->response->referrer);
    LOG_I("length: %ld     \r\n", hi->response->content_length);

flag_release_all:
    hc_free(path);

flag_release_host:
    hc_free(host);

flag_release_request:
    hc_free(request);

flag_exit:
    if (errCode == -1) {
        return errCode;
    }
    return recv_len;
}

/**
 * @brief Store data with @cbFunc after fetch data block when HTTP GET.
 *
 * @param hi HTTP_INFO object.
 * @param url_str URL, like "https://anime-music.jijidown.com/api/v2/music"
 * @param cb callback function.
 * @return int return how many bytes are actually received.
 */
int  http_get_with_callback(HTTP_INFO* hi, char* url_str, http_get_cb cbFunc)
{
    char        *request = NULL, *host = NULL, *path = NULL;
    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN] = {0};
    int         sock_fd, is_https, verify;
    int         ret, mode, opt;
    int         write_len = 0, recv_len = 0;
    int         errCode = 0;
    socklen_t   socklen;
    int8_t      is_recv_finished = 0;

    if(NULL == hi) {
        errCode = -1;
        LOG_EMSG("hi MUST NOT be NULL\r\n");
        goto flag_exit;
    }

    request = (char *)hc_malloc(FIELD_HTTP_HEADER_TOTAL_MAX_LEN);
    if (request == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @request failed!\r\n");
        goto flag_exit;
    }

    host = (char *)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (host == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @host failed!\r\n");
        goto flag_release_request;
    }

    path = (char *)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (path == NULL) {
        errCode = -1;
        LOG_EMSG("malloc for @path failed!\r\n");
        goto flag_release_host;
    }

    verify = hi->tls.verify;

    if (parse_url(url_str, &is_https, host, port, path) != 0) {
        errCode = -2;
        goto flag_release_all;
    }

    if( (hi->tls.ssl_fd.fd == -1) || (hi->url->is_https != is_https) ||
        (strcmp(hi->url->host, host) != 0) || (strcmp(hi->url->port, port) != 0) ) {

        https_close(&hi);
        https_init(&hi, is_https, verify);
        if((ret=https_connect(hi, host, port)) < 0) {
            https_close(&hi);
            errCode = -1;
            goto flag_release_all;
        }
    } else {
        sock_fd = hi->tls.ssl_fd.fd;
        socklen = sizeof(int);
        if((getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void *)&opt, &socklen) < 0) || (opt > 0)) {
            https_close(&hi);
            https_init(&hi, is_https, verify);
            if((ret=https_connect(hi, host, port)) < 0) {
                https_close(&hi);
                http_dump_err_msg();
                errCode = -1;
                goto flag_release_all;
            }
        }
    }

    /* Send HTTP request */
    write_len = snprintf(request, FIELD_HTTP_HEADER_TOTAL_MAX_LEN,
            "GET %s HTTP/1.1\r\n"
            "User-Agent: Lightningsemi HTTP Client/1.0\r\n"
            "Host: %s:%s\r\n"
			"Content-Length: 0\r\n" 
            "Content-Type: %s\r\n"
            "Connection: Keep-Alive\r\n"
            "%s\r\n",
            path, host, port, s_contentType[s_contentTypeIdx], hi->request->cookie);

    if((ret = https_write(hi, request, write_len)) != write_len) {
        https_close(&hi);

        http_dump_err_msg();
        errCode = -1;
        goto flag_release_all;
    }

    http_read_init(hi);

    while(1) {
        mode = http_parse(hi);
        if((mode & HTTP_PARSE_WRITE) == HTTP_PARSE_WRITE) {
            // NOTE: fetch data from HTTP.
            is_recv_finished = 0;
            if (cbFunc) {
                int32_t ret = 0;
                ret = (*cbFunc) (hi->body, hi->body_len, hi->response->content_length, is_recv_finished);
                if (ret < 0) {
                    errCode = -1;
                    goto flag_release_all;
                } else {
                    recv_len += ret;
                }
            }
        }

        if((mode & HTTP_PARSE_READ) == HTTP_PARSE_READ) {
            ret = https_read(hi, &hi->recv_buf[hi->recv_buf_len], (int) (FIELD_HTTP_RECV_BUF_MAX_LEN - hi->recv_buf_len));
            if (ret < 0) {
                https_close(&hi);

                http_dump_err_msg();
                errCode = -1;
                goto flag_release_all;
            } else if (ret == 0) {
                https_close(&hi);
                break;
            }

            hi->recv_buf_len += ret;
            hi->recv_buf[hi->recv_buf_len] = 0;
            hi->parse_ptr = hi->recv_buf;
        } else if((mode & HTTP_PARSE_CHUNK) == HTTP_PARSE_CHUNK) {
            LOG_I("return: HTTP_PARSE_CHUNK: chunk_size: %ld \r\n", hi->chunk_size);
        } else if((mode & HTTP_PARSE_END) == HTTP_PARSE_END) {
            LOG_I("return: HTTP_PARSE_END: content_length: %ld \r\n", hi->response->content_length);
            if (cbFunc) {
                is_recv_finished = 1;
                (*cbFunc) (hi->body, hi->body_len, hi->response->content_length, is_recv_finished);
            }
            break;
        } else if((mode & HTTP_PARSE_ERROR) == HTTP_PARSE_ERROR) {
            LOG_EMSG("http_get() parse error.\r\n");
            errCode = -1;
            goto flag_release_all;
        }
    } // while(1)

    if(hi->response->close == TRUE) {
        https_close(&hi);
    } else {
        strncpy(hi->url->host, host, strlen(host));
        strncpy(hi->url->port, port, strlen(port));
        strncpy(hi->url->path, path, strlen(path));
    }

    LOG_I("status_code: %d \r\n", hi->response->status_code);
    LOG_I("cookie: %s      \r\n", hi->response->cookie);
    LOG_I("location: %s    \r\n", hi->response->location);
    LOG_I("referrer: %s    \r\n", hi->response->referrer);
    LOG_I("length: %ld     \r\n", hi->response->content_length);

flag_release_all:
    hc_free(path);

flag_release_host:
    hc_free(host);

flag_release_request:
    hc_free(request);

flag_exit:
    if (errCode == -1) {
        return errCode;
    }
    return recv_len;
}


/**
 * @brief
 *
 * @param hi
 * @param url
 * @param data
 * @param recvBuf
 * @param size
 * @return int
 */
int http_post(HTTP_INFO *hi, char *url, char *data, char *recvBuf, int size)
{
    int         sock_fd, is_https, verify;
    int         ret, mode, opt, len;
    socklen_t   slen;
    int         retCode = 0; // record error code.
    char        * tempBuffer = NULL, * host = NULL, * path = NULL;
    const int   tempBufferSize = FIELD_URL_HOST_MAX_LEN + FIELD_URL_PATH_MAX_LEN + 4;
    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN] = { 0 };

    if(NULL == hi) {
        retCode = -1;
        return retCode;
    }

    tempBuffer = (char*)hc_malloc(tempBufferSize);
    if (NULL == tempBuffer) {
        retCode = -2;
        return retCode;
    }
    memset(tempBuffer, 0, tempBufferSize);

    host = (char*)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (NULL == host) {
        retCode = -3;
        goto flag_release_tempBuffer;
    }
    memset(host, 0, FIELD_URL_HOST_MAX_LEN);

    path = (char*)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (NULL == path) {
        retCode = -4;
        goto flag_release_host;
    }
    memset(path, 0, FIELD_URL_PATH_MAX_LEN);

    verify = hi->tls.verify;

    if ( parse_url(url, &is_https, host, port, path) != 0) {
        retCode = -5;
        goto flag_exit;
    }

    if( (hi->tls.ssl_fd.fd == -1) || (hi->url->is_https != is_https) ||
        (strcmp(hi->url->host, host) != 0) || (strcmp(hi->url->port, port) != 0) ) {

        if(hi->tls.ssl_fd.fd != -1)
            https_close(&hi);

        https_init(&hi, is_https, verify);

        if((ret=https_connect(hi, host, port)) < 0) {
            https_close(&hi);

            http_dump_err_msg();
            retCode = -6;
            goto flag_exit;
        }
    } else {
        sock_fd = hi->tls.ssl_fd.fd;

        slen = sizeof(int);

        if((getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void *)&opt, &slen) < 0) || (opt > 0)) {
            https_close(&hi);
            https_init(&hi, is_https, verify);

            if((ret=https_connect(hi, host, port)) < 0) {
                https_close(&hi);
				http_dump_err_msg();
				retCode = -7;
				goto flag_exit;
            }
        } else {
            LOG_E("socket reuse: %d \r\n", sock_fd);
        }
    }

    /* Send HTTP request */
    len = snprintf(tempBuffer, tempBufferSize,
            "POST %s HTTP/1.1\r\n"
            "User-Agent: Mozilla/4.0\r\n"
            "Host: %s:%s\r\n"
            "Connection: Keep-Alive\r\n"
            "Accept: */*\r\n"
            "Content-Type: application/json; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "%s\r\n"
            "%s",
            path, host, port,
            (int)strlen(data),
            hi->request->cookie,
            data);

    if((ret = https_write(hi, tempBuffer, len)) != len) {
        https_close(&hi);

		http_dump_err_msg();
		retCode = -8;
		goto flag_exit;
    }

    LOG_D("tempBuffer: \r\n[\r\n%s\r\n]\r\n", tempBuffer);

    http_read_init(hi);

    len = 0;

    while(1) {
        mode = http_parse(hi);
        if((mode & HTTP_PARSE_WRITE) == HTTP_PARSE_WRITE) {
            if(len < size - 1) {
                if (len + hi->body_len > size) {
                    strncpy(&recvBuf[len], hi->body, size - len - 1);
                    len = size - 1;
                    recvBuf[len] = 0;
                } else {
                    strncpy(&recvBuf[len], hi->body, hi->body_len);
                    len += hi->body_len;
                    recvBuf[len] = 0;
                }
            }
        }

        if((mode & HTTP_PARSE_READ) == HTTP_PARSE_READ) {
            ret = https_read(hi, &hi->recv_buf[hi->recv_buf_len], (int) (FIELD_HTTP_RECV_BUF_MAX_LEN - hi->recv_buf_len));
            if (ret < 0) {
                https_close(&hi);

				http_dump_err_msg();
				retCode = -9;
				goto flag_exit;
            } else if (ret == 0) {
                https_close(&hi);
                break;
            }

            hi->recv_buf_len += ret;
            hi->recv_buf[hi->recv_buf_len] = 0;

            hi->parse_ptr = hi->recv_buf;
        } else if((mode & HTTP_PARSE_CHUNK) == HTTP_PARSE_CHUNK) {
            LOG_D("return: HTTP_PARSE_CHUNK: chunk_size: %ld \r\n", hi->chunk_size);
        } else if((mode & HTTP_PARSE_END) == HTTP_PARSE_END) {
            LOG_D("return: HTTP_PARSE_END: content_length: %ld \r\n", hi->response->content_length);
            break;
        } else if((mode & HTTP_PARSE_ERROR) == HTTP_PARSE_ERROR) {
            LOG_EMSG("HTTP_PARSE_ERROR\r\n");
            retCode = -10;
            goto flag_exit;
        }
    }

    if(hi->response->close == TRUE) {
        https_close(&hi);
    } else {
        strncpy(hi->url->host, host, strlen(host));
        strncpy(hi->url->port, port, strlen(port));
        strncpy(hi->url->path, path, strlen(path));
    }

    LOG_D("status_code: %d \r\n", hi->response->status_code);
    LOG_D("     cookie: %s \r\n", hi->response->cookie);
    LOG_D("   location: %s \r\n", hi->response->location);
    LOG_D("   referrer: %s \r\n", hi->response->referrer);
    LOG_D("     length: %d \r\n", hi->response->content_length);
    LOG_D("   body len: %d \r\n", hi->body_len);

flag_exit:

flag_release_path:
    hc_free(path);

flag_release_host:
    hc_free(host);

flag_release_tempBuffer:
    hc_free(tempBuffer);

    if (retCode != 0) {
        return retCode;
    }

    return len;
}

int http_post_request(HTTP_INFO *hi, char *url, char *request, char *recvBuf, int size)
{
    int         sock_fd, is_https, verify;
    int         ret, mode, opt, len;
    socklen_t   slen;
    int         retCode = 0; // record error code.
    char        * tempBuffer = NULL, * host = NULL, * path = NULL;
    const int   tempBufferSize = FIELD_URL_HOST_MAX_LEN + FIELD_URL_PATH_MAX_LEN + 4;
    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN] = { 0 };

    if(NULL == hi) {
        retCode = -1;
        return retCode;
    }
#if 0
    tempBuffer = (char*)hc_malloc(tempBufferSize);
    if (NULL == tempBuffer) {
        retCode = -2;
        return retCode;
    }
    memset(tempBuffer, 0, tempBufferSize);
#endif
    host = (char*)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (NULL == host) {
        retCode = -3;
        goto flag_release_tempBuffer;
    }
    memset(host, 0, FIELD_URL_HOST_MAX_LEN);

    path = (char*)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (NULL == path) {
        retCode = -4;
        goto flag_release_host;
    }
    memset(path, 0, FIELD_URL_PATH_MAX_LEN);

    verify = hi->tls.verify;

    if ( parse_url(url, &is_https, host, port, path) != 0) {
        retCode = -5;
        goto flag_exit;
    }

    if( (hi->tls.ssl_fd.fd == -1) || (hi->url->is_https != is_https) ||
        (strcmp(hi->url->host, host) != 0) || (strcmp(hi->url->port, port) != 0) ) {

        if(hi->tls.ssl_fd.fd != -1)
            https_close(&hi);

        https_init(&hi, is_https, verify);

        if((ret=https_connect(hi, host, port)) < 0) {
            https_close(&hi);

            http_dump_err_msg();
            retCode = -6;
            goto flag_exit;
        }
    } else {
        sock_fd = hi->tls.ssl_fd.fd;

        slen = sizeof(int);

        if((getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void *)&opt, &slen) < 0) || (opt > 0)) {
            https_close(&hi);
            https_init(&hi, is_https, verify);

            if((ret=https_connect(hi, host, port)) < 0) {
                https_close(&hi);
				http_dump_err_msg();
				retCode = -7;
				goto flag_exit;
            }
        } else {
            LOG_E("socket reuse: %d \r\n", sock_fd);
        }
    }

    /* Send HTTP request */
#if 0    
    len = snprintf(tempBuffer, tempBufferSize,
            "POST %s HTTP/1.1\r\n"
            "User-Agent: Mozilla/4.0\r\n"
            "Host: %s:%s\r\n"
            "Connection: Keep-Alive\r\n"
            "Accept: */*\r\n"
            "Content-Type: application/json; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "%s\r\n"
            "%s",
            path, host, port,
            (int)strlen(data),
            hi->request->cookie,
            data);
#endif
    len = strlen(request);
    if((ret = https_write(hi, request, len)) != len) {
        https_close(&hi);

		http_dump_err_msg();
		retCode = -8;
		goto flag_exit;
    }

//    LOG_D("tempBuffer: \r\n[\r\n%s\r\n]\r\n", tempBuffer);

    http_read_init(hi);

    len = 0;

    while(1) {
        mode = http_parse(hi);
        if((mode & HTTP_PARSE_WRITE) == HTTP_PARSE_WRITE) {
            if(len < size - 1) {
                if (len + hi->body_len > size) {
                    strncpy(&recvBuf[len], hi->body, size - len - 1);
                    len = size - 1;
                    recvBuf[len] = 0;
                } else {
                    strncpy(&recvBuf[len], hi->body, hi->body_len);
                    len += hi->body_len;
                    recvBuf[len] = 0;
                }
            }
        }

        if((mode & HTTP_PARSE_READ) == HTTP_PARSE_READ) {
            ret = https_read(hi, &hi->recv_buf[hi->recv_buf_len], (int) (FIELD_HTTP_RECV_BUF_MAX_LEN - hi->recv_buf_len));
            if (ret < 0) {
                https_close(&hi);

				http_dump_err_msg();
				retCode = -9;
				goto flag_exit;
            } else if (ret == 0) {
                https_close(&hi);
                break;
            }

            hi->recv_buf_len += ret;
            hi->recv_buf[hi->recv_buf_len] = 0;

            hi->parse_ptr = hi->recv_buf;
        } else if((mode & HTTP_PARSE_CHUNK) == HTTP_PARSE_CHUNK) {
            LOG_D("return: HTTP_PARSE_CHUNK: chunk_size: %ld \r\n", hi->chunk_size);
        } else if((mode & HTTP_PARSE_END) == HTTP_PARSE_END) {
            LOG_D("return: HTTP_PARSE_END: content_length: %ld \r\n", hi->response->content_length);
            break;
        } else if((mode & HTTP_PARSE_ERROR) == HTTP_PARSE_ERROR) {
            LOG_EMSG("HTTP_PARSE_ERROR\r\n");
            retCode = -10;
            goto flag_exit;
        }
    }

    if(hi->response->close == TRUE) {
        https_close(&hi);
    } else {
        strncpy(hi->url->host, host, strlen(host));
        strncpy(hi->url->port, port, strlen(port));
        strncpy(hi->url->path, path, strlen(path));
    }

    LOG_D("status_code: %d \r\n", hi->response->status_code);
    LOG_D("     cookie: %s \r\n", hi->response->cookie);
    LOG_D("   location: %s \r\n", hi->response->location);
    LOG_D("   referrer: %s \r\n", hi->response->referrer);
    LOG_D("     length: %d \r\n", hi->response->content_length);
    LOG_D("   body len: %d \r\n", hi->body_len);

flag_exit:

flag_release_path:
    hc_free(path);

flag_release_host:
    hc_free(host);

flag_release_tempBuffer:
    //hc_free(tempBuffer);

    if (retCode != 0) {
        return retCode;
    }

    return len;
}



/**
 * @brief Print error msg here
 */
void http_dump_err_msg(void)
{
    #define ERR_STR_MAX_SIZE    (150)
    char errstr[ERR_STR_MAX_SIZE];

    mbedtls_strerror(_error, errstr, ERR_STR_MAX_SIZE);
    LOG_E("Error: %s\r\n", errstr);
}

/**
 * @brief Connect to server.
 *
 * @param hi
 * @param url
 * @return int Return 0 on success, others on failure.
 */
int http_open(HTTP_INFO *hi, char *url)
{
    int         sock_fd, is_https, verify;
    int         ret, opt;
    socklen_t   slen;

    char        *host = NULL, *path = NULL;
    char        port[FIELD_HTTP_HEADER_PORT_MAX_LEN] = {0};

    int         retCode = 0;

    host = (char *)hc_malloc(FIELD_URL_HOST_MAX_LEN);
    if (NULL == host) {
        retCode = -1;
        goto flag_exit;
    }

    path = (char *)hc_malloc(FIELD_URL_PATH_MAX_LEN);
    if (NULL == path) {
        retCode = -2;
        goto flag_release_host;
    }

    if (NULL == hi) return -1;

    verify = hi->tls.verify;

    if (parse_url(url, &is_https, host, port, path) != 0) {
        retCode = -3;
        goto flag_release_path;
    }

    if ((hi->tls.ssl_fd.fd == -1) || (hi->url->is_https != is_https) ||
        (strcmp(hi->url->host, host) != 0) || (strcmp(hi->url->port, port) != 0)) {

        if (hi->tls.ssl_fd.fd != -1)
            https_close(&hi);

        https_init(&hi, is_https, verify);

        if ((ret = https_connect(hi, host, port)) < 0) {
            https_close(&hi);
            _error = ret;
            return -1;
        }
    } else {
        sock_fd = hi->tls.ssl_fd.fd;
        slen = sizeof(int);
        if ((getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void *) &opt, &slen) < 0) || (opt > 0)) {

            https_close(&hi);
            https_init(&hi, is_https, verify);

            if ((ret = https_connect(hi, host, port)) < 0) {
                https_close(&hi);
                _error = ret;
                return -1;
            }
        } else {
         LOG_E("socket reuse: %d \r\n", sock_fd);
        }
    }

    strncpy(hi->url->host, host, strlen(host));
    strncpy(hi->url->port, port, strlen(port));
    strncpy(hi->url->path, path, strlen(path));

flag_release_path:
    hc_free(path);

flag_release_host:
    hc_free(host);

flag_exit:
    return retCode;
}

/**
 * @brief Construct header and send it.
 *
 * @param hi
 * @return int return 0 on success, -1 on failure.
 */
int http_write_header(HTTP_INFO *hi)
{
    char    *temp_write_buffer;
    int     ret, len;

    if (NULL == hi) return -1;

    temp_write_buffer = (char *)hc_malloc(FIELD_HTTP_HEADER_TOTAL_MAX_LEN);
    if (!temp_write_buffer) {
        LOG_EMSG("create @temp_write_buffer failed!!!\r\n");
        return -1;
    }

    /* Send HTTP request. */
    len = snprintf(temp_write_buffer, FIELD_HTTP_HEADER_TOTAL_MAX_LEN,
                   "%s %s HTTP/1.1\r\n"
                   "User-Agent: Mozilla/4.0\r\n"
                   "Host: %s:%s\r\n"
                   "Content-Type: %s\r\n",
                   hi->request->method, hi->url->path,
                   hi->url->host, hi->url->port,
                   hi->request->content_type);


    if(hi->request->referrer[0] != 0) {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Referer: %s\r\n", hi->request->referrer);
    }

    if(hi->request->chunked == TRUE) {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Transfer-Encoding: chunked\r\n");
    } else {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Content-Length: %ld\r\n", hi->request->content_length);
    }

    if(hi->request->close == TRUE) {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Connection: close\r\n");
    } else {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Connection: Keep-Alive\r\n");
    }

    if(hi->request->cookie[0] != 0) {
        len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len,
                        "Cookie: %s\r\n", hi->request->cookie);
    }

    len += snprintf(&temp_write_buffer[len], FIELD_HTTP_HEADER_TOTAL_MAX_LEN - len, "\r\n");

    LOG_D("HEADER: %s\r\n", temp_write_buffer);

    if ((ret = https_write(hi, temp_write_buffer, len)) != len) {
        https_close(&hi);
        _error = ret;
        return -1;
    }

    return 0;
}

/**
 * @brief Send data to server.
 *
 * @param hi
 * @param data
 * @param len
 * @return int return 0 on success, -1 on failure.
 */
int http_write(HTTP_INFO *hi, char *data, int len)
{
    char    prefixStr[INTERNAL_CHUNKED_PREFIX_STR_LEN] = {0};
    int     ret, prefixLen;

    if(NULL == hi || len <= 0) return -1;

    if ( TRUE == hi->request->chunked ) {
        // CHUNKED -- length of data first.
        prefixLen = snprintf(prefixStr, INTERNAL_CHUNKED_PREFIX_STR_LEN, "%x\r\n", len);

        if ( (ret = https_write(hi, prefixStr, prefixLen)) != len ) {
            https_close(&hi);
            _error = ret;
            return -1;
        }
    }

    if((ret = https_write(hi, data, len)) != len) {
        https_close(&hi);
        _error = ret;
        return -1;
    }

    if ( TRUE == hi->request->chunked ) {
        // CHUNKED -- append \r\n
        if ((ret = https_write(hi, "\r\n", 2)) != 2) {
            https_close(&hi);
            _error = ret;
            return -1;
        }
    }

    return len;
}

/**
 * @brief
 *
 * @param hi
 * @return int
 */
int http_write_end(HTTP_INFO *hi)
{
    int     ret = 0, len = 0;
    char    str[INTERNAL_CHUNKED_PREFIX_STR_LEN] = {0};

    if (NULL == hi) return -1;

    if(hi->request->chunked == TRUE) {
        // CHUNKED -- end with "0\r\n\r\n"
        len = snprintf(str, INTERNAL_CHUNKED_PREFIX_STR_LEN, "0\r\n\r\n");
    } else {
        len = snprintf(str, INTERNAL_CHUNKED_PREFIX_STR_LEN, "\r\n\r\n");
    }

    if ((ret = https_write(hi, str, len)) != len) {
        https_close(&hi);
        _error = ret;
        return -1;
    }

    return len;
}

/**
 * @brief Init HTTP_INFO object before reading operation.
 *
 * @param hi
 * @return int return 0 on success, -1 on failure.
 */
int  http_read_init(HTTP_INFO *hi)
{
    if (NULL == hi) return -1;
    hi->response->status_code = 0;
    hi->response->content_length = 0;
    hi->response->close = 0;
    hi->parse_ptr = NULL;
    hi->recv_buf_len = 0;
    hi->parse_state = HTTP_PARSE_HEADER;
    return 0;
}

/**
 * @brief Read data and save them in hi->recv_buf.
 *
 * @param hi
 * @return int return how many bytes (>0) are read on success, others on failure.
 */
int http_read(HTTP_INFO *hi)
{
    int ret = 0;

    if (NULL == hi) {
        return -1;
    }

    ret = https_read(hi, &hi->recv_buf[hi->recv_buf_len], (int) (HTTP_READ_ONE_TIME_MAX_LEN - hi->recv_buf_len));
    if (ret <= 0) {
        https_close(&hi);
        return ret;
    }

    hi->recv_buf_len += ret;
    hi->recv_buf[hi->recv_buf_len] = 0;
    hi->parse_ptr = hi->recv_buf;

    return ret;
}

void http_set_content_type(int _idx)
{
	if (_idx < 0 || _idx > 3)
	{
		return;
	}
	
	s_contentTypeIdx = _idx;
}
