// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex-forge_test.c - Test suite for Cortex Forge driver
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * Pass/fail test suite exercising all ioctls through the userspace library.
 * Exits 0 on success, 1 on any failure.
 */

#include "libcortex-forge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_START(name) do { \
	printf("  TEST: %s ... ", name); \
	fflush(stdout); \
} while (0)

#define TEST_PASS() do { \
	printf("PASS\n"); \
	tests_passed++; \
} while (0)

#define TEST_FAIL(msg) do { \
	printf("FAIL: %s\n", msg); \
	tests_failed++; \
} while (0)

#define TEST_SKIP() do { \
	printf("SKIP (driver not loaded)\n"); \
} while (0)

/* ── Test: Open/close ────────────────────────────────────────────────────── */

static void test_open_close(void)
{
	TEST_START("open/close");
	struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		TEST_SKIP();
		return;
	}
	cortex_forge_close(h);
	TEST_PASS();
}

/* ── Test: Submit and query task ──────────────────────────────────────────── */

static void test_submit_query_task(void)
{
	TEST_START("submit and query task");
	struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		TEST_SKIP();
		return;
	}

	struct cortex_forge_task_desc desc;
	memset(&desc, 0, sizeof(desc));
	desc.accel_type = CORTEX_FORGE_ACCEL_DLA0;
	desc.priority = 128;
	desc.input_size = 1024;
	desc.output_size = 1024;
	desc.input_addr = (const void *)0xDEADBEEF;
	desc.output_addr = (void *)0xCAFEBABE;

	int ret = cortex_forge_submit_task(h, &desc);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("submit failed");
		return;
	}

	struct cortex_forge_task_status status;
	memset(&status, 0, sizeof(status));
	status.task_id = desc.task_id;

	ret = cortex_forge_query_task(h, &status);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("query failed");
		return;
	}

	cortex_forge_close(h);
	TEST_PASS();
}

/* ── Test: Cancel task ───────────────────────────────────────────────────── */

static void test_cancel_task(void)
{
	TEST_START("cancel task");
	struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		TEST_SKIP();
		return;
	}

	struct cortex_forge_task_desc desc;
	memset(&desc, 0, sizeof(desc));
	desc.accel_type = CORTEX_FORGE_ACCEL_DLA0;

	int ret = cortex_forge_submit_task(h, &desc);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("submit failed");
		return;
	}

	ret = cortex_forge_cancel_task(h, desc.task_id);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("cancel failed");
		return;
	}

	cortex_forge_close(h);
	TEST_PASS();
}

/* ── Test: Get accelerator info ──────────────────────────────────────────── */

static void test_get_accel_info(void)
{
	TEST_START("get accelerator info");
	struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		TEST_SKIP();
		return;
	}

	struct cortex_forge_accel_info info;
	memset(&info, 0, sizeof(info));
	info.accel_type = CORTEX_FORGE_ACCEL_DLA0;

	int ret = cortex_forge_get_accel_info(h, &info);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("get_accel_info failed");
		return;
	}

	cortex_forge_close(h);
	TEST_PASS();
}

/* ── Test: Get version ───────────────────────────────────────────────────── */

static void test_get_version(void)
{
	TEST_START("get version");
	struct cortex_forge_handle *h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		TEST_SKIP();
		return;
	}

	uint32_t version;
	int ret = cortex_forge_get_version(h, &version);
	if (ret != 0) {
		cortex_forge_close(h);
		TEST_FAIL("get_version failed");
		return;
	}

	cortex_forge_close(h);
	TEST_PASS();
}

/* ── Test: Invalid arguments ─────────────────────────────────────────────── */

static void test_invalid_args(void)
{
	TEST_START("invalid arguments (NULL handle)");
	int ret = cortex_forge_submit_task(NULL, NULL);
	if (ret == -EINVAL)
		TEST_PASS();
	else
		TEST_FAIL("expected -EINVAL");
}

/* ── Main ────────────────────────────────────────────────────────────────── */

int main(void)
{
	printf("Cortex Forge Driver Test Suite\n");
	printf("==============================\n\n");

	test_open_close();
	test_submit_query_task();
	test_cancel_task();
	test_get_accel_info();
	test_get_version();
	test_invalid_args();

	printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
	return tests_failed > 0 ? 1 : 0;
}
