// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2021 sujiaxun <sujiaxun@uniontech.com>
 */

/*\
 * Verify that given an invalid scheduling policy, sched_get_priority_max(2)
 * returns -1 with errno EINVAL.
 */

#include <errno.h>
#include <sched.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#define SCHED_INVALID 1000

static void verif_sched_get_priority_max02(void)
{
	TST_EXP_FAIL(tst_syscall(__NR_sched_get_priority_max, SCHED_INVALID), EINVAL);
}

static struct tst_test test = {
	.test_all = verif_sched_get_priority_max02,
};
