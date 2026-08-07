// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2026 Linux Test Project
 */

/*\
 * Verify that :manpage:`mremap(2)` fails with errno ``ENOMEM`` when it is used
 * to expand an existing mapping in place, if the region cannot be expanded at
 * the current virtual address and the ``MREMAP_MAYMOVE`` flag is not set.
 *
 * [Algorithm]
 *
 * - map an anonymous region of three pages
 * - unmap the middle page to create a gap
 * - call mremap() on the first page to grow it to three pages with ``flags``
 *   set to 0
 * - expect the call to fail with ``MAP_FAILED`` and ``ENOMEM`` since the
 *   third page is occupied.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static size_t pagesize;
static void *addr = MAP_FAILED;

static void setup(void)
{
	pagesize = getpagesize();
}

static void run(void)
{
	size_t map_size = 3 * pagesize;
	size_t old_size = 1 * pagesize;
	size_t new_size = 3 * pagesize;

	addr = SAFE_MMAP(NULL, map_size, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	/* unmap the middle page to create a gap */
	SAFE_MUNMAP((char *)addr + pagesize, pagesize);

	TST_EXP_FAIL_PTR_VOID(mremap(addr, old_size, new_size, 0), ENOMEM);

	if (TST_RET_PTR != MAP_FAILED) {
		SAFE_MUNMAP(TST_RET_PTR, new_size);
		return;
	}

	SAFE_MUNMAP(addr, pagesize);
	SAFE_MUNMAP((char *)addr + 2 * pagesize, pagesize);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
