// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
 * AUTHOR: Nirmala Devi Dhanasekar <nirmala.devi@wipro.com>
 */

/*\
 * Test for ENOMEM error.
 *
 * munlock(2) fails with ENOMEM if some of the specified address range
 * does not correspond to mapped pages in the address space of the
 * process.
 */

#include <sys/mman.h>
#include "tst_test.h"

static size_t len, pg_size;
static void *addr;

static void run(void)
{
	TST_EXP_FAIL(munlock(addr, len), ENOMEM, "munlock(%p, %lu)",
		      addr, len);
}

static void setup(void)
{
	pg_size = getpagesize();
	len = 8 * pg_size;
	addr = SAFE_MMAP(NULL, len, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	memset(addr, 0x20, len);
	SAFE_MLOCK(addr, len);
	/*
	 * unmap part of the area, to create the condition for ENOMEM
	 */
	addr += 2 * pg_size;
	SAFE_MUNMAP(addr, 4 * pg_size);
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = run,
};
