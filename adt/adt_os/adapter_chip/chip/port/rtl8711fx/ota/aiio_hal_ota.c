//标准库
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
//中间件相关
#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_os_port.h"

//SDK相关
#include "ameba_soc.h"
#include "flash_api.h"
#include "ota_internal.h"

// IMG Flash Physical Address use for OTA
extern u32 IMG_ADDR[OTA_IMGID_MAX][2];

typedef struct
{
	uint8_t inited;

	// Parsed from OTA file header
	ota_hdr_manager_t hdr_mgr;
	uint8_t hdr_parsed;

	// For header buffering (first packet may be small)
	uint8_t *hdr_buf;
	uint32_t hdr_buf_len;
	uint32_t hdr_need_len;

	// Per image download state (indexed by hdr index, not ImgID)
	uint32_t slot_idx[OTA_IMGID_MAX];
	uint32_t image_base_phy[OTA_IMGID_MAX]; // physical start address of target slot

	// Per image progress (bytes received in this image, includes manifest)
	uint32_t recv_in_image[OTA_IMGID_MAX];

	// Erase management (physical address of next sector to erase)
	uint32_t next_erase_addr[OTA_IMGID_MAX];
} aiio_rtl8711fx_ota_ctx_t;

static aiio_rtl8711fx_ota_ctx_t g_ota_ctx;

static void ota_state_reset(void)
{
	memset(&g_ota_ctx, 0, sizeof(g_ota_ctx));
}

static int ensure_ota_inited(void)
{
	if (g_ota_ctx.inited) {
		return AIIO_OK;
	}

	ota_state_reset();

	if (ota_storage_init() != OTA_OK) {
		aiio_log_e("ota_storage_init failed\r\n");
		return AIIO_ERROR;
	}

	g_ota_ctx.inited = 1;
	return AIIO_OK;
}

static int ota_parse_file_headers(const uint8_t *buf, uint32_t len)
{
	// OTA file format (green2):
	//   ota_hdr_t (8 bytes) + ota_sub_hdr_t * HdrNum (24 bytes each)
	if (len < OTA_HEADER_LEN) {
		return AIIO_ERROR;
	}

	const ota_hdr_t *file_hdr = (const ota_hdr_t *)buf;
	g_ota_ctx.hdr_mgr.FileHdr.FwVer = file_hdr->FwVer;
	g_ota_ctx.hdr_mgr.FileHdr.HdrNum = file_hdr->HdrNum;

	if (g_ota_ctx.hdr_mgr.FileHdr.HdrNum == 0 || g_ota_ctx.hdr_mgr.FileHdr.HdrNum > OTA_IMGID_MAX) {
		aiio_log_e("invalid ota hdr num: %lu\r\n", (unsigned long)g_ota_ctx.hdr_mgr.FileHdr.HdrNum);
		return AIIO_ERROR;
	}

	const uint32_t need = OTA_HEADER_LEN + g_ota_ctx.hdr_mgr.FileHdr.HdrNum * OTA_SUB_HEADER_LEN;
	if (len < need) {
		return AIIO_ERROR;
	}

	memset(g_ota_ctx.hdr_mgr.FileImgHdr, 0, sizeof(g_ota_ctx.hdr_mgr.FileImgHdr));
	memset(g_ota_ctx.hdr_mgr.Manifest, 0, sizeof(g_ota_ctx.hdr_mgr.Manifest));

	uint8_t valid = 0;
	for (uint32_t i = 0; i < g_ota_ctx.hdr_mgr.FileHdr.HdrNum; i++) {
		const uint8_t *p = buf + OTA_HEADER_LEN + OTA_SUB_HEADER_LEN * i;
		const ota_sub_hdr_t *img_hdr = (const ota_sub_hdr_t *)p;

		if (strncmp("OTA", (const char *)img_hdr->Signature, 3) != 0) {
			aiio_log_e("invalid img signature at %lu\r\n", (unsigned long)i);
			return AIIO_ERROR;
		}

		memcpy(&g_ota_ctx.hdr_mgr.FileImgHdr[valid], img_hdr, sizeof(ota_sub_hdr_t));
		valid++;
	}

	g_ota_ctx.hdr_mgr.ValidImgCnt = valid;
	if (g_ota_ctx.hdr_mgr.ValidImgCnt == 0) {
		return AIIO_ERROR;
	}

	// Decide target slot & base address for each header index
	for (uint8_t idx = 0; idx < g_ota_ctx.hdr_mgr.ValidImgCnt; idx++) {
		const uint32_t img_id = g_ota_ctx.hdr_mgr.FileImgHdr[idx].ImgID;
		if (img_id >= OTA_IMGID_MAX) {
			aiio_log_e("invalid ImgID: %lu\r\n", (unsigned long)img_id);
			return AIIO_ERROR;
		}

		const uint8_t cur = ota_get_cur_index((uint8_t)img_id);
		const uint8_t slot = (cur == OTA_INDEX_1) ? OTA_INDEX_2 : OTA_INDEX_1;

		g_ota_ctx.slot_idx[idx] = slot;
		g_ota_ctx.image_base_phy[idx] = ota_storage_get_image_addr((uint8_t)img_id, slot);
		if (g_ota_ctx.image_base_phy[idx] == 0) {
			aiio_log_e("get image addr failed img=%lu slot=%lu\r\n", (unsigned long)img_id, (unsigned long)slot);
			return AIIO_ERROR;
		}

		g_ota_ctx.recv_in_image[idx] = 0;
		g_ota_ctx.next_erase_addr[idx] = g_ota_ctx.image_base_phy[idx];
	}

	g_ota_ctx.hdr_parsed = 1;
	return AIIO_OK;
}

