#ifndef __LN_SOFTAPCFG_API_H__
#define __LN_SOFTAPCFG_API_H__

#include "ln_types.h"

/**
 * @brief start softapconfig.
 * start softapconfig.
 *
 * @attention it's useful if you use Tencent Cloud or Ali Cloud related apps or micro programs.
 *            otherwise, don't care this API.
 *
 * @param[in]    product_id   pointer to the product id of IoT cloud.
 * @param[in]    device_name  pointer to the device name of IoT cloud.
 * @param[in]    proto_ver    pointer to the protocl version of IoT cloud's c-sdk.
 *
 * @return  Whether the setting is successful.
 * @retval  #LN_TRUE     successful.
 * @retval  #LN_FALSE    failed.
 */
int  ln_softapcfg_set_product_param(const char * product_id, const char * device_name, const char * proto_ver);

/**
 * @brief start softapconfig.
 * start softapconfig.
 *
 * @attention if pwd is NULL, the auth mode of softap is OPEN; otherwise, the auth mode is WPA2_PSK.
 *
 * @param[in]    ssid        pointer to the ssid of softAP,valid length range:1~63.
 * @param[in]    pwd         pointer to the password of softAP, valid length range:8~32.
 * @param[in]    local_ip    pointer to the local ip in softAP mode.(eg:"192.168.4.1")
 *
 * @return  Whether the startup is successful.
 * @retval  #LN_TRUE     successful.
 * @retval  #LN_FALSE    failed.
 */
int  ln_softapcfg_start(const char * ssid, const char * pwd, const char * local_ip);//

/**
 * @brief stop softapconfig.
 * stop softapconfig.
 *
 * @attention None
 *
 * @return  none.
 */
void  ln_softapcfg_stop(void);

/**
 * @brief check that softapconfig is complete.
 * check that softapconfig is complete.
 *
 * @attention None
 *
 * @return  is softapconfig complete?.
 * @retval  #LN_TRUE     complete.
 * @retval  #LN_FALSE    uncompleted.
 */
int  ln_softapcfg_is_complete(void);

/**
 * @brief get the ssid.
 * get the ssid.
 *
 * @attention None.
 *
 * @return  the pointer of ssid.
 */
uint8_t * ln_softapcfg_get_ssid(void);

/**
 * @brief get the password.
 * get the password.
 *
 * @attention None.
 *
 * @return  the pointer of password.
 */
uint8_t * ln_softapcfg_get_pwd(void);

/**
 * @brief get the token.
 * get the token.
 *
 * @attention it's useful if you use Tencent Cloud or Ali Cloud related apps or micro programs.
 *
 * @return  the pointer of token.
 */
uint8_t * ln_softapcfg_get_token(void);


#endif /* __LN_SOFTAPCFG_API_H__ */


