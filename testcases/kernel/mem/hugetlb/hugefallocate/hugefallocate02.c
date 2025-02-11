// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2015 Oracle Corporation
 * Author: Mike Kravetz
 */

/*\
 * It tests basic fallocate functionality in hugetlbfs. Preallocate huge
 * pages to a file in hugetlbfs, and then remove the pages via hole punch.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"
#include "lapi/fallocate.h"

#define MAX_PAGES_TO_USE 5
#define MNTPOINT "hugetlbfs/"

static int  fd = -1;
static long hpage_size;

static void run_test(void)
{
	int err;
	unsigned long max_iterations;
	unsigned long free_initial, free_after, free_end;

	free_initial = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	max_iterations = MIN(free_initial, MAX_PAGES_TO_USE);

	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);

	/* First preallocate file with max_iterations pages */
	err = fallocate(fd, 0, 0, hpage_size * max_iterations);
	if (err) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, "fallocate() Operation is not supported");
		tst_res(TFAIL|TERRNO, "fallocate()");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_initial - free_after != max_iterations) {
		tst_res(TFAIL, "fallocate did not preallocate %lu huge pages",
							max_iterations);
		goto cleanup;
	}

	/* Now punch a hole of the same size */
	err = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
			0, hpage_size * max_iterations);
	if (err) {
		tst_res(TFAIL|TERRNO, "fallocate(FALLOC_FL_PUNCH_HOLE)");
		goto cleanup;
	}

	free_end = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	TST_EXP_EQ_LU(free_end, free_initial);
cleanup:
	SAFE_CLOSE(fd);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {3, TST_NEEDS},
};
