// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Red Hat, Inc.
 * Copyright (C) 2026 Michael Menasherov <mmenashe@redhat.com>
 */

/*\
 * Check that :manpage:`futex(2)` with ``FUTEX_WAKE`` returns EFAULT when
 * uaddr points to unmapped, PROT_NONE, or truncated file-backed memory.
 *
 * For opflags=0 (no FUTEX_PRIVATE_FLAG) futex_wake() takes the
 * shared-futex path in get_futex_key() which must resolve the physical
 * page.
 *
 * The three cases exercise different code paths:
 *
 * - unmapped memory fails at find_vma() (no VMA exists)
 * - PROT_NONE memory fails at get_user_pages_fast() (VMA exists but
 *   page is inaccessible)
 * - a file-backed address beyond the file's end triggers VM_FAULT_SIGBUS
 *   inside the page fault handler, which get_user_pages() propagates as
 *   EFAULT
 */

#include <errno.h>
#include <sys/mman.h>

#include "futextest.h"

static futex_t *unmapped_addr;
static futex_t *prot_none_addr;
static futex_t *file_trunc_addr;

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
	futex_t **addr;
} testcases[] = {
	{
		.desc = "uaddr unmapped",
		.addr = &unmapped_addr,
	},
	{
		.desc = "uaddr PROT_NONE",
		.addr = &prot_none_addr,
	},
	{
		.desc = "uaddr file truncated",
		.addr = &file_trunc_addr,
	},
};

static void run(unsigned int n)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	struct testcase *tc = &testcases[n];

	TST_EXP_FAIL(futex_wake(tv->fntype, *tc->addr, 1, 0),
		     EFAULT, "%s", tc->desc);
}

static void setup(void)
{
	struct futex_test_variants *tv = &variants[tst_variant];
	size_t pagesize = getpagesize();
	int fd;

	tst_res(TINFO, "Testing variant: %s", tv->desc);
	futex_supported_by_kernel(tv->fntype);

	unmapped_addr = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE,
				  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	SAFE_MUNMAP((void *)unmapped_addr, pagesize);

	prot_none_addr = SAFE_MMAP(NULL, pagesize, PROT_NONE,
				   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	fd = SAFE_OPEN("futex_wake05_tmp", O_CREAT | O_RDWR | O_TRUNC, 0644);
	SAFE_FTRUNCATE(fd, pagesize);
	file_trunc_addr = SAFE_MMAP(NULL, pagesize, PROT_READ | PROT_WRITE,
				    MAP_SHARED, fd, 0);
	SAFE_FTRUNCATE(fd, 0);
	SAFE_CLOSE(fd);
	SAFE_UNLINK("futex_wake05_tmp");
}

static void cleanup(void)
{
	size_t pagesize = getpagesize();

	if (prot_none_addr)
		SAFE_MUNMAP((void *)prot_none_addr, pagesize);
	if (file_trunc_addr)
		SAFE_MUNMAP((void *)file_trunc_addr, pagesize);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test = run,
	.tcnt = ARRAY_SIZE(testcases),
	.test_variants = ARRAY_SIZE(variants),
	.needs_tmpdir = 1,
};
