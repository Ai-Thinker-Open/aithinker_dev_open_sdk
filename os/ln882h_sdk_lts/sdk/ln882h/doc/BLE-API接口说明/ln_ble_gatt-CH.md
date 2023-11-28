[TOC]
# 引言
gap通用访问接口，可以进行协议栈复位，获取设备信息，设置设备信息等功能
# 宏定义



# 枚举



# 结构体
## ln_gatt_sdp_cmd_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|operation|gatt请求类型，详见`gattc_operation`,这里只用到服务发现相关的命令：`GATTC_SDP_DISC_SVC`，`GATTC_SDP_DISC_SVC_ALL`，`GATTC_SDP_DISC_CANCEL`|
|uint8_t|uuid_len|服务UUID长度|
|uint8_t|start_hdl|服务的起始句柄|
|uint8_t|end_hdl|服务的结束句柄|
|uint8_t[16]|uuid|服务的UUID|



## ln_gatt_disc_cmd_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|operation|gatt请求类型，详见`gattc_operation`|
|uint8_t|uuid_len|服务UUID长度|
|uint8_t|start_hdl|服务的起始句柄|
|uint8_t|end_hdl|服务的结束句柄|
|uint8_t \*|uuid|服务的UUID|



## ln_gatt_att_desc_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t[16]|uuid|属性的UUID|
|uint16_t|perm|属性权限设置|
|uint16_t|max_len|属性的最大长度|
|uint16_t|ext_perm|额外权限说明|



## ln_gatt_svc_desc_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|start_hdl|服务起始句柄，为零时系统自动分配|
|uint8_t|perm|服务权限设置|
|uint8_t|nb_att|服务的属性个数|
|uint8_t[16]|uuid|服务的UUID|
|ln_gatt_att_desc_t \*|atts|属性描述列表，属性描述详见`ln_gatt_att_desc_t`|



## ln_gatt_add_svc_req_t
|类型|参数|描述|
|:-|:-|:-|
|ln_gatt_svc_desc_t|svc_desc|服务描述，详见`ln_gatt_svc_desc_t`|



## ln_gatt_read_simple_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|handle|读取的属性的句柄|
|uint16_t|offset|读取的属性值的偏移|
|uint16_t|length|读取的属性值的长度|



## ln_gatt_read_by_uuid_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|start_hdl|起始句柄|
|uint16_t|end_hdl|结束句柄|
|uint8_t|uuid_len|UUID的长度|
|uint8_t \*|uuid|通过该UUID值，来查找对应的特征，并读取特征值|



## ln_gatt_read_multiple_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|handle|要读取的属性的句柄|
|uint16_t|len|要读取的长度|




## ln_gatt_read_req_t
|类型|参数|描述|
|:-|:-|:-|
|ln_gatt_read_simple_t|simple|通用读取结构体类型|
|ln_gatt_read_by_uuid_t|by_uuid|通过UUID读取结构体类型|
|ln_gatt_read_multiple_t \*|multiple|多属性读取结构体类型|



## ln_gatt_read_cmd_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|operation|请求类型，详见`gattc_operation`|
|uint8_t|nb|读取个数，该参数只在多属性读取时用到|
|ln_gatt_read_req_t|req|读取类型|



## ln_gatt_write_cmd_t
|类型|参数|描述|
|:-|:-|:-|
|uint8_t|operation|请求类型，详见`gattc_operation`，这里主要使用0x0c~0x0f的值|
|uint16_t|auto_execute|自动写入|
|uint16_t|handle|要写入的属性的句柄|
|uint16_t|offset|写入偏移量|
|uint16_t|length|写入的数据长度|
|uint8_t \*|value|写入的数据|



## ln_gatt_send_evt_cmd_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|handle|待发送的属性值的句柄|
|uint16_t|length|发送数据的长度|
|uint8_t \*|value|发送的数据|



## ln_gatt_read_cfm_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|handle|被读取的属性值的句柄|
|uint8_t|status|读取操作的执行结果|
|uint16_t|length|读取的数据的长度|
|uint8_t \*|value|读取的数据|



