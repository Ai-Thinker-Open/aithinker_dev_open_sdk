[TOC]
# 引言
ble主设备扫描接口，提供启动/暂停扫描，扫描数据处理等接口



# 宏定义



# 枚举
## le_scan_state_t
|值|标记|描述|
|:-|:-|:-|
|0x00|LE_SCAN_STATE_UNINITIALIZED|蓝牙扫描模块未初始化|
|0x01|LE_SCAN_STATE_INITIALIZED|蓝牙扫描模块初始化完成|
|0x02|LE_SCAN_STATE_STARTING|正在启动蓝牙扫描|
|0x03|LE_SCAN_STATE_STARTED|蓝牙扫描已启动|
|0x04|LE_SCAN_STATE_STOPING|正在关闭蓝牙扫描|
|0x05|LE_SCAN_STATE_STOPED|蓝牙扫描已关闭|



## le_scan_filter_type_t
|值|标记|描述|
|:-|:-|:-|
|0x01|SCAN_FILTER_TYPE_RSSI|扫描报告使能RSSI过滤|
|0x02|SCAN_FILTER_TYPE_TX_PWR|扫描报告使能发射功率过滤|
|0x04|SCAN_FILTER_TYPE_PHY|扫描报告使能调制类型过滤|
|0x08|SCAN_FILTER_TYPE_ADDR_TYPE|扫描报告使能地址类型过滤|
|0x10|SCAN_FILTER_TYPE_ADDR|扫描报告使能地址过滤|
|0x20|SCAN_SFILTER_TYPE_ADV_DATA|扫描报告使能用户广播数据过滤|



# 结构体
## adv_data_item_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|len|蓝牙广播包中，一个广播项目的长度|
|uint8_t|type|该广播项目的类型|
|uint8_t[1]|data|该广播项目的数据内容|



## le_advertising_report_filter_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|filter_type|过滤类型掩码，详见`le_scan_filter_type_t`|
|int8_t|rssi|信号强度过滤，当掩码使能rssi过滤时有效|
|int8_t|tx_pwr|发射功率过滤，当掩码使能发射功率过滤时有效|
|uint8_t|phy|调制方式过滤，当掩码使能phy时有效|
|uint8_t[8]]|find_addr|地址过滤，当掩码使能地址过滤时有效|
|uint8_t|adv_data_len|过滤的广播数据的长度，当掩码使能内容过滤时有效|
|uint8_t[32]|adv_data_pattern|过滤的广播数据的内容，当掩码使能内容过滤时有效|



## le_scan_parameters_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|type|扫描类型，值域详见`gapm_scan_type`|
|uint8_t|prop|扫描属性，值域详见`gapm_scan_prop`|
|uint8_t|dup_filt_pol|重复包过滤策略，值域详见`gapm_dup_filter_pol`|



# 函数
## ln_ble_scan_actv_creat
设置ping码输入的超时时间
* **定义**  
**`void ln_ble_scan_actv_creat(void);`**
* **参数**
* **返回值**



## ln_ble_scan_set_rpt_filter
设置扫描报告的数据过滤
* **定义**  
**`void ln_ble_scan_set_rpt_filter(le_scan_report_filter_t *filter)`**
* **参数**
|类型|参数|说明|
|:-|:-|:-|
|le_scan_report_filter_t|filter|扫描报告的过滤设置`le_scan_report_filter_t`|
* **返回值**



## ln_ble_scan_clear_rpt_filter
清楚扫描报告的数据过滤设置
* **定义**  
**`void ln_ble_scan_clear_rpt_filter(void)`**
* **参数**
* **返回值**



## ln_ble_scan_start
设置ping码输入的超时时间
* **定义**  
**`void ln_ble_scan_start(void);`**
* **参数**
* **返回值**



## ln_ble_scan_stop
设置ping码输入的超时时间
* **定义**  
**`void ln_ble_scan_stop(void);`**
* **参数**
* **返回值**



## ln_ble_scan_mgr_init
安全管理模块初始化
* **定义**  
**`int ln_ble_scan_mgr_init(void);`**
* **参数**
* **返回值**
|类型|说明|
|:-|:-|
|int|详见错误码`BLE_ERR_CODE_T`|