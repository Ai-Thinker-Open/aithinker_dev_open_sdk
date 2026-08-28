#include "aiio_adapter_include.h"
#include "aiio_smartliving_demo.h"
#include "aiio_common.h"

#define AWSS_KV_RST                "awss.rst"
#define AWSS_KV_RST_TYPE           "awss.rst.type"
uint8_t g_aes_random[RANDOM_MAX_LEN] = {0};
uint8_t g_token_type = TOKEN_TYPE_INVALID;

iotx_vendor_dev_reset_type_t g_reset_type = IOTX_VENDOR_DEV_RESET_TYPE_UNBIND_SHADOW_CLEAR;
static uint8_t awss_report_reset_suc = 0;

int HAL_GetPartnerID(char *pid_str)
{
    memset(pid_str, 0x0, PID_STRLEN_MAX);
    strcpy(pid_str, "Ai-Thinker");
    return strlen(pid_str);
}

int HAL_GetModuleID(char *mid_str)
{
    memset(mid_str, 0x0, MID_STRLEN_MAX);
    strcpy(mid_str, "BL602");
    return strlen(mid_str);
}

#ifdef SUPPORT_TLS
static const char *iotx_ca_crt = \
{
    \
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\r\n" \
    "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\r\n" \
    "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\r\n" \
    "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\r\n" \
    "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\r\n" \
    "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\r\n" \
    "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\r\n" \
    "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\r\n" \
    "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\r\n" \
    "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\r\n" \
    "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\r\n" \
    "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\r\n" \
    "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\r\n" \
    "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\r\n" \
    "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\r\n" \
    "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\r\n" \
    "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\r\n" \
    "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\r\n" \
    "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\r\n" \
    "-----END CERTIFICATE-----"
};
#endif  /* #ifdef SUPPORT_TLS */

const char *iotx_ca_get(void)
{
#ifdef SUPPORT_TLS
    return iotx_ca_crt;
#else
    return NULL;
#endif
}

int awss_check_reset(iotx_vendor_dev_reset_type_t *reset_type)
{
    int len = 1;
    char rst = 0;

    int ret = AilinkFlashRead(AWSS_KV_RST, (uint8_t *)&rst, &len);

    if (rst != 0x01)
    { // reset flag is not set
        return 0;
    }

    len = sizeof(iotx_vendor_dev_reset_type_t);
    ret = AilinkFlashRead(AWSS_KV_RST_TYPE, &g_reset_type, &len);
    if (ret != 0 || len == 0)
    {
        aiio_log_e( "no rst type in kv");
        g_reset_type = IOTX_VENDOR_DEV_RESET_TYPE_UNBIND_SHADOW_CLEAR;
    }

    aiio_log_d("need report rst,type=%d", g_reset_type);
    awss_report_reset_suc = 0;

    if (reset_type)
    {
        *reset_type = g_reset_type;
    }

    return 1;
}

void LITE_hexbuf_convert(unsigned char *digest, char *out, int in_len, int uppercase)
{
    static char    *zEncode[] = {"0123456789abcdef", "0123456789ABCDEF"};
    int             j = 0;
    int             i = 0;
    int             idx = uppercase ? 1 : 0;

    for (i = 0; i < in_len; i ++) {
        int         a = digest[i];

        out[j++] = zEncode[idx][(a >> 4) & 0xf];
        out[j++] = zEncode[idx][a & 0xf];
    }
}

#define utils_hex_to_str(buf, buf_len, str, str_buf_len) LITE_hexbuf_convert(buf, str, buf_len, 1)

uint64_t HAL_GetTimeMs(void) 
{
    uint32_t tickCount = xTaskGetTickCount();  // 获取当前 tick 数  
    uint32_t tickDuration = configTICK_RATE_HZ / 1000;  // 每个 tick 对应的毫秒数  
    return tickCount * tickDuration;  // 返回毫秒值  
}

/* Common with mrand48.c, lrand48.c */
extern unsigned short __rand48_seed[3];

void srand48(long seedval)
{
	__rand48_seed[0] = 0x330e;
	__rand48_seed[1] = (unsigned short)seedval;
	__rand48_seed[2] = (unsigned short)((uint32_t) seedval >> 16);
}

// void srand(unsigned int __s)
// {
// 	srand48(__s);
// }

void srandom(unsigned int __s)
{
	srand48(__s);
}

void HAL_Srandom(uint32_t seed)
{
    srandom(seed);
}

