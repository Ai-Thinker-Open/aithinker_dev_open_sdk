#ifndef WIFI_CONFIG_COMM_SERVICE_H__
#define WIFI_CONFIG_COMM_SERVICE_H__

#include "aiio_adapter_include.h"

typedef enum
{
    CMD_SSID_PW_TOKEN = 1, /* SSID/PW/CHAN for softAP */
    CMD_DEVICE_REPLY = 2,  /* device reply */
    CMD_LOG_QUERY = 3,     /* app query log */
} WiFiConfigCmd_t;

typedef struct
{
    int sockfd;           /*!< client socket */
    char *source_address; /*!< Client IP */
    uint16_t port;        /*!< client port */
} udp_socket_client_t;

#define SOFTAP_BOARDING_VERSION "1.0.0"

CHIP_API aiio_ret_t aiio_decrypt_app_data(uint8_t *recv_data, int len, uint8_t *decrypted_data, int *out_len);
CHIP_API aiio_ret_t aiio_encryption_app_data(uint8_t *recv_data, int len, uint8_t *encryption_data, int *out_len);

CHIP_API aiio_ret_t udp_radiate_result(void);
CHIP_API uint32_t aiio_udp_start(void *pvParameters);
CHIP_API aiio_ret_t aiio_udp_write(udp_socket_client_t *udp_client, const uint8_t *write_data, uint16_t write_len);

#endif // !WIFI_CONFIG_COMM_SERVICE_H__
