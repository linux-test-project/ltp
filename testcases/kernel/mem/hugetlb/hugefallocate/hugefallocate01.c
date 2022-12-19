// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2015 Oracle Corporation
 * Author: Mike Kravetz
 */

/*\
 * [Description]
 *
 * It tests alignment of fallocate arguments. fallocate will take non-huge
 * page aligned offsets and addresses.  However, operations are only
 * performed on huge pages.  This is different that than fallocate
 * behavior in "normal" filesystems.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"
#include "lapi/fallocate.h"

#define MNTPOINT "hugetlbfs/"

static int  fd = -1;
static long hpage_size;

static void run_test(void)
{
	int err;
	unsigned long free_initial, free_after, free_after_delete;

	fd = tst_creat_unlinked(MNTPOINT, 0);

	free_initial = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);

	/*
	 * First preallocate file with just 1 byte.  Allocation sizes
	 * are rounded up, so we should get an entire huge page.
	 */
	err = fallocate(fd, 0, 0, 1);
	if (err) {
		if (errno == EOPNOTSUPP)
			tst_brk(TCONF, "Operation Not Supported");
		tst_res(TFAIL|TERRNO, "fallocate()");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_initial - free_after != 1) {
		tst_res(TFAIL, "fallocate 1 byte did not preallocate entire huge page");
		goto cleanup;
	}

	/*
	 * Now punch a hole with just 1 byte.  On hole punch, sizes are
	 * rounded down. So, this operation should not create a hole.
	 */
	err = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
			0, 1);
	if (err) {
		tst_res(TFAIL|TERRNO, "fallocate(FALLOC_FL_PUNCH_HOLE)");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_after == free_initial) {
		tst_res(TFAIL, "fallocate hole punch 1 byte free'ed a huge page");
		goto cleanup;
	}

	/*
	 * Now punch a hole with of 2 * hpage_size - 1 byte.  This size
	 * should be rounded down to a single huge page and the hole created.
	 */
	err = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
			0, (2 * hpage_size) - 1);
	if (err) {
		tst_res(TFAIL|TERRNO, "fallocate(FALLOC_FL_PUNCH_HOLE)");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_after != free_initial) {
		tst_res(TFAIL, "fallocate hole punch 2 * hpage_size - 1 byte did not"
				" free huge page");
		goto cleanup;
	}

	/*
	 * Perform a preallocate operation with offset 1 and size of
	 * hpage_size.  The offset should be rounded down and the
	 * size rounded up to preallocate two huge pages.
	 */
	err = fallocate(fd, 0, 1, hpage_size);
	if (err) {
		tst_res(TFAIL, "fallocate()");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_initial - free_after != 2) {
		tst_res(TFAIL, "fallocate 1 byte offset, huge page size did not"
				" preallocate two huge pages");
		goto cleanup;
	}

	/*
	 * The hole punch code will only delete 'whole' huge pags that are
	 * in the specified range.  The offset is rounded up, and (offset
	 * + size) is rounded down to determine the huge pages to be deleted.
	 * In this case, after rounding the range is (hpage_size, hpage_size).
	 * So, no pages should be deleted.
	 */
	err = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
			1, hpage_size);
	if (err) {
		tst_res(TFAIL|TERRNO, "fallocate(FALLOC_FL_PUNCH_HOLE)");
		goto cleanup;
	}

	free_after = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	if (free_initial - free_after != 2) {
		tst_res(TFAIL, "fallocate hole punch 1 byte offset, huge page size"
				" incorrectly deleted a huge page");
		goto cleanup;
	}

	/*
	 * To delete both huge pages, the range passed to hole punch must
	 * overlap the allocated pages
	 */
	err = fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
			0, 2 * hpage_size);
	if (err) {
		tst_res(TFAIL|TERRNO, "fallocate(FALLOC_FL_PUNCH_HOLE)");
		goto cleanup;
	}

	free_after_delete = SAFE_READ_MEMINFO(MEMINFO_HPAGE_FREE);
	TST_EXP_EQ_LU(free_after_delete, free_initial);
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
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
