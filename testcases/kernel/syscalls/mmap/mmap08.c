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
 * Test Name: mmap08
 *
 * Test Description:
 *  Verify that mmap() fails to map a file creating a mapped region
 *  when the file specified by file descriptor is not valid.
 *
 * Expected Result:
 *  mmap() should fail returning -1 and errno should get set to EBADF.
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
 *	if errno set == expected errno
 *		Issue sys call fails with expected return value and errno.
 *	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  mmap08 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -e   : Turn on errno logging.
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

char *TCID = "mmap08";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
int exp_enos[] = { EBADF, 0 };

size_t page_sz;			/* system page size */
char *addr;			/* addr of memory mapped region */
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

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;

		/*
		 * Call mmap to map the temporary file 'TEMPFILE'
		 * which is already closed. so, fildes is not valid.
		 */
		errno = 0;
		addr = mmap(0, page_sz, PROT_WRITE,
			    MAP_FILE | MAP_SHARED, fildes, 0);
		TEST_ERRNO = errno;

		/* Check for the return value of mmap() */
		if (addr != MAP_FAILED) {
			tst_resm(TFAIL, "mmap() didn't fail (%p != %p)",
				addr, MAP_FAILED);
			/* Unmap the mapped memory */
			if (munmap(addr, page_sz) != 0) {
				tst_brkm(TBROK, cleanup, "munmap() failed");
			}
			continue;
		}
		TEST_ERROR_LOG(TEST_ERRNO);
		if (TEST_ERRNO == EBADF) {
			tst_resm(TPASS,
				"mmap failed with EBADF");
		} else {
			tst_resm(TFAIL|TERRNO,
				"mmap failed with an invalid errno");
		}
	}

	cleanup();
	tst_exit();

}

/*
 * setup() - performs all ONE TIME setup for this test.
 *	     Get the system page size.
 *	     Create a temporary directory and a file under it.
 *	     Write some known data into file and close it.
 */
void setup()
{
	char *tst_buff;		/* test buffer to hold known data */

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, NULL,
			"getpagesize() fails to get system page size");
	}

	if ((tst_buff = (char *)calloc(page_sz, sizeof(char))) == NULL) {
		tst_brkm(TFAIL, NULL,
			"calloc() failed to allocate space for tst_buff");
	}

	/* Fill the test buffer with the known data */
	memset(tst_buff, 'A', page_sz);

	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_WRONLY | O_CREAT, 0666)) < 0) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "opening %s failed", TEMPFILE);
	}

	/* Write test buffer contents into temporary file */
	if (write(fildes, tst_buff, page_sz) != page_sz) {
		free(tst_buff);
		tst_brkm(TFAIL, cleanup, "writing to %s failed", TEMPFILE);
	}

	/* Free the memory allocated for test buffer */
	free(tst_buff);

	/* Close the temporary file opened for writing */
	if (close(fildes) < 0) {
		tst_brkm(TFAIL, cleanup, "closing %s failed", TEMPFILE);
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Remove the temporary directory created.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	tst_rmdir();
}