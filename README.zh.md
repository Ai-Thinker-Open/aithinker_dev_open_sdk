[![English](https://img.shields.io/badge/English-README-green)](README.md)

# 安信可统一开发 SDK

## 项目介绍

本仓库为多个安信可模组平台提供统一的应用开发层：应用代码位于 `application/`，`adt/` 屏蔽芯片和 RTOS 差异，最终固件生成由 `os/` 中选定的厂商 SDK 完成。

当前顶层构建集成包含 BL602、BL616、BL618 和 LN882H 的适配。个别应用中即使存在其他厂商命名目录，也不能仅凭目录存在就认为顶层构建已支持或验证该平台。

## 仓库结构

| 路径 | 用途 |
| --- | --- |
| `application/<名称>/src/` | 可移植应用代码，应用通常在这里定义 `aiio_main`。 |
| `application/<名称>/vendor/<芯片>/` | 应用级 Kconfig 和厂商构建配置。 |
| `adt/adt_os/` | 统一 API、核心组件、平台端口，以及固件到应用的启动桥接。 |
| `build/<芯片>_make/` | 芯片相关的源码组装、厂商构建调用、清理和产物收集。 |
| `os/<芯片>_sdk_lts/` | 厂商 SDK 与工具链集成。 |
| `tmp/` | 构建时生成的组装目录，不应作为应用源码目录。 |
| `out/<芯片>/<应用>/` | 构建生成的固件、ELF 和 MAP 文件。 |
| `tools/` | 环境、配置、格式化、文档和维护脚本。 |

真实入口和构建链路见[代码入口](docs/CODE_ENTRY.zh.md)与[架构说明](docs/ARCHITECTURE.zh.md)。

## 环境要求

- Linux，或安装了 Linux 发行版的 WSL2。`build.sh` 会主动拒绝非 Linux 环境。
- Bash 与 Git。
- 干净环境需要安装 `./build.sh environment` 所列的软件包。执行前请先审阅该命令：它会调用 `sudo apt-get` 和 `pip`，会改变主机环境且需要网络。
- 为随仓库提供的厂商 SDK、解压后的工具链、`tmp/` 和 `out/` 预留足够磁盘空间。

## 快速开始

在仓库根目录执行：

```bash
./build.sh help
./build.sh version
```

为 BL602 配置并构建最小参考应用：

```bash
./build.sh bl602 demo defconfig
./build.sh bl602 demo cn debug
```

也可以交互配置：

```bash
./build.sh bl602 demo menuconfig
```

构建命令格式为：

```text
./build.sh <芯片> <应用> <国家> <级别>
```

- `<芯片>`：必须同时存在 `os/<芯片>_sdk_lts/` 和 `build/<芯片>_make/`；当前为 `bl602`、`bl616`、`bl618` 或 `ln882h`。
- `<应用>`：`application/` 下的目录名。
- `<国家>`：`cn` 或 `en`。
- `<级别>`：`debug`、`debug_tmp`、`debug_ram` 或 `release`。

构建成功后，产物位于 `out/<芯片>/<应用>/bin`、`elf` 和 `map`。

## 应用入口

厂商启动宏最终展开 `adt/adt_os/adapter_main/main/aiio_main_init.c` 中的 `MAIN_ENTRY()`。该桥接层初始化芯片、日志和厂商信息，创建 `aiio_main` 任务并启动调度器。应用在 `application/<名称>/src/main.c` 中提供这个任务。

参考入口链路：

```text
厂商启动
  -> MAIN_ENTRY()
  -> 芯片/日志/厂商信息初始化
  -> 创建 aiio_main 任务
  -> application/demo/src/main.c:aiio_main()
```

## 常用操作

```bash
# 清理全部输出
./build.sh clean

# 清理单个应用
./build.sh bl602 demo clean

# 重新生成接口文档
./build.sh docs
```

以下维护命令需要谨慎使用：

- `./build.sh update` 会同步受管理源码区域，可能覆盖本地改动。
- `./build.sh reset` 会删除受管理目录、执行 `git reset --hard`，然后更新代码。请先提交或备份工作。
- 构建配置和工具链准备过程可能解压归档并修改生成目录或厂商工作树。

## 验证状态

本次记录覆盖仓库结构、启动链路、构建路由、Shell 语法、帮助/版本分发、双语文档配对和本地文档链接，详见[验证记录](docs/VALIDATION.zh.md)。

本次文档审查没有完成完整固件构建、烧录、启动、外设、无线功能或硬件回归。不能把文档检查理解为真机验证。

## 常见问题

- **出现 `error: unkonw os`**：请在 Linux 或 WSL2 中构建，不要直接在 PowerShell 或命令提示符中运行。
- **提示芯片或应用未适配**：确认 `os/<芯片>_sdk_lts`、`build/<芯片>_make`、`application/<名称>` 和 `application/<名称>/vendor/<芯片>` 同时存在。
- **提示缺少配置**：执行四参数构建前，先运行 `defconfig` 或 `menuconfig`。
- **工具链解压或依赖安装失败**：检查归档完整性、磁盘空间、权限和前述依赖。
- **update/reset 后出现意外变化**：从自己的提交或备份恢复；这些命令本身具有破坏性。

## 参与贡献

通用应用逻辑应放在 `application/<名称>/src`，平台配置放在 `application/<名称>/vendor/<芯片>`，芯片适配代码放在 `adt`。提交变更前：

1. 从干净配置至少构建一个受影响的芯片/应用组合。
2. 记录准确命令、警告、输出产物和未测试的硬件范围。
3. 行为变化时同步更新英文 `.md` 与中文 `.zh.md` 文档。
4. 除非仓库策略明确要求，不要提交生成的 `tmp/` 或 `out/` 内容。

## 许可证

本仓库使用 Apache License 2.0，详见 [`license.txt`](license.txt)。
