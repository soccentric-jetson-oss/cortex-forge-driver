// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file chardev.c
 * @brief Character device operations for Cortex Forge driver
 *
 * Implements open, release, and ioctl handlers that dispatch to
 * the task manager and accelerator manager modules.
 *
 * @copyright Copyright (c) 2026 SoC Centric LLC
 * @author Sandesh Ghimire
 */

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "cortex_forge_dev.h"
#include "cortex_forge_uapi.h"

static int cortex_forge_open(struct inode* inode, struct file* filp)
{
    filp->private_data = container_of(inode->i_cdev, struct cortex_forge_dev, cdev);
    return 0;
}

static int cortex_forge_release(struct inode* inode, struct file* filp)
{
    return 0;
}

static long cortex_forge_ioctl(struct file* filp, unsigned int cmd, unsigned long arg)
{
    struct cortex_forge_dev*  dev  = filp->private_data;
    void __user*              uarg = (void __user*) arg;
    struct cortex_forge_task* task;
    int                       ret = 0;

    switch (cmd)
    {

        case CORTEX_FORGE_IOCTL_SUBMIT_TASK:
        {
            struct cortex_forge_task_desc desc;

            if (copy_from_user(&desc, uarg, sizeof(desc)))
                return -EFAULT;
            if (desc.accel_type >= NUM_ACCELERATORS)
                return -EINVAL;
            if (desc.flags != 0)
                return -EINVAL;
            if (desc.input_size > 64 * 1024 * 1024)
                return -E2BIG;
            if (desc.output_size > 64 * 1024 * 1024)
                return -E2BIG;

            task = task_alloc(&dev->free_tasks, &dev->task_lock, &dev->next_task_id);
            if (!task)
                return -ENOMEM;

            task->accel_type  = desc.accel_type;
            task->priority    = desc.priority;
            task->input_addr  = desc.input_addr;
            task->input_size  = desc.input_size;
            task->output_addr = desc.output_addr;
            task->output_size = desc.output_size;
            task->timeout_ms  = desc.timeout_ms;

            ret = task_submit(task, &dev->accelerators[desc.accel_type]);
            if (ret)
            {
                task_free(task, &dev->free_tasks, &dev->task_lock);
                return ret;
            }

            desc.task_id = task->id;
            if (copy_to_user(uarg, &desc, sizeof(desc)))
                return -EFAULT;
            return 0;
        }

        case CORTEX_FORGE_IOCTL_QUERY_TASK:
        {
            struct cortex_forge_task_status status;

            if (copy_from_user(&status, uarg, sizeof(status)))
                return -EFAULT;
            return task_query(dev->tasks, status.task_id, uarg);
        }

        case CORTEX_FORGE_IOCTL_CANCEL_TASK:
        {
            u32 task_id;

            if (copy_from_user(&task_id, uarg, sizeof(task_id)))
                return -EFAULT;
            return task_cancel(dev->tasks, task_id);
        }

        case CORTEX_FORGE_IOCTL_GET_ACCEL_INFO:
        {
            struct cortex_forge_accel_info info;
            struct cortex_forge_accel*     accel;

            if (copy_from_user(&info, uarg, sizeof(info)))
                return -EFAULT;
            if (info.accel_type >= NUM_ACCELERATORS)
                return -EINVAL;

            accel = &dev->accelerators[info.accel_type];
            accel_get_info(accel, &info);

            if (copy_to_user(uarg, &info, sizeof(info)))
                return -EFAULT;
            return 0;
        }

        case CORTEX_FORGE_IOCTL_SET_POWER:
        {
            u32 mode;

            if (copy_from_user(&mode, uarg, sizeof(mode)))
                return -EFAULT;
            if (mode > 2)
                return -EINVAL;
            dev_info(&dev->pdev->dev, "Set power mode: %u\n", mode);
            return 0;
        }

        case CORTEX_FORGE_IOCTL_GET_VERSION:
        {
            u32 version = 0x00010000;

            if (copy_to_user(uarg, &version, sizeof(version)))
                return -EFAULT;
            return 0;
        }

        case CORTEX_FORGE_IOCTL_GET_PROFILE:
        {
            struct cortex_forge_profile_metrics pm;

            memset(&pm, 0, sizeof(pm));
            pm.compute_us      = 1000;
            pm.memory_us       = 500;
            pm.storage_us      = 200;
            pm.network_us      = 300;
            pm.thermal_celsius = 45;
            pm.power_mw        = 15000;
            if (copy_to_user(uarg, &pm, sizeof(pm)))
                return -EFAULT;
            return 0;
        }

        case CORTEX_FORGE_IOCTL_SET_ACCEL:
        {
            u32 accel;

            if (copy_from_user(&accel, uarg, sizeof(accel)))
                return -EFAULT;
            if (accel > 4)
                return -EINVAL;
            dev_info(&dev->pdev->dev, "Set accelerator: %u\n", accel);
            return 0;
        }

        default:
            return -ENOTTY;
    }
}

const struct file_operations cortex_forge_fops = {
    .owner          = THIS_MODULE,
    .open           = cortex_forge_open,
    .release        = cortex_forge_release,
    .unlocked_ioctl = cortex_forge_ioctl,
    .llseek         = noop_llseek,
};
