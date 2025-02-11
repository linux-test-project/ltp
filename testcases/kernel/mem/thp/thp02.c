// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2011-2017  Red Hat, Inc.
 */

/*\
 * Test for detecting mremap bug when THP is enabled.
 *
 * There was a bug in mremap THP support, sometimes causing crash
 * due to the following reason:
 *
 * "alloc_new_pmd was forcing the allocation of a pte before calling
 * move_huge_page and that resulted in a VM_BUG_ON in move_huge_page
 * because the pmd wasn't zero."
 *
 * There are 4 cases to test this bug:
 *
 * 1. old_addr hpage aligned, old_end not hpage aligned, new_addr
 *    hpage aligned
 *
 * 2. old_addr hpage aligned, old_end not hpage aligned, new_addr not
 *    hpage aligned
 *
 * 3. old_addr not hpage aligned, old_end hpage aligned, new_addr
 *    hpage aligned
 *
 * 4. old_addr not hpage aligned, old_end hpage aligned, new_addr not
 *    hpage aligned
 */

#define _GNU_SOURCE
#include "config.h"
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "tst_test.h"
#include "thp.h"

static int ps;
static long hps, size;

/*
 * Will try to do the following 4 mremaps cases:
 *   mremap(p, size-ps, size-ps, flag, p2);
 *   mremap(p, size-ps, size-ps, flag, p2+ps);
 *   mremap(p+ps, size-ps, size-ps, flag, p2);
 *   mremap(p+ps, size-ps, size-ps, flag, p2+ps);
 */
static void do_child(int i)
{
	long j, remap_size;
	unsigned char *p1, *p2, *ret, *old_addr, *new_addr;

	p1 = SAFE_MEMALIGN(hps, size);
	p2 = SAFE_MEMALIGN(hps, size);

	memset(p1, 0xff, size);
	memset(p2, 0x77, size);

	old_addr = p1 + ps * (i >> 1);
	new_addr = p2 + ps * (i & 1);
	remap_size = size - ps;

	tst_res(TINFO, "mremap (%p-%p) to (%p-%p)",
		old_addr, old_addr + remap_size,
		new_addr, new_addr + remap_size);

	ret = mremap(old_addr, remap_size, remap_size,
		    MREMAP_FIXED | MREMAP_MAYMOVE, new_addr);
	if (ret == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mremap");

	for (j = 0; j < remap_size; j++) {
		if (ret[j] != 0xff)
			tst_brk(TBROK, "mremap bug");
	}

	exit(0);
}

static void do_mremap(void)
{
	int i;

	for (i = 0; i < 4; i++) {
		if (SAFE_FORK() == 0)
			do_child(i);
		tst_reap_children();
	}
	tst_res(TPASS, "Still alive.");
}

static void setup(void)
{
	long memfree;

	if (access(PATH_THP, F_OK) == -1)
		tst_brk(TCONF, "THP not enabled in kernel?");

	check_hugepage();

	ps = sysconf(_SC_PAGESIZE);
	hps = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;
	size = hps * 4;

	memfree = (SAFE_READ_MEMINFO("MemFree:") * 1024 +
		SAFE_READ_MEMINFO("Cached:") * 1024);
	if (memfree < size * 2)
		tst_brk(TCONF, "not enough memory");
}

static struct tst_test test = {
	.setup = setup,
	.test_all = do_mremap,
	.forks_child = 1,
};
