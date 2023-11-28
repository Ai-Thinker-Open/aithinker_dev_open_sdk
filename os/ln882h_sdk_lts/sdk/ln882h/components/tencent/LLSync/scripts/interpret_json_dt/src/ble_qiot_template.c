/*
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT
 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifdef __cplusplus
extern "C" {
#endif
#include "ble_qiot_template.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "ble_qiot_export.h"
#include "ble_qiot_common.h"
#include "ble_qiot_param_check.h"
static int ble_property_power_switch_set(const char *data, uint16_t len)
{
	uint8_t tmp_bool = 0;
	tmp_bool = data[0];
	ble_qiot_log_d("set id power_switch bool value %02x", data[0]);
	return 0;
}

static int ble_property_power_switch_get( char *data, uint16_t len)
{
	uint8_t tmp_bool = 1;
	data[0] = tmp_bool;
	ble_qiot_log_d("get id power_switch bool value %02x", data[0]);
	return sizeof(uint8_t);
}

static int ble_property_color_set(const char *data, uint16_t len)
{
	uint16_t tmp_enum = 0;
	memcpy(&tmp_enum, data, sizeof(uint16_t));
	tmp_enum = NTOHS(tmp_enum);
	ble_qiot_log_d("set id color int value %d", tmp_enum);
	return 0;
}

static int ble_property_color_get( char *data, uint16_t len)
{
	uint16_t tmp_enum = 0;
	tmp_enum = HTONS(tmp_enum);
	memcpy(data, &tmp_enum, sizeof(uint16_t));
	ble_qiot_log_d("get id color int value %d", 1234);
	return sizeof(uint16_t);
}

static int ble_property_brightness_set(const char *data, uint16_t len)
{
	int tmp_int = 0;
	memcpy(&tmp_int, data, sizeof(int));
	tmp_int = NTOHL(tmp_int);
	ble_qiot_log_d("set id brightness int value %d", tmp_int);
	return 0;
}

static int ble_property_brightness_get( char *data, uint16_t len)
{
	int tmp_int = 1;
	tmp_int = HTONL(tmp_int);
	memcpy(data, &tmp_int, sizeof(int));
	ble_qiot_log_d("get id brightness int value %d", 12345678);
	return sizeof(int);
}

ble_property_t sg_ble_property_array[3] = {
	{ble_property_power_switch_set, ble_property_power_switch_get, 0, BLE_QIOT_DATA_TYPE_BOOL, 1},
	{ble_property_color_set, ble_property_color_get, 0, BLE_QIOT_DATA_TYPE_ENUM, 1},
	{ble_property_brightness_set, ble_property_brightness_get, 0, BLE_QIOT_DATA_TYPE_INT, 1},
};


#ifdef __cplusplus
}
#endif
