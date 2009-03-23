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
 * Test Name: msync01
 *
 * Test Description:
 *  Verify that, msync() succeeds, when the region to synchronize, is part
 *  of, or all of a mapped region.
 *
 * Expected Result:
 *  msync() should succeed with a return value of 0, and succesfully
 *  synchronize the memory region.  Data read from mapped region should be
 *  the same as the initialized data.
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
 *  msync01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"msync_file"
#define BUF_SIZE	256

char *TCID = "msync01";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */
char write_buf[BUF_SIZE];	/* buffer to hold data to be written */

void setup();			/* Main setup function of test */
void cleanup();			/* cleanup function for the test */

int main(int ac, char **av)
{
	int lc;			/* loop counter */
	char *msg;		/* message returned from parse_opts */
	char read_buf[BUF_SIZE];	/* buffer to hold data read from file */
	int nread = 0, count, err_flg = 0;

	/* Parse standard options given to run the test. */
	msg = parse_opts(ac, av, (option_t *) NULL, NULL);
	if (msg != (char *)NULL) {
		tst_brkm(TBROK, tst_exit, "OPTION PARSING ERROR - %s", msg);
	}

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Perform global setup for test */
		setup();

		/*
		 * Call msync to synchronize the mapped region
		 * with the specified file.
		 */
		TEST(msync(addr, page_sz, MS_ASYNC));

		/* Check for the return value of msync() */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "msync() failed to synchronize mapped "
				 "file %s, errno=%d : %s",
				 TEMPFILE, errno, strerror(errno));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Seek to the offset pos. where bytes were
			 * set in the setup.
			 */
			if (lseek(fildes, (off_t) 100, SEEK_SET) != (off_t) 100) {
				tst_brkm(TBROK, cleanup, "lseek() to specified "
					 "offset pos. Failed, error=%d : %s",
					 errno, strerror(errno));
				tst_exit();
			}

			/*
			 * Seeking to specified offset. successful.
			 * Now, read the data (256 bytes) and compare
			 * them with the expected.
			 */
			nread = read(fildes, read_buf, sizeof(read_buf));
			if (nread != BUF_SIZE) {
				tst_brkm(TBROK, cleanup, "read() on %s Failed, "
					 "error : %d", TEMPFILE, errno);
				tst_exit();
			} else {
				/*
				 * Check whether read data (from mapped
				 * file) contains the expected data
				 * which was initialised in the setup.
				 */
				for (count = 0; count < nread; count++) {
					if (read_buf[count] != 1) {
						/* invalid data */
						err_flg++;
					}
				}
			}

			if (err_flg != 0) {
				tst_resm(TFAIL,
					 "data read from file doesn't match");
			} else {
				tst_resm(TPASS,
					 "Functionality of msync() successful");
			}
		} else {
			tst_resm(TPASS, "call succeeded");
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
 * Initialize paged region (256 bytes) from the specified offset pos.
 */
void setup()
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */

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
		tst_brkm(TBROK, cleanup, "open() on %s failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* Write one page size of char data into temporary file */
	while (c_total < page_sz) {
		nwrite = write(fildes, write_buf, sizeof(write_buf));
		if (nwrite <= 0) {
			tst_brkm(TBROK, cleanup, "write() on %s failed, errno "
				 " = %d : %s", TEMPFILE, errno,
				 strerror(errno));
			tst_exit();
		} else {
			c_total += nwrite;
		}
	}

	/*
	 * Call mmap to map virtual memory (mul. of page size bytes) from the
	 * beginning of temporary file (offset is 0) into memory.
	 */
	addr = mmap(0, page_sz, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED,
		    fildes, 0);

	/* Check for the return value of mmap() */
	if (addr == (char *)MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap() failed on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* Set 256 bytes, at 100 byte offset in the mapped region */
	memset(addr + 100, 1, 256);
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
		tst_brkm(TBROK, NULL, "munmap() failed to unmap the memory, "
			 "errno=%d", errno);
	}

	/* Close the temporary file */
	if (close(fildes) < 0) {
		tst_brkm(TBROK, NULL, "close() on %s failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Remove tmp dir and all files in it */
	tst_rmdir();
}
