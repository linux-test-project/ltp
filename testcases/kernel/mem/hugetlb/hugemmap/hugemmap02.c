/*
 *
 *   Copyright (c) International Business Machines  Corp., 2004
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "test.h"
#include "safe_macros.h"
#include "mem.h"
#include "hugetlb.h"

#define LOW_ADDR       0x80000000
#define LOW_ADDR2      0x90000000

static char TEMPFILE[MAXPATHLEN];

char *TCID = "hugemmap02";
int TST_TOTAL = 1;
static unsigned long *addr;
static unsigned long *addr2;
static unsigned long low_addr = LOW_ADDR;
static unsigned long low_addr2 = LOW_ADDR2;
static unsigned long *addrlist[5];
static int i;
static int fildes;
static int nfildes;
static char *Hopt;
static long hugepages = 128;

static void help(void);

int main(int ac, char **av)
{
	int lc;
	int Hflag = 0;
	long page_sz, map_sz;
	int sflag = 0;

	option_t options[] = {
		{"H:", &Hflag, &Hopt},
		{"s:", &sflag, &nr_opt},
		{NULL, NULL, NULL}
	};

	tst_parse_opts(ac, av, options, &help);

	check_hugepage();

	if (!Hflag) {
		tst_tmpdir();
		Hopt = tst_get_tmpdir();
	}
	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	page_sz = getpagesize();
	map_sz = read_meminfo("Hugepagesize:") * 1024 * 2;

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Creat a temporary file used for huge mapping */
		fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666);
		if (fildes < 0)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "opening %s failed", TEMPFILE);

		/* Creat a file used for normal mapping */
		nfildes = open("/dev/zero", O_RDONLY, 0666);
		if (nfildes < 0)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "opening /dev/zero failed");

		tst_count = 0;

		/*
		 * Call mmap on /dev/zero 5 times
		 */
		for (i = 0; i < 5; i++) {
			addr = mmap(0, 256 * 1024 * 1024, PROT_READ,
				    MAP_SHARED, nfildes, 0);
			addrlist[i] = addr;
		}

		while (range_is_mapped(cleanup, low_addr, low_addr + map_sz) == 1) {
			low_addr = low_addr + 0x10000000;

			if (low_addr < LOW_ADDR)
				tst_brkm(TBROK | TERRNO, cleanup,
						"no empty region to use");
		}
		/* mmap using normal pages and a low memory address */
		addr = mmap((void *)low_addr, page_sz, PROT_READ,
			    MAP_SHARED | MAP_FIXED, nfildes, 0);
		if (addr == MAP_FAILED)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "mmap failed on nfildes");

		while (range_is_mapped(cleanup, low_addr2, low_addr2 + map_sz) == 1) {
			low_addr2 = low_addr2 + 0x10000000;

			if (low_addr2 < LOW_ADDR2)
				tst_brkm(TBROK | TERRNO, cleanup,
						"no empty region to use");
		}
		/* Attempt to mmap a huge page into a low memory address */
		addr2 = mmap((void *)low_addr2, map_sz, PROT_READ | PROT_WRITE,
			     MAP_SHARED, fildes, 0);
#if __WORDSIZE == 64		/* 64-bit process */
		if (addr2 == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "huge mmap failed unexpectedly"
				 " with %s (64-bit)", TEMPFILE);
		} else {
			tst_resm(TPASS, "huge mmap succeeded (64-bit)");
		}
#else /* 32-bit process */
		if (addr2 == MAP_FAILED)
			tst_resm(TFAIL | TERRNO, "huge mmap failed unexpectedly"
				 " with %s (32-bit)", TEMPFILE);
		else if (addr2 > 0) {
			tst_resm(TCONF,
				 "huge mmap failed to test the scenario");
		} else if (addr == 0)
			tst_resm(TPASS, "huge mmap succeeded (32-bit)");
#endif

		/* Clean up things in case we are looping */
		for (i = 0; i < 5; i++) {
			if (munmap(addrlist[i], 256 * 1024 * 1024) == -1)
				tst_resm(TBROK | TERRNO,
					 "munmap of addrlist[%d] failed", i);
		}

		if (munmap(addr2, map_sz) == -1)
			tst_brkm(TFAIL | TERRNO, NULL, "huge munmap failed");
		if (munmap(addr, page_sz) == -1)
			tst_brkm(TFAIL | TERRNO, NULL, "munmap failed");

		close(nfildes);
		close(fildes);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;
	tst_require_root();
	if (mount("none", Hopt, "hugetlbfs", 0, NULL) < 0)
		tst_brkm(TBROK | TERRNO, NULL, "mount failed on %s", Hopt);
	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	snprintf(TEMPFILE, sizeof(TEMPFILE), "%s/mmapfile%d", Hopt, getpid());
}

void cleanup(void)
{
	unlink(TEMPFILE);
	set_sys_tune("nr_hugepages", orig_hugepages, 0);

	umount(Hopt);
	tst_rmdir();
}

static void help(void)
{
	printf("  -H /..  Location of hugetlbfs, i.e. -H /var/hugetlbfs\n");
	printf("  -s num  Set the number of the been allocated hugepages\n");
}
