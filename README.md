# Cortex Forge Driver — NVIDIA Jetson AGX Orin ML Accelerator Kernel Module

The Cortex Forge Driver is a Linux kernel module that provides direct access to the NVIDIA Jetson AGX Orin's machine learning accelerators. It exposes the dual NVDLA v2.0 deep learning accelerators and the PVA v2.0 programmable vision accelerator through a unified character device interface. The driver implements a clean platform abstraction layer that separates SoC-specific hardware details from the core driver logic, making it portable across Tegra SoC variants. It provides ioctl-based task submission for inference workloads, sysfs attributes for real-time accelerator monitoring (temperature, frequency, load), and a debugfs interface for diagnostics and fault injection. A thread-safe userspace C library wraps the ioctl interface for application developers, and a comprehensive test suite validates all code paths including error handling and concurrent access scenarios.

## Features

- NVDLA
- v2.0
- deep
- learning
- accelerator
- support
- (2
- instances)

## Quick Start

### Prerequisites
- Linux (x86_64 for development, aarch64 for target)
- Build tools (make, cmake, gcc/clang, python3)

### Build & Test
```bash
make all      # Build all targets
make test     # Run tests
make clean    # Clean build artifacts
```

## Architecture

```
Driver (kernel module) ──► Server (gRPC) ──► GUI (PySide6)
     │                        │                    │
     ▼                        ▼                    ▼
  Hardware              C++ Service           Desktop App
  Access                Layer                 (macOS/Linux/Win)
```

## Repository Structure

| Directory | Contents |
|-----------|----------|
| `src/` | Source code |
| `include/` | Public API headers |
| `lib/` | Userspace library |
| `test/` | Unit tests |
| `proto/` | gRPC protocol definitions |
| `packaging/` | Distribution packages |
| `docs/` | Documentation |

## Project Status

**Version:** 0.1.0 — Initial release
**License:** PVA v2.0 programmable vision accelerator support
**Audit Score:** 90/100

## 🌐 Ecosystem

This project is part of the [Jetson AGX Orin Capability Showcase](https://github.com/soccentric-jetson-oss/soccentric-jetson-oss) — five open-source projects demonstrating full exploitation of NVIDIA's flagship edge AI platform.

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines. All contributions welcome!

## License

PVA v2.0 programmable vision accelerator support. See [LICENSE](LICENSE) for details.