unsigned short __rand48_seed[3];	/* Common with mrand48.c, srand48.c */

long lrand48(void)
{
	return (uint32_t) jrand48(__rand48_seed) >> 1;
}

// int rand(void)
// {
// 	return (int)lrand48();
// }

long random(void)
{
	return lrand48();
}

uint32_t HAL_Random(uint32_t region)
{
    return (region > 0) ? (random() % region) : 0;
}

void produce_random(uint8_t *random, uint32_t len)
{
    int i = 0;
    int time = HAL_GetTimeMs();
    HAL_Srandom(time);
    for (i = 0; i < len; i ++) {
        random[i] = HAL_Random(0xFF);
    }
    // random produced by device, set bind token type to device token
    g_token_type = TOKEN_TYPE_NOT_CLOUD;
}

int awss_set_token(uint8_t token[RANDOM_MAX_LEN], bind_token_type_t token_type)
{
    char rand_str[RANDOM_MAX_LEN * 2 + 1] = {0};
    if ((token == NULL) || (token_type >= TOKEN_TYPE_MAX)) {
        return -1;
    }

    memcpy(g_aes_random, token, RANDOM_MAX_LEN);
    g_token_type = token_type;
    utils_hex_to_str(g_aes_random, RANDOM_MAX_LEN, rand_str, sizeof(rand_str));
    // dump_dev_bind_status(STATE_BIND_SET_APP_TOKEN, "bind: app token set (%d):%s", g_token_type, rand_str);
    return 0;
}

int awss_get_token(uint8_t token_buf[], int token_buf_len, bind_token_type_t *p_token_type)
{
    int i = 0;
    char token_str[RANDOM_STR_MAX_LEN] = {0};

    if (!token_buf || token_buf_len < RANDOM_STR_MAX_LEN) 
    {
        return -1;
    }

    for (i = 0; i < sizeof(g_aes_random); i ++)  // check g_aes_random is initialed or not
    {
        if (g_aes_random[i] != 0x00) {
            break;
        }
    }

    if (i >= sizeof(g_aes_random)) { // g_aes_random needs to be initialed
        produce_random(g_aes_random, sizeof(g_aes_random));
        aiio_log_d("produce random:");
        #ifndef CONFIG_BLDTIME_MUTE_DBGLOG
        // _dump_hex((uint8_t *)g_aes_random, RANDOM_MAX_LEN, 24);
        #endif
    }

    utils_hex_to_str(g_aes_random, RANDOM_MAX_LEN, token_str, sizeof(token_str));
    memcpy(token_buf, token_str, RANDOM_STR_MAX_LEN);
    *p_token_type = g_token_type;

    return 0;
}

// char *LITE_json_value_of(char *key, char *src, ...)
// {
//     char       *value = NULL;
//     char       *ret = NULL;
//     char       *delim = NULL;
//     char       *key_iter;
//     char       *key_next;
//     char       *src_iter;
//     char       *module_name = NULL;

//     int         key_len;
//     int         value_len = -1;
//     int         src_iter_len;
//     int         magic = 0;

//     if (NULL == key || NULL == src) {
//         return NULL;
//     }

// #if WITH_MEM_STATS_PER_MODULE
//     va_list     ap;
//     va_start(ap, src);
//     magic = va_arg(ap, int);
//     if (MEM_MAGIC == magic) {
//         module_name = va_arg(ap, char *);
//     }
//     va_end(ap);
// #endif

//     src_iter = src;
//     src_iter_len = strlen(src_iter);
//     key_iter = key;

//     do {
//         if ((delim = strchr(key_iter, '.')) != NULL) {
//             key_len = delim - key_iter;
//             key_next = key_iter;

//             // value = json_get_value_by_name_len(src_iter, src_iter_len, key_next, key_len, &value_len, 0);
//             if (value == NULL) {
//                 return NULL;
//             }

//             src_iter = value;
//             src_iter_len = value_len;
//             key_iter = delim + 1;
//         }
//     } while (delim);

//     key_len = strlen(key_iter);
//     key_next = key_iter;
//     // value = json_get_value_by_name_len(src_iter, src_iter_len, key_next, key_len, &value_len, 0);
//     if (NULL == value) {
//         return NULL;
//     }
//     ret = malloc((value_len + 1) * sizeof(char), magic, module_name);
//     if (NULL == ret) {
//         return NULL;
//     }
//     snprintf(ret, value_len + 1, "%s", value);
//     return ret;
// }