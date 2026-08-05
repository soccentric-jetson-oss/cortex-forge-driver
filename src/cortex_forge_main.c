// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_main.c - Core driver for NVIDIA Jetson AGX Orin ML accelerators
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * This file owns probe/remove, platform_driver registration, and module
 * init/exit. It allocates the per-device state struct, acquires platform
 * resources via devres, and hands off to sub-modules (chardev, sysfs,
 * debugfs, irq). Locking: dev->mutex protects all configuration state;
 * dev->spinlock protects task list and interrupt-shared data.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/idr.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/pm_runtime.h>

#include "cortex_forge_platform.h"
#include "cortex_forge_regs.h"
#include "cortex_forge_uapi.h"

#define DRV_NAME "cortex-forge"
#define DRV_VERSION "0.1.0"

/* ── Per-device state ────────────────────────────────────────────────────── */

/**
 * struct cortex_forge_task - An accelerator task tracking entry
 * @task_id:     Unique task identifier
 * @accel_type:  Accelerator type (DLA0, DLA1, PVA0)
 * @status:      Current task status
 * @submit_jiffies: jiffies when task was submitted
 * @entry:       List node for the task list
 */
struct cortex_forge_task {
	u32              task_id;
	u32              accel_type;
	u32              status;
	unsigned long    submit_jiffies;
	struct list_head entry;
};

/**
 * struct cortex_forge_dev - Per-instance driver state
 * @pdev:           Platform device
 * @soc:            SoC-specific data (ops, quirks, regmap config)
 * @regs:           Register offset table
 * @base:           MMIO base address (devm_ioremap)
 * @regmap:         Regmap for register access
 * @mutex:          Protects configuration and task list
 * @spinlock:       Protects interrupt-shared data
 * @tasks:          List of active tasks
 * @next_task_id:   Monotonically increasing task ID counter
 * @irq:            IRQ number
 * @clks:           Array of clocks
 * @num_clks:       Number of clocks
 * @rst:            Reset control
 * @class:          Device class
 * @devt:           Device number
 * @cdev:           Character device
 * @dev:            Device pointer
 * @dla_count:      Number of DLA instances
 * @pva_count:      Number of PVA instances
 */
struct cortex_forge_dev {
	struct platform_device          *pdev;
	const struct cortex_forge_soc_data *soc;
	struct cortex_forge_regs        regs;
	void __iomem                    *base;
	struct regmap                   *regmap;
	struct mutex                     mutex;
	spinlock_t                       spinlock;
	struct list_head                 tasks;
	atomic_t                         next_task_id;
	int                              irq;
	struct clk_bulk_data            *clks;
	int                              num_clks;
	struct reset_control            *rst;
	const struct class              *class;
	dev_t                            devt;
	struct cdev                      cdev;
	struct device                    *dev;
	unsigned int                     dla_count;
	unsigned int                     pva_count;
};

/* ── File operations ─────────────────────────────────────────────────────── */

static int cortex_forge_open(struct inode *inode, struct file *filp)
{
	struct cortex_forge_dev *dev = container_of(inode->i_cdev, struct cortex_forge_dev, cdev);
	filp->private_data = dev;
	return 0;
}

