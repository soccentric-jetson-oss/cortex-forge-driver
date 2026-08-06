// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_accel.h - Accelerator management for Cortex Forge driver
 *
 * Defines the accelerator structure and management API for NVDLA and PVA
 * hardware accelerators on the NVIDIA Jetson AGX Orin.
 */

#ifndef CORTEX_FORGE_ACCEL_H
#define CORTEX_FORGE_ACCEL_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/atomic.h>
#include <linux/device.h>

#include "cortex_forge_uapi.h"

#define NUM_ACCELERATORS 3

/* ── Accelerator types ────────────────────────────────────────────────── */

#define ACCEL_TYPE_DLA0 0
#define ACCEL_TYPE_DLA1 1
#define ACCEL_TYPE_PVA0 2

/* ── Accelerator descriptor ──────────────────────────────────────────── */

/**
 * struct cortex_forge_accel - Represents a hardware accelerator
 * @type:          Accelerator type (ACCEL_TYPE_*)
 * @name:          Human-readable name (e.g. "nvdla0")
 * @freq_hz:       Current operating frequency in Hz
 * @temp_celsius:  Current temperature in degrees Celsius
 * @load_percent:  Estimated load percentage (0-100)
 * @fw_version:    Firmware version (major << 16 | minor)
 * @hw_version:    Hardware revision
 * @mem_total:     Total accelerator memory in bytes
 * @mem_free:      Free accelerator memory in bytes
 * @state:         Power state (0=off, 1=low, 2=active)
 * @lock:          Mutex protecting task queues
 * @task_queue:    Queue of pending tasks
 * @active_tasks:  List of currently executing tasks
 * @worker_thread: Kernel thread processing tasks
 * @work_wait:     Wait queue for work notification
 * @running:       Atomic flag indicating thread should keep running
 */
struct cortex_forge_accel {
    u32 type;
    char name[16];
    u32 freq_hz;
    s32 temp_celsius;
    u32 load_percent;
    u32 fw_version;
    u32 hw_version;
    u64 mem_total;
    u64 mem_free;
    u32 state;
    struct mutex lock;
    struct list_head task_queue;
    struct list_head active_tasks;
    struct task_struct *worker_thread;
    wait_queue_head_t work_wait;
    atomic_t running;
};

/* ── Accelerator management API ────────────────────────────────────────── */

/**
 * accel_init - Initialize an accelerator structure
 * @accel: Accelerator to initialize
 * @type:  Accelerator type
 * @name:  Human-readable name
 */
void accel_init(struct cortex_forge_accel *accel, u32 type, const char *name);

/**
 * accel_start_workers - Start worker threads for all accelerators
 * @accelerators: Array of accelerators
 * @num:          Number of accelerators in the array
 * @dev:          Opaque device pointer (for error reporting)
 *
 * Returns: 0 on success, negative errno on failure.
 * On failure, any started workers are stopped.
 */
int accel_start_workers(struct cortex_forge_accel *accelerators, int num,
                        struct device *dev);

/**
 * accel_stop_workers - Stop all accelerator worker threads
 * @accelerators: Array of accelerators
 * @num:          Number of accelerators in the array
 */
void accel_stop_workers(struct cortex_forge_accel *accelerators, int num);

/**
 * accel_get_info - Fill an accel_info struct from accelerator state
 * @accel: Source accelerator
 * @info:  Destination info struct (kernel-side, not yet copied to user)
 */
void accel_get_info(struct cortex_forge_accel *accel,
                    struct cortex_forge_accel_info *info);

#endif /* CORTEX_FORGE_ACCEL_H */
