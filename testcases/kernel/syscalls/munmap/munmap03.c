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
 * Test Name: munmap03
 *
 * Test Description:
 *  Verify that, munmap call will fail to unmap a mapped file or anonymous
 *  shared memory region from the calling process's address space if the
 *  address and the length of the region to be unmapped points outside the
 *  calling process's address space
 *
 * Expected Result:
 *  munmap call should fail with return value -1 and sets errno EINVAL.
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
 *   	if errno set == expected errno
 *   		Issue sys call fails with expected return value and errno.
 *   	Otherwise,
 *		Issue sys call fails with unexpected errno.
 *   Otherwise,
 *	Issue sys call returns unexpected value.
 *
 *  Cleanup:
 *   Print errno log and/or timing stats if options given
 *   Delete the temporary directory(s)/file(s) created.
 *
 * Usage:  <for command-line>
 *  munmap03 [-c n] [-e] [-i n] [-I x] [-P x] [-t]
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
#include <sys/resource.h>
#include <sys/stat.h>

#include "test.h"
#include "usctest.h"

#define TEMPFILE	"mmapfile"

char *TCID = "munmap03";	/* Test program identifier.    */
int TST_TOTAL = 1;		/* Total number of test cases. */
extern int Tst_count;		/* Test Case counter for tst_* routines */

size_t page_sz;			/* system page size */
char *addr;			/* addr of memory mapped region */
char *faddr;			/* addr of memory mapped region */
int fildes;			/* file descriptor for tempfile */
size_t map_len;			/* length of the region to be mapped */

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

	/* set the expected errnos... */
	TEST_EXP_ENOS(exp_enos);

	/* Check looping state if -i option given */
	for (lc = 0; TEST_LOOPING(lc); lc++) {

		/* Reset Tst_count in case we are looping. */
		Tst_count = 0;

		/* Perform global setup for test */
		setup();

		/*
		 * Attempt to unmap the mapped region of the temporary file
		 * from the address that points outside the process's
		 * address space.
		 */
		TEST(munmap(addr, map_len));

		/* Check for the return value of munmap() */
		if (TEST_RETURN != -1) {
			tst_resm(TFAIL, "munmap() returned %ld, expected -1",
				 TEST_RETURN);
			continue;
		}

		TEST_ERROR_LOG(TEST_ERRNO);

		/* Check for expected errno. */
		if (TEST_ERRNO == EINVAL) {
			tst_resm(TPASS, "munmap() fails, mapped address is "
				 "invalid, errno:%d", TEST_ERRNO);
		} else {
			tst_resm(TFAIL, "munmap() fails, invalid errno:%d, "
				 "expected:%d", TEST_ERRNO, EINVAL);
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
 * Get system page size, create a temporary file for reading/writing,
 * write one byte data into it, map the open file for the specified
 * map length.
 */
void setup()
{
	struct rlimit brkval;	/* variable to hold max. break val */

	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* call getrlimit function to get the maximum possible break value */
	getrlimit(RLIMIT_DATA, &brkval);

	/* Get the system page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TBROK, cleanup,
			 "getpagesize() fails to get system page size");
		tst_exit();
	}

	/*
	 * Get the length of the open file to be mapped into process
	 * address space.
	 */
	map_len = 3 * page_sz;

	/* make a temp directory and cd to it */
	tst_tmpdir();

	/* Creat a temporary file used for mapping */
	if ((fildes = open(TEMPFILE, O_RDWR | O_CREAT, 0666)) < 0) {
		tst_brkm(TBROK, cleanup, "open() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/*
	 * move the file pointer to maplength position from the beginning
	 * of the file.
	 */
	if (lseek(fildes, map_len, SEEK_SET) == -1) {
		tst_brkm(TBROK, cleanup, "lseek() fails on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* Write one byte into temporary file */
	if (write(fildes, "a", 1) != 1) {
		tst_brkm(TBROK, cleanup, "write() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/*
	 * map the open file 'TEMPFILE' from its beginning up to the maplength
	 * into the calling process's address space at the system choosen
	 * with read/write permissions to the the mapped region.
	 */
#ifdef UCLINUX
	/* MAP_SHARED not implemented on uClinux */
	faddr = mmap(0, map_len, PROT_READ | PROT_WRITE,
		     MAP_FILE | MAP_PRIVATE, fildes, 0);
#else
	faddr = mmap(0, map_len, PROT_READ | PROT_WRITE,
		     MAP_FILE | MAP_SHARED, fildes, 0);
#endif

	/* check for the return value of mmap system call */
	if (faddr == (char *)MAP_FAILED) {
		tst_brkm(TBROK, cleanup, "mmap() Failed on %s, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
		tst_exit();
	}

	/* convert the MAX. possible break value */
	addr = (char *)brkval.rlim_max;
}

/*
 * cleanup() - performs all ONE TIME cleanup for this test at
 *             completion or premature exit.
 *  	       Unmap the mapped region of the file.
 *  	       Close the temporary file.
 *  	       Remove the temporary directory.
 */
void cleanup()
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/*
	 * unmap the mapped region of the file from the process
	 * address space
	 */
	if (munmap(faddr, map_len) < 0) {
		tst_brkm(TBROK, NULL, "munmap() fails to unmap the mapped "
			 "region of the file");
	}

	/* Close the temporary file */
	if (close(fildes) < 0) {
		tst_brkm(TBROK, NULL, "close() on %s Failed, errno=%d : %s",
			 TEMPFILE, errno, strerror(errno));
	}

	/* Remove the temporary directory and all files in it */
	tst_rmdir();
}
