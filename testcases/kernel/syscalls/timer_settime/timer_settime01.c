// SPDX-License-Identifier: GPL-2.0-only
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
 * 1) General initialization: No old_value, no flags
 * 2) Setting a pointer to a itimerspec struct as old_set parameter
 * 3) Using a periodic timer
 * 4) Using absolute time
 *
 * All of these tests are supposed to be successful.
 *
 * This is also regression test for commit:
 * f18ddc13af98 ("alarmtimer: Use EOPNOTSUPP instead of ENOTSUPP")
 * e86fea764991 ("alarmtimer: Return relative times in timer_gettime")
 */

#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "time64_variants.h"
#include "tst_timer.h"

static struct tst_ts timenow;
static struct tst_its new_set, old_set;
static kernel_timer_t timer;

static struct testcase {
	struct tst_its		*old_ptr;
	int			it_value_tv_usec;
	int			it_interval_tv_usec;
	int			flag;
	char			*description;
} tcases[] = {
	{NULL, 50000, 0, 0, "general initialization"},
	{&old_set, 50000, 0, 0, "setting old_value"},
	{&old_set, 50000, 50000, 0, "using periodic timer"},
	{&old_set, 50000, 0, TIMER_ABSTIME, "using absolute time"},
};

static struct time64_variants variants[] = {
#if (__NR_timer_settime != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime, .timer_gettime = sys_timer_gettime, .timer_settime = sys_timer_settime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timer_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .clock_gettime = sys_clock_gettime64, .timer_gettime = sys_timer_gettime64, .timer_settime = sys_timer_settime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static volatile int caught_signal;

static void clear_signal(void)
{
	/*
	 * The busy loop is intentional. The signal is sent after X
	 * seconds of CPU time has been accumulated for the process and
	 * thread specific clocks.
	 */
	while (!caught_signal);

	if (caught_signal != SIGALRM) {
		tst_res(TFAIL, "Received incorrect signal: %s",
			tst_strsig(caught_signal));
	}

	caught_signal = 0;
}

static void sighandler(int sig)
{
	caught_signal = sig;
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static void run(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct testcase *tc = &tcases[n];
	long long val;
	unsigned int i;

	tst_res(TINFO, "Testing for %s:", tc->description);

	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

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

		new_set.type = old_set.type = tv->ts_type;
		val = tc->it_value_tv_usec;

		if (tc->flag & TIMER_ABSTIME) {
			timenow.type = tv->ts_type;
			if (tv->clock_gettime(clock, tst_ts_get(&timenow)) < 0) {
				tst_res(TFAIL,
					"clock_gettime(%s) failed - skipping the test",
					get_clock_str(clock));
				continue;
			}
			tst_ts_add_us(timenow, val);
			tst_its_set_value_from_ts(&new_set, timenow);
		} else {
			tst_its_set_value_from_us(&new_set, val);
		}

		tst_its_set_interval_from_us(&new_set, tc->it_interval_tv_usec);

		TEST(tv->timer_settime(timer, tc->flag, tst_its_get(&new_set), tst_its_get(tc->old_ptr)));

		if (TST_RET != 0) {
			tst_res(TFAIL | TTERRNO, "timer_settime(%s) failed",
				get_clock_str(clock));
		}

		TEST(tv->timer_gettime(timer, tst_its_get(&new_set)));
		if (TST_RET != 0) {
			tst_res(TFAIL | TTERRNO, "timer_gettime(%s) failed",
				get_clock_str(clock));
		} else if ((tst_its_get_interval_nsec(new_set) !=
				tc->it_interval_tv_usec * 1000) ||
			   (tst_its_get_value_nsec(new_set) >
				MAX(tc->it_value_tv_usec * 1000, tc->it_interval_tv_usec * 1000))) {
			tst_res(TFAIL | TTERRNO,
				"timer_gettime(%s) reported bad values (%llu: %llu)",
				get_clock_str(clock),
				tst_its_get_interval_nsec(new_set),
				tst_its_get_value_nsec(new_set));
		}

		clear_signal();

		/* Wait for another event when interval was set */
		if (tc->it_interval_tv_usec)
			clear_signal();

		tst_res(TPASS, "timer_settime(%s) passed",
			get_clock_str(clock));

		TEST(tst_syscall(__NR_timer_delete, timer));
		if (TST_RET != 0)
			tst_res(TFAIL | TTERRNO, "timer_delete() failed!");
	}
}

static struct tst_test test = {
	.test = run,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f18ddc13af98"},
		{"linux-git", "e86fea764991"},
		{}
	}
};
