[TOC]
# 引言
gap通用访问接口，可以进行协议栈复位，获取设备信息，设置设备信息等功能&nbsp;



# 宏定义



# 枚举
## ln_get_dev_info_cmd
|值|标记|描述|
|:-|:-|:-|
|0x00|GET_INFO_VERSION|获取本地设备的蓝牙版本|
|0x01|GET_INFO_BDADDR|获取本地设备的蓝牙地址|
|0x02|GET_INFO_ADV_TX_POWER|获取广播发射功率|
|0x03|GET_INFO_TX_POWER|获取控制器支持的最大和最小发射功率|
|0x04|GET_INFO_SUGG_DFT_DATA|获取蓝牙数据包的建议值|
|0x05|GET_INFO_SUPP_DFT_DATA|获取蓝牙数据包的支持值|
|0x06|GET_INFO_MAX_ADV_DATA_LEN|获取蓝牙广播包的最大长度|
|0x07|GET_INFO_WLIST_SIZE|获取白名单大小|
|0x08|GET_INFO_RAL_SIZE|获取解析地址列表大小|
|0x09|GET_INFO_PAL_SIZE|获取定期广播者列表大小|
|0x0a|GET_INFO_NB_ADV_SETS|获取可用广播集的数量|



## ln_get_peer_info
|值|标记|描述|
|:-|:-|:-|
|0x00|GET_PEER_NAME|获取对端设备的蓝牙名称|
|0x01|GET_PEER_VERSION|获取对端设备的蓝牙版本|
|0x02|GET_PEER_FEATURES|获取对端设备的特性|
|0x03|GET_CON_RSSI|获取RSSI|
|0x04|GET_CON_CHANNEL_MAP|获取可用的物理信道map|
|0x05|GET_PEER_APPEARANCE|获取对端的表现|
|0x06|GET_PEER_SLV_PREF_PARAMS|获取对端设备从属首选参数|
|0x07|GET_ADDR_RESOL_SUPP|获取支持地址解析|
|0x08|GET_LE_PING_TO|获取PING输入的超时时间|
|0x09|GET_PHY|获取活动连接的链路配置|
|0x0a|GET_CHAN_SEL_ALGO|获取物理信道选择算法|



# 结构体
## ln_gap_set_le_pkt_size_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|tx_octets|在单个链路层数据通道的PDU中，首选的最大负载字节数|
|uint16_t|tx_time|在单个链路层数据通道的PDU中，首选的最大时间，单位为微秒|



## ln_gap_set_dev_config_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|role|设备角色，详细见`gap_role`|
|uint16_t|renew_dur|启用隐私保护时，重新生成设备地址之前的持续时间,单位为秒|
|uint8_t[8]|addr|静态私有随机地址|
|uint8_t[16]|irk|用于生成可解析的随机地址的设备IRK|
|uint8_t|privacy_cfg|隐私配置，值域参考`gapm_priv_cfg`|
|uint8_t|pairing_mode|配对模式，值域参考`gapm_pairing_mode`|
|uint16_t|gap_start_hdl|GAP服务的起始地址|
|uint16_t|gatt_start_hdl|GATT服务的起始地址|
|uint8_t|att_devname_write_perm|设备名称写权限，值域参考`gapm_write_att_perm`|
|uint8_t|att_apperance_write_perm|设备外观写权限，值域参考`gapm_priv_cfg`|
|uint8_t|att_slv_pref_conn_param_present|是否优先启用从设备的连接参数|
|uint8_t|svc_change_feat_present|服务变更特征是否加入GATT属性数据库中|
|uint16_t|sugg_max_tx_octets|ble控制器要使用的最大有效负载的建议值|
|uint16_t|sugg_max_tx_time|ble控制器的最大数据包传输时间的建议值|
|uint16_t|max_mtu|设备可接受的最大MTU|
|uint16_t|max_mps|设备可接受的最大MPS包大小|
|uint8_t|max_nb_lecb|可建立的基于LE Credit的连接的最大数量|
|uint16_t|audio_cfg|LE音频模式配置,值域详见`gapm_audio_cfg_flag`|
|uint8_t|tx_pref_phy|用于数据发送的首选PHY，值域详见`gap_phy'|
|uint8_t|rx_pref_phy|用于数据接收的首选PHY，值域详见`gap_phy'|
|uint16_t|tx_path_comp|数据发送通道的RF补偿值，值域为-128dB~128dB，最小变化量0.1dB|
|uint16_t|rx_path_comp|数据发送通道的RF补偿值，值域为-128dB~128dB，最小变化量0.1dB|



