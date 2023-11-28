[TOC]
# 引言
蓝牙应用层默认配置，当`ble_app_user_cfg.h`中定义了同名宏定义配置时，会被覆盖



# 宏定义
	#define BLE_DEFAULT_ROLE                BLE_ROLE_ALL  
ble默认角色，可以初始化为外围设备或者中心设备，也可以两种角色共存  

	#define BLE_DEFAULT_DEVICE_NAME         ("ln882h")  
ble默认设备名称  

	##define BLE_DEV_NAME_MAX_LEN           (40)  
ble默认设备名称长度  

	##define BLE_DEV_NAME_MAX_LEN           (40)  
ble默认设备名称长度

	#define BLE_DEFAULT_PUBLIC_ADDR         ({0x56, 0x34, 0x12, 0x03, 0xFF, 0x00})
ble默认静态公有地址

	#define BLE_CONFIG_AUTO_ADV             (0)
当设备未到达连接上限时，自动开启广播

	#define BLE_CONFIG_AUTO_SCAN            (0)
当设备未到达连接上限时，自动开启扫描

	#define BLE_CONFIG_SECURITY             (0)
启动蓝牙配对

	#define BLE_CONFIG_MULTI_CONNECT        (0)
启用多连接，注意设备最大连接个数受**BLE_CONN_DEV_NUM_MAX**限制

	#define BLE_CONN_DEV_NUM_MAX        (CFG_CON)
设备可同时存在的链接数，当**BLE_CONFIG_MULTI_CONNECT**未使能时，最大只能为1

	#define BLE_DATA_TRANS_SERVER           (0)
启动自定义数据透传服务的服务器端

	#define BLE_DATA_TRANS_CLIENT           (0)
启动自定义数据透传服务的客户端



# 枚举
## ble_role_type_t
|值|标记|描述|
|:-|:-|:-|
|0x00|BLE_ROLE_UNKNOWN|未知设备角色|
|0x01|BLE_ROLE_PERIPHERAL|外设|
|0x02|BLE_ROLE_CENTRAL|中心设备|
|0x03|BLE_ROLE_ALL|同时支持外设和中心设备|
