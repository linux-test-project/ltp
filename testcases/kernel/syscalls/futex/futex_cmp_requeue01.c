// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Xiao Yang <ice_yangxiao@163.com>
 */

/*\
 * Verify the basic functionality of futex(FUTEX_CMP_REQUEUE).
 *
 * futex(FUTEX_CMP_REQUEUE) can wake up the number of waiters specified
 * by val argument and then requeue the number of waiters limited by val2
 * argument (i.e. move some remaining waiters from uaddr to uaddr2 address).
 */

#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/time.h>

#include "tst_test.h"
#include "futextest.h"
#include "lapi/futex.h"

struct shared_data {
	futex_t futexes[2];
	int spurious;
	int test_done;
};

static struct shared_data *sd;
static int max_sleep_ms;

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

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void do_child(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct tst_ts usec = tst_ts_from_ms(tv->tstype, max_sleep_ms);
	int slept_for_ms = 0;
	int pid = getpid();
	int ret = 0;

	if (futex_wait(tv->fntype, &sd->futexes[0], sd->futexes[0], &usec, 0) == -1) {
		if (errno == EAGAIN) {
			/* spurious wakeup or signal */
			tst_atomic_inc(&sd->spurious);
		} else {
			tst_res(TFAIL | TERRNO, "process %d wasn't woken up",
				pid);
			ret = 1;
		}
	}

	/* make sure TST_PROCESS_STATE_WAIT() can always succeed */
	while (!tst_atomic_load(&sd->test_done)
		&& (slept_for_ms < max_sleep_ms)) {
		usleep(50000);
		slept_for_ms += 50;
	}

	exit(ret);
}

static void verify_futex_cmp_requeue(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	int num_requeues = 0, num_waits = 0, num_total = 0;
	int i, status, spurious, woken_up;
	struct tcase *tc = &tcases[n];
	int pid[tc->num_waiters];
	int exp_ret = tc->set_wakes + tc->set_requeues;

	tst_atomic_store(0, &sd->spurious);
	tst_atomic_store(0, &sd->test_done);
	for (i = 0; i < tc->num_waiters; i++) {
		pid[i] = SAFE_FORK();
		if (!pid[i])
			do_child();
	}

	for (i = 0; i < tc->num_waiters; i++)
		TST_PROCESS_STATE_WAIT(pid[i], 'S', 0);

	tst_res(TINFO, "Test %d: waiters: %d, wakes: %d, requeues: %d",
		n, tc->num_waiters, tc->set_wakes, tc->set_requeues);

	/*
	 * change futex value, so any spurious wakeups or signals after
	 * this point get bounced back to userspace.
	 */
	sd->futexes[0]++;
	sd->futexes[1]++;

	/*
	 * Wakes up a maximum of tc->set_wakes waiters. tc->set_requeues
	 * specifies an upper limit on the number of waiters that are requeued.
	 * Returns the total number of waiters that were woken up or requeued.
	 */
	TEST(futex_cmp_requeue(tv->fntype, &sd->futexes[0], sd->futexes[0],
			&sd->futexes[1], tc->set_wakes, tc->set_requeues, 0));

	/* Fail if more than requested wakes + requeues were returned */
	if (TST_RET > exp_ret) {
		tst_res(TFAIL, "futex_cmp_requeue() returned %ld, expected <= %d",
			TST_RET, exp_ret);
	} else {
		tst_res(TINFO, "futex_cmp_requeue() returned %ld", TST_RET);
	}

	num_requeues = futex_wake(tv->fntype, &sd->futexes[1], tc->num_waiters, 0);
	num_waits = futex_wake(tv->fntype, &sd->futexes[0], tc->num_waiters, 0);

	tst_atomic_store(1, &sd->test_done);
	for (i = 0; i < tc->num_waiters; i++) {
		SAFE_WAITPID(pid[i], &status, 0);
		if (WIFEXITED(status) && !WEXITSTATUS(status))
			num_total++;
	}

	spurious = tst_atomic_load(&sd->spurious);
	tst_res(TINFO, "children woken, futex0: %d, futex1: %d, "
		"spurious wakeups: %d",
		num_waits, num_requeues, spurious);

	/* Fail if any waiter timed out */
	if (num_total != tc->num_waiters) {
		tst_res(TFAIL, "%d waiters were not woken up normally",
			tc->num_waiters - num_total);
		return;
	}

	/*
	 * num_requeues should be in range:
	 *     (tc->set_requeues - spurious, tc->set_requeues)
	 *
	 * Fewer processes than requested can be requeued at futex1
	 * if some woke up spuriously. Finding more processes than
	 * requested at futex1 is always a failure.
	 */
	if ((num_requeues < tc->set_requeues - spurious)
		|| (num_requeues > tc->set_requeues)) {
		tst_res(TFAIL,
			"requeued %d waiters, expected range: (%d, %d)",
			num_requeues, tc->set_requeues - spurious,
			tc->set_requeues);
		return;
	}

	/*
	 * woken_up = (TST_RET - num_requeues) should be in range:
	 *     (tc->set_wakes - spurious, tc->set_wakes + spurious)
	 *
	 * Fewer processes than requested can be woken up, if some of
	 * them woke up spuriously before requeue. More processes than
	 * requested may appear to be woken up, if some woke up
	 * spuriously after requeue.
	 */
	woken_up = TST_RET - num_requeues;
	if ((woken_up < tc->set_wakes - spurious)
		|| (woken_up > tc->set_wakes + spurious)) {
		tst_res(TFAIL,
			"woken up %d, expected range (%d, %d)",
			woken_up, tc->set_wakes - spurious,
			tc->set_wakes + spurious);
		return;
	}

	tst_res(TPASS, "futex_cmp_requeue()");
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	max_sleep_ms = tst_multiply_timeout(5000);

	sd = SAFE_MMAP(NULL, sizeof(*sd), PROT_READ | PROT_WRITE,
			    MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	sd->futexes[0] = FUTEX_INITIALIZER;
	sd->futexes[1] = FUTEX_INITIALIZER + 1000;
}

static void cleanup(void)
{
	if (sd)
		SAFE_MUNMAP((void *)sd, sizeof(*sd));
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_futex_cmp_requeue,
	.test_variants = ARRAY_SIZE(variants),
	.forks_child = 1,
};
