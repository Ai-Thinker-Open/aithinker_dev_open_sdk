[![中文](https://img.shields.io/badge/中文-README-blue)](README.zh.md)

# Ai-Thinker Unified Development SDK

This repository provides a common application-development layer for multiple Ai-Thinker module platforms. It keeps application code under `application/`, abstracts chip and RTOS differences under `adt/`, and delegates final firmware generation to the selected vendor SDK under `os/`.

The top-level build integration currently contains adapters for BL602, BL616, BL618, and LN882H. Other vendor-named directories may exist in individual applications, but their presence alone does not mean that the top-level build supports or validates them.

## Repository map

| Path | Purpose |
| --- | --- |
| `application/<name>/src/` | Portable application code. Each application normally defines `aiio_main`. |
| `application/<name>/vendor/<chip>/` | Per-application Kconfig and vendor build configuration. |
| `adt/adt_os/` | Common API, core components, platform ports, and the firmware-to-application startup bridge. |
| `build/<chip>_make/` | Chip-specific source assembly, vendor build invocation, cleanup, and artifact collection. |
| `os/<chip>_sdk_lts/` | Vendor SDK and toolchain integration. |
| `tmp/` | Generated, assembled build tree. Do not treat it as an application source directory. |
| `out/<chip>/<application>/` | Firmware binaries, ELF files, and map files produced by a build. |
| `tools/` | Environment, configuration, formatting, documentation, and maintenance scripts. |

See [Code entry](docs/CODE_ENTRY.md) and [Architecture](docs/ARCHITECTURE.md) for the traced startup and build flows.

## Requirements

- Linux, or WSL2 with a Linux distribution. `build.sh` intentionally rejects non-Linux environments.
- Bash and Git.
- The packages installed by `./build.sh environment` when a clean environment is used. Review that command before running it: it uses `sudo apt-get` and `pip`, and therefore changes the host environment and needs network access.
- Enough disk space for the included vendor SDKs, extracted toolchains, `tmp/`, and `out/`.

## Quick start

Run commands from the repository root:

```bash
./build.sh help
./build.sh version
```

Configure and build the smallest reference application for BL602:

```bash
./build.sh bl602 demo defconfig
./build.sh bl602 demo cn debug
```

Interactive configuration is also available:

```bash
./build.sh bl602 demo menuconfig
```

The build command has this form:

```text
./build.sh <chip> <application> <country> <level>
```

- `<chip>`: a chip with both `os/<chip>_sdk_lts/` and `build/<chip>_make/`; currently `bl602`, `bl616`, `bl618`, or `ln882h`.
- `<application>`: a directory name below `application/`.
- `<country>`: `cn` or `en`.
- `<level>`: `debug`, `debug_tmp`, `debug_ram`, or `release`.

Successful builds place artifacts in `out/<chip>/<application>/bin`, `elf`, and `map`.

## Application entry

The vendor startup macro expands `MAIN_ENTRY()` in `adt/adt_os/adapter_main/main/aiio_main_init.c`. That bridge initializes the selected chip, logging, and manufacturer data, creates the `aiio_main` task, and starts the scheduler. Applications provide that task in `application/<name>/src/main.c`.

The reference entry is:

```text
vendor startup
  -> MAIN_ENTRY()
  -> chip/log/manufacturer initialization
  -> aiio_main task creation
  -> application/demo/src/main.c:aiio_main()
```

## Common operations

```bash
# Clean every output
./build.sh clean

# Clean one application
./build.sh bl602 demo clean

# Regenerate interface documentation
./build.sh docs
```

Use the maintenance commands carefully:

- `./build.sh update` synchronizes managed source areas and may overwrite local changes.
- `./build.sh reset` deletes managed directories, runs `git reset --hard`, and then updates them. Commit or back up work first.
- Build configuration and toolchain preparation can extract archives and modify generated/vendor work trees.

## Verification status

Repository structure, startup flow, build routing, shell syntax, help/version dispatch, bilingual document pairs, and local document links are covered by the recorded checks in [Validation](docs/VALIDATION.md).

A complete firmware build, flashing, boot test, peripheral test, wireless test, and hardware regression were not completed by that documentation review. Do not interpret a documentation check as device validation.

## Troubleshooting

- **`error: unkonw os`**: run the build from Linux or WSL2, not directly from PowerShell or Command Prompt.
- **No adaptation for a chip/application**: confirm the matching `os/<chip>_sdk_lts`, `build/<chip>_make`, `application/<name>`, and `application/<name>/vendor/<chip>` paths exist.
- **Missing configuration**: run `defconfig` or `menuconfig` before the four-argument build command.
- **Toolchain extraction or package errors**: verify archive integrity, available disk space, permissions, and the prerequisites listed above.
- **Unexpected changes after update/reset**: restore from your own commit or backup; these commands are intentionally destructive.

## Contributing

Keep reusable application logic in `application/<name>/src`, platform-specific configuration in `application/<name>/vendor/<chip>`, and chip adaptation code in `adt`. Before opening a change:

1. Build at least one affected chip/application pair from a clean configuration.
2. Record the exact command, warnings, output artifacts, and untested hardware scope.
3. Update both English `.md` and Chinese `.zh.md` documents when behavior changes.
4. Avoid committing generated `tmp/` or `out/` content unless the repository policy explicitly requires it.

## License

This repository is distributed under the Apache License 2.0. See [`license.txt`](license.txt).
