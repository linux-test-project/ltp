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
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * Test Name: hugemmap03
 *
 * Test Description: Test that a normal page cannot be mapped into a high
 * memory region.
 *
 * HISTORY
 *  04/2004 Written by Robbie Williamson
 *
 * RESTRICTIONS:
 *  Must be compiled in 64-bit mode.
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
#include "usctest.h"
#include "safe_macros.h"
#include "mem.h"

#define HIGH_ADDR      (void *)(0x1000000000000)

static char TEMPFILE[MAXPATHLEN];

char *TCID = "hugemmap03";
int TST_TOTAL = 1;
static unsigned long *addr;
static int fildes;
static char *Hopt;
static char *nr_opt;
static long hugepages = 128;
static long orig_hugepages;

static void help(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int Hflag = 0;
	int sflag = 0;
	int page_sz;

#if __WORDSIZE == 32  /* 32-bit compiled */
	tst_brkm(TCONF, NULL, "This test is only for 64bit");
#endif

	option_t options[] = {
		{ "H:", &Hflag, &Hopt },
		{ "s:", &sflag, &nr_opt },
		{ NULL, NULL, NULL }
	};

	msg = parse_opts(ac, av, options, &help);
	if (msg)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s, use -help",
			 msg);

	if (!Hflag) {
		tst_tmpdir();
		Hopt = get_tst_tmpdir();
	}
	if (sflag)
		hugepages = SAFE_STRTOL(NULL, nr_opt, 0, LONG_MAX);

	page_sz = getpagesize();

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Creat a temporary file used for huge mapping */
		fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666);
		if (fildes < 0)
			tst_brkm(TBROK|TERRNO, cleanup,
				 "opening %s failed", TEMPFILE);

		Tst_count = 0;

		/*
		 * Attempt to mmap using normal pages and
		 * a high memory address
		 */
		addr = mmap(HIGH_ADDR, page_sz, PROT_READ,
			    MAP_SHARED | MAP_FIXED, fildes, 0);
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL|TERRNO, "Normal mmap() into high region"
				 " unexpectedly succeeded on %s", TEMPFILE);
			close(fildes);
			continue;
		} else {
			tst_resm(TPASS, "Normal mmap() into high region"
				 " failed correctly");
			close(fildes);
			break;
		}

		close(fildes);
	}

	cleanup();

	tst_exit();
}

void setup(void)
{
	TEST_PAUSE;
	tst_require_root(NULL);
	if (mount("none", Hopt, "hugetlbfs", 0, NULL) < 0)
		tst_brkm(TBROK|TERRNO, NULL,
			 "mount failed on %s", Hopt);
	orig_hugepages = get_sys_tune("nr_hugepages");
	set_sys_tune("nr_hugepages", hugepages, 1);
	snprintf(TEMPFILE, sizeof(TEMPFILE), "%s/mmapfile%d",
		 Hopt, getpid());
}

void cleanup(void)
{
	TEST_CLEANUP;

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
