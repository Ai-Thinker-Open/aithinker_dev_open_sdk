/**
 * @file     hal_aes.c
 * @author   BSP Team 
 * @brief    This file provides AES function.
 * @version  0.0.0.1
 * @date     2021-06-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "hal/hal_aes.h"
#include "string.h"

void hal_aes_init(uint32_t aes_base,aes_init_t_def* aes_init)
{
    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));

    hal_assert(IS_AES_CBC_MODE(aes_init->aes_cbc_mod));
    hal_assert(IS_AES_KEY_LEN(aes_init->aes_key_len));
    hal_assert(IS_AES_OPCODE(aes_init->aes_opcode));
    hal_assert(IS_AES_BIG_ENDIAN(aes_init->aes_big_endian));

    sysc_cmp_aes_gate_en_setf(1);
    for(volatile int i = 0; i < 20; i++)
        __NOP();

    if(aes_init->aes_cbc_mod == AES_CBC_MOD_ECB_MOD){
        aes_cbc_setf(aes_base,0);
    }
    else if (aes_init->aes_cbc_mod == AES_CBC_MOD_CBC_MOD){
        aes_cbc_setf(aes_base,1);
    }

    switch (aes_init->aes_key_len)
    {
        case AES_KEY_LEN_128_BIT:
            aes_keylen_setf(aes_base,0);
            break;
        case AES_KEY_LEN_192_BIT:
            aes_keylen_setf(aes_base,1);
            break;
        case AES_KEY_LEN_256_BIT:
            aes_keylen_setf(aes_base,2);
            break;
        default:
            break;
    }

    if(aes_init->aes_opcode == AES_OPCODE_ENCRYPT){
        aes_opcode_setf(aes_base,0);
    }
    else if (aes_init->aes_opcode == AES_OPCODE_DECRYPT){
        aes_opcode_setf(aes_base,1);
    }
    else if(aes_init->aes_opcode == AES_OPCODE_KEYEXPAND){
        aes_opcode_setf(aes_base,2);
    }
    
    if(aes_init->aes_big_endian == AES_LITTLE_ENDIAN){
        aes_bigendian_setf(aes_base,0);
    }
    else if (aes_init->aes_big_endian == AES_BIG_ENDIAN){
        aes_bigendian_setf(aes_base,1);
    }
}

void hal_aes_deinit(void)
{
    sysc_cmp_srstn_aes_setf(0);
    sysc_cmp_srstn_aes_setf(1);

    sysc_cmp_aes_gate_en_setf(0);
}

void hal_aes_start(uint32_t aes_base)
{
    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));
    aes_start_setf(aes_base,1);
}

void hal_aes_set_plain_text(uint32_t aes_base,uint8_t *data,uint32_t data_len)
{
    uint32_t data_buf[4] = {0,0,0,0};

    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));

    memcpy(data_buf,data,data_len);

    aes_ptr0_setf(aes_base,data_buf[0]);
    aes_ptr1_setf(aes_base,data_buf[1]);
    aes_ptr2_setf(aes_base,data_buf[2]);
    aes_ptr3_setf(aes_base,data_buf[3]);

}

void hal_aes_set_key(uint32_t aes_base,uint8_t *data,uint32_t data_len)
{
    uint32_t data_buf[8] = {0,0,0,0,0,0,0,0};

    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));

    memcpy(data_buf,data,data_len);

    aes_ikr0_setf(aes_base,data_buf[0]);
    aes_ikr1_setf(aes_base,data_buf[1]);
    aes_ikr2_setf(aes_base,data_buf[2]);
    aes_ikr3_setf(aes_base,data_buf[3]);

    aes_ikr4_setf(aes_base,data_buf[4]);
    aes_ikr5_setf(aes_base,data_buf[5]);
    aes_ikr6_setf(aes_base,data_buf[6]);
    aes_ikr7_setf(aes_base,data_buf[7]);
}


void hal_aes_set_vector(uint32_t aes_base,uint8_t *data,uint32_t data_len)
{
    uint32_t data_buf[4] = {0,0,0,0};

    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));

    memcpy(data_buf,data,data_len);

    aes_iv0_setf(aes_base,data_buf[0]);
    aes_iv1_setf(aes_base,data_buf[1]);
    aes_iv2_setf(aes_base,data_buf[2]);
    aes_iv3_setf(aes_base,data_buf[3]);
}

void hal_aes_get_cipher_text(uint32_t aes_base,uint8_t *data,uint32_t *data_len)
{
    uint32_t data_buf[4] = {0,0,0,0};

    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));

    data_buf[0] = aes_ctr0_getf(aes_base);
    data_buf[1] = aes_ctr1_getf(aes_base);
    data_buf[2] = aes_ctr2_getf(aes_base);
    data_buf[3] = aes_ctr3_getf(aes_base);
    
    memcpy(data,data_buf,16);

    *data_len = 16;
}

uint8_t hal_aes_get_status_flag(uint32_t aes_base,aes_status_flag_t aes_status_flag)
{
    /* check the parameters */
    uint8_t it_flag = 0;

    hal_assert(IS_AES_ALL_PERIPH(aes_base));
    hal_assert(IS_AES_STATUS_FLAG(aes_status_flag));
    switch (aes_status_flag)
    {
        case AES_DATA_STATUS_FLAG:
            it_flag = aes_data_int_flg_getf(aes_base);
            break;
        case AES_KEY_STATUS_FLAG:
            it_flag = aes_key_int_flg_getf(aes_base);
            break;
        case AES_BUSY_STATUS_FLAG:
            it_flag = aes_busy_getf(aes_base);
            break;
        default:
            break;
    }
    return it_flag;
}

