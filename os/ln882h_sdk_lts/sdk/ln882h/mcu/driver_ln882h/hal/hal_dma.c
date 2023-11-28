/**
 * @file     hal_dma.c
 * @author   BSP Team 
 * @brief    This file provides DMA function.
 * @version  0.0.0.1
 * @date     2020-12-15
 * 
 * @copyright Copyright (c) 2020 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_dma.h"
     
void hal_dma_init(uint32_t dma_x_base,dma_init_t_def* dma_init)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));

    /* Check the dma parameters */
    hal_assert(IS_DMA_MEM_TO_MEM_EN(dma_init->dma_mem_to_mem_en));
    hal_assert(IS_DMA_PRI_LEV(dma_init->dma_pri_lev));
    hal_assert(IS_DMA_MEM_SIZE(dma_init->dma_mem_size));
    hal_assert(IS_DMA_P_SIZE(dma_init->dma_p_size));
    hal_assert(IS_DMA_MEM_INC_EN(dma_init->dma_mem_inc_en));
    hal_assert(IS_DMA_P_INC_EN(dma_init->dma_p_inc_en));
    hal_assert(IS_DMA_CIRC_MODE_EN(dma_init->dma_circ_mode_en));
    hal_assert(IS_DMA_DIR(dma_init->dma_dir));
    hal_assert(IS_DMA_NDT_VALUE(dma_init->dma_data_num));
    hal_assert(IS_DMA_MA_VALUE(dma_init->dma_mem_addr));
    hal_assert(IS_DMA_PA_VALUE(dma_init->dma_p_addr));

    sysc_cmp_dma_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    //set the parameter
    if (dma_init->dma_mem_to_mem_en == DMA_MEM_TO_MEM_DIS) {
        dma_mem2mem_setf(dma_x_base,0);
    }
    else if(dma_init->dma_mem_to_mem_en == DMA_MEM_TO_MEM_EN) {
        dma_mem2mem_setf(dma_x_base,1);
    }
    switch (dma_init->dma_pri_lev)
    {
        case DMA_PRI_LEV_LOW:
            dma_pl_setf(dma_x_base,0);
            break;
        case DMA_PRI_LEV_MEDIUM:
            dma_pl_setf(dma_x_base,1);
            break;
        case DMA_PRI_LEV_HIGH:
            dma_pl_setf(dma_x_base,2);
            break;
        default:
            break;
    }
    switch (dma_init->dma_mem_size)
    {
        case DMA_MEM_SIZE_8_BIT:
            dma_msize_setf(dma_x_base,0);
            break;
        case DMA_MEM_SIZE_16_BIT:
            dma_msize_setf(dma_x_base,1);
            break;
        case DMA_MEM_SIZE_32_BIT:
            dma_msize_setf(dma_x_base,2);
            break;
        default:
            break;
    }
    switch (dma_init->dma_p_size)
    {
        case DMA_P_SIZE_8_BIT:
            dma_psize_setf(dma_x_base,0);
            break;
        case DMA_P_SIZE_16_BIT:
            dma_psize_setf(dma_x_base,1);
            break;
        case DMA_P_SIZE_32_BIT:
            dma_psize_setf(dma_x_base,2);
            break;
        default:
            break;
    }
    if (dma_init->dma_mem_inc_en == DMA_MEM_INC_DIS) {
        dma_minc_setf(dma_x_base,0);
    }
    else if (dma_init->dma_mem_inc_en == DMA_MEM_INC_EN) {
        dma_minc_setf(dma_x_base,1);
    }

    if (dma_init->dma_p_inc_en == DMA_P_INC_DIS) {
        dma_pinc_setf(dma_x_base,0);
    }
    else if (dma_init->dma_p_inc_en == DMA_P_INC_EN) {
        dma_pinc_setf(dma_x_base,1);
    }

    if (dma_init->dma_circ_mode_en == DMA_CIRC_MODE_DIS) {
        dma_circ_setf(dma_x_base,0);
    }
    else if (dma_init->dma_circ_mode_en == DMA_CIRC_MODE_EN) {
        dma_circ_setf(dma_x_base,1);
    }

    if (dma_init->dma_dir == DMA_READ_FORM_P) {
        dma_dir_setf(dma_x_base,0);
    }
    else if (dma_init->dma_dir == DMA_READ_FORM_MEM) {
        dma_dir_setf(dma_x_base,1);
    }
    dma_ndt_setf(dma_x_base,dma_init->dma_data_num);
    dma_pa_setf(dma_x_base,dma_init->dma_p_addr);
    dma_ma_setf(dma_x_base,dma_init->dma_mem_addr);
}

