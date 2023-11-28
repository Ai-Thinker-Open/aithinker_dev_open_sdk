/** @brief PWM HAL layer header file
 *
 *  @file        ai_hal_pwm.h
 *  @copyright   Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note
 *  @par         Change Logs:
 *  <table>
 *  <tr><th>Date                <th>Version         <th>Author          <th>Notes
 *  <tr><th>2023/02/06          <td>V1.0.0          <td>hewm            <td>Define the PWM header API
 *  <table>
 *
 */

#ifndef __AIIO_HAL_PWM_H__
#define __AIIO_HAL_PWM_H__

#include "stdint.h"
#include "aiio_log.h"
#include "aiio_type.h"
#include "aiio_error.h"
#include "aiio_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief PWM channel number enumeration
 */
typedef enum
{
    AIIO_PWM_CHA_0 = 0,
    AIIO_PWM_CHA_1 = 1,
    AIIO_PWM_CHA_2 = 2,
    AIIO_PWM_CHA_3 = 3,
    AIIO_PWM_CHA_4 = 4,
    AIIO_PWM_CHA_5 = 5,
    AIIO_PWM_CHA_6 = 6,
    AIIO_PWM_CHA_7 = 7,
    AIIO_PWM_CHA_8 = 8,
    AIIO_PWM_CHA_9 = 9,
    AIIO_PWM_CHA_10 = 10,
    AIIO_PWM_CHA_11 = 11,
}aiio_pwm_chann_t;

/**
 * @brief PWM channel port enumeration
 */
typedef enum
{
    AIIO_PWM_PORTA = 0,
    AIIO_PWM_PORTB = 1,
} aiio_pwm_port_t;

typedef struct
{
    uint32_t freq;                    /**< Set the PWM frequency, unit hz, range 100hz ~ 20Mhz*/
    float duty;                       /**< Set PWM duty cycle, in % range 0% ~ 100%*/
    uint16_t gpio_port;               /**< Set the PWM GPIO port*/
    uint16_t gpio_pin;                /**< Set the PWM GPIO pin*/
} aiio_pwm_init_t;

/**
 * @brief PWM initialization
 * @param  init Device initialization information
 * @return Status  - Returns the specific channel number, fail -1
 */
CHIP_API int aiio_pwm_init(aiio_pwm_init_t *init);

/**
 * @brief PWM begins to be generated
 * @param  pwm_channel_num Set the channel number
 * @return Status  - 0: Succeed, else: fail
 */
CHIP_API int aiio_pwm_start(aiio_pwm_chann_t pwm_channel_num);

/**
 * @brief Set the PWM duty cycle
 * @param  duty Set PWM duty cycle, in % range 0% ~ 100%
 * @param  pwm_channel_num Set the channel number
 * @return Status  - 0: Succeed, else: fail
 */
CHIP_API int aiio_pwm_set_duty(aiio_pwm_chann_t pwm_channel_num, float duty);

/**
 * @brief Obtain the duty cycle of the corresponding channel
 * @param  pwm_channel_num Set the channel number
 * @return returns PWM duty cycle, unit %, range 0% ~ 100%
 */
CHIP_API float aiio_pwm_get_duty(aiio_pwm_chann_t pwm_channel_num);

/**
 * @brief Set the PWM frequency
 * @param  pwm_channel_num Set the channel number
 * @param  period Set PWM period, unit us, range 0 ~ 1638us (undivided)
 * @return Status  - 0: Succeed, else: fail
 */
CHIP_API int aiio_pwm_set_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t period);

/** @brief Get the PWM frequency
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @param[in]      period          PWM period value
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwm_get_freq(aiio_pwm_chann_t pwm_channel_num, uint32_t *period);

/** @brief pwm output close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwm_output_close(aiio_pwm_chann_t pwm_channel_num);

/** @brief pwm output close
 *
 *  @param[in]      pwm_channel_num Set the channel number
 *  @param[in]      limit           limit number
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 *  @author         liq
 */
CHIP_API int32_t aiio_pwm_output(aiio_pwm_chann_t pwm_channel_num,int cnt, uint32_t freq, float duty,uint32_t gpio_port, uint32_t gpio_pin);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__AIIO_HAL_PWM_H__
