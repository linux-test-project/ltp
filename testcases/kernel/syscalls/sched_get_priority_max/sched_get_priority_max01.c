// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2021 sujiaxun <sujiaxun@uniontech.com>
 */

/*\
 * [Description]
 *
 * Basic test for the sched_get_priority_max(2) system call.
 *
 * Obtain different maximum priority for different schedulling policies and
 * compare them with expected value.
 */

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
	{POLICY_DESC(SCHED_FIFO), 99},
	{POLICY_DESC(SCHED_RR), 99}
};

static void run_test(unsigned int nr)
{

	struct test_case *tc = &tcases[nr];

	TST_EXP_VAL(tst_syscall(__NR_sched_get_priority_max, tc->policy),
			tc->retval, "%s", tc->desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = run_test,
};
