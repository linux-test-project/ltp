// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 */

/*\
 * Check that futex(FUTEX_CMP_REQUEUE) returns EFAULT when uaddr or
 * uaddr2 points to unmapped memory, and EACCES (or EFAULT on older kernels)
 * when uaddr points to memory without read permission (PROT_NONE).
 *
 * The EACCES behavior for PROT_NONE was introduced in kernel 5.9.
 */

#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex_var = FUTEX_INITIALIZER;
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
	futex_t *uaddr;
	futex_t *uaddr2;
	int exp_errno;
} testcases[3];

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];

	TST_EXP_FAIL(futex_cmp_requeue(tv->fntype, tc->uaddr, futex_var,
		tc->uaddr2, 1, 1, 0), tc->exp_errno, "%s", tc->desc);
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
		.uaddr = unmapped,
		.uaddr2 = &futex_var,
		.exp_errno = EFAULT,
	};
	testcases[1] = (struct testcase){
		.desc = "uaddr2 unmapped",
		.uaddr = &futex_var,
		.uaddr2 = unmapped,
		.exp_errno = EFAULT,
	};
	testcases[2] = (struct testcase){
		.desc = "uaddr PROT_NONE",
		.uaddr = prot_none_addr,
		.uaddr2 = &futex_var,
		.exp_errno = tst_kvercmp(5, 9, 0) >= 0 ? EACCES : EFAULT,
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
