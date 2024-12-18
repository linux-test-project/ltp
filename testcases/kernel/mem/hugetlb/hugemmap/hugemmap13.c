// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2006 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * On some old ppc64 kernel, when hpage is mmaped on 32 bit boundary and
 * normal page below it, it triggers the bug caused by off-by-one error.
 *
 * WARNING: The offsets and addresses used within are specifically
 * calculated to trigger the bug as it existed. Don't mess with them
 * unless you *really* know what you're doing.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>
#include <lapi/mmap.h>

#include "hugetlb.h"

#define FOURGB (1ULL << 32)
#define MNTPOINT "hugetlbfs/"
static int  fd = -1;
static unsigned long hpage_size;
static int page_size;

static void run_test(void)
{
	void *p, *q = NULL;
	unsigned long long lowaddr;
	unsigned long long below_start;
	unsigned long long above_end;

	p = mmap((void *)FOURGB, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_SHARED | MAP_FIXED, fd, 0);
	if (p == MAP_FAILED) {
		/* slice 0 (high) spans from 4G-1T */
		below_start = FOURGB;
		above_end = 1024ULL*1024*1024*1024;

		if (tst_mapping_in_range(below_start, above_end) == 1) {
			tst_res(TINFO|TERRNO, "region 4G-IT is not free & "
					"mmap() failed expected");
			tst_res(TPASS, "Successful but inconclusive");
		} else
			tst_res(TFAIL|TERRNO, "mmap() huge failed unexpected");
		goto cleanup;
	}
	if (p != (void *)FOURGB) {
		tst_res(TFAIL, "Wrong address with MAP_FIXED huge");
		goto cleanup;
	}

	tst_res(TINFO, "Mapped hugetlb at %p", p);

	memset(p, 0, hpage_size);

	/* Test just below 4GB to check for off-by-one errors */
	lowaddr = FOURGB - MMAP_GRANULARITY;
	q = mmap((void *)lowaddr, MMAP_GRANULARITY, PROT_READ|PROT_WRITE,
		 MAP_SHARED|MAP_FIXED|MAP_ANONYMOUS, 0, 0);
	if (q == MAP_FAILED) {
		below_start = FOURGB - MMAP_GRANULARITY;
		above_end = FOURGB;

		if (tst_mapping_in_range(below_start, above_end) == 1) {
			tst_res(TINFO|TERRNO, "region (4G-MMAP_GRANULARITY)-4G is not free & "
					"mmap() failed expected");
			tst_res(TPASS, "Successful but inconclusive");
		} else
			tst_res(TFAIL|TERRNO, "mmap() normal failed unexpected");
		goto cleanup;
	}
	if (q != (void *)lowaddr) {
		tst_res(TFAIL, "Wrong address with MAP_FIXED normal");
		goto cleanup;
	}

	memset(q, 0, page_size);
	tst_res(TPASS, "Successful");

cleanup:
	if (p && p != MAP_FAILED)
		SAFE_MUNMAP(p, hpage_size);
	if (q && q != MAP_FAILED)
		SAFE_MUNMAP(q, page_size);
}

static void setup(void)
{
	page_size = getpagesize();
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;

	if (sizeof(void *) <= 4)
		tst_brk(TCONF, "Machine must be >32 bit");
	if (hpage_size > FOURGB)
		tst_brk(TCONF, "Huge page size is too large");
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd > 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "9a94c5793a7b"},
		{}
	},
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.needs_tmpdir = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
