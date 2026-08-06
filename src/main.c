// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file main.c
 * @brief Module entry point for Cortex Forge driver
 *
 * Thin module glue: init, exit, probe, remove. Delegates all
 * functional logic to task_manager, accel_manager, and chardev modules.
 *
 * @copyright Copyright (c) 2026 SoC Centric LLC
 * @author Sandesh Ghimire
 */

#include <linux/cdev.h>
#include <linux/clk.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/slab.h>

#include "cortex_forge_dev.h"
#include "cortex_forge_uapi.h"

#define DRV_NAME "cortex-forge"
#define DRV_VERSION "0.1.0"

struct cortex_forge_dev* g_dev;

static const struct class cortex_forge_class = {
    .name = DRV_NAME,
};

static int cortex_forge_probe(struct platform_device* pdev)
{
    struct device*           devp = &pdev->dev;
    struct cortex_forge_dev* dev;
    int                      ret;

    dev = devm_kzalloc(devp, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->pdev = pdev;
    platform_set_drvdata(pdev, dev);
    g_dev = dev;

    /* ── Initialize locks and task pool ──────────────────────────────── */
    mutex_init(&dev->dev_lock);
    spin_lock_init(&dev->task_lock);
    INIT_LIST_HEAD(&dev->free_tasks);
    atomic_set(&dev->next_task_id, 0);
    task_pool_init(dev->tasks, &dev->free_tasks, &dev->task_lock);

    /* ── Initialize accelerators ────────────────────────────────────── */
    accel_init(&dev->accelerators[0], ACCEL_TYPE_DLA0, "nvdla0");
    accel_init(&dev->accelerators[1], ACCEL_TYPE_DLA1, "nvdla1");
    accel_init(&dev->accelerators[2], ACCEL_TYPE_PVA0, "pva0");

    /* ── Hardware resources ─────────────────────────────────────────── */
    dev->base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(dev->base))
        return dev_err_probe(devp, PTR_ERR(dev->base), "MMIO failed\n");

    dev->num_clks = devm_clk_bulk_get_all(devp, &dev->clks);
    if (dev->num_clks < 0)
        return dev_err_probe(devp, dev->num_clks, "clocks failed\n");

    dev->rst = devm_reset_control_get_exclusive(devp, NULL);
    if (IS_ERR(dev->rst))
        return dev_err_probe(devp, PTR_ERR(dev->rst), "reset failed\n");

    dev->irq = platform_get_irq(pdev, 0);
    if (dev->irq < 0)
        return dev_err_probe(devp, dev->irq, "IRQ failed\n");

    /* ── Start accelerator worker threads ────────────────────────────── */
    ret = accel_start_workers(dev->accelerators, NUM_ACCELERATORS, devp);
    if (ret)
        return ret;

    /* ── Character device registration ──────────────────────────────── */
    ret = alloc_chrdev_region(&dev->devt, 0, 1, DRV_NAME);
    if (ret)
        goto err_stop_workers;

    cdev_init(&dev->cdev, &cortex_forge_fops);
    dev->cdev.owner = THIS_MODULE;
    ret             = cdev_add(&dev->cdev, dev->devt, 1);
    if (ret)
    {
        unregister_chrdev_region(dev->devt, 1);
        goto err_stop_workers;
    }

    dev->dev = device_create(&cortex_forge_class, devp, dev->devt, dev, DRV_NAME "%u", 0);
    if (IS_ERR(dev->dev))
    {
        cdev_del(&dev->cdev);
        unregister_chrdev_region(dev->devt, 1);
        ret = PTR_ERR(dev->dev);
        goto err_stop_workers;
    }

    dev_info(devp, "Cortex Forge v%s: 2xNVDLA + 1xPVA\n", DRV_VERSION);
    return 0;

err_stop_workers:
    accel_stop_workers(dev->accelerators, NUM_ACCELERATORS);
    return ret;
}

static void cortex_forge_remove(struct platform_device* pdev)
{
    struct cortex_forge_dev* dev = platform_get_drvdata(pdev);

    device_destroy(&cortex_forge_class, dev->devt);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->devt, 1);
    accel_stop_workers(dev->accelerators, NUM_ACCELERATORS);
    dev_info(&pdev->dev, "Cortex Forge removed\n");
}

/* ── Device tree match table ─────────────────────────────────────────── */

static const struct of_device_id cortex_forge_of_match[] = {
    {.compatible = "nvidia,tegra234-cortex-forge"}, {}};
MODULE_DEVICE_TABLE(of, cortex_forge_of_match);

static struct platform_driver cortex_forge_driver = {
    .probe  = cortex_forge_probe,
    .remove = cortex_forge_remove,
    .driver =
        {
            .name           = DRV_NAME,
            .of_match_table = cortex_forge_of_match,
        },
};

/* ── Module init / exit ──────────────────────────────────────────────── */

static int __init cortex_forge_init(void)
{
    int r;

    r = class_register(&cortex_forge_class);
    if (r)
        return r;

    r = platform_driver_register(&cortex_forge_driver);
    if (r)
        class_unregister(&cortex_forge_class);

    pr_info("Cortex Forge driver v%s initialized\n", DRV_VERSION);
    return r;
}

static void __exit cortex_forge_exit(void)
{
    platform_driver_unregister(&cortex_forge_driver);
    class_unregister(&cortex_forge_class);
    pr_info("Cortex Forge driver unloaded\n");
}

module_init(cortex_forge_init);
module_exit(cortex_forge_exit);

MODULE_AUTHOR("Sandesh Ghimire <sandesh@soccentric.com>");
MODULE_DESCRIPTION("NVIDIA Jetson AGX Orin ML accelerator (NVDLA/PVA) driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(DRV_VERSION);
MODULE_ALIAS("platform:" DRV_NAME);
