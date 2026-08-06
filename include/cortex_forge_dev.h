// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_dev.h - Main device structure for Cortex Forge driver
 *
 * Aggregates all sub-components (task pool, accelerators, char device,
 * hardware resources) into a single device structure.
 */

#ifndef CORTEX_FORGE_DEV_H
#define CORTEX_FORGE_DEV_H

#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/regmap.h>

#include "cortex_forge_task.h"
#include "cortex_forge_accel.h"

/**
 * struct cortex_forge_dev - Main driver device structure
 * @pdev:         Platform device
 * @cdev:         Character device
 * @devt:         Device number
 * @dev:          Kernel device
 * @class:        Device class
 * @base:         MMIO base address
 * @regmap:       Register map for accelerator control
 * @dev_lock:     Mutex protecting device-level state
 * @task_lock:    Spinlock protecting task pool
 * @accelerators: Array of hardware accelerators (DLA0, DLA1, PVA)
 * @tasks:        Array of all task slots
 * @free_tasks:   List of free (unallocated) tasks
 * @next_task_id: Atomic counter for task ID generation
 * @irq:          Interrupt number
 * @clks:         Bulk clock handles
 * @num_clks:     Number of clocks
 * @rst:          Reset control
 */
struct cortex_forge_dev {
    struct platform_device *pdev;
    struct cdev cdev;
    dev_t devt;
    struct device *dev;
    const struct class *class;
    void __iomem *base;
    struct regmap *regmap;
    struct mutex dev_lock;
    spinlock_t task_lock;
    struct cortex_forge_accel accelerators[NUM_ACCELERATORS];
    struct cortex_forge_task tasks[MAX_TASKS];
    struct list_head free_tasks;
    atomic_t next_task_id;
    int irq;
    struct clk_bulk_data *clks;
    int num_clks;
    struct reset_control *rst;
};

/* ── Global device pointer (set during probe) ─────────────────────────── */

extern struct cortex_forge_dev *g_dev;

/* ── Char device operations (defined in chardev.c) ──────────────────── */

extern const struct file_operations cortex_forge_fops;

#endif /* CORTEX_FORGE_DEV_H */
