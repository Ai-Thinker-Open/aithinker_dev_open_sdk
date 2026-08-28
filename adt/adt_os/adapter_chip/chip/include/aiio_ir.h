/** @brief      ir application interface.
 *
 *  @file       aiio_ir.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       ir application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/08/08      <td>1.0.0       <td>hongjz      <td>Define file
 *  </table>
 *
 */

#ifndef _AIIO_IR_H_
#define _AIIO_IR_H_

#include <stdint.h>

#include "aiio_type.h"
#include "aiio_error.h"

typedef int aiio_err_t;

typedef enum
{
    AIIO_IR_TX,
    AIIO_IR_RX,
    AIIO_IR_TXRX,
} aiio_ir_direction_t;

typedef enum
{
    AIIO_IR_RX_NEC,             /*!< IR RX NEC mode */
    AIIO_IR_RX_RC5,             /*!< IR RX RC5 mode */
    AIIO_IR_RX_SWM,             /*!< IR RX software pulse-width detection mode */
} aiio_rx_mode_t;

typedef struct
{
    uint8_t input_inverse;      /*!< Enable or disable signal of input inverse */
    uint8_t deglitch_enable;    /*!< Enable or disable signal of rx input de-glitch function */
    uint8_t deglitch_cnt;       /*!< De-glitch function cycle count */
    aiio_rx_mode_t mode;        /*!< RX mode select, use @ref aiio_rx_mode_t */
    uint16_t data_threshold;    /*!< Pulse width threshold for logic 0/1 detection (don't care if SWM is enabled) */
    uint16_t end_threshold;     /*!< Pulse width threshold to trigger end condition */
    uint16_t fifo_threshold;    /*!< RX FIFO threshold */
} aiio_ir_rx_cfg_t;

/** @brief Init ir rx gpio
 *
 *  @param[in]      port            The IO port to be initialized
 *  @param[in]      pin             The IO pins to be initialized
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API aiio_err_t aiio_ir_rx_gpio_init(uint32_t port, uint16_t pin);

/** @brief Init ir rx
 *
 *  @param[in]      cfg             ir rx config struct
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API aiio_err_t aiio_ir_rx_init(aiio_ir_rx_cfg_t *cfg);

/** @brief ir rx enable
 *
 *  @param[in]      enable          1 is enable,0 is disable
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API aiio_err_t aiio_ir_rx_enable(uint8_t enable);

/** @brief ir receive data
 *
 *  @param[in]      rx_data         ir rx data
 *  @param[in]      bit_cnt         ir recv bit cnt
 *  @return                         Return the operation status. When the return value is AIIO_OK, is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note
 *  @see
 */
CHIP_API aiio_err_t aiio_ir_receive(uint64_t *rx_data, uint8_t *bit_cnt);

#endif //_AIIO_IR_H_
