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
#include "time64_variants.h"
#include "tst_timer.h"

static struct tst_its new_set, old_set;
static struct tst_its *pnew_set = &new_set, *pold_set = &old_set, *null_set;
static void *faulty_set;
static kernel_timer_t timer;
static kernel_timer_t timer_inval = (kernel_timer_t)-1;
static volatile int caught_sig;

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
	struct tst_its		**new_ptr;
	struct tst_its		**old_ptr;
	int			it_value_tv_nsec;
	int			error;
} tcases[] = {
	{&timer, &null_set, &pold_set, 0, EINVAL},
	{&timer, &pnew_set, &pold_set, -1, EINVAL},
	{&timer, &pnew_set, &pold_set, NSEC_PER_SEC + 1, EINVAL},
	{&timer_inval, &pnew_set, &pold_set, 0, EINVAL},
	{&timer, (struct tst_its **)&faulty_set, &pold_set, 0, EFAULT},
	{&timer, &pnew_set, (struct tst_its **)&faulty_set, 0, EFAULT},
};

static struct time64_variants variants[] = {
#if (__NR_timer_settime != __LTP__NR_INVALID_SYSCALL)
	{ .timer_settime = sys_timer_settime, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_timer_settime64 != __LTP__NR_INVALID_SYSCALL)
	{ .timer_settime = sys_timer_settime64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void sighandler(int sig)
{
	caught_sig = sig;
}

static void setup(void)
{
	tst_res(TINFO, "Testing variant: %s", variants[tst_variant].desc);
	faulty_set = tst_get_bad_addr(NULL);
	signal(SIGALRM, sighandler);
}

static void run(unsigned int n)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct testcase *tc = &tcases[n];
	void *new, *old;
	unsigned int i;

	tst_res(TINFO, "Testing for %s:", descriptions[n]);

	for (i = 0; i < CLOCKS_DEFINED; ++i) {
		clock_t clock = clock_list[i];

		caught_sig = 0;

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

		new_set.type = old_set.type = tv->ts_type;
		tst_its_set_interval_sec(&new_set, 0);
		tst_its_set_interval_nsec(&new_set, 0);
		tst_its_set_value_sec(&new_set, 5);
		tst_its_set_value_nsec(&new_set, tc->it_value_tv_nsec);

		new = (tc->new_ptr == (struct tst_its **)&faulty_set) ? faulty_set : tst_its_get(*tc->new_ptr);
		old = (tc->old_ptr == (struct tst_its **)&faulty_set) ? faulty_set : tst_its_get(*tc->old_ptr);

		TEST(tv->timer_settime(*tc->timer_id, 0, new, old));

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

		if (caught_sig) {
			tst_res(TFAIL,
				"Caught unexpected signal %s while testing %s",
				tst_strsig(caught_sig), get_clock_str(clock));
		}
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
		{}
	}
};
