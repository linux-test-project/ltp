// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Red Hat, Inc.
 *
 * Check that futex(FUTEX_WAKE) returns EFAULT when:
 * 1) uaddr points to unmapped memory
 * 2) uaddr points to memory without read permission (PROT_NONE)
 */

#include <errno.h>
#include <sys/mman.h>
#include "futextest.h"

static void *unmapped_addr;
static void *prot_none_addr;

static struct futex_test_variants variants[] = {
#if (__NR_futex != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX, .desc = "syscall with old kernel spec"},
#endif

#if (__NR_futex_time64 != __LTP__NR_INVALID_SYSCALL)
	{ .fntype = FUTEX_FN_FUTEX64, .desc = "syscall time64 with kernel spec"},
#endif
};

static struct testcase {
	const char *desc;
	int opflags;
	int exp_errno;
} testcases[] = {
	{ "uaddr unmapped", 0, EFAULT },
	{ "uaddr PROT_NONE", 0, EFAULT },
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];
	futex_t *addr;
	int res;

	if (n == 0)
		addr = (futex_t *)unmapped_addr;
	else
		addr = (futex_t *)prot_none_addr;
	res = futex_wake(tv->fntype, addr, 1, tc->opflags);
	if (res != -1) {
		tst_res(TFAIL, "futex_wake() succeeded unexpectedly for '%s'", tc->desc);
		return;
	}
	if (errno != tc->exp_errno) {
		tst_res(TFAIL | TERRNO, "futex_wake() failed with unexpected error for '%s', we expected: %s", tc->desc, tst_strerrno(tc->exp_errno));
		return;
	}
	tst_res(TPASS | TERRNO, "futex_wake() failed as expected for '%s'", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	size_t pagesize = getpagesize();

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	unmapped_addr = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(unmapped_addr, pagesize);
	prot_none_addr = SAFE_MMAP(NULL, pagesize, PROT_NONE, MAP_PRIVATE| MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (prot_none_addr) {
		SAFE_MUNMAP(prot_none_addr, getpagesize());
	}
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
