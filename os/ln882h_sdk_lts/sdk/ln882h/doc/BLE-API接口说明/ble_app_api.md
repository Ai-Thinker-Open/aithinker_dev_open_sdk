## 结构
ble 应用层的代码结构如下所示：



`|-- ble_common  
|   |-- [ln_ble_app_defaut_cfg.h](./ble_api_document/ln_ble_app_defaut_cfg-CH.md)   
|   |-- [ln_ble_app_defines.h](./ble_api_document/ln_ble_app_defines-CH.md)  
|   |-- ln_ble_app_err_code.h  
|-- ble_connection_manager  
|   |-- ln_ble_connection_manager.c  
|   |-- [ln_ble_connection_manager.h](./ble_api_document/ln_ble_connection_manager-CH.md)  
|-- ble_device_manager  
|   |-- ln_ble_device_manager.c  
|   |-- ln_ble_device_manager.h  
|-- ble_event  
|   |-- ln_ble_event_manager.c  
|   |-- ln_ble_event_manager.h  
|-- ble_gap  
|   |-- gap_advertising  
|   |   |-- ln_ble_advertising.c  
|   |   |-- [ln_ble_advertising.h](./ble_api_document/ln_ble_advertising-CH.md)  
|   |-- gap_misc  
|   |   |-- ln_ble_gap.c  
|   |   |-- [ln_ble_gap.h](./ble_api_document/ln_ble_gap-CH.md)  
|   |   |-- ln_ble_gap_ind_handler.c  
|   |   |-- ln_ble_gap_ind_handler.h  
|   |-- gap_scan  
|       |-- ln_ble_scan.c  
|       |--[ ln_ble_scan.h](./ble_api_document/ln_ble_connection_manager-CH.md)  
|-- ble_gatt  
|   |-- gatt_client  
|   |   |-- ln_ble_battery_client.c  
|   |   |-- ln_ble_battery_client.h  
|   |   |-- ln_ble_hid_client.c  
|   |   |-- ln_ble_hid_client.h  
|   |   |-- ln_ble_trans_client.c  
|   |   |-- ln_ble_trans_client.h  
|   |-- gatt_common  
|   |   |-- ln_ble_gatt.c  
|   |   |--[ ln_ble_gatt.h](./ble_api_document/ln_ble_gatt-CH.md)  
|   |   |-- ln_ble_gatt_ind_handler.c  
|   |   |-- ln_ble_gatt_ind_handler.h  
|   |-- gatt_server  
|       |-- ln_ble_battery_server.c  
|       |-- ln_ble_battery_server.h  
|       |-- ln_ble_hid_server.c  
|       |-- ln_ble_hid_server.h  
|       |-- ln_ble_trans_server.c  
|       |-- ln_ble_trans_server.h  
|-- ble_import  
|   |-- ln_ble_rw_app_task.c  
|   |-- ln_ble_rw_app_task.h  
|-- ble_smp  
|   |-- ln_ble_smp.c  
|   |-- [ln_ble_smp.h](./ble_api_document/ln_ble_smp-CH.md)   
|-- ble_store  
|   |-- ln_ble_app_kv.c  
|   |-- ln_ble_app_kv.h  
|   |-- ln_ble_app_nvds.c  
|   |-- ln_ble_app_nvds.h  
|-- ble_test  
|    |-- ln_ble_test.c  
|    |-- ln_ble_test.h  `