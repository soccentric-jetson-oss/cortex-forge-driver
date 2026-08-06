// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex_forge_sysfs.c - sysfs attributes for Cortex Forge driver
 *
 * Copyright (c) 2026 SoC Centric LLC
 *
 * Author: Sandesh Ghimire
 *
 * Exposes accelerator status and configuration via sysfs. One value per file,
 * human-readable, with documented units.
 */

#include <linux/device.h>
#include <linux/stat.h>
#include <linux/sysfs.h>

#include "cortex_forge_platform.h"
#include "cortex_forge_uapi.h"
#include "cortex_forge_dev.h"

#define DRV_NAME "cortex-forge"

/* ── Accelerator attributes ──────────────────────────────────────────────── */

static ssize_t dla0_freq_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	struct cortex_forge_dev *cdev = dev_get_drvdata(dev);
	/* TODO(HW): Read actual DLA0 frequency from hardware */
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla0_freq);

static ssize_t dla0_temp_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	/* TODO(HW): Read actual DLA0 temperature sensor */
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla0_temp);

static ssize_t dla0_load_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	/* TODO(HW): Read actual DLA0 load percentage */
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla0_load);

static ssize_t dla1_freq_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla1_freq);

static ssize_t dla1_temp_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla1_temp);

static ssize_t dla1_load_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(dla1_load);

static ssize_t pva0_freq_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(pva0_freq);

static ssize_t pva0_temp_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(pva0_temp);

static ssize_t pva0_load_show(struct device *dev, struct device_attribute *attr,
			       char *buf)
{
	return sysfs_emit(buf, "0\n");
}
static DEVICE_ATTR_RO(pva0_load);

static ssize_t version_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	return sysfs_emit(buf, DRV_VERSION "\n");
}
static DEVICE_ATTR_RO(version);

/* ── Attribute groups ────────────────────────────────────────────────────── */

static struct attribute *cortex_forge_attrs[] = {
	&dev_attr_dla0_freq.attr,
	&dev_attr_dla0_temp.attr,
	&dev_attr_dla0_load.attr,
	&dev_attr_dla1_freq.attr,
	&dev_attr_dla1_temp.attr,
	&dev_attr_dla1_load.attr,
	&dev_attr_pva0_freq.attr,
	&dev_attr_pva0_temp.attr,
	&dev_attr_pva0_load.attr,
	&dev_attr_version.attr,
	NULL,
};

ATTRIBUTE_GROUPS(cortex_forge);
