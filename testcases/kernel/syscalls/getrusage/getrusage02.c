// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR : Saji Kumar.V.R <saji.kumar@wipro.com>
 */

/*\
 * [Description]
 *
 * Verify that getrusage() fails with:
 *
 * - EINVAL with invalid who
 * - EFAULT with invalid usage pointer
 */

#include <errno.h>
#include <sched.h>
#include <sys/resource.h>
#include "tst_test.h"

static struct rusage usage;

struct tc_t {
	int who;
	struct rusage *usage;
	int exp_errno;
} tc[] = {
	{-2, &usage, EINVAL},
	{RUSAGE_SELF, (struct rusage *)-1, EFAULT}
};

static void verify_getrusage(unsigned int i)
{
	TST_EXP_FAIL(getrusage(tc[i].who, tc[i].usage), tc[i].exp_errno,
	             "getrusage(%i, %p)", tc[i].who, tc[i].usage);
}

static struct tst_test test = {
	.test = verify_getrusage,
	.tcnt = ARRAY_SIZE(tc),
};
