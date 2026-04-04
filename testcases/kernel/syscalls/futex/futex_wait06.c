// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Red Hat, Inc.
 *
 * Check that futex(FUTEX_WAIT) returns EFAULT when:
 * 1) uaddr points to unmapped memory
 * 2) timeout points to unmapped memory
 */
#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex = FUTEX_INITIALIZER;
static void *bad_addr;

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
} testcases[] = {
	{ "uaddr points to unmapped memory" },
	{ "timeout points to unmapped memory" },
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];
	int res;

	if (n == 0) {
		res = futex_syscall(tv->fntype, (futex_t *)bad_addr, FUTEX_WAIT, 0, NULL, NULL, 0, 0);
	} else if (n == 1) {
		res = futex_syscall(tv->fntype, &futex, FUTEX_WAIT, futex, bad_addr, NULL, 0, 0);
	} else {
		tst_brk(TBROK, "Invalid test case %u", n);
		return;
	}

	if (res != -1) {
		tst_res(TFAIL, "futex_wait() succeeded unexpectedly for '%s'", tc->desc);
		return;
	}

	if (errno != EFAULT) {
		tst_res(TFAIL | TERRNO, "futex_wait() expected EFAULT for '%s', got", tc->desc);
		return;
	}

	tst_res(TPASS | TERRNO, "futex_wait() failed as expected for '%s'", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);
	bad_addr = mmap(NULL, getpagesize(), PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (bad_addr == MAP_FAILED) {
		tst_brk(TBROK | TERRNO, "mmap() failed");
	}
	SAFE_MUNMAP(bad_addr, getpagesize());
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
