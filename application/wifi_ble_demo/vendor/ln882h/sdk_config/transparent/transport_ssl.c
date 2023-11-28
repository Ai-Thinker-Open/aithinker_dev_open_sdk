#include "transport_ssl.h"
#include "hal/hal_flash.h"
#include "transparent.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "net/mbedtls/include/mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define mbedtls_time            time
#define mbedtls_time_t          time_t
#define mbedtls_fprintf         fprintf
#define mbedtls_printf          printf
#define mbedtls_exit            exit
#define MBEDTLS_EXIT_SUCCESS    EXIT_SUCCESS
#define MBEDTLS_EXIT_FAILURE    EXIT_FAILURE
#endif /* MBEDTLS_PLATFORM_C */

#if !defined(MBEDTLS_BIGNUM_C) || !defined(MBEDTLS_ENTROPY_C) ||  \
    !defined(MBEDTLS_SSL_TLS_C) || !defined(MBEDTLS_SSL_CLI_C) || \
    !defined(MBEDTLS_RSA_C) ||         \
    !defined(MBEDTLS_CERTS_C) || !defined(MBEDTLS_PEM_PARSE_C) || \
    !defined(MBEDTLS_CTR_DRBG_C) || !defined(MBEDTLS_X509_CRT_PARSE_C)
int transp_create_ssl_link( void *params )
{
    mbedtls_printf("MBEDTLS_BIGNUM_C and/or MBEDTLS_ENTROPY_C and/or "
           "MBEDTLS_SSL_TLS_C and/or MBEDTLS_SSL_CLI_C and/or "
           "MBEDTLS_NET_C and/or MBEDTLS_RSA_C and/or "
           "MBEDTLS_CTR_DRBG_C and/or MBEDTLS_X509_CRT_PARSE_C "
           "not defined.\n");
    while(1);
}
#else

#include "mbedtls/net_sockets.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"

#include <string.h>

int exit_code = MBEDTLS_EXIT_FAILURE;

mbedtls_net_context server_fd;
mbedtls_entropy_context entropy;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cacert;

mbedtls_x509_crt clicert;
mbedtls_pk_context pkey;

// less debug message if the level num is smaller.
#define DEBUG_LEVEL  0

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
    ((void) level);

    LOG(LOG_LVL_INFO, "%s:%04d: %s\r\n", file, line, str);
}

/**
 * @brief A demo using SSL to connect to HTTPS server.
 *
 * @param params
 */