//interrupt
void hal_aes_it_cfg(uint32_t aes_base,aes_it_flag_t aes_it_flag,hal_en_t en)
{
    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));
    hal_assert(IS_AES_IT_FLAG(aes_it_flag));
    hal_assert(IS_FUNCTIONAL_STATE(en));

    switch (aes_it_flag)
    {
        case AES_DATA_INT_FLAG:
            if(en == HAL_ENABLE){
                aes_data_int_en_setf(aes_base,1);
            }
            else{
                aes_data_int_en_setf(aes_base,0);
            }
            break;
        case AES_KEY_INT_FLAG:
            if(en == HAL_ENABLE){
                aes_key_int_en_setf(aes_base,1);
            }
            else{
                aes_key_int_en_setf(aes_base,0);
            }
            break;
        default:
            break;
    }

}
uint8_t hal_aes_get_it_flag(uint32_t aes_base,aes_it_flag_t aes_it_flag)
{
    /* check the parameters */
    uint8_t it_flag = 0;

    hal_assert(IS_AES_ALL_PERIPH(aes_base));
    hal_assert(IS_AES_IT_FLAG(aes_it_flag));
    switch (aes_it_flag)
    {
        case AES_DATA_INT_FLAG:
            it_flag = aes_data_int_flg_getf(aes_base);
            break;
        case AES_KEY_INT_FLAG:
            it_flag = aes_key_int_flg_getf(aes_base);
            break;
        default:
            break;
    }

    return it_flag;
}
void hal_aes_clr_it_flag(uint32_t aes_base,aes_it_flag_t aes_it_flag)
{
    /* check the parameters */
    hal_assert(IS_AES_ALL_PERIPH(aes_base));
    hal_assert(IS_AES_IT_FLAG(aes_it_flag));

    switch (aes_it_flag)
    {
        case AES_DATA_INT_FLAG:
            aes_data_int_clr_setf(aes_base,0);
            break;
        case AES_KEY_INT_FLAG:
            aes_key_int_clr_setf(aes_base,0);
            break;
        default:
            break;
    }
}
