#ifndef  __LN_WIFI_ERR_H__
#define  __LN_WIFI_ERR_H__

typedef enum ln_wifi_err {
    WIFI_ERR_NONE              = 0u,

    WIFI_ERR_INVALID_PARAM     = 10u,
    WIFI_ERR_SIZE_EXCEEDED,
    WIFI_ERR_MEM,

    WIFI_ERR_WID_RSP           = 20u,
    WIFI_ERR_TIMEOUT,
    WIFI_ERR_STATE,
    WIFI_ERR_MODE,
 
    WIFI_ERR_OS_SERVICE        = 30u,

} ln_wifi_err_t;


#endif /* __LN_WIFI_ERR_H__ */