static void ota_erase_to_cover(uint8_t hdr_idx, uint32_t write_phy_addr, uint32_t write_len)
{
	// Erase sectors from next_erase_addr up to end of this write range (inclusive),
	// keep it best-effort; underlying flash APIs typically tolerate re-erase.
	const uint32_t end_phy = write_phy_addr + write_len - 1;
	uint32_t erase_addr = g_ota_ctx.next_erase_addr[hdr_idx];

	// Align down to sector
	erase_addr = (erase_addr / 4096) * 4096;
	uint32_t end_sector = (end_phy / 4096) * 4096;

	while (erase_addr <= end_sector) {
		ota_storage_erase_sector(erase_addr);
		erase_addr += 4096;
	}

	g_ota_ctx.next_erase_addr[hdr_idx] = erase_addr;
}

static int ota_process_payload(uint32_t file_offset, const uint8_t *data, uint32_t len)
{
	// Stream through all image regions that overlap [file_offset, file_offset+len)
	// and perform:
	// - cache manifest bytes into hdr_mgr.Manifest[]
	// - write non-manifest bytes into flash at target slot
	for (uint8_t idx = 0; idx < g_ota_ctx.hdr_mgr.ValidImgCnt; idx++) {
		const ota_sub_hdr_t *img = &g_ota_ctx.hdr_mgr.FileImgHdr[idx];
		const uint32_t img_start = img->Offset;
		const uint32_t img_end = img->Offset + img->ImgLen; // exclusive

		const uint32_t seg_start = file_offset;
		const uint32_t seg_end = file_offset + len; // exclusive

		if (seg_end <= img_start || seg_start >= img_end) {
			continue;
		}

		// overlap
		const uint32_t ov_start = (seg_start > img_start) ? seg_start : img_start;
		const uint32_t ov_end = (seg_end < img_end) ? seg_end : img_end;
		const uint32_t ov_len = ov_end - ov_start;

		const uint32_t data_off = ov_start - file_offset;
		const uint32_t in_img_off = ov_start - img_start; // offset into this image payload (manifest included)

		const uint8_t *p = data + data_off;

		// 1) Cache manifest portion
		if (in_img_off < sizeof(ota_manifest_t)) {
			uint32_t m_off = in_img_off;
			uint32_t m_len = sizeof(ota_manifest_t) - m_off;
			if (m_len > ov_len) {
				m_len = ov_len;
			}
			memcpy(((uint8_t *)&g_ota_ctx.hdr_mgr.Manifest[idx]) + m_off, p, m_len);

			// if overlap is only manifest bytes, continue next image
			if (m_len == ov_len) {
				continue;
			}

			// advance past manifest for flash write
			p += m_len;
			// in_img_off += m_len;
			// ov_len -= m_len;
			const uint32_t write_in_img_off = in_img_off + m_len;
			const uint32_t write_len = ov_len - m_len;
			const uint32_t write_phy = g_ota_ctx.image_base_phy[idx] + write_in_img_off;

			ota_erase_to_cover(idx, write_phy, write_len);
			if (ota_storage_write(write_phy - SPI_FLASH_BASE, (uint8_t *)p, write_len) != OTA_OK) {
				aiio_log_e("ota_storage_write failed\r\n");
				return AIIO_ERROR;
			}
			continue;
		}

		// 2) Normal flash write (non-manifest)
		const uint32_t write_phy = g_ota_ctx.image_base_phy[idx] + in_img_off;
		ota_erase_to_cover(idx, write_phy, ov_len);
		if (ota_storage_write(write_phy - SPI_FLASH_BASE, (uint8_t *)p, ov_len) != OTA_OK) {
			aiio_log_e("ota_storage_write failed\r\n");
			return AIIO_ERROR;
		}
	}

	return AIIO_OK;
}

int32_t aiio_init_ota_partition(void)
{
	return ensure_ota_inited();
}

