// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 */

/*\
 * Check that futex(FUTEX_WAKE) returns EFAULT when uaddr points to
 * unmapped or PROT_NONE memory.
 *
 * Note: FUTEX_WAKE never reads *uaddr, so PROT_NONE triggers EFAULT
 * (not EACCES). The EACCES behavior only applies to syscalls that read
 * *uaddr (e.g. FUTEX_WAIT, FUTEX_CMP_REQUEUE).
 */

#include <errno.h>
#include <sys/mman.h>
#include "futextest.h"

static futex_t *prot_none_addr;

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
	futex_t *addr;
	int exp_errno;
} testcases[2];

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];

	TST_EXP_FAIL(futex_wake(tv->fntype, tc->addr, 1, 0),
		tc->exp_errno, "%s", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	size_t pagesize = getpagesize();
	futex_t *unmapped;

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	unmapped = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(unmapped, pagesize);

	prot_none_addr = SAFE_MMAP(NULL, pagesize, PROT_NONE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	testcases[0] = (struct testcase){
		.desc = "uaddr unmapped",
		.addr = unmapped,
		.exp_errno = EFAULT,
	};
	testcases[1] = (struct testcase){
		.desc = "uaddr PROT_NONE",
		.addr = prot_none_addr,
		.exp_errno = EFAULT,
	};
}

static void cleanup(void)
{
	if (prot_none_addr)
		SAFE_MUNMAP(prot_none_addr, getpagesize());
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
