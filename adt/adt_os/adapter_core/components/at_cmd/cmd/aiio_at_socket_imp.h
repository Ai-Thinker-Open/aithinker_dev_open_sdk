#ifndef __AIIO_AT_SOCKET_IMP_H__
#define __AIIO_AT_SOCKET_IMP_H__

int32_t aiio_tcp_connect(const char *host, int32_t port, int32_t conn_timeout, int32_t keep_alive, int32_t *sockfd);
int32_t aiio_tcp_send(int32_t sockfd, uint8_t *buff, int32_t length);
int32_t aiio_tcp_send_blocking(int32_t sockfd, uint8_t *buff, int32_t length);
int32_t aiio_tcp_recv(int32_t sockfd, uint8_t *buff, int32_t length);
int32_t aiio_tcp_close(int32_t sockfd);

int32_t aiio_udp_create(const char *host, int32_t port, int32_t local_port, int32_t *sockfd, void **ctx);
int32_t aiio_udp_send(int32_t sockfd, uint8_t *buff, int32_t length, void *ctx);
int32_t aiio_udp_recv(int32_t sockfd, uint8_t *buff, int32_t length, void *ctx);
int32_t aiio_udp_close(int32_t sockfd, void *ctx);

int32_t aiio_ssl_connect(const char *host, int32_t port, int32_t conn_timeout, int32_t keep_alive, int32_t *sockfd, void **ctx);
int32_t aiio_ssl_send(void *ctx, uint8_t *buff, int32_t length);
int32_t aiio_ssl_recv(void *ctx, uint8_t *buff, int32_t length);
int32_t aiio_ssl_close(void *ctx);
int32_t aiio_ssl_cli_close(void *ctx);

int32_t aiio_tcp_server_create(int32_t port, int32_t *sockfd);
int32_t aiio_tcp_server_accept(int32_t sockfd, int32_t keep_alive, int32_t *accpet_fd, int32_t *remote_port, char *addr_name, int32_t name_size);
int32_t aiio_tcp_server_close(int32_t sockfd);

int32_t aiio_ssl_server_create(int32_t port, int32_t *sockfd, void **ctx);
int32_t aiio_ssl_server_refuse(void *ctx);
int32_t aiio_ssl_server_accept(void *ctx, int32_t *accpet_fd, void **accept_ctx, int32_t *remote_port, char *addr_name, int32_t name_size);
int32_t aiio_ssl_server_close(void *ctx);

#endif
