/**
 * @file     hal_spi.c
 * @author   BSP Team 
 * @brief    This file provides spi function.
 * @version  0.0.0.1
 * @date     2020-12-09
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_spi.h"


void hal_spi_init(uint32_t spi_x_base,spi_init_type_def* spi_init)
{

    /* check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));

    /* Check the SPI parameters */
    hal_assert(IS_SPI_DIRECTION_MODE(spi_init->spi_direction));
    hal_assert(IS_SPI_MODE(spi_init->spi_mode));
    hal_assert(IS_SPI_DATASIZE(spi_init->spi_data_size));
    hal_assert(IS_SPI_CPOL(spi_init->spi_cpol));
    hal_assert(IS_SPI_CPHA(spi_init->spi_cpha));
    hal_assert(IS_SPI_NSS_MODEL(spi_init->spi_nss_mode));
    hal_assert(IS_SPI_BAUDRATE_PRESCALER(spi_init->spi_baud_rate_prescaler));
    hal_assert(IS_SPI_FIRST_BIT(spi_init->spi_first_bit));
    hal_assert(IS_SPI_CRC_POLYNOMIAL(spi_init->spi_crc_polynomial));

    /* Enable the spi clock */
    if(spi_x_base == SPI0_BASE)
    {
        sysc_cmp_spi0_gate_en_setf(1);
        for(volatile int i = 0; i < 20; i++)
            __NOP();
    }
    else if(spi_x_base == SPI1_BASE)
    {
        sysc_cmp_spi1_gate_en_setf(1);
        for(volatile int i = 0; i < 20; i++)
            __NOP();
    }

    /*---------------------------- SPIx CR1 Configuration ------------------------*/

    /* Set BIDImode, BIDIOE and RxONLY bits according to SPI_Direction value */
    
    switch (spi_init->spi_direction)
    {
        case SPI_DIRECTION_2LINES_FULLDUPLEX:
            spi_bidimode_setf(spi_x_base,0);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,0);
            break;
        case SPI_DIRECTION_2LINES_RXONLY:
            spi_bidimode_setf(spi_x_base,0);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,1);
            break;
        case SPI_DIRECTION_1LINE_RX:
            spi_bidimode_setf(spi_x_base,1);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,1);
            break;
        case SPI_DIRECTION_1LINE_TX:
            spi_bidimode_setf(spi_x_base,1);
            spi_bidioe_setf(spi_x_base,1);
            spi_rxonly_setf(spi_x_base,0);
            break;
        default:
            break;
    }

    /* Set SSM, SSI and MSTR bits according to SPI_Mode and SPI_NSS values */
    switch (spi_init->spi_nss_mode)
    {
        case SPI_NSS_HARD:
            spi_ssm_setf(spi_x_base,0);
            break;
        case SPI_NSS_SOFT:
            spi_ssm_setf(spi_x_base,1);
            break;
        default:
            break;
    }

    switch (spi_init->spi_mode)
    {
        case SPI_MODE_SLAVE:
            spi_mstr_setf(spi_x_base,0);
            break;
        case SPI_MODE_MASTER:
            spi_mstr_setf(spi_x_base,1);
            break;
        default:
            break;
    }

    /* Set LSBFirst bit according to SPI_FirstBit value */
    switch (spi_init->spi_first_bit)
    {
        case SPI_FIRST_BIT_MSB:
            spi_lsbfirst_setf(spi_x_base,0);
            break;
        case SPI_FIRST_BIT_LSB:
            spi_lsbfirst_setf(spi_x_base,1);
            break;
        default:
            break;
    }

    /* Set BR bits according to SPI_BaudRatePrescaler value */
    switch (spi_init->spi_baud_rate_prescaler)
    {
        case SPI_BAUDRATEPRESCALER_2:
            spi_br_setf(spi_x_base,0);
            break;
        case SPI_BAUDRATEPRESCALER_4:
            spi_br_setf(spi_x_base,1);
            break;
        case SPI_BAUDRATEPRESCALER_8:
            spi_br_setf(spi_x_base,2);
            break;
        case SPI_BAUDRATEPRESCALER_16:
            spi_br_setf(spi_x_base,3);
            break;
        case SPI_BAUDRATEPRESCALER_32:
            spi_br_setf(spi_x_base,4);
            break;
        case SPI_BAUDRATEPRESCALER_64:
            spi_br_setf(spi_x_base,5);
            break;
        case SPI_BAUDRATEPRESCALER_128:
            spi_br_setf(spi_x_base,6);
            break;
        case SPI_BAUDRATEPRESCALER_256:
            spi_br_setf(spi_x_base,7);
            break;
        default:
            break;
    }

    /* Set CPOL bit according to SPI_CPOL value */
    switch (spi_init->spi_cpol)
    {
        case SPI_CPOL_LOW:
            spi_cpol_setf(spi_x_base,0);
            break;
        case SPI_CPOL_HIGH:
            spi_cpol_setf(spi_x_base,1);
            break;
        default:
            break;
    }

    /* Set CPHA bit according to SPI_CPHA value */
    switch (spi_init->spi_cpha)
    {
        case SPI_CPHA_1EDGE:
            spi_cpha_setf(spi_x_base,0);
            break;
        case SPI_CPHA_2EDGE:
            spi_cpha_setf(spi_x_base,1);
            break;
        default:
            break;
    }
    
    /* Set data size */
    switch (spi_init->spi_data_size)
    {
        case SPI_DATASIZE_8B:
            spi_dff_setf(spi_x_base,0);
            break;
        case SPI_DATASIZE_16B:
            spi_dff_setf(spi_x_base,1);
            break;
        default:
            break;
    }
}

