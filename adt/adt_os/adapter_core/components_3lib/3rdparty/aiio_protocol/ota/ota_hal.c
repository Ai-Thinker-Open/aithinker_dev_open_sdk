#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

#include "aiio_system.h"
#include "aiio_os_port.h"
#include "aiio_platform_ota.h"

int aiio_hosal_random_num_read(void *buf, uint32_t bytes)
{
    uint32_t tmp;
    uint8_t *val;
    int i, nwords;

	if(buf == NULL)
	{
		return -1;
	}
    
    val = (uint8_t *)buf;

    nwords = bytes / 4;

    for (i = 0; i < nwords; i++) {
        tmp = aiio_random();
        memcpy(val, &tmp, sizeof(tmp));
        val += sizeof(tmp);
    }

    tmp = aiio_random();
   
    memcpy(val, &tmp, bytes % 4);
   
    return 0;
}


int aiio_hal_random(void *p_rng,uint8_t *output,uint32_t output_len){
	(void)p_rng;
	return aiio_hosal_random_num_read(output, output_len);
}


static int init_ota_partition(void)
{
    return aiio_init_ota_partition();
}

//#if defined(MBEDTLS_RANDOM_CONFIG) 
/*描述：硬件随机接口，由各个平台适配，根据output_len参数生成多少位随机数
*p_rng：rng随机数句柄，无需处理
*output:指向随机数存储空间指针
*output_len:生产随机数位数
*/
int ai_random(void *p_rng, unsigned char *output, long unsigned int output_len)
{
    ( void ) p_rng;
    return aiio_hal_random(p_rng,output,output_len);
}
//#endif

/*
*描述：设置自己内存实现的API
*当前该API只有RTL平台需要使用，其它平台无需实现
*/
void ai_paltform_set_calloc_free(void)
{
    // mbedtls_platform_set_calloc_free(my_calloc, vPortFree);
    return;
}

/*******************************
*描述：写flash回调函数，由各平台适配
*dst_offset：偏移地址
*src：写flash的数据
*size：数据长度
*return: -1:failed   0:success
********************************/
int partition_write_ota_farmware(int dst_offset, const void *src, int size)
{
    return aiio_partition_write_ota_farmware(dst_offset,src,size);
}


/*******************************
*描述：擦除flash回调函数，由各平台适配
*start_addr：擦除起始地址
*size：擦除长度
*return: -1:failed   0:success
********************************/
int partition_erase(int start_addr, int size)
{
    return aiio_partition_erase(start_addr,size);
}


/*******************************
*描述：设置启动分区，由各平台适配
********************************/
void set_boot_partition(void)
{
    aiio_set_boot_partition();
}


/*******************************
*描述：重启，由各平台适配
*ota_result:    true:OTA成功
                false:OTA失败
********************************/
void set_reboot(_Bool ota_result)
{
    aiio_os_tick_dealy(aiio_os_ms2tick(1000));
    aiio_restart();
}
