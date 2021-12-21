// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2021 sujiaxun <sujiaxun@uniontech.com>
 */

/*\
 * [Description]
 *
 * Basic test for the sched_get_priority_min(2) system call.
 *
 * Obtain different minimum priority scheduling strategies and
 * compare them with the expected value.
 */

#include <errno.h>
#include <sched.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define POLICY_DESC(x) .desc = #x, .policy = x

static struct test_case {
	char *desc;
	int policy;
	int retval;
} tcases[] = {
	{POLICY_DESC(SCHED_OTHER), 0},
	{POLICY_DESC(SCHED_FIFO), 1},
	{POLICY_DESC(SCHED_RR), 1}
};

static void run_test(unsigned int nr)
{
	struct test_case *tc = &tcases[nr];

	TST_EXP_VAL(tst_syscall(__NR_sched_get_priority_min, tc->policy),
			tc->retval, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run_test,
};
