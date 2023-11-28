/**
 * @file     hal_uart.h
 * @author   BSP Team 
 * @brief    This file contains all of UART functions prototypes.
 * @version  0.0.0.1
 * @date     2021-08-16
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __HAL_UART_H
#define __HAL_UART_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_clock.h"

#include "reg_ln_uart.h"


typedef enum 
{
    UART_WORD_LEN_8 = 0,
    UART_WORD_LEN_9 = 1,
} uart_word_len_enum;

typedef enum 
{
    UART_OVER_SAMPL_16 = 0,
    UART_OVER_SAMPL_8  = 1,
} uart_over_sampl_enum;

typedef enum 
{
    UART_PARITY_EVEN = 0,
    UART_PARITY_ODD  = 1, 
    UART_PARITY_NONE = 2,
} uart_parity_enum;

typedef enum 
{
    UART_STOP_BITS_1   = 0,
    UART_STOP_BITS_0_5 = 1,
    UART_STOP_BITS_2   = 2,
    UART_STOP_BITS_1_5 = 3,
} uart_stop_bits_enum;


typedef enum 
{
    UART_HW_FLOW_CTRL_NONE    = ((uint16_t)0x0000),
    UART_HW_FLOW_CTRL_RTS     = ((uint16_t)0x0100),
    UART_HW_FLOW_CTRL_CTS     = ((uint16_t)0x0200),
    UART_HW_FLOW_CTRL_RTS_CTS = ((uint16_t)0x0300),
} uart_hw_flow_ctrl_enum;

typedef enum 
{
    USART_DMA_REQ_TX  = ((uint16_t)0x0080),
    USART_DMA_REQ_RX  = ((uint16_t)0x0040),
} uart_dma_req_enum;

typedef enum 
{
    USART_WAKEUP_IDLE_LINE = ((uint16_t)0x0000),
    USART_WAKEUP_ADDR_MARK = ((uint16_t)0x0800),
} uart_wakeup_mode_t;

typedef enum 
{
    USART_IT_RXFIFO_TIMEOUT         = (0U << 0),//TODO
    
    USART_IT_CME                    = (1U << 14),
    USART_IT_RXFIFO_AF              = (1U << 12),
    USART_IT_PE                     = (1U << 8),
    USART_IT_TXE                    = (1U << 7),
    USART_IT_TC                     = (1U << 6),
    USART_IT_RXNE                   = (1U << 5),
    USART_IT_IDLE                   = (1U << 4),
    USART_IT_ORE                    = (1U << 3),
    USART_IT_WUE                    = (1U << 22),
    USART_IT_CTS                    = (1U << 10),
    USART_IT_ERR                    = (1U << 0),
} uart_it_en_enum;

typedef enum 
{
    USART_IT_FLAG_CLEAR_WU          = (1U << 20),
    USART_IT_FLAG_CLEAR_CM          = (1U << 17),
    USART_IT_FLAG_CLEAR_CTS         = (1U << 9),
    USART_IT_FLAG_CLEAR_TC          = (1U << 6),
    USART_IT_FLAG_CLEAR_IDLE        = (1U << 4),
    USART_IT_FLAG_CLEAR_NOISE       = (1U << 2),
    USART_IT_FLAG_CLEAR_FE          = (1U << 1),
    USART_IT_FLAG_CLEAR_PE          = (1U << 0),
} uart_it_flag_clear_enum;

typedef enum 
{    
    USART_FLAG_TX_FIFO_FULL         = (1U << 24),
    
    USART_FLAG_RX_FIFO_ALMOST_FULL  = (1U << 21),
    USART_FLAG_WU                   = (1U << 20),
    USART_FLAG_RWU                  = (1U << 19),
    
    USART_FLAG_CM                   = (1U << 17),
    USART_FLAG_BUSY                 = (1U << 16),
    
    USART_FLAG_CTS                  = (1U << 10),
    USART_FLAG_CTSIT                = (1U << 9),
    
    USART_FLAG_TXE                  = (1U << 7),
    USART_FLAG_TC                   = (1U << 6),
    USART_FLAG_RXNE                 = (1U << 5),
    USART_FLAG_IDLE                 = (1U << 4),
    USART_FLAG_ORE                  = (1U << 3),
    USART_FLAG_NOISE                = (1U << 2),
    USART_FLAG_FE                   = (1U << 1),
    USART_FLAG_PE                   = (1U << 0),
} uart_flag_enum;

typedef struct
{
    uint32_t  baudrate;
    uint16_t  word_len;
    uint16_t  stop_bits;
    uint16_t  parity;
    uint16_t  over_sampl;
} uart_init_t_def;


/**==================================hal uart api=================================================**/
void          hal_uart_init(uint32_t uart_base, uart_init_t_def* uart_init);
void          hal_uart_deinit(uint32_t uart_base);
void          hal_uart_send_data(uint32_t uart_base, uint16_t data);
uint16_t      hal_uart_recv_data(uint32_t uart_base);

void          hal_uart_rx_mode_en(uint32_t uart_base, hal_en_t en);
void          hal_uart_tx_mode_en(uint32_t uart_base, hal_en_t en);
void          hal_uart_hardware_flow_rts_en(uint32_t uart_base, hal_en_t en);
void          hal_uart_hardware_flow_cts_en(uint32_t uart_base, hal_en_t en);
void          hal_uart_en(uint32_t uart_base, hal_en_t en);
void          hal_uart_dma_en(uint32_t uart_base, uart_dma_req_enum dma_req, hal_en_t en);
void          hal_uart_baudrate_set(uint32_t uart_base, uint32_t baudrate);

void          hal_uart_it_en(uint32_t uart_base, uart_it_en_enum it_en);
void          hal_uart_it_disable(uint32_t uart_base, uart_it_en_enum it_en);
void          hal_uart_it_flag_clear(uint32_t uart_base, uart_it_flag_clear_enum it_flag_clear);
uint8_t       hal_uart_flag_get(uint32_t uart_base, uart_flag_enum flag);

uint8_t       hal_uart_it_en_status_get(uint32_t uart_base, uart_it_en_enum it_en);
void          hal_uart_clr_over_run_err(uint32_t uart_base);

uint8_t       hal_uart_tx_fifo_level_get(uint32_t uart_base);
uint8_t       hal_uart_rx_fifo_level_get(uint32_t uart_base);
void          hal_uart_rx_fifo_it_trig_level_set(uint32_t uart_base, uint8_t level);

//void          uart_wakeup_cfg(uint32_t uart_base, uint16_t wakeup);
//void          uart_recv_wakeup_enale(uint32_t uart_base, hal_en_t en);
//void          uart_send_break(uint32_t uart_base);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __HAL_UART_H */
