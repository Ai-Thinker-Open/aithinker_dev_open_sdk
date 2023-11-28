[TOC]
# 引言
设备连接管理模块，包含连接状态机维护，连接事件回调处理，和保存连接信息等



# 宏定义



# 枚举
## ble_conn_state_t
|值|标记|描述|
|:-|:-|:-|
|0x00|LE_CONN_STATE_UNINITIALIZED|连接管理模块未初始化|
|0x01|LE_CONN_STATE_INITIALIZED|连接管理模块初始化完成|
|0x02|LE_CONN_STATE_DISCONNECTED|蓝牙已断开|
|0x03|LE_CONN_STATE_CONNECTING|蓝牙正在连接|
|0x04|LE_CONN_STATE_CONNECTED|蓝牙已连接|
|0x05|LE_CONN_STATE_DISCONNECTING|蓝牙正在断开|



## ln_ble_conn_mode_t
|值|标记|描述|
|:-|:-|:-|
|0x00|LN_BLE_CONN_MODE_SLOW|蓝牙连接低速模式|
|0x01|LN_BLE_CONN_MODE_NORMAL|蓝牙连接正常工作模式|
|0x02|LN_BLE_CONN_MODE_FAST|蓝牙连接高速率模式|
|0x03|LN_BLE_CONN_MODE_MAX|蓝牙连接模式数量|



# 结构体
## ln_ble_conn_param_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|intv_min|连接间隔最小值，值域范围:6~3200，（实际时间=数值\*1.25毫秒）|
|uint16_t|intv_max|连接间隔最大值，值域范围:6~3200，（实际时间=数值\*1.25毫秒）|
|uint16_t|latency|从机延时应答次数，值域范围:0~499|
|uint16_t|time_out|连接超时时间，值域范围:10~3200，（实际时间=数值\*10毫秒）|



# 函数
## ln_ble_conn_mode_set
设置从设备的回复广播数据包数据。该数据包用于主设备扫描时，从设备回复主设备
* **定义**  
**`void ln_ble_conn_mode_set(int conidx, ln_ble_conn_mode_t mode);`**
* **参数**
|参数|说明|
|:-|:-|
|mode|蓝牙连接工作模式，详见枚举`ln_ble_conn_mode_t`|
| | |
* **返回值**



## ln_ble_conn_param_update
设置蓝牙连接参数
* **定义**  
**`void ln_ble_conn_param_update(int conidx, ln_ble_conn_param_t *p_conn_param);`**
* **参数**
|参数|说明|
|:-|:-|
|p_conn_param|连接参数，包括连接间隔、延时等，详见结构体`ln_ble_conn_param_t`|
| | |
* **返回值**



## ln_ble_disc_req
根据连接id，断开蓝牙连接
* **定义**  
**`void ln_ble_disc_req(int conidx);`**
* **参数**
|参数|说明|
|:--|:--|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
| | |
* **返回值**



## ln_ble_conn_req
根据对端的蓝牙地址，发起连接请求
* **定义**  
**`void ln_ble_conn_req(ln_bd_addr_t *addr);`**
* **参数**
|参数|说明|
|:--|:--|
|addr|要连接的对端蓝牙设备的地址|
| | |
* **返回值**



## ln_ble_init_actv_creat
创建一个连接活动，发起连接请求必需要有该过程
* **定义**  
**`void ln_ble_init_actv_creat(void);`**
* **参数**
* **返回值**



## ln_ble_is_connected
获取蓝牙连接状态
* **定义**  
**`bool ln_ble_is_connected(void)`**
* **参数**
* **返回值**



## ln_ble_conn_mgr_init
连接管理模块初始化
* **定义**  
int ln_ble_conn_mgr_init(void);`**
* **参数**
* **返回值**
|类型|说明|
|:-|:-|
|int|详见错误码`BLE_ERR_CODE_T`|
| | |