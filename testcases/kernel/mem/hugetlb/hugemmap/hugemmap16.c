// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: Eric B Munson and Mel Gorman
 */

/*\
 * madvise() on some kernels can cause the reservation counter to get
 * corrupted. The problem is that the patches are allocated
 * for the reservation but not faulted in at the time of allocation. The
 * counters do not get updated and effectively "leak". This test
 * identifies whether the kernel is vulnerable to the problem or not.
 * It is fixed in kernel by commit f2deae9d4e70
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static int  fd = -1;
static long hpage_size;

static void run_test(void)
{
	void *p;
	unsigned long initial_rsvd, map_rsvd, madvise_rsvd, end_rsvd;

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	initial_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "Reserve count before map: %lu", initial_rsvd);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 fd, 0);
	map_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "Reserve count after map: %lu", map_rsvd);

	if (madvise(p, hpage_size, MADV_WILLNEED) == -1)
		tst_brk(TBROK|TERRNO, "madvise()");
	madvise_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "Reserve count after madvise: %lu", madvise_rsvd);

	SAFE_MUNMAP(p, hpage_size);
	SAFE_CLOSE(fd);
	end_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "Reserve count after close(): %lu", end_rsvd);

	TST_EXP_EQ_LU(end_rsvd, initial_rsvd);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "f2deae9d4e70"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {1, TST_NEEDS},
};
