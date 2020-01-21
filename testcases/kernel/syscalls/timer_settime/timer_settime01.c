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
#include "tst_test.h"
#include "lapi/common_timers.h"

static struct timespec timenow;
static struct itimerspec new_set, old_set;
static kernel_timer_t timer;

static struct testcase {
	struct itimerspec	*old_ptr;
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

static void run(unsigned int n)
{
	unsigned int i;
	struct testcase *tc = &tcases[n];

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

		new_set.it_value.tv_sec = tc->it_value_tv_sec;
		new_set.it_interval.tv_sec = tc->it_interval_tv_sec;

		if (tc->flag & TIMER_ABSTIME) {
			if (clock_gettime(clock, &timenow) < 0) {
				tst_res(TFAIL,
					"clock_gettime(%s) failed - skipping the test",
					get_clock_str(clock));
				continue;
			}
			new_set.it_value.tv_sec += timenow.tv_sec;
		}

		TEST(tst_syscall(__NR_timer_settime, timer,
			tc->flag, &new_set, tc->old_ptr));

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
	SAFE_SIGNAL(SIGALRM, sighandler);
}

static struct tst_test test = {
	.test = run,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f18ddc13af98"},
		{}
	}
};
