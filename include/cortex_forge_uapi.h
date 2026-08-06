// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_uapi.h - Userspace API definitions for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Defines ioctl numbers and data structures shared between kernel and
 * userspace. All structs use fixed-width types and explicit padding for
 * 32/64-bit compatibility.
 */

#ifndef CORTEX_FORGE_UAPI_H
#define CORTEX_FORGE_UAPI_H

#include <linux/types.h>
#include <linux/ioctl.h>

#define CORTEX_FORGE_MAGIC 0xCF

/* ── Accelerator types ──────────────────────────────────────────────────── */

#define CORTEX_FORGE_ACCEL_DLA0  0
#define CORTEX_FORGE_ACCEL_DLA1  1
#define CORTEX_FORGE_ACCEL_PVA0  2
#define CORTEX_FORGE_ACCEL_COUNT 3

/* ── Task status ─────────────────────────────────────────────────────────── */

#define CORTEX_FORGE_TASK_PENDING   0
#define CORTEX_FORGE_TASK_RUNNING   1
#define CORTEX_FORGE_TASK_COMPLETE  2
#define CORTEX_FORGE_TASK_FAILED    3
#define CORTEX_FORGE_TASK_CANCELLED 4

/* ── Task submission ─────────────────────────────────────────────────────── */

/**
 * struct cortex_forge_task_desc - Descriptor for an accelerator task
 * @task_id:      Output: assigned task identifier
 * @accel_type:   Accelerator type (CORTEX_FORGE_ACCEL_*)
 * @priority:     Task priority (0=lowest, 255=highest)
 * @input_size:   Size of input buffer in bytes
 * @output_size:  Size of output buffer in bytes
 * @input_addr:   Userspace pointer to input data
 * @output_addr:  Userspace pointer to output buffer
 * @timeout_ms:   Timeout in milliseconds (0 = no timeout)
 * @flags:        Reserved, must be zero
 * @reserved:     Padding for future use
 */
struct cortex_forge_task_desc {
	__u32 task_id;
	__u32 accel_type;
	__u32 priority;
	__u32 input_size;
	__u32 output_size;
	__u64 input_addr;
	__u64 output_addr;
	__u32 timeout_ms;
	__u32 flags;
	__u64 reserved[4];
};

/**
 * struct cortex_forge_task_status - Status of a submitted task
 * @task_id:    Task identifier
 * @status:     One of CORTEX_FORGE_TASK_*
 * @exec_usec:  Execution time in microseconds (valid when status == COMPLETE)
 * @error_code: Driver-specific error code (valid when status == FAILED)
 * @reserved:   Padding
 */
struct cortex_forge_task_status {
	__u32 task_id;
	__u32 status;
	__u64 exec_usec;
	__s32 error_code;
	__u32 reserved[3];
};

/**
 * struct cortex_forge_accel_info - Accelerator hardware information
 * @accel_type:     Accelerator type
 * @freq_hz:        Current operating frequency in Hz
 * @temp_celsius:   Current temperature in degrees Celsius
 * @load_percent:   Estimated load percentage (0-100)
 * @fw_version:     Firmware version encoded as (major << 16 | minor)
 * @hw_version:     Hardware revision
 * @mem_total:      Total accelerator memory in bytes
 * @mem_free:       Free accelerator memory in bytes
 * @state:          Power state (0=off, 1=low, 2=active)
 * @reserved:       Padding
 */
struct cortex_forge_accel_info {
	__u32 accel_type;
	__u32 freq_hz;
	__s32 temp_celsius;
	__u32 load_percent;
	__u32 fw_version;
	__u32 hw_version;
	__u64 mem_total;
	__u64 mem_free;
	__u32 state;
	__u32 reserved[3];
};

/* ── IOCTL definitions ──────────────────────────────────────────────────── */

#define CORTEX_FORGE_IOCTL_SUBMIT_TASK    _IOWR(CORTEX_FORGE_MAGIC, 1, struct cortex_forge_task_desc)
#define CORTEX_FORGE_IOCTL_QUERY_TASK     _IOWR(CORTEX_FORGE_MAGIC, 2, struct cortex_forge_task_status)
#define CORTEX_FORGE_IOCTL_CANCEL_TASK    _IOW(CORTEX_FORGE_MAGIC, 3, __u32)
#define CORTEX_FORGE_IOCTL_GET_ACCEL_INFO _IOR(CORTEX_FORGE_MAGIC, 4, struct cortex_forge_accel_info)
#define CORTEX_FORGE_IOCTL_SET_POWER      _IOW(CORTEX_FORGE_MAGIC, 5, __u32)
#define CORTEX_FORGE_IOCTL_GET_VERSION    _IOR(CORTEX_FORGE_MAGIC, 6, __u32)

/* GPU acceleration support */
#define CORTEX_FORGE_ACCEL_GPU   3
#define CORTEX_FORGE_ACCEL_AUTO  4

/* Profiling metrics */
struct cortex_forge_profile_metrics {
    __u64 compute_us;
    __u64 memory_us;
    __u64 storage_us;
    __u64 network_us;
    __u32 thermal_celsius;
    __u32 power_mw;
    __u64 reserved[4];
};

#define CORTEX_FORGE_IOCTL_GET_PROFILE _IOR(CORTEX_FORGE_MAGIC, 7, struct cortex_forge_profile_metrics)
#define CORTEX_FORGE_IOCTL_SET_ACCEL   _IOW(CORTEX_FORGE_MAGIC, 8, __u32)

#endif /* CORTEX_FORGE_UAPI_H */
