/** @brief      AT socket command api.
 *
 *  @file       aiio_at_socket_api.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/03/01      <td>1.0.0       <td>wusen       <td>First version
 * 	<tr><td>2023/04/12      <td>1.0.1       <td>wusen       <td>add at socket command api
 *  <tr><td>2023/05/04      <td>1.0.2       <td>wusen       <td>add socket NV save & auto tt mode
 *  </table>
 *
 */

#ifndef __AIIO_AT_SOCKET_API_H__
#define __AIIO_AT_SOCKET_API_H__
#include <stdint.h>
#include "aiio_type.h"
#include "aiio_at_wifi_api.h"
#include "aiio_os_port.h"
#include "aiio_linux_llist.h"

#include <lwip/sockets.h>
#include "lwip/icmp.h"
#include "lwip/ip.h"

//socket
#include <sys/types.h>          /* See NOTES */
#include <errno.h>
#include <unistd.h>
//DNS
#include <netdb.h>

//mbedtls
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include <mbedtls/ssl.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/certs.h>
#include <mbedtls/platform.h>
#include <mbedtls/debug.h>
#include "lwip/inet_chksum.h"

#define AIIO_CONFIG_HAVE_SSL_SOCKET  1

//socket
#define AIIO_CONFIG_SOCKET_PACKAGE_SIZE (1024)
#define AIIO_CONFIG_SOCKET_TT_TIMEOUT       (20)    //unit:ms
#define AIIO_CONFIG_SOCKET_THREAD_SIZE  (1536*4 + AIIO_CONFIG_SOCKET_PACKAGE_SIZE)
#define AIIO_CONFIG_SOCKET_MAX_LISTINE_COUNT    (5)
#define AIIO_CONFIG_SOCKET_THREAD_PRIORITY  (2)
#define AIIO_CONFIG_SSL_CERT_BUF_SIZE   (2048)

//switch穿透消告警的宏定义
#ifndef __has_cpp_attribute
	#define __has_cpp_attribute(x) 0
#endif
#ifndef __has_c_attribute
	#define __has_c_attribute(x) 0
#endif
#ifndef __has_attribute
	#define __has_attribute(x) 0
#endif
#if __has_cpp_attribute(fallthrough) || __has_c_attribute(fallthrough) 
	#define FALLTHROUGH [[fallthrough]]
#elif __has_attribute(fallthrough)
	#define FALLTHROUGH __attribute__((fallthrough));
#else
	#define FALLTHROUGH
#endif

//Automatic transparent parameters
typedef struct{
    //Current mode
    //0：Disable automatic through-pass
    //1：UDPServer
    //2：UDPClient
    //3：TCPServer
    //4：TCPClient
    //5：TCPSeed(Placeholder type, unavailable)
    //6：SSLServer
    //7：SSLClient
    //8：SSLSeed(Placeholder type, unavailable)
    uint8_t mode;
    char host_name[128];
    int32_t port;
}aiio_socket_tt_config_t;

typedef enum{
    SOCKET_RECVMODE_PASSIVE=0,
    SOCKET_RECVMODE_ACTIVE=1,
}aiio_socket_recvmode_type_t;

//ssl certificate
typedef struct {
    char caCert[AIIO_CONFIG_SSL_CERT_BUF_SIZE]; //Server public key
    char ownCert[AIIO_CONFIG_SSL_CERT_BUF_SIZE];    //Client public key
    char ownPrivateCert[AIIO_CONFIG_SSL_CERT_BUF_SIZE]; //Client private key
}aiio_sslcert_t;

typedef struct{
    aiio_socket_recvmode_type_t socketRecvMode;
    aiio_socket_tt_config_t socketTTConfig;
    aiio_sslcert_t sslCert; //ssl certificate
}aiio_socket_save_config_t;

//ping information returned after the test is complete
typedef struct {
    uint32_t count; //ping time
    uint32_t lost;  //Number of packet loss
    uint32_t argvTimeMs;    //Mean ping time，unit:ms
    uint32_t maxTimeMs; //Max ping time，unit:ms
    uint32_t minTimeMs; //Min ping time，unit:ms
    char ip[16];    //ping address (Return the dotted IP address if it is a domain name)
}aiio_pingtest_info_t;

