// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_platform.h - Platform abstraction seam for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * This header defines the hardware abstraction interface. Every SoC variant
 * supplies an ops table and soc_data through cortex_forge_platform.c.
 * The core driver never contains a SoC name, base address, or register offset.
 */

#ifndef CORTEX_FORGE_PLATFORM_H
#define CORTEX_FORGE_PLATFORM_H

#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/types.h>

/* ── Forward declarations ───────────────────────────────────────────────── */

struct cortex_forge_dev;
struct cortex_forge_config;

/* ── Quirk flags ────────────────────────────────────────────────────────── */

#define CORTEX_FORGE_QUIRK_NONE 0
#define CORTEX_FORGE_QUIRK_DLA1_SLOW BIT(0)
#define CORTEX_FORGE_QUIRK_PVA_ERRATA BIT(1)

/* ── Hardware operations ─────────────────────────────────────────────────── */

/**
 * struct cortex_forge_hw_ops - Platform-specific hardware operations
 * @init:        One-time block bring-up: clocks, resets, power, default state
 * @deinit:      Tear-down, exact inverse of @init
 * @irq_ack:     Acknowledge and clear interrupt source, return event mask
 * @configure:   Apply validated configuration to the hardware
 * @submit_task: Submit a task descriptor to the accelerator hardware
 * @poll_task:   Poll for task completion
 * @quirk_fixup: Optional erratum workaround applied late in probe; may be NULL
 */
struct cortex_forge_hw_ops
{
    int (*init)(struct cortex_forge_dev* dev);
    void (*deinit)(struct cortex_forge_dev* dev);
    u32 (*irq_ack)(struct cortex_forge_dev* dev);
    int (*configure)(struct cortex_forge_dev* dev, const struct cortex_forge_config* cfg);
    int (*submit_task)(struct cortex_forge_dev* dev, u32 task_id, u64 input_addr, u32 input_size,
                       u64 output_addr, u32 output_size);
    int (*poll_task)(struct cortex_forge_dev* dev, u32 task_id, u32* status);
    int (*quirk_fixup)(struct cortex_forge_dev* dev);
};

/* ── SoC data ────────────────────────────────────────────────────────────── */

/**
 * struct cortex_forge_soc_data - Compile-time description of one SoC variant
 * @name:           Human-readable SoC identifier
 * @ops:            Hardware operation table
 * @regmap_cfg:     Regmap configuration
 * @num_dla:        Number of NVDLA instances
 * @num_pva:        Number of PVA instances
 * @clk_names:      NULL-terminated list of clock names
 * @quirks:         Bitmask of CORTEX_FORGE_QUIRK_* flags
 */
struct cortex_forge_soc_data
{
    const char*                       name;
    const struct cortex_forge_hw_ops* ops;
    const struct regmap_config*       regmap_cfg;
    unsigned int                      num_dla;
    unsigned int                      num_pva;
    const char* const*                clk_names;
    u32                               quirks;
};

/* ── Platform functions ──────────────────────────────────────────────────── */

const struct cortex_forge_soc_data* cortex_forge_get_soc_data(struct device* dev);

#endif /* CORTEX_FORGE_PLATFORM_H */
