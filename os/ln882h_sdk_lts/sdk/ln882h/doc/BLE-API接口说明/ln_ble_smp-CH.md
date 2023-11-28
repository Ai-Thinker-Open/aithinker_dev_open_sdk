[TOC]
# 引言
蓝牙安全管理模块，提供加密，配对等接口



# 宏定义



# 枚举



# 结构体
## ln_gapc_encrypt_cmd
|类型|参数|描述|
|:-|:-|:-|
|struct gapc_ltk|ltk|长期密钥的信息|
| | |


## ln_smp_config_t
蓝牙配对加密配置
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|iocap|io功能属性描述，详见`gap_io_cap`|
|uint8_t|oob|带外属性描述，详见`gap_oob`|
|uint8_t|auth|认证属性描述，详见`gap_auth`|
|uint8_t|key_size|加密密钥长度|
|uint8_t|ikey_dist|启动端密钥派发情况描述，详见`gap_kdist`|
|uint8_t|rkey_dist|接受端密钥派发情况描述，详见`gap_kdist`|
|uint8_t|sec_req|安全请求等级，详见`gap_sec_req`|
|int|static_key|静态配对码，当小于0时使用随机配对码|



# 函数
## ln_smp_bond_req
主机发起绑定请求  
* **定义**  
**`void ln_smp_bond_req(int conidx);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
* **返回值** 



## ln_smp_encrypt_start
发起加密请求  
* **定义**  
**`void ln_smp_encrypt_start(int conidx, ln_encrypt_info_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
* **返回值**



## ln_smp_req_security
从机请求进行安全认证  
* **定义**  
**`void ln_smp_req_security(int conidx, uint8_t auth);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|auth|配对方式，详见枚举`gap_auth`|
* **返回值**



## ln_app_set_ping_timeout
设置ping码输入的超时时间  
* **定义**  
**`void ln_app_set_ping_timeout(int conidx, uint16_t timeout);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|auth|配对方式，详见枚举`gap_auth`|
* **返回值**



## ln_smp_rd_p256_public_key
生成public key  
* **定义**  
**`void ln_smp_rd_p256_public_key(void *cbk);`**
* **参数**
|参数|说明|
|:-|:-|
|cbk|注册回调函数，当获取到public key之后，被调用|
| | |
* **返回值**



## ln_smp_gen_dh_key
生成dh key  
* **定义**  
**`void ln_smp_gen_dh_key(const uint8_t remote_pk[64], void *cbk);`**
* **参数**
|参数|说明|
|:-|:-|
|remote_pk[64]|public key|
|cbk|注册回调函数，当成功生成dh之后，被调用|
* **返回值**



## ln_smp_gen_random_nb
生成8字节的随机数 
* **定义**  
**`void ln_smp_gen_dh_key(void *cbk);`**
* **参数**
|参数|说明|
|:-|:-|
|cbk|注册回调函数，当成功生成随机数之后，被调用|
| | |
* **返回值**



## ln_smp_gen_local_irk
生成irk  
* **定义**  
**`void ln_smp_gen_local_irk(void);`**
* **参数**
* **返回值**



## ln_ble_smp_init
安全管理模块初始化
* **定义**  
**`int ln_ble_smp_init(void);`**
* **参数**
* **返回值**
|类型|说明|
|:-|:-|
|int|详见错误码`BLE_ERR_CODE_T`|