// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * 07/2001 Ported by Wayne Boyer
 * 11/2001 Modified by Manoj Iyer <manjo@austin.ibm.com>
 * 02/2008 Modified by Renaud Lottiaux <Renaud.Lottiaux@kerlabs.com>
 * Copyright (c) Linux Test Project, 2026
 */

/*\
 * Verify that :manpage:`mremap(2)` fails with errno ``ENOMEM`` when it is used
 * to expand an existing mapping in place, if the region cannot be expanded at
 * the current virtual address and the ``MREMAP_MAYMOVE`` flag is not set.
 *
 * [Algorithm]
 *
 * - map an anonymous region of two pages
 * - call mremap() on the mapped address to grow it from one page to two
 *   pages with ``flags`` set to 0 (no ``MREMAP_MAYMOVE``)
 * - expect the call to fail with ``MAP_FAILED`` and ``ENOMEM``
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include "tst_test.h"

static void *addr = MAP_FAILED;
static size_t memsize;
static size_t newsize;

static void setup(void)
{
	memsize = getpagesize();
	newsize = memsize * 2;

	addr = SAFE_MMAP(NULL, newsize, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}

static void run(void)
{
	/*
	 * Pass old_size of one page while the segment is two pages: the
	 * mismatch is intentional and must be preserved. The next page
	 * is occupied by our own mapping, so it cannot be expanded in place.
	 */
	TST_EXP_FAIL_PTR_VOID(mremap(addr, memsize, newsize, 0), ENOMEM);

	if (TST_RET_PTR != MAP_FAILED) {
		SAFE_MUNMAP(TST_RET_PTR, newsize);
		addr = SAFE_MMAP(NULL, newsize, PROT_READ | PROT_WRITE,
				 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	}
}

static void cleanup(void)
{
	if (addr != MAP_FAILED)
		SAFE_MUNMAP(addr, newsize);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run,
};
