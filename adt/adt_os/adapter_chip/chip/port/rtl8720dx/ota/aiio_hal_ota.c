/** @brief      ota application interface.
 *
 *  @file       aiio_hal_ota.c
 *  @copyright  Copyright (C) 2020-2023, Shenzhen Anxinke Technology Co., Ltd
 *  @note       Wifi application interface.
 *  @par        Change Logs:
 *  <table>
 *  <tr><th>Date            <th>Version     <th>Author      <th>Notes
 *  <tr><td>2024/04/23      <td>1.0.0       <td>specter       <td>frist version
 *  </table>
 *
 */
//标准库
#include <stdio.h>
#include <string.h>
//中间件相关
#include "aiio_system.h"
#include "aiio_os_port.h"
//sdk相关
#include <device_lock.h>
#include "flash_api.h"
#include "rtl8721d_ota.h"

extern const u32 IMG_ADDR[MAX_IMG_NUM][2];

static update_ota_target_hdr OtaTargetHdr;
static uint32_t ota_target_index = OTA_INDEX_2;

/*int32_t aiio_init_ota_partition(void)
{
    return AIIO_OK;
}*/


int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size)
{
    static unsigned char ota_header[32];
	static uint32_t NewFWAddr = 0;
	flash_t flash_ota;
	
	//AI_DataPrint(AI_PRINT_LEVE_DEBUG,AI_DATA_CH_LOG,"[%s()-%d]dst_offset=%d size=%d'\r\n",__func__,__LINE__,dst_offset,size);
	if(0==dst_offset){
		if(size<sizeof(ota_header)+8){
            aiio_log_e("first buf size %d<%d\r\n",size,sizeof(ota_header)+8);
			return AIIO_ERROR;	//首次数据必须大于32字节
		}
		memset(ota_header,0,sizeof(ota_header));
		memset(&OtaTargetHdr,0,sizeof(update_ota_target_hdr));

		//-------------------step2:获取固件头---------------------
		if (ota_get_cur_index() == OTA_INDEX_1) {
			ota_target_index = OTA_INDEX_2;
		} else {
			ota_target_index = OTA_INDEX_1;
		}
		memcpy(ota_header,src,sizeof(ota_header));
		OtaTargetHdr.FileHdr.FwVer = ota_header[0]|ota_header[1]<<8|ota_header[2]<<16|ota_header[3]<<24;
		OtaTargetHdr.FileHdr.HdrNum= ota_header[4]|ota_header[5]<<8|ota_header[6]<<16|ota_header[7]<<24;
		
		//--------------step3: parse firmware file header and get the target OTA image header------------	
		if(!get_ota_tartget_header(ota_header, 32, &OtaTargetHdr, ota_target_index)) {
            aiio_log_e("get OTA header failed\r\n");
			return AIIO_ERROR;
		}
		//收到的数据偏移过头部32字节
		src+=sizeof(ota_header);
		dst_offset+=sizeof(ota_header);
		size-=sizeof(ota_header);
		
		memcpy(OtaTargetHdr.Sign[0],src,8);
		//收到的数据偏移OtaTargetHdr.Sign[0]的8字节
		src+=8;
		dst_offset+=8;
		size-=8;

		NewFWAddr = OtaTargetHdr.FileImgHdr->FlashAddr;
	}
	//-------------------------step5: copy image to flash-------------------------
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	if(flash_burst_write(&flash_ota, NewFWAddr+8 + (dst_offset-sizeof(ota_header)-8)-SPI_FLASH_BASE, size, src) < 0){
		device_mutex_unlock(RT_DEV_LOCK_FLASH);
        aiio_log_e("write flash error\r\n");
		return AIIO_ERROR;
	}
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
    return AIIO_OK;
}


int32_t aiio_partition_erase(int32_t start_addr, int32_t size)
{
    uint32_t NewFWAddr = 0;
	uint32_t NewFWBlkSize = 0;
	uint32_t ota_target_index = OTA_INDEX_2;
	flash_t flash_ota;

	if(size>2024*1024){
		//固件大小超出固件分区大小了
        aiio_log_e("image size overload,skip flash erase\r\n");
		return AIIO_ERROR;
	}
	
	//根据挡墙镜像判断要擦除的镜像位置
	if (ota_get_cur_index() == OTA_INDEX_1) {
		ota_target_index = OTA_INDEX_2;
        aiio_log_i("OTA2 address space will be erase\r\n");
	} else {
		ota_target_index = OTA_INDEX_1;
        aiio_log_i("OTA1 address space will be erase\r\n");
	}

	//------------------------step4: erase flash space for new firmware------------------------
	//擦除flash
	NewFWAddr = IMG_ADDR[0][ota_target_index];	//根据镜像位置确定flash擦除起始未知
	NewFWBlkSize = ((size - 1)/4096) + 2;	//根据长度计算要擦除的扇区大小
	aiio_log_i("erase size=%d bytes NewFWBlkSize=%d\r\n",size,NewFWBlkSize);
	device_mutex_lock(RT_DEV_LOCK_FLASH);
	for(int i = 0; i < NewFWBlkSize; i++){
		flash_erase_sector(&flash_ota, NewFWAddr + i * 4096);
	}
	device_mutex_unlock(RT_DEV_LOCK_FLASH);
    return AIIO_OK;
}

//设置启动分区
void aiio_set_boot_partition(void)
{
    //对固件进行校验，校验成功后切换分区
	if(verify_ota_checksum(&OtaTargetHdr)){
		if(!change_ota_signature(&OtaTargetHdr, ota_target_index)) {
            aiio_log_e("Change signature failed\r\n");
			return;
		}
        aiio_log_i("Change signature success\r\n");
		return;
	} else{
		//固件校验失败
        aiio_log_e("verify_ota_checksum() error\r\n");
		return;
	}
}
