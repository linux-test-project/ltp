// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * At one stage, a misconversion of hugetlb_vmtruncate_list to a prio_tree
 * meant that on 32-bit machines, certain combinations of mapping and
 * truncations could truncate incorrect pages, or overwrite pmds from
 * other VMAs, triggering BUG_ON()s or other wierdness.
 *
 * Test adapted from an example by Kenneth Chen <kenneth.w.chen@intel.com>
 *
 * WARNING: The offsets and addresses used within are specifically
 * calculated to trigger the bug as it existed.  Don't mess with them
 * unless you *really* know what you're doing.
 *
 * The kernel bug in question was fixed with commit
 * 856fc2950555.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
#define MAP_LENGTH	(4UL * hpage_size)
#if defined(__s390__) && __WORDSIZE == 32
#define TRUNCATE_POINT 0x20000000UL
#else
#define TRUNCATE_POINT 0x60000000UL
#endif
#define HIGH_ADDR	0xa0000000UL
#define FOURGIG		((off64_t)0x100000000ULL)

static unsigned long hpage_size;
static int  fd = -1;

static void run_test(void)
{
	char *p, *q;
	unsigned long i;

	p = SAFE_MMAP(0, MAP_LENGTH + TRUNCATE_POINT, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE | MAP_NORESERVE, fd, 0);

	SAFE_MUNMAP(p, MAP_LENGTH + TRUNCATE_POINT);

	q = SAFE_MMAP((void *)HIGH_ADDR, MAP_LENGTH, PROT_READ | PROT_WRITE,
		 MAP_PRIVATE, fd, 0);
	tst_res(TINFO, "High map at %p", q);

	for (i = 0; i < MAP_LENGTH; i += hpage_size)
		q[i] = 1;

	SAFE_FTRUNCATE(fd, TRUNCATE_POINT);

	if (q[0] != 1)
		tst_res(TFAIL, "data mismatch");
	else
		tst_res(TPASS, "Successful");

	SAFE_MUNMAP(q, MAP_LENGTH);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;

	if (hpage_size > TRUNCATE_POINT)
		tst_brk(TCONF, "Huge page size is too large");
	if (TRUNCATE_POINT % hpage_size)
		tst_brk(TCONF, "Truncation point is not aligned to huge page size");
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "856fc2950555"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {4, TST_NEEDS},
};