static int cortex_forge_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static long cortex_forge_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct cortex_forge_dev *dev = filp->private_data;
	void __user *user_arg = (void __user *)arg;

	switch (cmd) {
	case CORTEX_FORGE_IOCTL_SUBMIT_TASK: {
		struct cortex_forge_task_desc desc;
		struct cortex_forge_task *task;

		if (copy_from_user(&desc, user_arg, sizeof(desc)))
			return -EFAULT;

		if (desc.accel_type >= CORTEX_FORGE_ACCEL_COUNT)
			return -EINVAL;

		if (desc.flags != 0)
			return -EINVAL;

		task = kzalloc(sizeof(*task), GFP_KERNEL);
		if (!task)
			return -ENOMEM;

		mutex_lock(&dev->mutex);
		task->task_id = (u32)atomic_inc_return(&dev->next_task_id);
		task->accel_type = desc.accel_type;
		task->status = CORTEX_FORGE_TASK_PENDING;
		task->submit_jiffies = jiffies;
		list_add_tail(&task->entry, &dev->tasks);
		desc.task_id = task->task_id;
		mutex_unlock(&dev->mutex);

		if (copy_to_user(user_arg, &desc, sizeof(desc))) {
			mutex_lock(&dev->mutex);
			list_del(&task->entry);
			mutex_unlock(&dev->mutex);
			kfree(task);
			return -EFAULT;
		}

		return 0;
	}

	case CORTEX_FORGE_IOCTL_QUERY_TASK: {
		struct cortex_forge_task_status status;
		struct cortex_forge_task *task;
		bool found = false;

		if (copy_from_user(&status, user_arg, sizeof(status)))
			return -EFAULT;

		mutex_lock(&dev->mutex);
		list_for_each_entry(task, &dev->tasks, entry) {
			if (task->task_id == status.task_id) {
				status.status = task->status;
				status.exec_usec = 0;
				status.error_code = 0;
				found = true;
				break;
			}
		}
		mutex_unlock(&dev->mutex);

		if (!found)
			return -ENOENT;

		if (copy_to_user(user_arg, &status, sizeof(status)))
			return -EFAULT;

		return 0;
	}

	case CORTEX_FORGE_IOCTL_CANCEL_TASK: {
		u32 task_id;
		struct cortex_forge_task *task;
		bool found = false;

		if (copy_from_user(&task_id, user_arg, sizeof(task_id)))
			return -EFAULT;

		mutex_lock(&dev->mutex);
		list_for_each_entry(task, &dev->tasks, entry) {
			if (task->task_id == task_id) {
				task->status = CORTEX_FORGE_TASK_CANCELLED;
				list_del(&task->entry);
				kfree(task);
				found = true;
				break;
			}
		}
		mutex_unlock(&dev->mutex);

		return found ? 0 : -ENOENT;
	}

	case CORTEX_FORGE_IOCTL_GET_ACCEL_INFO: {
		struct cortex_forge_accel_info info;

		if (copy_from_user(&info, user_arg, sizeof(info)))
			return -EFAULT;

		if (info.accel_type >= CORTEX_FORGE_ACCEL_COUNT)
			return -EINVAL;

		info.freq_hz = 0;
		info.temp_celsius = 0;
		info.load_percent = 0;
		info.fw_version = 0x00010000;
		info.hw_version = 0x02000000;
		info.mem_total = 0;
		info.mem_free = 0;
		info.state = 2;

		if (copy_to_user(user_arg, &info, sizeof(info)))
			return -EFAULT;

		return 0;
	}

	case CORTEX_FORGE_IOCTL_GET_VERSION: {
		u32 version = 0x00010000;

		if (copy_to_user(user_arg, &version, sizeof(version)))
			return -EFAULT;

		return 0;
	}

	default:
		return -ENOTTY;
	}
}

#ifdef CONFIG_COMPAT
static long cortex_forge_compat_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	return cortex_forge_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static const struct file_operations cortex_forge_fops = {
	.owner          = THIS_MODULE,
	.open           = cortex_forge_open,
	.release        = cortex_forge_release,
	.unlocked_ioctl = cortex_forge_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = cortex_forge_compat_ioctl,
#endif
	.llseek         = no_llseek,
};

/* ── Probe / Remove ──────────────────────────────────────────────────────── */

static int cortex_forge_probe(struct platform_device *pdev)
{
	struct cortex_forge_dev *dev;
	struct device *devp = &pdev->dev;
	int ret;

	dev = devm_kzalloc(devp, sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->pdev = pdev;
	platform_set_drvdata(pdev, dev);

	/* Get SoC-specific data */
	dev->soc = cortex_forge_get_soc_data(devp);
	if (!dev->soc) {
		dev_err(devp, "no SoC match data found\n");
		return -ENODEV;
	}

	dev->dla_count = dev->soc->num_dla;
	dev->pva_count = dev->soc->num_pva;

	/* Initialize locks */
	mutex_init(&dev->mutex);
	spin_lock_init(&dev->spinlock);
	INIT_LIST_HEAD(&dev->tasks);
	atomic_set(&dev->next_task_id, 0);

	/* Map MMIO region */
	dev->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(dev->base))
		return dev_err_probe(devp, PTR_ERR(dev->base), "failed to map MMIO\n");

	/* Initialize regmap */
	if (dev->soc->regmap_cfg) {
		dev->regmap = devm_regmap_init_mmio(devp, dev->base, dev->soc->regmap_cfg);
		if (IS_ERR(dev->regmap))
			return dev_err_probe(devp, PTR_ERR(dev->regmap), "regmap init failed\n");
	}

	/* Get clocks */
	dev->num_clks = devm_clk_bulk_get_all(devp, &dev->clks);
	if (dev->num_clks < 0)
		return dev_err_probe(devp, dev->num_clks, "failed to get clocks\n");

	/* Get reset */
	dev->rst = devm_reset_control_get_exclusive(devp, NULL);
	if (IS_ERR(dev->rst))
		return dev_err_probe(devp, PTR_ERR(dev->rst), "failed to get reset\n");

	/* Get IRQ */
	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0)
		return dev_err_probe(devp, dev->irq, "failed to get IRQ\n");

	/* Request threaded IRQ */
	ret = devm_request_threaded_irq(devp, dev->irq, NULL,
		NULL, IRQF_SHARED, DRV_NAME, dev);
	if (ret)
		return dev_err_probe(devp, ret, "failed to request IRQ\n");

	/* Initialize hardware */
	if (dev->soc->ops && dev->soc->ops->init) {
		ret = dev->soc->ops->init(dev);
		if (ret)
			return dev_err_probe(devp, ret, "hardware init failed\n");
	}

	/* Apply quirks */
	if (dev->soc->ops && dev->soc->ops->quirk_fixup) {
		ret = dev->soc->ops->quirk_fixup(dev);
		if (ret)
			dev_warn(devp, "quirk fixup failed (non-fatal): %d\n", ret);
	}

	/* Register character device */
	ret = alloc_chrdev_region(&dev->devt, 0, 1, DRV_NAME);
	if (ret)
		return dev_err_probe(devp, ret, "failed to allocate chrdev region\n");

	cdev_init(&dev->cdev, &cortex_forge_fops);
	dev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&dev->cdev, dev->devt, 1);
	if (ret) {
		unregister_chrdev_region(dev->devt, 1);
		return dev_err_probe(devp, ret, "failed to add cdev\n");
	}

	/* Create device in class */
	dev->dev = device_create(dev->class, devp, dev->devt, dev, DRV_NAME "%u", 0);
	if (IS_ERR(dev->dev)) {
		cdev_del(&dev->cdev);
		unregister_chrdev_region(dev->devt, 1);
		return dev_err_probe(devp, PTR_ERR(dev->dev), "failed to create device\n");
	}

	dev_info(devp, "Cortex Forge driver v%s loaded (%u DLA, %u PVA)\n",
		 DRV_VERSION, dev->dla_count, dev->pva_count);

	return 0;
}