void hal_spi_deinit(uint32_t spi_x_base)
{   
    if(spi_x_base == SPI0_BASE)
    {
        sysc_cmp_srstn_spi0_setf(0);
        sysc_cmp_srstn_spi0_setf(1);
        sysc_cmp_spi0_gate_en_setf(0);
    }
    else if(spi_x_base == SPI1_BASE)
    {
        sysc_cmp_srstn_spi1_setf(0);
        sysc_cmp_srstn_spi1_setf(1);
        sysc_cmp_spi1_gate_en_setf(0);
    }
}

void hal_spi_set_nss(uint32_t spi_x_base,spi_nss_model_t software_config)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_NSS_MODEL(software_config));
    switch (software_config)
    {
        case SPI_NSS_HARD:
            spi_ssm_setf(spi_x_base,0);
            break;
        case SPI_NSS_SOFT:
            spi_ssm_setf(spi_x_base,1);
            break;
        default:
            break;
    }
}

void hal_spi_ssoe_en(uint32_t spi_x_base,hal_en_t en)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    switch (en)
    {
        case HAL_DISABLE:
            spi_ssoe_setf(spi_x_base,0);
            break;
        case HAL_ENABLE:
            spi_ssoe_setf(spi_x_base,1);
            break;
        default:
            break;
    }
   
}

void  hal_spi_it_cfg(uint32_t spi_x_base,spi_it_flag_t spi_it,hal_en_t en)
{
    unsigned char tmp_reg = 0;

    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_IT_FLAG(spi_it));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    for(int i = 0 ; i < 8 ; i++)
    {
        tmp_reg  = (spi_it  & (0x01 << i));
        switch (tmp_reg)
        {
            case SPI_IT_FLAG_RXNE:
            {
                switch (en)
                {
                    case HAL_DISABLE:
                        spi_rxneie_setf(spi_x_base,0);
                        break;
                    case HAL_ENABLE:
                        spi_rxneie_setf(spi_x_base,1);
                        break;
                    default:
                        break;
                }
                break;
            } 
            case SPI_IT_FLAG_TXE:
            {
                switch (en)
                {
                    case HAL_DISABLE:
                        spi_txeie_setf(spi_x_base,0);
                        break;
                    case HAL_ENABLE:
                        spi_txeie_setf(spi_x_base,1);
                        break;
                    default:
                        break;
                }
                break;
            }
            case SPI_IT_FLAG_CRC_ERR:
            case SPI_IT_FLAG_MODF:
            case SPI_IT_FLAG_OVR:
            {
                switch (en)
                {
                    case HAL_DISABLE:
                        spi_errie_setf(spi_x_base,0);
                        break;
                    case HAL_ENABLE:
                        spi_errie_setf(spi_x_base,1);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
    }
}


void hal_spi_set_data_size(uint32_t spi_x_base,spi_data_size_t data_size)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_DATASIZE(data_size));
    switch (data_size)
    {
        case SPI_DATASIZE_8B:
            spi_dff_setf(spi_x_base,0);
            break;
        case SPI_DATASIZE_16B:
            spi_dff_setf(spi_x_base,1);
            break;
        default:
            break;
    }
}


