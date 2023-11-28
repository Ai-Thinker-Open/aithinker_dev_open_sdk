#ifndef __HAL_QSPI_H
#define __HAL_QSPI_H

#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#include "hal/hal_common.h"
#include "reg_qspi.h"

#define QSPI_READ_MAX_LENGTH            0x10000
#define QSPI_TX_FIFO_DEPTH              16
#define QSPI_RX_FIFO_DEPTH              16
#define QSPI_SLAVE_INDEX                1
#define QSPI_DMA_Receive_Data_Level     8  //match with DMAC.CTL.SRC_MSIZE
#define QSPI_DMA_Transmit_Data_Level    8  //match with DMAC.CTL.DST_MSIZE


typedef enum
{
    QSPI_STANDARD                = 0,
    QSPI_DUAL                    = 1,
    QSPI_QUAD                    = 2,
} qspi_format_t;

typedef enum
{
    DFS_32_8_BITS                = 7,
    DFS_32_16_BITS               = 15,
    DFS_32_24_BITS               = 23,
    DFS_32_32_BITS               = 31,
} qspi_data_frame_size_t;

typedef enum
{
    QSPI_TX_RX                   = 0,
    QSPI_TX_ONLY                 = 1,
    QSPI_RX_ONLY                 = 2,
    QSPI_EEPROM_READ             = 3,
} qspi_tx_mode_t;

typedef enum
{
    CLK_INACTIVE_LOW             = 0,
    CLK_INACTIVE_HIGH            = 1,
} qspi_clock_polarity_t;

typedef enum
{
    CLK_TOGGLE_IN_MIDDLE         = 0,
    CLK_TOGGLE_AT_START          = 1,
} qspi_clock_phase_t;

typedef enum
{
    QSPI_MOTOROLA                = 0,
    QSPI_TEXAS_INSTRUMENTS_SSP   = 1,
    QSPI_NATIONAL_SEMI_MICROWIRE = 2,
} qspi_protocol_type_t;

typedef enum
{
    QSPI_DISABLE                 = 0,
    QSPI_ENABLE                  = 1,
} qspi_en_t;

typedef enum
{
    QSPI_TX_FIFO_NOT_EMPTY       = 0,
    QSPI_TX_FIFO_EMPTY           = 1,
} qspi_status_tfe_t;

typedef enum
{
    QSPI_TX_FIFO_FULL            = 0,
    QSPI_TX_FIFO_NOT_FULL        = 1,
} qspi_status_tfnf_t;

typedef enum
{
    QSPI_IDLE_OR_DISABLE         = 0,
    QSPI_BUSY                    = 1,
} qspi_status_busy_t;

typedef enum
{
    QSPI_TX_DMA_DISABLE          = 0,
    QSPI_TX_DMA_ENABLE           = 1,
} qspi_tx_dma_en_t;

typedef enum
{
    QSPI_RX_DMA_DISABLE          = 0,
    QSPI_RX_DMA_ENABLE           = 1,
} qspi_rx_dma_en_t;


void  hal_qspi_init(uint16_t clk_divider, uint8_t rx_sample_dly);
void  hal_qspi_deinit(void);
void  hal_qspi_standard_write(uint8_t *bufptr, uint32_t length);
void  hal_qspi_standard_read_byte(uint8_t *rd_ptr, uint32_t rd_len, uint8_t *wr_ptr, uint8_t wr_len);
void  hal_qspi_standard_read_word(uint32_t *rd_ptr, uint32_t rd_len_in_word, uint8_t instruction, uint32_t addr);


#ifdef __cplusplus
    extern "C" {
#endif // __cplusplus

#endif /* __HAL_QSPI_H*/
