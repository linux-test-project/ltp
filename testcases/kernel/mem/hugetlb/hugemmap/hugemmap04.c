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
 * Test Name: hugemmap04
 *
 * Test Description:
 *  Verify that, a hugetlb mmap() succeeds when used to map the largest
 *  size possible.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the hugetlb mapped region.
 *  The number of free huge pages should decrease.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Pause for SIGUSR1 if option specified.
 *   Create temporary directory.
 *
 * Test:
 *  Loop if the proper options are given.
 *  Execute system call
 *  Check return code, if system call failed (return=-1)
 *  Log the errno and Issue a FAIL message.
 * Cleanup:
 *  Print timing stats if options given
 *  Delete the temporary directory created.
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

static char TEMPFILE[MAXPATHLEN];

char *TCID = "hugemmap04";
int TST_TOTAL = 1;
static long *addr;
static long long mapsize;
static int fildes;
static long freepages;
static long beforetest;
static long aftertest;
static long hugepagesmapped;
static long hugepages = 128;
static char *Hopt;

static void help(void);

int main(int ac, char **av)
{
	int lc;
	int Hflag = 0;
	int sflag = 0;
	int huge_pagesize = 0;

	option_t options[] = {
		{"H:", &Hflag, &Hopt},
		{"s:", &sflag, &nr_opt},
		{NULL, NULL, NULL}
	};

	tst_parse_opts(ac, av, options, &help);

	if (!Hflag) {
		tst_tmpdir();
		Hopt = tst_get_tmpdir();
	}
	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Creat a temporary file used for huge mapping */
		fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666);
		if (fildes < 0)
			tst_brkm(TFAIL | TERRNO, cleanup, "open %s failed",
				 TEMPFILE);

		tst_count = 0;

		/* Note the number of free huge pages BEFORE testing */
		freepages = read_meminfo("HugePages_Free:");
		beforetest = freepages;

		/* Note the size of huge page size BEFORE testing */
		huge_pagesize = read_meminfo("Hugepagesize:");
		tst_resm(TINFO, "Size of huge pages is %d KB", huge_pagesize);

#if __WORDSIZE == 32
		tst_resm(TINFO, "Total amount of free huge pages is %d",
			 freepages);
		tst_resm(TINFO, "Max number allowed for 1 mmap file in"
			 " 32-bits is 128");
		if (freepages > 128)
			freepages = 128;
#endif
		mapsize = (long long)freepages *huge_pagesize * 1024;
		addr = mmap(NULL, mapsize, PROT_READ | PROT_WRITE,
			    MAP_SHARED, fildes, 0);
		sleep(2);
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL | TERRNO, "mmap() Failed on %s",
				 TEMPFILE);
			close(fildes);
			continue;
		} else {
			tst_resm(TPASS,
				 "Succeeded mapping file using %ld pages",
				 freepages);
			/* force to allocate page and change HugePages_Free */
			*(int *)addr = 0;
		}

		/*
		 * Make sure the number of free huge pages
		 * AFTER testing decreased
		 */
		aftertest = read_meminfo("HugePages_Free:");
		hugepagesmapped = beforetest - aftertest;
		if (hugepagesmapped < 1)
			tst_resm(TWARN, "Number of HUGEPAGES_FREE stayed the"
				 " same. Okay if multiple copies running due"
				 " to test collision.");

		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
		if (munmap(addr, mapsize) != 0)
			tst_brkm(TFAIL | TERRNO, NULL, "munmap failed");

		close(fildes);
	}

	cleanup();
	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;
	tst_require_root();
	check_hugepage();
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
}
