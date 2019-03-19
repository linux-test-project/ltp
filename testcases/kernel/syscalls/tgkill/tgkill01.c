// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Google, Inc.
 *
 * tgkill() delivers a signal to a specific thread.  Test this by installing
 * a SIGUSR1 handler which records the current pthread ID.  Start a number
 * of threads in parallel, then one-by-one call tgkill(..., tid, SIGUSR1)
 * and check that the expected pthread ID was recorded.
 */

#include <pthread.h>
#include <stdlib.h>

#include "tst_safe_pthread.h"
#include "tst_test.h"
#include "tgkill.h"

struct thread_state {
	pthread_t thread;
	pid_t tid;
};

static char *str_threads;
static int n_threads = 10;
static struct thread_state *threads;

static pthread_t sigusr1_thread;

static void sigusr1_handler(int signum __attribute__((unused)))
{
	sigusr1_thread = pthread_self();
}

static void *thread_func(void *arg)
{
	struct thread_state *thread = arg;

	/**
	 * There is no standard way to map pthread -> tid, so we will have the
	 * child stash its own tid then notify the parent that the stashed tid
	 * is available.
	 */
	thread->tid = sys_gettid();

	TST_CHECKPOINT_WAKE(0);

	TST_CHECKPOINT_WAIT(1);

	return arg;
}

static void start_thread(struct thread_state *thread)
{
	SAFE_PTHREAD_CREATE(&thread->thread, NULL, thread_func, thread);

	TST_CHECKPOINT_WAIT(0);
}

static void stop_threads(void)
{
	int i;

	TST_CHECKPOINT_WAKE2(1, n_threads);

	for (i = 0; i < n_threads; i++) {
		if (threads[i].tid == -1)
			continue;

		SAFE_PTHREAD_JOIN(threads[i].thread, NULL);
		threads[i].tid = -1;
	}

	if (threads)
		free(threads);
}

static void run(void)
{
	int i;

	for (i = 0; i < n_threads; i++) {
		sigusr1_thread = pthread_self();

		TEST(sys_tgkill(getpid(), threads[i].tid, SIGUSR1));
		if (TST_RET) {
			tst_res(TFAIL | TTERRNO, "tgkill() failed");
			return;
		}

		while (pthread_equal(sigusr1_thread, pthread_self()))
			usleep(1000);

		if (!pthread_equal(sigusr1_thread, threads[i].thread)) {
			tst_res(TFAIL, "SIGUSR1 delivered to wrong thread");
			return;
		}
	}

	tst_res(TPASS, "SIGUSR1 delivered to correct threads");
}

static void setup(void)
{
	int i;

	if (tst_parse_int(str_threads, &n_threads, 1, INT_MAX))
		tst_brk(TBROK, "Invalid number of threads '%s'", str_threads);

	threads = SAFE_MALLOC(sizeof(*threads) * n_threads);

	struct sigaction sigusr1 = {
		.sa_handler = sigusr1_handler,
	};
	SAFE_SIGACTION(SIGUSR1, &sigusr1, NULL);

	for (i = 0; i < n_threads; i++)
		threads[i].tid = -1;

	for (i = 0; i < n_threads; i++)
		start_thread(&threads[i]);
}

static struct tst_option options[] = {
	{"t:", &str_threads, "-t       Number of threads (default 10)"},
	{NULL, NULL, NULL},
};

static struct tst_test test = {
	.options = options,
	.needs_checkpoints = 1,
	.setup = setup,
	.test_all = run,
	.cleanup = stop_threads,
};