int32_t aiio_partition_write_ota_farmware(int32_t dst_offset, const void *src, int32_t size)
{
	if (src == NULL || size <= 0 || dst_offset < 0) {
		return AIIO_ERROR;
	}
	if (ensure_ota_inited() != AIIO_OK) {
		return AIIO_ERROR;
	}

	const uint8_t *buf = (const uint8_t *)src;
	uint32_t u_dst = (uint32_t)dst_offset;
	uint32_t u_size = (uint32_t)size;

	// Parse file header/sub-headers once, buffering if needed.
	if (!g_ota_ctx.hdr_parsed) {
		if (u_dst != 0) {
			aiio_log_e("ota header not parsed but dst_offset=%lu\r\n", (unsigned long)u_dst);
			return AIIO_ERROR;
		}

		// append to header buffer
		uint8_t *new_buf = (uint8_t *)realloc(g_ota_ctx.hdr_buf, g_ota_ctx.hdr_buf_len + u_size);
		if (new_buf == NULL) {
			aiio_log_e("hdr realloc failed\r\n");
			free(g_ota_ctx.hdr_buf);
			g_ota_ctx.hdr_buf = NULL;
			g_ota_ctx.hdr_buf_len = 0;
			return AIIO_ERROR;
		}
		g_ota_ctx.hdr_buf = new_buf;
		memcpy(g_ota_ctx.hdr_buf + g_ota_ctx.hdr_buf_len, buf, u_size);
		g_ota_ctx.hdr_buf_len += u_size;

		// Decide how many bytes we need for headers
		if (g_ota_ctx.hdr_buf_len >= OTA_HEADER_LEN && g_ota_ctx.hdr_need_len == 0) {
			const ota_hdr_t *fh = (const ota_hdr_t *)g_ota_ctx.hdr_buf;
			uint32_t hdr_num = fh->HdrNum;
			if (hdr_num == 0 || hdr_num > OTA_IMGID_MAX) {
				aiio_log_e("invalid ota hdr num: %lu\r\n", (unsigned long)hdr_num);
				return AIIO_ERROR;
			}
			g_ota_ctx.hdr_need_len = OTA_HEADER_LEN + hdr_num * OTA_SUB_HEADER_LEN;
		}

		if (g_ota_ctx.hdr_need_len == 0 || g_ota_ctx.hdr_buf_len < g_ota_ctx.hdr_need_len) {
			// still waiting more header bytes
			return AIIO_OK;
		}

		if (ota_parse_file_headers(g_ota_ctx.hdr_buf, g_ota_ctx.hdr_buf_len) != AIIO_OK) {
			aiio_log_e("ota_parse_file_headers failed\r\n");
			return AIIO_ERROR;
		}

		// Now process any remaining bytes after header region
		const uint32_t hdr_len = g_ota_ctx.hdr_need_len;
		if (g_ota_ctx.hdr_buf_len > hdr_len) {
			const uint32_t remain = g_ota_ctx.hdr_buf_len - hdr_len;
			const uint8_t *remain_p = g_ota_ctx.hdr_buf + hdr_len;
			if (ota_process_payload(hdr_len, remain_p, remain) != AIIO_OK) {
				return AIIO_ERROR;
			}
		}

		free(g_ota_ctx.hdr_buf);
		g_ota_ctx.hdr_buf = NULL;
		g_ota_ctx.hdr_buf_len = 0;
		return AIIO_OK;
	}

	// Normal streaming payload processing (file_offset == dst_offset)
	return ota_process_payload(u_dst, buf, u_size);
}

int32_t aiio_partition_erase(int32_t start_addr, int32_t size)
{
	// Erase is done lazily per sector when writing on this platform.
	(void)start_addr;
	(void)size;
	return AIIO_OK;
}

void aiio_set_boot_partition(void)
{
	if (!g_ota_ctx.hdr_parsed) {
		aiio_log_e("ota not ready: headers not parsed\r\n");
		return;
	}

	for (uint8_t idx = 0; idx < g_ota_ctx.hdr_mgr.ValidImgCnt; idx++) {
		const uint8_t slot = (uint8_t)g_ota_ctx.slot_idx[idx];
		const uint8_t img_id = (uint8_t)g_ota_ctx.hdr_mgr.FileImgHdr[idx].ImgID;

		if (img_id == OTA_IMGID_BOOT && slot == OTA_INDEX_2) {
			if (ota_check_bootloader_ota2()) {
				aiio_log_w("Bootloader OTA2 invalid, skip\r\n");
				continue;
			}
		}

		if (ota_storage_verify_checksum(&g_ota_ctx.hdr_mgr, slot, idx) != OTA_OK) {
			aiio_log_e("verify checksum failed idx=%lu\r\n", (unsigned long)idx);
			return;
		}

		if (ota_storage_update_manifest(&g_ota_ctx.hdr_mgr, slot, idx) != OTA_OK) {
			aiio_log_e("update manifest failed idx=%lu\r\n", (unsigned long)idx);
			return;
		}
	}

	// reset state after switching
	ota_state_reset();
}