## ln_gap_set_wl_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|size|添加的白名单的个数|
|ln_bdaddr_t \*|wl_info|白名单列表|



## ln_gap_set_phy_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|tx_phy|ble数据发送支持的PHY，值域详见`gap_phy`|
|uint8_t|rx_phy|ble数据接收支持的PHY，值域详见`gap_phy`|
|uint8_t|phy_opt|PHY选项，值域详见`gapc_phy_option`|



## ln_gap_set_channel_map_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t[5]|chmap|ble共有40个物理信道，映射到5个字节中|



# 函数
## ln_gap_reset
蓝牙协议栈复位函数
* **定义**  
**`void ln_gap_reset(void);`**
* **参数**
* **返回值**



## ln_gap_get_dev_info_by_type
获取本地设备信息
* **定义**  
**`void ln_gap_get_dev_info_by_type(uint8_t type);`**
*  **参数**
|参数|说明|
|:--|:--|
|type|操作类型，详见枚举`ln_get_dev_info_cmd`|
* **返回值**



## ln_gap_get_peer_info_by_type
获取对端设备的信息
* **定义**  
**`void ln_gap_get_peer_info_by_type(int conidx, uint8_t type);`**
*  **参数**
|参数|说明|
|:--|:--|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|type|操作类型，详见枚举`ln_get_peer_info`|
* **返回值**



## ln_gap_set_le_pkt_size
设置链路层支持的数据包的大小
* **定义**  
**`void ln_gap_set_le_pkt_size(int conidx, ln_gap_set_le_pkt_size_t *pkt_size);`**
*  **参数**
|参数|说明|
|:--|:--|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|pkt_size|物理链路数据包配置，详见结构体`ln_gap_set_le_pkt_size_t`|
* **返回值**



## ln_gap_set_dev_config
蓝牙参数配置
* **定义**  
**`void ln_gap_set_dev_config(ln_gap_set_dev_config_t *cfg_param);`**
* **参数**
|参数|说明|
|:--|:--|
|cfg_param|低功耗蓝牙相关的参数，详见结构体`ln_gap_set_dev_config_t`|
* **返回值**



## ln_gap_set_white_list
设置低功耗蓝牙的白名单，当一个广播/扫描/连接的过程正在使用白名单，这个命令不可用
* **定义**  
**`void ln_gap_set_white_list(ln_gap_set_wl_t *wlist);`**
*  **参数**
|参数|说明|
|:--|:--|
|wlist|白名单操作及白名单列表，详见结构体`ln_gap_set_wl_t`|
* **返回值**



## ln_gap_set_phy
设置蓝牙发送和接受的物理链路，物理链路有1M，1M和2M编码三种类型，可以同时支持
* **定义**  
**`void ln_gap_set_phy(int conidx, ln_gap_set_phy_t *p_phy);`**
*  **参数**
|参数|说明|
|:--|:--|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_phy|详见结构体`ln_gap_set_phy_t`|
* **返回值**



## ln_gap_set_chn_map
蓝牙物理信道配置，低功耗蓝牙共有40个信道，使用五个字节的以掩码的形式来描述
* **定义**  
**`void ln_gap_set_chn_map(ln_gap_set_channel_map_t *p_chn_map);`**
*  **参数**
|参数|说明|
|:--|:--|
|p_chn_map|详见结构体`ln_gap_set_channel_map_t`|
* **返回值**



## ln_gap_app_init
应用层gap模块初始化
* **定义**  
**`int ln_gap_app_init(void);`**
*  **参数**
* **返回值**
|类型|说明|
|:--|:--|
|int|详见错误代码`BLE_ERR_CODE_T`|