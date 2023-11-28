/**
 ****************************************************************************************
 *
 * @file ble_lib_version.h
 *
 * @brief  ble full stack library version serial number management .
 *
 *Copyright (C) 2021.Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

#ifndef __BLE_LIB_VERSION_H__
#define __BLE_LIB_VERSION_H__

const char *ble_lib_version_string_get(void);
uint32_t    ble_lib_version_number_get(void);
const char *ble_lib_build_time_get(void);


#endif // __BLE_LIB_VERSION_H__

