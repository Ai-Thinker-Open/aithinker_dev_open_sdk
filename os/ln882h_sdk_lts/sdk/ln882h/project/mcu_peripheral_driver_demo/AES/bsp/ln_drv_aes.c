/**
 * @file     ln_drv_aes.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-11-03
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */
#include "ln_drv_aes.h"

/**
 * @brief AES ECB模式加密
 * 
 * @param input  文本输入
 * @param output 文本输出
 * @param key    秘钥
 * @return uint8_t 返回是否加密成功
 */

/**
 * @brief AES ECB模式加密
 * 
 * @param input      文本输入
 * @param input_len  输入文本长度
 * @param output     文本输出
 * @param output_len 文本输出最大长度
 * @param key        秘钥
 * @param key_len    秘钥长度
 * @return uint8_t   返回是否加密成功，0：不成功，1：成功
 */
uint8_t aes_ecb_encrypt(uint8_t *input, uint32_t input_len,
                        uint8_t *output,uint32_t output_len,
                        uint8_t *key,   uint32_t key_len)
{
    uint8_t plain_text[17];
    uint8_t cipher[17];
    uint32_t str_pos,str_len,cipher_len,cipher_pos;
    
    uint8_t key_buf[32];
    memset(&key_buf,0,32);
    memcpy(key_buf,key,key_len);

    aes_init_t_def aes_init;
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_ECB_MOD;     //设置AES为ECB加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_ENCRYPT;      //设置当前模式为加密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）
    hal_aes_init(AES_BASE,&aes_init);               //初始化AES
    hal_aes_set_key(AES_BASE,(uint8_t *)key_buf,32);    

    str_pos = 0;
    str_len = input_len;
    cipher_len = 0;
    cipher_pos = 0;

    memset(plain_text,0,sizeof(plain_text));
    memset(cipher,0,sizeof(cipher));

    while(str_len >= 16)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memcpy(plain_text,input + str_pos,16);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)cipher,&cipher_len);
        memcpy(output + cipher_pos,cipher,cipher_len);
        str_pos += 16;
        cipher_pos += 16;

        str_len -= 16;
        output_len -= 16;
    }

    if(str_len > 0)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memset(plain_text,0,sizeof(plain_text));
        memcpy(plain_text,input + str_pos,str_len);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)cipher,&cipher_len);
        memcpy(output + cipher_pos,cipher,cipher_len);
    }
    return 1;
}

/**
 * @brief AES ECB模式解密
 * 
 * @param input      文本输入
 * @param input_len  输入文本长度
 * @param output     文本输出
 * @param output_len 文本输出最大长度
 * @param key        秘钥
 * @param key_len    秘钥长度
 * @return uint8_t   返回是否解密成功，0：不成功，1：成功
 */
uint8_t aes_ecb_decipher(uint8_t *input, uint32_t input_len,
                         uint8_t *output,uint32_t output_len,
                         uint8_t *key,   uint32_t key_len)
{
    uint8_t plain_text[17];
    uint8_t decipher[17];
    uint32_t str_pos,str_len,decipher_len,decipher_pos;

    uint8_t key_buf[32];
    memset(&key_buf,0,32);
    memcpy(key_buf,key,key_len);

    aes_init_t_def aes_init;
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_ECB_MOD;     //设置AES为ECB加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_DECRYPT;      //设置当前模式为解密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）
    hal_aes_init(AES_BASE,&aes_init);               //初始化AES

    hal_aes_set_key(AES_BASE,key_buf,32); 

    str_pos = 0;
    str_len = strlen((const char *)input);
    decipher_len = 0;
    decipher_pos = 0;

    memset(plain_text,0,sizeof(plain_text));
    memset(decipher,0,sizeof(decipher));

    while(str_len >= 16)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;
            
        memcpy(plain_text,input + str_pos,16);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
        str_pos += 16;
        decipher_pos += 16;

        str_len -= 16;
        output_len -= 16;
    }
    if(str_len > 0)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memset(plain_text,0,sizeof(plain_text));
        memcpy(plain_text,input + str_pos,str_len);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
    }
    return 1;
}


/**
 * @brief AES CBC模式加密
 * 
 * @param input      文本输入
 * @param input_len  输入文本长度
 * @param output     文本输出
 * @param output_len 文本输出最大长度
 * @param key        秘钥
 * @param key_len    秘钥长度
 * @param iv         初始化向量
 * @param iv_len     初始化向量长度
 * @return uint8_t   返回是否加密成功，0：不成功，1：成功
 */
