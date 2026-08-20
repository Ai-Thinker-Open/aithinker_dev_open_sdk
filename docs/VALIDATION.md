[![中文](https://img.shields.io/badge/中文-文档-blue)](VALIDATION.zh.md)

# Validation record

## Scope

- Repository: `Ai-Thinker-Open/aithinker_dev_open_sdk`
- Source baseline inspected: `a457e237b2a70c0245ca6992be54699ac3f1d965`
- Review date: 2026-08-20
- Host: Windows with WSL2 Ubuntu available

## Checks performed

| Check | Result | Evidence |
| --- | --- | --- |
| Repository and branch inventory | Pass | Default branch `master`; baseline recorded above; the additional `mosh-obj` branch was left unchanged because its purpose is not proven to be temporary. |
| Runtime entry trace | Pass | `MAIN_ENTRY()` in `adt/adt_os/adapter_main/main/aiio_main_init.c` creates the application `aiio_main` task; `application/demo/src/main.c` supplies the reference entry. |
| Build routing trace | Pass | `build.sh` resolves application, ADT, vendor SDK, chip adapter, temporary tree, and output paths; BL602 routing was followed through `build/bl602_make/os_make.sh` and the application `genromap` wrapper. |
| Supported adapter inventory | Pass | Matching top-level build and SDK paths were confirmed for BL602, BL616, BL618, and LN882H. |
| Shell syntax | Pass | `bash -n` completed for `build.sh` and `build_ci.sh` from the recorded baseline. |
| Non-mutating command dispatch | Pass | `build.sh help` and `build.sh version` completed in Linux without invoking a firmware build. |
| Bilingual documentation | Pass | README, code-entry, architecture, and validation documents have English `.md` and Chinese `.zh.md` counterparts with reciprocal badges. |
| Documentation links | Pass | Local links introduced by these documents resolve against the proposed repository tree. |

## Build status

No complete firmware compilation is claimed by this review. The Git smart-HTTP endpoint repeatedly timed out or reset while the REST and source metadata endpoints remained available, so the 1.39 GB repository could not be checked out reliably during this run. The review therefore stopped at source-backed entry/build tracing and non-mutating shell checks instead of presenting an incomplete download as a successful build.

The minimum follow-up build is:

```bash
./build.sh bl602 demo defconfig
./build.sh bl602 demo cn debug
```

A build report should record the exact commit, clean/configure commands, compiler warnings and errors, and SHA-256 hashes for the resulting BIN/ELF/MAP artifacts.

## Not validated

- Full compilation of any chip/application matrix entry.
- Compilation of every application listed by `build_ci.sh`.
- Flashing, boot, serial output, RTOS scheduling behavior, or memory stability.
- GPIO, UART, I2C, SPI, PWM, ADC, storage, display, camera, sensor, or other peripheral behavior.
- Wi-Fi, BLE, provisioning, cloud, TLS, MQTT, OTA, or RF behavior.
- Hardware compatibility across module revisions.
- The behavior and data-loss boundary of `build.sh update` and `build.sh reset`.

## Release gate recommendation

Before publishing a release, run at least one clean reference build per supported chip adapter, then perform a hardware smoke test for every affected application category. A release should not be inferred from this documentation-only validation.
