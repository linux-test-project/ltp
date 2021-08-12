// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * [Description]
 *
 * Testcase to check sched_getscheduler() returns correct return value.
 *
 * [Algorithm]
 *
 * Call sched_setcheduler() to set the scheduling policy of the current
 * process. Then call sched_getscheduler() to ensure that this is same
 * as what set by the previous call to sched_setscheduler().
 *
 * Use SCHED_RR, SCHED_FIFO, SCHED_OTHER as the scheduling policies for
 * sched_setscheduler().
 *
 */

#include <errno.h>
#include <stdio.h>

#include "tst_test.h"
#include "tst_sched.h"

struct test_cases_t {
	int priority;
	int policy;
} tcases[] = {
	{1, SCHED_RR},
	{0, SCHED_OTHER},
	{1, SCHED_FIFO}
};

static void run(unsigned int n)
{
	struct sched_variant *tv = &sched_variants[tst_variant];
	struct test_cases_t *tc = &tcases[n];
	struct sched_param p = { .sched_priority = tc->priority };

	TST_EXP_PASS_SILENT(tv->sched_setscheduler(0, tc->policy, &p));

	if (!TST_PASS)
		return;

	TEST(tv->sched_getscheduler(0));
	if (TST_RET == tc->policy)
		tst_res(TPASS, "got expected policy %d", tc->policy);
	else
		tst_res(TFAIL | TERRNO, "got policy %ld, expected %d", TST_RET, tc->policy);
}

static void setup(void)
{
	tst_res(TINFO, "Testing %s variant", sched_variants[tst_variant].desc);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.tcnt = ARRAY_SIZE(tcases),
	.test = run,
};
