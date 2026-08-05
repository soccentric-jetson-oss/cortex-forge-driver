# Changelog

## v0.1.0 (2026-08-05)

- Initial driver implementation for NVIDIA Jetson AGX Orin ML accelerators
- Char device interface for NVDLA (2×) and PVA (1×)
- ioctl-based task submission, query, and cancellation
- sysfs attributes for accelerator status
- debugfs interface for diagnostics
- Userspace C library (libcortex-forge)
- Test suite and multi-threaded stress tester
- Platform abstraction layer for SoC-specific data
- Register map stubbed (TODO(HW): verify against TRM)
