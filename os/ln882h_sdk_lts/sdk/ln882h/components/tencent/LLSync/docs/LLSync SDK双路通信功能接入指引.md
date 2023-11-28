## LLSync SDK双路通信功能接入指引

双路通信功能仅应用于``Wi-Fi + BLE`的`Combo`芯片方案，当设备`Wi-Fi`断开后，可以通过`BLE`作为一种补充能力控制设备。

双路通信功能需要同时使用Wi-Fi通信能力和BLE通信能力，设备需要同时使用[C SDK](https://github.com/tencentyun/qcloud-iot-explorer-sdk-embedded-c)和[LLSync SDK](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded)，您可以下载LLSync双模通信[示例程序](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded-demo/tree/master/qcloud-llsync-dual-comm)参考。

### 一、控制台创建产品

1. 控制台创建产品时通信方式必须选择`Wi-Fi+BLE`。

![](https://main.qcloudimg.com/raw/1b64ead0b40225aa2fadc1775030f952.png)

### 二、移植SDK

1. 双路通信功能同时使用了`LLSync 配网功能`和`LLSync 标准蓝牙功能`，请分别参考[标准蓝牙功能详细接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E6%A0%87%E5%87%86%E8%93%9D%E7%89%99%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)和[辅助配网功能详细接入指引](https://github.com/tencentyun/qcloud-iot-explorer-BLE-sdk-embedded/blob/master/docs/LLSync%20SDK%E8%BE%85%E5%8A%A9%E9%85%8D%E7%BD%91%E5%8A%9F%E8%83%BD%E6%8E%A5%E5%85%A5%E6%8C%87%E5%BC%95.md)进行移植。配置文件内需要同时使能标准蓝牙功能和蓝牙配网功能。

   ```c
   #define BLE_QIOT_LLSYNC_STANDARD    1   // support llsync standard
   #define BLE_QIOT_LLSYNC_CONFIG_NET  1   // support llsync configure network
   #if (1 == BLE_QIOT_LLSYNC_STANDARD) && (1 == BLE_QIOT_LLSYNC_CONFIG_NET)
   #define BLE_QIOT_LLSYNC_DUAL_COM    1   // support llsync dual communication, do not use ble ota under the mode
   #endif
   ```

2. `C SDK`移植请参考[C SDK使用参考](https://cloud.tencent.com/document/product/1081/48363)。

3. `LLSync SDK`和`C SDK`有部分接口可以复用，这部分接口使用`C SDK`提供的即可，不编译`LLSync SDK`相关文件(`ble_qiot_utils_base64.c,ble_qiot_utils_md5.c,ble_qiot_utils_sha1.c`)。

### 三、使用说明

 1. 当设备检测到`Wi-Fi`断开的时候，小程序首页会显示设备离线，此时进入设备会提示用户进行`BLE`连接。用户连接`BLE`后，可以通过`BLE`控制设备。

    ![](https://main.qcloudimg.com/raw/d7f3e11c2a72b091eecc49e52779cccf.jpg)

 2. `LLSync SDK`和`C SDK`必须使用相同的数据模版进行代码转换。

 3. 使用双路通信功能时，设备暂不支持通过`BLE`进行升级。

