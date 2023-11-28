/**
 ****************************************************************************************
 *
 * @file ln_ble_app_kv.c
 *
 * @brief kv APIs source code
 *
 * Copyright (C) 2021. Shanghai Lightning Semiconductor Ltd
 *
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <string.h>

#include "ln_kv_key_def.h"
#include "ln_kv_api.h"

#include "utils/debug/log.h"

#include "ln_ble_app_kv.h"

#define KV_SYSPARAM_BLE_NAME                ((const char *)"1_ble_name")
#define KV_SYSPARAM_BLE_ADDR                ((const char *)"2_ble_addr")
#define KV_SYSPARAM_BLE_IRK                 ((const char *)"3_ble_irk")

#define KV_SYSPARAM_BLE_USR_DATA            ((const char *)"4_ble_usr_data")

static kv_name_info_t g_bt_name;
static ln_bd_addr_t   g_bt_addr;
static kv_irk_info_t  g_loc_irk;
static kv_usr_data_t  g_bt_usr_data;


static inline int btparam_delete(const char *key)
{
    int ret = ln_kv_del(key);
    if (ret != KV_ERR_NONE) {
        return BT_KV_ERR_STORE;
    }
    return BT_KV_ERR_NONE;
}

static inline int btparam_store(const char *key, const void *value, size_t value_len)
{
    int ret = ln_kv_set(key, value, value_len);
    if (ret != KV_ERR_NONE) {
        return BT_KV_ERR_STORE;
    }
    return BT_KV_ERR_NONE;
}

static inline int btparam_load(const char *key, void *value_buf, size_t value_buf_size)
{
    size_t v_len = 0;
    int ret = ln_kv_get(key, value_buf, value_buf_size, &v_len);
    if (ret != KV_ERR_NONE) {
        return BT_KV_ERR_LOAD;
    }
    return BT_KV_ERR_NONE;
}

static inline int btparam_has_stored(const char *key)
{
    return ln_kv_has_key(key);
}

int ln_kv_ble_name_store(const uint8_t *name, uint16_t name_len)
{
    memset(&g_bt_name, 0, sizeof(kv_name_info_t));
    if(name_len > BLE_DEV_NAME_MAX_LEN)
        name_len = BLE_DEV_NAME_MAX_LEN;
    memcpy(g_bt_name.name, name, name_len);
    g_bt_name.name_len = name_len;
    return btparam_store(KV_SYSPARAM_BLE_NAME, (void *)&g_bt_name, sizeof(kv_name_info_t));
}

static int ln_kv_ble_name_load(void)
{
    memset(&g_bt_name, 0, sizeof(kv_name_info_t));
    if (btparam_has_stored(KV_SYSPARAM_BLE_NAME))
    {
        btparam_load(KV_SYSPARAM_BLE_NAME, (void *)&g_bt_name, sizeof(kv_name_info_t));
    }
    if(!g_bt_name.name_len)
    {
        memcpy(g_bt_name.name, BLE_DEFAULT_DEVICE_NAME, BLE_DEVICE_NAME_LEN);
        g_bt_name.name_len = BLE_DEVICE_NAME_LEN;
    }

    return BT_KV_ERR_NONE;
}

uint8_t *ln_kv_ble_dev_name_get(uint16_t *name_len)
{
    *name_len = g_bt_name.name_len;

    return g_bt_name.name;
}

int ln_kv_ble_addr_store(ln_bd_addr_t addr)
{
    g_bt_addr = addr;
    return btparam_store(KV_SYSPARAM_BLE_ADDR, (void *)&g_bt_addr, sizeof(ln_bd_addr_t));
}

static int ln_kv_ble_addr_load(void)
{
    ln_bd_addr_t addr = {BLE_DEFAULT_PUBLIC_ADDR};
    ln_bd_addr_t addr_null = {0x00,0x00,0x00,0x00,0x00,0x00};

    memset(&g_bt_addr, 0, sizeof(ln_bd_addr_t));
    if (btparam_has_stored(KV_SYSPARAM_BLE_ADDR))
    {
        btparam_load(KV_SYSPARAM_BLE_ADDR, (void *)&g_bt_addr, sizeof(ln_bd_addr_t));
    }
    if(!memcmp(&g_bt_addr, &addr_null, LN_BD_ADDR_LEN))
    {
        memcpy(&g_bt_addr, &addr, LN_BD_ADDR_LEN);
    }

    return BT_KV_ERR_NONE;
}

ln_bd_addr_t *ln_kv_ble_pub_addr_get(void)
{
    return &g_bt_addr;
}

int ln_kv_ble_irk_store(uint8_t irk[16])
{
    g_loc_irk.vailed = 1;
    memcpy(g_loc_irk.irk, irk, 16);
    return btparam_store(KV_SYSPARAM_BLE_IRK, (void *)g_loc_irk.irk, 16);
}

static int ln_kv_ble_irk_load(void)
{
    memset(&g_loc_irk, 0, sizeof(kv_irk_info_t));
    if (btparam_has_stored(KV_SYSPARAM_BLE_IRK))
    {
        btparam_load(KV_SYSPARAM_BLE_IRK, (void *)&g_loc_irk.irk, 16);
        g_loc_irk.vailed = 1;
    }

    return BT_KV_ERR_NONE;
}

uint8_t * ln_kv_ble_irk_get(void)
{
    if(!g_loc_irk.vailed)
        return NULL;
    return g_loc_irk.irk;
}

static int ln_kv_ble_user_data_load(void)
{
    int ret = BT_KV_ERR_NONE;

    memset(&g_bt_usr_data, 0, sizeof(g_bt_usr_data));
    if (btparam_has_stored(KV_SYSPARAM_BLE_USR_DATA))
    {
        ret = btparam_load(KV_SYSPARAM_BLE_USR_DATA, (void *)&g_bt_usr_data, sizeof(g_bt_usr_data));
    }

    if(BLE_APP_NVDS_MAGIC_NUM != g_bt_usr_data.magic)
    {
        g_bt_usr_data.ble_role = BLE_DEFAULT_ROLE;
    }

    return ret;;
}

int ln_kv_ble_usr_data_store(void)
{
    int ret = BT_KV_ERR_NONE;
    g_bt_usr_data.magic = BLE_APP_NVDS_MAGIC_NUM;
    ret = btparam_store(KV_SYSPARAM_BLE_USR_DATA, (void *)&g_bt_usr_data, sizeof(g_bt_usr_data));

    return ret;
}

kv_usr_data_t *ln_kv_ble_usr_data_get(void)
{
    return &g_bt_usr_data;
}

int ln_kv_ble_factory_reset(void)
{
    ln_bd_addr_t addr = {BLE_DEFAULT_PUBLIC_ADDR};
    memcpy(&g_bt_addr, &addr, LN_BD_ADDR_LEN);

    memset(&g_bt_name, 0, sizeof(kv_name_info_t));
    memcpy(g_bt_name.name, BLE_DEFAULT_DEVICE_NAME, BLE_DEVICE_NAME_LEN);
    g_bt_name.name_len = BLE_DEVICE_NAME_LEN;

    memset(&g_loc_irk, 0, sizeof(kv_irk_info_t));

    memset(&g_bt_usr_data, 0, sizeof(g_bt_usr_data));
    g_bt_usr_data.ble_role = BLE_DEFAULT_ROLE;

    btparam_delete(KV_SYSPARAM_BLE_NAME);
    btparam_delete(KV_SYSPARAM_BLE_ADDR);
    btparam_delete(KV_SYSPARAM_BLE_IRK);
    btparam_delete(KV_SYSPARAM_BLE_USR_DATA);

    return 0;
}


int ln_kv_ble_app_init(void)
{
    LOG(LOG_LVL_TRACE,"ln_kv_ble_app_init...\r\n");

    ln_kv_ble_name_load();
    ln_kv_ble_addr_load();
    ln_kv_ble_irk_load();
    ln_kv_ble_user_data_load();

    return 0;
}

