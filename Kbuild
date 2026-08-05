# SPDX-License-Identifier: GPL-2.0-only
#
# Kbuild - kernel module build description for cortex-forge
#
# Copyright (C) 2026 SoC Centric
# Author: Sandesh <sandesh@soccentric.com>
#

obj-m += cortex-forge.o

cortex-forge-y := src/cortex_forge_main.o \
                  src/cortex_forge_platform.o \
                  src/cortex_forge_chardev.o \
                  src/cortex_forge_sysfs.o \
                  src/cortex_forge_debugfs.o \
                  src/cortex_forge_irq.o

cortex-forge-$(CONFIG_CORTEX_FORGE_FAULT_INJECT) += src/cortex_forge_fault.o

ccflags-y := -I$(src)/include -DDRV_VERSION=\"$(DRV_VERSION)\"
CFLAGS_cortex_forge_trace.o := -I$(src)/include
