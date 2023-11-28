/**
 * @file     hal_cache.c
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2021-08-17
 *
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#include "hal/hal_cache.h"


#if defined(__CC_ARM)
  #pragma push
  #pragma O2
#elif defined(__GNUC__)
  #pragma GCC push_options
  #pragma GCC optimize(2)
#endif


typedef enum
{
    CACHE_STATE_IDLE             = 0,
    CACHE_STATE_FILLING          = 1,
} cache_state_t;

typedef enum
{
    CACHE_DISABLE                = 0,
    CACHE_ENABLE                 = 1,
} Cache_enable_t;

typedef enum
{
    CACHE_FLUSH_DISABLE          = 0,
    CACHE_FLUSH_ENABLE           = 1,
} cache_flush_en_t;

typedef enum
{
    CACHE_FLUSH_STATE_DONE       = 0,
    CACHE_FLUSH_STATE_INPROGRESS = 1,
} cache_flush_state_t;

typedef enum
{
    FLASH_LITTLE_ENDIAN          = 0,
    FLASH_BIG_ENDIAN             = 1,
} flash_output_data_format_t;

typedef enum
{
    RIGHT_JUSTIFIED              = 0,
    LEFT_JUSTIFIED               = 1,
} cmd_addr_alignment_t;



void flash_cache_init(uint32_t flash_base_addr)
{
    #define QSPI_INSTRUCTION_LEN_8_BITS     (0x02)
    #define QSPI_BOTH_STANDARD_SPI_MODE     (0)

    #define FLASH_QUAD_READ_CMD             (0x6b)
    #define FLASH_STANDARD_READ_CMD         (0x03)
    #define CACHE_LINE_SIZE_IN_BYTES        (32)

    qspi_ssi_en_setf( QSPI_DISABLE );
    qspi_sckdv_setf( 2 );
    qspi_rsd_setf(1);//QSPI_RxSampleDlySet(1);

    qspi_ctrlr0_pack( QSPI_QUAD, DFS_32_32_BITS, 0, 0, 0, QSPI_RX_ONLY, CLK_INACTIVE_LOW, CLK_TOGGLE_IN_MIDDLE, QSPI_MOTOROLA);
    qspi_ctrlr1_pack( (CACHE_LINE_SIZE_IN_BYTES / sizeof(uint32_t)) - 1 );

    qspi_spi_ctrlr0_pack( 8, QSPI_INSTRUCTION_LEN_8_BITS, 6, QSPI_BOTH_STANDARD_SPI_MODE );

    qspi_imr_set(0);                  //QSPI_IntMaskAll
    qspi_ser_setf( QSPI_SLAVE_INDEX );//LL_QSPI_SlaveSelect
    qspi_ssi_en_setf( QSPI_ENABLE );  //LL_QSPI_Enable

    cache_flash_base_addr_setf( flash_base_addr );
    cache_qspi_dr_addr_setf( (uint32_t)&(hwp_qspi->dr) );
    cache_reg_flash_cmd_pack( 0, 2, RIGHT_JUSTIFIED, FLASH_LITTLE_ENDIAN, RIGHT_JUSTIFIED, FLASH_QUAD_READ_CMD );
    cache_cache_en_setf( CACHE_ENABLE );
    cache_flush_all_setf( 1 );
    cache_flush_en_setf( CACHE_FLUSH_ENABLE );
    sysc_awo_o_flash_mode_setf(1);
    while(cache_flush_en_getf()){};
}

void flash_cache_disable(void)
{
    while(cache_cache_cs_getf() == CACHE_STATE_FILLING);
    cache_cache_en_setf(CACHE_DISABLE);
    qspi_ssi_en_setf(QSPI_DISABLE);
}

void flash_cache_flush(uint32_t low_addr,uint32_t high_addr)
{
    cache_reg_flush_addr_l_set(low_addr);
    cache_reg_flush_addr_h_set(high_addr);

    cache_flush_en_setf(CACHE_FLUSH_ENABLE);
    while(cache_flush_en_getf() == CACHE_FLUSH_STATE_INPROGRESS){};
}

void flash_cache_flush_all(void)
{
    cache_flush_all_setf(1);
    cache_flush_en_setf(CACHE_FLUSH_ENABLE);
    while(cache_flush_en_getf() == CACHE_FLUSH_STATE_INPROGRESS){};
}


#if defined(__CC_ARM)
  #pragma pop
#elif defined(__GNUC__)
  #pragma GCC pop_options
#endif
