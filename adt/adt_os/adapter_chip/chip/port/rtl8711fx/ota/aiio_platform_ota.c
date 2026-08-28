/**
 * @brief   rtl8711fx ota platform interface
 *
 * @file    aiio_platform_ota.c
 * @copyright Copyright (C) 2020-2026, Shenzhen Anxinke Technology Co., Ltd
 */

#include <stdint.h>

#include "aiio_error.h"
#include "aiio_log.h"
#include "aiio_platform_ota.h"

static uint8_t s_platform_ota_started = 0;
static uint8_t s_erase_once = 0;
static uint32_t s_fw_len = 0;

int aiio_platform_ota_start(void)
{
	if (s_platform_ota_started) {
		aiio_log_e("platform ota already start\r\n");
		return AIIO_ERROR;
	}

	s_erase_once = 0;
	s_fw_len = 0;
	s_platform_ota_started = 1;

	// Ensure storage/layout is ready
	if (aiio_init_ota_partition() != AIIO_OK) {
		s_platform_ota_started = 0;
		return AIIO_ERROR;
	}

	return AIIO_OK;
}

int aiio_platform_ota_stop(void)
{
	if (!s_platform_ota_started) {
		aiio_log_e("platform ota already stop\r\n");
		return AIIO_ERROR;
	}

	s_platform_ota_started = 0;
	return AIIO_OK;
}

int aiio_platform_ota_install(uint8_t *data, uint32_t data_len, uint32_t data_total_len)
{
	if (!s_platform_ota_started) {
		aiio_log_e("platform ota stop\r\n");
		return AIIO_ERROR;
	}

	// best-effort: some transports want a one-time "erase" hook.
	// For rtl8711fx, erase is done lazily per-sector during streaming write.
	if (!s_erase_once) {
		(void)aiio_partition_erase(0, (int32_t)((data_total_len / 4096 + 1) * 4096));
		s_erase_once = 1;
	}

	if (aiio_partition_write_ota_farmware((int32_t)s_fw_len, data, (int32_t)data_len) != AIIO_OK) {
		goto __install_err;
	}

	s_fw_len += data_len;
	if (s_fw_len == data_total_len) {
		// Switch slot by updating manifest(s)
		aiio_set_boot_partition();
		return AIIO_OTA_INSTALL_SUCCESS;
	}

	return AIIO_OK;

__install_err:
	aiio_platform_ota_stop();
	return AIIO_ERROR;
}

