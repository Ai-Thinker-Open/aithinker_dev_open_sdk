[![English](https://img.shields.io/badge/English-Docs-green)](VALIDATION.md)

# 验证记录

## 范围

- 仓库：`Ai-Thinker-Open/aithinker_dev_open_sdk`
- 已审查源码基线：`a457e237b2a70c0245ca6992be54699ac3f1d965`
- 审查日期：2026-08-20
- 主机：Windows，可用 WSL2 Ubuntu

## 已执行检查

| 检查 | 结果 | 证据 |
| --- | --- | --- |
| 仓库与分支清单 | 通过 | 默认分支为 `master`，基线如上；额外的 `mosh-obj` 分支用途无法证明为临时分支，因此保持不变。 |
| 运行入口追踪 | 通过 | `adt/adt_os/adapter_main/main/aiio_main_init.c` 中的 `MAIN_ENTRY()` 创建应用 `aiio_main` 任务；`application/demo/src/main.c` 提供参考入口。 |
| 构建路由追踪 | 通过 | `build.sh` 解析应用、ADT、厂商 SDK、芯片适配、临时树和输出路径；BL602 链路已继续追踪到 `build/bl602_make/os_make.sh` 与应用 `genromap` 包装脚本。 |
| 支持适配清单 | 通过 | 已确认 BL602、BL616、BL618 和 LN882H 同时存在顶层构建路径与 SDK 路径。 |
| Shell 语法 | 通过 | 对记录基线的 `build.sh` 和 `build_ci.sh` 执行 `bash -n` 均完成。 |
| 非修改性命令分发 | 通过 | 在 Linux 中执行 `build.sh help` 和 `build.sh version` 均完成，未触发固件构建。 |
| 双语文档 | 通过 | README、代码入口、架构和验证文档均有英文 `.md` 与中文 `.zh.md` 配对，并带双向徽章。 |
| 文档链接 | 通过 | 本次文档新增的本地链接在拟提交仓库树中均可解析。 |

## 构建状态

本次审查不声称完成了任何完整固件编译。Git 智能 HTTP 入口持续超时或被重置，而 REST 与源码元数据入口正常，因此本次无法可靠检出 1.39 GB 仓库。审查停留在有源码依据的入口/构建链路追踪和非修改性 Shell 检查，没有把不完整下载描述成构建成功。

建议后续最小构建为：

```bash
./build.sh bl602 demo defconfig
./build.sh bl602 demo cn debug
```

构建报告应记录准确提交、清理/配置命令、编译器警告与错误，以及生成 BIN/ELF/MAP 的 SHA-256。

## 未验证范围

- 任意芯片/应用矩阵项的完整编译。
- `build_ci.sh` 所列全部应用的编译。
- 烧录、启动、串口输出、RTOS 调度行为或内存稳定性。
- GPIO、UART、I2C、SPI、PWM、ADC、存储、显示、摄像头、传感器等外设行为。
- Wi-Fi、BLE、配网、云端、TLS、MQTT、OTA 或射频行为。
- 不同模组硬件版本的兼容性。
- `build.sh update` 与 `build.sh reset` 的行为及数据丢失边界。

## 发布门禁建议

发布版本前，应至少对每个受支持芯片适配完成一次干净参考构建，并对每类受影响应用完成硬件冒烟测试。不能依据本次仅文档级验证推断发布就绪。
