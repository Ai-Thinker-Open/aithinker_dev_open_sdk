[![中文](https://img.shields.io/badge/中文-文档-blue)](ARCHITECTURE.zh.md)

# Architecture

## Layer model

```text
Application (`application/<name>/src`)
        |
        v
Common API and components (`adt/adt_os`)
        |
        v
Chip/RTOS ports (`adt/adt_os/adapter_chip`)
        |
        v
Vendor SDK (`os/<chip>_sdk_lts`)
        |
        v
Firmware artifacts (`out/<chip>/<application>`)
```

- **Application layer** owns product behavior and examples. Portable code should depend on `aiio_*` interfaces.
- **Common abstraction layer** exposes shared interfaces and reusable core components.
- **Chip port layer** maps those interfaces to each supported SDK and defines the platform startup macro.
- **Vendor layer** supplies RTOS, drivers, wireless stacks, link scripts, and toolchains.
- **Build layer** assembles the selected sources and configuration into the vendor project, then copies deliverables to a stable output path.

## Build data flow

```text
build.sh arguments
  + application/<app>/src
  + application/<app>/vendor/<chip>/app_config
  + application/<app>/vendor/<chip>/sdk_config
  + adt/adt_os
  + os/<chip>_sdk_lts
          |
          v
tmp/src/<chip>/<app>  -> vendor project build -> out/<chip>/<app>
```

Configuration is a two-stage operation. `defconfig` or `menuconfig` prepares `tmp/kconfig_h/<chip>/<application>/aiio_autoconf.h`; the subsequent four-argument build refuses to continue if that generated header is absent.

## Supported top-level adapters

The source tree contains these top-level build adapters:

| Chip | Build adapter | Vendor SDK path |
| --- | --- | --- |
| BL602 | `build/bl602_make` | `os/bl602_sdk_lts` |
| BL616 | `build/bl616_make` | `os/bl616_sdk_lts` |
| BL618 | `build/bl618_make` | `os/bl618_sdk_lts` |
| LN882H | `build/ln882h_make` | `os/ln882h_sdk_lts` |

An application is only a candidate for a chip when its matching `vendor/<chip>` configuration also exists. Candidate status is not equivalent to a successful build.

## Ownership boundaries

- Change `application/<name>/src` for application behavior.
- Change `application/<name>/vendor/<chip>` for application-specific platform configuration.
- Change `adt` when introducing or correcting a shared abstraction.
- Change `build/<chip>_make` only when source assembly, vendor invocation, or artifact collection changes.
- Treat `os/<chip>_sdk_lts` as vendor-derived code: isolate local patches and retain upstream provenance.
- Treat `tmp` and `out` as build state, not authoritative source.

## Maintenance risks

- `build.sh update` synchronizes managed repositories and can overwrite local work.
- `build.sh reset` removes managed directories and performs a hard reset before updating.
- Included SDKs and toolchains make checkout and build storage requirements large.
- A directory or configuration file can become stale independently of the corresponding source, SDK, or toolchain. Validate a concrete matrix entry instead of inferring support from names.
- Firmware compilation does not cover flashing, boot, radio, cloud, peripheral, timing, or long-duration behavior.
