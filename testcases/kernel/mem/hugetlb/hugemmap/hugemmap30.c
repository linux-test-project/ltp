// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 IBM Corporation.
 * Author: Mel Gorman
 */

/*\
 * [Description]
 *
 * readahead() on some kernels can cause the reservation counter to get
 * corrupted. The problem is that the pages are allocated for the
 * reservation but not faulted in at the time of allocation. The
 * counters do not get updated and effectively "leak". This test
 * identifies whether the kernel is vulnerable to the problem or not.
 * It's fixed in kernel by commit f2deae9d4e70.
 */

#define _GNU_SOURCE
#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int fd = -1;

static void run_test(void)
{
	void *p;
	unsigned long initial_rsvd, map_rsvd, readahead_rsvd, end_rsvd;

	fd = tst_creat_unlinked(MNTPOINT, 0);
	initial_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		 fd, 0);
	map_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "map_rsvd: %lu", map_rsvd);

	readahead(fd, 0, hpage_size);
	readahead_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);
	tst_res(TINFO, "readahead_rsvd: %lu", readahead_rsvd);

	memset(p, 1, hpage_size);

	SAFE_MUNMAP(p, hpage_size);
	SAFE_CLOSE(fd);
	end_rsvd = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(end_rsvd, initial_rsvd);
}

static void setup(void)
{
	hpage_size = tst_get_hugepage_size();
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
