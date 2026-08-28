/** @brief      Uart application interface.
 *
 *  @file       aiio_uart.h
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Uart application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2023/01/30      <td>1.0.0       <td>chenzf      <td>Define file
 *  </table>
 *
 */

#ifndef __AIIO_UART_H__
#define __AIIO_UART_H__


#include "stdio.h"
#include "stdlib.h"
#include "aiio_type.h"
#include "aiio_gpio.h"
#include "aiio_chip_spec.h"

enum
{
    AIIO_UART0 = __AIIO_UART0, /*!< UART0 ID */
#if defined(__AIIO_UART1)
    AIIO_UART1 = __AIIO_UART1, /*!< UART1 ID */
#endif
#if defined(__AIIO_UART2)
    AIIO_UART2 = __AIIO_UART2, /*!< UART2 ID */
#endif
    AIIO_UART_MAX,
};

#define AIIO_UART0_TX           __AIIO_UART0_TX
#define AIIO_UART0_RX           __AIIO_UART0_RX

#if defined(__AIIO_UART1)
#define AIIO_UART1_TX           __AIIO_UART1_TX
#define AIIO_UART1_RX           __AIIO_UART1_RX
#endif

#if defined(__AIIO_UART2)
#define AIIO_UART2_TX           __AIIO_UART2_TX
#define AIIO_UART2_RX           __AIIO_UART2_RX
#endif

#define AIIO_LOG_MAX_LEN    (1024)


typedef void (*aiio_uart_int_callback)(uint8_t *buf, uint16_t size);
typedef enum{
    AIIO_UART_DATA_BITS_5=5,
    AIIO_UART_DATA_BITS_6=6,
    AIIO_UART_DATA_BITS_7=7,
    AIIO_UART_DATA_BITS_8=8,
}aiio_uart_data_bits;
typedef enum{
    AIIO_UART_STOPBITS_0_5=0,
    AIIO_UART_STOP_BITS_1=1,
    AIIO_UART_STOP_BITS_1_5=2,
    AIIO_UART_STOP_BITS_2=3,
}aiio_uart_stop_bits;
typedef enum{
    AIIO_UART_PARITY_NONE=0,
    AIIO_UART_PARITY_ODD=1,
    AIIO_UART_PARITY_EVEN=2,
}aiio_uart_parity;
typedef enum{
    AIIO_UART_FLOW_CONTROL_NONE=0,
    AIIO_UART_FLOW_CONTROL_RTS=1,
    AIIO_UART_FLOW_CONTROL_CTS=2,
    AIIO_UART_FLOW_CONTROL_RTS_CTS=3,
}aiio_uart_flow_control;

/**
 *  @brief Log dedicated serial port structure.
 */
typedef struct
{
    uint8_t                     uart_num;       /*!< UART ID */
    uint16_t                     uart_tx_pin;    /*!< UART TX */
    uint16_t                     uart_rx_pin;    /*!< UART RX  */
    uint32_t                    baud_rate;      /*!< UART BAUD_RATE */
    aiio_uart_int_callback      irq_rx_bk;      /*!< UART recv data callback function*/
    aiio_uart_data_bits dataBits;
    aiio_uart_stop_bits stopBits;
    aiio_uart_parity parity;
    aiio_uart_flow_control flowControl;
    uint8_t                     in_at_mode;     /*!< 0:normal mode 1:at mode */
} aiio_uart_config_t;

/** @brief Log uart initialization function.
 *
 *  @param[in]      uart            Configure serial port printing, including serial port ID, pin, and baud rate.
 *  @return                         Return the operation status. When the return value is AIIO_OK, the initialization is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note                           This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *                                  The main function is to initialize the chip, including clock initialization, register initialization, etc., must call this function to run other interfaces.
 *  @see
 */
CHIP_API int32_t aiio_uart_log_init(aiio_uart_config_t uart);

/** @brief Log uart send date function.
 *
 *  @param[in]      *fmt            Format string.
 *  @param[in]      ...             Parameter scale.
 *  @return                         Return the operation status. When the return value is AIIO_OK, the send is successful.
 *  @retval         AIIO_OK         Init successful.
 *  @retval         AIIO_ERROR      Init error.
 *  @note                           This function needs to be adapted according to different platforms, and the system engineer should complete the adaptation.
 *
 *  @see
 */
CHIP_API int32_t aiio_uart_log_send(const char *fmt, ...);

/**
 * @brief Serial port param check
 * @param  uart Configure serial port printing, including serial port ID, pin, and baud rate.
 * @return  AIIO_OK
 *          AIIO_UART_NUMBER_NOT_SUPPORT
 *          AIIO_UART_PIN_NOT_SUPPORT
 *          AIIO_UART_BAUD_RATE_NOT_SUPPORT
 *          AIIO_UART_DATA_BITS_NOT_SUPPORT
 *          AIIO_UART_STOP_BITS_NOT_SUPPORT
 *          AIIO_UART_PARITY_NOT_SUPPORT
 *          AIIO_UART_FLOW_CONTROL_NOT_SUPPORT
 */
CHIP_API int32_t aiio_uart_param_check(aiio_uart_config_t uart);

/**
 * @brief Serial port init
 * @param  uart Configure serial port printing, including serial port ID, pin, and baud rate.
 * @return  AIIO_OK    Return the operation status. When the return value is AIIO_OK, the send is successful.
 *          AIIO_ERROR  set error
 *          AIIO_PARAM_ERROR
 *          AIIO_UART_NUMBER_NOT_SUPPORT
 *          AIIO_UART_PIN_NOT_SUPPORT
 *          AIIO_UART_BAUD_RATE_NOT_SUPPORT
 *          AIIO_UART_DATA_BITS_NOT_SUPPORT
 *          AIIO_UART_STOP_BITS_NOT_SUPPORT
 *          AIIO_UART_PARITY_NOT_SUPPORT
 *          AIIO_UART_FLOW_CONTROL_NOT_SUPPORT
 */
CHIP_API int32_t aiio_uart_init(aiio_uart_config_t uart);

/**
 * @brief Serial port deinit
 * @param  uart Configure serial port printing, including serial port ID, pin, and baud rate.
 * @return Return the operation status. When the return value is AIIO_OK, the send is successful.
 */
CHIP_API int32_t aiio_uart_deinit(aiio_uart_config_t uart);

/**
 * @brief The serial port sends data
 * @param  uart_num The serial port ID that needs to be set @see AIIO_UART0
 * @param  buf The serial port sends a data buffer
 * @param  size The data length of the serial port transmit data buffer
 * @return Return the operation status. When the return value is AIIO_OK, the send is successful.
 *
 */
CHIP_API int32_t aiio_uart_send_data(uint8_t uart_num, uint8_t *buf, uint16_t size);

/**
 * @brief The serial port receives data
 * @param  uart_num The serial port ID that needs to be set @see AIIO_UART0
 * @param  buf The serial port receives the data buffer
 * @param  size The length of the data read by the serial port
 * @return Return the operation status. When the return value is AIIO_OK, the send is successful.
 */
CHIP_API int32_t aiio_uart_recv_data(uint8_t uart_num, uint8_t *buf, uint16_t size);

#endif