void hal_spi_set_bidirectional_line(uint32_t spi_x_base,spi_direction_t spi_direction)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_DIRECTION_MODE(spi_direction));

    switch (spi_direction)
    {
        case SPI_DIRECTION_2LINES_FULLDUPLEX:
            spi_bidimode_setf(spi_x_base,0);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,0);
            break;
        case SPI_DIRECTION_2LINES_RXONLY:
            spi_bidimode_setf(spi_x_base,0);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,1);
            break;
        case SPI_DIRECTION_1LINE_RX:
            spi_bidimode_setf(spi_x_base,1);
            spi_bidioe_setf(spi_x_base,0);
            spi_rxonly_setf(spi_x_base,1);
            break;
        case SPI_DIRECTION_1LINE_TX:
            spi_bidimode_setf(spi_x_base,1);
            spi_bidioe_setf(spi_x_base,1);
            spi_rxonly_setf(spi_x_base,0);
            break;
        default:
            break;
    }
}

void hal_spi_en(uint32_t spi_x_base,hal_en_t en)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    switch (en)
    {
        case HAL_DISABLE:
            spi_spe_setf(spi_x_base,0);
            break;
        case HAL_ENABLE:
            spi_spe_setf(spi_x_base,1);
            break;
        default:
            break;
    }
}

void hal_spi_dma_en(uint32_t spi_x_base,spi_dma_en_t spi_dma_en, hal_en_t en)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_DMA_EN(spi_dma_en));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    switch (spi_dma_en)
    {
        case SPI_DMA_TX_EN:
        {
            switch (en)
            {
                case HAL_DISABLE:
                    spi_txdmaen_setf(spi_x_base,0);
                    break;
                case HAL_ENABLE:
                    spi_txdmaen_setf(spi_x_base,1);
                    break;
                default:
                    break;
            }
            break;
        }
        case SPI_DMA_RX_EN:
        {
            switch (en)
            {
                case HAL_DISABLE:
                    spi_rxdmaen_setf(spi_x_base,0);
                    break;
                case HAL_ENABLE:
                    spi_rxdmaen_setf(spi_x_base,1);
                    break;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

void hal_spi_send_data(uint32_t spi_x_base,uint16_t data)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    spi_dr_set(spi_x_base,data);
}

uint16_t hal_spi_recv_data(uint32_t spi_x_base)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    return spi_dr_get(spi_x_base);
}

uint8_t hal_spi_wait_txe(uint32_t spi_x_base,uint32_t  timeout)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));

    unsigned int count = 0;
    while(spi_txe_getf(spi_x_base) == 0)
    {
        count++;
        if(count >= timeout)
            return HAL_RESET;
    }
    return HAL_SET;
}

uint8_t hal_spi_wait_rxne(uint32_t spi_x_base,uint32_t  timeout)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));

    unsigned int count = 0;
    while(spi_rxne_getf(spi_x_base) == 0)
    {
        count++;
        if(count >= timeout)
            return HAL_RESET;
    }
    return HAL_SET;
}

uint8_t hal_spi_wait_bus_idle(uint32_t spi_x_base,uint32_t  timeout)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));

    unsigned int count = 0;
    while(spi_bsy_getf(spi_x_base) == 1)
    {
        count++;
        if(count >= timeout)
            return HAL_RESET;
    }
    return HAL_SET;
}

void hal_spi_transmit_crc(uint32_t spi_x_base)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    spi_crcnext_setf(spi_x_base,1);
}

void hal_spi_calculate_crc_en(uint32_t spi_x_base,hal_en_t en)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (en)
    {
        case HAL_DISABLE:
            spi_crcen_setf(spi_x_base,0);
            break;
        case HAL_ENABLE:
            spi_crcen_setf(spi_x_base,1);
            break;
        default:
            break;
    }
    
}