void hal_dma_deinit(void)
{
    sysc_cmp_srstn_dma_setf(0);
    sysc_cmp_srstn_dma_setf(1);

    sysc_cmp_dma_gate_en_setf(0);
}

void hal_dma_en(uint32_t dma_x_base,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_FUNCTIONAL_STATE(en));
    if (en == HAL_DISABLE) {
        dma_en_setf(dma_x_base,0);
    }
    else if (en == HAL_ENABLE) {
        dma_en_setf(dma_x_base,1);
    }
}

void hal_dma_set_dir(uint32_t dma_x_base,dma_dir_t dma_dir)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_DIR(dma_dir));

    if (dma_dir == DMA_READ_FORM_P) {
        dma_dir_setf(dma_x_base,0);
    }
    else if (dma_dir == DMA_READ_FORM_MEM) {
        dma_dir_setf(dma_x_base,1);
    }
}

void hal_dma_set_mem_addr(uint32_t dma_x_base,uint32_t dma_mem_addr)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_MA_VALUE(dma_mem_addr));

    dma_ma_setf(dma_x_base,dma_mem_addr);
}   

void hal_dma_set_p_addr(uint32_t dma_x_base,uint32_t dma_p_addr)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_PA_VALUE(dma_p_addr));

    dma_pa_setf(dma_x_base,dma_p_addr);
}

void hal_dma_set_data_num(uint32_t dma_x_base,uint16_t dma_data_num)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_NDT_VALUE(dma_data_num));

    dma_ndt_setf(dma_x_base,dma_data_num);
}

void hal_dma_set_mem_size(uint32_t dma_x_base,dma_mem_size_t dma_mem_size)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_MEM_SIZE(dma_mem_size));
    switch (dma_mem_size)
    {
        case DMA_MEM_SIZE_8_BIT:
            dma_msize_setf(dma_x_base,0);
            break;
        case DMA_MEM_SIZE_16_BIT:
            dma_msize_setf(dma_x_base,1);
            break;
        case DMA_MEM_SIZE_32_BIT:
            dma_msize_setf(dma_x_base,2);
            break;
        default:
            break;
    }
}


void hal_dma_set_p_size(uint32_t dma_x_base,dma_p_size_t dma_p_size)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_P_SIZE(dma_p_size));
    switch (dma_p_size)
    {
        case DMA_P_SIZE_8_BIT:
            dma_psize_setf(dma_x_base,0);
            break;
        case DMA_P_SIZE_16_BIT:
            dma_psize_setf(dma_x_base,1);
            break;
        case DMA_P_SIZE_32_BIT:
            dma_psize_setf(dma_x_base,2);
            break;
        default:
            break;
    }
}

void hal_dma_set_pri_lev(uint32_t dma_x_base,dma_pri_lev_t dma_pri_lev)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_PRI_LEV(dma_pri_lev));
    switch (dma_pri_lev)
    {
        case DMA_PRI_LEV_LOW:
            dma_pl_setf(dma_x_base,0);
            break;
        case DMA_PRI_LEV_MEDIUM:
            dma_pl_setf(dma_x_base,1);
            break;
        case DMA_PRI_LEV_HIGH:
            dma_pl_setf(dma_x_base,2);
            break;
        default:
            break;
    }
}

void hal_dma_set_p_inc_en(uint32_t dma_x_base,dma_p_inc_en_t dma_p_inc_en)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_P_INC_EN(dma_p_inc_en));
    if (dma_p_inc_en == DMA_P_INC_DIS) {
        dma_pinc_setf(dma_x_base,0);
    }
    else if (dma_p_inc_en == DMA_P_INC_EN) {
        dma_pinc_setf(dma_x_base,1);
    }
}

void hal_dma_set_mem_inc_en(uint32_t dma_x_base,dma_mem_inc_en_t dma_mem_inc_en)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_MEM_INC_EN(dma_mem_inc_en));
    if (dma_mem_inc_en == DMA_MEM_INC_DIS) {
        dma_minc_setf(dma_x_base,0);
    }
    else if (dma_mem_inc_en == DMA_MEM_INC_EN) {
        dma_minc_setf(dma_x_base,1);
    }
}

uint16_t hal_dma_get_data_num(uint32_t dma_x_base)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    return dma_ndt_getf(dma_x_base);
}

