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
 * Test Name: mmap05
 *
 * Test Description:
 *  Call mmap() to map a file creating mapped memory with no access under
 *  the following conditions -
 *	- The prot parameter is set to PROT_NONE
 *	- The file descriptor is open for read(any mode other than write)
 *	- The minimum file permissions should be 0444.
 *
 *  The call should succeed to map the file creating mapped memory with the
 *  required attributes.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  and an attempt to access the contents of the mapped region should give
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
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory created.
 *
 * Usage:  <for command-line>
 *  mmap05 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "mmap05";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
size_t page_sz;			/* system page size */
volatile char *addr;		/* addr of memory mapped region */
int fildes;			/* file descriptor for temporary file */
volatile int pass = 0;
sigjmp_buf env;			/* environment for sigsetjmp/siglongjmp */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */
void sig_handler();		/* signal handler to catch SIGSEGV */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char file_content;	/* tempfile content */

	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * with no access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_NONE,
			    MAP_FILE | MAP_SHARED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL|TERRNO, "mmap() failed on %s", TEMPFILE);
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {

			/*
			 * Try to access the mapped region.  This should
			 * generate a SIGSEGV which will be caught below.
			 *
			 * This is wrapped by the sigsetjmp() call that will
			 * take care of restoring the program's context in an
			 * elegant way in conjunction with the call to
			 * siglongjmp() in the signal handler.
			 */
			if (sigsetjmp(env, 1) == 0) {
				file_content = addr[0];
			}

			if (pass) {
				tst_resm(TPASS, "Got SIGSEGV as expected");
			} else {
				tst_resm(TFAIL, "Mapped memory region with NO "
					 "access is accessible");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* Unmap mapped memory and reset pass in case we are looping */
		if (munmap((void *)addr, page_sz) != 0) {
			tst_brkm(TFAIL|TERRNO, cleanup, "munmap failed");
		}
		pass = 0;

	}
	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the system page size.
 *	     Create a temporary directory and a file under it.
 *	     Write some known data into file and close it.
 *	     Change the mode permissions on file to 0444
 *	     Re-open the file for reading.
 */
void setup(void)
{
	char *tst_buff;		/* test buffer to hold known data */

	tst_sig(NOFORK, sig_handler, cleanup);

	TEST_PAUSE;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, NULL,
			 "getpagesize() fails to get system page size");
	}

	/* Allocate space for the test buffer */
	if ((tst_buff = calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL, "calloc failed (tst_buff)");
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_WRONLY | O_CREAT, 0666)) < 0) {
		free(tst_buff);
		tst_brkm(TFAIL|TERRNO, cleanup, "opening %s failed", TEMPFILE);
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) != page_sz) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "writing to %s failed",
			 TEMPFILE);
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Make sure proper permissions set on file */
	if (fchmod(fildes, 0444) < 0) {
		tst_brkm(TFAIL|TERRNO, cleanup, "fchmod of %s failed",
			 TEMPFILE);
	}

	/* Close the temporary file opened for write */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL|TERRNO, cleanup, "closing %s failed",
			 TEMPFILE);
	}

	/* Open the temporary file again for reading */
	if ((fildes = open(TEMPFILE, O_RDONLY)) < 0) {
		tst_brkm(TFAIL|TERRNO, cleanup, "opening %s readonly failed",
			TEMPFILE);
	}
}

/*
 * sig_handler() - Signal Catching function.
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
	} else
		tst_brkm(TBROK, cleanup, "received an unexpected signal: %d",
			sig);
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Remove the temporary directory created.
 */
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	close(fildes);

	TEST_CLEANUP;

	tst_rmdir();
}
