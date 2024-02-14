// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Saji Kumar.V.R <saji.kumar@wipro.com>
 * Copyright (c) 2021 Xie Ziyao <xieziyao@huawei.com>
 */

/*\
 * [Description]
 *
 * Test that getrusage() with RUSAGE_SELF and RUSAGE_CHILDREN succeeds.
 */

#include "tst_test.h"

static struct rusage *usage;

struct test_case_t {
	int who;
	char *desc;
} tc[] = {
	{RUSAGE_SELF, "RUSAGE_SELF"},
	{RUSAGE_CHILDREN, "RUSAGE_CHILDREN"},
};

static void run(unsigned int i)
{

	TST_EXP_PASS(getrusage(tc[i].who, usage), "getrusage(%s, %p)", tc[i].desc, usage);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.test = run,
	.bufs = (struct tst_buffers[]) {
		{&usage, .size = sizeof(struct rusage)},
		{}
	}
};
