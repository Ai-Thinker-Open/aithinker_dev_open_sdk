#ifndef __LN_SMARTCFG_API_H__
#define __LN_SMARTCFG_API_H__

#include "ln_types.h"
#include "ln_smartcfg_port.h"


typedef enum
{
    CHANNEL1_MASK     = (1UL << 1 ),
    CHANNEL2_MASK     = (1UL << 2 ),
    CHANNEL3_MASK     = (1UL << 3 ),
    CHANNEL4_MASK     = (1UL << 4 ),
    CHANNEL5_MASK     = (1UL << 5 ),
    CHANNEL6_MASK     = (1UL << 6 ),
    CHANNEL7_MASK     = (1UL << 7 ),
    CHANNEL8_MASK     = (1UL << 8 ),
    CHANNEL9_MASK     = (1UL << 9 ),
    CHANNEL10_MASK    = (1UL << 10),
    CHANNEL11_MASK    = (1UL << 11),
    CHANNEL12_MASK    = (1UL << 12),
    CHANNEL13_MASK    = (1UL << 13),
} channel_mask_t;


/**
 * @brief start smartconfig.
 * start smartconfig.
 *
 * @attention None
 *
 * @param[in]    channel_map    scan those channels before locking channel, see CHANNEL*_MASK.
 *
 * @return  Whether the startup is successful.
 * @retval  #LN_TRUE     successful.
 * @retval  #LN_FALSE    failed.
 */
int  ln_smartconfig_start(uint16_t channel_map);

/**
 * @brief stop smartconfig.
 * stop smartconfig.
 *
 * @attention None
 *
 * @return  none.
 */
void  ln_smartconfig_stop(void);

/**
 * @brief check that smartconfig is complete.
 * check that smartconfig is complete.
 *
 * @attention None
 *
 * @return  is smartconfig complete?.
 * @retval  #LN_TRUE     complete.
 * @retval  #LN_FALSE    uncompleted.
 */
int  ln_smartconfig_is_complete(void);

/**
 * @brief smartconfig send ack
 * send the random to smartphone app.
 *
 * @attention None
 *
 * @return  Whether the ack was sent successfully.
 * @retval  #LN_TRUE     successful.
 * @retval  #LN_FALSE    failed.
 */
int  ln_smartconfig_send_ack(void);

/**
 * @brief get the ssid.
 * get the ssid.
 *
 * @attention None.
 *
 * @return  the pointer of ssid.
 */
uint8_t * ln_smartconfig_get_ssid(void);

/**
 * @brief get the password.
 * get the password.
 *
 * @attention None.
 *
 * @return  the pointer of password.
 */
uint8_t * ln_smartconfig_get_pwd(void);


#endif /* __LN_SMARTCFG_API_H__ */