void hal_dma_it_cfg(uint32_t dma_x_base,dma_it_flag_t dma_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_IT_FLAG_VALUE(dma_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (dma_it_flag)
    {
        case DMA_IT_FLAG_TRAN_ERR:
            if (en == HAL_DISABLE) {
                dma_teie_setf(dma_x_base,0);
            }
            else if (en == HAL_ENABLE) {
                dma_teie_setf(dma_x_base,1);
            }
            break;
        case DMA_IT_FLAG_TRAN_HALF:
            if (en == HAL_DISABLE) {
                dma_htie_setf(dma_x_base,0);
            }
            else if (en == HAL_ENABLE) {
                dma_htie_setf(dma_x_base,1);
            }
            break;
        case DMA_IT_FLAG_TRAN_COMP:
            if (en == HAL_DISABLE) {
                dma_tcie_setf(dma_x_base,0);
            }
            else if (en == HAL_ENABLE) {
                dma_tcie_setf(dma_x_base,1);
            }
        case DMA_IT_FLAG_TRAN_GLOB:
            break;
        
        default:
            break;
    }
            
     
}

uint8_t hal_dma_get_it_flag(uint32_t dma_x_base,dma_it_flag_t dma_it_flag)
{
    uint8_t it_flag = 0;
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_IT_FLAG_VALUE(dma_it_flag));

    switch (dma_x_base)
    {
        case DMA_CH_1:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif1_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif1_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif1_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif1_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_2:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif2_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif2_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif2_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif2_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }    
        case DMA_CH_3:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif3_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif3_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif3_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif3_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_4:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif4_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif4_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif4_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif4_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_5:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif5_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif5_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif5_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif5_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_6:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif6_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif6_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif6_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif6_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_7:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    it_flag = dma_teif7_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    it_flag = dma_htif7_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    it_flag = dma_tcif7_getf(DMA_IT);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    it_flag = dma_gif7_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        
        default:
            break;
    }
    return it_flag;
}
void hal_dma_clr_it_flag(uint32_t dma_x_base,dma_it_flag_t dma_it_flag)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_DMA_IT_FLAG_VALUE(dma_it_flag));

    switch (dma_x_base)
    {
        case DMA_CH_1:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 0);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 0);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 0);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 0);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_2:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 4);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 4);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 4);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 4);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_3:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 8);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 8);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 8);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 8);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_4:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 12);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 12);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 12);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 12);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_5:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 16);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 16);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 16);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 16);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_6:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 20);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 20);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 20);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 20);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_7:
        {
            switch (dma_it_flag)
            {
                case DMA_IT_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 24);
                    break;
                case DMA_IT_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 24);
                    break;
                case DMA_IT_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 24);
                    break;
                case DMA_IT_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 24);
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

uint8_t hal_dma_get_status_flag(uint32_t dma_x_base,dma_status_flag_t dma_status_flag)
{
    uint8_t status_flag = 0;
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_STATUS_FLAG_VALUE(dma_status_flag));

    switch (dma_x_base)
    {
        case DMA_CH_1:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif1_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif1_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif1_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif1_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_2:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif2_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif2_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif2_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif2_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_3:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif3_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif3_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif3_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif3_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_4:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif4_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif4_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif4_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif4_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_5:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif5_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif5_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif5_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif5_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_6:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif6_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif6_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif6_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif6_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_7:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    status_flag = dma_teif7_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    status_flag = dma_htif7_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    status_flag = dma_tcif7_getf(DMA_IT);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    status_flag = dma_gif7_getf(DMA_IT);
                    break;
                default:
                    break;
            }
            break;
        }
        
        default:
            break;
    }
    return status_flag;
}

void hal_dma_clr_status_flag(uint32_t dma_x_base,dma_status_flag_t dma_status_flag)
{
    /* check the parameters */
    hal_assert(IS_DMA_ALL_PERIPH(dma_x_base));
    hal_assert(IS_STATUS_FLAG_VALUE(dma_status_flag));

    switch (dma_x_base)
    {
        case DMA_CH_1:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 0);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 0);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 0);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 0);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_2:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 4);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 4);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 4);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 4);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_3:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 8);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 8);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 8);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 8);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_4:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 12);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 12);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 12);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 12);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_5:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 16);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 16);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 16);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 16);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_6:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 20);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 20);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 20);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 20);
                    break;
                default:
                    break;
            }
            break;
        }
        case DMA_CH_7:
        {
            switch (dma_status_flag)
            {
                case DMA_STATUS_FLAG_TRAN_ERR:
                    dma_ifcr_set(DMA_IT,8 << 24);
                    break;
                case DMA_STATUS_FLAG_TRAN_HALF:
                    dma_ifcr_set(DMA_IT,4 << 24);
                    break;
                case DMA_STATUS_FLAG_TRAN_COMP:
                    dma_ifcr_set(DMA_IT,2 << 24);
                    break;
                case DMA_STATUS_FLAG_TRAN_GLOB:
                    dma_ifcr_set(DMA_IT,1 << 24);
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


/************************END OF FILE*******************************/

