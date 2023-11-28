#include "network_transport.h"

#include <assert.h>
#include <string.h>

#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#include "debug/log.h"


static void aws_ada_debug(void *ctx, int level,
        const char *file, int line, const char *str)
{
    /* Unused parameters. */
    ( void ) ctx;
    ( void ) file;
    ( void ) line;

    /* Send the debug string to the portable logger. */
    LOG(LOG_LVL_INFO, "mbedTLS: |%d| %s", level, str );
}

static int aws_ada_tls_init(NetworkContext_t * nw_context)
{
    assert(nw_context != NULL);

    memset(&nw_context->tls, 0, sizeof(nw_context->tls));

    mbedtls_net_init(&nw_context->tls.ssl_fd);
    mbedtls_ssl_init(&nw_context->tls.ssl);
    mbedtls_ssl_config_init(&nw_context->tls.conf);
    mbedtls_x509_crt_init(&nw_context->tls.cacert);
    mbedtls_x509_crt_init(&nw_context->tls.clicert);
    mbedtls_ctr_drbg_init(&nw_context->tls.ctr_drbg);
    mbedtls_pk_init(&nw_context->tls.pkey);
    mbedtls_entropy_init( &nw_context->tls.entropy );

    nw_context->tls.inited = true;
    return 0;
}

static int aws_ada_tls_deinit(NetworkContext_t * nw_context)
{
    assert(nw_context != NULL);

    if (!nw_context->tls.inited) {
        return 0;
    }

    mbedtls_net_free(&nw_context->tls.ssl_fd);
    mbedtls_ssl_free(&nw_context->tls.ssl);
    mbedtls_ssl_config_free(&nw_context->tls.conf);
    mbedtls_x509_crt_free(&nw_context->tls.cacert);
    mbedtls_x509_crt_free(&nw_context->tls.clicert);
    mbedtls_ctr_drbg_free(&nw_context->tls.ctr_drbg);
    mbedtls_pk_free(&nw_context->tls.pkey);
    mbedtls_entropy_free(&nw_context->tls.entropy);

    nw_context->tls.inited = false;
    return 0;
}

