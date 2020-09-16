// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2015 Cyril Hrubis <chrubis@suse.cz>
 *
 * futex_wake() returns 0 (0 woken up processes) when no processes wait on the mutex.
 */

#include <limits.h>

#include "futextest.h"

struct testcase {
	futex_t *f_addr;
	int nr_wake;
	int opflags;
};

static futex_t futex = FUTEX_INITIALIZER;

static struct testcase testcases[] = {
	/* nr_wake = 0 is noop */
	{&futex, 0, 0},
	{&futex, 0, FUTEX_PRIVATE_FLAG},
	{&futex, 1, 0},
	{&futex, 1, FUTEX_PRIVATE_FLAG},
	{&futex, INT_MAX, 0},
	{&futex, INT_MAX, FUTEX_PRIVATE_FLAG},
};

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .desc = "syscall time64 with kernel spec"},
#endif
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];
	int res;

	res = futex_wake(tv->fntype, tc->f_addr, tc->nr_wake, tc->opflags);
	if (res != 0) {
		tst_res(TFAIL | TERRNO, "futex_wake() failed");
		return;
	}

	tst_res(TPASS, "futex_wake() passed");
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
