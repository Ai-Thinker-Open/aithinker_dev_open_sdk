[TOC]
# 引言
设备广播管理模块



# 宏定义
	#define BLE_USE_LEGACY_ADV      	 (1)  
设置为1时，使用经典广播包，为0时使用扩展广播包

	#define APP_ADV_CHMAP                (0x07) 
ble广播信道，默认3个信道都启用  

	#define APP_ADV_INT_MIN              (160)
最小广播间隔，默认为160\*0.625=100ms  

	#define APP_ADV_INT_MAX              (160)
最大广播间隔，默认为160\*0.625=100ms  

	#define ADV_DATA_LEGACY_MAX          (31)
经典广播包数据最大长度  

	#define ADV_DATA_EXTEND_MAX          (250)
扩展广播包数据最大长度  

	#define ADV_TYPE_FLAG_LEN            (3)
广播类型字段的数据长度，非beacon广播包，底层自动添加该字段  



# 枚举
## ble_adv_state_t
|值|标记|描述|
|:-|:-|:-|
|0x00|LE_ADV_STATE_UNINITIALIZED|广播管理模块未初始化|
|0x01|LE_ADV_STATE_INITIALIZED|广播管理模块初始化完成|
|0x02|LE_ADV_STATE_STARTING|正在启动蓝牙广播|
|0x03|LE_ADV_STATE_STARTED|蓝牙广播已启动|
|0x04|LE_ADV_STATE_STOPING|正在关闭蓝牙广播|
|0x05|LE_ADV_STATE_STOPED|蓝牙广播已关闭|



# 结构体
## ln_adv_data_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|length|自定义广播包数据长度|
|uint8_t \*|data|自定义广播包数据内容|



## adv_param_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|own_addr_type|地址类型，值域参考`gapm_own_addr`|
|uint8_t|adv_type|广播类型，值域参考`gapm_adv_type`|
|uint8_t|disc_mode|可发现模式配置，值域参考`gapm_adv_disc_mode`|
|uint8_t|adv_prop|广播特性，值域参考`gapm_leg_adv_prop`和`gapm_ext_adv_prop`|
|uint32_t|adv_intv_min|最小广播间隔（实际时间=数值\*625微秒），必须大于20毫秒|
|uint32_t|adv_intv_max|最小广播间隔（实际时间=数值\*625微秒），必须大于20毫秒|



# 函数
## ln_ble_adv_scan_rsp_data_set
设置从设备的回复广播数据包数据。该数据包用于主设备扫描时，从设备回复主设备
* **定义**  
**`void ln_ble_adv_scan_rsp_data_set(ln_adv_data_t *scan_rsp_data);`**
* **参数**  
|参数|说明|
|:-|:-|
|scan_rsp_data|回复扫描的广播数据包，详见结构体`ln_adv_data_t`|
| | |
* **返回值**



## ln_ble_adv_data_set
设置广播数据包参数
* **定义**  
**`void ln_ble_adv_data_set(ln_adv_data_t *adv_data);`**
* **参数**  
|参数|说明|
|:-|:-|
|adv_data|广播数据包，详见结构体`ln_adv_data_t`|
| | |
* **返回值**



## ln_ble_adv_actv_creat
创建一个用于广播的活动（活动包括广播，扫描和连接三类），设备如果需要广播的功能，必需先创建该活动，活动可创建的最大数量由**BLE_ACTIVITY_MAX**指定
* **定义**  
**`void ln_ble_adv_actv_creat(adv_param_t *param);`**
|参数|说明|
|:-|:-|
|param|广播参数，详见结构体`adv_param_t`|
| | |
* **参数**  
* **返回值**



## ln_ble_adv_start
设备启动广播
* **定义**  
**`void ln_ble_adv_start(void);`**
* **参数**  
* **返回值**



## ln_ble_adv_stop
设备停止广播
* **定义**  
**`void ln_ble_adv_stop(void);`**
* **参数**  
* **返回值**



## ln_ble_adv_mgr_init
广播管理模块初始化
* **定义**  
int **`ln_ble_adv_mgr_init(void);`**  
* **参数**  
* **返回值**
|类型|说明|
|:-|:-|
|int|详见错误码`BLE_ERR_CODE_T`|
| | |