static int aws_ada_tls_config(NetworkContext_t * nw_context)
{
    int ret = 0;

    ret = mbedtls_ssl_config_defaults(&nw_context->tls.conf,
                                    MBEDTLS_SSL_IS_CLIENT,
                                    MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
    if(ret != 0) {
        return ret;
    }

    nw_context->tls.certProfile = mbedtls_x509_crt_profile_default;

    if (nw_context->alpn_protos != NULL) {
        ret = mbedtls_ssl_conf_alpn_protocols(&nw_context->tls.conf, nw_context->alpn_protos);
        if(ret != 0) {
            LOG(LOG_LVL_ERROR, "Set ALPN failed!\r\n");
            return ret;
        }
    }

    mbedtls_ssl_conf_authmode(&nw_context->tls.conf,
                        MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_rng(&nw_context->tls.conf,
                        mbedtls_ctr_drbg_random, &nw_context->tls.ctr_drbg);
    mbedtls_ssl_conf_cert_profile(&nw_context->tls.conf,
                        &nw_context->tls.certProfile);
    mbedtls_ssl_conf_read_timeout(&nw_context->tls.conf, 5000);

    mbedtls_ssl_conf_dbg(&nw_context->tls.conf, aws_ada_debug, NULL);           /* set debug log output api */
    mbedtls_debug_set_threshold(AWS_ADA_MBEDTLS_DEBUG_LOG_LEVEL);               /* set log output level: 0 - 5 */

    ret = mbedtls_ctr_drbg_seed(&nw_context->tls.ctr_drbg,
                        mbedtls_entropy_func, &nw_context->tls.entropy, NULL, 0);
    if( ret != 0 ) {
        LOG(LOG_LVL_ERROR, "mbedtls_ctr_drbg_seed error, return -0x%x\r\n", -ret);
        return ret;
    }

    ret = mbedtls_x509_crt_parse(&nw_context->tls.cacert,
                        (const unsigned char *)nw_context->server_root_ca_pem,
                        nw_context->server_root_ca_pem_size);
    if( ret != 0 ) {
        LOG(LOG_LVL_ERROR, "parse root ca cert error, return -0x%x\r\n", -ret);
        return ret;
    }

    mbedtls_ssl_conf_ca_chain(&nw_context->tls.conf,
                        &nw_context->tls.cacert, NULL);

    ret = mbedtls_x509_crt_parse(&nw_context->tls.clicert,
                        (const unsigned char *)nw_context->client_cert_pem,
                        nw_context->client_cert_pem_size);
    if(ret != 0) {
        LOG(LOG_LVL_ERROR, "parse client cert error, return -0x%x\r\n", -ret);
        return ret;
    }

    ret = mbedtls_pk_parse_key(&nw_context->tls.pkey,
                        (const unsigned char *)nw_context->client_key_pem,
                        nw_context->client_key_pem_size,
                        "", 0);
    if (ret != 0) {
        LOG(LOG_LVL_ERROR, "parse client key error, return -0x%x\r\n", -ret);
        return ret;
    }

    (void)mbedtls_ssl_conf_own_cert(&(nw_context->tls.conf),
                        &(nw_context->tls.clicert),
                        &(nw_context->tls.pkey));

    if ((ret = mbedtls_ssl_set_hostname(&nw_context->tls.ssl,
                        (const char *)nw_context->hostname)) != 0) {
        LOG(LOG_LVL_ERROR, "set hostname error, return -0x%x\r\n", -ret);
        return ret;
    }

    ret = mbedtls_ssl_setup(&nw_context->tls.ssl, &nw_context->tls.conf);
    if( ret != 0 ) {
        LOG(LOG_LVL_ERROR, "ssl setup error, return -0x%x\r\n", -ret);
        return ret;
    }

    mbedtls_ssl_set_bio(&nw_context->tls.ssl, &nw_context->tls.ssl_fd,
                        mbedtls_net_send, mbedtls_net_recv,
                        mbedtls_net_recv_timeout);
    return 0;
}

aws_ada_tls_status_e aws_ada_tls_conn(NetworkContext_t *nw_context)
{
    int ret = 0;
    aws_ada_tls_status_e status = AWS_ADA_TLS_STATUS_SUCCESS;
    char port_string[6] = {0};

    if (nw_context == NULL) {
        return AWS_ADA_TLS_STATUS_INVALID_PARAM;
    }

    ret = aws_ada_tls_init(nw_context);
    if (ret != 0) {
        return AWS_ADA_TLS_STATUS_INIT_FAIL;
    }

    ret = aws_ada_tls_config(nw_context);
    if (ret != 0) {
        status = AWS_ADA_TLS_STATUS_CONFIG_FAIL;
        goto __exit;
    }

    snprintf(port_string, sizeof(port_string), "%u", nw_context->port);
    LOG(LOG_LVL_INFO, "-- tls conn <%s:%s> --\r\n",
                    nw_context->hostname, port_string);

    ret = mbedtls_net_connect(&nw_context->tls.ssl_fd,
                    nw_context->hostname,
                    (const char *)port_string,
                    MBEDTLS_NET_PROTO_TCP);
    if(ret != 0) {
        LOG(LOG_LVL_ERROR, "Failed to conn <%s:%s>, return -0x%x\r\n",
                    nw_context->hostname, port_string, -ret);
        status = AWS_ADA_TLS_STATUS_CONN_FAIL;
        goto __exit;
    }

    do {
        ret = mbedtls_ssl_handshake(&nw_context->tls.ssl);
    } while(ret == MBEDTLS_ERR_SSL_WANT_READ ||
            ret == MBEDTLS_ERR_SSL_WANT_WRITE);

    if ((ret != 0) ||
        (mbedtls_ssl_get_verify_result(&nw_context->tls.ssl) != 0)) {
        LOG(LOG_LVL_ERROR, "TLS handshake failed! return -0x%x\r\n", -ret);
        status = AWS_ADA_TLS_HANDSHAKE_ERROR;
        goto __exit;
    }
    LOG(LOG_LVL_INFO, "TLS conn to <%s> success!\r\n", nw_context->hostname);

    return AWS_ADA_TLS_STATUS_SUCCESS;
__exit:
    aws_ada_tls_deinit(nw_context);
    return status;
}

int aws_ada_tls_disconn(NetworkContext_t *nw_context)
{
    int ret = 0;

    ret = mbedtls_ssl_close_notify(&nw_context->tls.ssl);
    if (ret == 0) {
        LOG(LOG_LVL_INFO, "Closing TLS connection: TLS close-notify sent.\r\n");
    } else if ((ret == MBEDTLS_ERR_SSL_WANT_READ) &&
                (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        /* WANT_READ and WANT_WRITE can be ignored. Logging for debugging purposes. */
        LOG(LOG_LVL_INFO,
                "TLS close-notify sent; received %s as the TLS status "
                "which can be ignored for close-notify.\r\n",
                (ret == MBEDTLS_ERR_SSL_WANT_READ ) ? "WANT_READ" : "WANT_WRITE");
    } else {
        /* Ignore the WANT_READ and WANT_WRITE return values. */
        LOG(LOG_LVL_ERROR,
                "Failed to send TLS close-notify: mbedTLSError= -%x.\r\n", ret);
    }

    aws_ada_tls_deinit(nw_context);
    return ret;
}

int32_t aws_ada_tls_send(NetworkContext_t *nw_context, const void *data, size_t data_len)
{
    int32_t ret = 0;
    assert(data != NULL);

    ret = (int32_t)mbedtls_ssl_write(&nw_context->tls.ssl, data, data_len);
    if ((ret == MBEDTLS_ERR_SSL_TIMEOUT) ||
        (ret == MBEDTLS_ERR_SSL_WANT_READ) ||
        (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        ret = 0;
    } else if (ret < 0) {
        LOG(LOG_LVL_ERROR, "mbedtls_client_write data error, return -0x%x\r\n", -ret);
    }

    return ret;
}

int32_t aws_ada_tls_recv(NetworkContext_t *nw_context, void *data, size_t data_len)
{
    int32_t ret = 0;
    assert(data != NULL);

    ret = (int32_t)mbedtls_ssl_read(&nw_context->tls.ssl, data, data_len);
    if ((ret == MBEDTLS_ERR_SSL_TIMEOUT) ||
        (ret == MBEDTLS_ERR_SSL_WANT_READ) ||
        (ret == MBEDTLS_ERR_SSL_WANT_WRITE)) {
        ret = 0;
    } else if (ret < 0) {
        LOG(LOG_LVL_ERROR, "mbedtls_client_read data error, return -0x%x\r\n", -ret);
    }

    return ret;
}
