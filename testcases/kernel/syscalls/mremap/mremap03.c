// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 11/2001 Modified by Manoj Iyer <manjo@austin.ibm.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Verify that :manpage:`mremap(2)` fails with errno ``EFAULT`` when it is used
 * to expand a mapping whose ``old_address`` refers to a region that is not
 * mapped.
 *
 * [Algorithm]
 *
 * - Obtain an unmapped address via ``tst_get_bad_addr()``.
 * - Call mremap() to grow it to twice its size with ``MREMAP_MAYMOVE``,
 *   passing the unmapped ``old_address``.
 * - Expect the call to fail with ``MAP_FAILED`` and ``EFAULT``.
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static long page_size;
static size_t memsize;
static size_t newsize;
static void *bad_addr;

static void setup(void)
{
	page_size = getpagesize();
	memsize = 1000 * page_size;
	newsize = 2 * memsize;

	bad_addr = tst_get_bad_addr(NULL);
	SAFE_MUNMAP(bad_addr, page_size);
}

static void run(void)
{
	TST_EXP_FAIL_PTR_VOID(mremap(bad_addr, memsize, newsize,
				     MREMAP_MAYMOVE), EFAULT);

	if (TST_RET_PTR != MAP_FAILED)
		SAFE_MUNMAP(TST_RET_PTR, newsize);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
};
