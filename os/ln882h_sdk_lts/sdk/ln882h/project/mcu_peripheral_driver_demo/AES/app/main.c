/**
 * @file     main.c
 * @author   BSP Team 
 * @brief    
 * @version  0.0.0.1
 * @date     2021-08-05
 * 
 * @copyright Copyright (c) 2021 Shanghai Lightning Semiconductor Technology Co. Ltd
 * 
 */

#include "hal/hal_common.h"
#include "hal/hal_gpio.h"
#include "ln_test_common.h"
#include "ln_show_reg.h"
#include "utils/debug/log.h"
#include "hal/hal_timer.h"
#include "hal/hal_clock.h"

#include "ln_drv_aes.h"

uint8_t input_data[100] = "Shanghai Lightning Semiconductor Technology Co. Ltd";
uint8_t output_data[100];
uint8_t decipher_data[100];
uint8_t key[33] = "FEDCBA0987654321FEDCBA0987654321";
uint8_t iv[16] = "1234567890ABCDEF";

/*
EBC 结果：
D0 DC C4 5C 6F 07 3D D0 A1 AE 8B 98 0F 6A 16 AA 53 45 E3 5F 42 9E 80 C8 4B 3B C4 8F B7 
7B C2 0F 5D E0 79 BB D0 16 AD 16 A3 3F E6 AF C5 77 98 C1 EC 24 C1 D0 66 E2 C4 21 B4 91 
11 7E 79 A5 83 F9

在线加密结果：d0dcc45c6f073dd0a1ae8b980f6a16aa5345e35f429e80c84b3bc48fb77bc20f5de079bbd016ad16a33fe6afc57798c1ec24c1d066e2c421b491117e79a583f9


CBC 结果：
4E 75 A2 8C FA 59 28 BD D3 C6 65 E8 FE 1C CB C9 0B 31 47 24 8B E1 07 E2 35 3B 3D F0 EE 
D0 45 40 8F 62 42 9F FB 68 36 36 18 E4 97 68 1A 3E 80 0F 7B E7 18 93 7E 2C 50 CB 80 D7 
36 78 77 75 A3 17 

在线加密结果：4e75a28cfa5928bdd3c665e8fe1ccbc90b3147248be107e2353b3df0eed045408f62429ffb68363618e497681a3e800f7be718937e2c50cb80d736787775a317
*/


/*
 *  注意！！！ 
 *  在驱动例程中，有一些函数的参数是指向字符串（数组）地址的指针，函数中没有检查这些指针的有效性，
 *  没有判断指针是否为空，也没有判断指针指向的数据是否还有剩余空间，这些都需要使用者自行决定！！！
*/
int main (int argc, char* argv[])
{  
    /****************** 1. 系统初始化 ***********************/
    SetSysClock();
    log_init();   
    LOG(LOG_LVL_INFO,"ln882H init! \n");
    ln_show_reg_init();

    /****************** 2. EBC 加解密测试***********************/
    LOG(LOG_LVL_INFO,"Start ebc encipher !\r\n");
    aes_ecb_encrypt(input_data,strlen((const char*)input_data),output_data,100,key,32);
    LOG(LOG_LVL_INFO,"input_data: %s \r\n",input_data);
    LOG(LOG_LVL_INFO,"key: %s \r\n",key);
    LOG(LOG_LVL_INFO,"output_data: ");
    for(int i = 0; i < (strlen((const char *)input_data) / 16 + 1) *16; i ++)
        LOG(LOG_LVL_INFO,"%02X ",output_data[i]);
    LOG(LOG_LVL_INFO,"\r\n");
    LOG(LOG_LVL_INFO,"\r\n");

    LOG(LOG_LVL_INFO,"Start ebc decipher !\r\n");
    aes_ecb_decipher(output_data,(strlen((const char *)input_data) / 16 + 1) *16,decipher_data,100,key,32);
    LOG(LOG_LVL_INFO,"input_data: %s \r\n",output_data);
    LOG(LOG_LVL_INFO,"key: %s \r\n",key);
    LOG(LOG_LVL_INFO,"decipher_data: %s \r\n",decipher_data);
    LOG(LOG_LVL_INFO,"\r\n");
    
    
    /****************** 3. CBC 加解密测试***********************/
    LOG(LOG_LVL_INFO,"Start cbc encipher !\r\n");
    aes_cbc_encrypt(input_data,strlen((const char*)input_data),output_data,100,key,32,iv,16);
    LOG(LOG_LVL_INFO,"input_data: %s \r\n",input_data);
    LOG(LOG_LVL_INFO,"key: %s \r\n",key);
    LOG(LOG_LVL_INFO,"iv: %s \r\n",iv);
    LOG(LOG_LVL_INFO,"output_data: ");
    for(int i = 0; i < (strlen((const char *)input_data) / 16 + 1) *16; i ++)
        LOG(LOG_LVL_INFO,"%02X ",output_data[i]);
    LOG(LOG_LVL_INFO,"\r\n");
    LOG(LOG_LVL_INFO,"\r\n");

    LOG(LOG_LVL_INFO,"Start cbc decipher !\r\n");
    aes_cbc_decipher(output_data,(strlen((const char *)input_data) / 16 + 1) *16,decipher_data,100,key,32,iv,16);
    LOG(LOG_LVL_INFO,"input_data: %s \r\n",output_data);
    LOG(LOG_LVL_INFO,"key: %s \r\n",key);
    LOG(LOG_LVL_INFO,"iv: %s \r\n",iv);
    LOG(LOG_LVL_INFO,"decipher_data: %s \r\n",decipher_data);
    LOG(LOG_LVL_INFO,"\r\n");

    while(1)
    {
        LOG(LOG_LVL_INFO,"ln882H running! \n");
        
        ln_delay_ms(100);
    }
}
