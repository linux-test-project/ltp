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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>

#include "test.h"
#include "usctest.h"
#include "system_specific_hugepages_info.h"

#define HIGH_ADDR      (void *)(0x1000000000000)

static char *TEMPFILE = "mmapfile";

char *TCID = "hugemmap03";
int TST_TOTAL = 1;
static unsigned long *addr;
static int fildes;
static char *Hopt;

static void setup(void);
static void cleanup(void);
static void help(void);

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int Hflag = 0;
	int page_sz;

#if __WORDSIZE == 32  /* 32-bit compiled */
	tst_brkm(TCONF, NULL, "This test is only for 64bit");
#endif

	option_t options[] = {
		{ "H:",   &Hflag, &Hopt },
		{ NULL, NULL, NULL }
	};

	msg = parse_opts(ac, av, options, &help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s, use -help",
			 msg);

	if (!Hflag)
		tst_brkm(TBROK, NULL, "-H option is REQUIRED for this test, "
			 "use -h for options help");

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
				 " unexpectedly succeeded on %s, TEMPFILE");
			continue;
		} else {
			tst_resm(TPASS, "Normal mmap() into high region"
				 " failed correctly");
			break;
		}

		close(fildes);
	}

	cleanup();

	tst_exit();
}

static void setup(void)
{
	char mypid[40];

	sprintf(mypid, "/%d", getpid());
	TEMPFILE = strcat(mypid, TEMPFILE);
	TEMPFILE = strcat(Hopt, TEMPFILE);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

static void cleanup(void)
{
	TEST_CLEANUP;

	unlink(TEMPFILE);

}

static void help(void)
{
	printf("  -H /..  Location of hugetlbfs, i.e. -H /var/hugetlbfs\n");
}