//SOCKET
typedef enum {
    AIIO_SOCKET_UDP_SERVER=1,
    AIIO_SOCKET_UDP_CLIENT=2,
    AIIO_SOCKET_TCP_SERVER=3,
    AIIO_SOCKET_TCP_CLIENT=4,
    AIIO_SOCKET_TCP_SEED=5,
#if AIIO_CONFIG_HAVE_SSL_SOCKET
    AIIO_SOCKET_SSL_SERVER=6,
    AIIO_SOCKET_SSL_CLIENT=7,
    AIIO_SOCKET_SSL_SEED=8,
#endif
}aiio_socket_type_t;

//The structure item that holds the data message received by the socket
typedef struct{
    uint8_t *data;  //Data received (this is just a pointer, space needs malloc, when used needs free)
    int32_t len;    //data len
    struct AI_list_head list;   //Kernel linked list node
}aiio_socket_mes_item_t;

//UDP server Select the passthrough client mode
typedef enum {
    AIIO_UDP_TT_USE_FIRST_NOT_INIT=0,   //The UDP server Passthrough object fixes the port used by the first received client data
                                        //The current state is the state that has not received the first data
                                        /*
                                        (For example, if the server receives data from the client for the first time from 192.168.1.100:8080,
                                            the passthrough object will remain 192.168.1.100:8080.)
                                        */
    AIIO_UDP_TT_USE_FIRST_INIT=1,       //AI_UDP_TT_USE_FIRST_NOT_INIT records the client information after receiving the data, and then switches to AI_UDP_TT_USE_FIRST_INIT
    AIIO_UDP_TT_AUTO=2,                 /*The UDP server transparent object is dynamically modified. The target object is the client that last received data
                                        (For example, if the client sends data to 192.168.1.100:8080, the server returns data to 192.168.1.100:8080;
                                            When 192.168.1.109:9090 is sent, the server returns data to 192.168.1.109:9090)
                                        */
}aiio_udp_tt_type_t;

#if AIIO_CONFIG_HAVE_SSL_SOCKET
typedef struct {
    mbedtls_net_context net;
    mbedtls_x509_crt ca_cert;
    mbedtls_x509_crt owncert;
    mbedtls_ssl_config conf;
    mbedtls_ssl_context ssl;
    mbedtls_pk_context pkey;
} aiio_ssl_param_t;
#endif

typedef struct{
    aiio_os_function_return_t threadFunc;
    char *threadName;
    void *userParam;
    uint32_t stackSize;
    uint8_t priority;
    aiio_os_thread_handle_t *pTaskHandle;
}aiio_thread_params_t;

//Structure to record socket connection information
typedef struct {
    aiio_at_connect_status_t connectStatus;
    aiio_socket_type_t socketType;
    char *remoteHost;
    int32_t remotePort;
    int32_t localPort;
    int32_t keeplive;
    int32_t sockfd;
    uint32_t conID;
    uint32_t parentConID;                   //The parent connection ID, valid only for the AIIO_SOCKET_TCP_SEED schema (other schemas default to 0),
                                            //indicates the connection (tcp server) from which the connection was obtained
    struct sockaddr_in socketObj;           //socket connection object
    aiio_udp_tt_type_t udpServerTTMode;     //This variable is valid only for udpserver transparent transmission.
                                            //This variable is used to control UDP server transparent transmission objects

    struct sockaddr_in udpServerReciveObj;  //This variable is valid only for udpserver transparent transmission.
                                            //This is used to record the client connection object that the upd server gets after receiving the data
#if AIIO_CONFIG_HAVE_SSL_SOCKET
    aiio_ssl_param_t sslClientParam;        //ssl Slave parameter
    mbedtls_ssl_context *context;           //ssl context handle
    //certificate and key for ssl server
    mbedtls_x509_crt *socketSslSrvCrt;      //server_x509
    mbedtls_pk_context *socketSslSrvKey;    //server_pk
    char *socketSslServerCrt;               // If the user has entered the user information, the user information will be used first
    char *socketSslServerCaList;            // If the user has entered the user information, the user information will be used first
    char *socketSslServerKey;               // If the user has entered the user information, the user information will be used first
#endif
    aiio_thread_params_t socketThreadParam;
    aiio_socket_mes_item_t msgListHead;     // The header of a linked list that holds the received data
    struct AI_list_head list;               // kernel list node
}aiio_socket_item_t;


