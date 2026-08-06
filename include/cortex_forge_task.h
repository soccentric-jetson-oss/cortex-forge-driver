// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_task.h - Task management for Cortex Forge driver
 *
 * Defines task states, the task structure, and the task management API.
 * Tasks represent inference jobs submitted to NVDLA/PVA accelerators.
 */

#ifndef CORTEX_FORGE_TASK_H
#define CORTEX_FORGE_TASK_H

#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include "cortex_forge_uapi.h"

#define MAX_TASKS 256

/* ── Task states ──────────────────────────────────────────────────────── */

enum task_state
{
    TASK_STATE_FREE = 0,
    TASK_STATE_PENDING,
    TASK_STATE_RUNNING,
    TASK_STATE_COMPLETE,
    TASK_STATE_FAILED,
    TASK_STATE_CANCELLED,
};

/* ── Task descriptor ──────────────────────────────────────────────────── */

/**
 * struct cortex_forge_task - Represents a single inference task
 * @id:              Unique task identifier
 * @state:           Current task state (see enum task_state)
 * @accel_type:      Target accelerator (0=DLA0, 1=DLA1, 2=PVA)
 * @priority:        Task priority (0-255, higher = more urgent)
 * @input_addr:      Userspace pointer to input buffer
 * @input_size:      Size of input data in bytes
 * @output_addr:     Userspace pointer to output buffer
 * @output_size:     Size of output buffer in bytes
 * @timeout_ms:      Execution timeout in milliseconds (0 = no timeout)
 * @submit_jiffies:  Jiffies at submission time
 * @complete_jiffies: Jiffies at completion time
 * @error_code:      Error code if state == FAILED
 * @entry:           List node for free/queue/active lists
 * @done:            Completion for synchronous wait
 */
struct cortex_forge_task
{
    u32               id;
    enum task_state   state;
    u32               accel_type;
    u32               priority;
    u64               input_addr;
    u32               input_size;
    u64               output_addr;
    u32               output_size;
    u32               timeout_ms;
    unsigned long     submit_jiffies;
    unsigned long     complete_jiffies;
    int               error_code;
    struct list_head  entry;
    struct completion done;
};

/* ── Task management API ──────────────────────────────────────────────── */

/**
 * task_pool_init - Initialize the task pool with free tasks
 * @tasks:    Array of task structs
 * @free_list: Head of the free list to populate
 * @lock:     Spinlock protecting the free list
 */
void task_pool_init(struct cortex_forge_task* tasks, struct list_head* free_list, spinlock_t* lock);

/**
 * task_alloc - Allocate a task from the free pool
 * @free_list: Head of the free list
 * @lock:      Spinlock protecting the free list
 * @next_id:   Atomic counter for task IDs
 *
 * Returns: Pointer to allocated task, or NULL if pool exhausted.
 */
struct cortex_forge_task* task_alloc(struct list_head* free_list, spinlock_t* lock,
                                     atomic_t* next_id);

/**
 * task_free - Return a task to the free pool
 * @task:      Task to free
 * @free_list: Head of the free list
 * @lock:      Spinlock protecting the free list
 */
void task_free(struct cortex_forge_task* task, struct list_head* free_list, spinlock_t* lock);

/**
 * task_submit - Enqueue a task on an accelerator's work queue
 * @task:  Task to submit
 * @accel: Target accelerator (opaque pointer to cortex_forge_accel)
 *
 * Returns: 0 on success, negative errno on failure.
 */
int task_submit(struct cortex_forge_task* task, void* accel);

/**
 * task_query - Query the status of a task by ID
 * @tasks:    Array of all tasks
 * @task_id:  Task ID to query
 * @ustatus:  Userspace status struct to fill
 *
 * Returns: 0 on success, -ENOENT if task not found, -EFAULT on copy error.
 */
int task_query(struct cortex_forge_task* tasks, u32 task_id,
               struct cortex_forge_task_status __user* ustatus);

/**
 * task_cancel - Cancel a task by ID
 * @tasks:   Array of all tasks
 * @task_id: Task ID to cancel
 *
 * Returns: 0 on success, -ENOENT if task not found.
 */
int task_cancel(struct cortex_forge_task* tasks, u32 task_id);

#endif /* CORTEX_FORGE_TASK_H */
