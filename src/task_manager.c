// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file task_manager.c
 * @brief Task pool management for Cortex Forge driver
 *
 * Implements task allocation, submission, query, and cancellation.
 * Tasks are pre-allocated in a fixed pool and managed via lock-free
 * list operations under a spinlock.
 *
 * @copyright Copyright (c) 2026 SoC Centric LLC
 * @author Sandesh Ghimire
 */

#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "cortex_forge_dev.h"
#include "cortex_forge_uapi.h"

/**
 * @brief Initialize the task pool with all tasks in the free list.
 * @param tasks     Array of task structs to initialize
 * @param free_list Head of the free list to populate
 * @param lock      Spinlock protecting the free list (unused here, owned by caller)
 */
void task_pool_init(struct cortex_forge_task* tasks, struct list_head* free_list, spinlock_t* lock)
{
    int i;
    for (i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].state = TASK_STATE_FREE;
        init_completion(&tasks[i].done);
        list_add_tail(&tasks[i].entry, free_list);
    }
}

/**
 * @brief Allocate a task from the free pool.
 * @param free_list Head of the free list
 * @param lock      Spinlock protecting the free list
 * @param next_id   Atomic counter for task ID generation
 * @return Pointer to allocated task, or NULL if pool exhausted
 */
struct cortex_forge_task* task_alloc(struct list_head* free_list, spinlock_t* lock,
                                     atomic_t* next_id)
{
    struct cortex_forge_task* task = NULL;
    unsigned long             flags;

    spin_lock_irqsave(lock, flags);
    if (!list_empty(free_list))
    {
        task = list_first_entry(free_list, struct cortex_forge_task, entry);
        list_del(&task->entry);
        memset(task, 0, sizeof(*task));
        init_completion(&task->done);
        task->id = atomic_inc_return(next_id);
    }
    spin_unlock_irqrestore(lock, flags);

    return task;
}

/**
 * @brief Return a task to the free pool.
 * @param task      Task to free
 * @param free_list Head of the free list
 * @param lock      Spinlock protecting the free list
 */
void task_free(struct cortex_forge_task* task, struct list_head* free_list, spinlock_t* lock)
{
    unsigned long flags;

    spin_lock_irqsave(lock, flags);
    task->state = TASK_STATE_FREE;
    list_add_tail(&task->entry, free_list);
    spin_unlock_irqrestore(lock, flags);
}

/**
 * @brief Enqueue a task on an accelerator's work queue.
 * @param task      Task to submit
 * @param accel_ptr Pointer to the target cortex_forge_accel
 * @return 0 on success, -EINVAL if accelerator type is invalid
 */
int task_submit(struct cortex_forge_task* task, void* accel_ptr)
{
    struct cortex_forge_accel* accel = accel_ptr;

    if (task->accel_type >= NUM_ACCELERATORS)
        return -EINVAL;

    task->state          = TASK_STATE_PENDING;
    task->submit_jiffies = jiffies;

    mutex_lock(&accel->lock);
    list_add_tail(&task->entry, &accel->task_queue);
    mutex_unlock(&accel->lock);

    wake_up(&accel->work_wait);
    return 0;
}

/**
 * @brief Query the status of a task by ID.
 * @param tasks   Array of all task slots
 * @param task_id Task ID to query
 * @param ustatus Userspace status struct to fill
 * @return 0 on success, -ENOENT if not found, -EFAULT on copy error
 */
int task_query(struct cortex_forge_task* tasks, u32 task_id,
               struct cortex_forge_task_status __user* ustatus)
{
    struct cortex_forge_task*       task;
    struct cortex_forge_task_status status;
    int                             i;

    for (i = 0; i < MAX_TASKS; i++)
    {
        task = &tasks[i];
        if (task->id == task_id && task->state != TASK_STATE_FREE)
        {
            memset(&status, 0, sizeof(status));
            status.task_id    = task->id;
            status.status     = task->state;
            status.exec_usec  = (task->complete_jiffies - task->submit_jiffies) * 1000000 / HZ;
            status.error_code = task->error_code;
            if (copy_to_user(ustatus, &status, sizeof(status)))
                return -EFAULT;
            return 0;
        }
    }
    return -ENOENT;
}

/**
 * @brief Cancel a task by ID.
 * @param tasks   Array of all task slots
 * @param task_id Task ID to cancel
 * @return 0 on success, -ENOENT if task not found
 */
int task_cancel(struct cortex_forge_task* tasks, u32 task_id)
{
    struct cortex_forge_task* task;
    int                       i;

    for (i = 0; i < MAX_TASKS; i++)
    {
        task = &tasks[i];
        if (task->id == task_id && task->state != TASK_STATE_FREE)
        {
            task->state      = TASK_STATE_CANCELLED;
            task->error_code = -ECANCELED;
            complete_all(&task->done);
            return 0;
        }
    }
    return -ENOENT;
}
