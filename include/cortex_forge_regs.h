// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_regs.h - Register offset definitions for Cortex Forge
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Register offsets for NVIDIA Tegra Orin NVDLA v2.0 and PVA v2.0 blocks.
 * These are PLACEHOLDERS until verified against the L4T TRM.
 */

#ifndef CORTEX_FORGE_REGS_H
#define CORTEX_FORGE_REGS_H

#include <linux/bitfield.h>
#include <linux/bits.h>

/*
 * TODO(HW): Verify all register offsets against NVIDIA Tegra Orin TRM,
 * section "NVDLA v2.0 Register Map" and "PVA v2.0 Register Map".
 * These offsets are PLACEHOLDERS and are deliberately invalid until verified.
 */

#define CORTEX_FORGE_REG_UNVERIFIED 0xFFFFFFFFU

/* ── NVDLA v2.0 registers (per instance) ─────────────────────────────────── */

#define CORTEX_FORGE_DLA_REG_CTRL       CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_STATUS     CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_INT_MASK   CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_INT_STATUS CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_TASK_ADDR  CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_TASK_SIZE  CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_FW_STATUS  CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_DLA_REG_PERF_CNT   CORTEX_FORGE_REG_UNVERIFIED

/* CTRL register fields */
#define CORTEX_FORGE_DLA_CTRL_ENABLE    BIT(0)
#define CORTEX_FORGE_DLA_CTRL_RESET     BIT(1)
#define CORTEX_FORGE_DLA_CTRL_SUSPEND   BIT(2)

/* STATUS register fields */
#define CORTEX_FORGE_DLA_STATUS_READY   BIT(0)
#define CORTEX_FORGE_DLA_STATUS_BUSY    BIT(1)
#define CORTEX_FORGE_DLA_STATUS_ERROR   BIT(2)

/* ── PVA v2.0 registers ───────────────────────────────────────────────────── */

#define CORTEX_FORGE_PVA_REG_CTRL       CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_STATUS     CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_INT_MASK   CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_INT_STATUS CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_TASK_ADDR  CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_TASK_SIZE  CORTEX_FORGE_REG_UNVERIFIED
#define CORTEX_FORGE_PVA_REG_FW_STATUS  CORTEX_FORGE_REG_UNVERIFIED

/* CTRL register fields */
#define CORTEX_FORGE_PVA_CTRL_ENABLE    BIT(0)
#define CORTEX_FORGE_PVA_CTRL_RESET     BIT(1)

/* STATUS register fields */
#define CORTEX_FORGE_PVA_STATUS_READY   BIT(0)
#define CORTEX_FORGE_PVA_STATUS_BUSY    BIT(1)
#define CORTEX_FORGE_PVA_STATUS_ERROR   BIT(2)

/* ── Register offset table ──────────────────────────────────────────────── */

/**
 * struct cortex_forge_regs - Register offset table for one SoC variant
 */
struct cortex_forge_regs {
	u32 ctrl;
	u32 status;
	u32 int_mask;
	u32 int_status;
	u32 task_addr;
	u32 task_size;
	u32 fw_status;
	u32 perf_cnt;
};

/* ── Helper ────────────────────────────────────────────────────────────────── */

static inline bool cortex_forge_regs_unverified(const struct cortex_forge_regs *regs)
{
	return regs->ctrl == CORTEX_FORGE_REG_UNVERIFIED;
}

#endif /* CORTEX_FORGE_REGS_H */
