#ifndef __AIIO_SMARTLIVING_DEMO_H_
#define __AIIO_SMARTLIVING_DEMO_H_

#include "aiio_adapter_include.h"

#define KV_MQTT_URL_KEY "mqtt_url"
#define KV_REGION_ID_KEY "regionid"
#define GUIDER_PREAUTH_URL_FMT "https://%s/auth/bootstrap"
#define GUIDER_PREAUTH_GET_URL_FMT "https://%s/auth/bootstrap/regionId/%d"
#define GUIDER_DEFAULT_TS_STR "2524608000000"
#define LIVING_SDK_VERSION  "1.6.6-20"
#define LINKKIT_VERSION     "2.3.0" "_FY_" LIVING_SDK_VERSION
#define SHA_METHOD "hmacsha1"
#define MD5_METHOD "hmacmd5"
#define KV_REGION_ID_VALUE_LEN (6)
#define GUIDER_MAX_DIRECT_CONNECT_COUNT (3)
#define GUIDER_BOOTSTRAP_DONE (1688)
#define GUIDER_TS_LEN (16)
#define GUIDER_PREAUTH_REQUEST_LEN (256)
#define HOST_ADDRESS_LEN    (128)
#define RANDOM_MAX_LEN  (16)
#define RANDOM_STR_MAX_LEN  (RANDOM_MAX_LEN * 2 + 1)
#define GUIDER_RESET_AND_TOKEN_LEN (64)
#define GUIDER_SIGN_LEN (48)
#define GUIDER_SIGN_SOURCE_LEN (256)
#define CLIENT_ID_LEN       (384)   /* Enlarge this buffer size due to add token params etc */
#define CUSTOME_DOMAIN_LEN_MAX (60)
#define GUIDER_PREAUTH_RESPONSE_LEN (512)
#define MEM_MAGIC                       (0x1234)
#define DEVICE_ID_LEN       (64)

typedef struct {
    int domain_type;
    int mqtt_port_num;
    int env;
    int dynamic_register;
#ifdef REPORT_UUID_ENABLE
    int uuid_enabled;
#endif
} sdk_impl_ctx_t;

typedef struct {
    uint16_t        port;
    uint8_t         init;
    char            *host_name;
    char            *client_id;
    char            *username;
    char            *password;
    const char      *pub_key;

} iotx_conn_info_t, *iotx_conn_info_pt;

typedef enum _secure_mode_e
{
    MODE_TLS_GUIDER = -1,
    MODE_TCP_GUIDER_PLAIN = 0,
    MODE_TCP_GUIDER_ID2_ENCRYPT = 1,
    MODE_TLS_DIRECT = 2,
    MODE_TCP_DIRECT_PLAIN = 3,
    MODE_TCP_DIRECT_ID2_ENCRYPT = 4,
    MODE_TLS_GUIDER_ID2_ENCRYPT = 5,
    MODE_TLS_DIRECT_ID2_ENCRYPT = 7,
    MODE_ITLS_DNS_ID2 = 8,
} secure_mode_e;

typedef enum _connect_method_e
{
    CONNECT_PREAUTH = 0,
    CONNECT_DIRECT,
    CONNECT_METHOD_MAX
} connect_method_e;

typedef enum _guider_env_e
{
    GUIDER_ENV_DAILY = 1,
    GUIDER_ENV_PRERELEASE,
    GUIDER_ENV_ONLINE,
    GUIDER_ENV_MAX
} guider_env_e;

/* domain type define */
typedef enum _DOMAIN_TYPE {
    GUIDER_DOMAIN_MQTT,
    GUIDER_DOMAIN_HTTP,
    GUIDER_DOMAIN_DYNAMIC_REGISTER_HTTP,
    GUIDER_DOMAIN_MAX
} domain_type_t;

/* region type */
typedef enum IOTX_CLOUD_REGION_TYPES {
    /* Shanghai */
    IOTX_CLOUD_REGION_SHANGHAI,

    /* Singapore */
    IOTX_CLOUD_REGION_SINGAPORE,

    /* Japan */
    IOTX_CLOUD_REGION_JAPAN,

    /* America east*/
    IOTX_CLOUD_REGION_USA_EAST,

    /* Germany */
    IOTX_CLOUD_REGION_GERMANY,

    /* America west*/
    IOTX_CLOUD_REGION_USA_WEST,

    /* Out-of-bounds read from a buffer */
    /*Define the valid maximum region id is 19999*/
    IOTX_CLOUD_REGION_MAX = 19999,

    /* Custom setting */
    IOTX_CLOUD_REGION_CUSTOM = 20000,

    /* Maximum number of custom region */
    IOTX_CLOUD_CUSTOM_REGION_MAX
} iotx_cloud_region_types_t;

