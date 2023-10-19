// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */
/*\
 * [Description]
 *
 * This test spawns multiple threads, then check for each one of them if the
 * parent ID is different AND if the thread ID is different from all the other
 * spwaned threads.
 */

#include "tst_test.h"
#include "lapi/syscalls.h"
#include "tst_safe_pthread.h"

#define THREADS_NUM 10

static volatile pid_t tids[THREADS_NUM];

static void *threaded(void *arg)
{
	int i = *(int *)arg;
	pid_t pid, tid;

	pid = tst_syscall(__NR_getpid);
	tid = tst_syscall(__NR_gettid);

	TST_EXP_EXPR(pid != tid,
		"parent ID (%d) differs from thread[%d] ID (%d)",
		pid, i, tid);
	tids[i] = tid;
	return NULL;
}

static void run(void)
{
	pthread_t thread[THREADS_NUM];
	int args[THREADS_NUM];
	int error = 0;

	for (int i = 0; i < THREADS_NUM; i++) {
		args[i] = i;
		SAFE_PTHREAD_CREATE(&thread[i], NULL, threaded, &args[i]);
	}
	for (int i = 0; i < THREADS_NUM; i++)
		SAFE_PTHREAD_JOIN(thread[i], NULL);

	for (int i = 0; i < THREADS_NUM; i++) {
		for (int j = i + 1; j < THREADS_NUM; j++) {
			if (tids[i] == tids[j]) {
				tst_res(TINFO, "thread[%d] and thread[%d] have the same ID %d", i, j, tids[i]);
				error = 1;
			}
		}
	}

	if (error)
		tst_res(TFAIL, "Some threads have the same TID");
	else
		tst_res(TPASS, "All threads have a different TID");
}

static struct tst_test test = {
	.test_all = run,
};
