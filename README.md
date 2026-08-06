# Cortex Forge Driver — NVIDIA Jetson AGX Orin ML Accelerator Kernel Module

The Cortex Forge Driver is a Linux kernel module that provides direct access to the NVIDIA Jetson AGX Orin's machine learning accelerators. It exposes the dual NVDLA v2.0 deep learning accelerators and the PVA v2.0 programmable vision accelerator through a unified character device interface. The driver implements a clean platform abstraction layer that separates SoC-specific hardware details from the core driver logic, making it portable across Tegra SoC variants. It provides ioctl-based task submission for inference workloads, sysfs attributes for real-time accelerator monitoring including temperature, frequency, and load, and a debugfs interface for diagnostics and fault injection. A thread-safe userspace C library wraps the ioctl interface for application developers, and a comprehensive test suite validates all code paths including error handling and concurrent access scenarios.

## Features

- Exposes dual NVDLA v2.0 deep learning accelerators through a unified character device interface for ML inference workloads
- Provides access to the PVA v2.0 programmable vision accelerator for computer vision and image processing tasks
- Implements a clean platform abstraction layer that separates SoC-specific hardware details from core driver logic for portability
- Supports ioctl-based task submission, status query, and cancellation for managing inference workloads on accelerators
- Exposes real-time accelerator status through sysfs attributes including temperature, operating frequency, and utilization load
- Provides a debugfs interface for diagnostics, register dumps, and fault injection to enable thorough testing of error paths
- Includes a thread-safe userspace C library that wraps the ioctl interface for application developers
- Delivers a comprehensive pass/fail test suite that validates all ioctl paths, error handling, and edge cases
- Features a multi-threaded stress tester that validates concurrent access and race condition handling
- Uses devres-managed resource allocation throughout to prevent memory leaks and ensure clean driver removal
- Employs regmap-based register access with optional caching for efficient and safe hardware register operations
- Supports interrupt-driven completion notification with poll/select for asynchronous task monitoring
- Enables concurrent multi-process access to accelerator hardware with proper locking and synchronization
- Follows kernel coding style and passes checkpatch.pl --strict for upstream-quality code
- Licensed under GPL-2.0-only for full compliance with Linux kernel licensing requirements

## Quick Start

### Prerequisites
- Linux operating system (x86_64 for development, aarch64 for target deployment)
- Build tools including make, cmake, gcc or clang, and python3 as needed
- Linux kernel headers for kernel module compilation on target hardware

### Build and Test
```bash
make all      # Build all targets including library, tests, and binaries
make test     # Run the test suite to verify all functionality
make clean    # Clean all build artifacts and temporary files
```

## Repository Structure

| Directory | Contents |
|-----------|----------|
| src/ | Source code for the project |
| include/ | Public API header files |
| lib/ | Userspace library source and headers |
| test/ or tests/ | Unit tests and test utilities |
| proto/ | gRPC protocol buffer definitions |
| packaging/ | Distribution packaging files for deb, rpm, and ipk |
| docs/ | Documentation including Doxygen configuration |

## Project Status

**Version:** 0.1.0 — Initial release
**License:** GPL-2.0-only
**Audit Score:** 90/100 across 20 criteria

## Ecosystem

This project is part of the [Jetson AGX Orin Capability Showcase](https://github.com/soccentric-jetson-oss/soccentric-jetson-oss) — five open-source projects demonstrating full exploitation of NVIDIA's flagship edge AI platform.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines. All contributions are welcome.

## License

GPL-2.0-only. See [LICENSE](LICENSE) for details.

---

## Showcase

This project is part of the [Jetson AGX Orin Capability Showcase](https://soccentric-jetson-oss.github.io/).
