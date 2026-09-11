// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Verify that :manpage:`sched_setattr(2)` fails and sets errno to:
 *
 * - :c:macro:`ESRCH` when pid is unused
 * - :c:macro:`EINVAL` when pid is negative
 * - :c:macro:`EINVAL` when sched_attr address is NULL
 * - :c:macro:`EFAULT` when sched_attr address is invalid
 * - :c:macro:`E2BIG` when sched_attr size is smaller than version 0
 * - :c:macro:`EINVAL` when flags are invalid
 * - :c:macro:`EINVAL` when sched_policy is invalid
 * - :c:macro:`EINVAL` when runtime exceeds deadline
 */

#define _GNU_SOURCE

#include <errno.h>

#include "tst_test.h"
#include "lapi/sched.h"

#define RUNTIME_VAL 10000000
#define PERIOD_VAL 30000000
#define DEADLINE_VAL 30000000

static pid_t unused_pid;
static pid_t invalid_pid = -1;
static void *bad_addr;

static struct sched_attr attr = {
	.size = sizeof(struct sched_attr),
	.sched_policy = SCHED_DEADLINE,
	.sched_runtime = RUNTIME_VAL,
	.sched_period = PERIOD_VAL,
	.sched_deadline = DEADLINE_VAL,
};

static struct sched_attr attr_small = {
	.size = SCHED_ATTR_SIZE_VER0 - 1,
};

static struct sched_attr attr_invalid_policy = {
	.size = sizeof(struct sched_attr),
	.sched_policy = 999,
};

static struct sched_attr attr_bad_dl = {
	.size = sizeof(struct sched_attr),
	.sched_policy = SCHED_DEADLINE,
	.sched_runtime = PERIOD_VAL,
	.sched_deadline = RUNTIME_VAL,
	.sched_period = PERIOD_VAL,
};

static struct tcase {
	pid_t *pid;
	struct sched_attr *attr;
	int bad_attr;
	unsigned int flags;
	int exp_errno;
	const char *desc;
} tcases[] = {
	{
		.pid = &unused_pid,
		.attr = &attr,
		.exp_errno = ESRCH,
		.desc = "sched_setattr() with unused pid",
	},
	{
		.pid = &invalid_pid,
		.attr = &attr,
		.exp_errno = EINVAL,
		.desc = "sched_setattr() with negative pid",
	},
	{
		.exp_errno = EINVAL,
		.desc = "sched_setattr() with NULL sched_attr",
	},
	{
		.bad_attr = 1,
		.exp_errno = EFAULT,
		.desc = "sched_setattr() with invalid sched_attr address",
	},
	{
		.attr = &attr_small,
		.exp_errno = E2BIG,
		.desc = "sched_setattr() with size smaller than version 0",
	},
	{
		.attr = &attr,
		.flags = 1000,
		.exp_errno = EINVAL,
		.desc = "sched_setattr() with invalid flags",
	},
	{
		.attr = &attr_invalid_policy,
		.exp_errno = EINVAL,
		.desc = "sched_setattr() with invalid sched_policy",
	},
	{
		.attr = &attr_bad_dl,
		.exp_errno = EINVAL,
		.desc = "sched_setattr() with runtime exceeding deadline",
	},
};

static void verify_sched_setattr(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	pid_t pid = tc->pid ? *tc->pid : 0;
	struct sched_attr *target_attr = tc->bad_attr ? bad_addr : tc->attr;

	/*
	 * The kernel writes sizeof(struct sched_attr) back to uattr->size
	 * on the -E2BIG error path, clobbering our test input. Refresh
	 * before each call so re-runs (e.g. -i N) still exercise the
	 * intended size.
	 */
	attr_small.size = SCHED_ATTR_SIZE_VER0 - 1;

	TST_EXP_FAIL(sched_setattr(pid, target_attr, tc->flags),
		     tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
	bad_addr = tst_get_bad_addr(NULL);
}

static struct tst_test test = {
	.test = verify_sched_setattr,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
};
