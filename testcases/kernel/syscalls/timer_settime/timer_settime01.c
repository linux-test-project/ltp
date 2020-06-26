// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) Wipro Technologies Ltd, 2003.  All Rights Reserved.
 *
 * Author:	Aniruddha Marathe <aniruddha.marathe@wipro.com>
 *
 * Ported to new library:
 * 07/2019      Christian Amann <camann@suse.com>
 */
/*
 * This tests the timer_settime(2) syscall under various conditions:
 *
 * 1) General initialization: No old_value, no flags, 5-second-timer
 * 2) Setting a pointer to a itimerspec struct as old_set parameter
 * 3) Using a periodic timer
 * 4) Using absolute time
 *
 * All of these tests are supposed to be successful.
 *
 * This is also regression test for commit:
 * f18ddc13af98 ("alarmtimer: Use EOPNOTSUPP instead of ENOTSUPP")
 */

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "tst_timer.h"

static struct tst_ts timenow;
static struct tst_its new_set, old_set;
static timer_t timer;

static struct testcase {
	struct tst_its		*old_ptr;
	int			it_value_tv_sec;
	int			it_interval_tv_sec;
	int			flag;
	char			*description;
} tcases[] = {
	{NULL,     5, 0, 0, "general initialization"},
	{&old_set, 5, 0, 0, "setting old_value"},
	{&old_set, 0, 5, 0, "using periodic timer"},
	{&old_set, 5, 0, TIMER_ABSTIME, "using absolute time"},
};

static struct test_variants {
	int (*gettime)(clockid_t clk_id, void *ts);
	int (*func)(timer_t timerid, int flags, void *its,
		    void *old_its);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
#if (__NR_timer_settime != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime, .func = sys_timer_settime, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timer_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .gettime = sys_clock_gettime64, .func = sys_timer_settime64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void run(unsigned int n)
{
	struct test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &tcases[n];
	long long val;
	unsigned int i;

	tst_res(TINFO, "Testing for %s:", tc->description);

	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

		if (clock == CLOCK_PROCESS_CPUTIME_ID ||
			clock == CLOCK_THREAD_CPUTIME_ID) {
			if (!have_cputime_timers())
				continue;
		}

		TEST(tst_syscall(__NR_timer_create, clock, NULL, &timer));
		if (TST_RET != 0) {
			if (possibly_unsupported(clock) &&
				(TST_ERR == EINVAL || TST_ERR == ENOTSUP)) {
				tst_res(TCONF | TTERRNO, "%s unsupported",
					get_clock_str(clock));
			} else {
				tst_res(TFAIL | TTERRNO,
					"timer_create(%s) failed",
					get_clock_str(clock));
			}
			continue;
		}

		memset(&new_set, 0, sizeof(new_set));
		memset(&old_set, 0, sizeof(old_set));

		new_set.type = old_set.type = tv->type;

		val = tc->it_value_tv_sec;

		if (tc->flag & TIMER_ABSTIME) {
			timenow.type = tv->type;
			if (tv->gettime(clock, tst_ts_get(&timenow)) < 0) {
				tst_res(TFAIL,
					"clock_gettime(%s) failed - skipping the test",
					get_clock_str(clock));
				continue;
			}
			val += tst_ts_get_sec(timenow);
		}

		tst_its_set_interval_sec(&new_set, tc->it_interval_tv_sec);
		tst_its_set_interval_nsec(&new_set, 0);
		tst_its_set_value_sec(&new_set, val);
		tst_its_set_value_nsec(&new_set, 0);

		TEST(tv->func(timer, tc->flag, tst_its_get(&new_set), tst_its_get(tc->old_ptr)));

		if (TST_RET != 0) {
			tst_res(TFAIL | TTERRNO, "%s failed",
					get_clock_str(clock));
		} else {
			tst_res(TPASS, "%s was successful",
					get_clock_str(clock));
		}

		TEST(tst_syscall(__NR_timer_delete, timer));
		if (TST_RET != 0)
			tst_res(TFAIL | TTERRNO, "timer_delete() failed!");
	}
}

static void sighandler(int sig)
{
	/* sighandler for CLOCK_*_ALARM */
	tst_res(TINFO, "Caught signal %s", tst_strsig(sig));
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static struct tst_test test = {
	.test = run,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f18ddc13af98"},
		{}
	}
};
