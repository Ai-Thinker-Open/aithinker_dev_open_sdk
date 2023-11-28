/**
 * @file     hal_aes.h
 * @author   BSP Team 
 * @brief    This file contains all of AES functions prototypes.
 * @version  0.0.0.1
 * @date     2021-06-29
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#ifndef __HAL_AES_H
#define __HAL_AES_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "hal/hal_common.h"
#include "reg_aes.h"

#define IS_AES_ALL_PERIPH(PERIPH) (((PERIPH) == AES_BASE) ) 


typedef enum
{
    AES_CBC_MOD_ECB_MOD     = 0,             
    AES_CBC_MOD_CBC_MOD     = 1,            
}aes_cbc_mod_t;

#define IS_AES_CBC_MODE(MODE)  (((MODE) == AES_CBC_MOD_ECB_MOD) || ((MODE) == AES_CBC_MOD_CBC_MOD))

typedef enum
{
    AES_KEY_LEN_128_BIT     = 0,             
    AES_KEY_LEN_192_BIT     = 1,             
    AES_KEY_LEN_256_BIT     = 2,                   
}aes_key_len_t;

#define IS_AES_KEY_LEN(LEN)  (((LEN) == AES_KEY_LEN_128_BIT) || ((LEN) == AES_KEY_LEN_192_BIT) || ((LEN) == AES_KEY_LEN_256_BIT))

typedef enum
{
    AES_OPCODE_ENCRYPT      = 0,             
    AES_OPCODE_DECRYPT       = 1,             
    AES_OPCODE_KEYEXPAND     = 2,                   
}aes_opcode_t;

#define IS_AES_OPCODE(VALUE)  (((VALUE) == AES_OPCODE_ENCRYPT) || ((VALUE) == AES_OPCODE_DECRYPT) || ((VALUE) == AES_OPCODE_KEYEXPAND))

typedef enum
{
    AES_LITTLE_ENDIAN       = 0,             
    AES_BIG_ENDIAN          = 1,                           
}aes_big_endian_t;

#define IS_AES_BIG_ENDIAN(VALUE)  (((VALUE) == AES_LITTLE_ENDIAN) || ((VALUE) == AES_BIG_ENDIAN))



typedef enum
{

    AES_DATA_STATUS_FLAG           = 0,              // [0] data finish interrupt flag        
    AES_KEY_STATUS_FLAG            = 1,              // [1] key expand finish interrupt flag, write this bit with 0 will clear
    AES_BUSY_STATUS_FLAG           = 2,              // [2] busy indication bit. 0: IP is idle;  1: IP is busy with key expanding or encoding or decoding

}aes_status_flag_t;

#define IS_AES_STATUS_FLAG(FLAG)        (( FLAG <= AES_BUSY_STATUS_FLAG ))

typedef enum
{

    AES_DATA_INT_FLAG           = 0,              // [0] data finish interrupt flag        
    AES_KEY_INT_FLAG            = 1,              // [1] key expand finish interrupt flag, write this bit with 0 will clear

}aes_it_flag_t;

#define IS_AES_IT_FLAG(FLAG)        (( FLAG <= AES_KEY_INT_FLAG ))


typedef struct
{

    aes_cbc_mod_t                       aes_cbc_mod;        /*!< Specifies the AES CBC mode.
                                                            This parameter can be a value of aes_cbc_mod_t */

    aes_key_len_t                       aes_key_len;        /*!< Specifies the AES key mode.
                                                            This parameter can be a value of aes_key_len_t */

    aes_opcode_t                        aes_opcode;         /*!< Specifies the AES opdcode.
                                                            This parameter can be a value of aes_opcode_t */

    aes_big_endian_t                    aes_big_endian;     /*!< Specifies the AES big endian.
                                                            This parameter can be a value of aes_big_endian_t */

}aes_init_t_def;


            //AES init and config
void        hal_aes_init(uint32_t aes_base,aes_init_t_def* aes_init);              
void        hal_aes_deinit(void);
void        hal_aes_start(uint32_t aes_base);

void        hal_aes_set_plain_text(uint32_t aes_base,uint8_t *data,uint32_t data_len);
void        hal_aes_set_key(uint32_t aes_base,uint8_t *data,uint32_t data_len);
void        hal_aes_set_vector(uint32_t aes_base,uint8_t *data,uint32_t data_len);

void        hal_aes_get_cipher_text(uint32_t aes_base,uint8_t *data,uint32_t *data_len);

uint8_t     hal_aes_get_status_flag(uint32_t aes_base,aes_status_flag_t aes_status_flag);

            //interrupt
void        hal_aes_it_cfg(uint32_t aes_base,aes_it_flag_t aes_it_flag,hal_en_t en);
uint8_t     hal_aes_get_it_flag(uint32_t aes_base,aes_it_flag_t aes_it_flag);
void        hal_aes_clr_it_flag(uint32_t aes_base,aes_it_flag_t aes_it_flag);



#ifdef __cplusplus
}
#endif

#endif /* __HAL_AES_H */


/**************************************END OF FILE********************************************/











