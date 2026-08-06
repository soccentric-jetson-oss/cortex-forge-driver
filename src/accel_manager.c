// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file accel_manager.c
 * @brief Accelerator management for Cortex Forge driver
 *
 * Implements accelerator initialization, worker thread lifecycle,
 * and info querying for NVDLA v2.0 and PVA v2.0 accelerators.
 *
 * @copyright Copyright (c) 2026 SoC Centric LLC
 * @author Sandesh Ghimire
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/device.h>
#include "cortex_forge_dev.h"
#include "cortex_forge_uapi.h"

/**
 * @brief Initialize an accelerator structure with default values.
 * @param accel Accelerator to initialize
 * @param type  Accelerator type (ACCEL_TYPE_DLA0, _DLA1, or _PVA0)
 * @param name  Human-readable name (e.g. "nvdla0")
 */
void accel_init(struct cortex_forge_accel *accel, u32 type, const char *name)
{
    accel->type = type;
    strncpy(accel->name, name, sizeof(accel->name) - 1);
    accel->freq_hz = 1000000000;
    accel->temp_celsius = 40;
    accel->load_percent = 0;
    accel->fw_version = 0x00010000;
    accel->hw_version = 0x02000000;
    accel->mem_total = 1024ULL * 1024 * 1024;
    accel->mem_free = 1024ULL * 1024 * 1024;
    accel->state = 2;
    mutex_init(&accel->lock);
    INIT_LIST_HEAD(&accel->task_queue);
    INIT_LIST_HEAD(&accel->active_tasks);
    init_waitqueue_head(&accel->work_wait);
    atomic_set(&accel->running, 1);
}

/**
 * @brief Worker thread function for an accelerator.
 *
 * Processes tasks from the accelerator's task queue in FIFO order.
 * Simulates hardware execution with configurable timeout support.
 *
 * @param data Pointer to the cortex_forge_accel struct
 * @return 0 on normal exit
 */
static int accel_worker_thread(void *data)
{
    struct cortex_forge_accel *accel = data;
    struct cortex_forge_task *task;

    while (atomic_read(&accel->running)) {
        wait_event_interruptible_timeout(accel->work_wait,
            !list_empty(&accel->task_queue) || !atomic_read(&accel->running),
            msecs_to_jiffies(100));

        if (!atomic_read(&accel->running))
            break;

        mutex_lock(&accel->lock);
        if (!list_empty(&accel->task_queue)) {
            task = list_first_entry(&accel->task_queue,
                                    struct cortex_forge_task, entry);
            list_del(&task->entry);
            list_add_tail(&task->entry, &accel->active_tasks);
            mutex_unlock(&accel->lock);

            task->state = TASK_STATE_RUNNING;
            accel->load_percent = min(100U, accel->load_percent + 10);

            if (task->timeout_ms > 0) {
                unsigned long timeout = msecs_to_jiffies(task->timeout_ms);
                unsigned long start = jiffies;
                while (jiffies - start < timeout &&
                       task->state == TASK_STATE_RUNNING) {
                    msleep(1);
                    if (kthread_should_stop())
                        break;
                }
            } else {
                msleep(10);
            }

            task->complete_jiffies = jiffies;
            task->state = TASK_STATE_COMPLETE;
            task->error_code = 0;
            accel->load_percent = max(0, (int)accel->load_percent - 10);
            complete_all(&task->done);

            mutex_lock(&accel->lock);
            list_del(&task->entry);
            mutex_unlock(&accel->lock);
        } else {
            mutex_unlock(&accel->lock);
        }
    }
    return 0;
}

/**
 * @brief Start worker threads for all accelerators.
 * @param accelerators Array of accelerator structs
 * @param num          Number of accelerators in the array
 * @param dev          Kernel device for error reporting
 * @return 0 on success, negative errno on failure
 */
int accel_start_workers(struct cortex_forge_accel *accelerators, int num,
                        struct device *dev)
{
    int i;
    int ret;

    for (i = 0; i < num; i++) {
        accelerators[i].worker_thread = kthread_run(accel_worker_thread,
            &accelerators[i], "cortex-forge-%s", accelerators[i].name);
        if (IS_ERR(accelerators[i].worker_thread)) {
            ret = PTR_ERR(accelerators[i].worker_thread);
            dev_err(dev, "Failed to start worker for %s: %d\n",
                    accelerators[i].name, ret);
            goto err_stop;
        }
    }
    return 0;

err_stop:
    accel_stop_workers(accelerators, i);
    return ret;
}

/**
 * @brief Stop all accelerator worker threads.
 * @param accelerators Array of accelerator structs
 * @param num          Number of accelerators in the array
 */
void accel_stop_workers(struct cortex_forge_accel *accelerators, int num)
{
    int i;

    for (i = 0; i < num; i++) {
        if (accelerators[i].worker_thread) {
            atomic_set(&accelerators[i].running, 0);
            wake_up(&accelerators[i].work_wait);
            kthread_stop(accelerators[i].worker_thread);
            accelerators[i].worker_thread = NULL;
        }
    }
}

/**
 * @brief Fill an accel_info struct from accelerator state.
 * @param accel Source accelerator
 * @param info  Destination info struct (kernel-side)
 */
void accel_get_info(struct cortex_forge_accel *accel,
                    struct cortex_forge_accel_info *info)
{
    mutex_lock(&accel->lock);
    info->freq_hz = accel->freq_hz;
    info->temp_celsius = accel->temp_celsius;
    info->load_percent = accel->load_percent;
    info->fw_version = accel->fw_version;
    info->hw_version = accel->hw_version;
    info->mem_total = accel->mem_total;
    info->mem_free = accel->mem_free;
    info->state = accel->state;
    mutex_unlock(&accel->lock);
}
