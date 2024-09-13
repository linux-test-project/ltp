// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (c) Linux Test Project, 2015-2024
 */

/*\
 * [Description]
 *
 * Verify that, sched_getattr(2) returns -1 and sets errno to:
 *
 * 1. ESRCH if pid is unused.
 * 2. EINVAL if address is NULL.
 * 3. EINVAL if size is invalid.
 * 4. EINVAL if flag is not zero.
 */

#define _GNU_SOURCE

#include <errno.h>
#include "tst_test.h"
#include "lapi/sched.h"

static pid_t pid;
static pid_t unused_pid;
static struct sched_attr attr_copy;

static struct test_case {
	pid_t *pid;
	struct sched_attr *attr;
	unsigned int size;
	unsigned int flags;
	int exp_errno;
} tcase[] = {
	{&unused_pid, &attr_copy, sizeof(struct sched_attr), 0, ESRCH},
	{&pid, NULL, sizeof(struct sched_attr), 0, EINVAL},
	{&pid, &attr_copy, SCHED_ATTR_SIZE_VER0 - 1, 0, EINVAL},
	{&pid, &attr_copy, sizeof(struct sched_attr), 1000, EINVAL}
};

static void verify_sched_getattr(unsigned int n)
{
	struct test_case *tc = &tcase[n];

	TST_EXP_FAIL(sched_getattr(*(tc->pid), tc->attr, tc->size, tc->flags),
				 tc->exp_errno, "sched_getattr(%d, ..., %d, %d)", *tc->pid,
				 tc->size, tc->flags);
}

static void setup(void)
{
	unused_pid = tst_get_unused_pid();
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test = verify_sched_getattr,
	.tcnt = ARRAY_SIZE(tcase),
	.setup = setup,
};
