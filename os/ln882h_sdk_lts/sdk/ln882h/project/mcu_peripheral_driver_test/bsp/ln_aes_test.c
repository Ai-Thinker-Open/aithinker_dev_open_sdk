/**
 * @file     ln_aes_test.c
 * @author   BSP Team 
 * @brief 
 * @version  0.0.0.1
 * @date     2021-08-24
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */


/*
        AES使用说明：
        
                1. 需要注意的是，加解密操作一次只能操作16字节，如果超过16字节要重复加密的过程。
                
                2. 解密的时候，要重新设置秘钥和初始化向量。
                
                
                3. 如下为网上验证过的AES加密结果，可以试验对比
                
                    使用CBC方式加密,秘钥长度256bit
                    明文：1234567890ABCDEF 
                    秘钥：FEDCBA0987654321FEDCBA0987654321
                    初始化向量：1234567890ABCDEF
                    加密之后的密文：A5 CB 75 C3 26 A5 2F 1C  51 FB E2 5F 78 08 0F FE 
                    
                    使用ECB方式加密,秘钥长度256bit
                    明文：1234567890ABCDEF 
                    秘钥：FEDCBA0987654321FEDCBA0987654321
                    初始化向量：1234567890ABCDEF
                    加密之后的密文：E4 BE A6 DD A0 F4 4C 59  7B 9D 51 8C B9 38 52 80 


*/

#include "hal_aes.h"
#include "ln_test_common.h"
#include "utils/debug/log.h"


// 输入明文
uint8_t input[33]  = "1234567890ABCDEF";
uint8_t output[33];
const uint8_t input_backup[33] = "11112222333344445555666677778888";

// 加密用到的key 和初始化向量
const uint8_t key[33] = "FEDCBA0987654321FEDCBA0987654321";
const uint8_t iv[17] = "1234567890ABCDEF";

uint8_t out[33];
uint32_t out_len = 0;

uint8_t decrpt_out[32];

void ln_aes_test()
{

    /* 1. 初始化AES模块 */
    aes_init_t_def aes_init;
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_ECB_MOD;     //设置AES为CBC加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_ENCRYPT;     //设置当前模式为加密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）

    hal_aes_init(AES_BASE,&aes_init);               //初始化AES
    
    
    /* 2. 设置要加密的字符串，加密秘钥，和初始化向量 */
    LOG(LOG_LVL_INFO,"set key: %s \r\n",key);
    LOG(LOG_LVL_INFO,"set iv: %s \r\n",iv);
    LOG(LOG_LVL_INFO,"set plain text: %s \r\n",input);
    
    hal_aes_set_key(AES_BASE,(uint8_t *)key,strlen((const char *)key));             //设置加密秘钥
    hal_aes_set_vector(AES_BASE,(uint8_t *)iv,strlen((const char *)iv));            //设置初始化向量
    hal_aes_set_plain_text(AES_BASE,(uint8_t *)input,strlen((const char *)input));  //设置要加密的字符串，明文需要小于128bit,如果大于128bit，则需要循环这个流程，重新开始。
    
    /* 3. 配置中断（加密完成后产生标志位） */
    hal_aes_it_cfg(AES_BASE,AES_DATA_INT_FLAG,HAL_ENABLE);

    /* 4. 启动加密 */
    hal_aes_start(AES_BASE);
    
    /* 5. 等待加密完成 */
    while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);

    /* 6. 得到加密之后的密文 */
    hal_aes_get_cipher_text(AES_BASE,(uint8_t *)output,&out_len);
    
    LOG(LOG_LVL_INFO,"get cipher text:");
    for(int i = 0; i < 16; i++)
        LOG(LOG_LVL_INFO,"%X ",output[i]);
    
    hal_aes_clr_it_flag(AES_BASE,AES_DATA_INT_FLAG);
        
    /* 7. 开始解密密文，首先初始化AES模块，配置为解密模式 */   
    memset(&aes_init,0,sizeof(aes_init));

    aes_init.aes_cbc_mod = AES_CBC_MOD_ECB_MOD;     //设置AES为CBC加密模式
    aes_init.aes_key_len = AES_KEY_LEN_256_BIT;     //设置密码长度为256bit
    aes_init.aes_opcode  = AES_OPCODE_DECRYPT;      //设置当前模式为解密模式
    aes_init.aes_big_endian = AES_BIG_ENDIAN;       //默认配置（不要更改）

    hal_aes_init(AES_BASE,&aes_init);               //初始化AES    
    
    /* 8. 设置要解密的密文，其余的秘钥和初始化向量不变 */   
    hal_aes_set_plain_text(AES_BASE,(uint8_t *)output,strlen((const char *)output));
    hal_aes_set_key(AES_BASE,(uint8_t *)key,strlen((const char *)key));             //设置加密秘钥
    hal_aes_set_vector(AES_BASE,(uint8_t *)iv,strlen((const char *)iv));            //设置初始化向量
    
    /* 9. 启动解密 */
    hal_aes_start(AES_BASE);
    
    /* 10. 等待加密完成 */
    while(hal_aes_get_status_flag(AES_BASE,AES_DATA_STATUS_FLAG) == HAL_RESET);

    /* 11. 得到加密之后的密文 */
    hal_aes_get_cipher_text(AES_BASE,(uint8_t *)output,&out_len);
    
    LOG(LOG_LVL_INFO,"\r\nget decipher text: %s",output);
    

    while(1)
    {
        ln_delay_ms(500);
    }

}

