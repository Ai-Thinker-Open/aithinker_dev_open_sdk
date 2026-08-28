#ifndef _AXK_OTA_H_
#define _AXK_OTA_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*f_write)(int dst_offset, const void *src,int size);
// typedef void (*f_read)(void *stream, int len, void *buf);
typedef int (*f_erase)(int start_addr, int size);
typedef void (*f_partition)(void);
typedef void (*f_reboot)(_Bool ota_result);

typedef struct axk_remote_fw_info
{
    uint8_t md5[33];
    uint8_t url[128];
} axk_remote_fw_info_t;

typedef struct
{
    uint32_t status_code;
    uint32_t header_len;
    uint8_t *body;
    uint32_t body_len;
    uint8_t *header_bak;
    uint32_t parse_status;
} axk_http_response_result;

typedef struct
{
    char *host;
    int port;
    char *resoure;
    f_write write_cb;                  // 写flash回调
    f_erase erase_cb;                  // 擦除flash回调
    f_partition set_boot_partition_cb; // 设置boot启动分区回调
    f_reboot rebooot_cb;               // 重启回调
} ota_parame;

typedef struct axk_ota
{
    int32_t (*remote_data_parse)(uint8_t *data, uint16_t len);
} axk_ota_t;

extern axk_ota_t g_axk_ota;

#endif //_AXK_OTA_H_
