/**
 * @file     hal_sdio.c
 * @author   BSP Team 
 * @brief    This file provides SDIO DEVICE function.
 * @version  0.0.0.1
 * @date     2021-03-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

/* Includes ------------------------------------------------------------------*/
#include "hal/hal_sdio_device.h"

uint8_t*   hal_sdio_device_cis_func_get(sdio_dev_func_num_t fn)
{
    uint8_t *fn_reg_base = NULL;

    switch (fn) 
    {
        case SDIO_FUNC0:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn0_get();
            break;
        case SDIO_FUNC1:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn1_get();
            break;
        case SDIO_FUNC2:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn2_get();
            break;
        case SDIO_FUNC3:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn3_get();
            break;
        case SDIO_FUNC4:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn4_get();
            break;
        case SDIO_FUNC5:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn5_get();
            break;
        case SDIO_FUNC6:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn6_get();
            break;
        case SDIO_FUNC7:
            fn_reg_base = (uint8_t *)sdio_sdio_cis_fn7_get();
            break;
    }
    
    return fn_reg_base;
}
uint8_t hal_sdio_device_cis_fn_set(sdio_dev_func_num_t fn,uint32_t offset,uint8_t value)
{
    uint8_t *fn_reg_base = NULL;

    fn_reg_base = hal_sdio_device_cis_func_get(fn);
    
    if(fn_reg_base) 
    {
        *(fn_reg_base + offset) = value;
        return 1;
    }
    return 0;
}


void hal_sdio_device_init(sdio_init_t *sdio_init)
{
    /* check the parameters */
    hal_assert(IS_SDIO_DEV_FUNC_NUM(sdio_init->sdio_dev_func_num));
    
    hal_assert(IS_SDIO_DEV_CSA_SUPPORT(sdio_init->sdio_dev_csa_support));
    hal_assert(IS_SDIO_DEV_SCSI(sdio_init->sdio_dev_scsi));
    hal_assert(IS_SDIO_DEV_SDC(sdio_init->sdio_dev_sdc));
    hal_assert(IS_SDIO_DEV_SMB(sdio_init->sdio_dev_smb));
    hal_assert(IS_SDIO_DEV_SRW(sdio_init->sdio_dev_srw));
    hal_assert(IS_SDIO_DEV_SBS(sdio_init->sdio_dev_sbs));
    hal_assert(IS_SDIO_DEV_S4MI(sdio_init->sdio_dev_s4mi));
    hal_assert(IS_SDIO_DEV_E4MI(sdio_init->sdio_dev_e4mi));
    hal_assert(IS_SDIO_DEV_LSC(sdio_init->sdio_dev_lsc));
    hal_assert(IS_SDIO_DEV_4BLS(sdio_init->sdio_dev_4bls));
    
    sysc_cmp_sdio_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();
    
    sdio_sdio_ior_reg1_set(SDIO_IO_READY);      //FN1 ready
    sdio_sdio_progreg_set(SDIO_DEVICE_READY | SDIO_CARD_READY | SDIO_CPU_IN_ACTIVE | SDIO_SUPPORT_FUNC_NUM);//device ready ahb and arm in active state
    
    sdio_sdio_cccr_pack(0, 0, 0, 0, 0);
    
    sdio_revision_reg_sd_setf(REVISION_REG_SD);
    
    sdio_card_cap_sd_setf(sdio_init->sdio_dev_scsi << 0 | \
                          sdio_init->sdio_dev_sdc  << 1 | \
                          sdio_init->sdio_dev_smb  << 2 | \
                          sdio_init->sdio_dev_srw  << 3 | \
                          sdio_init->sdio_dev_sbs  << 4 | \
                          sdio_init->sdio_dev_s4mi);
    
    sdio_supp_high_speed_setf(1);
    
    //write base address for local RAM3
    sdio_sdio_cis_fn0_set((uint32_t)sdio_init->cis_fn0_base);
    sdio_sdio_cis_fn1_set((uint32_t)sdio_init->cis_fn1_base);
    
    hal_sdio_device_set_recv_buf_addr(sdio_init->recv_buff);
    
    sdio_sdio_fbr_reg1_pack(1, SDIO_FBR1_REG, SDIO_INTERFACE_CODE);
    
    //sdio_device_cis_init();           //CIS needs to be self defined
}

void hal_sdio_device_deinit(void)
{
    sysc_cmp_srstn_sdio_setf(0);
    sysc_cmp_srstn_sdio_setf(1);

    sysc_cmp_sdio_gate_en_setf(0);
}

void hal_sdio_device_clear_busy(void)
{
    sdio_clr_busy_sd_setf(0);
}

void hal_sdio_device_set_busy(void)
{
    sdio_clr_busy_sd_setf(1);
}

uint32_t* hal_sdio_device_get_recv_buf_addr(void)
{
    return (uint32_t *)sdio_sdio_write_fn1_get();
}

void hal_sdio_device_set_recv_buf_addr(uint8_t * addr)
{
    sdio_sdio_write_fn1_set((uint32_t)addr);
}

uint16_t hal_sdio_device_get_recv_buf_size(void)
{
    return sdio_sdio_xfer_cnt_getf();
}

uint32_t* hal_sdio_device_get_send_buf_addr(void)
{
    return (uint32_t *)sdio_sdio_read_fn1_get();
}

void hal_sdio_device_set_send_buf_addr(uint8_t *addr)
{
    sdio_sdio_read_fn1_set((uint32_t)addr);
}

void hal_sdio_device_set_send_buf_size(uint32_t size)
{
    sdio_xfer_cnt_setf(size);
}

void hal_sdio_device_trig_host_data1_int(void)
{
    sdio_iir_reg_setf(SDIO_HOST_IIR_REG_SET);
}

void hal_sdio_device_trig_host_s2m1_int(void)
{
    sdio_s2m1_value_setf(SDIO_HOST_IIR_REG_SET);
}

void hal_sdio_device_it_cfg(uint32_t sdio_dev_int_flag,hal_en_t en)
{
    unsigned int en_status = 0;
    en_status = sdio_sdio_ahb_int_sts_en1_get();
    if(en == HAL_ENABLE)
    {
        en_status |= sdio_dev_int_flag;
    }
    else
    {
        en_status &= ~sdio_dev_int_flag;
    }
    sdio_sdio_ahb_int_sts_en1_set(en_status);
}

uint8_t hal_sdio_device_it_get_flag(sdio_dev_int_flag_t sdio_dev_int_flag)
{
    unsigned char status = 0;
    if((sdio_sdio_ahb_int_sts1_get() & sdio_dev_int_flag) == sdio_dev_int_flag)
    {
        status = 1;
    }
    return status;
}

void hal_sdio_device_it_clr_flag(sdio_dev_int_flag_t sdio_dev_int_flag)
{
    unsigned int flag = sdio_sdio_ahb_int_sts1_get();
    flag &= ~sdio_dev_int_flag;
    sdio_sdio_ahb_int_sts1_set(flag);
}
