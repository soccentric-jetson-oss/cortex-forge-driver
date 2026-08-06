// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_debugfs.c - Debugfs interface for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Provides diagnostic registers dumps, interrupt/error counters, and
 * fault injection controls. All debugfs entries are explicitly unstable.
 */

#include <linux/debugfs.h>
#include <linux/errno.h>
#include <linux/seq_file.h>

#include "cortex_forge_dev.h"
#include "cortex_forge_platform.h"
#include "cortex_forge_uapi.h"

#define DRV_NAME "cortex-forge"

/* ── Register dump ───────────────────────────────────────────────────────── */

static int cortex_forge_regs_show(struct seq_file* s, void* data)
{
    struct cortex_forge_dev* dev = s->private;

    seq_printf(s, "Cortex Forge Register Dump (%s)\n", dev->soc->name);
    seq_puts(s, "  Register offsets are unverified (TODO(HW))\n");
    seq_printf(s, "  DLA instances: %u\n", dev->dla_count);
    seq_printf(s, "  PVA instances: %u\n", dev->pva_count);

    return 0;
}

static int cortex_forge_regs_open(struct inode* inode, struct file* file)
{
    return single_open(file, cortex_forge_regs_show, inode->i_private);
}

static const struct file_operations cortex_forge_regs_fops = {
    .owner   = THIS_MODULE,
    .open    = cortex_forge_regs_open,
    .read    = seq_read,
    .release = single_release,
};

/* ── Fault injection ──────────────────────────────────────────────────────── */

#ifdef CONFIG_CORTEX_FORGE_FAULT_INJECT

static int cortex_forge_fault_probe_fail_set(void* data, u64 val)
{
    /* TODO: Implement probe failure injection */
    return 0;
}
DEFINE_DEBUGFS_ATTRIBUTE(cortex_forge_fault_probe_fail_fops, NULL,
                         cortex_forge_fault_probe_fail_set, "%llu\n");

#endif /* CONFIG_CORTEX_FORGE_FAULT_INJECT */

/* ── Init / Cleanup ──────────────────────────────────────────────────────── */

int cortex_forge_debugfs_init(struct cortex_forge_dev* dev)
{
    struct dentry* dir;

    dir = debugfs_create_dir(DRV_NAME, NULL);
    if (IS_ERR(dir))
        return PTR_ERR(dir);

    debugfs_create_file("registers", 0444, dir, dev, &cortex_forge_regs_fops);

#ifdef CONFIG_CORTEX_FORGE_FAULT_INJECT
    {
        struct dentry* fault_dir;
        fault_dir = debugfs_create_dir("fault", dir);
        debugfs_create_file("probe_fail", 0200, fault_dir, dev,
                            &cortex_forge_fault_probe_fail_fops);
    }
#endif

    return 0;
}

void cortex_forge_debugfs_cleanup(struct cortex_forge_dev* dev)
{
    debugfs_remove_recursive(debugfs_lookup(DRV_NAME, NULL));
}
