
- [LLSync相关文档](#llsync相关文档)
- [标准蓝牙功能问题](#标准蓝牙功能问题)
  - [编译缺少ble_qiot_template.c和ble_qiot_template.h](#编译缺少ble_qiot_templatec和ble_qiot_templateh)
  - [小程序发现页搜索不到设备](#小程序发现页搜索不到设备)
  - [小程序绑定设备失败](#小程序绑定设备失败)
  - [设备绑定成功后,再次连接设备失败](#设备绑定成功后再次连接设备失败)
  - [小程序绑定设备后,发现页依然可以搜索到设备](#小程序绑定设备后发现页依然可以搜索到设备)
  - [小程序删除设备后,发现页搜索不到设备](#小程序删除设备后发现页搜索不到设备)
  - [设备应该如何上报属性数据](#设备应该如何上报属性数据)
  - [设备如何获取小程序下发的控制数据](#设备如何获取小程序下发的控制数据)
  - [小程序控制设备,但是设备端触发的属性不符合预期](#小程序控制设备但是设备端触发的属性不符合预期)
  - [设备升级成功,小程序显示升级失败](#设备升级成功小程序显示升级失败)
  - [设备如何通知小程序设置MTU](#设备如何通知小程序设置mtu)
  - [如何开启动态注册功能](#如何开启动态注册功能)
- [蓝牙配网功能问题](#蓝牙配网功能问题)
  - [小程序发现页无法显示设备](#小程序发现页无法显示设备)
  - [小程序蓝牙配网失败](#小程序蓝牙配网失败)

## LLSync相关文档
[LLSync SDK下载](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded)：`LLSync SDK`源码。
[标准蓝牙功能接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E6%A0%87%E5%87%86%E8%93%9D%E7%89%99%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)：小程序仅通过`BLE`控制设备。
[蓝牙配网功能接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E8%BE%85%E5%8A%A9%E9%85%8D%E7%BD%91%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)：小程序通过`BLE`给设备配置网络。
[双路通信功能接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E5%8F%8C%E8%B7%AF%E9%80%9A%E4%BF%A1%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)：小程序优先通过`Wi-Fi`控制设备，当`Wi-Fi`不可用时，通过`BLE`控制设备。
[标准蓝牙功能示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-iot-ble-esp32)：标准蓝牙功能在`ESP32`开发板上的示例程序。
[标准蓝牙功能示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-iot-ble-nrf52832)：标准蓝牙功能在`NRF52832`开发板上的示例程序。
[蓝牙配网功能示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-llsync-config-net-esp32)：蓝牙配网功能在`ESP32`开发板上的示例程序。
[双路通信功能示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-llsync-dual-comm)：双路通信功能在`ESP32`开发板上的示例程序。
[蓝牙OTA功能开发引导](https://cloud.tencent.com/document/product/1081/50973)：介绍如何配置 `OTA` 功能、如何上传固件以及在小程序启动升级。
[蓝牙设备接入最佳实践](https://cloud.tencent.com/document/product/1081/50969)：介绍如何使用 `NRF52832`模组接入物联网开发平台。
[LLSync 蓝牙接入协议 ](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%E8%93%9D%E7%89%99%E8%AE%BE%E5%A4%87%E6%8E%A5%E5%85%A5%E5%8D%8F%E8%AE%AE.pdf)：`LLSync`蓝牙协议文档。
[LLSync 接入认证标准](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/%E8%85%BE%E8%AE%AF%E4%BA%91IoT%20BLE%E8%AE%BE%E5%A4%87%E6%8A%80%E6%9C%AF%E8%AE%A4%E8%AF%81%E6%B5%8B%E8%AF%95%E6%A0%87%E5%87%86%E5%8F%8A%E6%8A%A5%E5%91%8Av2.0.docx) ：`LLSync`认证标准定义。
[LLSync 认证模版文件](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/LLSync%E6%A0%87%E5%87%86%E8%93%9D%E7%89%99%E8%AE%A4%E8%AF%81%E6%A8%A1%E7%89%88) ：提供的示例模版文件，用户可以直接使用。也可以使用自定义模版文件。

## 标准蓝牙功能问题
### 编译缺少ble_qiot_template.c和ble_qiot_template.h
* 使用`Python`脚本转换物模型文件生成模版`C`文件。物模型文件可以在物联网开发平台控制台拷贝，会在同级目录下生成模版文件。
```
iot$ python3 interpret_dt_ble.py -c example.json 
reading json file start
reading json file end
generate header file start
generate header file end
generate source file start
generate source file end
```
### 小程序发现页搜索不到设备
* 请检查`Service UUID`是否正确，`Service UUID`应该是`0xFFE0`。
* 请检查`Manufacturer Specific Data`是否正确。
示例：`0xE7FE21CBD52F25B5E148525736584447343033`。
| Field | Size(Octets) | Notes |
| ------ | ------ | ------ |
| UUID | 2 | 固定0xFEE7 |
| Frame Control | 1 | 未配网固定为0x21 |
| MAC | 6 | CBD52F25B5E1 |
| Product ID | 10 |48525736584447343033 |
### 小程序绑定设备失败
* 请确认物联网开发平台创建设备时选择的通信方式为`BLE`。
* 请确认设备的`Product ID`、`Device Name`、`Device Secret`填写正确。
* 请检查`Service UUID`和`Characteristics UUID`是否正确。
* 请检查`Characteristics`的`Properties`是否正确。
![](https://qcloudimg.tencent-cloud.cn/raw/70a958005e56d2aa62c6e9b62fa04ab6.png)
### 设备绑定成功后,再次连接设备失败
* 请检查设备是否实现`ble_write_flash`，连接密钥是否正确存储。
* 请检查设备是否实现`ble_read_flash`，设备重启后密钥是否正确读取。
### 小程序绑定设备后,发现页依然可以搜索到设备
* 蓝牙连接成功后，应该关闭蓝牙广播。再次开启广播时，`LLSync`会刷新广播内容，`Frame Control `字段会变化为`0x22`。
### 小程序删除设备后,发现页搜索不到设备
* 蓝牙未连接时，小程序删除设备，设备端绑定信息无法清除。此时需要设备提供外部`UI`清除绑定信息，例如通过按键来清除绑定信息。
* 蓝牙连接时，小程序删除设备，首先确认设备端绑定信息是否清除；其次请检查设备端断开连接后是否有主动发起广播。
### 设备应该如何上报属性数据
* 设备端通过`ble_event_report_property`来获取用户数据进行上报。会依次调用模版`C`文件内的属性数据获取函数`ble_property_xxx_get()`函数获取单个属性数据。当该函数返回值非0时上报数据，返回值为0时不上报。
### 设备如何获取小程序下发的控制数据
* 模版`C`文件内的属性数据设置函数`ble_property_xxx_set()`函数会传递控制数据给用户。
### 小程序控制设备,但是设备端触发的属性不符合预期
* 一般是因为控制台的物模型和设备端物模型不一致。建议使用最新重新生成模版`C`文件，解绑后重新绑定设备。
### 设备升级成功,小程序显示升级失败
* 可能是设备最后一个响应报文没有发送完成设备就重启了，建议在`ble_ota_stop_cb`中延时几秒再重启。
* 请确保设备重启后上报的版本号和控制台填写的版本号一致。
### 设备如何通知小程序设置MTU
* 部分安卓手机蓝牙连接默认`MTU`是23，通信效率较低，`OTA`速度太慢。
* 打开宏定义`BLE_QIOT_REMOTE_SET_MTU`，小程序会根据设备`ble_get_user_data_mtu_size()`接口的返回值请求重新协商`MTU`。设备端收到蓝牙协议栈的`MTU`变化通知后，应该调用`ble_event_sync_mtu`同步`MTU`到小程序。
* 该功能在`IOS`上不生效。`IOS`上小程序无法设置`MTU`，默认`MTU`一般是`185`。
### 如何开启动态注册功能
* 打开`BLE_QIOT_DYNREG_ENABLE`定义时，使能动态注册功能。广播包中`Frame Control`字段为`0x28`。
* 使能动态注册功能时需要适配`ble_get_product_key`和`ble_set_psk`接口，分别用于获取产品密钥和存储动态注册获取的设备密钥。
* `LLSync`通过判断`ble_get_psk`接口获取到的设备密钥决定设备是否要进行动态注册广播动态。当该接口返回`FF`填充的设备密钥时不进行广播动态注册。
## 蓝牙配网功能问题
### 小程序发现页无法显示设备
* 请检查广播数据是否正确。相对于标准蓝牙功能，蓝牙配网功能的`Service UUID`变化为`0xFFF0`。
### 小程序蓝牙配网失败
* 请检查设备三元组信息是否填写正确。
* 如果是自行实现的配网协议，使用默认`MTU`通信时报文需要分片。
