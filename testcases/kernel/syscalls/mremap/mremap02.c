// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 11/2001 Modified by Manoj Iyer <manjo@austin.ibm.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Verify that :manpage:`mremap(2)` fails with errno ``EINVAL`` when it is used
 * to expand an existing mapping while the passed ``old_address`` is not page
 * aligned.
 *
 * [Algorithm]
 *
 * - Map a single anonymous page.
 * - Call mremap() to grow it to two pages with ``MREMAP_MAYMOVE``, passing a
 *   deliberately misaligned ``old_address`` (mapping start + 1 byte).
 * - Expect the call to fail with ``MAP_FAILED`` and ``EINVAL``.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static long page_size;
static size_t new_size;
static void *page;
static void *bad_addr;

static void setup(void)
{
	page_size = getpagesize();
	new_size = page_size * 2;

	page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	/* Force a non-page-aligned old_address, the invalid argument. */
	bad_addr = (char *)page + 1;
}

static void run(void)
{
	TST_EXP_FAIL_PTR_VOID(mremap(bad_addr, page_size, new_size,
				     MREMAP_MAYMOVE), EINVAL);

	if (TST_RET_PTR != MAP_FAILED) {
		SAFE_MUNMAP(TST_RET_PTR, new_size);
		page = SAFE_MMAP(NULL, page_size, PROT_READ | PROT_WRITE,
				 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		bad_addr = (char *)page + 1;
	}
}

static void cleanup(void)
{
	SAFE_MUNMAP(page, page_size);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
