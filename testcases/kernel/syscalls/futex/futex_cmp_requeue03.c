// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 * Copyright (C) 2026 Michael Menasherov <mmenashe@redhat.com>
 */

/*\
 * Check that :manpage:`futex(2)`" with "``FUTEX_CMP_REQUEUE`` returns EFAULT
 * when uaddr or uaddr2 points to unmapped memory, or when uaddr or uaddr2
 * points to memory without read permission (PROT_NONE).
 *
 * The test uses opflags=0 (no FUTEX_PRIVATE_FLAG) so get_futex_key()
 * takes the shared-futex path and resolves the physical page; this
 * lookup fails with EFAULT for unmapped and PROT_NONE addresses.
 * get_futex_key() is called for both uaddr and uaddr2 before the
 * *uaddr == val check; futex_var and val are both FUTEX_INITIALIZER.
 */

#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t futex_var = FUTEX_INITIALIZER;
static futex_t *futex_ptr = &futex_var;
static futex_t *unmapped_addr;
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
	futex_t **uaddr;
	futex_t **uaddr2;
} testcases[] = {
	{
		.desc = "uaddr unmapped",
		.uaddr = &unmapped_addr,
		.uaddr2 = &futex_ptr,
	},
	{
		.desc = "uaddr2 unmapped",
		.uaddr = &futex_ptr,
		.uaddr2 = &unmapped_addr,
	},
	{
		.desc = "uaddr PROT_NONE",
		.uaddr = &prot_none_addr,
		.uaddr2 = &futex_ptr,
	},
	{
		.desc = "uaddr2 PROT_NONE",
		.uaddr = &futex_ptr,
		.uaddr2 = &prot_none_addr,
	},
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];

	TST_EXP_FAIL(futex_cmp_requeue(tv->fntype, *tc->uaddr, futex_var,
				       *tc->uaddr2, 1, 1, 0), EFAULT, "%s", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	size_t pagesize = getpagesize();

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	unmapped_addr = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE,
				  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP((void *)unmapped_addr, pagesize);

	prot_none_addr = SAFE_MMAP(NULL, pagesize, PROT_NONE,
				   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	if (prot_none_addr)
		SAFE_MUNMAP((void *)prot_none_addr, getpagesize());
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
};
