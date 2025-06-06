// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Crackerjack Project., 2007
 * Ported to LTP by Manas Kumar Nayak <maknayak@in.ibm.com>
 * Copyright (c) 2015 Linux Test Project
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test checks if exit_group() correctly ends a spawned child and all its
 * running threads.
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_pthread.h"

static int cpu_count;

static struct worker_data {
	pid_t tid;
	tst_atomic_t counter;
} *workers_data;

static void *worker(void *arg)
{
	struct worker_data *data;

	data = (struct worker_data *)arg;
	data->tid = tst_gettid();

	while (1) {
		tst_atomic_inc(&data->counter);
		sched_yield();
	}

	return arg;
}

static void spawn_threads(void)
{
	pthread_t threads[cpu_count];

	for (int i = 0; i < cpu_count; i++)
		SAFE_PTHREAD_CREATE(&threads[i], NULL, worker, (void *)(workers_data + i));
}

static void check_counters(void)
{
	struct worker_data data_copy[cpu_count];

	memset(data_copy, 0, sizeof(struct worker_data) * cpu_count);
	memcpy(data_copy, workers_data, sizeof(struct worker_data) * cpu_count);

	tst_res(TINFO, "Checking if threads are still running");
	usleep(100000);

	struct worker_data *old_data;
	struct worker_data *new_data;

	for (int i = 0; i < cpu_count; i++) {
		old_data = data_copy + i;
		new_data = workers_data + i;

		if (old_data->counter != new_data->counter) {
			tst_res(TFAIL, "Counter value has changed for thread[%d]", i);
			return;
		}
	}

	tst_res(TINFO, "Threads counters value didn't change");
}

static void run(void)
{
	pid_t pid;
	int status;

	pid = SAFE_FORK();
	if (!pid) {
		spawn_threads();

		TEST(tst_syscall(__NR_exit_group, 4));
		if (TST_RET == -1)
			tst_brk(TBROK | TERRNO, "exit_group() error");

		return;
	}

	SAFE_WAITPID(pid, &status, 0);

	TST_EXP_EXPR(WIFEXITED(status) && WEXITSTATUS(status) == 4,
		"exit_group() succeeded");

	check_counters();
}

static void setup(void)
{
	cpu_count = MAX(2, tst_ncpus());

	workers_data = SAFE_MMAP(
		NULL,
		sizeof(struct worker_data) * cpu_count,
		PROT_READ | PROT_WRITE,
		MAP_SHARED | MAP_ANONYMOUS,
		-1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(workers_data, sizeof(struct worker_data) * cpu_count);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
