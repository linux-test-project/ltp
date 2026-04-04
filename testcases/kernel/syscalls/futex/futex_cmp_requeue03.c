// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 Red Hat, Inc.
 *
 * Check that futex(FUTEX_CMP_REQUEUE) returns EFAULT when uaddr or
 * uaddr2 points to unmapped memory, and EACCES when uaddr points to
 * memory without read permission (PROT_NONE).
 */

#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex = FUTEX_INITIALIZER;
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
	/* 1 = uaddr is bad, 0 = uaddr2 is bad */
	int bad_uaddr;
	/* 1 = PROT_NONE address, 0 = unmapped address */
	int use_prot_none;
} testcases[] = {
	{ "uaddr unmapped", 1, 0 },
	{ "uaddr2 unmapped", 0, 0 },
	{ "uaddr PROT_NONE", 1, 1 },
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];
	futex_t *bad;
	futex_t *uaddr, *uaddr2;
	int res;

	if (tc->use_prot_none)
		bad = (futex_t *)prot_none_addr;
	else
		bad = (futex_t *)unmapped_addr;

	/* Assign bad address to uaddr or uaddr2, keep the other valid. */
	if (tc->bad_uaddr) {
		uaddr = bad;
		uaddr2 = &futex;
	} else {
		uaddr = &futex;
		uaddr2 = bad;
	}

	res = futex_cmp_requeue(tv->fntype, uaddr, futex, uaddr2, 1, 1, 0);
	if (res != -1) {
		tst_res(TFAIL, "futex_cmp_requeue() succeeded unexpectedly for '%s'", tc->desc);
		return;
	}
	if (errno != EFAULT && errno != EACCES) {
		tst_res(TFAIL | TERRNO, "futex_cmp_requeue() failed with unexpected error for '%s', expected EFAULT or EACCES",tc->desc);
		return;
	}
	tst_res(TPASS | TERRNO, "futex_cmp_requeue() failed as expected for '%s'", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	size_t pagesize = getpagesize();

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	unmapped_addr = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP(unmapped_addr, pagesize);
	/* PROT_NONE = mapped but no read permission, triggers EACCES or EFAULT */
	prot_none_addr = SAFE_MMAP(NULL, pagesize, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
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
