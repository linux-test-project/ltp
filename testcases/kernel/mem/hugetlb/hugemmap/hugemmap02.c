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
 * Test Name: hugemmap02
 *
 * Test Description: There is both a low hugepage region (at 2-3G for use by 32-bit
 * processes) and a high hugepage region (at 1-1.5T).  The high region is always
 * exclusively for hugepages, but the low region has to be activated before it can
 * be used for hugepages.  When the kernel attempts to do a hugepage mapping in a
 * 32-bit process it will automatically attempt to open the low region.  However,
 * that will fail if there are any normal (non-hugepage) mappings in the region
 * already.
 *   When run as a 64-bit process the kernel will still do a non-hugepage mapping
 * in the low region, but the following hugepage mapping will succeed. This is
 * because it comes from the high region, which is available to the 64-bit process.
 *   This test case is checking this behavior.
 *
 * Usage:  <for command-line>
 *  hugemmap02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
 *  None.
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
#define HPAGE_SIZE     ((1UL) << 24) 	/* Huge page size */
#define MAP_SIZE       (2*HPAGE_SIZE) 	/* Huge map page size */
#define LOW_ADDR       (void *)(0x80000000)
#define LOW_ADDR2      (void *)(0x90000000)

char* TEMPFILE="mmapfile";

char *TCID="hugemmap02";	/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
unsigned long *addr;		/* addr of memory mapped region */
unsigned long *addr2;		/* addr of memory mapped region */
unsigned long *addrlist[5];	/* list of addresses of memory mapped region */
unsigned long i;		/* for loop counter */
int fildes;			/* file descriptor for tempfile */
int nfildes;			/* file descriptor for /dev/zero */
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

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

	        /* Creat a temporary file used for huge mapping */
		if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "open() on %s Failed, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
		}
	        /* Creat a file used for normal mapping */
		if ((nfildes = open("/dev/zero",O_RDONLY, 0666)) < 0) {
			tst_brkm(TBROK, cleanup,
				 "open() on /dev/zero Failed, errno=%d : %s",
				  errno, strerror(errno));
		}
		/* Reset Tst_count in case we are looping. */
		Tst_count=0;

		/*
		 * Call mmap on /dev/zero 5 times
		 */
	       	for (i = 0; i < 5; i++) {
                	addr = mmap(0, 256*1024*1024, PROT_READ,
                        	MAP_SHARED, nfildes, 0);
                	addrlist[i] = addr;
        	}
	
		/* mmap using normal pages and a low memory address */
		errno = 0;
		addr = mmap(LOW_ADDR, PAGE_SIZE, PROT_READ,
			    MAP_SHARED | MAP_FIXED, nfildes, 0);
		if (addr == MAP_FAILED)
			tst_brkm(TBROK, cleanup,"mmap failed on nfildes");

		/* Attempt to mmap a huge page into a low memory address */
		errno = 0;
		addr2 = mmap(LOW_ADDR2, MAP_SIZE, PROT_READ | PROT_WRITE,
			    MAP_SHARED, fildes, 0);
	
#if __WORDSIZE==64 /* 64-bit process */
		if (addr2 == MAP_FAILED) {
			tst_resm(TFAIL, "huge mmap() unexpectedly failed on %s for 64-bit, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		} else {
			tst_resm(TPASS, "huge mmap() correctly succeeded for 64-bit");
		}
#else /* 32-bit process */
                if (addr2 > 0){
                        tst_resm(TCONF, "huge mmap() failed to test the scenario");
                        continue;
                } else if (addr == 0) {
                        tst_resm(TPASS, "huge mmap() succeeded for 32-bit");
                } else {
                        tst_resm(TFAIL, "huge mmap() unexpectedly failed %s for 32-bit, errno=%d : %s",
                                TEMPFILE, errno, strerror(errno));
                }
#endif

		/* Clean up things in case we are looping */
		for (i = 0; i < 5; i++) {
                	if (munmap(addrlist[i], 256*1024*1024))
                        	tst_resm(TBROK,"munmap of addrlist[%d] failed",i);
        	}
	
#if __WORDSIZE==64
		if (munmap(addr2, MAP_SIZE) != 0) {
			tst_brkm(TFAIL, NULL, "huge munmap() fails to unmap the "
				 "memory, errno=%d", errno);
		}
#endif
		if (munmap(addr, PAGE_SIZE) != 0) {
			tst_brkm(TFAIL, NULL, "munmap() fails to unmap the "
				 "memory, errno=%d", errno);
		}
	

		close(fildes);
	}	/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	/*NOTREACHED*/
	return 1;
}	/* End main */

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

	/* capture signals */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
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

	/* exit with return code appropriate for results */
	tst_exit();
}
