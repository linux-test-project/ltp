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
 * Usage:  <for command-line>
 *  hugemmap03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	04/2004 Written by Robbie Williamson
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

#define PAGE_SIZE      ((1UL) << 12) 	/* Normal page size */
#define HIGH_ADDR      (void *)(0x1000000000000)

char* TEMPFILE="mmapfile";

char *TCID="hugemmap03";	/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
unsigned long *addr;		/* addr of memory mapped region */
int fildes;			/* file descriptor for tempfile */
char *Hopt;                     /* location of hugetlbfs */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

void help()
{
	printf("  -H /..  Location of hugetlbfs, i.e. -H /var/hugetlbfs \n");
}

int
main(int ac, char **av)
{
#if __WORDSIZE==32  /* 32-bit compiled */
      	tst_resm(TCONF,"This test is only for 64bit");
	tst_exit();

       	return 1;
#else	/* 64-bit compiled */
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
        int Hflag=0;              /* binary flag: opt or not */

       	option_t options[] = {
        	{ "H:",   &Hflag, &Hopt },    /* Required for location of hugetlbfs */
            	{ NULL, NULL, NULL }          /* NULL required to end array */
       	};

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, options, &help);
	if (msg != (char *) NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s, use -help", msg);
		tst_exit();
	}

	if (Hflag == 0) {
		tst_brkm(TBROK, NULL, "-H option is REQUIRED for this test, use -h for options help");
		tst_exit();
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

	        /* Creat a temporary file used for huge mapping */
		if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "open() on %s Failed, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
		}

		Tst_count=0;

		/* Attempt to mmap using normal pages and a high memory address */
		errno = 0;
		addr = mmap(HIGH_ADDR, PAGE_SIZE, PROT_READ,
			    MAP_SHARED | MAP_FIXED, fildes, 0);
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL, "Normal mmap() into high region unexpectedly succeeded on %s, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		} else {
			tst_resm(TPASS, "Normal mmap() into high region failed correctly");
			break;
		}

		close(fildes);
	}

	cleanup();

	tst_exit();
#endif
}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 * 	     Get system page size, allocate and initialize the string dummy.
 * 	     Initialize addr such that it is more than one page below the break
 * 	     address of the process, and initialize one page region from addr
 * 	     with char 'A'.
 * 	     Creat a temporary directory and a file under it.
 * 	     Write some known data into file and get the size of the file.
 */
void
setup()
{
	char mypid[40];

	sprintf(mypid,"/%d",getpid());
	TEMPFILE=strcat(mypid,TEMPFILE);
	TEMPFILE=strcat(Hopt,TEMPFILE);

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 * 	       Remove the temporary directory created.
 */
void
cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	TEST_CLEANUP;

	unlink(TEMPFILE);

}
