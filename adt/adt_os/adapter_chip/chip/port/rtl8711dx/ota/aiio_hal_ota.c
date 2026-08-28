//标准库
#include <stdio.h>
#include <string.h>
//中间件相关
#include "aiio_system.h"
#include "aiio_os_port.h"
#include "aiio_log.h"

//SDK相关
#include "ameba_soc.h"
#include "os_wrapper.h"
#include "flash_api.h"
#include "ameba_ota.h"

extern u32 IMG_ADDR[OTA_IMGID_MAX][2];

static uint8_t rtl_ota_inited=0;
ota_context *ctx = NULL;
static bool first_buff_flag = false;
int32_t first_buff_len =0;
static uint8_t *first_buff = NULL;
static uint8_t *temp_buff = NULL;

 int32_t aiio_init_ota_partition(void)
{
	int32_t ai_ret = AIIO_ERROR;

	if(rtl_ota_inited){
		aiio_log_i("rtl_ota_inited\r\n");
		return AIIO_OK;
	}

	if(ctx){
		aiio_log_e("ota_context not null\r\n");
		return AIIO_ERROR;
	}
	ctx = (ota_context *)rtos_mem_malloc(sizeof(ota_context));
	if (ctx == NULL) {
		aiio_log_e("ctx malloc failed\r\n");
		ai_ret = AIIO_ERROR;
		goto exit;
	}
	memset(ctx, 0, sizeof(ota_context));
	//----------- 下面是用于实现 ameba-rtos\component\soc\amebadplus\misc\ameba_ota.c demo 的 ota_update_init() 函数中与http下载无关的内容 ----------------------------
	flash_get_layout_info(IMG_BOOT, &IMG_ADDR[OTA_IMGID_BOOT][OTA_INDEX_1], NULL);
	flash_get_layout_info(IMG_BOOT_OTA2, &IMG_ADDR[OTA_IMGID_BOOT][OTA_INDEX_2], NULL);
	flash_get_layout_info(IMG_APP_OTA1, &IMG_ADDR[OTA_IMGID_APP][OTA_INDEX_1], NULL);
	flash_get_layout_info(IMG_APP_OTA2, &IMG_ADDR[OTA_IMGID_APP][OTA_INDEX_2], NULL);

	ctx->otactrl = (update_ota_ctrl_info *)rtos_mem_malloc(sizeof(update_ota_ctrl_info));
	if (ctx->otactrl) {
		memset(ctx->otactrl, 0, sizeof(update_ota_ctrl_info));
	} else {
		aiio_log_e("otactrl malloc failed\r\n");
		ai_ret = AIIO_ERROR;
		goto exit;
	}

	ctx->redirect = (update_redirect_conn *)rtos_mem_malloc(sizeof(update_redirect_conn));
	if (ctx->redirect) {
		memset(ctx->redirect, 0, sizeof(update_redirect_conn));
	} else {
		aiio_log_e("redirect malloc failed\r\n");
		ai_ret = AIIO_ERROR;
		goto exit;
	}

	ctx->otaTargetHdr = (update_ota_target_hdr *)rtos_mem_malloc(sizeof(update_ota_target_hdr));
	if (ctx->otaTargetHdr) {
		memset(ctx->otaTargetHdr, 0, sizeof(update_ota_target_hdr));
	} else {
		aiio_log_e("otaTargetHdr malloc failed\r\n");
		ai_ret = AIIO_ERROR;
		goto exit;
	}
	//----------- ota_update_init() 函数功能结束(不包含http下载相关的内容) ----------------------------

	rtl_ota_inited=1;

    return AIIO_OK;

exit:
	if (ctx) {
		ota_update_deinit(ctx);
		rtos_mem_free(ctx);
		ctx=NULL;
	}
	return ai_ret;
}

void aiio_first_packet_processing(const void *src, int32_t size)
{
	int program_size=0;	//本次烧录的数据长度
	update_file_hdr *pOtaFileHdr;
	pOtaFileHdr = (update_file_hdr *)(src);
	ctx->otaTargetHdr->FileHdr.FwVer = pOtaFileHdr->FwVer;
	ctx->otaTargetHdr->FileHdr.HdrNum = pOtaFileHdr->HdrNum;
	aiio_log_d("ota header num: %lu\n", ctx->otaTargetHdr->FileHdr.HdrNum);
	if (ctx->otaTargetHdr->FileHdr.HdrNum > MAX_IMG_NUM) {
		aiio_log_e("ota header num: %lu is invaild\n", ctx->otaTargetHdr->FileHdr.HdrNum);
		return AIIO_ERROR;
	}
	u32 RevHdrLen=(ctx->otaTargetHdr->FileHdr.HdrNum * SUB_HEADER_LEN) + HEADER_LEN;	//包头长度
	/* -------step3: parse firmware file header and get the target OTA image header-----*/
	if (!get_ota_tartget_header(ctx,src, size)) {
		aiio_log_e("get OTA header failed");
		return AIIO_ERROR;
	}
	ctx->otactrl->NextImgLen = size - RevHdrLen;
	if (ctx->otactrl->NextImgLen > 0) {
		memset(ctx->otactrl->NextImgBuf, 0, BUF_SIZE);
		memcpy((void *)ctx->otactrl->NextImgBuf, (void *)(src + RevHdrLen),ctx->otactrl->NextImgLen);
		ctx->otactrl->NextImgFg = 1;
	}
	//----------- ota_update_s1_prepare() 函数功能结束(不包含http下载相关的内容) ----------------------------
	//下面是首包数据接收到后处理包头之后的数据部分
	ctx->otactrl->index = 0;
	ctx->otactrl->IsGetOTAHdr = 0;
	aiio_log_d("download image index : %d", ctx->otactrl->index);
	if (ctx->otactrl->NextImgFg == 1) {
		//本次收到的数据除了包头还有ota文件本体，处理ota文件本体
		program_size = download_packet_process(ctx, ctx->otactrl->NextImgBuf, ctx->otactrl->NextImgLen);
		ctx->otactrl->NextImgFg = 0;
		// download_percentage(program_size, ctx->otactrl->ImageLen);
	}
}

