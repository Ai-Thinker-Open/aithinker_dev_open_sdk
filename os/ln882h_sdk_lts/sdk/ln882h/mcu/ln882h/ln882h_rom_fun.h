#ifndef  __LN882H_ROM_FUN_H__
#define  __LN882H_ROM_FUN_H__

#define ROM_FUN_AES_SETUP           0x00003a39   //    aes_setup,
#define ROM_FUN_AES_ENCRYPT         0x000037b9   //    aes_encrypt,
#define ROM_FUN_AES_DECRYPT         0x0000352d   //    aes_decrypt,
                                    
#define ROM_FUN_MD5_INIT            0x000052a5   //    md5_init,
#define ROM_FUN_MD5_APPEND          0x00005195   //    md5_append,
#define ROM_FUN_MD5_FINISH          0x0000523d   //    md5_finish,
                                    
#define ROM_FUN_SHA1_TRANSFORM      0x000019e9   //    SHA1_Transform,
#define ROM_FUN_SHA1_INIT           0x000019b9   //    SHA1_Init,
#define ROM_FUN_SHA1_UPDATE         0x00002975   //    SHA1_Update,
#define ROM_FUN_SHA1_FINAL          0x000018e5   //    SHA1_Final,
#define ROM_FUN_HMAC_SHA1           0x00004a31   //    hmac_sha1,

#define ROM_FUN_CRC16TAB            0x000073a0   //    crc16tab,           
#define ROM_FUN_CRC32TAB            0x00006fa0   //    Crc32Table,         
#define ROM_FUN_CRC16_CCITT         0x00004039   //    crc16_ccitt,        
#define ROM_FUN_CRC32_INIT          0x000050ef   //    ln_crc32_init,      
#define ROM_FUN_CRC32_UPDATE        0x0000512d   //    ln_crc32_update,    
#define ROM_FUN_CRC32_FINAL         0x000050e5   //    ln_crc32_final,     
#define ROM_FUN_CRC32_SIGNLE_CAL    0x000050f9   //    ln_crc32_signle_cal,

#define ROM_FUN_PRINTF              0x00002a1d   // printf    
#define ROM_FUN_SPRINTF             0x00002a65   // sprintf   
#define ROM_FUN_SNPRINTF            0x00002a3d   // snprintf  
#define ROM_FUN_VSPRINTF            0x00002ab1   // vsprintf  
#define ROM_FUN_VSNPRINTF           0x00002a8d   // vsnprintf 

#define ROM_FUN_ATOF                0x00002b71   //    atof,     T __hardfp_atof   
#define ROM_FUN_ATOI                0x000006af   //    atoi,     T atoi            
#define ROM_FUN_ATOL                0x000006c9   //    atol,     T atol            
#define ROM_FUN_STRTOD              0x00002ba9   //    strtod,   T __hardfp_strtod 
#define ROM_FUN_STRTOF              0x00002bd1   //    strtof,   T __hardfp_strtof 
#define ROM_FUN_STRTOLD             0x00002c09   //    strtold,  T __hardfp_strtold
#define ROM_FUN_STRTOL              0x000005c5   //    strtol,   T strtol          
#define ROM_FUN_STRTOUL             0x00000635   //    strtoul,  T strtoul         
#define ROM_FUN_STRTOLL             0x000006a7   //    strtoll,  T strtoll         
#define ROM_FUN_STRTOULL            0x000006ab   //    strtoull, T strtoull        
#define ROM_FUN_RAND                0x00000191   //    rand,     T rand            
#define ROM_FUN_SRAND               0x000001a5   //    srand,    T srand       

#endif /* __LN882H_ROM_FUN_H__ */
