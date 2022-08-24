// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright(c) 2022 Huawei Technologies Co., Ltd
 * Author: Li Mengfei <limengfei4@huawei.com>
 *         Zhao Gongyi <zhaogongyi@huawei.com>
 */

/*\
 * [Description]
 *
 * 1. Create a high nice thread and a low nice thread, the main
 *    thread wake them at the same time
 * 2. Both threads run on the same CPU
 * 3. Verify that the low nice thread executes more time than
 *    the high nice thread
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include "tst_test.h"
#include "tst_safe_pthread.h"
#include "lapi/syscalls.h"
#include "tst_safe_clocks.h"
#include "tst_timer.h"

static pthread_barrier_t barrier;

static void set_nice(int nice_inc)
{
	int orig_nice;

	orig_nice = SAFE_GETPRIORITY(PRIO_PROCESS, 0);

	TEST(nice(nice_inc));

	if (TST_RET != (orig_nice + nice_inc)) {
		tst_brk(TBROK | TTERRNO, "nice(%d) returned %li, expected %i",
			nice_inc, TST_RET, orig_nice + nice_inc);
	}

	if (TST_ERR)
		tst_brk(TBROK | TTERRNO, "nice(%d) failed", nice_inc);
}

static void do_something(void)
{
	volatile int number = 0;

	while (1) {
		number++;

		TEST(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL));
		if (TST_RET != 0) {
			tst_brk(TBROK | TRERRNO,
				"pthread_setcancelstate() failed");
		}
		pthread_testcancel();
	}
}

static void *thread_fn(void *arg)
{
	set_nice((intptr_t)arg);
	SAFE_PTHREAD_BARRIER_WAIT(&barrier);
	do_something();

	return NULL;
}

static void setup(void)
{
	size_t size;
	size_t i;
	int nrcpus = 1024;
	cpu_set_t *set;
	int some_cpu;

	set = CPU_ALLOC(nrcpus);
	if (!set)
		tst_brk(TBROK | TERRNO, "CPU_ALLOC()");

	size = CPU_ALLOC_SIZE(nrcpus);
	CPU_ZERO_S(size, set);
	if (sched_getaffinity(0, size, set) < 0)
		tst_brk(TBROK | TERRNO, "sched_getaffinity()");

	for (i = 0; i < size * 8; i++)
		if (CPU_ISSET_S(i, size, set))
			some_cpu = i;

	CPU_ZERO_S(size, set);
	CPU_SET_S(some_cpu, size, set);
	if (sched_setaffinity(0, size, set) < 0)
		tst_brk(TBROK | TERRNO, "sched_setaffinity()");

	CPU_FREE(set);
}

static void verify_nice(void)
{
	intptr_t nice_inc_high = -1;
	intptr_t nice_inc_low = -2;
	clockid_t nice_low_clockid, nice_high_clockid;
	struct timespec nice_high_ts, nice_low_ts;
	long long delta;
	pthread_t thread[2];

	SAFE_PTHREAD_BARRIER_INIT(&barrier, NULL, 3);

	SAFE_PTHREAD_CREATE(&thread[0], NULL, thread_fn,
			(void *)nice_inc_high);
	SAFE_PTHREAD_CREATE(&thread[1], NULL, thread_fn,
			(void *)nice_inc_low);

	SAFE_PTHREAD_BARRIER_WAIT(&barrier);

	sleep(tst_remaining_runtime());

	TEST(pthread_getcpuclockid(thread[1], &nice_low_clockid));
	if (TST_RET != 0)
		tst_brk(TBROK | TRERRNO, "clock_getcpuclockid() failed");

	TEST(pthread_getcpuclockid(thread[0], &nice_high_clockid));
	if (TST_RET != 0)
		tst_brk(TBROK | TRERRNO, "clock_getcpuclockid() failed");

	SAFE_CLOCK_GETTIME(nice_low_clockid, &nice_low_ts);
	SAFE_CLOCK_GETTIME(nice_high_clockid, &nice_high_ts);

	tst_res(TINFO, "Nice low thread CPU time: %ld.%09ld s",
			nice_low_ts.tv_sec, nice_low_ts.tv_nsec);
	tst_res(TINFO, "Nice high thread CPU time: %ld.%09ld s",
			nice_high_ts.tv_sec, nice_high_ts.tv_nsec);

	delta = tst_timespec_diff_ns(nice_low_ts, nice_high_ts);
	if (delta < 0) {
		tst_res(TFAIL, "executes less cycles than "
				"the high nice thread, delta: %lld ns", delta);
	} else {
		tst_res(TPASS, "executes more cycles than "
				"the high nice thread, delta: %lld ns", delta);
	}

	SAFE_PTHREAD_CANCEL(thread[0]);
	SAFE_PTHREAD_CANCEL(thread[1]);
	SAFE_PTHREAD_BARRIER_DESTROY(&barrier);
	SAFE_PTHREAD_JOIN(thread[0], NULL);
	SAFE_PTHREAD_JOIN(thread[1], NULL);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = verify_nice,
	.needs_root = 1,
	.forks_child = 1,
	.max_runtime = 3,
};
