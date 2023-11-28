
#ifndef __NETWORK_TRANSPORT_H__
#define __NETWORK_TRANSPORT_H__

#include <stdint.h>
#include <stdbool.h>
#include "transport_interface.h"

#include "mbedtls/net.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509_crt.h"

typedef enum
{
    AWS_ADA_TLS_STATUS_SUCCESS = 0,
    AWS_ADA_TLS_STATUS_INVALID_PARAM = -2,
    AWS_ADA_TLS_STATUS_INIT_FAIL,
    AWS_ADA_TLS_STATUS_CONFIG_FAIL,
    AWS_ADA_TLS_STATUS_CONN_FAIL,
    AWS_ADA_TLS_STATUS_ERROR,
    AWS_ADA_TLS_STATUS_X509_CERT_ERROR,
    AWS_ADA_TLS_STATUS_X509_CERT_VERIFY_ERROR,
    AWS_ADA_TLS_HANDSHAKE_ERROR,
    AWS_ADA_TLS_STATUS_UNKNOWN
} aws_ada_tls_status_e;

typedef struct
{
    bool                        inited;
    mbedtls_net_context         ssl_fd;
    mbedtls_entropy_context     entropy;
    mbedtls_ctr_drbg_context    ctr_drbg;
    mbedtls_ssl_context         ssl;
    mbedtls_ssl_config          conf;
    mbedtls_x509_crt            cacert;
    mbedtls_x509_crt            clicert;
    mbedtls_pk_context          pkey;
    mbedtls_x509_crt_profile    certProfile;
} aws_ada_tls_context;

/**
 * NetworkContext_t: struct NetworkContext
 * NetworkContext_t is defined in "transport_interface.h" .
*/
struct NetworkContext
{
    int port;
    int server_root_ca_pem_size;
    int client_cert_pem_size;
    int client_key_pem_size;
    bool use_secure_element;
    const char *hostname;
    const char *server_root_ca_pem;
    const char *client_cert_pem;
    const char *client_key_pem;
    const char **alpn_protos;
    aws_ada_tls_context tls;
};

#define AWS_ADA_MBEDTLS_DEBUG_LOG_LEVEL (0) // default 0, 0 - 5

aws_ada_tls_status_e aws_ada_tls_conn(NetworkContext_t *nw_context);
int aws_ada_tls_disconn(NetworkContext_t *nw_context);
int32_t aws_ada_tls_send(NetworkContext_t *nw_context, const void *data, size_t data_len);
int32_t aws_ada_tls_recv(NetworkContext_t *nw_context, void *data, size_t data_len);

#endif /* __NETWORK_TRANSPORT_H__ */
