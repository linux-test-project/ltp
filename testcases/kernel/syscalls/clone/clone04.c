// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * Copyright (c) 2012 Wanlong Gao <gaowanlong@cn.fujitsu.com>
 */

/*\
 * [Description]
 *
 * Verify that clone(2) fails with
 *
 * - EINVAL if child stack is set to NULL
 */

#include <stdlib.h>
#include "tst_test.h"
#include "clone_platform.h"

static int child_fn(void *arg LTP_ATTRIBUTE_UNUSED);

static struct tcase {
	int (*child_fn)(void *arg);
	void *child_stack;
	int exp_errno;
	char err_desc[10];
} tcases[] = {
	{child_fn, NULL, EINVAL, "NULL stack"},
};

static int child_fn(void *arg LTP_ATTRIBUTE_UNUSED)
{
	exit(0);
}

static void verify_clone(unsigned int nr)
{
	struct tcase *tc = &tcases[nr];

	TST_EXP_FAIL(ltp_clone(0, tc->child_fn, NULL,
				CHILD_STACK_SIZE, tc->child_stack),
				tc->exp_errno, "%s", tc->err_desc);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_clone,
};
