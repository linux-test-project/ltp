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
 * Test Name: msync02
 *
 * Test Description:
 *  Verify that msync() succeeds when the region to synchronize is mapped
 *  shared and the flags argument is MS_INVALIDATE.
 *
 * Expected Result:
 *  msync() should succeed with a return value of 0, and successfully
 *  synchronize the memory region.
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
 *  msync02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
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

char *TCID = "msync02";		/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

char *addr;			/* addr of memory mapped region */
size_t page_sz;			/* system page size */
int fildes;			/* file descriptor for tempfile */
char write_buf[10] = "Testing";	/* buffer to hold data to be written */

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
		TEST(msync(addr, page_sz, MS_INVALIDATE));

		/* Check for the return value of msync() */
		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "msync() failed to synchronize mapped "
				 "file %s, TEST_ERRNO=%d : %s",
				 TEMPFILE, TEST_ERRNO, strerror(TEST_ERRNO));
			continue;
		}

		/*
		 * Perform functional verification if test
		 * executed without (-f) option.
		 */
		if (STD_FUNCTIONAL_TEST) {
			/*
			 * Compare the mapped memory region at
			 * specified offset (100) with the string
			 * data written to the file.
			 */
			if (memcmp(addr + 100, write_buf,
				   strlen(write_buf)) != 0) {
				tst_resm(TFAIL,
					 "memory region contains invalid data");
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
 * Map the file to the specified page size region.
 * Seek the file ptr to the specified offset pos (100) from beginning.
 * Write known data at the specified offset position.
 */
void setup()
{
	int c_total = 0, nwrite = 0;	/* no. of bytes to be written */
	char tst_buf[BUF_SIZE];

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
			tst_brkm(TBROK, cleanup, "write() on %s Failed, errno "
				 "= %d : %s", TEMPFILE, errno, strerror(errno));
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
		tst_brkm(TBROK, cleanup, "mmap() Failed on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* Again, Seek to the specified byte offset (100) position */
	if (lseek(fildes, (off_t) 100, SEEK_SET) != (off_t) 100) {
		tst_brkm(TBROK, cleanup, "lseek() fails to seek to specified "
			 "offset pos., error=%d", errno);
		tst_exit();
	}

	/* Write the string in write_buf at the 100 byte offset */
	if (write(fildes, write_buf, strlen(write_buf)) != strlen(write_buf)) {
		tst_brkm(TBROK, cleanup, "write() fails to write specified "
			 "string, error=%d", errno);
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
