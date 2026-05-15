// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 */

/*\
 * Check that futex(FUTEX_WAIT) returns EFAULT when:
 *
 * 1) uaddr points to unmapped memory
 * 2) timeout points to unmapped memory
 */
#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex = FUTEX_INITIALIZER;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .tstype = TST_KERN_OLD_TIMESPEC, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .tstype = TST_KERN_TIMESPEC, .desc = "syscall time64 with kernel spec"},
#endif
};

static struct testcase {
	const char *desc;
	futex_t *uaddr;
	void *timeout;
} testcases[2];

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];

	TST_EXP_FAIL(futex_syscall(tv->fntype, tc->uaddr, FUTEX_WAIT, futex,
		tc->timeout, NULL, 0, 0), EFAULT, "%s", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	void *bad;

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	bad = SAFE_MMAP(NULL, getpagesize(), PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(bad, getpagesize());

	testcases[0] = (struct testcase){
		.desc = "uaddr points to unmapped memory",
		.uaddr = bad,
		.timeout = NULL,
	};
	testcases[1] = (struct testcase){
		.desc = "timeout points to unmapped memory",
		.uaddr = &futex,
		.timeout = bad,
	};
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
