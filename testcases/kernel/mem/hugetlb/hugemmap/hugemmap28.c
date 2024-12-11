// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2013 LG Electronics.
 * Author: Joonsoo Kim
 */

/*\
 * [Description]
 *
 * Test to correct handling for reserve count. If no reserved mapping is
 * created to reserved file region, it should be considered as reserve
 * mapping. Otherwise, reserve count will be overflowed.
 */

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int fd = -1;

static void run_test(void)
{
	unsigned long initial_resv, end_resv;
	int fd;
	char *p, *q;

	initial_resv = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
	p = SAFE_MMAP(NULL, hpage_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	q = SAFE_MMAP(NULL, hpage_size,
		PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, fd, 0);

	*q = 's';

	SAFE_MUNMAP(p, hpage_size);
	SAFE_MUNMAP(q, hpage_size);
	SAFE_CLOSE(fd);

	end_resv = SAFE_READ_MEMINFO(MEMINFO_HPAGE_RSVD);

	TST_EXP_EQ_LU(initial_resv, end_resv);
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
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
