/**
 * @file     ln_drv_aes.h
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_DRV_AES_H
#define __LN_DRV_AES_H

#ifdef __cplusplus
    extern "C" {
#endif /* __cplusplus */

#include "hal/hal_common.h"
#include "hal/hal_aes.h"

uint8_t aes_ecb_encrypt(uint8_t *input, uint32_t input_len,
                        uint8_t *output,uint32_t output_len,
                        uint8_t *key,   uint32_t key_len);

uint8_t aes_ecb_decipher(uint8_t *input, uint32_t input_len,
                         uint8_t *output,uint32_t output_len,
                         uint8_t *key,   uint32_t key_len);

uint8_t aes_cbc_encrypt(uint8_t *input, uint32_t input_len,
                        uint8_t *output,uint32_t output_len,
                        uint8_t *key,   uint32_t key_len,
                        uint8_t *iv,    uint32_t iv_len);

uint8_t aes_cbc_decipher(uint8_t *input, uint32_t input_len,
                         uint8_t *output,uint32_t output_len,
                         uint8_t *key,   uint32_t key_len,
                         uint8_t *iv,    uint32_t iv_len);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__LN_DRV_AES_H
