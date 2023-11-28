
<table>
    <tr>
        <td>日期</td>
        <td>当前SDK版本</td>
        <td>上游版本</td>
        <td>摘要</td>
    </tr>   
    <tr>
        <td>
            2023-03-25 22:30:00
        </td>
        <td>
            v1.8
        </td>
        <td>
            97b642f3
        </td>
        <td>
            1. WIFI Lib 版本更新为 v1.1.2 <br />
            2. BLE Lib 版本更新为 v1.0.5 <br />
            3. SDK 版本更新为 v1.8 <br />
             <br /><b>SDK changelog:</b> <br />
		    1. BLE MAC中断优先级调整为最高 <br />
		    2. 增加 linux 代码兼容适配 sys/time.h <br />
		    3. 更新 http 组件，增加扩展接口，保持向前兼容 <br />
		    4. 添加smartconfig 示例工程 <br />
		    5. 优化高阶速率(OFDM)接收 <br />
		    6. 修复AT+CIPSTA?查询IP返回ERROR，修复CIPSTAMAC命令使用vsscanf(gcc@nona_lib)的bug <br />
		    7. lwip优化ARP，DHCP，CHKSUM算法 <br />
		    8. 修复bootheader的一些标志位设置不成功的bug(比如CRP_FLAG被配配置为1，但生成image没有置位) <br />
		    9. 完善got ip接口，方便应用层在wifi连接成功，等待IP有效后执行网络应用 <br />
		    10. BLE lib log等级更改为INFO <br />
		    11. rebuild 2ndboot，添加flash补丁 <br />
		    12. 删除 ble_mcu_scan示例工程，在ble_mcu_data_trans示例工程中增加scan示例 <br />
		    13. 添加BLE APP封装层，AT命令，简化BLE应用，用新的ble API更新ble，combo相关示例工程，添加BLE API接口文档。(移除老版本ble api) <br />
            14. 更新<b>wifilib</b> <br />
				14.1. AP模式优化 <br />
				14.2. ping测试/高低温优化 <br />
				14.3  恢复信令测试模式速率集
            15. 更新<b>blelib</b> <br />
				15.1. 修复扩展广播相关问题 <br />
				15.2. 增加BLE协议栈调度机会 <br />
				15.3. ble assert分类 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-11-25 15:00:00
        </td>
        <td>
            v1.7
        </td>
        <td>
            73bf2b87
        </td>
        <td>
            1. 修复链接脚本问题（配置默认小端格式，修复 app 分区可能越界的问题） <br />
            2. 增加亚马逊 aws 连接组件和 demo 工程 <br />
            3. 修复上次解决复位后 PHY 异常问题引入的单 ble 模式不工作的问题，在 PHY 的初始化前后，开关 RF_TRX <br />
            4. 修复低功耗起来后频点偏差的问题 <br />
            5. AT 组件增加 dump 参数的调试接口 <br />
            6. 增加 AP 模式下扫描的功能 <br />
            7. 修复硬件过滤器多路配置的同步问题 <br />
            8. WiFi 组件增加 MAC_STA_STARTUP、MAC_AP_STARTUP 状态 <br />
            9. WiFi TX 功率补偿接口支持 b 模式 gn 模式单独补偿 <br />
            10. 改善低温（-40~-10）下 EVM 指标 <br />
            11. 增加腾讯云 qcloud 组件和 demo <br />
            12. 修复 WiFi disconnect 的时候会锁定信道的问题 <br />
            13. 修复 WiFi PA 误开关的问题 <br />
            14. 更新低功耗策略，WiFi disconnect 之后默认不允许 CPU 休眠 <br />
            15. 移除 cJSON 组件，版本太旧，且跟新版本 API 不兼容，顾移除。如果用户业务或者组件需要使用 cJSON，请添加到该业务代码或者组件里自行维护 <br />
            16. 修复速率集相关代码重构引入的 STA 模式连接 11b only 路由器会失败的问题 <br />
            17. 修复 STA 关联上路由器后 scan 操作可能的死机问题 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-10-12 10:00:00
        </td>
        <td>
            v1.6
        </td>
        <td>
            c5088226
        </td>
        <td>
            1. WIFI Lib 版本更新为 v1.0.9 <br />
            2. SDK 版本更新为 v1.6 <br />
             <br /><b>SDK changelog:</b> <br />
            1. 优化light_sleep模式下功耗，修复该模式下OSTick补偿方式(24小时误差小于15S） <br />
            2. 修复PWM Demo示例工程中不能配置引脚功能的问题 <br />
            3. 增强AT命令(wifi scan)的健壮性 <br />
            4. 更新<b>wifilib</b> <br />
				4.1. 优化内存，增加用户可用内存(os_heap)33KB+ <br />
				4.2. PLL校准只在温度变化跨度较大情况下进行，降低高温区由于温区边界宽度较小反复触发PLL校准的问题. <br />
				4.3. 修复 WiFi scan 在扫描指定信道的时候可能使用内存脏数据的问题 <br />
				4.4. 增加指定信道扫描功能（当 STA 处于连接状态，不允许指定信道扫描） <br />
				4.5. RFDC校准增强健壮性 <br />
				4.6. 优化配置，修复在某特定场景下的CCA判断错误的问题，增强抗干扰特性 <br />
				4.7. SoftAP启动时增加随机ms延时 <br />
				4.8. 优化PHY的配置时间，减小light_sleep模式下tbtt提前唤醒时间 <br />
				4.9. 开启monitor模式时可以自动信道切换进行scan操作，保持smartcfg配网(usr_sniffer)模式下信道切换交给用户操作 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-09-07 10:00:00
        </td>
        <td>
            v1.5
        </td>
        <td>
            1d029698
        </td>
        <td>
            1. WIFI 版本更新为 v1.0.8 <br />
            2. SDK 版本更新为 v1.5 <br />
             <br /><b>SDK changelog:</b> <br />
            1. 增加 GPIO 测试 Demo <br />
            2. 修改 ramcode_dl 的复位接口<br />
            3. 修改 pll 周期校准启动时机     <br />
            4. 添加读取 Flash UID 接口 <br />
            5. 修复 ap 模式下发送 send_null_frame_to_AP 函数的问题 <br />
            6. 优化 lwip 配置 <br />
            7. 增加设置 erp 类型和 rts threshold 的接口<br />
            8. 修复 dhcpd 任务被多次创建同时运行的问题 <br />
            9. AP 模式默认使用高功率表  <br />
            10. 重新设计速率集配置，将 bg 速率集移动到 wifi_port.c/h 以供用户自行配置<br />
            11. 修复 sniffer mem pool 被反复初始化的问题 <br />
            13. 增加软件 duration 配置失能接口，默认使能软件配置功能，调用该接口后失能
        </td>
    </tr>
<tr>
        <td>
            2022-07-15 10:50:00
        </td>
        <td>
            v1.4
        </td>
        <td>
            288ee4c6
        </td>
        <td>
            1. WIFI 版本更新为 v1.0.7 <br />
            2. BLE 版本更新为 v1.0.4 <br />
            3. SDK 版本更新为 v1.4 <br />
             <br /><b>SDK changelog:</b> <br />
            1. 新增 RAMCODE 命令，增加获取 flash 信息和 Flash 整片测试命令 <br />
            2. at 指令增加 nvic reset 功能<br />
            3. 优化 wifi 网络性能<br />
            4. 修复 connect 的时候 scan 到目标网络 但是没有发起 join 的问题     <br />
            5. img 校准使用的 RAM 区域 4 字节对齐 <br />
            6. wifi rf 校准优化 <br />
            7. 增加 BLE 硬件复位接口 <br />
            8. 增加 BLE 边带测试命令 <br />
            9. 修复在重关联的时候拿到 ip 后偶现 dhcp release 的问题 <br />
            10. wifi 校准函数合并为一个 wifi_rf_calibration
    原来为 wifi_rf_preprocess 和 wifi_rf_image_cal 
            <br />
        </td>
    </tr>
<tr>
        <td>
            2022-06-24 10:50:00
        </td>
        <td>
            no version
        </td>
        <td>
            33ed5a55
        </td>
        <td>
            1. 优化 mbedtls 内存. MBEDTLS_SSL_OUT_CONTENT_LEN 设置为4K <br />
            2. 优化 wifi 传输速率 <br />
            3. BLE 示例工程 sct 更新 <br />
            4. add battery release project <br />
            5. add mouse release project <br />
            6. 添加接口 wifi_softap_set_max_supp_sta_num,wifi_softap_get_max_supp_sta_num 可以设置/获取最大支持的sta数目 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-06-01 20:55:00
        </td>
        <td>
            no version
        </td>
        <td>
            c2382740
        </td>
        <td>
            1. 增加 single tone 调试 AT+PVTCMD=s_tone_mode BLE AT+PVTCMD=s_tone_mode WIFI <br />
            2. 修复在未设置 got ip function callback 的时候 WiFi 无法进入睡眠的问题 <br />
            3. 增加 AT OTA 命令程序 URL 缓存大小；增加 AT CMD 缓存大小； <br />
            4. 增加 https client path 和 CONTENT 缓冲区大小； <br />
            5. mbedtls 移除对 MBEDTLS_SSL_IN_CONTENT_LEN 和 MBEDTLS_SSL_OUT_CONTENT_LEN 的配置，继承 MBEDTLS_SSL_MAX_CONTENT_LEN。 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-05-25 16:45:00
        </td>
        <td>
            no version
        </td>
        <td>
            75faa75d
        </td>
        <td>
            1. 修复在未设置 got ip function callback 的时候 WiFi 无法进入睡眠的问题 <br />
            2. 射频测试的功能更新，增加 s_tone_mode 命令 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-05-24 18:04:00
        </td>
        <td>
            no version
        </td>
        <td>
            75faa75d
        </td>
        <td>
            1. 修复 log 输出参数不匹配问题及其他编译警告 <br />
            2. 不定参使用 ##__VA_ARGS__ 来避免无参数错误 <br />
            3. GNU 下，使用 __attribute__((format(printf, 2, 3))) 和 -Wformat=2 增强对 printf 类型的不定参函数进行 format 和 参数匹配检查 <br />
            4. 更新 boot 程序，并添加 AT 指令 AT+BLE_START 以允许跳转到 OTA 分区执行代码 <br />
            5. 增加 combo ble hci 认证固件 combo_ble_hci_exec_addr_0x00133100_log_B9_921600_cmd_A2A3_115200_sha1_7cd12ad9.bin <br />
            6. start_build.py 脚本更新合并固件的功能 <br />
            python3 start_build.py merge_fw --fw A.bin --offset 0x1000 --fw B.bin --offset 0x2000 --out outfile.bin <br />
            7. 移除 ignore trailing bytes 相关的 dump 日志 <br />
            8. 移除 wifi_sta_set_dtim_period 接口；增加 wifi_sta_set_listen_interval 接口；增加 wifi_softap_set_dtim_period 接口 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-04-25 16:50:00
        </td>
        <td>
            no version
        </td>
        <td>
            acf49953
        </td>
        <td>
            1. 涂鸦工程增加 ATE 业务逻辑 <br />
            2. 更新 ble MTU 配置为 512；更新主栈为 1.5K；更新 lwip 任务栈为 3K；更新 RETENTION 区为 1K。 <br /> 用于增加可用内存 <br />
            3. AT+CWJAP 指令优化 <br />
            4. 修复 beacon 解析函数 ssid 匹配问题 <br />
            5. 增加清除 scan 结果的 API wifi_manager_cleanup_scan_results() <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-04-15 12:09:00
        </td>
        <td>
            no version
        </td>
        <td>
            d2089b6a
        </td>
        <td>
            1. 增加 JD joylink 组件和工程 <br />
            2. 修复 NVDS 频偏补偿和功率补偿 <br />
            3. 修复 AT 解析器对有符号整数解析的问题 <br />
            4. 增加 ATE AT 指令 <br />
            5. http client 增加 http_post_request 接口 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-04-07 21:21:00
        </td>
        <td>
            no version
        </td>
        <td>
            b75ed3f9
        </td>
        <td>
            1. 更新 AT 指令，增加 AT+RST 的实现，增加 WiFi AT 指令 <br />
            2. 同步 powersave 组件，增加 frozen 模式 <br />
            3. wifi lib 修复 vtrim adc 匹配问题 <br />
            4. 更新 autorate，修复逻辑问题 <br />
            5. 优化发射功率调整 <br />
            6. 优化 get_tx_pow 接口 <br />
            7. 修复 pr_to_ur 11b long prem 处理错误 <br />
            8. 优化 WiFi ble 共存 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-03-02 11:19:00
        </td>
        <td>
            v1.3_rc2
        </td>
        <td>
            7648678c
        </td>
        <td>
            1. wifi 更新版本为 v1.0.7_rc2 <br />
            2. sdk 版本更新为 v1.3_rc2 <br />
            <br /><b>SDK changelog:</b> <br />
            1. 修复 ate 错误 <br />
            2. 修复 WiFi 高速率丢包问题 <br />
            3. 修复 rx 饱和门限 <br />
            4. 修复 xtal_cap tx_power 指令问题 <br />
            5. 修复发射功率补偿逻辑错误 <br />
            6. 增加 autorate 功能，增强抗干扰能力 <br />
            7. 更新 WiFi RF PHY 寄存器参数，优化 WiFi 性能，杂散测试 <br />
            8. 外设时钟初始化方式修改为按需初始化 <br />
            9. 修复 reboot 组件 reboot 原因获取问题 <br />
            10. 修复 adc bug <br />
            11. 移除 power_up_phy 和 power_down_phy <br />
            12. 更新 lwip 为 2.1.3 版本 <br />
            13. 默认使用 ROM function（AES, CRC, SHA1, MD5） <br />
            14. 修复 gcc 下 浮点 输入输出问题 <br />
            15. WiFi 优化 monitor 模式 <br />
            16. 修复 wifi_sta_get_powersave 接口错误 <br />
            17. 完善 AT 指令 <br />
            18. 增加 ota demo <br />
            19. 统一串口：日志串口为 B9:TX B8:RX; 命令串口 A2:TX A3:RX <br />
            20. 修复 hal_timer 驱动 hal_tim_get_current_cnt_value 接口错误 <br />
        </td>
    </tr>
    <tr>
        <td>
            2022-01-14 15:54:00
        </td>
        <td>
            v1.2
        </td>
        <td>
            76f51138
        </td>
        <td>
            <b>SDK changelog:</b> <br />
            1. ble 禁用 ke_msg_send lock，该 lock 在使用不当的时候会导致死锁 <br />
            2. ble 更新 ble 硬件参数，提升 phy 的稳定性 <br />
            3. ble cmp event 事件增加 active index 参数，为了方便在应用层 cb 里知道是哪个 active 的事件 <br />
            4. nvds 增加 ate 相关数据存储 <br />
            5. freertos 配置 进入休眠的 idle 事件为 3 个 tick，原来是 8 个 tick <br />
            6. pm 组件，更新 <br />
            7. reboot 使用更安全的接口 <br />
            8. WiFi 增加 WiFi monitor 模式，并增加 monitor callback <br />
            9. wifi 增加 wlib_awo_set_r_vtrim 接口 <br />
            10. WiFi 完善低功耗接口 <br />
            11. 优化 adc 精度 <br />
        </td>
    </tr>
    <tr>
        <td>
            2021-12-30 14:30:00
        </td>
        <td>
            v1.1
        </td>
        <td>
            223160a1
        </td>
        <td>
            <b>SDK changelog:</b><br />
            1. 优化 boot 启动时间长的问题，增加 boot 支持降级升级的功能<br />
            2. 完善 newlib 接口<br />
            3. 调整各个工程的 flash 分区结构，降低 boot 分区大小为 24KB<br />
            4. 修复 ap 模式无法使用 iperf 的问题<br />
            5. 增加低功耗组件，完善低功耗功能<br />
            6. 修复 awo 寄存器错误<br />
            7. 更新 adc 校准参数<br />
            8. 增加 SHA1,AES,CRC ROM 代码使用，默认不适用 ROM 代码，可选择性启用<br />
            9. 更新 ota 组件，移除对 root 的耦合<br />
            10. 移除 reboot_trace 组件对 wdt 的耦合<br />
            11. 修复 WiFi 断开连接的时候依旧打印 ip 地址的问题<br />
            <b>wifi changelog:</b><br />
            1. 优化内存，降低 WiFi 1600 内存池数量，降低 sniffer 内存使用数量<br />
            2. 优化 RF 性能，提升 RF 在高低温情况下的稳定性和性能<br />
            3. 优化 RF 校准时间，降低 temp 和 img 校准时间，加快启动速度<br />
            4. 优化 WiFi 发射滤波器和发射动态功率数据，提升 WiFi 发射性能<br />
            5. 优化 WiFi 低功耗机制，支持 40M 到 32K 时钟切换<br />
            8. 修复路由器一直发 moredata 导致 MCU 无法休眠的问题<br />
            9. 修复 WiFi SCAN 过程中记录路由器信息的时候会遗漏进而导致内存泄漏的问题<br />
            10. 修复不支持 WEP40 和 WEP104 的问题<br />
            11. 修复处理 CFG_MSG_INFO 消息的时候，在处理各种状态 cb 的时候可能出现的在逻辑死锁问题<br />
            13. 修复 RF/PHY 寄存器、LUT 访问不可靠的问题，防止编译器优化<br />
            14. 修复 ble rssi 不准确问题，涉及 PHYRXLNAGAIN7TABLE_VALUE 寄存器<br />
            16. 增加 LUT_HW_ERROR 错误类型<br />
            17. 增加 WID_ALLOW_CPU_SLEEP WID，用于低功耗控制<br />
            18. 增加 11n 使能接口 wifi_set_11n_enable<br />
            19. 移除 WID_DHCP_STATUS WID，弃用<br />
            <b>ble changelog:</b><br />
            1. 移除 common 目录<br />
            2. 使用指针替代结构体里的零数组<br />
            3. 增加 ble_arch 目录，用于平台相关的处理<br />
            6. 更新 mac 地址配置接口 ln_ble_mac_set ln_ble_mac_get<br />
            7. 修复 rssi 不准确、不稳定的问题<br />
            8. 禁用内部 NVDS 功能，禁用 AOA 和 AOD 功能
        </td>
    </tr>
    <tr>
        <td>
            2021-10-24 18:19:00
        </td>
        <td>
            v1.0
        </td>
        <td>
            3037bbcc
        </td>
        <td>
            2.1. 优化温度补偿校准.<br/>
            2.2. 修复隐藏连接隐藏AP的问题.<br/>
            2.3. 优化改善WiFi BLE共存稳定性.<br/>
            2.4. 修复暗室测试过程中非标速率集导致的死循环问题.<br/>
            2.5. WiFi发射功率依据eFuse标定值.<br/>
            2.6. 更新WiFi tx power VS time. <br/>
            2.7. 优化 11b EVM.<br/>
            2.8. 增加在不同速率集下使用不同的发射功率的功能.<br/>
            2.9. 优化 ram/codesize.<br/>
            3. 实现WiFi BLE共存，优化改善共存稳定性。<br/>
            4. wifi/ble stack (gcc/armcc)lib 使用O1编译.<br/>
            5. 更新peripher hal driver.<br/>
            6. 更新peripher driver test.<br/>
            7. 修复部分编译警告.<br/>
            8. 新增keil MDK(win平台)，gcc(win/linux平台)编译自动化检查功能.<br/>
            9. 新增ble_mcu_data_trans, combo_mcu_basic_example示例工程，支持keil/gcc编译.<br/>
            10. 新增Tuya OS的GCC编译配置入口（已作为submodule适配到Tuya OS SDK）<br/>
            11. 更新部分示例工程的分区表配置.<br/>
            12. 部分代码使用Apache License V2.0<br/>
            13. 更新CMake list.
        </td>
    </tr>
    <tr>
        <td>
            2021-10-16 17:58:00
        </td>
        <td>
            v0.2
        </td>
        <td>
            eddb49be
        </td>
        <td>
            1. 正式发布ln882h wifi mac stack，完全解除与platform（soc、os、net...）的耦合，全面深度重构和优化，精简api。<br />
            2. 全新的cmake构建体系 。<br />
            3. 更新keil MDK(win平台)，gcc(win/linux平台)开发环境指导文档。<br />
            4. 增加LN882H分区地址辅助计算工具。<br />
            5. 全新的编译预处理、后处理脚本工具体系。<br />
            6. 新版AT命令组件。<br />
            7. 修复armcc5，gcc编译警告若干。<br />
            8. 更新peripher hal driver.<br />
            9. 更新Jflash烧录配置参数。
        </td>
    </tr>
    <tr>
        <td>
            2021-09-30 16:10:00
        </td>
        <td>
            v0.1
        </td>
        <td>
            38a947c8
        </td>
        <td>
            LN882H SDK 初始版本
        </td>
    </tr>
</table>
