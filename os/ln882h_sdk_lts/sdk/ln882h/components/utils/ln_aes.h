/**
 * @file     ln_aes.h
 * @author   LightningSemi
 * @brief    This file provides ADC function.
 * @version  1.0.0
 * @date     2018-05-01
 * 
 * @copyright Copyright (c) 2018-2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#ifndef __LN_AES_H__
#define __LN_AES_H__

#include <stdint.h>
#include <stdlib.h>

#define NUM_ROUNDS         10 // 128 -> 10; 196 -> 12; 256 -> 14
#define KEY_SIZE           16 // 128 -> 16; 192 -> 24; 256 -> 32  uint: byte

#define LN_SW_AES_USING_ROM_CODE (1)

typedef struct
{
  uint32_t keys[60];
  uint32_t ikeys[60];
  uint32_t nrounds;
} ln_aes_context_t;

extern void ln_aes_setup(ln_aes_context_t *ctx, uint8_t keysize, const uint8_t *key);
extern void ln_aes_encrypt(ln_aes_context_t *ctxt, const uint8_t *ptext, uint8_t *ctext);
extern void ln_aes_decrypt(ln_aes_context_t *ctxt, const uint8_t *ctext, uint8_t *ptext);

#endif /* __LN_AES_H__ */
