// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 * Copyright (C) 2013 Joonsoo Kim, LG Electronics.
 * Author: Joonsoo Kim
 */

/*\
 * [Description]
 *
 * Test sanity of cow optimization on page cache. If a page in page cache
 * has only 1 ref count, it is mapped for a private mapping directly and
 * is overwritten freely, so next time we access the page, we can see
 * corrupt data.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/types.h>

#include "hugetlb.h"

#define MNTPOINT "hugetlbfs/"
static long hpage_size;
static int huge_fd = -1;

static void run_test(void)
{
	char *p;
	char c;

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
			huge_fd, 0);
	*p = 's';
	tst_res(TINFO, "Write %c to %p via shared mapping", *p, p);
	SAFE_MUNMAP(p, hpage_size);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_PRIVATE,
			huge_fd, 0);
	*p = 'p';
	tst_res(TINFO, "Write %c to %p via private mapping", *p, p);
	SAFE_MUNMAP(p, hpage_size);

	p = SAFE_MMAP(NULL, hpage_size, PROT_READ|PROT_WRITE, MAP_SHARED,
			huge_fd, 0);
	c = *p;
	tst_res(TINFO, "Read %c from %p via shared mapping", *p, p);
	SAFE_MUNMAP(p, hpage_size);

	/* Direct write from huge page */
	if (c != 's')
		tst_res(TFAIL, "Data got corrupted.");
	else
		tst_res(TPASS, "Successful");
}

static void setup(void)
{
	hpage_size = SAFE_READ_MEMINFO(MEMINFO_HPAGE_SIZE)*1024;
	huge_fd = tst_creat_unlinked(MNTPOINT, 0, 0600);
}

static void cleanup(void)
{
	SAFE_CLOSE(huge_fd);
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = MNTPOINT,
	.needs_hugetlbfs = 1,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run_test,
	.hugepages = {2, TST_NEEDS},
};
