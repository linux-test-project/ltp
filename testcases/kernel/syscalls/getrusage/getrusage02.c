// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR : Saji Kumar.V.R <saji.kumar@wipro.com>
 */

/*\
 * Verify that getrusage() fails with:
 *
 * - EINVAL with invalid who
 * - EFAULT with invalid usage pointer
 */

#include <errno.h>
#include <sched.h>
#include <sys/resource.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

static struct rusage usage;

static int libc_getrusage(int who, void *usage)
{
	return getrusage(who, usage);
}

static int sys_getrusage(int who, void *usage)
{
	return tst_syscall(__NR_getrusage, who, usage);
}

struct tc_t {
	int who;
	struct rusage *usage;
	int exp_errno;
} tc[] = {
	{-2, &usage, EINVAL},
	{RUSAGE_SELF, (struct rusage *)-1, EFAULT}
};

static struct test_variants
{
	int (*getrusage)(int who, void *usage);
	char *desc;
} variants[] = {
	{ .getrusage = libc_getrusage, .desc = "libc getrusage()"},
#if (__NR_getrusage != __LTP__NR_INVALID_SYSCALL)
	{ .getrusage = sys_getrusage,  .desc = "__NR_getrusage syscall"},
#endif
};

static void verify_getrusage(unsigned int i)
{
	struct test_variants *tv = &variants[tst_variant];

	if (tc[i].exp_errno == EFAULT &&
	    tv->getrusage == libc_getrusage) {
		tst_res(TCONF, "EFAULT is skipped for libc variant");
		return;
	}

	TST_EXP_FAIL(tv->getrusage(tc[i].who, tc[i].usage), tc[i].exp_errno,
	             "getrusage(%i, %p)", tc[i].who, tc[i].usage);
}

static void setup(void)
{
	struct test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
}

static struct tst_test test = {
	.test = verify_getrusage,
	.setup = setup,
	.tcnt = ARRAY_SIZE(tc),
	.test_variants = ARRAY_SIZE(variants),
};
