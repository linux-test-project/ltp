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
 * Test Name: msync04
 *
 * Test Description:
 *  Verify that, msync() fails, when the region to synchronize, is mapped
 *  but the flags argument is invalid.
 *
 * Expected Result:
 *  msync() should fail with a return value of -1, and set errno EINVAL.
 *
 * Algorithm:
 *  Setup:
 *   Setup signal handling.
 *   Create temporary directory.
 *   Pause for SIGUSR1 if option specified.
 *
 *  Test:
 *   Loop if the proper options are given.
 *   Execute system call
 *   Check return code, if system call failed (return=-1)
 *      if errno set == expected errno
 *              Issue sys call fails with expected return value and errno.
 *      Otherwise,
 *              Issue sys call fails with unexpected errno.
 *   Otherwise,
 *      Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  msync04 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"msync_file"
#define BUF_SIZE	256
#define INV_SYNC	-1

char *TCID = "msync04";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */

int exp_enos[] = { EINVAL, 0 };

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Perform global setup for test */
		setup();

		/*
		 * Call msync to synchronize the mapped region
		 * with the specified file using invalid flag.
		 * INV_SYNC.
		 */
		TEST(msync(addr, page_sz, INV_SYNC));

		/* Check for the return value of msync() */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "msync() returns unexpected value %ld, "
				 "expected:-1", TEST_RETURN);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/*
		 * Verify whether expected errno is
		 * set (EINVAL).
		 */
		if (TEST_ERRNO == EINVAL) {
			tst_resm(TPASS, "mapped region is private and cannot "
				 "sync, errno:%d", TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "msync() failed, unexpected errno:%d, "
				 "expected: EINVAL", TEST_ERRNO);
		}

		/* Call cleanup() to undo setup done for the test. */
		cleanup();

	}			/* End for TEST_LOOPING */

	/* exit with return code appropriate for results */
	tst_exit();

	 /*NOTREACHED*/ return 0;

}				/* End main */

/*
 * setup() - performs all ONE TIME setup for this test.
 *
 * Get system page size,
 * Creat a temporary directory and a file under it used for mapping.
 * Write 1 page size char data into file.
 * Map the file to the specified page size region.
 */
void setup()
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */
	char tst_buf[BUF_SIZE];	/* buffer to hold data to be written */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TBROK, cleanup,
			 "getpagesize() fails to get system page size");
		tst_exit();
	}

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* Write one page size of char data into temporary file */
	while (c_total < page_sz) {
		if ((nwrite = write(fildes, tst_buf, sizeof(tst_buf))) <= 0) {
			tst_brkm(TBROK, cleanup,
				 "write() on %s Failed, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			tst_exit();
		} else {
			c_total += nwrite;
		}
	}

	/*
	 * Call mmap to map virtual memory (mul. of page size bytes) from the
	 * beginning of temporary file (offset is 0) into memory.
	 */
	addr = mmap(0, page_sz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_PRIVATE,
		    fildes, 0);

	/* Check for the return value of mmap() */
	if (addr == (char *)MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap() Failed on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *	       Unmap the mapped memory area done in the test.
 *	       Close the temporary file.
 *	       Remove the temporary directory created.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Unmap the mapped memory */
	if (munmap(addr, page_sz) != 0) {
		tst_brkm(TBROK, NULL, "munmap() fails to unmap the memory, "
			 "errno=%d", errno);
	}

	/* Close the temporary file */
	if (close(fildes) < 0) {
		tst_brkm(TBROK, NULL, "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();
}
