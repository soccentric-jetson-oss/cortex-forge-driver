// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_irq.c - Interrupt handling for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Hardirq handler acknowledges the interrupt and defers processing to
 * the threaded handler. The threaded handler updates task status and
 * wakes any poll waiters.
 */

#include <linux/interrupt.h>
#include <linux/slab.h>

#include "cortex_forge_platform.h"
#include "cortex_forge_uapi.h"
#include "cortex_forge_dev.h"

#define DRV_NAME "cortex-forge"

/* ── Hardirq handler ─────────────────────────────────────────────────────── */

static irqreturn_t cortex_forge_hardirq(int irq, void *data)
{
	struct cortex_forge_dev *dev = data;
	u32 events;

	if (!dev->soc->ops || !dev->soc->ops->irq_ack)
		return IRQ_NONE;

	events = dev->soc->ops->irq_ack(dev);
	if (!events)
		return IRQ_NONE; /* Shared line, not ours */

	/* Store events for threaded handler */
	WRITE_ONCE(dev->pending_events, events);

	return IRQ_WAKE_THREAD;
}

/* ── Threaded handler ────────────────────────────────────────────────────── */

static irqreturn_t cortex_forge_threaded_irq(int irq, void *data)
{
	struct cortex_forge_dev *dev = data;
	u32 events = READ_ONCE(dev->pending_events);

	/* TODO(HW): Process completed tasks, update status, wake waiters */

	return IRQ_HANDLED;
}

/* ── IRQ setup ───────────────────────────────────────────────────────────── */

int cortex_forge_irq_init(struct cortex_forge_dev *dev)
{
	int ret;

	ret = devm_request_threaded_irq(&dev->pdev->dev, dev->irq,
					 cortex_forge_hardirq,
					 cortex_forge_threaded_irq,
					 IRQF_SHARED, DRV_NAME, dev);
	if (ret)
		dev_err(&dev->pdev->dev, "failed to request IRQ %d: %d\n",
			dev->irq, ret);

	return ret;
}