int transp_create_ssl_link(int id)
{
    int ret = 1;

    char sPort[20] = {0};
	ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
	ln_netlink_handle_t pLink = &(p->tLinkItems[id]);
    
    uint32_t flags;
    const char *pers = pLink->peerName;

    lwip_itoa(sPort, 10, pLink->remotePort);
    
#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold( DEBUG_LEVEL );
#endif

    // while(LINK_UP != ethernetif_get_link_state()){
    //     OS_MsDelay(500);
    // }

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_net_init( &server_fd );
    mbedtls_ssl_init( &ssl );
    mbedtls_ssl_config_init( &conf );
    mbedtls_x509_crt_init( &cacert );
    mbedtls_ctr_drbg_init( &ctr_drbg );

    if (p->ssl_cert_pk_en)
    {
        mbedtls_x509_crt_init(&clicert);
        mbedtls_pk_init(&pkey);
    }

    mbedtls_printf( "\r\n  . Seeding the random number generator..." );

    mbedtls_entropy_init( &entropy );
    if( ( ret = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
                               (const unsigned char *) pers,
                               strlen( pers ) ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ctr_drbg_seed returned %d\r\n", ret );
        transp_delete_ssl_link(id);
        return ret;
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 0. Initialize certificates
     */
    mbedtls_printf( "  . Loading the CA root certificate ..." );

    unsigned char ssl_all_data[CERT_TOTAL_SIZE];
    unsigned char *ssl_data_ptr = ssl_all_data + CERT_BLOCK_SIZE * 2;
    unsigned int ssl_file_len = 0;

    hal_flash_read_by_cache(CERT_SSL_CA_PK_START, CERT_TOTAL_SIZE, ssl_all_data);

	ssl_file_len = *ssl_data_ptr | (*(ssl_data_ptr + 1) << 8);
	if((ssl_file_len != 0) && (ssl_file_len != 0xFFFF))
	{
		ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) (ssl_data_ptr + 2), ssl_file_len + 1 );
	}
	else
	{
		ret = mbedtls_x509_crt_parse( &cacert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len );
	}

    if( ret < 0 )
    {
        mbedtls_printf( " failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x\r\n", -ret );
        transp_delete_ssl_link(id);
        return ret;
    }
    mbedtls_printf( " ok (%d skipped)\r\n", ret );

    if (p->ssl_cert_pk_en)
    {
        ssl_data_ptr = ssl_all_data + CERT_BLOCK_SIZE;
        ssl_file_len = *ssl_data_ptr | (*(ssl_data_ptr + 1) << 8);
        mbedtls_printf("  . Loading the client cert. and key...");
        ret = mbedtls_x509_crt_parse(&clicert, (unsigned char const *)(ssl_data_ptr + 2), ssl_file_len + 1);
        if (ret != 0)
        {
            mbedtls_printf(" failed\r\n  !  mbedtls_x509_crt_parse returned -0x%x while parsing device cert\r\n", -ret);
            transp_delete_ssl_link(id);
            return ret;
        }

        ssl_data_ptr = ssl_all_data;
        ssl_file_len = *ssl_data_ptr | (*(ssl_data_ptr + 1) << 8);
        ret = mbedtls_pk_parse_key(&pkey, (unsigned char const *)(ssl_data_ptr + 2), ssl_file_len + 1, (unsigned char const *)"", 0);
        if (ret != 0)
        {
            mbedtls_printf(" failed\r\n  !  mbedtls_pk_parse_key returned -0x%x while parsing private key\r\n", -ret);
            transp_delete_ssl_link(id);
            return ret;
        }
        mbedtls_printf(" ok\n");
    }

    /*
     * 1. Start the connection
     */
    mbedtls_printf( "  . Connecting to tcp/%s/%s...", pLink->remoteIp, sPort );

    if( ( ret = mbedtls_net_connect( &server_fd, pLink->remoteIp,
                                         sPort, MBEDTLS_NET_PROTO_TCP ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_net_connect returned %d\r\n", ret );
        transp_delete_ssl_link(id);
        return ret;
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 2. Setup stuff
     */
    mbedtls_printf( "  . Setting up the SSL/TLS structure..." );

    if( ( ret = mbedtls_ssl_config_defaults( &conf,
                    MBEDTLS_SSL_IS_CLIENT,
                    MBEDTLS_SSL_TRANSPORT_STREAM,
                    MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ssl_config_defaults returned %d\r\n", ret );
        transp_delete_ssl_link(id);
        return ret;
    }
    mbedtls_printf( " ok\r\n" );

    /* OPTIONAL is not optimal for security,
     * but makes interop easier in this simplified example */
    mbedtls_ssl_conf_authmode( &conf, p->ssl_ca_en ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
    //mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_NONE );
    mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );

    if (p->ssl_cert_pk_en)
    {
        if ((ret = mbedtls_ssl_conf_own_cert(&conf, &(clicert), &(pkey))) != 0)
        {
            mbedtls_printf(" failed\n  ! mbedtls_ssl_conf_own_cert returned %d\r\n", ret);
            transp_delete_ssl_link(id);
			return ret;
        }
    }

    mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
    mbedtls_ssl_conf_dbg( &conf, my_debug, stdout );

    if( ( ret = mbedtls_ssl_setup( &ssl, &conf ) ) != 0 )
    {
        mbedtls_printf( " failed\r\n  ! mbedtls_ssl_setup returned %d\r\n", ret );
        transp_delete_ssl_link(id);
        return ret;
    }

    // if( ( ret = mbedtls_ssl_set_hostname( &ssl, "localhost" ) ) != 0 )
    // {
    //     mbedtls_printf( " failed\r\n  ! mbedtls_ssl_set_hostname returned %d\r\n", ret );
    //     transp_delete_ssl_link(id);
    //     return ret;
    // }

    mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

    /*
     * 4. Handshake
     */
    mbedtls_printf( "  . Performing the SSL/TLS handshake..." );

    while( ( ret = mbedtls_ssl_handshake( &ssl ) ) != 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\r\n  ! mbedtls_ssl_handshake returned -0x%x\r\n", -ret );
            transp_delete_ssl_link(id);
            return ret;
        }
    }

    mbedtls_printf( " ok\r\n" );

    /*
     * 5. Verify the server certificate
     */
    mbedtls_printf( "  . Verifying peer X.509 certificate..." );

    /* In real life, we probably want to bail out when ret != 0 */
    if( ( flags = mbedtls_ssl_get_verify_result( &ssl ) ) != 0 )
    {
        char vrfy_buf[512];

        mbedtls_printf( " failed\r\n" );

        mbedtls_x509_crt_verify_info( vrfy_buf, sizeof( vrfy_buf ), "  ! ", flags );

        mbedtls_printf( "%s\r\n", vrfy_buf );

        return ret;
    }
    else
        mbedtls_printf( " ok\r\n" );

    exit_code = MBEDTLS_EXIT_SUCCESS;

    return exit_code;
}

int ssl_send(const unsigned char* _buf, size_t _len)
{
    int len = 0;
	int ret = 0;

    mbedtls_printf( "  > Write to server:" );

    while( ( ret = mbedtls_ssl_write( &ssl, _buf, _len ) ) <= 0 )
    {
        if( ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
        {
            mbedtls_printf( " failed\r\n  ! mbedtls_ssl_write returned %d\r\n", ret );
            ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
            OS_QueueSend(&p->tryCloseQueue, &p->curId, 100);
            return ret;
        }
    }

    len = ret;
    mbedtls_printf( " %d bytes written\r\n%s", len, (char *) _buf );
	
	return len;
}

int ssl_rece(unsigned char *_buf, size_t _len)
{
    int ret = -1;

    memset(_buf, 0, _len * sizeof(unsigned char));
    ret = mbedtls_ssl_read(&ssl, _buf, _len);

    if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
        return 0;

    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
    {
        return 0;
    }

    if (ret == 0)
    {
        return 0;
    }

    mbedtls_printf("  < Read from server:");
    if (ret < 0)
    {
        mbedtls_printf("failed\r\n  ! mbedtls_ssl_read returned %d\n\n", ret);
        ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();
        OS_QueueSend(&p->tryCloseQueue, &p->curId, 100);
        return ret;
    }

    mbedtls_printf(" %d bytes read\r\n%s", ret, (char *)_buf);

    return ret;
}

void transp_delete_ssl_link(int id)
{
    ln_trans_sys_stat_handle_t p = ln_get_tcpip_handle();

    if (exit_code == MBEDTLS_EXIT_SUCCESS)
    {
        exit_code = MBEDTLS_EXIT_FAILURE;
        mbedtls_ssl_close_notify( &ssl );
    }
    
    mbedtls_net_free( &server_fd );
    mbedtls_x509_crt_free( &cacert );
    mbedtls_ssl_free( &ssl );
    mbedtls_ssl_config_free( &conf );
    mbedtls_ctr_drbg_free( &ctr_drbg );
    mbedtls_entropy_free( &entropy );

    if (p->ssl_cert_pk_en)
    {
        mbedtls_x509_crt_free(&clicert);
        mbedtls_pk_free(&pkey);
    }
    
    p->ssl_existed = false;
}
#endif /* MBEDTLS_BIGNUM_C && MBEDTLS_ENTROPY_C && MBEDTLS_SSL_TLS_C &&
          MBEDTLS_SSL_CLI_C && MBEDTLS_NET_C && MBEDTLS_RSA_C &&
          MBEDTLS_CERTS_C && MBEDTLS_PEM_PARSE_C && MBEDTLS_CTR_DRBG_C &&
          MBEDTLS_X509_CRT_PARSE_C */
