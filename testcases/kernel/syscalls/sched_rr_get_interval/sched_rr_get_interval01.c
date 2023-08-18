// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 *
 * Gets round-robin time quantum by calling sched_rr_get_interval() and
 * checks that the value is sane.
 *
 * It is also a regression test for kernel
 * commit 975e155ed873 ("sched/rt: Show the 'sched_rr_timeslice' SCHED_RR
 * timeslice tuning knob in milliseconds").
 */

#include "time64_variants.h"
#include "tst_timer.h"
#include "tst_sched.h"

#define PROC_SCHED_RR_TIMESLICE_MS	"/proc/sys/kernel/sched_rr_timeslice_ms"
static int proc_flag;

struct tst_ts tp;

static struct time64_variants variants[] = {
	{ .sched_rr_get_interval = libc_sched_rr_get_interval, .ts_type = TST_LIBC_TIMESPEC, .desc = "vDSO or syscall with libc spec"},

#if (__NR_sched_rr_get_interval != __LTP__NR_INVALID_SYSCALL)
	{ .sched_rr_get_interval = sys_sched_rr_get_interval, .ts_type = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_sched_rr_get_interval_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .sched_rr_get_interval = sys_sched_rr_get_interval64, .ts_type = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static void setup(void)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct sched_param p = { 1 };

	tst_res(TINFO, "Testing variant: %s", tv->desc);

	tp.type = tv->ts_type;

	if ((sys_sched_setscheduler(0, SCHED_RR, &p)) == -1)
		tst_res(TFAIL | TERRNO, "sched_setscheduler() failed");

	proc_flag = !access(PROC_SCHED_RR_TIMESLICE_MS, F_OK);
}

static void run(void)
{
	struct time64_variants *tv = &variants[tst_variant];

	TEST(tv->sched_rr_get_interval(0, tst_ts_get(&tp)));

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

	if (proc_flag)
		TST_ASSERT_INT(PROC_SCHED_RR_TIMESLICE_MS, tst_ts_to_ms(tp));
}

static struct tst_test test = {
	.test_all = run,
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
	.tags = (const struct tst_tag[]) {
		{"linux-git", "975e155ed873"},
		{"linux-git", "c7fcb99877f9"},
		{}
	}
};
