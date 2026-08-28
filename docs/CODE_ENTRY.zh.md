[![English](https://img.shields.io/badge/English-Docs-green)](CODE_ENTRY.md)

# 代码入口

## 运行入口

统一运行桥接位于 `adt/adt_os/adapter_main/main/aiio_main_init.c`。

1. `MAIN_ENTRY()` 通过当前芯片端口的 `__MAIN_ENTRY()` 展开为对应启动符号。
2. 桥接层依次调用 `aiio_wsrc_call()`、`aiio_chip_init()`、`aiio_log_init()` 和 `aiio_manufacturer_info()`。
3. 它通过 `aiio_os_thread_create()` 创建名为 `aiio_main` 的 RTOS 任务。
4. 应用在 `application/<应用>/src/main.c` 中实现 `aiio_main()`。
5. 桥接层调用 `aiio_os_thread_start_scheduler()` 启动调度器。

具体参考实现为 `application/demo/src/main.c`。它包含统一头文件 `aiio_adapter_include.h`，输出日志，然后停留在周期任务循环中。

## 构建入口

`build.sh` 是公开构建入口。四参数构建会解析：

- 应用源码：`application/<应用>/src`；
- 应用/芯片配置：`application/<应用>/vendor/<芯片>`；
- 抽象层：`adt/adt_os` 与 `adt/adt_tools`；
- 厂商 SDK：`os/<芯片>_sdk_lts`；
- 芯片构建适配：`build/<芯片>_make`；
- 生成的组装目录：`tmp/src/<芯片>/<应用>`；
- 输出目录：`out/<芯片>/<应用>`。

以 BL602 为例，`build/bl602_make/os_make.sh` 会把选定应用、生成配置和抽象层源码复制到厂商 `customer_app/dev_project`，执行其 `genromap` 包装脚本，再通过 `bin_out.sh` 收集产物。其他受支持芯片通过各自的 `<芯片>_make` 目录遵循同一顶层约定。

## 新增应用

可参考 `application/demo` 这类小型应用的结构：

```text
application/my_app/
├── src/
│   └── main.c              # 实现 aiio_main
└── vendor/
    └── <芯片>/
        ├── app_config/     # Kconfig/.config/生成配置输入
        └── sdk_config/     # 厂商工程和构建包装输入
```

可复用逻辑放在 `src`，芯片相关值放在 `vendor/<芯片>`；可移植应用代码应调用统一 `aiio_*` 接口，不应直接调用厂商 SDK。

## 证据边界

上述入口来自仓库源码追踪，但不能据此证明所有应用/芯片组合都能构建，也不能证明固件能够在硬件上启动；详见[验证记录](VALIDATION.zh.md)。
