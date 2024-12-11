// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2005-2007 David Gibson & Adam Litke, IBM Corporation.
 * Author: David Gibson & Adam Litke
 */

/*\
 * [Description]
 *
 * Just as normal mmap()s can't have an address, length or offset which
 * is not page aligned, so hugepage mmap()s can't have an address, length
 * or offset with is not hugepage aligned.
 *
 * However, from time to time when the various mmap() /
 * get_unmapped_area() paths are updated, somebody misses one of the
 * necessary checks for the hugepage paths.  This testcase ensures
 * that attempted hugepage mappings with parameters which are not
 * correctly hugepage aligned are rejected.
 *
 * However starting with 3.10-rc1, length passed in mmap() doesn't need
 * to be aligned because commit af73e4d9506d3b797509f3c030e7dcd554f7d9c4
 * added ALIGN() to kernel side, in mmap_pgoff(), when mapping huge page
 * files.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int  fd = -1;
static long page_size;

static void run_test(void)
{
	void *p, *q;

	/*
	 * First see what an ok mapping looks like, as a basis for our
	 * bad addresses and so forth
	 */
	p = mmap(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (p == MAP_FAILED) {
		tst_res(TFAIL|TERRNO, "mmap() without hint failed");
		return;
	}
	if (((unsigned long)p % hpage_size) != 0) {
		tst_res(TFAIL, "mmap() without hint at misaligned address");
		goto cleanup1;
	}

	tst_res(TINFO, "Mapped at %p, length 0x%lx", p, hpage_size);

	SAFE_MUNMAP(p, hpage_size);

	/* 1) Try a misaligned hint address */
	q = mmap(p + page_size, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_PRIVATE, fd, 0);
	if (q == MAP_FAILED) {
		/* Bad hint shouldn't fail, just ignore the hint */
		tst_res(TFAIL|TERRNO, "mmap() with hint failed");
		return;
	}
	if (((unsigned long)q % hpage_size) != 0) {
		tst_res(TFAIL, "mmap() with hint at misaligned address");
		goto cleanup2;
	}
	SAFE_MUNMAP(q, hpage_size);

	/* 2) Try a misaligned address with MAP_FIXED */
	q = mmap(p + page_size, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_PRIVATE|MAP_FIXED, fd, 0);
	if (q != MAP_FAILED) {
		tst_res(TFAIL, "mmap() MAP_FIXED at misaligned address succeeded");
		goto cleanup2;
	}

	/* 3) Try a misaligned length */
	q = mmap(NULL, page_size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (q == MAP_FAILED) {
		tst_res(TFAIL, "mmap() with misaligned length 0x%lx failed",
			page_size);
		return;
	}
	SAFE_MUNMAP(q, hpage_size);

	/* 4) Try a misaligned length with MAP_FIXED */
	q = mmap(p, page_size, PROT_READ|PROT_WRITE,
			MAP_PRIVATE|MAP_FIXED, fd, 0);
	if (q == MAP_FAILED) {
		tst_res(TFAIL, "mmap() MAP_FIXED with misaligned length 0x%lx "
			"failed", page_size);
		return;
	}
	SAFE_MUNMAP(q, hpage_size);

	/* 5) Try a misaligned offset */
	q = mmap(NULL, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_PRIVATE, fd, page_size);
	if (q != MAP_FAILED) {
		tst_res(TFAIL, "mmap() with misaligned offset 0x%lx succeeded",
		     page_size);
		goto cleanup2;
	}

	/* 6) Try a misaligned offset with MAP_FIXED*/
	q = mmap(p, hpage_size, PROT_READ|PROT_WRITE,
		 MAP_PRIVATE|MAP_FIXED, fd, page_size);
	if (q != MAP_FAILED) {
		tst_res(TFAIL, "mmap() MAP_FIXED with misaligned offset 0x%lx succeeded",
		     page_size);
		goto cleanup2;
	}

	tst_res(TPASS, "mmap worked as expected with misaligned addr and length");
	return;
cleanup2:
	SAFE_MUNMAP(q, hpage_size);
	return;
cleanup1:
	SAFE_MUNMAP(p, hpage_size);
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO("Hugepagesize:")*1024;
	page_size = getpagesize();
	fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	if (fd >= 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.tags = (struct tst_tag[]) {
		{"linux-git", "af73e4d9506d"},
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