uint8_t aes_cbc_encrypt(uint8_t *input, uint32_t input_len,
                        uint8_t *output,uint32_t output_len,
                        uint8_t *key,   uint32_t key_len,
                        uint8_t *iv,    uint32_t iv_len)
{
    uint8_t plain_text[17];
    uint8_t decipher[17];
    uint32_t str_pos,str_len,decipher_len,decipher_pos;

    uint8_t key_buf[32];
    uint8_t iv_buf[32];

    memset(&key_buf,0,32);
    memset(&iv_buf,0,32);
    memcpy(key_buf,key,key_len);
    memcpy(iv_buf,iv,iv_len);

    aes_init_t_def aes_init;
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_CBC_MOD;     //设置AES为ECB加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_ENCRYPT;      //设置当前模式为加密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）
    hal_aes_init(AES_BASE,&aes_init);               //初始化AES

    hal_aes_set_key(AES_BASE,key_buf,key_len); 
    hal_aes_set_vector(AES_BASE,iv_buf,iv_len);

    str_pos = 0;
    str_len = strlen((const char *)input);
    decipher_len = 0;
    decipher_pos = 0;

    memset(plain_text,0,sizeof(plain_text));
    memset(decipher,0,sizeof(decipher));

    while(str_len >= 16)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memcpy(plain_text,input + str_pos,16);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
        str_pos += 16;
        decipher_pos += 16;

        str_len -= 16;
        output_len -= 16;
    }
    if(str_len > 0)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memset(plain_text,0,sizeof(plain_text));
        memcpy(plain_text,input + str_pos,str_len);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
    }
    return 1;
}

/**
 * @brief AES CBC模式解密
 * 
 * @param input      文本输入
 * @param input_len  输入文本长度
 * @param output     文本输出
 * @param output_len 文本输出最大长度
 * @param key        秘钥
 * @param key_len    秘钥长度
 * @param iv         初始化向量
 * @param iv_len     初始化向量长度
 * @return uint8_t   返回是否解密成功，0：不成功，1：成功
 */
uint8_t aes_cbc_decipher(uint8_t *input, uint32_t input_len,
                         uint8_t *output,uint32_t output_len,
                         uint8_t *key,   uint32_t key_len,
                         uint8_t *iv,    uint32_t iv_len)
{
    uint8_t plain_text[17];
    uint8_t decipher[17];
    uint32_t str_pos,str_len,decipher_len,decipher_pos;

    uint8_t key_buf[32];
    uint8_t iv_buf[32];

    memset(&key_buf,0,32);
    memset(&iv_buf,0,32);
    memcpy(key_buf,key,key_len);
    memcpy(iv_buf,iv,iv_len);

    aes_init_t_def aes_init;
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_CBC_MOD;     //设置AES为ECB加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_DECRYPT;      //设置当前模式为解密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）
    hal_aes_init(AES_BASE,&aes_init);               //初始化AES

    hal_aes_set_key(AES_BASE,(uint8_t *)key,strlen((const char *)key)); 
    hal_aes_set_vector(AES_BASE,(uint8_t *)iv,strlen((const char *)iv));

    str_pos = 0;
    str_len = strlen((const char *)input);
    decipher_len = 0;
    decipher_pos = 0;

    memset(plain_text,0,sizeof(plain_text));
    memset(decipher,0,sizeof(decipher));

    while(str_len >= 16)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memcpy(plain_text,input + str_pos,16);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
        str_pos += 16;
        decipher_pos += 16;

        str_len -= 16;
        output_len -= 16;
    }
    if(str_len > 0)
    {
        //判断output剩余空间是否足够
        if(output_len < 16)
            return 0;

        memset(plain_text,0,sizeof(plain_text));
        memcpy(plain_text,input + str_pos,str_len);
        hal_aes_set_plain_text(AES_BASE,(uint8_t *)plain_text,strlen((const char *)plain_text));
        hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);
        hal_aes_start(AES_BASE);
        while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);
        hal_aes_get_cipher_text(AES_BASE,(uint8_t *)decipher,&decipher_len);
        memcpy(output + decipher_pos,decipher,decipher_len);
    }
    return 1;
}