typedef enum {
    IOTX_IOCTL_SET_REGION,              /* value(int*): iotx_cloud_region_types_t */
    IOTX_IOCTL_GET_REGION,              /* value(int*) */
    IOTX_IOCTL_SET_MQTT_DOMAIN,         /* value(const char*): point to mqtt domain string */
    IOTX_IOCTL_SET_MQTT_PORT,           /* value(int*): point to mqtt port number*/
    IOTX_IOCTL_SET_ENV,                 /* value(int*): 0 - env is ONLINE; 1 - env is PRE; 2 - env is DAILY*/
    IOTX_IOCTL_SET_HTTP_DOMAIN,         /* value(const char*): point to http domain string */
    IOTX_IOCTL_SET_DYNAMIC_REGISTER,    /* value(int*): 0 - Disable Dynamic Register, 1 - Enable Dynamic Register */
    IOTX_IOCTL_GET_DYNAMIC_REGISTER,    /* value(int*) */
    IOTX_IOCTL_RECV_PROP_REPLY,         /* value(int*): 0 - Disable property post reply by cloud; 1 - Enable property post reply by cloud */
    IOTX_IOCTL_RECV_EVENT_REPLY,        /* value(int*): 0 - Disable event post reply by cloud; 1 - Enable event post reply by cloud */
    IOTX_IOCTL_SEND_PROP_SET_REPLY,     /* value(int*): 0 - Disable send post set reply by devid; 1 - Enable property set reply by devid */
    IOTX_IOCTL_SET_SUBDEV_SIGN,         /* value(const char*): only for slave device, set signature of subdevice */
    IOTX_IOCTL_GET_SUBDEV_LOGIN,        /* value(int*): 0 - SubDev is logout; 1 - SubDev is login */
    IOTX_IOCTL_QUERY_DEVID,             /* value(iotx_linkkit_dev_meta_info_t*): device meta info, only productKey and deviceName is required, ret value is subdev_id or -1 */
    IOTX_IOCTL_SEND_EVENT_NOTIFY_REPLY  /* value(int*): 0 - Disable send post set reply by devid; 1 - Enable event notify reply by devid */
} iotx_ioctl_option_t;

typedef enum {
    IOTX_VENDOR_DEV_RESET_TYPE_UNBIND_ONLY         = 0,
    IOTX_VENDOR_DEV_RESET_TYPE_UNBIND_SHADOW_CLEAR = 1,
    IOTX_VENDOR_DEV_RESET_TYPE_UNBIND_ALL_CLEAR    = 2,
	IOTX_VENDOR_DEV_RESET_TYPE_INVALID
} iotx_vendor_dev_reset_type_t;

typedef enum _ext_params_e
{
    EXT_PLAIN_ROUTE = 0,
    EXT_SMART_ROUTE,
    EXT_PARAM_MAX
} ext_params_e;

typedef enum {
    TOKEN_TYPE_NOT_CLOUD = 0x00,
    TOKEN_TYPE_CLOUD,
    TOKEN_TYPE_MAX,
    TOKEN_TYPE_INVALID = 0xFF
} bind_token_type_t;

/** @defgroup httpclient_enum Enum
 *  @{
 */
/** @brief   This enumeration defines the HTTP request type.  */
typedef enum {
    HTTPCLIENT_GET,
    HTTPCLIENT_POST,
    HTTPCLIENT_PUT,
    HTTPCLIENT_DELETE,
    HTTPCLIENT_HEAD
} HTTPCLIENT_REQUEST_TYPE;

#define IOTX_CLOUD_REGION_INVALID (-100)

typedef enum _region_type_e
{
    REGION_TYPE_ID = 0,
    REGION_TYPE_MQTTURL,
    REGION_TYPE_MAX
} region_type_e;

int IOT_SetupConnInfo(void **info_ptr);
int iotx_guider_authenticate(iotx_conn_info_t *conn, void (*cb)(void *), void *client);
const char *iotx_ca_get(void);
int awss_check_reset(iotx_vendor_dev_reset_type_t *reset_type);
int awss_get_token(uint8_t token_buf[], int token_buf_len, bind_token_type_t *p_token_type);
int HAL_GetPartnerID(char *pid_str);
int HAL_GetModuleID(char *mid_str);
int iotx_guider_set_dynamic_region(int region);
int awss_set_token(uint8_t token[RANDOM_MAX_LEN], bind_token_type_t token_type);
int awss_get_token(uint8_t token_buf[], int token_buf_len, bind_token_type_t *p_token_type);

#endif