typedef struct {
    uint8_t *buff;
    uint16_t length;
    aiio_socket_item_t p_sendSocket;
} aiio_at_socket_passthrough_t;

extern aiio_socket_item_t aiio_socketListHead;
extern aiio_socket_save_config_t  aiio_socket_save_config;

/** @brief 	transparent mode auto run
 *  @return				                       
 *  @retval         	
 *  @retval         	
 *  @note		
 *  @see				
 */
void aiio_socket_ttauto_run(void);

/** @brief  Enter transparent mode
 *  @param[in]          p_sendSocket            Send socket configuration parameter.
 *  @return             return AIIO_OK or AIIO_ERROR
 *  @retval             AIIO_OK:    The transparent mode is successful
 *  @retval             AIIO_ERROR: transparent mode failed
 *  @note
 *  @see
 */
CORE_API int32_t aiio_socket_tt_mode_enter(aiio_socket_item_t *p_sendSocket);

/** @brief  Pint test
 *  @param[in]          server              IP address (dotted string) or domain name to be pinged.
 *  @param[in]          count               Number of pings (0 means circular ping, not ending)
 *  @param[out]         result              This is the exit parameter, which returns the test information
 *  @return             return at error code
 *  @retval             0: The pingtest is successful
 *  @retval             !=0: pingtest failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API uint8_t aiio_ping_test(char *server,uint32_t count,aiio_pingtest_info_t *result);

/** @brief  Create a tcp client
 *  @param[in]          conid               The value 0 indicates automatic allocation, and the other values indicate the specified conID.
 *  @param[in]          url                 The IP address or domain name to connect to
 *  @param[in]          port                The number of the remote port to connect to
 *  @param[in]          keeplive            Keepalive time, (0 indicates disabled/not set, 1~7200 indicates detection interval, unit: second)
 *  @return             return at error code
 *  @retval             0: The Create is successful
 *  @retval             !=0: Create failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API int32_t aiio_create_tcp_client(uint32_t conid,char *url,int32_t port,int32_t keeplive);

/** @brief  Create a tcp server
 *  @param[in]          conid               The value 0 indicates automatic allocation, and the other values indicate the specified conID.
 *  @param[in]          port                The number of the local port to listen to
 *  @param[in]          keeplive            Keepalive time, (0 indicates disabled/not set, 1~7200 indicates detection interval, unit: second)
 *  @return             return at error code
 *  @retval             0: The Create is successful
 *  @retval             !=0: Create failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API int32_t aiio_create_tcp_server(uint32_t conid,int32_t port,int32_t keeplive);

/** @brief  Create a udp client
 *  @param[in]          conid               The value 0 indicates automatic allocation, and the other values indicate the specified conID.
 *  @param[in]          url                 The IP address or domain name to connect to
 *  @param[in]          port                The number of the remote port to connect to
 *  @return             return at error code
 *  @retval             0: The Create is successful
 *  @retval             !=0: Create failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API int32_t aiio_create_udp_client(uint32_t conid,char *url,int32_t port);

/** @brief  Create a udp server
 *  @param[in]          conid               The value 0 indicates automatic allocation, and the other values indicate the specified conID.
 *  @param[in]          port                The number of the local port to listen to
 *  @return             return at error code
 *  @retval             0: The Create is successful
 *  @retval             !=0: Create failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API int32_t aiio_create_udp_server(uint32_t conid,int32_t port);

/** @brief  Iterate over the current socket connection to determine if conID is occupied (note that this is thread-unsafe)
 *
 *  @param[in]          conId
 *  @return             return AIIO_OK or AIIO_ERROR
 *  @retval             AIIO_OK: The conid is be used
 *  @retval             AIIO_ERROR: The conid is not used
 *  @note
 *  @see
 */
