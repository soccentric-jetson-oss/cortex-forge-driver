// SPDX-License-Identifier: GPL-2.0-only
/*
 * libcortex_forge.h - Public API for Cortex Forge userspace library
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * Thread-safe C library wrapping the Cortex Forge kernel driver ioctl
 * interface. Provides task submission, status query, and accelerator
 * information retrieval.
 */

#ifndef LIBCORTEX_FORGE_H
#define LIBCORTEX_FORGE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Opaque handle ───────────────────────────────────────────────────────── */

struct cortex_forge_handle;

/* ── Accelerator types ──────────────────────────────────────────────────── */

#define CORTEX_FORGE_ACCEL_DLA0 0
#define CORTEX_FORGE_ACCEL_DLA1 1
#define CORTEX_FORGE_ACCEL_PVA0 2

/* ── Task status values ──────────────────────────────────────────────────── */

#define CORTEX_FORGE_TASK_PENDING   0
#define CORTEX_FORGE_TASK_RUNNING   1
#define CORTEX_FORGE_TASK_COMPLETE  2
#define CORTEX_FORGE_TASK_FAILED    3
#define CORTEX_FORGE_TASK_CANCELLED 4

/* ── Task descriptor ─────────────────────────────────────────────────────── */

/**
 * @brief Descriptor for an accelerator task.
 */
struct cortex_forge_task_desc {
	uint32_t task_id;       /**< Output: assigned task ID */
	uint32_t accel_type;    /**< Accelerator type (CORTEX_FORGE_ACCEL_*) */
	uint32_t priority;      /**< Priority (0=low, 255=high) */
	uint32_t input_size;    /**< Input buffer size in bytes */
	uint32_t output_size;   /**< Output buffer size in bytes */
	const void *input_addr; /**< Pointer to input data */
	void *output_addr;      /**< Pointer to output buffer */
	uint32_t timeout_ms;    /**< Timeout in ms (0 = no timeout) */
	uint32_t flags;         /**< Reserved, must be 0 */
};

/**
 * @brief Task status information.
 */
struct cortex_forge_task_status {
	uint32_t task_id;       /**< Task identifier */
	uint32_t status;        /**< One of CORTEX_FORGE_TASK_* */
	uint64_t exec_usec;     /**< Execution time in microseconds */
	int32_t  error_code;    /**< Error code (valid when FAILED) */
};

/**
 * @brief Accelerator hardware information.
 */
struct cortex_forge_accel_info {
	uint32_t accel_type;    /**< Accelerator type */
	uint32_t freq_hz;       /**< Current frequency in Hz */
	int32_t  temp_celsius;  /**< Temperature in °C */
	uint32_t load_percent;  /**< Load percentage (0-100) */
	uint32_t fw_version;    /**< Firmware version */
	uint32_t hw_version;    /**< Hardware revision */
	uint64_t mem_total;     /**< Total memory in bytes */
	uint64_t mem_free;      /**< Free memory in bytes */
	uint32_t state;         /**< Power state (0=off, 1=low, 2=active) */
};

/* ── Lifecycle ───────────────────────────────────────────────────────────── */

/**
 * @brief Open a connection to the Cortex Forge driver.
 * @param device_path Path to the device node (e.g., "/dev/cortex-forge0").
 * @return Opaque handle on success, NULL on failure (errno set).
 */
struct cortex_forge_handle *cortex_forge_open(const char *device_path);

/**
 * @brief Close a connection and release resources.
 * @param h Handle returned by cortex_forge_open(). NULL is safe.
 */
void cortex_forge_close(struct cortex_forge_handle *h);

/* ── Task operations ─────────────────────────────────────────────────────── */

/**
 * @brief Submit a task to an accelerator.
 * @param h    Handle from cortex_forge_open().
 * @param desc Task descriptor (task_id is filled on success).
 * @return 0 on success, negative errno on failure.
 */
int cortex_forge_submit_task(struct cortex_forge_handle *h,
			      struct cortex_forge_task_desc *desc);

/**
 * @brief Query the status of a previously submitted task.
 * @param h      Handle from cortex_forge_open().
 * @param status Task status structure (task_id must be set).
 * @return 0 on success, negative errno on failure.
 */
int cortex_forge_query_task(struct cortex_forge_handle *h,
			     struct cortex_forge_task_status *status);

/**
 * @brief Cancel a pending or running task.
 * @param h       Handle from cortex_forge_open().
 * @param task_id Task identifier to cancel.
 * @return 0 on success, negative errno on failure.
 */
int cortex_forge_cancel_task(struct cortex_forge_handle *h, uint32_t task_id);

/* ── Accelerator information ─────────────────────────────────────────────── */

/**
 * @brief Get information about an accelerator.
 * @param h    Handle from cortex_forge_open().
 * @param info Accelerator info structure (accel_type must be set).
 * @return 0 on success, negative errno on failure.
 */
int cortex_forge_get_accel_info(struct cortex_forge_handle *h,
				 struct cortex_forge_accel_info *info);

/**
 * @brief Get the driver version.
 * @param h       Handle from cortex_forge_open().
 * @param version Output: version encoded as (major << 16 | minor).
 * @return 0 on success, negative errno on failure.
 */
int cortex_forge_get_version(struct cortex_forge_handle *h, uint32_t *version);

/* ── Error handling ──────────────────────────────────────────────────────── */

/**
 * @brief Get a human-readable string for an error code.
 * @param errnum Negative errno value.
 * @return Static string describing the error.
 */
const char *cortex_forge_strerror(int errnum);

#ifdef __cplusplus
}
#endif

#endif /* LIBCORTEX_FORGE_H */
