// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * Gets round-robin time quantum by calling sched_rr_get_interval() and
 * checks that the value is sane.
 */

#include <sched.h>
#include "tst_timer.h"

struct tst_ts tp;

static struct test_variants {
	int (*func)(pid_t pid, void *ts);
	enum tst_ts_type type;
	char *desc;
} variants[] = {
	{ .func = libc_sched_rr_get_interval, .type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_sched_rr_get_interval != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_sched_rr_get_interval, .type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_sched_rr_get_interval_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .func = sys_sched_rr_get_interval64, .type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];
	struct sched_param p = { 1 };

	tst_res(TINFO, "Testing variant: %s", tv->desc);

	tp.type = tv->type;

	if ((sched_setscheduler(0, SCHED_RR, &p)) == -1)
		tst_res(TFAIL | TTERRNO, "sched_setscheduler() failed");
}

static void run(void)
{
	struct test_variants *tv = &variants[tst_variant];

	TEST(tv->func(0, tst_ts_get(&tp)));

	if (!TST_RET) {
		tst_res(TPASS, "sched_rr_get_interval() passed");
	} else {
		tst_res(TFAIL | TTERRNO, "Test Failed, sched_rr_get_interval() returned %ld",
			TST_RET);
	}

	if (!tst_ts_valid(&tp)) {
		tst_res(TPASS, "Time quantum %llis %llins",
		        tst_ts_get_sec(tp), tst_ts_get_nsec(tp));
	} else {
		tst_res(TFAIL, "Invalid time quantum %llis %llins",
		        tst_ts_get_sec(tp), tst_ts_get_nsec(tp));
	}

}

static struct tst_test test = {
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
};
