// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_platform.c - Platform-specific data for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Supplies SoC-specific ops tables and match data for supported platforms.
 * This is the only file that contains hardware-specific register offsets,
 * clock names, and compatible strings.
 */

#include <linux/device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regmap.h>

#include "cortex_forge_dev.h"
#include "cortex_forge_platform.h"
#include "cortex_forge_regs.h"
#include "cortex_forge_uapi.h"

#define DRV_NAME "cortex-forge"

/* ── Tegra234 (Jetson AGX Orin) register map ──────────────────────────────── */

/*
 * TODO(HW): Verify register offsets against NVIDIA Tegra Orin TRM.
 * These are PLACEHOLDERS and will cause probe to fail until replaced.
 */

static const struct cortex_forge_regs cortex_forge_tegra234_dla_regs = {
    .ctrl       = CORTEX_FORGE_REG_UNVERIFIED,
    .status     = CORTEX_FORGE_REG_UNVERIFIED,
    .int_mask   = CORTEX_FORGE_REG_UNVERIFIED,
    .int_status = CORTEX_FORGE_REG_UNVERIFIED,
    .task_addr  = CORTEX_FORGE_REG_UNVERIFIED,
    .task_size  = CORTEX_FORGE_REG_UNVERIFIED,
    .fw_status  = CORTEX_FORGE_REG_UNVERIFIED,
    .perf_cnt   = CORTEX_FORGE_REG_UNVERIFIED,
};

static const struct cortex_forge_regs cortex_forge_tegra234_pva_regs = {
    .ctrl       = CORTEX_FORGE_REG_UNVERIFIED,
    .status     = CORTEX_FORGE_REG_UNVERIFIED,
    .int_mask   = CORTEX_FORGE_REG_UNVERIFIED,
    .int_status = CORTEX_FORGE_REG_UNVERIFIED,
    .task_addr  = CORTEX_FORGE_REG_UNVERIFIED,
    .task_size  = CORTEX_FORGE_REG_UNVERIFIED,
    .fw_status  = CORTEX_FORGE_REG_UNVERIFIED,
    .perf_cnt   = CORTEX_FORGE_REG_UNVERIFIED,
};

/* ── Regmap config ───────────────────────────────────────────────────────── */

static const struct regmap_config cortex_forge_regmap_cfg = {
    .reg_bits     = 32,
    .val_bits     = 32,
    .reg_stride   = 4,
    .max_register = 0x1000,
    .cache_type   = REGCACHE_FLAT,
    .volatile_reg = NULL,
    .precious_reg = NULL,
};

/* ── Hardware operations ──────────────────────────────────────────────────── */

static int cortex_forge_tegra234_init(struct cortex_forge_dev* dev)
{
    /* TODO(HW): Enable clocks, deassert resets, configure power domain */
    dev_info(&dev->pdev->dev, "Tegra234 hardware init (stub)\n");
    return 0;
}

static void cortex_forge_tegra234_deinit(struct cortex_forge_dev* dev)
{
    /* TODO(HW): Disable clocks, assert resets */
    dev_dbg(&dev->pdev->dev, "Tegra234 hardware deinit (stub)\n");
}

static u32 cortex_forge_tegra234_irq_ack(struct cortex_forge_dev* dev)
{
    /* TODO(HW): Read and acknowledge interrupt status register */
    return 0;
}

static int cortex_forge_tegra234_configure(struct cortex_forge_dev*          dev,
                                           const struct cortex_forge_config* cfg)
{
    /* TODO(HW): Apply configuration to hardware registers */
    return 0;
}

static int cortex_forge_tegra234_submit_task(struct cortex_forge_dev* dev, u32 task_id,
                                             u64 input_addr, u32 input_size, u64 output_addr,
                                             u32 output_size)
{
    /* TODO(HW): Write task descriptor to accelerator doorbell */
    return 0;
}

static int cortex_forge_tegra234_poll_task(struct cortex_forge_dev* dev, u32 task_id, u32* status)
{
    /* TODO(HW): Read task completion status register */
    *status = CORTEX_FORGE_TASK_COMPLETE;
    return 0;
}

static const struct cortex_forge_hw_ops cortex_forge_tegra234_ops = {
    .init        = cortex_forge_tegra234_init,
    .deinit      = cortex_forge_tegra234_deinit,
    .irq_ack     = cortex_forge_tegra234_irq_ack,
    .configure   = cortex_forge_tegra234_configure,
    .submit_task = cortex_forge_tegra234_submit_task,
    .poll_task   = cortex_forge_tegra234_poll_task,
    .quirk_fixup = NULL,
};

/* ── Clock names ─────────────────────────────────────────────────────────── */

static const char* const cortex_forge_tegra234_clocks[] = {
    "dla0_core", "dla0_falcon", "dla1_core", "dla1_falcon", "pva0_core", "pva0_falcon", NULL,
};

/* ── SoC data ─────────────────────────────────────────────────────────────── */

const struct cortex_forge_soc_data cortex_forge_tegra234_data = {
    .name       = "Tegra234 (Jetson AGX Orin)",
    .ops        = &cortex_forge_tegra234_ops,
    .regmap_cfg = &cortex_forge_regmap_cfg,
    .num_dla    = 2,
    .num_pva    = 1,
    .clk_names  = cortex_forge_tegra234_clocks,
    .quirks     = CORTEX_FORGE_QUIRK_NONE,
};

/* ── Match table ─────────────────────────────────────────────────────────── */

static const struct of_device_id cortex_forge_of_match[] = {
    {
        .compatible = "nvidia,tegra234-cortex-forge",
        .data       = &cortex_forge_tegra234_data,
    },
    {/* sentinel */}};
MODULE_DEVICE_TABLE(of, cortex_forge_of_match);

/* ── Lookup ──────────────────────────────────────────────────────────────── */

const struct cortex_forge_soc_data* cortex_forge_get_soc_data(struct device* dev)
{
    const struct of_device_id* match;

    if (!dev->of_node)
        return NULL;

    match = of_match_node(cortex_forge_of_match, dev->of_node);
    if (!match)
        return NULL;

    return match->data;
}
EXPORT_SYMBOL_GPL(cortex_forge_get_soc_data);
