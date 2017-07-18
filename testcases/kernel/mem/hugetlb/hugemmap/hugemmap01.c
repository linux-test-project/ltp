/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 */

/*
 * Test Name: hugemmap01
 *
 * Test Description:
 *  Verify that, mmap() succeeds when used to map a file in a hugetlbfs.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the hugetlb mapped region.
 *  The number of free huge pages should decrease.
 *
 * Test:
 *  Loop if the proper options are given.
 *  Execute system call
 *  Check return code, if system call failed (return=-1)
 *  Log the errno and Issue a FAIL message.
 *
 * HISTORY
 *  04/2004 Written by Robbie Williamson
 */

#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>

#include "mem.h"
#include "hugetlb.h"

static struct tst_option options[] = {
	{"H:", &Hopt,   "-H   /..  Location of hugetlbfs, i.e.  -H /var/hugetlbfs"},
	{"s:", &nr_opt, "-s   num  Set the number of the been allocated hugepages"},
	{NULL, NULL, NULL}
};

static long *addr;
static int  fildes;
static long beforetest;
static long aftertest;
static long hugepagesmapped;
static long hugepages = 128;
static char TEMPFILE[MAXPATHLEN];

static void test_hugemmap(void)
{
	long page_sz = 0;

	fildes = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);

	beforetest = SAFE_READ_MEMINFO("HugePages_Free:");

	page_sz = SAFE_READ_MEMINFO("Hugepagesize:") * 1024;

	addr = mmap(NULL, page_sz, PROT_READ | PROT_WRITE,
			MAP_SHARED, fildes, 0);

	if (addr == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "mmap() Failed on %s",
				TEMPFILE);
	} else {
		tst_res(TPASS, "call succeeded");

		/* force to allocate page and change HugePages_Free */
		*(int *)addr = 0;
		/* Make sure the number of free huge pages AFTER testing decreased */
		aftertest = SAFE_READ_MEMINFO("HugePages_Free:");
		hugepagesmapped = beforetest - aftertest;
		if (hugepagesmapped < 1)
			tst_res(TWARN, "Number of HUGEPAGES_FREE stayed the"
					" same. Okay if multiple copies running due"
					" to test collision.");
		munmap(addr, page_sz);
	}

	close(fildes);
}

void setup(void)
{
	check_hugepage();
	orig_hugepages = get_sys_tune("nr_hugepages");

	if (!Hopt)
		Hopt = tst_get_tmpdir();
	if (mount("none", Hopt, "hugetlbfs", 0, NULL) < 0)
		tst_brk(TBROK | TERRNO, "mount failed on %s", Hopt);

	if (nr_opt)
		hugepages = SAFE_STRTOL(nr_opt, 0, LONG_MAX);
	set_sys_tune("nr_hugepages", hugepages, 1);

	snprintf(TEMPFILE, sizeof(TEMPFILE), "%s/mmapfile%d", Hopt, getpid());
}

void cleanup(void)
{
	unlink(TEMPFILE);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	umount(Hopt);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = options,
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugemmap,
};
