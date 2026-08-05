> ## ⚠️ PRELIMINARY — NOT HARDWARE-VALIDATED
>
> This is a **preliminary driver framework**, generated as a starting point. It has been
> checked for compilation and style only. It has **not** been run on real hardware, and no
> register offset, interrupt number, clock name, or timing value in `src/cortex_forge_platform.c`
> should be trusted until verified against the NVIDIA Tegra Orin Technical Reference Manual.
>
> Before running this on a board:
>
> 1. `grep -rn "TODO(" .` and resolve every entry.
> 2. Verify the register map in `include/cortex_forge_regs.h` against the TRM.
> 3. Verify the device tree overlay in `dts/` against the board schematic.
> 4. Confirm no in-tree driver already claims this hardware (`/proc/iomem`, `dmesg`), and
>    blacklist or unbind it if one does.
> 5. Test first on a board you can afford to brick.
>
> Writing to an incorrect physical address can hang the bus, corrupt an unrelated peripheral,
> or damage the board.

# Cortex Forge Driver

Linux kernel driver for the NVIDIA Jetson AGX Orin's machine learning accelerators —
NVDLA v2.0 (2×) and PVA v2.0 (1×). Part of the [Cortex Forge](https://github.com/soccentric-jetson-oss/cortex-forge) project.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Userspace                                 │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │  libcortex-   │  │  Test Suite  │  │  Stress Tester   │  │
│  │  forge.so     │  │  (pass/fail) │  │  (multi-thread)  │  │
│  └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘  │
│         │                  │                    │            │
│    ┌────┴──────────────────┴────────────────────┴──────┐    │
│    │              ioctl() / sysfs / /dev               │    │
│    └────────────────────────┬──────────────────────────┘    │
├──────────────────────────────┼──────────────────────────────┤
│                    Kernel    │                              │
│    ┌─────────────────────────┴──────────────────────────┐   │
│    │              cortex-forge.ko                        │   │
│    │  ┌──────────┐ ┌──────────┐ ┌──────────┐ ┌────────┐ │   │
│    │  │  chardev │ │  sysfs   │ │ debugfs  │ │   irq  │ │   │
│    │  │  (ioctl) │ │ (status) │ │ (diag)   │ │ (event)│ │   │
│    │  └──────────┘ └──────────┘ └──────────┘ └────────┘ │   │
│    │  ┌──────────────────────────────────────────────┐  │   │
│    │  │         platform abstraction layer            │  │   │
│    │  │  (cortex_forge_platform.c / .h)              │  │   │
│    │  └──────────────────────────────────────────────┘  │   │
│    └──────────────────────┬────────────────────────────┘   │
├───────────────────────────┼─────────────────────────────────┤
│                    Hardware                                 │
│    ┌──────────────────────┴──────────────────────────────┐ │
│    │  NVIDIA Jetson AGX Orin                              │ │
│    │  ┌──────────┐ ┌──────────┐ ┌──────────┐            │ │
│    │  │ NVDLA 0  │ │ NVDLA 1  │ │ PVA v2.0 │            │ │
│    │  │ (v2.0)   │ │ (v2.0)   │ │          │            │ │
│    │  └──────────┘ └──────────┘ └──────────┘            │ │
│    └─────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Components

| Component | Description |
|-----------|-------------|
| **Kernel module** (`cortex-forge.ko`) | Char driver exposing NVDLA and PVA accelerators via `/dev/cortex-forge*` |
| **Userspace library** (`libcortex-forge.so`) | C API wrapping driver ioctls for application developers |
| **Test suite** (`cortex-forge_test`) | Pass/fail tests exercising all ioctl paths |
| **Stress tester** (`cortex-forge_stress`) | Multi-threaded concurrent access test |

## Quick Start

```bash
# Build userspace library and tests
make lib test

# Build kernel module (requires Jetson kernel headers)
make module

# Run tests
LD_LIBRARY_PATH=build ./build/cortex-forge_test

# Install
sudo make install
```

## Hardware Support

| SoC | Platform | Status |
|-----|----------|--------|
| Tegra234 | Jetson AGX Orin | Implemented (register offsets unverified) |

## License

GPL-2.0-only

## 🌐 Ecosystem Website
Visit the [Jetson AGX Orin Capability Showcase](https://github.com/soccentric-jetson-oss/soccentric-jetson-oss) for an overview of all projects.
