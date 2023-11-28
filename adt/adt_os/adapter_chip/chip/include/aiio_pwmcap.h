/** @brief      pwmcap application interface.
 *
 *  @file       aiio_pwmcap.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       pwmcap application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/02/07      <td>1.0.0       <td>liq         <td>pwmcap Init
 *  </table>
 *
 */

#ifndef __AIIO_PWMCAP_H__
#define __AIIO_PWMCAP_H__
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"


typedef enum 
{
    PULSE_CAP_NONE   = 0,
    PULSE_CAP_FAIL   ,
    PULSE_CAP_WAIT   ,
    PULSE_CAP_DONE   ,
    PULSE_CAP_SUCESS ,
}PULSE_CAP_STA;

/**
 *  @brief pwmcap pin config interface
 */
typedef struct
{
    uint32_t aiio_gpio_pwmcap_port; /*!< GPIO port */
    uint32_t aiio_gpio_pwmcap_pin; /*!< GPIO pin */
} aiio_pwmcap_pin_cfg_t;

/** @brief pwm capture init
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwmcap_capture_init(aiio_pwmcap_pin_cfg_t *aiio_i2c_cfg);

/** @brief pwm capture start
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwmcap_capture_start(void);

/** @brief pwm capture close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwmcap_capture_close(void);

/** @brief pwm capture close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @param[in]      freqlist        freqlist value
 *  @param[in]      dutylist        dutylist value
 *  @param[in]      size            list size
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwmcap_capture_value(uint32_t *freqlist,float *dutylist,uint32_t *size);
#endif
