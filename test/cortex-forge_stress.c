// SPDX-License-Identifier: GPL-2.0-only
/*
 * cortex-forge_stress.c - Multi-threaded stress test for Cortex Forge driver
 *
 * Copyright (C) 2026 SoC Centric
 *
 * Author: Sandesh <sandesh@soccentric.com>
 *
 * Spawns N threads that hammer the driver with concurrent task submissions,
 * queries, and cancellations. Designed to be run under lockdep, KASAN, and
 * KCSAN to detect races.
 */

#include "libcortex-forge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 4
#define OPS_PER_THREAD 100

static pthread_barrier_t g_barrier;

static void *worker_thread(void *arg)
{
	int thread_id = *(int *)arg;
	struct cortex_forge_handle *h;
	int i;

	h = cortex_forge_open("/dev/cortex-forge0");
	if (!h) {
		printf("Thread %d: cannot open device (driver may not be loaded)\n", thread_id);
		return NULL;
	}

	/* Wait for all threads to be ready */
	pthread_barrier_wait(&g_barrier);

	for (i = 0; i < OPS_PER_THREAD; i++) {
		struct cortex_forge_task_desc desc;
		memset(&desc, 0, sizeof(desc));
		desc.accel_type = rand() % 3;
		desc.priority = rand() % 256;
		desc.input_size = 1024;
		desc.output_size = 1024;

		/* Submit */
		int ret = cortex_forge_submit_task(h, &desc);
		if (ret == 0) {
			/* Query */
			struct cortex_forge_task_status status;
			memset(&status, 0, sizeof(status));
			status.task_id = desc.task_id;
			cortex_forge_query_task(h, &status);

			/* Cancel half of them */
			if (rand() % 2 == 0)
				cortex_forge_cancel_task(h, desc.task_id);
		}
	}

	cortex_forge_close(h);
	return NULL;
}

int main(int argc, char **argv)
{
	pthread_t threads[NUM_THREADS];
	int thread_ids[NUM_THREADS];
	int i;

	printf("Cortex Forge Stress Test\n");
	printf("Threads: %d, Ops/thread: %d\n", NUM_THREADS, OPS_PER_THREAD);

	srand((unsigned int)time(NULL));
	pthread_barrier_init(&g_barrier, NULL, NUM_THREADS);

	for (i = 0; i < NUM_THREADS; i++) {
		thread_ids[i] = i;
		pthread_create(&threads[i], NULL, worker_thread, &thread_ids[i]);
	}

	for (i = 0; i < NUM_THREADS; i++)
		pthread_join(threads[i], NULL);

	pthread_barrier_destroy(&g_barrier);
	printf("Stress test complete.\n");

	return 0;
}
