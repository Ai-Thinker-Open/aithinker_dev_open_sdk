[![English](https://img.shields.io/badge/English-Docs-green)](ARCHITECTURE.md)

# 架构说明

## 分层模型

```text
应用层（`application/<名称>/src`）
        |
        v
统一 API 与组件（`adt/adt_os`）
        |
        v
芯片/RTOS 端口（`adt/adt_os/adapter_chip`）
        |
        v
厂商 SDK（`os/<芯片>_sdk_lts`）
        |
        v
固件产物（`out/<芯片>/<应用>`）
```

- **应用层**负责产品行为和示例，可移植代码应依赖 `aiio_*` 接口。
- **统一抽象层**提供共享接口与可复用核心组件。
- **芯片端口层**把统一接口映射到各受支持 SDK，并定义平台启动宏。
- **厂商层**提供 RTOS、驱动、无线协议栈、链接脚本和工具链。
- **构建层**把选定源码与配置组装进厂商工程，再把交付物复制到稳定输出路径。

## 构建数据流

```text
build.sh 参数
  + application/<应用>/src
  + application/<应用>/vendor/<芯片>/app_config
  + application/<应用>/vendor/<芯片>/sdk_config
  + adt/adt_os
  + os/<芯片>_sdk_lts
          |
          v
tmp/src/<芯片>/<应用> -> 厂商工程构建 -> out/<芯片>/<应用>
```

配置分两个阶段：`defconfig` 或 `menuconfig` 生成 `tmp/kconfig_h/<芯片>/<应用>/aiio_autoconf.h`；后续四参数构建如果找不到该头文件会拒绝继续。

## 顶层已适配芯片

源码树包含以下顶层构建适配：

| 芯片 | 构建适配 | 厂商 SDK 路径 |
| --- | --- | --- |
| BL602 | `build/bl602_make` | `os/bl602_sdk_lts` |
| BL616 | `build/bl616_make` | `os/bl616_sdk_lts` |
| BL618 | `build/bl618_make` | `os/bl618_sdk_lts` |
| LN882H | `build/ln882h_make` | `os/ln882h_sdk_lts` |

应用只有在同时存在对应 `vendor/<芯片>` 配置时，才是该芯片的候选构建对象。候选不等于已经成功构建。

## 修改边界

- 应用行为修改放在 `application/<名称>/src`。
- 应用级平台配置修改放在 `application/<名称>/vendor/<芯片>`。
- 新增或修正共享抽象时修改 `adt`。
- 只有源码组装、厂商构建调用或产物收集变化时才修改 `build/<芯片>_make`。
- `os/<芯片>_sdk_lts` 属于厂商派生代码，应隔离本地补丁并保留上游来源。
- `tmp` 和 `out` 属于构建状态，不是权威源码。

## 维护风险

- `build.sh update` 会同步受管理仓库，可能覆盖本地工作。
- `build.sh reset` 会删除受管理目录、执行硬重置，然后更新。
- 仓库内包含 SDK 和工具链，检出与构建需要较大存储空间。
- 目录或配置文件可能独立于源码、SDK 或工具链而过期。应验证具体矩阵项，不能只凭名称推断支持状态。
- 固件编译不能覆盖烧录、启动、射频、云端、外设、时序或长时间运行行为。
