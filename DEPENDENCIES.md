# Dependencies

## Upstream Dependencies

This driver has no runtime dependencies on other Cortex Forge components.
It is a standalone kernel module.

## Build Dependencies

- Linux kernel headers (L4T 35.x / kernel 5.15 for Jetson AGX Orin)
- GCC or Clang
- GNU Make
- (Optional) Docker for cross-build

## Runtime Dependencies

- NVIDIA Jetson AGX Orin (Tegra234 SoC)
- L4T kernel with NVDLA and PVA device tree nodes enabled
