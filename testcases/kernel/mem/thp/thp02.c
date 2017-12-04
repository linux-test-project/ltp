/*
 * Copyright (C) 2011-2017  Red Hat, Inc.
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

/* thp02 - detect mremap bug when THP is enabled.
 *
 * There was a bug in mremap THP support, sometimes crash happened
 * due to the following reason according to developers:
 *
 * "alloc_new_pmd was forcing the allocation of a pte before calling
 * move_huge_page and that resulted in a VM_BUG_ON in move_huge_page
 * because the pmd wasn't zero."
 *
 * There are 4 cases to test this bug:
 *
 * 1) old_addr hpage aligned, old_end not hpage aligned, new_addr
 *    hpage aligned;
 * 2) old_addr hpage aligned, old_end not hpage aligned, new_addr not
 *    hpage aligned;
 * 3) old_addr not hpage aligned, old_end hpage aligned, new_addr
 *    hpage aligned;
 * 4) old_addr not hpage aligned, old_end hpage aligned, new_addr not
 *    hpage aligned.
 *
 */

#define _GNU_SOURCE
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mem.h"

#ifdef HAVE_MREMAP_FIXED
static int ps;
static long hps, size;
static void *p, *p2, *p3, *p4;

static void do_mremap(void)
{
	int i;
	void *old_addr, *new_addr;

	for (i = 0; i < 4; i++) {
		p = SAFE_MEMALIGN(hps, size);
		p2 = SAFE_MEMALIGN(hps, size);
		p3 = SAFE_MEMALIGN(hps, size);

		memset(p, 0xff, size);
		memset(p2, 0xff, size);
		memset(p3, 0x77, size);

		/*
		 * Will try to do the following 4 mremaps cases:
		 *   mremap(p, size-ps, size-ps, flag, p3);
		 *   mremap(p, size-ps, size-ps, flag, p3+ps);
		 *   mremap(p+ps, size-ps, size-ps, flag, p3);
		 *   mremap(p+ps, size-ps, size-ps, flag, p3+ps);
		 */
		old_addr = p + ps * (i >> 1);
		new_addr = p3 + ps * (i & 1);
		tst_res(TINFO, "mremap %p to %p", old_addr, new_addr);

		p4 = mremap(old_addr, size - ps, size - ps,
			    MREMAP_FIXED | MREMAP_MAYMOVE, new_addr);
		if (p4 == MAP_FAILED)
			tst_brk(TBROK | TERRNO, "mremap");
		if (memcmp(p4, p2, size - ps))
			tst_brk(TBROK, "mremap bug");
	}

	tst_res(TPASS, "Still alive.");
}

static void setup(void)
{
	if (access(PATH_THP, F_OK) == -1)
		tst_brk(TCONF, "THP not enabled in kernel?");

	check_hugepage();

	ps = sysconf(_SC_PAGESIZE);
	hps = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	size = hps * 4;
}

static struct tst_test test = {
	.needs_root = 1,
	.setup = setup,
	.test_all = do_mremap,
};

#else
	TST_TEST_TCONF("MREMAP_FIXED not present in <sys/mman.h>");
#endif /* HAVE_MREMAP_FIXED */