uint16_t hal_spi_get_crc(uint32_t spi_x_base,spi_crc_model_t spi_crc_model)
{
    unsigned short crc_reg = 0;
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_CRC_MODEL(spi_crc_model));
    switch (spi_crc_model)
    {
        case SPI_CRC_TX:
            crc_reg = spi_txcrc_getf(spi_x_base);
            break;
        case SPI_CRC_RX:
            crc_reg = spi_rxcrc_getf(spi_x_base);
            break;
        default:
            break;
    }
    return crc_reg;
}



uint16_t hal_spi_get_crc_polynomial(uint32_t spi_x_base)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    return spi_crcpoly_getf(spi_x_base);
}
void hal_spi_set_crc_polynomial(uint32_t spi_x_base,uint32_t crc_poly)
{
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    spi_crcpoly_setf(spi_x_base,crc_poly);
}

uint8_t hal_spi_get_status_flag(uint32_t spi_x_base,spi_status_flag_t spi_status_flag)
{
    unsigned char status_flag = 0;
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_STATUS_FLAG(spi_status_flag));

    switch (spi_status_flag)
    {
        case SPI_STATUS_FLAG_RXNE:
            status_flag = spi_rxne_getf(spi_x_base);
            break;
        case SPI_STATUS_FLAG_TXE:
            status_flag = spi_txe_getf(spi_x_base);
            break;
        case SPI_STATUS_FLAG_CRC_ERR:
            status_flag = spi_crcerr_getf(spi_x_base);
            break;
        case SPI_STATUS_FLAG_MODF:
            //TODO
            break;
        case SPI_STATUS_FLAG_OVR:
            status_flag = spi_ovr_getf(spi_x_base);
            break;
        case SPI_STATUS_FLAG_BSY:
            status_flag = spi_bsy_getf(spi_x_base);
            break;
    default:
        break;
    }
    return status_flag;
}

void hal_spi_clr_status_flag(uint32_t spi_x_base,spi_status_flag_t spi_status_flag)
{
    unsigned char status_flag = 0;
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_STATUS_FLAG(spi_status_flag));

    for(int i = 0 ; i < SPI_STATUS_FLAG_BIT_NUM ; i++)
    {
        status_flag  = (spi_status_flag  & (0x01 << i));

        switch (status_flag)
        {
            case SPI_STATUS_FLAG_CRC_ERR:
                spi_crcerr_setf(spi_x_base,0);
                break;
            default:
                break;
        }
    }
}

uint8_t hal_spi_get_it_flag(uint32_t spi_x_base,spi_it_flag_t spi_it_flag)
{
    unsigned char it_flag = 0;
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_IT_FLAG(spi_it_flag));

    switch (spi_it_flag)
    {
        case SPI_IT_FLAG_RXNE:
            it_flag = spi_rxne_getf(spi_x_base);
            break;
        case SPI_IT_FLAG_TXE:
            it_flag = spi_txe_getf(spi_x_base);
            break;
        case SPI_IT_FLAG_CRC_ERR:
            it_flag = spi_crcerr_getf(spi_x_base);
            break;
        case SPI_IT_FLAG_MODF:
            it_flag = spi_ovr_getf(spi_x_base);
            break;
        case SPI_IT_FLAG_OVR:
            it_flag = spi_ovr_getf(spi_x_base);
            break;
        default:
            break;
    }
    return it_flag;
}

void hal_spi_clr_it_flag(uint32_t spi_x_base,spi_it_flag_t spi_it_flag)
{
    unsigned char it_flag = 0;
    /* Check the parameters */
    hal_assert(IS_SPI_ALL_PERIPH(spi_x_base));
    hal_assert(IS_SPI_IT_FLAG(spi_it_flag));

    for(int i = 0 ; i < SPI_IT_FLAG_BIT_NUM ; i++)
    {
        it_flag  = (spi_it_flag  & (0x01 << i));
        
        switch (it_flag)
        {
            case SPI_IT_FLAG_CRC_ERR:
                spi_crcerr_setf(spi_x_base,0);
                break;
            default:
                break;
        }
    }
}


/************************END OF FILE*******************************/
