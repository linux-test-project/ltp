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
 * Test Name: hugemmap01
 *
 * Test Description:
 *  Verify that, mmap() succeeds when used to map a file in a hugetlbfs.
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
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *   	Log the errno and Issue a FAIL message.
 *  Cleanup:
 *   Print timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  hugemmap01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#define BUFFER_SIZE  256

char* TEMPFILE="mmapfile";

char *TCID="hugemmap01";	/* Test program identifier.    */
int TST_TOTAL=1;		/* Total number of test cases. */
long *addr;			/* addr of memory mapped region */
int fildes;			/* file descriptor for tempfile */
char *Hopt;                     /* location of hugetlbfs */
int beforetest=0;		/* Amount of free huge pages before testing */
int aftertest=0;		/* Amount of free huge pages after testing */
int hugepagesmapped=0;		/* Amount of huge pages mapped after testing */

void setup();			/* Main setup function of test */
int getfreehugepages();		/* Reads free huge pages */
int get_huge_pagesize();        /* Reads huge page size */
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
	int page_sz=0;

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

	        /* Creat a temporary file used for mapping */
		if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
			tst_brkm(TFAIL, cleanup,
				 "open() on %s Failed, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
		}

		Tst_count=0;

		/* Note the number of free huge pages BEFORE testing */
		beforetest = getfreehugepages();

		/* Note the size of huge page size BEFORE testing */
		page_sz = get_huge_pagesize();

		/*
		 * Call mmap
		 */
		errno = 0;
                addr = mmap(NULL, page_sz*1024, PROT_READ | PROT_WRITE,
			    MAP_SHARED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL, "mmap() Failed on %s, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		} else {
			tst_resm(TPASS, "call succeeded");
			/* force to allocate page and change HugePages_Free */
			*(int*)addr = 0;
		}

		/* Make sure the number of free huge pages AFTER testing decreased */
		aftertest = getfreehugepages();
		hugepagesmapped = beforetest - aftertest;
		if (hugepagesmapped < 1) {
			tst_resm(TWARN,"Number of HUGEPAGES_FREE stayed the same. Okay if");
			tst_resm(TWARN,"multiple copies running due to test collision.");
		}
		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
                if (munmap(addr, page_sz*1024) != 0) {
			tst_brkm(TFAIL, NULL, "munmap() fails to unmap the "
				 "memory, errno=%d", errno);
		}
		close(fildes);
	}

	cleanup();

	return 1;
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
 * getfreehugepages() - Reads the number of free huge pages from /proc/meminfo
 */
int
getfreehugepages()
{
	int hugefree;
	FILE* f;
	int retcode=0;
	char buff[BUFFER_SIZE];

        f = fopen("/proc/meminfo", "r");
	if (!f)
     		tst_brkm(TFAIL, cleanup, "Could not open /proc/meminfo for reading");

	while (fgets(buff,BUFFER_SIZE, f) != NULL) {
		if ((retcode = sscanf(buff, "HugePages_Free: %d ", &hugefree)) == 1)
	  		break;
	}

        if (retcode != 1) {
        	fclose(f);
       		tst_brkm(TFAIL, cleanup, "Failed reading number of huge pages free.");
     	}
	fclose(f);
	return(hugefree);
}

/*
 * get_huge_pagesize() - Reads the size of huge page size from /proc/meminfo
 */
int
get_huge_pagesize()
{
        int hugesize;
        FILE* f;
        int retcode=0;
        char buff[BUFFER_SIZE];

        f = fopen("/proc/meminfo", "r");
        if (!f)
                tst_brkm(TFAIL, cleanup, "Could not open /proc/meminfo for reading");

        while (fgets(buff,BUFFER_SIZE, f) != NULL) {
                if ((retcode = sscanf(buff, "Hugepagesize: %d ", &hugesize)) == 1)
                        break;
        }

        if (retcode != 1) {
                fclose(f);
                tst_brkm(TFAIL, cleanup, "Failed reading size of huge page.");
        }
        fclose(f);
        return(hugesize);
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