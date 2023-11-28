/**
 ****************************************************************************************
 *
 * @file ln_ble_device_manager.c
 *
 * @brief device manager APIs source code
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/debug/log.h"

#include "ln_ble_device_manager.h"
#include "ln_ble_app_kv.h"
#include "ln_ble_app_default_cfg.h"
#include "ln_ble_app_err_code.h"
#include "ln_ble_app_defines.h"


bool ln_ble_dev_is_exist(bt_device_info_t *dev)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();

    for(int i=0; i<MAX_DEV_BOND_MUN; i++)
    {
        if(!memcmp(&nvds->pair_list_dev[i].addr, &dev->addr, sizeof(ln_bd_addr_t))
                && nvds->pair_list_dev[i].mru)
        {
            return true;
        }
    }

    return false;
}

uint8_t ln_ble_get_index_by_addr(ln_bd_addr_t *addr)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();

    for(int i=0; i<MAX_DEV_BOND_MUN; i++)
    {
        if(!memcmp(&(nvds->pair_list_dev[i].addr), addr, sizeof(ln_bd_addr_t)))
        {
            return i;
        }
    }

    return DEV_LIST_INVAILD_INDEX;
}

bt_device_info_t *ln_ble_get_dev_by_addr(ln_bd_addr_t *addr)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();

    for(int i=0; i<MAX_DEV_BOND_MUN; i++)
    {
        if(!memcmp(&(nvds->pair_list_dev[i].addr), addr, sizeof(ln_bd_addr_t)))
        {
            return &nvds->pair_list_dev[i];
        }
    }

    return NULL;
}

static void ln_ble_dev_upadte_mru(uint8_t index, uint8_t opr)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();

    for(int i=0; i<MAX_DEV_BOND_MUN; i++)
    {
        uint8_t *mru = &(nvds->pair_list_dev[i].mru);

        if(index == i || !(*mru))
            continue;

        switch(opr)
        {
            case BLE_DEVICE_ADD:
                (*mru)++;
                if(*mru > MAX_DEV_BOND_MUN)
                    *mru = 0;
                break;
            case BLE_DEVICE_DELETE:
                if(*mru > nvds->pair_list_dev[index].mru)
                    (*mru)--;
                break;
            case BLE_DEVICE_UPDATE:
                if(*mru < nvds->pair_list_dev[index].mru)
                    (*mru)++;
                break;

            default:
                break;
        }
        LOG(LOG_LVL_TRACE,"ln_ble_dev_upadte_mru index=%d,mru==%d\r\n",i, *mru);
    }

    if(BLE_DEVICE_DELETE == opr)
        nvds->pair_list_dev[index].mru = 0;
    else
        nvds->pair_list_dev[index].mru = 1;
}

int ln_ble_dev_insert(bt_device_info_t *new_dev)
{
    kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
    bool need_store = true;

    if(ln_ble_dev_is_exist(new_dev))
    {
        uint8_t index = ln_ble_get_index_by_addr(&new_dev->addr);
        //device is exist,but is not lastest device, update mru
        if(nvds->pair_list_dev[index].mru > 1)
        {
            ln_ble_dev_upadte_mru(index, BLE_DEVICE_UPDATE);
        }
        else
        {
            need_store = false;
        }
    }
    else
    {
        uint8_t dev_index = ln_ble_get_index_by_addr((&new_dev->addr));
        if(DEV_LIST_INVAILD_INDEX == dev_index)
        {
            for(int i=0; i<MAX_DEV_BOND_MUN; i++)
            {
                uint8_t mru = nvds->pair_list_dev[i].mru;
                //if device null can be insert, or device is last dev can be cover
                if(!mru || MAX_DEV_BOND_MUN == mru)
                {
                    dev_index = i;
                    break;
                }
            }
        }
        memcpy(&nvds->pair_list_dev[dev_index], new_dev, sizeof(bt_device_info_t));
        ln_ble_dev_upadte_mru(dev_index, BLE_DEVICE_ADD);
    }

    if(need_store) {
        ln_kv_ble_usr_data_store();
    }

    return BLE_ERR_NONE;
}

int ln_ble_dev_delete(bt_device_info_t *dev)
{
    if(ln_ble_dev_is_exist(dev))
    {
        uint8_t index = ln_ble_get_index_by_addr(&dev->addr);
        kv_usr_data_t *nvds = ln_kv_ble_usr_data_get();
        ln_ble_dev_upadte_mru(index, BLE_DEVICE_DELETE);
        memset(&nvds->pair_list_dev[index], 0, sizeof(bt_device_info_t));
        ln_kv_ble_usr_data_store();

        return BLE_ERR_NONE;
    }

    return BLE_ERR_INVALID_PARAM;
}