CORE_API int32_t aiio_conid_is_notuse(uint32_t conId);

/** @brief  Traversal prints the current socket connection
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_show_all_socket(void);

/** @brief  Sends data to the specified socket
 *
 *  @param[in]          conID               Socket connection ID
 *  @param[in]          buff                send date buff
 *  @param[in]          len                 send date len
 *  @return             return at error code
 *  @retval             = 0:  send OK
 *  @retval             < 0:  send failed
 *  @note
 *  @see
 */
CORE_API int32_t aiio_socket_send(int32_t conID,uint8_t *buff,int32_t len);

/** @brief  Query connection information according to the conID
 *
 *  @param[in]          conID               The conID to be queried
 *  @return             The pointer to the queried node is successfully returned,return NULL on failure
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API aiio_socket_item_t* aiio_get_connect_by_conid(uint32_t conID);

/** @brief  Reads the received data from the specified socket
 *
 *  @param[in]          conID               Socket connection ID
 *  @return             return at error code
 *  @retval              =0:  read success
 *  @retval             !=0:  read failed
 *  @note
 *  @see                aiio_at_errcode.h
 */
CORE_API int32_t aiio_socket_read(int32_t conID);

/** @brief  socket monitor thread
 *  @param[in]          arg                 thread arg
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_socket_thread(void *arg);

/** @brief  The tcp client connection receives data processing functions
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_tcp_client_read_func(aiio_socket_item_t* p_node);

/** @brief  tcp server listens to threads
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_tcp_server_listen_func(aiio_socket_item_t* p_node);

/** @brief  A child thread created for seed when tcp receives a new connection
 *  @param[in]          arg                 thread arg
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_tcp_seed_thread(void *arg);

/** @brief  udp server listens to threads
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_udp_server_listen_func(aiio_socket_item_t* p_node);

/** @brief  The udp client connection receives data processing functions
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_udp_client_read_func(aiio_socket_item_t* p_node);

/** @brief  Create a ssl server
 *  @param[in]          port                The number of the local port to listen to
 *  @param[in]          keeplive            Keepalive time, (0 indicates disabled/not set, 1~7200 indicates detection interval, unit: second)
 *  @return
 *  @retval             >=0 :   The Create is successful  return connid
 *  @retval             <0  :   Create failed
 *  @note
 *  @see
 */
CORE_API int32_t aiio_create_ssl_server(int32_t port,int32_t keeplive);

/** @brief  Create a ssl client
 *  @param[in]          url                 The IP address or domain name to connect to
 *  @param[in]          port                The number of the remote port to connect to
 *  @param[in]          keeplive            Keepalive time, (0 indicates disabled/not set, 1~7200 indicates detection interval, unit: second)
 *  @return
 *  @retval             >=0 :   The Create is successful  return connid
 *  @retval             <0  :   Create failed
 *  @note
 *  @see
 */
CORE_API int32_t aiio_create_ssl_client(char *url,int32_t port,int32_t keeplive);

/** @brief  socket malloc function used by internal ssl calls
 *  @param[in]          nelements
 *  @param[in]          elementSize
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void* aiio_socket_ssl_malloc(size_t nelements, size_t elementSize);

/** @brief  socket ssl debug print function
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_socket_ssl_debug(void *ctx, int32_t level, const char *file, int32_t line, const char *str);

/** @brief  The ssl client connection receives data processing functions
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_ssl_client_read_func(aiio_socket_item_t* p_node);

/** @brief  ssl server listens to threads
 *  @param[in]          p_node                  socket connection information
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_ssl_server_listen_func(aiio_socket_item_t* p_node);

/** @brief  A child thread created for seed when ssl server receives a new connection
 *  @param[in]          arg                 thread arg
 *  @return
 *  @retval
 *  @retval
 *  @note
 *  @see
 */
CORE_API void aiio_ssl_seed_thread(void *arg);


#endif
