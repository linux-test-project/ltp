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
 * This tests basic error handling of the timer_settime(2) syscall:
 *
 * 1) Setting pointer to new settings to NULL -> EINVAL
 * 2) Setting tv_nsec of the itimerspec structure to a negative value
 *    -> EINVAL
 * 3) Setting tv_nsec of the itimerspec structure to something larger
 *    than NSEC_PER_SEC -> EINVAL
 * 4) Passing an invalid timer -> EINVAL
 * 5) Passing an invalid address for new_value -> EFAULT
 * 6) Passing an invalid address for old_value -> EFAULT
 *
 * This is also regression test for commit:
 * f18ddc13af98 ("alarmtimer: Use EOPNOTSUPP instead of ENOTSUPP")
 */

#include <errno.h>
#include <time.h>
#include "tst_test.h"
#include "lapi/common_timers.h"

static struct itimerspec new_set, old_set;
static kernel_timer_t timer;
static kernel_timer_t timer_inval = -1;

/* separate description-array to (hopefully) improve readability */
static const char * const descriptions[] = {
	"setting new_set pointer to NULL",
	"setting tv_nsec to a negative value",
	"setting tv_nsec value too high",
	"passing pointer to invalid timer_id",
	"passing invalid address for new_value",
	"passing invalid address for old_value",
};

static struct testcase {
	kernel_timer_t		*timer_id;
	struct itimerspec	*new_ptr;
	struct itimerspec	*old_ptr;
	int			it_value_tv_nsec;
	int			error;
} tcases[] = {
	{&timer, NULL, &old_set, 0, EINVAL},
	{&timer, &new_set, &old_set, -1, EINVAL},
	{&timer, &new_set, &old_set, NSEC_PER_SEC + 1, EINVAL},
	{&timer_inval, &new_set, &old_set, 0, EINVAL},
	{&timer, (struct itimerspec *) -1, &old_set, 0, EFAULT},
	{&timer, &new_set, (struct itimerspec *) -1, 0, EFAULT},
};

static void run(unsigned int n)
{
	unsigned int i;
	struct testcase *tc = &tcases[n];

	tst_res(TINFO, "Testing for %s:", descriptions[n]);

	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

		if (clock == CLOCK_PROCESS_CPUTIME_ID ||
			clock == CLOCK_THREAD_CPUTIME_ID) {
			if (!have_cputime_timers())
				continue;
		}

		/* Init temporary timer */
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

		new_set.it_value.tv_sec  = 5;
		new_set.it_value.tv_nsec = tc->it_value_tv_nsec;

		TEST(tst_syscall(__NR_timer_settime, *tc->timer_id,
					0, tc->new_ptr,	tc->old_ptr));

		if (tc->error != TST_ERR) {
			tst_res(TFAIL | TTERRNO,
				 "%s didn't fail as expected. Expected: %s - Got",
				 get_clock_str(clock),
				 tst_strerrno(tc->error));
		} else {
			tst_res(TPASS | TTERRNO,
				"%s failed as expected",
				get_clock_str(clock));
		}

		/* Delete temporary timer */
		TEST(tst_syscall(__NR_timer_delete, timer));
		if (TST_RET != 0)
			tst_res(TFAIL | TTERRNO, "timer_delete() failed!");
	}
}

static struct tst_test test = {
	.test = run,
	.needs_root = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.tags = (const struct tst_tag[]) {
		{"linux-git", "f18ddc13af98"},
		{}
	}
};
