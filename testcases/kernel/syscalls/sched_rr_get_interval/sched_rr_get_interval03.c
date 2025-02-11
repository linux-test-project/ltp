// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 *    AUTHOR		: Saji Kumar.V.R <saji.kumar@wipro.com>
 */
/*\
 * Verify that:
 *
 * - sched_rr_get_interval() fails with errno set to EINVAL for an
 *   invalid pid
 *
 * - sched_rr_get_interval() fails with errno set to ESRCH if the
 *   process with specified pid does not exists
 *
 * - sched_rr_get_interval() fails with errno set to EFAULT if the
 *   address specified as &tp is invalid
 */

#include "time64_variants.h"
#include "tst_timer.h"
#include "tst_sched.h"

static pid_t unused_pid;
static pid_t inval_pid = -1;
static pid_t zero_pid;

static struct tst_ts tp;
static void *bad_addr;

struct test_cases_t {
	pid_t *pid;
	struct tst_ts *tp;
	int exp_errno;
} test_cases[] = {
	{ &inval_pid, &tp, EINVAL},
	{ &unused_pid, &tp, ESRCH},
	{ &zero_pid, NULL, EFAULT}
};

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

	bad_addr = tst_get_bad_addr(NULL);
	tp.type = tv->ts_type;

	if ((sys_sched_setscheduler(0, SCHED_RR, &p)) == -1)
		tst_res(TFAIL | TERRNO, "sched_setscheduler() failed");

	unused_pid = tst_get_unused_pid();
}

static void run(unsigned int i)
{
	struct time64_variants *tv = &variants[tst_variant];
	struct test_cases_t *tc = &test_cases[i];
	struct timerspec *ts;

	if (tc->exp_errno == EFAULT
		&& tv->sched_rr_get_interval == libc_sched_rr_get_interval) {
		tst_res(TCONF, "EFAULT skipped for libc_variant");
		return;
	}

	if (tc->exp_errno == EFAULT)
		ts = bad_addr;
	else
		ts = tst_ts_get(tc->tp);

	TST_EXP_FAIL(tv->sched_rr_get_interval(*tc->pid, ts), tc->exp_errno,
	             "sched_rr_get_interval(%i, %p)", *tc->pid, ts);
}

static struct tst_test test = {
	.test = run,
	.tcnt = ARRAY_SIZE(test_cases),
	.test_variants = ARRAY_SIZE(variants),
	.setup = setup,
	.needs_root = 1,
};