static void cortex_forge_remove(struct platform_device *pdev)
{
	struct cortex_forge_dev *dev = platform_get_drvdata(pdev);
	struct cortex_forge_task *task, *tmp;

	device_destroy(dev->class, dev->devt);
	cdev_del(&dev->cdev);
	unregister_chrdev_region(dev->devt, 1);

	/* Cancel all pending tasks */
	list_for_each_entry_safe(task, tmp, &dev->tasks, entry) {
		list_del(&task->entry);
		kfree(task);
	}

	if (dev->soc->ops && dev->soc->ops->deinit)
		dev->soc->ops->deinit(dev);

	dev_info(&pdev->dev, "Cortex Forge driver removed\n");
}

/* ── Power management ─────────────────────────────────────────────────────── */

static int cortex_forge_suspend(struct device *devp)
{
	struct cortex_forge_dev *dev = dev_get_drvdata(devp);

	if (dev->soc->ops && dev->soc->ops->deinit)
		dev->soc->ops->deinit(dev);

	return 0;
}

static int cortex_forge_resume(struct device *devp)
{
	struct cortex_forge_dev *dev = dev_get_drvdata(devp);

	if (dev->soc->ops && dev->soc->ops->init)
		return dev->soc->ops->init(dev);

	return 0;
}

static DEFINE_SIMPLE_DEV_PM_OPS(cortex_forge_pm_ops,
				cortex_forge_suspend,
				cortex_forge_resume);

/* ── Device tree match table ─────────────────────────────────────────────── */

static const struct of_device_id cortex_forge_of_match[] = {
	{ .compatible = "nvidia,tegra234-cortex-forge", .data = &cortex_forge_tegra234_data },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, cortex_forge_of_match);

/* ── Platform driver ─────────────────────────────────────────────────────── */

static struct platform_driver cortex_forge_driver = {
	.probe  = cortex_forge_probe,
	.remove = cortex_forge_remove,
	.driver = {
		.name   = DRV_NAME,
		.of_match_table = cortex_forge_of_match,
		.pm     = pm_sleep_ptr(&cortex_forge_pm_ops),
	},
};

/* ── Module init / exit ───────────────────────────────────────────────────── */

static const struct class cortex_forge_class = {
	.name = DRV_NAME,
	.owner = THIS_MODULE,
};

static int __init cortex_forge_init(void)
{
	int ret;

	ret = class_register(&cortex_forge_class);
	if (ret)
		return ret;

	ret = platform_driver_register(&cortex_forge_driver);
	if (ret) {
		class_unregister(&cortex_forge_class);
		return ret;
	}

	pr_info("Cortex Forge driver v%s initialized\n", DRV_VERSION);
	return 0;
}

static void __exit cortex_forge_exit(void)
{
	platform_driver_unregister(&cortex_forge_driver);
	class_unregister(&cortex_forge_class);
	pr_info("Cortex Forge driver unloaded\n");
}

module_init(cortex_forge_init);
module_exit(cortex_forge_exit);

MODULE_AUTHOR("Sandesh <sandesh@soccentric.com>");
MODULE_DESCRIPTION("NVIDIA Jetson AGX Orin ML accelerator (NVDLA/PVA) driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:" DRV_NAME);
