## LLSync SDK标准蓝牙功能接入指引

为了提升您在新的硬件平台适配`LLSync SDK`的效率，本文档选择一款硬件开发板进行移植，向您展示`LLSync SDK`的源码移植适配、功能开发、功能验证的完整过程。

`LLSync SDK`移植的主要流程如下。

1. 硬件设备选择
2. 控制台创建产品
3. 获取`SDK`
4. 移植`SDK`
5. BLE功能验证
6. 其他功能开发及验证

### 一、硬件设备选择

`LLSync SDK`的资源需求如下，请您结合自身产品特性选择合适的硬件设备。

| 资源名称  | 推荐要求 |
| :-------- | :------- |
| BLE协议栈 | ≥BLE 4.2 |
| Flash     | 32KByte  |
| RAM       | 2KByte   |

*以上资源占用是LLSync全功能版本在`ESP32`上统计得到，不同硬件平台可能存在差异，该数据仅供参考。*

本文选择`ESP32`作为目标硬件平台进行`LLSync SDK`移植。

### 二、控制台创建产品

1. 登录[物联网开发平台](https://console.cloud.tencent.com/iotexplorer)。
2. 选择[新建项目](https://cloud.tencent.com/document/product/1081/50969#.E6.96.B0.E5.BB.BA.E9.A1.B9.E7.9B.AE)。
3. [创建产品](https://cloud.tencent.com/document/product/1081/50969#.E6.96.B0.E5.BB.BA.E4.BA.A7.E5.93.81)，通信方式选择`BLE`。
4. 根据产品特性添加[数据模版](https://cloud.tencent.com/document/product/1081/50969#.E5.88.9B.E5.BB.BA.E6.95.B0.E6.8D.AE.E6.A8.A1.E6.9D.BF)。
5. 选择设备开发方式为[基于标准蓝牙协议开发](https://cloud.tencent.com/document/product/1081/50969#.E9.80.89.E6.8B.A9.E8.AE.BE.E5.A4.87.E5.BC.80.E5.8F.91.E6.96.B9.E5.BC.8F)。
6. [创建设备](https://cloud.tencent.com/document/product/1081/50969#.E6.96.B0.E5.BB.BA.E8.AE.BE.E5.A4.87)。

### 三、获取SDK

请下载最新版本的`LLSync SDK`，[下载地址](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded)。

### 四、移植SDK

移植`SDK`可以分为几步进行，我们已经在[ESP32 Demo](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-iot-ble-esp32)和[nrf52832](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-iot-ble-nrf52832)上进行了移植供您参考。

1. 配置文件编写

   编辑`config/ble_qiot_config.h`文件，进行功能配置。

   ```c
   // 配置为0，SDK初始化后默认广播。配置为1，SDK初始化不广播，需要通过设备UI操作触发广播（例如按键）。
   #define BLE_QIOT_BUTTON_BROADCAST 		(0)
   
   // 按键广播超时后自动关闭广播功能，默认超时时间120 * 1000毫秒
   #define BLE_QIOT_BIND_TIMEOUT (2 * 60 * 1000)
   
   #define __ORDER_LITTLE_ENDIAN__ 			(1234)
   #define __ORDER_BIG_ENDIAN__    			(4321)
   // 设备大小端定义。
   #define __BYTE_ORDER__          			(__ORDER_LITTLE_ENDIAN__)
   
   // 设备端串口输出函数，普通日志输出和Hex数据输出。
   #define BLE_QIOT_LOG_PRINT(...) 			printf(__VA_ARGS__)
   #define BLE_QIOT_USER_DEFINE_HEXDUMP 	0
   
   // 设备端和小程序通信过程中的单条数据最大长度，参考数据模版内所有属性、单个事件、单个行为最大长度定义。
   #define BLE_QIOT_EVENT_MAX_SIZE 			(128)
   
   // 取MTU和BLE_QIOT_EVENT_MAX_SIZE中的较小值，LLSync会对数据分片后再发送。
   #define BLE_QIOT_EVENT_BUF_SIZE 			(23)
   
   // 配置为1，建立蓝牙连接后小程序尝试设置MTU，通过ble_get_user_data_mtu_size接口获取；配置为0，默认MTU为23。
   // 请参考<<LLSync蓝牙设备接入协议>>中MTU部分描述。
   #define BLE_QIOT_REMOTE_SET_MTU 			(1)
   
   // 配置为1，使能LLSync标准蓝牙
   #define BLE_QIOT_LLSYNC_STANDARD    	(1)
   
   // 配置为1，使能动态注册功能
   #define BLE_QIOT_DYNREG_ENABLE  			(1)
   
   // 配置为1，小程序发起绑定请求后需要设备端通过UI操作确认。配置为0，绑定不需要设备端确认。
   #define BLE_QIOT_SECURE_BIND 					(0)
   
   // 小程序发起绑定请求时等待设备端确认超时时间，默认时间 60 秒
   #define BLE_QIOT_BIND_WAIT_TIME 			(60)
   
   // LLSync Core信息存储地址，长度小于128字节，需要设备端分配存储地址
   #define BLE_QIOT_RECORD_FLASH_ADDR 		(0x3F000)
   
   // 设备端版本号定义，OTA时需要和云端进行版本号比较，仅支持向上升级
   #define BLE_QIOT_USER_DEVELOPER_VERSION "0.0.1"
   
   // 配置为1，支持OTA功能。配置为0，不支持OTA功能
   #define BLE_QIOT_SUPPORT_OTA 					(0)
   
   // 配置为1，支持断点续传功能。配置为0，不支持断点续传功能
   #define BLE_QIOT_SUPPORT_RESUMING 		(1)
   
   // OTA断点信息储存地址，长度小于128字节
   #define BLE_QIOT_OTA_INFO_FLASH_ADDR (0x3E000)
   
   // OTA过程中，小程序连续发送指定数量数据包后，设备端需要回复一个确认报文
   #define BLE_QIOT_TOTAL_PACKAGES 			(0xFF)
   
   // OTA过程中，小程序每次发送的数据包中有效升级数据的长度
   #define BLE_QIOT_PACKAGE_LENGTH 			(0x10)
   
   // OTA过程中，失败重传的最大次数
   #define BLE_QIOT_RETRY_TIMEOUT  			(0x05)
   
   // OTA过程中，文件下载结束后，小程序等待设备重新启动的最大时间，单位：秒
   #define BLE_QIOT_REBOOT_TIME      		(0x14)
   
   // OTA过程中，小程序连续发送两包的发送间隔，单位：毫秒
   #define BLE_QIOT_PACKAGE_INTERVAL 		(0x05)
   
   // OTA过程中，设备端升级数据的缓存大小，当缓存内没有空间存储接收数据时会触发一次写数据操作
   #define BLE_QIOT_OTA_BUF_SIZE 				(4096)
   ```

   您可以参考`ESP32`的[配置文件](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/samples/esp32/components/qcloud_llsync/hal/ble_qiot_config.h)来编写您自己产品的配置文件。

2. 设备接口适配

   `inc/ble_qiot_import.h`中定义了`LLSync SDK`依赖的设备`HAL`实现，需要您在自己的硬件平台上进行实现。

   ```c
   /* 获取设备的Product ID，示例：
   #define PRODUCT_ID  "DHZX03IQAZ"
   int ble_get_product_id(char *product_id)
   {
       memcpy(product_id, PRODUCT_ID, strlen(PRODUCT_ID));
       return 0;
   }
   */
   int ble_get_product_id(char *product_id);
   
   /* 获取设备的Device Name，示例：
   #define DEVICE_NAME "test001"
   int ble_get_device_name(char *device_name)
   {
       memcpy(device_name, DEVICE_NAME, strlen(DEVICE_NAME));
       return strlen(DEVICE_NAME);
   }
   */
   int ble_get_device_name(char *device_name);
   
   /* 获取设备密钥，示例：
   #define SECRET_KEY  "LvgDDBIoXibsQFq3OCPSXg=="
   int ble_get_psk(char *psk)
   {
       memcpy(psk, SECRET_KEY, strlen(SECRET_KEY));
       return 0;
   }
   当启用动态注册，并且没有设备密钥时，psk使用0xFF填充，SDK通过psk内容判断设备使用需要动态注册。
   */
   int ble_get_psk(char *psk);
   
   /* 存储设备密钥，启用动态注册功能时实现。示例：
   int ble_set_psk(const char *psk, uint8_t len)
   {
       memcpy(sg_device_secret, psk, BLE_QIOT_PSK_LEN);
       return 0;
   }
   */
   int ble_set_psk(const char *psk, uint8_t len);
   
   /* 获取产品密钥，启用动态注册功能时实现。示例：
   #define PRODUCT_KEY "LMygrr2b6npAQM2vl5kLSCQt"
   int ble_get_product_key(char *product_secret)
   {
       memcpy(product_secret, PRODUCT_KEY, strlen(PRODUCT_KEY));
       return 0;
   }
   */
   int ble_get_product_key(char *product_secret);
   
   /* 获取设备MAC地址，示例：
   int ble_get_mac(char *mac)
   {
       char *address = (char *)esp_bt_dev_get_address();
       memcpy(mac, address, 6);
       return 0;
   }
   */
   int ble_get_mac(char *mac);
   
   /* 设备写Flash操作，示例：
   int ble_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
   {
       int ret = spi_flash_erase_range(flash_addr, BLE_QIOT_RECORD_FLASH_PAGESIZE);
       ret     = spi_flash_write(flash_addr, write_buf, write_len);
       return ret == ESP_OK ? write_len : ret;
   }
   */
   int ble_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len);
   
   /* 设备读Flash操作，示例：
   int ble_read_flash(uint32_t flash_addr, char *read_buf, uint16_t read_len)
   {
       int ret = spi_flash_read(flash_addr, read_buf, read_len);
       return ret == ESP_OK ? read_len : ret;
   }
   */
   int ble_read_flash(uint32_t flash_addr, char *read_buf, uint16_t read_len);
   
   /* 是否允许设备OTA，允许用户在某些场景禁止OTA功能，例如低电量、设备使用中。仅在OTA启用时适配。示例：
   uint8_t ble_ota_is_enable(const char *version)
   {
       ble_qiot_log_e("ota version: %s, enable ota", version);
       return BLE_OTA_ENABLE;
   }
   */
   uint8_t ble_ota_is_enable(const char *version);
   
   /* 获取OTA文件存储地址。仅在OTA启用时适配。示例：
   uint32_t ble_ota_get_download_addr(void)
   {
       esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);
       ble_qiot_log_i("otafile download address: %d", partition->address);
       return partition->address;
   }
   */
   uint32_t ble_ota_get_download_addr(void);
   
   /* OTA写数据接口，部分设备普通数据和OTA数据写接口存在差异。仅在OTA启用时适配。示例：
   int ble_ota_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len)
   {
       int ret = 0;
       if (flash_addr % 4096 == 0) {
           ret = spi_flash_erase_range(flash_addr, 4096);
       } else {
           if ((flash_addr + write_len - 1) / 4096 != flash_addr / 4096) {
               ret = spi_flash_erase_range(((flash_addr / 4096) + 1) * 4096, 4096);
           }
       }
       ret = spi_flash_write(flash_addr, write_buf, write_len);
       return ret == ESP_OK ? write_len : ret;
   }
   */
   int ble_ota_write_flash(uint32_t flash_addr, const char *write_buf, uint16_t write_len);
   
   
   /* 小程序请求绑定设备时该接口被调用。仅在设备确认绑定功能开启时适配。示例：
   void ble_secure_bind_user_cb(void)
   {
       printf("please choose connect?(Y/N)\r\n");
       return;
   }
   */
   void ble_secure_bind_user_cb(void);
   
   /* 小程序取消绑定请求时通知用户取消绑定请求原因，可能是设备确认超时或小程序主动取消。仅在设备确认绑定功能开启时适配。示例：
   void ble_secure_bind_user_notify(uint8_t result)
   {
       printf("the binding canceled, result: %d\r\n", result);
       return;
   }
   */
   void ble_secure_bind_user_notify(uint8_t result);
   
   /* 定时器创建接口。仅在启用按键广播功能或OTA功能时需要适配。示例：
   typedef struct ble_esp32_timer_id_ {
       uint8_t       type;
       ble_timer_cb  handle;
       TimerHandle_t timer;
   } ble_esp32_timer_id;
   ble_timer_t ble_timer_create(uint8_t type, ble_timer_cb timeout_handle)
   {
       ble_esp32_timer_id *p_timer = malloc(sizeof(ble_esp32_timer_id));
       if (NULL == p_timer) {
           return NULL;
       }
       p_timer->type   = type;
       p_timer->handle = timeout_handle;
       p_timer->timer  = NULL;
       return (ble_timer_t)p_timer;
   }
   */
   ble_timer_t ble_timer_create(uint8_t type, ble_timer_cb timeout_handle);
   
   /* 定时器启动接口。仅在启用按键广播功能或OTA功能时需要适配。示例：
   ble_qiot_ret_status_t ble_timer_start(ble_timer_t timer_id, uint32_t period)
   {
       ble_esp32_timer_id *p_timer = (ble_esp32_timer_id *)timer_id;
       if (NULL == p_timer->timer) {
           p_timer->timer =
               (ble_timer_t)xTimerCreate("ota_timer", period / portTICK_PERIOD_MS,
   									p_timer->type == BLE_TIMER_PERIOD_TYPE ? pdTRUE : pdFALSE, NULL, p_timer->handle);
       }
       xTimerReset(p_timer->timer, portMAX_DELAY);
       return BLE_QIOT_RS_OK;
   }
   */
   ble_qiot_ret_status_t ble_timer_start(ble_timer_t timer_id, uint32_t period);
   
   /* 定时器停止接口。仅在启用按键广播功能或OTA功能时需要适配。示例：
   ble_qiot_ret_status_t ble_timer_stop(ble_timer_t timer_id)
   {
       ble_esp32_timer_id *p_timer = (ble_esp32_timer_id *)timer_id;
       xTimerStop(p_timer->timer, portMAX_DELAY);
       return BLE_QIOT_RS_OK;
   }
   */
   ble_qiot_ret_status_t ble_timer_stop(ble_timer_t timer_id);
   
   /* 定时器删除接口。仅在启用按键广播功能或OTA功能时需要适配。示例：
   ble_qiot_ret_status_t ble_timer_delete(ble_timer_t timer_id)
   {
       ble_esp32_timer_id *p_timer = (ble_esp32_timer_id *)timer_id;
       xTimerDelete(p_timer->timer, portMAX_DELAY);
       free(p_timer);
       return BLE_QIOT_RS_OK;
   }
   */
   ble_qiot_ret_status_t ble_timer_delete(ble_timer_t timer_id);
   
   /* 设备启动接口，设备绑定+连接成功后SDK会调用此函数 */
    void ble_qiot_dev_start(void);

   ```

   您可以参考`ESP32`的[接口](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/samples/esp32/components/qcloud_llsync/hal/ble_qiot_ble_device.c)实现。

3. 蓝牙协议栈适配

   `inc/ble_qiot_import.h`中定义了`LLSync SDK`依赖的蓝牙协议栈`HAL`实现，需要您在自己的硬件平台上进行实现。

   ```c
   /* 通过ble_get_qiot_services获取蓝牙服务，将蓝牙服务添加到协议栈。配网功能使用的服务UUID是0xFFE0，特征值UUID是0xFFE1、0xFFE2、0xFFE3和0xFFE4。您也可以选择其他方式将服务添加到协议栈，示例为ESP32上添加蓝牙服务代码：
   static uint8_t llsync_service_uuid[16] = {
       0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0xe0, 0xff, 0x00, 0x00,};
   static uint8_t llsync_device_info_uuid[16] = {
       0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0xe1, 0xff, 0x00, 0x00,};
   static uint8_t llsync_data_uuid[16] = {
       0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0xe2, 0xff, 0x00, 0x00,};
   static uint8_t llsync_event_uuid[16] = {
       0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0xe3, 0xff, 0x00, 0x00,};
   static uint8_t llsync_ota_uuid[16] = {
       0xe2, 0xa4, 0x1b, 0x54, 0x93, 0xe4, 0x6a, 0xb5, 0x20, 0x4e, 0xd0, 0x65, 0xe4, 0xff, 0x00, 0x00,};
   static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] = {
       // Service Declaration
       [IDX_SVC] = {{ESP_GATT_AUTO_RSP},
                    {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(llsync_service_uuid), sizeof(llsync_service_uuid), (uint8_t *)llsync_service_uuid}},
       [IDX_CHAR_A] = {{ESP_GATT_AUTO_RSP},
                       {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},
       [IDX_CHAR_VAL_A] = {{ESP_GATT_AUTO_RSP},
   				{ESP_UUID_LEN_128, (uint8_t *)llsync_device_info_uuid, ESP_GATT_PERM_WRITE, LLSYNC_CHAR_VAL_LEN_MAX, 0, NULL}},
       [IDX_CHAR_B] = {{ESP_GATT_AUTO_RSP},
                       {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},
       [IDX_CHAR_VAL_B] = {{ESP_GATT_AUTO_RSP},
                           {ESP_UUID_LEN_128, (uint8_t *)llsync_data_uuid, ESP_GATT_PERM_WRITE, LLSYNC_CHAR_VAL_LEN_MAX, 0, NULL}},
       [IDX_CHAR_C] = {{ESP_GATT_AUTO_RSP},
                       {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_notify}},
       [IDX_CHAR_VAL_C] = {{ESP_GATT_AUTO_RSP},
                           {ESP_UUID_LEN_128, (uint8_t *)llsync_event_uuid, 0, LLSYNC_CHAR_VAL_LEN_MAX, 0, NULL}},
       [IDX_CHAR_CFG_C] = {{ESP_GATT_AUTO_RSP},
                           {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
                            ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(uint16_t), sizeof(heart_measurement_ccc), (uint8_t *)heart_measurement_ccc}},
       [IDX_CHAR_D] = {{ESP_GATT_AUTO_RSP},
                       {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write_no_rsp}},
       [IDX_CHAR_VAL_D] = {{ESP_GATT_AUTO_RSP},
                           {ESP_UUID_LEN_128, (uint8_t *)llsync_ota_uuid, ESP_GATT_PERM_WRITE, LLSYNC_CHAR_VAL_LEN_MAX, 0, NULL}},
   };
   ……     
   esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
   };
   
   ESP32上并未使用ble_services_add接口，您也可以参考nrf52832上添加蓝牙服务代码。
   */
   void ble_services_add(const qiot_service_init_s *p_service);
   
   /* 通过ble_get_my_broadcast_data获取广播数据，将其作为厂商数据开始广播，Compnay ID是0xFEE7。示例：
   ……
   adv_info_s my_adv_info;
   adv_data_len = ble_get_my_broadcast_data((char *)adv_data, sizeof(adv_data));
   my_adv_info.manufacturer_info.company_identifier = TENCENT_COMPANY_IDENTIFIER;
   my_adv_info.manufacturer_info.adv_data           = adv_data;
   my_adv_info.manufacturer_info.adv_data_len       = adv_data_len;
   ble_advertising_start(&my_adv_info);
   ……
   ble_qiot_ret_status_t ble_advertising_start(adv_info_s *adv)
   {
   		static uint8_t raw_adv_data[32] = {0x02, 0x01, 0x06, 0x03, 0x03, 0xF0, 0xFF};
       uint8_t usr_adv_data[31] = {0};
       uint8_t len              = 0;
       uint8_t index            = 0;
       memcpy(usr_adv_data, &adv->manufacturer_info.company_identifier, sizeof(uint16_t));
       len = sizeof(uint16_t);
       memcpy(usr_adv_data + len, adv->manufacturer_info.adv_data, adv->manufacturer_info.adv_data_len);
       len += adv->manufacturer_info.adv_data_len;
   
       index                 = 7;
       raw_adv_data[index++] = len + 1;
       raw_adv_data[index++] = 0xFF;
       // 添加厂商广播数据
       memcpy(raw_adv_data + index, usr_adv_data, len);
       index += len;
       ……
       esp_log_buffer_hex("adv", raw_adv_data, index);
       esp_err_t ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, index);
       if (ret) {
           ESP_LOGE(LLSYNC_LOG_TAG, "config adv data failed, error code = %x", ret);
       }
       adv_config_done |= ADV_CONFIG_FLAG;
       esp_ble_gap_start_advertising(&adv_params);
       return 0;
   }
   */
   ble_qiot_ret_status_t ble_advertising_start(adv_info_s *adv);
   
   /* 停止广播接口，示例：
   ble_qiot_ret_status_t ble_advertising_stop(void)
   {
       esp_ble_gap_stop_advertising();
       return 0;
   }
   */
   ble_qiot_ret_status_t ble_advertising_stop(void);
   
   /* 特征值UUID FFE3向小程序写数据的接口，示例：
   ble_qiot_ret_status_t ble_send_notify(uint8_t *buf, uint8_t len)
   {
       esp_ble_gatts_send_indicate(llsync_profile_tab[PROFILE_APP_IDX].gatts_if,
   			llsync_profile_tab[PROFILE_APP_IDX].conn_id, llsync_handle_table[IDX_CHAR_VAL_C], len, buf, false);
       return BLE_QIOT_RS_OK;
   }
   */
   ble_qiot_ret_status_t ble_send_notify(uint8_t *buf, uint8_t len);
   
   /* 用户指定MTU。当BLE_QIOT_REMOTE_SET_MTU设置为1时，小程序连接设备后会去修改MTU并通知设备。否则使用默认MTU(23)进行通信。
   uint16_t ble_get_user_data_mtu_size(e_system type)
   {
       return (SYSTEM_IS_ANDROID == type) ? 128 : 500;
   }
   */
   uint16_t ble_get_user_data_mtu_size(e_system type);
   ```

   您可以参考`ESP32`的蓝牙协议栈适配代码，[请参见](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/samples/esp32/components/qcloud_llsync/hal/ble_qiot_ble_service.c)。

4. 数据模版开发

   `LLSync SDK`提供了`Python`脚本，可以快速将`Json`格式的数据模版转换为`C`代码模版，提升您的开发效率。脚本使用方法：

   ```shell
   iot$ python3 interpret_dt_ble.py ../example.json 
   reading json file start
   reading json file end
   generate header file start
   generate header file end
   generate source file start
   generate source file end
   ```

   其中，`example.json`是您的物模型`json`文本文件，命令执行结束会生成模版文件`ble_qiot_template.c`和`ble_qiot_template.h`文件，`ble_qiot_template.c`需要您做应用代码实现。示例：

   物模型内容如下：

   ```json
   {
    "id": "switch",
    "name": "开关",
    "desc": "",
    "mode": "rw",
    "define": {
    "type": "bool",
    "mapping": {
    "0": "关",
    "1": "开"
    }
    },
    "required": false
    },
   ```

   经过脚本转换后，在`ble_qiot_template.c`中对应`C`代码如下：

   ```c
   static int ble_property_switch_set(const char *data, uint16_t len)
   {
       return 0;
   }
   static int ble_property_switch_get(char *data, uint16_t buf_len)
   {
       return sizeof(uint8_t);
   }
   ```

   假设该物模型对应的是灯泡开关功能，您只需要在`ble_property_switch_set`函数中进行灯泡开关控制，在`ble_property_switch_get`函数中获取灯泡实时状态即可。

   ```c
   static bool     sg_switch     = 0;
   static int ble_property_switch_set(const char *data, uint16_t len)
   {
       sg_switch = data[0];
     	if (sg_switch){
         open_light();
       }else{
         close_light();
       }
       return 0;
   }
   static int ble_property_switch_get(char *data, uint16_t buf_len)
   {
       data[0] = sg_switch;
       return sizeof(uint8_t);
   }
   ```

   更多实现请您参考`ESP32`的[示例代码](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/blob/master/qcloud-iot-ble-esp32/components/qcloud_llsync/date_template/ble_qiot_template.c)。

5. `SDK`编译

   `src`是源码目录，`inc`是头文件目录。不同产品的编译方式不同，您可以参考`ESP32`的[CMakeLists.txt](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/blob/master/qcloud-iot-ble-esp32/components/qcloud_llsync/CMakeLists.txt)。

6. `API`调用

   `inc/ble_qiot_export.h`中定义了`LLSync SDK`对外提供的`API`。

   ```c
   /* 获取LLSync蓝牙服务，您可以在代码中获取蓝牙服务后将蓝牙服务添加到蓝牙协议栈。*/
   const qiot_service_init_s *ble_get_qiot_services(void);
   
   /* LLSync SDK初始化接口，主要进行蓝牙服务添加和Flash数据初始化。
   int main(void)
   {
   		……
       ble_stack_init();
       gap_params_init();
       gatt_init();
       services_init();
   		……
       ble_qiot_explorer_init();
       ……
   }
   */
   ble_qiot_ret_status_t ble_qiot_explorer_init(void);
   
   /* LLSync广播启动接口，请你选择合适的位置调用。例如在按键或者蓝牙断开时重新开始广播。
   ……
   ESP_LOGI(LLSYNC_LOG_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
   ble_qiot_advertising_start();
   ……
   */
   ble_qiot_ret_status_t ble_qiot_advertising_start(void);
   
   /* LLSync广播停止接口，请你选择合适的位置调用。*/
   ble_qiot_ret_status_t ble_qiot_advertising_stop(void);
   
   /* GAP连接时通知LLSync SDK。例如：
   case ESP_GATTS_CONNECT_EVT:
   		……
   		esp_ble_gap_update_conn_params(&conn_params);
   		ble_gap_connect_cb();
       ……
   */
   void ble_gap_connect_cb(void);
   
   /* GAP断开时通知LLSync SDK。例如：
   ……
   ESP_LOGI(LLSYNC_LOG_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
   ble_gap_disconnect_cb();
   ……
   */
   void ble_gap_disconnect_cb(void);
   
   /* 小程序无法获取蓝牙MTU接口，设备端收到MTU修改通知后调用此接口通知小程序。例如：
   case ESP_GATTS_MTU_EVT:
   		ESP_LOGI(LLSYNC_LOG_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
   		ble_event_sync_mtu(param->mtu.mtu);
   		break;
   */
   ble_qiot_ret_status_t ble_event_sync_mtu(uint16_t llsync_mtu);
   
   /* 特征值UUID FFE1数据处理接口，收到数据后调用此接口。例如：
   case ESP_GATTS_WRITE_EVT:
   		if (!param->write.is_prep) {
   				// the data length of gattc write  must be less than LLSYNC_CHAR_VAL_LEN_MAX.
   				if (param->write.handle == llsync_handle_table[IDX_CHAR_VAL_A]) {
                       ble_device_info_write_cb(param->write.value, param->write.len);}
   				……
   		}           
   */
   void ble_device_info_write_cb(const uint8_t *buf, uint16_t len);
   
   /* 特征值UUID FFE2数据处理接口，收到数据后调用此接口。例如：
   case ESP_GATTS_WRITE_EVT:
   		if (!param->write.is_prep) {
   				// the data length of gattc write  must be less than LLSYNC_CHAR_VAL_LEN_MAX.
   				if (param->write.handle == llsync_handle_table[IDX_CHAR_VAL_B]) {
                       ble_lldata_write_cb(param->write.value, param->write.len);}
   				……
   		}           
   */
   void ble_lldata_write_cb(const uint8_t *buf, uint16_t len);
   
   /* 特征值UUID FFE4数据处理接口，收到数据后调用此接口。例如：
   case ESP_GATTS_WRITE_EVT:
   		if (!param->write.is_prep) {
   				// the data length of gattc write  must be less than LLSYNC_CHAR_VAL_LEN_MAX.
   				if (param->write.handle == llsync_handle_table[IDX_CHAR_VAL_D]) {
                       ble_ota_write_cb(param->write.value, param->write.len);}
   				……
   		}           
   */
   void ble_ota_write_cb(const uint8_t *buf, uint16_t len);
   
   /* 向云端请求设备设备属性的最新信息。调用此接口后，云端会下发设备属性的最新信息。请您根据需求调用。示例：
   void ble_iot_button4_change(uint8_t button_action)
   {
       static uint8_t last_button_action = 0;
       if (last_button_action == 1 && button_action == 0) {
           ble_event_get_status();    
       }
       last_button_action = button_action;
   }
   */
   ble_qiot_ret_status_t ble_event_get_status(void);
   
   /* 向云端上报设备属性信息。调用此接口后，设备端上报属性信息。请您根据需求调用。示例：
   void ble_iot_button4_change(uint8_t button_action)
   {
       static uint8_t last_button_action = 0;
       if (last_button_action == 1 && button_action == 0) {
           ble_event_report_property();
       }
       last_button_action = button_action;
   }
   */
   ble_qiot_ret_status_t ble_event_report_property(void);
   
   /* 向云端上报设备事件。调用此接口后，设备端上报指定ID的事件信息。请您根据需求调用。示例：
   void ble_iot_button2_change(uint8_t button_action)
   {
       static uint8_t last_button_action = 0;
       if (last_button_action == 1 && button_action == 0) {
           ble_event_post(0);
       }
       last_button_action = button_action;
   }
   */
   ble_qiot_ret_status_t ble_event_post(uint8_t event_id);
   
   /* 设备端请求绑定时，设备端通过此接口通知小程序是否允许绑定。仅在请求绑定功能启用时有用。示例：
   ble_secure_bind_user_confirm(BLE_QIOT_SECURE_BIND_CONFIRM);
   */
   ble_qiot_ret_status_t ble_secure_bind_user_confirm(ble_qiot_secure_bind_t choose);
   
   /* OTA函数注册接口，注册OTA启动通知函数、结束通知函数、文件校验函数。示例：
   void ble_ota_start_cb(void)
   {
       printf("ble ota start callback\r\n");
       return;
   }
   ble_qiot_ret_status_t ble_ota_valid_file_cb(uint32_t file_size, char *file_version)
   {
       printf("user valid file, size %d, file_version: %s\r\n", file_size, file_version);
       return BLE_QIOT_RS_OK;
   }
   
   static void ble_ota_reboot_timer(void *param)
   {
       esp_restart();
   }
   void ble_ota_stop_cb(uint8_t result)
   {
       printf("ble ota stop callback, result %d\r\n", result);
       if (result == BLE_QIOT_OTA_SUCCESS) {
           esp_partition_t *partition = esp_ota_get_next_update_partition(NULL);
           esp_ota_set_boot_partition(partition);
           printf("ota success, device restart after 5 seconds\r\n");
           ota_reboot_timer = xTimerCreate("reboot_timer", 5000 / portTICK_PERIOD_MS, pdFALSE, NULL, ble_ota_reboot_timer);
           xTimerStart(ota_reboot_timer, portMAX_DELAY);
       }
       return;
   }
   注意：接收到OTA结束通知后，可以延时下再重启设备，确认设备端最后一个确认报文发送成功。
   */
   void ble_ota_callback_reg(ble_ota_start_callback start_cb, ble_ota_stop_callback stop_cb,
                             ble_ota_valid_file_callback valid_file_cb)
   ```
   
   您可以参考`ESP32`的[示例代码1](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/samples/esp32/components/qcloud_llsync/hal/ble_qiot_ble_service.c)，[示例代码2](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/samples/esp32/main/app_main.c)。

### 五、BLE功能验证

​	您完成`LLSync SDK`移植后，可以先通过一些`BLE`调试工具验证广播和特征值读写是否正常。

 1. 广播

    ![](https://main.qcloudimg.com/raw/4562c9844dc7bbf4d6a1b0b6478f2221.png)

    对广播数据中标出的字段进行解析。

    | 字段值                 | 含义                                  |
    | ---------------------- | ------------------------------------- |
    | 0xE0FF                 | LLSync Service uuid，0xFFE0           |
    | 0xE7FF                 | 腾讯公司厂商ID，0xFFE7                |
    | 0x21                   | LLSync协议版本号2，设备处于绑定中状态 |
    | 0x4C11AEEB78AA         | BLE设备MAC地址                        |
    | 0x51575150443330594A59 | Product ID，QWQPD30YJY                |

 2. 特征值

    使用调试工具连接`BLE`设备后，可以看到蓝牙服务如下。

    ![](https://main.qcloudimg.com/raw/5ad6e44c3b6685a9359b244cb8046a25.jpg)

    请确认蓝牙服务和特征值的UUID和读写权限正确，您可以通过蓝牙测试工具验证各特征值的读写是否正常。

### 六、其他功能开发及验证

1. 安全绑定功除了设备端启用功能宏(`BLE_QIOT_SECURE_BIND`)外，还需要在`腾讯云物联网开发平台`进行设置。

   ![](https://main.qcloudimg.com/raw/d31bdfff4d2b899e9cbb448502be2fb7.jpg)

2. 数据模版通信能力验证请[参见](https://cloud.tencent.com/document/product/1081/50969#.E8.85.BE.E8.AE.AF.E8.BF.9E.E8.BF.9E.E5.B0.8F.E7.A8.8B.E5.BA.8F.E8.B0.83.E8.AF.95)。

3. OTA能力验证请[参见](https://cloud.tencent.com/document/product/1081/50973)。