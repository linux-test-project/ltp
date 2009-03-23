/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
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
 * Test Name: mmap03
 *
 * Test Description:
 *  Call mmap() to map a file creating a mapped region with execute access
 *  under the following conditions -
 *	- The prot parameter is set to PROT_EXE
 *	- The file descriptor is open for read
 *	- The file being mapped has execute permission bit set.
 *	- The minimum file permissions should be 0555.
 *
 *  The call should succeed to map the file creating mapped memory with the
 *  required attributes.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and the mapped region should contain the contents of the mapped file.
 *  but with ia64 and PARISC/hppa,
 *  an attempt to access the contents of the mapped region should give
 *  rise to the signal SIGSEGV.
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
 *	Log the errno and Issue a FAIL message.
 *   Otherwise,
 *	Verify the Functionality of system call
 *      if successful,
 *		Issue Functionality-Pass message.
 *      Otherwise,
 *		Issue Functionality-Fail message.
 *  Cleanup:
 *   Print timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  mmap03 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *	       -i n : Execute test n times.
 *	       -I x : Execute test for x seconds.
 *	       -P x : Pause for x seconds between iterations.
 *	       -t   : Turn on syscall timing.
 *
 * HISTORY
 *	07/2001 Ported by Wayne Boyer
 *
 * RESTRICTIONS:
 *  None.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <setjmp.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap03";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */
size_t page_sz;			/* system page size */
char *addr;			/* addr of memory mapped region */
char *dummy;			/* dummy variable to hold temp file contents */
int fildes;			/* file descriptor for temporary file */
volatile int pass = 0;		/* pass flag perhaps set to 1 in sig_handler */
sigjmp_buf env;			/* environment for sigsetjmp/siglongjmp */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal handler to catch SIGSEGV */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
		tst_exit();
	}

	/* Perform global setup for test */
	setup();

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * with execute access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_EXEC,
			    MAP_FILE | MAP_SHARED, fildes, 0);

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL, "mmap() Failed on %s, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Read the file contents into the dummy
			 * variable.
			 */
			if (read(fildes, dummy, page_sz) < 0) {
				tst_brkm(TFAIL, cleanup, "read() on %s Failed, "
					 "error:%d", TEMPFILE, errno);
			}

			/*
			 * Check whether the mapped memory region
			 * has the file contents.
			 *
			 * with ia64 and PARISC/hppa, this should
			 * generate a SIGSEGV which will be caught below.
			 *
			 */

			if (sigsetjmp(env, 1) == 0) {
				if (memcmp(dummy, addr, page_sz)) {
					tst_resm(TFAIL,
						 "mapped memory region contains "
						 "invalid data");
				} else {
					tst_resm(TPASS,
						 "Functionality of mmap() successful");
				}
			}
#if defined(__ia64__) || defined(__hppa__)
			if (pass) {
				tst_resm(TPASS, "Got SIGSEGV as expected");
			} else {
				tst_resm(TFAIL, "Mapped memory region with NO "
					 "access is accessible");
			}
#endif

		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
		if (munmap(addr, page_sz) != 0) {
			tst_brkm(TFAIL, NULL, "munmap() fails to unmap the "
				 "memory, errno=%d", errno);
		}
		pass = 0;

	}			/* End for TEST_LOOPING */

	/* Call cleanup() to undo setup done for the test. */
	cleanup();

	 /*NOTREACHED*/ return 0;
}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the system page size.
 *	     Create a temporary directory and a file under it.
 *	     Write some known data into file and close it.
 *	     Change the mode permissions on file to 0555.
 *	     Re-open the file for reading.
 */
void setup()
{
	char *tst_buff;		/* test buffer to hold known data */

	/* capture signals */
	tst_sig(NOFORK, sig_handler, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, NULL,
			 "getpagesize() fails to get system page size");
		tst_exit();
	}

	/* Allocate space for the test buffer */
	if ((tst_buff = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL,
			 "calloc() failed to allocate space for tst_buff");
		tst_exit();
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_WRONLY | O_CREAT, 0666)) < 0) {
		tst_brkm(TFAIL, NULL, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		free(tst_buff);
		cleanup();
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) < page_sz) {
		tst_brkm(TFAIL, NULL, "write() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		free(tst_buff);
		cleanup();
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Make sure proper permissions set on file */
	if (chmod(TEMPFILE, 0555) < 0) {
		tst_brkm(TFAIL, cleanup, "chmod() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Close the temporary file opened for write */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, cleanup, "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Allocate and initialize dummy string of system page size bytes */
	if ((dummy = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, cleanup,
			 "calloc() failed to allocate memory for dummy");
	}

	/* Open the temporary file again for reading */
	if ((fildes = open(TEMPFILE, O_RDONLY)) < 0) {
		tst_brkm(TFAIL, cleanup, "open(%s) with read-only Failed, "
			 "errno:%d", TEMPFILE, errno);
	}
}

/*
 * sig_handler() - Signal Cathing function.
 *   This function gets executed when the test process receives
 *   the signal SIGSEGV while trying to access the contents of memory which
 *   is not accessible.
 */
void sig_handler(sig)
{
	if (sig == SIGSEGV) {
		/* set the global variable and jump back */
		pass = 1;
		siglongjmp(env, 1);
	} else {
		tst_brkm(TBROK, cleanup, "received an unexpected signal");
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 *		Free the memory allocated to dummy variable.
 *		Remove the temporary directory created.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	close(fildes);

	TEST_CLEANUP;

	/* Free the memory space allocated for dummy variable */
	if (dummy) {
		free(dummy);
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();
}
