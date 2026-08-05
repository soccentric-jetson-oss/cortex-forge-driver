// SPDX-License-Identifier: GPL-2.0-only
/*
 * libcortex_forge.c - Userspace library implementation
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * Implements the public API by wrapping driver ioctls via /dev node.
 * Thread-safe: each handle has its own fd and mutex.
 */

#include "libcortex-forge.h"

#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/ioctl.h>

/* ── Kernel UAPI definitions (duplicated for userspace) ──────────────────── */

#define CORTEX_FORGE_MAGIC 0xCF

#define CORTEX_FORGE_IOCTL_SUBMIT_TASK    _IOWR(CORTEX_FORGE_MAGIC, 1, struct cortex_forge_task_desc)
#define CORTEX_FORGE_IOCTL_QUERY_TASK     _IOWR(CORTEX_FORGE_MAGIC, 2, struct cortex_forge_task_status)
#define CORTEX_FORGE_IOCTL_CANCEL_TASK    _IOW(CORTEX_FORGE_MAGIC, 3, uint32_t)
#define CORTEX_FORGE_IOCTL_GET_ACCEL_INFO _IOR(CORTEX_FORGE_MAGIC, 4, struct cortex_forge_accel_info)
#define CORTEX_FORGE_IOCTL_GET_VERSION    _IOR(CORTEX_FORGE_MAGIC, 6, uint32_t)

/* ── Handle structure ────────────────────────────────────────────────────── */

struct cortex_forge_handle {
	int fd;
	pthread_mutex_t lock;
};

/* ── Lifecycle ───────────────────────────────────────────────────────────── */

struct cortex_forge_handle *cortex_forge_open(const char *device_path)
{
	struct cortex_forge_handle *h;
	int fd;

	if (!device_path) {
		errno = EINVAL;
		return NULL;
	}

	fd = open(device_path, O_RDWR);
	if (fd < 0)
		return NULL;

	h = calloc(1, sizeof(*h));
	if (!h) {
		close(fd);
		return NULL;
	}

	h->fd = fd;
	pthread_mutex_init(&h->lock, NULL);

	return h;
}

void cortex_forge_close(struct cortex_forge_handle *h)
{
	if (!h)
		return;

	pthread_mutex_destroy(&h->lock);
	close(h->fd);
	free(h);
}

/* ── Task operations ─────────────────────────────────────────────────────── */

int cortex_forge_submit_task(struct cortex_forge_handle *h,
			      struct cortex_forge_task_desc *desc)
{
	int ret;

	if (!h || !desc)
		return -EINVAL;

	pthread_mutex_lock(&h->lock);
	ret = ioctl(h->fd, CORTEX_FORGE_IOCTL_SUBMIT_TASK, desc);
	if (ret < 0)
		ret = -errno;
	pthread_mutex_unlock(&h->lock);

	return ret;
}

int cortex_forge_query_task(struct cortex_forge_handle *h,
			     struct cortex_forge_task_status *status)
{
	int ret;

	if (!h || !status)
		return -EINVAL;

	pthread_mutex_lock(&h->lock);
	ret = ioctl(h->fd, CORTEX_FORGE_IOCTL_QUERY_TASK, status);
	if (ret < 0)
		ret = -errno;
	pthread_mutex_unlock(&h->lock);

	return ret;
}

int cortex_forge_cancel_task(struct cortex_forge_handle *h, uint32_t task_id)
{
	int ret;

	if (!h)
		return -EINVAL;

	pthread_mutex_lock(&h->lock);
	ret = ioctl(h->fd, CORTEX_FORGE_IOCTL_CANCEL_TASK, &task_id);
	if (ret < 0)
		ret = -errno;
	pthread_mutex_unlock(&h->lock);

	return ret;
}

/* ── Accelerator information ─────────────────────────────────────────────── */

int cortex_forge_get_accel_info(struct cortex_forge_handle *h,
				 struct cortex_forge_accel_info *info)
{
	int ret;

	if (!h || !info)
		return -EINVAL;

	pthread_mutex_lock(&h->lock);
	ret = ioctl(h->fd, CORTEX_FORGE_IOCTL_GET_ACCEL_INFO, info);
	if (ret < 0)
		ret = -errno;
	pthread_mutex_unlock(&h->lock);

	return ret;
}

int cortex_forge_get_version(struct cortex_forge_handle *h, uint32_t *version)
{
	int ret;

	if (!h || !version)
		return -EINVAL;

	pthread_mutex_lock(&h->lock);
	ret = ioctl(h->fd, CORTEX_FORGE_IOCTL_GET_VERSION, version);
	if (ret < 0)
		ret = -errno;
	pthread_mutex_unlock(&h->lock);

	return ret;
}

/* ── Error handling ──────────────────────────────────────────────────────── */

const char *cortex_forge_strerror(int errnum)
{
	switch (errnum) {
	case 0:          return "Success";
	case -EINVAL:    return "Invalid argument";
	case -EFAULT:    return "Bad address";
	case -ENOMEM:    return "Out of memory";
	case -ENOENT:    return "Task not found";
	case -ENOTTY:    return "Invalid ioctl";
	case -EBUSY:     return "Device or resource busy";
	case -ETIMEDOUT: return "Operation timed out";
	default:         return strerror(-errnum);
	}
}