## ln_gatt_write_cfm_t
|类型|参数|描述|
|:-|:-|:-|
|uint16_t|handle|被写入的属性值的句柄|
|uint8_t|status|写入操作的执行结果|




# 函数
## ln_gatt_exc_mtu
交换mtu，一个连接过程中只能调用一次
* **定义**  
**`void ln_gatt_exc_mtu(int conidx);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
* **返回值**



## ln_gatt_sdp
客户端请求服务发现，可以查找单个服务，也可以查询所有服务
* **定义**  
**`void ln_gatt_sdp(int conidx, ln_gatt_sdp_cmd_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|服务发现命令参数，详见结构体`ln_gatt_sdp_cmd_t`|
* **返回值**



## ln_gatt_discovery
通用服务发现接口，可以用于客户端查找服务器端的：服务(service)，包含(include)，特征(Characteristic)等
* **定义**  
**`void ln_gatt_discovery(int conidx, ln_gatt_disc_cmd_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|服务发现命令参数，详见结构体`ln_gatt_disc_cmd_t`|
* **返回值**



## ln_gatt_add_svc
服务端添加一个服务，比如添加一个电池服务，让客户端可以访问
* **定义**  
**`void ln_gatt_add_svc(ln_gatt_add_svc_req_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|p_param|一个服务的描述，参数内容详见结构体`ln_gatt_add_svc_req_t`|
* **返回值**



## ln_gatt_read
客户端读请求接口
* **定义**  
**`void  ln_gatt_read(int conidx, ln_gatt_read_cmd_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|读取命令参数，参数内容详见结构体`ln_gatt_read_cmd_t`|
* **返回值**



## ln_gatt_write
客户端写请求接口
* **定义**  
**`void  ln_gatt_write(int conidx, ln_gatt_write_cmd_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|读取命令参数，参数内容详见结构体`ln_gatt_write_cmd_t`|
* **返回值**



## ln_gatt_excute_write
写队列执行或者取消命令
* **定义**  
**`void ln_gatt_excute_write(int conidx, bool execute);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|execute|0，取消所有挂起的写入动作;1，将挂起的写入数据全部立即写入|
* **返回值**



## ln_gatt_send_ind
服务器端发送一个指示到客户端，需要收到客户端的回复
* **定义**  
**`void ln_gatt_send_ind(int conidx, ln_gatt_send_evt_cmd_t *p_param );`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|数据发送参数，包括句柄，荷载等，参数内容详见结构体`ln_gatt_send_evt_cmd_t`|
* **返回值**



## ln_gatt_send_ntf
服务器端发送一个通知到客户端，不需要收到客户端的回复
* **定义**  
**`void ln_gatt_send_ntf(int conidx, ln_gatt_send_evt_cmd_t *p_param );`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|数据发送参数，包括句柄，荷载等，参数内容详见结构体`ln_gatt_send_evt_cmd_t`|
* **返回值**



## ln_gatt_read_req_cfm
该函数用于服务器端收到客户端的读取请求之后，给予客户端的回复，回复确认中包含被读取的数据
* **定义**  
**`void ln_gatt_read_req_cfm(uint8_t conidx, ln_gatt_read_cfm_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|读取的回复参数，参数内容详见结构体`ln_gatt_read_cfm_t`|
* **返回值**



## ln_gatt_write_req_cfm
该函数用于服务器端收到客户端的写入请求之后，给予客户端的回复
* **定义**  
**`void ln_gatt_write_req_cfm(uint8_t conidx, ln_gatt_write_cfm_t *p_param);`**
* **参数**
|参数|说明|
|:-|:-|
|conidx|连接索引值，建立连接之后创建，用来标识该连接|
|p_param|写入确认的回复参数，参数内容详见结构体`ln_gatt_write_cfm_t`|
* **返回值**



## ln_gatt_app_init
gatt管理模块初始化
* **定义**  
**`int ln_gatt_app_init(void);`**
* **参数**
* **返回值**
|类型|说明|
|:-|:-|
|int|详见错误码`BLE_ERR_CODE_T`|