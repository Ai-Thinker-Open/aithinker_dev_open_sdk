/**
 * @file     hal_qspi.c
 * @author   BSP Team
 * @brief
 * @version  0.0.0.1
 * @date     2021-10-19
 *
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 *
 */

#include "hal/hal_qspi.h"

#if defined (__CC_ARM)
  #pragma push
  #pragma O2
#elif defined (__GNUC__)
  #pragma GCC push_options
  #pragma GCC optimize(2)
#endif


void hal_qspi_init(uint16_t clk_divider, uint8_t rx_sample_dly)
{
    //select spif IO as pad
    sysc_cmp_spif_io_en_setf(1);

    //set rx sample delay
    qspi_rsd_setf(rx_sample_dly);

    //reset chip
    qspi_ssi_en_setf(QSPI_DISABLE);
    qspi_imr_set(0);
    qspi_sckdv_setf(clk_divider);
}

void hal_qspi_deinit()
{
    //QSPI IO disable
    sysc_cmp_spif_io_en_setf(0);

    qspi_ssi_en_setf(QSPI_DISABLE);
    qspi_imr_set(0);
}

void hal_qspi_standard_read_byte(uint8_t *rd_ptr, uint32_t rd_len, uint8_t *wr_ptr, uint8_t wr_len)
{
    uint32_t i = 0;

    qspi_sckdv_setf(16);

    //QSPI_SlaveSelect
    qspi_ser_setf(0);
    qspi_ctrlr0_pack(QSPI_STANDARD, DFS_32_8_BITS, 0, 0, 0, QSPI_EEPROM_READ, CLK_INACTIVE_LOW, CLK_TOGGLE_IN_MIDDLE, QSPI_MOTOROLA);
    qspi_ctrlr1_pack( rd_len - 1 );

    qspi_ser_setf(QSPI_SLAVE_INDEX);//LL_QSPI_SlaveSelect
    qspi_ssi_en_setf(QSPI_ENABLE);

    for(i = 0; i < wr_len; i++) {
        qspi_dr_set(*wr_ptr++);
    }
    while( !qspi_tfe_getf() ){};

    for(i = 0; i < rd_len; i++) {
        while( !qspi_rfne_getf()){};
        *rd_ptr++ = qspi_dr_get();
    }

    while( qspi_busy_getf() == QSPI_BUSY ){};
    qspi_ssi_en_setf( QSPI_DISABLE );

    qspi_sckdv_setf(4);
}

void hal_qspi_standard_read_word(uint32_t *rd_ptr, uint32_t rd_len_in_word, uint8_t instruction, uint32_t addr)
{
    uint32_t i = 0, rd_tmp = 0;
    uint8_t * pdata = (uint8_t *)rd_ptr;

    qspi_sckdv_setf(16);

    qspi_ser_setf(0);//LL_QSPI_SlaveSelect(0);
    qspi_ctrlr0_pack(QSPI_STANDARD, DFS_32_32_BITS, 0, 0, 0, QSPI_EEPROM_READ, CLK_INACTIVE_LOW, CLK_TOGGLE_IN_MIDDLE, QSPI_MOTOROLA);
    qspi_ctrlr1_pack( rd_len_in_word - 1);

    qspi_ser_setf( QSPI_SLAVE_INDEX );//LL_QSPI_SlaveSelect
    qspi_ssi_en_setf( QSPI_ENABLE );

    qspi_dr_set( (addr & 0x00ffffff) | (instruction << 24) );
    while( !qspi_tfe_getf() ){};

    for(i = 0; i < rd_len_in_word; i++)
    {
        while( !qspi_rfne_getf()){};
        rd_tmp = qspi_dr_get();
        *pdata++ = (uint8_t)(rd_tmp >> 24);
        *pdata++ = (uint8_t)(rd_tmp >> 16);
        *pdata++ = (uint8_t)(rd_tmp >> 8);
        *pdata++ = (uint8_t)(rd_tmp);
    }

    while( qspi_busy_getf() == QSPI_BUSY ){};
    qspi_ssi_en_setf(QSPI_DISABLE);

    qspi_sckdv_setf(4);
}

void hal_qspi_standard_write(uint8_t *bufptr, uint32_t length)
{
    qspi_sckdv_setf(16); //QSPI SCK DIV

    qspi_ser_setf(0);//LL_QSPI_SlaveSelect
    qspi_ctrlr0_pack(QSPI_STANDARD, DFS_32_8_BITS, 0, 0, 0, QSPI_TX_ONLY, CLK_INACTIVE_LOW, CLK_TOGGLE_IN_MIDDLE, QSPI_MOTOROLA);

    qspi_ssi_en_setf( QSPI_ENABLE );
    qspi_ser_setf( QSPI_SLAVE_INDEX );//LL_QSPI_SlaveSelect

    while(length-- > 0)
    {
        qspi_dr_set(*bufptr++);    // LL_QSPI_DataRegSet(*bufptr++);
        while(0 == qspi_tfnf_getf()){};
    }

    while(qspi_tfe_getf() != QSPI_TX_FIFO_EMPTY){};
    while(qspi_busy_getf() == QSPI_BUSY ){};

    qspi_ssi_en_setf (QSPI_DISABLE);

    qspi_sckdv_setf(4); //QSPI SCK DIV
}


#if defined (__CC_ARM)
  #pragma pop
#elif defined (__GNUC__)
  #pragma GCC pop_options
#endif
