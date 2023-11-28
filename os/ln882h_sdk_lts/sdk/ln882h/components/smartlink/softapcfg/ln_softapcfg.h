#ifndef __LN_SOFTAPCFG_H__
#define __LN_SOFTAPCFG_H__

#include "ln_types.h"


#define WLAN_SSID_MAX_LEN           (32)//rang:1-32
#define WLAN_PWD_MAX_LEN            (64)//rang:8-63ascii 8-64hex
#define TOKEN_MAX_LEN               (48)

#define RECV_JSON_DATA_LEN_MIN      (74)
#define RECV_JSON_DATA_LEN_MAX      (255)
#define RXTX_BUF_SIZE               (256)

#define SOFTAP_LOCAL_PORT           (8266)


typedef struct {
    const char * product_id;
    const char * device_name;
    const char * proto_ver;
} product_param_t;

typedef struct {
    uint8_t * ssid;
    uint8_t * pwd;
    uint8_t * token;
    uint8_t   is_complete;
    uint8_t   stop;
} softapcfg_ctl_t;

#endif /* __LN_SOFTAPCFG_H__ */
