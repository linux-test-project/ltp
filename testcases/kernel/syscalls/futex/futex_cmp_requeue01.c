// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Xiao Yang <ice_yangxiao@163.com>
 *
 * Description:
 * Testcase to check the basic functionality of futex(FUTEX_CMP_REQUEUE).
 * futex(FUTEX_CMP_REQUEUE) can wake up the number of waiters specified
 * by val argument and then requeue the number of waiters limited by val2
 * argument(i.e. move some remaining waiters from uaddr to uaddr2 address).
 */

#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <linux/futex.h>
#include <sys/time.h>

#include "tst_timer_test.h"
#include "tst_test.h"
#include "futextest.h"

static futex_t *futexes;

static struct tcase {
	int num_waiters;
	int set_wakes;
	int set_requeues;
} tcases[] = {
	{10, 3, 7},
	{10, 0, 10},
	{10, 2, 6},
	{100, 50, 50},
	{100, 0, 70},
	{1000, 100, 900},
	{1000, 300, 500},
};

static void do_child(void)
{
	struct timespec usec = tst_ms_to_timespec(2000);
	int pid = getpid();

	if (!futex_wait(&futexes[0], futexes[0], &usec, 0))
		exit(0);

	tst_res(TINFO | TERRNO, "process %d wasn't woken up", pid);
	exit(1);
}

static void verify_futex_cmp_requeue(unsigned int n)
{
	int num_requeues = 0, num_waits = 0, num_total = 0;
	int i, status;
	struct tcase *tc = &tcases[n];
	int pid[tc->num_waiters];
	int exp_ret = tc->set_wakes + tc->set_requeues;

	for (i = 0; i < tc->num_waiters; i++) {
		pid[i] = SAFE_FORK();
		if (!pid[i])
			do_child();
	}

	for (i = 0; i < tc->num_waiters; i++)
		TST_PROCESS_STATE_WAIT(pid[i], 'S');

	TEST(futex_cmp_requeue(&futexes[0], futexes[0], &futexes[1],
	     tc->set_wakes, tc->set_requeues, 0));
	if (TST_RET != exp_ret) {
		tst_res(TFAIL, "futex_cmp_requeue() returned %ld, expected %d",
			TST_RET, exp_ret);
	}

	num_requeues = futex_wake(&futexes[1], tc->num_waiters, 0);
	num_waits = futex_wake(&futexes[0], tc->num_waiters, 0);

	for (i = 0; i < tc->num_waiters; i++) {
		SAFE_WAITPID(pid[i], &status, 0);
		if (WIFEXITED(status) && !WEXITSTATUS(status))
			num_total++;
	}

	if (num_total != tc->num_waiters) {
		tst_res(TFAIL, "%d waiters were not woken up normally",
			tc->num_waiters - num_total);
		return;
	}

	if (num_requeues != tc->set_requeues) {
		tst_res(TFAIL,
			"futex_cmp_requeue() requeues %d waiters, expected %d",
			num_requeues, tc->set_requeues);
		return;
	}

	if (tc->num_waiters - num_requeues - num_waits != tc->set_wakes) {
		tst_res(TFAIL,
			"futex_cmp_requeue() woke up %d waiters, expected %d",
			tc->num_waiters - num_requeues - num_waits,
			tc->set_wakes);
		return;
	}

	tst_res(TPASS,
		"futex_cmp_requeue() woke up %d waiters and requeued %d waiters",
		tc->set_wakes, tc->set_requeues);
}

static void setup(void)
{
	futexes = SAFE_MMAP(NULL, sizeof(futex_t) * 2, PROT_READ | PROT_WRITE,
			    MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	futexes[0] = FUTEX_INITIALIZER;
	futexes[1] = FUTEX_INITIALIZER + 1;
}

static void cleanup(void)
{
	if (futexes)
		SAFE_MUNMAP((void *)futexes, sizeof(futex_t) * 2);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_futex_cmp_requeue,
	.forks_child = 1,
};