int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size){
	int program_size=0;	//本次烧录的数据长度
	if(0==rtl_ota_inited){
		aiio_init_ota_partition();
	}
	//收到的第一包数据
	if(0==dst_offset){
		//ota文件头的 manifest_info必须完整,不足则等待first_buff拼包，足则正常升级
		if( size < (SUB_HEADER_LEN + HEADER_LEN + sizeof(Manifest_TypeDef)) ){//20240902 SDK升级 update_manifest_info 结构体换为 Manifest_TypeDef
            aiio_log_w("first buf size %d<%d\r\n",size,(SUB_HEADER_LEN + HEADER_LEN + sizeof(Manifest_TypeDef))); 
			first_buff = (uint8_t*)malloc(size);
			if(first_buff == NULL){
				aiio_log_e("malloc failed\r\n");
				return AIIO_ERROR;
			}
			memset(first_buff, 0, size);
			memcpy(first_buff, src, size);	
			first_buff_len = size;
		}else{
			aiio_first_packet_processing(src, size);
			first_buff_flag = true;
		}
		return AIIO_OK; 	//第一包数据处理完成了
	}
	//非首包数据，拼包结束判断，未结束继续拼包，结束则正常升级
	if(!first_buff_flag){
		//首包长度不够需要拼包
		temp_buff = (uint8_t*)malloc(first_buff_len);
		if(temp_buff == NULL){
			aiio_log_e("realloc failed\r\n");
			free(first_buff);
			first_buff = NULL;
			first_buff_len = 0; 
			return AIIO_ERROR;
		}
		memcpy(temp_buff, first_buff, first_buff_len);
		free(first_buff);
		first_buff = NULL;
		first_buff = (uint8_t*)malloc(first_buff_len + size);
		if(first_buff == NULL){
			aiio_log_e("realloc failed\r\n");
			free(temp_buff);
			temp_buff = NULL;
			first_buff_len = 0; 
			return AIIO_ERROR;
		}	
		memcpy(first_buff, temp_buff, first_buff_len);
		memcpy(first_buff + first_buff_len, src, size);
		first_buff_len += size;
		free(temp_buff);
		temp_buff = NULL;
		if( first_buff_len >= (SUB_HEADER_LEN + HEADER_LEN + sizeof(Manifest_TypeDef))){
			aiio_log_i("first buf size %d>=%d,fir buf get over\r\n",size,(SUB_HEADER_LEN + HEADER_LEN + sizeof(Manifest_TypeDef))); 
			aiio_first_packet_processing(first_buff, first_buff_len);
			free(first_buff);  
            first_buff = NULL;  
            first_buff_len = 0;
			first_buff_flag = true;
		}
		return AIIO_OK;
	}else{
		program_size = download_packet_process(ctx, src, size);
		// download_percentage(program_size, ctx->otactrl->ImageLen);
	}
	//aiio_log_d("RemainBytes=%d",ctx->otactrl->RemainBytes);
	if (ctx->otactrl->RemainBytes <= 0) {
		// if (ctx->otactrl->ImgId != OTA_IMGID_APP) {
			//20240902 SDK升级 update_manifest_info 结构体换为 Manifest_TypeDef
			program_size += sizeof(Manifest_TypeDef);    //add the manifest length
		// }
		// download_percentage(program_size, ctx->otactrl->ImageLen);
		aiio_log_d("Update file size: %d bytes, start addr:0x%08x\n", program_size, (unsigned int)(ctx->otactrl->FlashAddr + SPI_FLASH_BASE));
		if ((u32)(program_size) != ctx->otactrl->ImageLen) {
			aiio_log_e("download new firmware failed");
			return AIIO_ERROR;
		}
	}
	return AIIO_OK;
}

int32_t aiio_partition_erase(int32_t start_addr, int32_t size){
	aiio_log_i("skip flash erase,erase in download");
	return AIIO_OK;
}

void aiio_set_boot_partition(void){
	if(0==rtl_ota_inited){
		aiio_log_e("ota init error\r\n");
		return;
	}
	//aiio_log_d("RemainBytes=%d",ctx->otactrl->RemainBytes);
	if (ctx->otactrl->RemainBytes <= 0) {
		if (ctx->otactrl->SkipBootOTAFg) {
			aiio_log_e("Bootloader OTA2 address is invalid, skip Bootloader OTA2");
			return;
		}
		/*----------step4: verify checksum and update signature-----------------*/
		if (verify_ota_checksum(ctx->otaTargetHdr, ctx->otactrl->targetIdx, ctx->otactrl->index)) {
			if (!ota_update_manifest(ctx->otaTargetHdr, ctx->otactrl->targetIdx, ctx->otactrl->index)) {
				aiio_log_e("Change signature failed");
				return;
			}
		}
		/*check if another image is needed to download*/
		aiio_log_d("download image end, total image num: %d",ctx->otaTargetHdr->ValidImgCnt);
		if (ctx) {
			ota_update_deinit(ctx);
			rtos_mem_free(ctx);
			ctx=NULL;
		}
	}else{
		aiio_log_e("image not finish");
		return;
	}
}
