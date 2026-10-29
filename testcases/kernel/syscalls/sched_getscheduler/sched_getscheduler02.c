// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021, BELLSOFT. All rights reserved.
 * Copyright (c) International Business Machines  Corp., 2001
 */

/*\
 * Pass an unused pid to sched_getscheduler() and test for ESRCH.
 */

#include <stdio.h>
#include <errno.h>

#include "tst_test.h"
#include "tst_sched.h"

static pid_t unused_pid;

static void setup(void)
{
	tst_res(TINFO, "Testing %s variant", sched_variants[tst_variant].desc);

	unused_pid = tst_get_unused_pid();
}

static void run(void)
{
	struct sched_variant *tv = &sched_variants[tst_variant];

	TST_EXP_FAIL(tv->sched_getscheduler(unused_pid), ESRCH,
		     "sched_getscheduler(%d)", unused_pid);
}

static struct tst_test test = {
	.setup = setup,
	.test_variants = ARRAY_SIZE(sched_variants),
	.test_all = run,
};
