// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2012 Linux Test Project, Inc.
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Verify the behavior of the ``MREMAP_FIXED`` flag of :manpage:`mremap(2)`
 * when invalid arguments are provided:
 *
 * - ``MREMAP_FIXED`` fails with ``EINVAL`` without ``MREMAP_MAYMOVE``.
 * - ``MREMAP_FIXED | MREMAP_MAYMOVE`` fails with ``EINVAL`` if the target
 *   address is not page aligned.
 * - ``MREMAP_FIXED | MREMAP_MAYMOVE`` fails with ``EINVAL`` if the old range
 *   overlaps with the new range.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static int pagesize;

static struct tcase {
	size_t old_pages;
	size_t new_pages;
	int flags;
	int align_offset;
	int overlap;
	int free_dst;
	int exp_errno;
	const char *msg;
} tcases[] = {
	{
		.old_pages = 1,
		.new_pages = 1,
		.flags = MREMAP_FIXED,
		.free_dst = 1,
		.exp_errno = EINVAL,
		.msg = "MREMAP_FIXED requires MREMAP_MAYMOVE",
	},
	{
		.old_pages = 1,
		.new_pages = 1,
		.flags = MREMAP_FIXED | MREMAP_MAYMOVE,
		.align_offset = 1,
		.exp_errno = EINVAL,
		.msg = "new_addr has to be page aligned",
	},
	{
		.old_pages = 2,
		.new_pages = 1,
		.flags = MREMAP_FIXED | MREMAP_MAYMOVE,
		.overlap = 1,
		.exp_errno = EINVAL,
		.msg = "old/new area must not overlap",
	},
};

static void *get_test_area(size_t size, int free_area)
{
	void *p;

	p = SAFE_MMAP(NULL, size, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (free_area)
		SAFE_MUNMAP(p, size);

	return p;
}

static void setup(void)
{
	pagesize = getpagesize();
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	char *old_address;
	char *new_address;
	size_t old_size = tc->old_pages * pagesize;
	size_t new_size = tc->new_pages * pagesize;

	old_address = get_test_area(old_size, 0);

	if (tc->overlap) {
		new_address = old_address;
	} else if (tc->align_offset) {
		new_address = get_test_area(new_size + pagesize, 1) +
			tc->align_offset;
	} else {
		new_address = get_test_area(new_size, tc->free_dst);
	}

	TST_EXP_FAIL_PTR_VOID(mremap(old_address, old_size, new_size,
				     tc->flags, new_address),
			      tc->exp_errno, "%s", tc->msg);

	if (TST_RET_PTR == MAP_FAILED)
		SAFE_MUNMAP(old_address, old_size);
	else
		SAFE_MUNMAP(TST_RET_PTR, new_size);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
