// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Verify the behavior of the ``MREMAP_FIXED`` flag of :manpage:`mremap(2)`:
 *
 * - ``MREMAP_FIXED | MREMAP_MAYMOVE`` can move a mapping to a new, free
 *   address, preserving its content.
 * - ``MREMAP_FIXED | MREMAP_MAYMOVE`` unmaps a previously existing mapping at
 *   the target address range and replaces it with the moved mapping.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static int pagesize;

static struct tcase {
	size_t old_pages;
	size_t new_pages;
	int flags;
	int free_dst;
	const char *msg;
} tcases[] = {
	{
		.old_pages = 1,
		.new_pages = 1,
		.flags = MREMAP_FIXED | MREMAP_MAYMOVE,
		.free_dst = 1,
		.msg = "move mapping to free address",
	},
	{
		.old_pages = 1,
		.new_pages = 1,
		.flags = MREMAP_FIXED | MREMAP_MAYMOVE,
		.free_dst = 0,
		.msg = "move mapping onto an occupied address",
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

static void fill_pattern(char *addr, size_t pages, char *new_addr, int free_dst)
{
	size_t i;

	for (i = 0; i < pages; i++)
		addr[i * pagesize] = (char)(0x1 + i);

	if (!free_dst)
		*new_addr = 0x7f;
}

static int check_pattern(char *addr, size_t pages)
{
	size_t i;

	for (i = 0; i < pages; i++) {
		char got = addr[i * pagesize];
		char exp = (char)(0x1 + i);

		if (got != exp)
			return 1;
	}

	return 0;
}

static void run(unsigned int n)
{
	struct tcase *tc = &tcases[n];
	char *old_address;
	char *new_address;
	char *ret;
	size_t old_size = tc->old_pages * pagesize;
	size_t new_size = tc->new_pages * pagesize;

	old_address = get_test_area(old_size, 0);
	new_address = get_test_area(new_size, tc->free_dst);

	fill_pattern(old_address, tc->old_pages, new_address, tc->free_dst);

	TESTPTR(mremap(old_address, old_size, new_size, tc->flags, new_address));
	ret = TST_RET_PTR;

	if (ret == MAP_FAILED) {
		tst_res(TFAIL | TTERRNO, "%s failed", tc->msg);
		SAFE_MUNMAP(old_address, old_size);
		if (!tc->free_dst)
			SAFE_MUNMAP(new_address, new_size);
		return;
	}

	if (ret != new_address)
		tst_res(TFAIL, "%s: ret %p, expected %p", tc->msg, ret, new_address);
	else if (check_pattern(ret, tc->new_pages))
		tst_res(TFAIL, "%s: pattern mismatch", tc->msg);
	else
		tst_res(TPASS, "%s", tc->msg);

	SAFE_MUNMAP(ret, new_size);
}

static struct tst_test test = {
	.setup = setup,
	.test = run,
	.tcnt = ARRAY_SIZE(tcases),
};
