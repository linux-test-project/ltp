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
 * Test Name: mmap02
 *
 * Test Description:
 *  Call mmap() with prot parameter set to PROT_READ and with the file
 *  descriptor being open for read, to  map a file creating mapped memory
 *  with read access. The minimum file permissions should be 0444.
 *
 *  The call should succeed to create the mapped region with required
 *  attributes.
 *
 * Expected Result:
 *  mmap() should succeed returning the address of the mapped region,
 *  the mapped region should contain the contents of the mapped file.
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
 *  mmap02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"mmapfile"

char *TCID = "mmap02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
char *addr;			/* addr of memory mapped region */
char *dummy;			/* dummy string */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for temporary file */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * with read access.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_READ,
			    MAP_FILE | MAP_SHARED, fildes, 0);

		/* Check for the return value of mmap() */
		if (addr == MAP_FAILED) {
			tst_resm(TFAIL|TERRNO, "mmap of %s failed", TEMPFILE);
			continue;
		}
		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Read the file contents into the dummy
			 * string.
			 */
			if (read(fildes, dummy, page_sz) < 0) {
				tst_brkm(TFAIL|TERRNO, cleanup,
					"reading %s failed", TEMPFILE);
			}

			/*
			 * Check whether mapped memory region has
			 * the file contents.
			 */
			if (memcmp(dummy, addr, page_sz)) {
				tst_resm(TFAIL, "mapped memory area contains "
					 "invalid data");
			} else {
				tst_resm(TPASS,
					 "Functionality of mmap() successful");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
		}

		/* Clean up things in case we are looping */
		/* Unmap the mapped memory */
		if (munmap(addr, page_sz) != 0) {
			tst_brkm(TFAIL|TERRNO, cleanup, "munmapping failed");
		}

	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 *	Get system page size, allocate and initialize the string dummy.
 *	Initialize addr such that it is more than one page below the break
 *	address of the process, and initialize one page region from addr
 *	with char 'A'.
 *	Create a temporary directory a file under it.
 *	Write some known data into file and close it.
 *	Change mode permissions on file to 0444.
 */
void setup()
{
	char *tst_buff;		/* test buffer to hold known data */

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, NULL,
			 "getpagesize() fails to get system page size");
	}

	/* Allocate space for the test buffer */
	if ((tst_buff = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL, "calloc failed (tst_buff)");
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		free(tst_buff);
		tst_brkm(TFAIL|TERRNO, cleanup, "opening %s failed", TEMPFILE);
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) < page_sz) {
		free(tst_buff);
		tst_brkm(TFAIL|TERRNO, cleanup,
			"writing to %s failed", TEMPFILE);
		cleanup();
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Change Mode permissions on Temporary file */
	if (fchmod(fildes, 0444) < 0) {
		tst_brkm(TFAIL|TERRNO, cleanup, "fchmod(%s, 0444) failed",
			TEMPFILE);
	}

	/* Close the temporary file */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL|TERRNO, cleanup, "closing %s failed", TEMPFILE);
	}

	/* Open the temporary file again, - Readonly mode */
	if ((fildes = open(TEMPFILE, O_RDONLY)) < 0) {
		tst_brkm(TFAIL, cleanup, "reopening %s readonly failed",
			TEMPFILE);
	}

	/* Allocate and initialize dummy string of system page size bytes */
	if ((dummy = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, cleanup, "calloc failed (dummy)");
	}

}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Free the memory allocated to dummy variable.
 *	       Remove the temporary directory created.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 */
	close(fildes);

	TEST_CLEANUP;

	/* Free the memory allocated for dummy string */
	if (dummy) {
		free(dummy);
	}

	tst_rmdir();
}
