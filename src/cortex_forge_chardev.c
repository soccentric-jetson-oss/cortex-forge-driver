// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_chardev.c - Character device operations for Cortex Forge
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * Implements file_operations for /dev/cortex-forge*. The primary ioctl
 * dispatch lives in cortex_forge_main.c; this file provides the open/release
 * and any read/write support for data transfer.
 */

#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/poll.h>

#include "cortex_forge_platform.h"
#include "cortex_forge_uapi.h"

/* ── Read / Write (bulk data transfer) ───────────────────────────────────── */

static ssize_t cortex_forge_read(struct file *filp, char __user *buf,
				  size_t count, loff_t *f_pos)
{
	struct cortex_forge_dev *dev = filp->private_data;
	ssize_t ret = 0;

	/* TODO(HW): Implement read for result data transfer */
	return ret;
}

static ssize_t cortex_forge_write(struct file *filp, const char __user *buf,
				   size_t count, loff_t *f_pos)
{
	struct cortex_forge_dev *dev = filp->private_data;
	ssize_t ret = 0;

	/* TODO(HW): Implement write for input data transfer */
	return ret;
}

/* ── Poll (event notification) ───────────────────────────────────────────── */

static __poll_t cortex_forge_poll(struct file *filp, struct poll_table_struct *wait)
{
	struct cortex_forge_dev *dev = filp->private_data;
	__poll_t mask = 0;

	/* TODO(HW): Implement poll for task completion notification */
	poll_wait(filp, NULL, wait);

	return mask;
}
