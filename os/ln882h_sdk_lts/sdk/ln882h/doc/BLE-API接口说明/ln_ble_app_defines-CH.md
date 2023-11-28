[TOC]
# 引言
蓝牙应用层通用定义



# 宏定义
    #define LN_RAND_NB_LEN                  (8)
随机数长度  

    #define LN_GAP_KEY_LEN                  (16)
蓝牙密钥长度  

    #define LN_BD_ADDR_LEN                  (6)
蓝牙地址长度  

    #define LN_BLE_CHNL_MAP_LEN             (5)
蓝牙物理信道映射字节数  

    #define LN_ATT_INVALID_HANDLE           (0xFFFF)
att无效句柄  

    #define APP_ACTV_INVALID_IDX            (0xFF)
无效的activity的索引值  

    #define APP_CONN_INVALID_IDX            (0xFF)
无效的连接索引值  

    #define APP_CONN_INVALID_HANDLE         (0xFFFF)

无效的连接句柄  



# 结构体
## ln_bd_addr_t
ble地址字段
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|addr[8]|ble地址|
| | | |



## ln_bdaddr_t
ble地址字段
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|addr_type|ble地址类型，0为静态公有地址，1为随机地址|
|ln_bd_addr_t|addr|地址，描述详见`ln_bd_addr_t`|



## ln_gap_ltk_t
长期密钥描述
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|ltk[16]|配对中生成的长期密钥|
|uint16_t|ediv|多元化加密方式|
|uint8_t|randnb[8]|配对中生成的随机数|
|uint8_t|key_size|密钥长度|



## ln_gap_irk_t
身份解析密钥描述
|类型|参数|描述|
|:-|:-|:-|
|uint8_t |key[16]|密钥字段|
| | | |