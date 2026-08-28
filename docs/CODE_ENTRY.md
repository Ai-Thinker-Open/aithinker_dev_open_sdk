[![中文](https://img.shields.io/badge/中文-文档-blue)](CODE_ENTRY.zh.md)

# Code entry

## Runtime entry

The common runtime bridge is `adt/adt_os/adapter_main/main/aiio_main_init.c`.

1. `MAIN_ENTRY()` expands to the startup symbol selected by the active chip port through `__MAIN_ENTRY()`.
2. The bridge calls `aiio_wsrc_call()`, `aiio_chip_init()`, `aiio_log_init()`, and `aiio_manufacturer_info()`.
3. It creates an RTOS task named `aiio_main` through `aiio_os_thread_create()`.
4. The application supplies `aiio_main()` in `application/<application>/src/main.c`.
5. The bridge starts the scheduler with `aiio_os_thread_start_scheduler()`.

The concrete reference implementation is `application/demo/src/main.c`. It includes the public umbrella header `aiio_adapter_include.h`, writes log messages, and then remains in a periodic task loop.

## Build entry

`build.sh` is the public build entry. For a four-argument build it resolves:

- application source: `application/<application>/src`;
- application/chip configuration: `application/<application>/vendor/<chip>`;
- abstraction layer: `adt/adt_os` and `adt/adt_tools`;
- vendor SDK: `os/<chip>_sdk_lts`;
- chip build adapter: `build/<chip>_make`;
- generated assembly tree: `tmp/src/<chip>/<application>`;
- output: `out/<chip>/<application>`.

For BL602, `build/bl602_make/os_make.sh` copies the selected application, generated configuration, and abstraction sources into the vendor `customer_app/dev_project`, runs its `genromap` wrapper, and collects artifacts through `bin_out.sh`. Other supported chips follow the same top-level contract through their own `<chip>_make` directory.

## Adding an application

Use an existing small application such as `application/demo` as the structural reference:

```text
application/my_app/
├── src/
│   └── main.c              # defines aiio_main
└── vendor/
    └── <chip>/
        ├── app_config/     # Kconfig/.config/generated config inputs
        └── sdk_config/     # vendor project and build wrapper inputs
```

Keep reusable behavior in `src`, keep chip-specific values below `vendor/<chip>`, and call the common `aiio_*` interfaces instead of directly calling a vendor SDK from portable application code.

## Evidence boundary

These entry points were traced from repository source. The trace does not prove that every application/chip combination builds or that the resulting firmware boots on hardware; see [Validation](VALIDATION.md).
