// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2004
 * Copyright (c) Linux Test Project, 2004-2017
 *
 * Test Name: hugemmap02
 *
 * Test Description: There is both a low hugepage region (at 2-3G for use by
 * 32-bit processes) and a high hugepage region (at 1-1.5T).  The high region
 * is always exclusively for hugepages, but the low region has to be activated
 * before it can be used for hugepages.  When the kernel attempts to do a
 * hugepage mapping in a 32-bit process it will automatically attempt to open
 * the low region.  However, that will fail if there are any normal
 * (non-hugepage) mappings in the region already.
 *
 * When run as a 64-bit process the kernel will still do a non-hugepage mapping
 * in the low region, but the following hugepage mapping will succeed. This is
 * because it comes from the high region, which is available to the 64-bit
 * process.
 *
 * This test case is checking this behavior.
 *
 * HISTORY
 *  04/2004 Written by Robbie Williamson
 */

#include <stdio.h>
#include <sys/mount.h>
#include <limits.h>
#include <sys/param.h>
#include "lapi/abisize.h"
#include "hugetlb.h"

#define LOW_ADDR       0x80000000
#define LOW_ADDR2      0x90000000

static char TEMPFILE[MAXPATHLEN];

static unsigned long *addr;
static unsigned long *addr2;
static unsigned long low_addr = LOW_ADDR;
static unsigned long low_addr2 = LOW_ADDR2;
static unsigned long *addrlist[5];
static int fildes;
static int nfildes;

static void test_hugemmap(void)
{
	int i;
	long page_sz, map_sz;

	page_sz = getpagesize();
	map_sz = SAFE_READ_MEMINFO("Hugepagesize:") * 1024 * 2;

	fildes = SAFE_OPEN(TEMPFILE, O_RDWR | O_CREAT, 0666);

	nfildes = SAFE_OPEN("/dev/zero", O_RDONLY, 0666);

	for (i = 0; i < 5; i++) {
		addr = mmap(0, 256 * 1024 * 1024, PROT_READ,
				MAP_SHARED, nfildes, 0);
		addrlist[i] = addr;
	}

	while (tst_mapping_in_range(low_addr, low_addr + map_sz) == 1) {
		low_addr = low_addr + 0x10000000;

		if (low_addr < LOW_ADDR)
			tst_brk(TBROK | TERRNO, "no empty region to use");
	}
	/* mmap using normal pages and a low memory address */
	addr = mmap((void *)low_addr, page_sz, PROT_READ,
			MAP_SHARED | MAP_FIXED, nfildes, 0);
	if (addr == MAP_FAILED)
		tst_brk(TBROK | TERRNO, "mmap failed on nfildes");

	while (tst_mapping_in_range(low_addr2, low_addr2 + map_sz) == 1) {
		low_addr2 = low_addr2 + 0x10000000;

		if (low_addr2 < LOW_ADDR2)
			tst_brk(TBROK | TERRNO, "no empty region to use");
	}
	/* Attempt to mmap a huge page into a low memory address */
	addr2 = mmap((void *)low_addr2, map_sz, PROT_READ | PROT_WRITE,
			MAP_SHARED, fildes, 0);
#ifdef TST_ABI64 /* 64-bit process */
	if (addr2 == MAP_FAILED) {
		tst_res(TFAIL | TERRNO, "huge mmap failed unexpectedly"
				" with %s (64-bit)", TEMPFILE);
	} else {
		tst_res(TPASS, "huge mmap succeeded (64-bit)");
	}
#else /* 32-bit process */
	if (addr2 == MAP_FAILED)
		tst_res(TFAIL | TERRNO, "huge mmap failed unexpectedly"
				" with %s (32-bit)", TEMPFILE);
	else if (addr2 > 0) {
		tst_res(TCONF,
				"huge mmap failed to test the scenario");
	} else if (addr == 0)
		tst_res(TPASS, "huge mmap succeeded (32-bit)");
#endif

	for (i = 0; i < 5; i++) {
		if (munmap(addrlist[i], 256 * 1024 * 1024) == -1)
			tst_res(TFAIL | TERRNO,
					"munmap of addrlist[%d] failed", i);
	}

	if (munmap(addr2, map_sz) == -1)
		tst_res(TFAIL | TERRNO, "huge munmap failed");
	if (munmap(addr, page_sz) == -1)
		tst_res(TFAIL | TERRNO, "munmap failed");

	close(nfildes);
	close(fildes);
}

static void setup(void)
{
	if (tst_hugepages == 0)
		tst_brk(TCONF, "Not enough hugepages for testing.");

	if (!Hopt)
		Hopt = tst_tmpdir_path();
	SAFE_MOUNT("none", Hopt, "hugetlbfs", 0, NULL);

	snprintf(TEMPFILE, sizeof(TEMPFILE), "%s/mmapfile%d", Hopt, getpid());
}

static void cleanup(void)
{
	unlink(TEMPFILE);
	umount(Hopt);
}

static struct tst_test test = {
	.needs_root = 1,
	.needs_tmpdir = 1,
	.options = (struct tst_option[]) {
		{"H:", &Hopt,   "Location of hugetlbfs, i.e.  -H /var/hugetlbfs"},
		{"s:", &nr_opt, "Set the number of the been allocated hugepages"},
		{}
	},
	.setup = setup,
	.cleanup = cleanup,
	.test_all = test_hugemmap,
	.hugepages = {128, TST_REQUEST},
};
